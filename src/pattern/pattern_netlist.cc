// Copyright 2026 The ICL Toolsets Authors.
// SPDX-License-Identifier: MIT
#include "pattern/pattern_netlist.h"

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <functional>
#include <set>
#include <stdexcept>
#include <utility>

#include "pattern/pattern_flatten.h"
#include "pattern/pattern_logic.h"

namespace pattern {
namespace {
constexpr size_t kMaxObjects = 1000000;
constexpr size_t kMaxWidth = 1024 * 1024;
constexpr size_t kMaxDepth = 128;
constexpr size_t kMaxMuxBits = 4 * 1024 * 1024;
constexpr size_t kMaxPortGroupBits = 4 * 1024 * 1024;

void SpendMuxBits(size_t bits, size_t* remaining) {
  if (bits > *remaining) {
    throw std::runtime_error("mux expansion bit limit exceeded");
  }
  *remaining -= bits;
}

std::string Path(const std::string& scope, const std::string& name) {
  return scope.empty() ? name : scope + "." + name;
}

ModelKind Model(const std::string& kind) {
  if (kind == "ScanRegister" || kind == "DataRegister") {
    return ModelKind::kRegister;
  }
  if (kind == "ScanMux" || kind == "DataMux" || kind == "ClockMux") {
    return ModelKind::kMux;
  }
  if (kind == "LogicSignal") return ModelKind::kLogic;
  if (kind == "Alias") return ModelKind::kAlias;
  if (kind == "Enum") return ModelKind::kEnum;
  if (kind == "ScanInterface") return ModelKind::kInterface;
  if (kind == "OneHotScanGroup" || kind == "OneHotDataGroup" ||
      kind == "PortGroup") {
    return ModelKind::kGroup;
  }
  if (kind == "AccessLink") return ModelKind::kAccessLink;
  return ModelKind::kPort;
}

size_t TokenAt(const ModuleDefinition& module, size_t byte) {
  const auto& tokens = module.document->driver().unit().tokens();
  return static_cast<size_t>(
      std::lower_bound(tokens.begin(), tokens.end(), byte,
                       [](const icl::SourceToken& token, size_t offset) {
                         return token.begin < offset;
                       }) -
      tokens.begin());
}

Expression Tokens(const ModuleDefinition& module, const icl::AstNode& node,
                  size_t scope) {
  const auto range = module.document->Range(node.span());
  return {scope, TokenAt(module, range.begin), TokenAt(module, range.end)};
}

const std::vector<icl::SourceToken>& SourceTokens(const Netlist& netlist,
                                                  size_t scope) {
  return netlist.scopes.at(scope).definition.document->driver().unit().tokens();
}

Constant Parameter(const InstanceScope& scope, const std::string& name) {
  const auto found = scope.parameters.find(name);
  if (found == scope.parameters.end()) {
    throw std::runtime_error("unknown parameter " + name);
  }
  return found->second;
}

bool CallbackIdentifier(const std::string& name) {
  const auto letter = [](unsigned char c) {
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
  };
  if (name.empty() || !letter(name.front())) return false;
  return std::all_of(name.begin() + 1, name.end(), [&](unsigned char c) {
    return letter(c) || (c >= '0' && c <= '9') || c == '_';
  });
}

RegisterCallback ReadRegisterCallback(const Netlist& netlist,
                                      const Expression& expression) {
  const auto& tokens = SourceTokens(netlist, expression.scope);
  size_t cursor = expression.begin;
  const auto identifier = [&]() {
    const auto& text = tokens.at(cursor++).text;
    const auto value =
        text.front() == '$'
            ? netlist.Value({expression.scope, cursor - 1, cursor})
            : Constant{true, true, text};
    if (!value.is_string || !CallbackIdentifier(value.data)) {
      throw std::runtime_error("invalid resolved callback identifier");
    }
    return value.data;
  };
  RegisterCallback result;
  if (tokens.at(cursor).text == "::") {
    result.name_space = "::";
    ++cursor;
  }
  result.name_space += identifier();
  while (cursor < expression.end && tokens[cursor].text == "::") {
    ++cursor;
    result.name_space += "::" + identifier();
  }
  result.procedure = identifier();
  while (cursor < expression.end) {
    const auto& text = tokens[cursor].text;
    if (text == "<D>" || text == "<R>") {
      result.arguments.push_back({text == "<D>"
                                      ? CallbackArgumentKind::kData
                                      : CallbackArgumentKind::kRegister,
                                  {}});
      ++cursor;
      continue;
    }
    const size_t begin = cursor;
    const auto atom = [&]() {
      if (tokens[cursor++].text != "(") return;
      size_t depth = 1;
      while (cursor < expression.end && depth) {
        const auto& token = tokens[cursor++].text;
        if (token == "(") ++depth;
        if (token == ")") --depth;
      }
    };
    atom();
    // Callback numbers share the existing integer-expression grammar.
    while (cursor < expression.end && tokens[cursor].text.size() == 1 &&
           std::string("+-*/%").find(tokens[cursor].text[0]) !=
               std::string::npos) {
      ++cursor;
      atom();
    }
    result.arguments.push_back(
        {CallbackArgumentKind::kValue,
         netlist.Value({expression.scope, begin, cursor})});
  }
  return result;
}

ClockPeriod ReadClockPeriod(const Netlist& netlist,
                            const Expression& expression) {
  const auto& source = SourceTokens(netlist, expression.scope);
  auto number = source.at(expression.begin);
  ClockPeriod period;
  const auto suffix = number.text.find_first_not_of("0123456789_");
  if (suffix != std::string::npos) {
    period.unit = number.text.substr(suffix);
    number.text.resize(suffix);
  } else if (expression.end > expression.begin + 1) {
    period.unit = source.at(expression.begin + 1).text;
  }
  if (period.unit != "s" && period.unit != "ms" && period.unit != "us" &&
      period.unit != "ns" && period.unit != "ps" && period.unit != "fs" &&
      period.unit != "as") {
    throw std::runtime_error("invalid Period time unit");
  }
  period.magnitude = EvaluateInteger({number}, 0, 1, {});
  if (period.magnitude <= 0) {
    throw std::runtime_error("Period must be positive");
  }
  return period;
}

std::optional<std::pair<int64_t, int64_t>> SelectedRange(
    const Netlist& netlist, const Expression& selection) {
  if (selection.begin == selection.end) return std::nullopt;
  const auto& tokens = SourceTokens(netlist, selection.scope);
  if (tokens[selection.begin].text != "[" ||
      tokens[selection.end - 1].text != "]") {
    throw std::runtime_error("invalid vector selection");
  }
  size_t colon = selection.begin + 1;
  while (colon < selection.end - 1 && tokens[colon].text != ":") ++colon;
  const auto left =
      netlist.Integer({selection.scope, selection.begin + 1, colon});
  return std::make_pair(left, colon < selection.end - 1
                                  ? netlist.Integer({selection.scope, colon + 1,
                                                     selection.end - 1})
                                  : left);
}

constexpr unsigned char kResetUse = 1;
constexpr unsigned char kCaptureUse = 2;
constexpr unsigned char kUpdateUse = 4;
constexpr unsigned char kTmsUse = 8;
constexpr unsigned char kTrstUse = 16;
constexpr unsigned char kObservedUse = 32;

unsigned char PortControlUse(const std::string& kind) {
  if (kind == "ResetPort" || kind == "ToResetPort") {
    return kResetUse | kTrstUse;
  }
  if (kind == "CaptureEnPort" || kind == "ToCaptureEnPort") return kCaptureUse;
  if (kind == "UpdateEnPort" || kind == "ToUpdateEnPort") return kUpdateUse;
  if (kind == "TMSPort" || kind == "ToTMSPort") return kTmsUse;
  if (kind == "TRSTPort" || kind == "ToTRSTPort") return kTrstUse;
  return 0;
}

struct ControlContexts {
  std::vector<size_t> offsets;
  std::vector<unsigned char> uses;
};

ControlContexts FindMuxControlUses(const Netlist& netlist) {
  constexpr unsigned char kQueued = 128;
  ControlContexts result;
  result.offsets.push_back(0);
  size_t pending_capacity = 0;
  for (const auto& node : netlist.nodes) {
    result.offsets.push_back(result.offsets.back() + node.Width());
    if (node.model == ModelKind::kAlias || node.kind == "DataMux") {
      pending_capacity += node.Width();
    }
  }
  // The preceding combinational check bounds total width to16M bits.
  result.uses.resize(result.offsets.back());
  std::vector<uint32_t> pending;
  pending.reserve(pending_capacity);
  const auto mark = [&](const BitReference& bit, unsigned char mask) {
    if (bit.node == std::numeric_limits<size_t>::max()) return;
    const auto& node = netlist.nodes.at(bit.node);
    if (node.model != ModelKind::kAlias && node.kind != "DataMux") return;
    const size_t position =
        node.left >= node.right ? node.left - bit.index : bit.index - node.left;
    const size_t id = result.offsets.at(bit.node) + position;
    auto& use = result.uses.at(id);
    if ((use & mask) == mask) return;
    const bool queued = use & kQueued;
    use |= mask;
    if (!queued) {
      use |= kQueued;
      pending.push_back(static_cast<uint32_t>(id));
    }
  };
  for (const auto& node : netlist.nodes) {
    for (const auto& [role, bits] : node.inputs) {
      if (node.model == ModelKind::kAlias && role == "Source") continue;
      const unsigned char use =
          kObservedUse | (role == "Source" ? PortControlUse(node.kind) : 0);
      for (const auto& bit : bits) mark(bit, use);
    }
    if (node.kind != "DataMux") {
      for (const auto& choice : node.choices) {
        for (const auto& bit : choice.source) mark(bit, kObservedUse);
      }
    }
    if (node.reset) mark(*node.reset, kObservedUse | kResetUse | kTrstUse);
    for (const auto& source : node.one_hot_sources) {
      for (const auto& bit : source.bits) mark(bit, kObservedUse);
    }
    if (node.scan_interface) {
      for (const auto& bit : node.scan_interface->ports) {
        mark(bit, kObservedUse);
      }
      for (const auto& chain : node.scan_interface->chains) {
        for (const auto& bit : chain.ports) mark(bit, kObservedUse);
      }
    }
  }
  while (!pending.empty()) {
    const size_t bit = pending.back();
    pending.pop_back();
    const unsigned char use = result.uses[bit] & ~kQueued;
    result.uses[bit] = use;
    const size_t id = static_cast<size_t>(
        std::upper_bound(result.offsets.begin(), result.offsets.end(), bit) -
        result.offsets.begin() - 1);
    const auto& node = netlist.nodes[id];
    const size_t position = bit - result.offsets[id];
    if (node.model == ModelKind::kAlias) {
      mark(node.inputs.at("Source").at(position), use);
    } else {
      for (const auto& choice : node.choices) {
        mark(choice.source.at(position), use);
      }
    }
  }
  return result;
}

class AliasResolver {
 public:
  explicit AliasResolver(const Netlist& netlist) : netlist_(netlist) {}

  BitReference operator()(BitReference bit) {
    std::vector<std::pair<Key, bool>> path;
    bool inverted = bit.inverted;
    while (bit.node != std::numeric_limits<size_t>::max()) {
      const auto& node = netlist_.nodes.at(bit.node);
      if (node.model != ModelKind::kAlias) break;
      const Key key{bit.node, bit.index};
      const auto found = aliases_.find(key);
      if (found != aliases_.end()) {
        if (!found->second) {
          throw std::runtime_error(
              node.origin + ": combinational cycle in alias " + node.name);
        }
        bit = *found->second;
        inverted = inverted != bit.inverted;
        break;
      }
      aliases_.emplace(key, std::nullopt);
      path.emplace_back(key, inverted);
      const size_t offset = node.left >= node.right ? node.left - bit.index
                                                    : bit.index - node.left;
      bit = node.inputs.at("Source").at(offset);
      inverted = inverted != bit.inverted;
    }
    bit.inverted = inverted;
    for (const auto& [key, incoming_inversion] : path) {
      auto definition = bit;
      definition.inverted = definition.inverted != incoming_inversion;
      aliases_.at(key) = definition;
    }
    return bit;
  }

 private:
  using Key = std::pair<size_t, int64_t>;
  const Netlist& netlist_;
  std::map<Key, std::optional<BitReference>> aliases_;
};

std::optional<bool> ResetPolarity(const Netlist& netlist,
                                  AliasResolver& terminal, BitReference bit) {
  bit = terminal(bit);
  if (bit.node == std::numeric_limits<size_t>::max()) return std::nullopt;
  const auto& node = netlist.nodes.at(bit.node);
  if (node.kind != "ResetPort" && node.kind != "ToResetPort" &&
      node.kind != "TRSTPort" && node.kind != "ToTRSTPort") {
    return std::nullopt;
  }
  return node.active_low != bit.inverted;
}

void CheckSignalSources(const Netlist& netlist, AliasResolver& terminal) {
  enum class Kind {
    kOther,
    kData,
    kDataPort,
    kClock,
    kTck,
    kReset,
    kTrst,
    kTms,
    kScan,
    kAddress,
    kWriteEn,
    kReadEn,
    kScanRegister,
    kIntermediateScanRegister,
    kGlobalReset,
    kAsyncReset,
    kCapture,
    kShift,
    kUpdate,
    kConstant,
    kUnconnected,
    kIrSelect
  };
  enum class Family {
    kData,
    kClock,
    kTck,
    kReset,
    kCapture,
    kShift,
    kUpdate,
    kTms,
    kTrst,
    kScan,
    kScanInput,
    kAddress,
    kWriteEn,
    kReadEn,
    kDataMux
  };
  static const std::set<std::string> kDataKinds = {
      "SelectPort",  "ToSelectPort", "DataRegister",
      "LogicSignal", "DataMux",      "OneHotDataGroup"};
  static const std::set<std::string> kClockKinds = {"ClockPort", "ToClockPort",
                                                    "ClockMux"};
  std::vector<Kind> kinds;
  kinds.reserve(netlist.nodes.size());
  for (const auto& node : netlist.nodes) {
    if ((node.kind == "TRSTPort" || node.kind == "ToTRSTPort") &&
        !node.active_low) {
      throw std::runtime_error(node.origin +
                               ": invalid fixed-low TRST polarity");
    }
    kinds.push_back(
        kDataKinds.count(node.kind) ? Kind::kData
        : node.kind == "DataInPort" || node.kind == "DataOutPort"
            ? Kind::kDataPort
        : node.kind == "ScanRegister" ? Kind::kScanRegister
        : node.kind == "ScanInPort" || node.kind == "ScanOutPort" ||
                node.kind == "ScanMux" || node.kind == "OneHotScanGroup"
            ? Kind::kScan
        : node.kind == "CaptureEnPort" || node.kind == "ToCaptureEnPort"
            ? Kind::kCapture
        : node.kind == "ShiftEnPort" || node.kind == "ToShiftEnPort"
            ? Kind::kShift
        : node.kind == "UpdateEnPort" || node.kind == "ToUpdateEnPort"
            ? Kind::kUpdate
        : node.kind == "ResetPort" || node.kind == "ToResetPort" ? Kind::kReset
        : node.kind == "TRSTPort" || node.kind == "ToTRSTPort"   ? Kind::kTrst
        : node.kind == "TMSPort" || node.kind == "ToTMSPort"     ? Kind::kTms
        : node.kind == "AddressPort"                         ? Kind::kAddress
        : node.kind == "WriteEnPort"                         ? Kind::kWriteEn
        : node.kind == "ReadEnPort"                          ? Kind::kReadEn
        : node.kind == "TCKPort" || node.kind == "ToTCKPort" ? Kind::kTck
        : kClockKinds.count(node.kind) &&
                !node.properties.count("DifferentialInvOf")
            ? Kind::kClock
        : node.kind == "ToIRSelectPort" ? Kind::kIrSelect
                                        : Kind::kOther);
  }
  const auto classify = [&](BitReference bit) {
    bit = terminal(bit);
    if (bit.node == std::numeric_limits<size_t>::max()) {
      // Missing input-binding holes retain their existing validation path.
      return bit.virtual_signal == VirtualSignal::kNone
                 ? (bit.constant == '?' ? Kind::kUnconnected : Kind::kConstant)
             : bit.virtual_signal == VirtualSignal::kGlobalReset
                 ? Kind::kGlobalReset
                 : Kind::kAsyncReset;
    }
    const auto& node = netlist.nodes.at(bit.node);
    const auto kind = kinds.at(bit.node);
    return kind == Kind::kScanRegister && bit.index != node.right
               ? Kind::kIntermediateScanRegister
               : kind;
  };
  const auto control_use = [](Kind kind) -> unsigned char {
    return kind == Kind::kReset     ? kResetUse
           : kind == Kind::kCapture ? kCaptureUse
           : kind == Kind::kUpdate  ? kUpdateUse
           : kind == Kind::kTms     ? kTmsUse
           : kind == Kind::kTrst    ? kTrstUse
                                    : 0;
  };
  unsigned char mux_control_sources = 0;
  bool intercepted_control_mux = false;
  for (const auto& node : netlist.nodes) {
    const auto check = [&](const std::string& role,
                           const std::vector<BitReference>& bits,
                           Family family = Family::kData) {
      const bool ir_select = role == "SelectedBy" && node.kind == "ScanMux" &&
                             bits.size() == 1 && node.choices.size() == 2;
      const bool addressed = family == Family::kAddress ||
                             family == Family::kWriteEn ||
                             family == Family::kReadEn;
      const auto parent = netlist.scopes.at(node.scope).parent;
      const auto reference_scope =
          role == "Source" && IsInputPort(node.kind) &&
                  parent != std::numeric_limits<size_t>::max()
              ? parent
              : node.scope;
      const auto check_source_scope = [&](const BitReference& raw,
                                          const BitReference& bit) {
        if (role == "DifferentialInvOf" && node.kind == "ClockPort") {
          if (bit.node == std::numeric_limits<size_t>::max() ||
              netlist.nodes.at(bit.node).kind != "ClockPort") {
            throw std::runtime_error(node.origin + ": " + node.name + ": " +
                                     role + ": requires a positive ClockPort");
          }
        }
        if (raw.node != std::numeric_limits<size_t>::max()) {
          const auto& reference = netlist.nodes.at(raw.node);
          if (reference.model == ModelKind::kAlias &&
              reference.scope != reference_scope) {
            throw std::runtime_error(
                node.origin + ": " + node.name + ": " + role +
                ": hierarchical source must name an output port: " +
                reference.name);
          }
        }
        if (bit.node == std::numeric_limits<size_t>::max()) return;
        const auto& driver = netlist.nodes.at(bit.node);
        if (role == "DifferentialInvOf" && node.kind == "ToClockPort" &&
            driver.kind == "ToClockPort" && driver.scope == reference_scope) {
          return;
        }
        if (driver.kind == "ToIRSelectPort" &&
            driver.scope == reference_scope) {
          return;
        }
        const bool output = driver.kind == "DataOutPort" ||
                            driver.kind == "ScanOutPort" ||
                            driver.kind.compare(0, 2, "To") == 0;
        if (output ? netlist.scopes.at(driver.scope).parent != reference_scope
                   : driver.scope != reference_scope) {
          throw std::runtime_error(
              node.origin + ": " + node.name + ": " + role +
              (output ? ": output must be referenced from an instance in the "
                        "reference scope: "
                      : ": source must be local to the reference scope: ") +
              driver.name);
        }
      };
      for (const auto& bit : bits) {
        const auto resolved = terminal(bit);
        const auto kind = classify(resolved);
        // Tessent permits named secondary sources on negative clock inputs.
        const bool secondary_clock_source =
            family == Family::kClock && role == "Source" &&
            node.kind == "ClockPort" &&
            node.inputs.count("DifferentialInvOf") &&
            resolved.node < netlist.nodes.size() &&
            (netlist.nodes[resolved.node].kind == "ClockPort" ||
             netlist.nodes[resolved.node].kind == "ToClockPort") &&
            netlist.nodes[resolved.node].properties.count("DifferentialInvOf");
        if (family == Family::kReset) {
          const auto polarity = ResetPolarity(netlist, terminal, resolved);
          if (polarity && *polarity != node.active_low) {
            throw std::runtime_error(node.origin + ": " + node.name +
                                     ": inconsistent reset source polarity");
          }
        }
        const bool typed_control =
            (family == Family::kTck && kind == Kind::kTck) ||
            (family == Family::kCapture && kind == Kind::kCapture) ||
            (family == Family::kShift && kind == Kind::kShift) ||
            (family == Family::kUpdate && kind == Kind::kUpdate) ||
            (family == Family::kTms && kind == Kind::kTms) ||
            (family == Family::kTrst && kind == Kind::kTrst);
        if (typed_control && resolved.inverted) {
          throw std::runtime_error(
              node.origin + ": " + node.name + ": " + role +
              " requires a non-inverted typed control source");
        }
        if (kind == Kind::kIntermediateScanRegister &&
            (family == Family::kScanInput ||
             (node.kind == "ScanOutPort" && role == "Source"))) {
          throw std::runtime_error(
              node.origin + ": " + node.name + ": " + role +
              " must use the rightmost bit of a ScanRegister");
        }
        if (family == Family::kDataMux && control_use(kind)) {
          check_source_scope(bit, resolved);
          mux_control_sources |= control_use(kind);
          continue;
        }
        if ((family == Family::kReset || family == Family::kTms ||
             family == Family::kTrst) &&
            (kind == Kind::kData || kind == Kind::kDataPort ||
             kind == Kind::kScanRegister ||
             kind == Kind::kIntermediateScanRegister)) {
          const auto source = resolved;
          const auto& driver = netlist.nodes.at(source.node);
          if (driver.kind != "DataMux") {
            throw std::runtime_error(node.origin + ": " + node.name + ": " +
                                     role + " data source " + driver.name +
                                     " requires a direct DataMux data driver");
          }
          intercepted_control_mux = true;
        }
        if ((family != Family::kScanInput &&
             ((!addressed && kind == Kind::kData) || kind == Kind::kDataPort ||
              kind == Kind::kScanRegister ||
              kind == Kind::kIntermediateScanRegister)) ||
            (!addressed && kind == Kind::kConstant) ||
            kind == Kind::kUnconnected ||
            (family == Family::kClock &&
             (kind == Kind::kClock || kind == Kind::kTck ||
              secondary_clock_source)) ||
            (family == Family::kTck && kind == Kind::kTck) ||
            (family == Family::kReset &&
             (kind == Kind::kReset || kind == Kind::kTrst ||
              kind == Kind::kGlobalReset)) ||
            (family == Family::kCapture && kind == Kind::kCapture) ||
            (family == Family::kShift && kind == Kind::kShift) ||
            (family == Family::kUpdate && kind == Kind::kUpdate) ||
            (family == Family::kTms && kind == Kind::kTms) ||
            (family == Family::kTrst &&
             (kind == Kind::kTrst || kind == Kind::kAsyncReset)) ||
            (family == Family::kAddress && kind == Kind::kAddress) ||
            (family == Family::kWriteEn && kind == Kind::kWriteEn) ||
            (family == Family::kReadEn && kind == Kind::kReadEn) ||
            ((family == Family::kScan || family == Family::kScanInput) &&
             (kind == Kind::kScan || kind == Kind::kScanRegister)) ||
            (ir_select && kind == Kind::kIrSelect)) {
          check_source_scope(bit, resolved);
          continue;
        }
        throw std::runtime_error(
            node.origin + ": " + node.name + ": " + role +
            (addressed ? " requires " + node.kind +
                             ", DataInPort, DataOutPort or ScanRegister sources"
             : family == Family::kDataMux
                 ? " requires a data signal or supported control source"
             : family == Family::kClock ? " requires a clock or data signal"
             : family == Family::kTck   ? " requires a TCK or data signal"
             : family == Family::kReset ? " requires a reset or data signal"
             : family == Family::kCapture
                 ? " requires a capture-enable or data signal"
             : family == Family::kShift
                 ? " requires a shift-enable or data signal"
             : family == Family::kUpdate
                 ? " requires an update-enable or data signal"
             : family == Family::kTms       ? " requires a TMS or data signal"
             : family == Family::kTrst      ? " requires a TRST or data signal"
             : family == Family::kScan      ? " requires a scan or data signal"
             : family == Family::kScanInput ? " requires a scan signal"
                                            : " requires a data signal"));
      }
    };
    for (const auto& [role, bits] : node.inputs) {
      if (role == "ScanInSource") check(role, bits, Family::kScanInput);
      if (role == "DifferentialInvOf" ||
          (role == "Source" &&
           (node.kind == "ClockPort" || node.kind == "ToClockPort" ||
            node.kind == "ToTCKPort"))) {
        check(role, bits, Family::kClock);
      }
      if (role == "Source" && node.kind == "TCKPort") {
        check(role, bits, Family::kTck);
      }
      if (role == "Source" &&
          (node.kind == "ScanInPort" || node.kind == "ScanOutPort")) {
        check(role, bits, Family::kScan);
      }
      if (role == "Source" &&
          (node.kind == "ResetPort" || node.kind == "ToResetPort")) {
        check(role, bits, Family::kReset);
      }
      if (role == "Source") {
        if (node.kind == "AddressPort") {
          check(role, bits, Family::kAddress);
        } else if (node.kind == "WriteEnPort") {
          check(role, bits, Family::kWriteEn);
        } else if (node.kind == "ReadEnPort") {
          check(role, bits, Family::kReadEn);
        }
        if (node.kind == "CaptureEnPort" || node.kind == "ToCaptureEnPort") {
          check(role, bits, Family::kCapture);
        } else if (node.kind == "ShiftEnPort" || node.kind == "ToShiftEnPort") {
          check(role, bits, Family::kShift);
        } else if (node.kind == "UpdateEnPort" ||
                   node.kind == "ToUpdateEnPort") {
          check(role, bits, Family::kUpdate);
        }
        if (node.kind == "TMSPort" || node.kind == "ToTMSPort") {
          check(role, bits, Family::kTms);
        } else if (node.kind == "TRSTPort" || node.kind == "ToTRSTPort") {
          check(role, bits, Family::kTrst);
        }
      }
      if (role == "CaptureSource" || role == "WriteDataSource" ||
          role == "ReadDataSource" || role == "WriteEnSource" ||
          role == "Enable" || role == "SelectedBy" ||
          (role == "Source" &&
           (node.kind == "DataOutPort" || node.kind == "ToSelectPort" ||
            node.kind == "DataInPort" || node.kind == "SelectPort"))) {
        check(role, bits);
      }
    }
    if (node.kind == "ClockMux") {
      for (const auto& choice : node.choices) {
        check("clock choice", choice.source, Family::kClock);
      }
    }
    if (node.kind == "ScanMux") {
      for (const auto& choice : node.choices) {
        check("scan choice", choice.source, Family::kScan);
      }
    }
    if (node.kind == "DataMux") {
      for (const auto& choice : node.choices) {
        check("data choice", choice.source, Family::kDataMux);
      }
    }
    for (const auto& instruction : node.logic) {
      check("LogicSignal operand", instruction.signals);
    }
    if (node.kind == "OneHotDataGroup" || node.kind == "OneHotScanGroup") {
      for (const auto& source : node.one_hot_sources) {
        if (source.kind == OneHotSourceKind::kPort) {
          check("Port", source.bits,
                node.kind == "OneHotScanGroup" ? Family::kScan : Family::kData);
        }
      }
    }
  }
  if (!mux_control_sources && !intercepted_control_mux) return;
  auto contexts = FindMuxControlUses(netlist);
  for (size_t id = 0; id < netlist.nodes.size(); ++id) {
    const auto& node = netlist.nodes[id];
    if (node.kind != "DataMux") continue;
    for (const auto& choice : node.choices) {
      for (size_t i = 0; i < choice.source.size(); ++i) {
        const auto required = control_use(classify(choice.source[i]));
        const auto use = contexts.uses.at(contexts.offsets[id] + i);
        if (required && (use & kObservedUse) && !(use & required)) {
          const auto bit = node.left + (node.left <= node.right ? 1 : -1) *
                                           static_cast<int64_t>(i);
          throw std::runtime_error(
              node.origin + ": " + node.name + ": DataMux input for bit " +
              std::to_string(bit) + " requires matching control use");
        }
      }
    }
  }
  constexpr unsigned char kGateFamilies = kCaptureUse | kUpdateUse;
  constexpr unsigned char kShapeFamilies = kResetUse | kTmsUse | kTrstUse;
  if (!(mux_control_sources & kGateFamilies) && !intercepted_control_mux) {
    return;
  }
  constexpr uint16_t kUnknownOrigin = 32768;
  constexpr uint16_t kLowResetOrigin = 32, kHighResetOrigin = 64;
  constexpr uint16_t kLowTrstOrigin = 128, kHighTrstOrigin = 256;
  constexpr uint16_t kLowCaptureOrigin = 512, kHighCaptureOrigin = 1024;
  constexpr uint16_t kLowUpdateOrigin = 2048, kHighUpdateOrigin = 4096;
  constexpr uint16_t kLowTmsOrigin = 8192, kHighTmsOrigin = 16384;
  const auto control_polarities = [](uint16_t origin, bool inverted) {
    constexpr uint16_t kLow = kLowResetOrigin | kLowTrstOrigin |
                              kLowCaptureOrigin | kLowUpdateOrigin |
                              kLowTmsOrigin;
    constexpr uint16_t kHigh = kHighResetOrigin | kHighTrstOrigin |
                               kHighCaptureOrigin | kHighUpdateOrigin |
                               kHighTmsOrigin;
    return static_cast<uint16_t>(!inverted ? origin
                                           : (origin & ~(kLow | kHigh)) |
                                                 ((origin & kLow) << 1) |
                                                 ((origin & kHigh) >> 1));
  };
  std::vector<uint16_t> origins(contexts.uses.size(), kUnknownOrigin);
  const auto bit_id = [&](const BitReference& bit) {
    const auto& node = netlist.nodes.at(bit.node);
    const size_t position =
        node.left >= node.right ? node.left - bit.index : bit.index - node.left;
    return contexts.offsets[bit.node] + position;
  };
  struct GateVisit {
    size_t node, position, next;
    uint16_t families;
  };
  std::vector<GateVisit> pending;
  for (size_t id = 0; id < netlist.nodes.size(); ++id) {
    const auto& node = netlist.nodes[id];
    if (node.kind != "DataMux") continue;
    for (size_t position = 0; position < node.Width(); ++position) {
      if (origins[contexts.offsets[id] + position] != kUnknownOrigin) continue;
      pending.push_back({id, position, 0, 0});
      while (!pending.empty()) {
        auto& visit = pending.back();
        const auto& current = netlist.nodes[visit.node];
        if (visit.next == current.choices.size()) {
          origins[contexts.offsets[visit.node] + visit.position] =
              visit.families;
          pending.pop_back();
          continue;
        }
        const auto source =
            terminal(current.choices[visit.next].source[visit.position]);
        if (source.node < netlist.nodes.size() &&
            netlist.nodes[source.node].kind == "DataMux") {
          const auto origin = origins[bit_id(source)];
          if (origin == kUnknownOrigin) {
            pending.push_back({source.node,
                               bit_id(source) - contexts.offsets[source.node],
                               0, 0});
            continue;
          }
          visit.families |= control_polarities(origin, source.inverted);
        } else {
          const auto family = control_use(classify(source));
          visit.families |= family;
          if (family & (kResetUse | kTrstUse)) {
            const bool low =
                netlist.nodes[source.node].active_low != source.inverted;
            visit.families |= low ? kLowResetOrigin : kHighResetOrigin;
            if (family & kTrstUse) {
              visit.families |= low ? kLowTrstOrigin : kHighTrstOrigin;
            }
          }
          if (family & (kGateFamilies | kTmsUse)) {
            const auto low = family == kCaptureUse  ? kLowCaptureOrigin
                             : family == kUpdateUse ? kLowUpdateOrigin
                                                    : kLowTmsOrigin;
            visit.families |= source.inverted ? low : low << 1;
          }
        }
        ++visit.next;
      }
    }
  }
  for (size_t id = 0; id < netlist.nodes.size(); ++id) {
    const auto& node = netlist.nodes[id];
    if (node.kind != "DataMux") continue;
    for (size_t position = 0; position < node.Width(); ++position) {
      const auto origin =
          origins[contexts.offsets[id] + position] & kGateFamilies;
      if (!origin) continue;
      const auto families =
          origin |
          (contexts.uses[contexts.offsets[id] + position] & kGateFamilies);
      for (const unsigned char family : {kCaptureUse, kUpdateUse}) {
        if (!(families & family)) continue;
        std::optional<BitReference> control;
        for (const auto& choice : node.choices) {
          const auto source = terminal(choice.source[position]);
          if (source.node == std::numeric_limits<size_t>::max() &&
              source.virtual_signal == VirtualSignal::kNone &&
              source.constant == (source.inverted ? '1' : '0')) {
            continue;
          }
          const bool corresponding =
              source.node < netlist.nodes.size() &&
              ((control_use(classify(source)) & family) ||
               (netlist.nodes[source.node].kind == "DataMux" &&
                (origins[bit_id(source)] & family)));
          if (!corresponding ||
              (control && (control->node != source.node ||
                           control->index != source.index ||
                           control->inverted != source.inverted))) {
            const auto index = node.left + (node.left <= node.right ? 1 : -1) *
                                               static_cast<int64_t>(position);
            throw std::runtime_error(
                node.origin + ": " + node.name + ": " +
                (family == kCaptureUse ? "CaptureEn" : "UpdateEn") +
                " DataMux bit " + std::to_string(index) +
                " requires zero and one corresponding control input");
          }
          control = source;
        }
      }
    }
  }
  // Matching-use masks have finished; reuse their bounded storage for shape
  // work.
  if (intercepted_control_mux) {
    std::fill(contexts.uses.begin(), contexts.uses.end(), 0);
  }
  constexpr unsigned char kQueued = 128;
  std::vector<uint32_t> shape_pending;
  const auto mark_shape = [&](const BitReference& source,
                              unsigned char family) {
    if (source.node >= netlist.nodes.size() ||
        netlist.nodes[source.node].kind != "DataMux") {
      return;
    }
    const auto id = bit_id(source);
    auto& use = contexts.uses[id];
    if ((use & family) == family) return;
    const bool queued = use & kQueued;
    use |= family;
    if (!queued) {
      use |= kQueued;
      shape_pending.push_back(static_cast<uint32_t>(id));
    }
  };
  std::optional<std::string> control_path_error;
  for (const auto& node : netlist.nodes) {
    const unsigned char family =
        node.kind == "ResetPort" || node.kind == "ToResetPort" ? kResetUse
        : node.kind == "TMSPort" || node.kind == "ToTMSPort"   ? kTmsUse
        : node.kind == "TRSTPort" || node.kind == "ToTRSTPort" ? kTrstUse
        : node.kind == "CaptureEnPort" || node.kind == "ToCaptureEnPort"
            ? kCaptureUse
        : node.kind == "UpdateEnPort" || node.kind == "ToUpdateEnPort"
            ? kUpdateUse
            : 0;
    const auto source = node.inputs.find("Source");
    if (!family || source == node.inputs.end()) continue;
    const auto required = family == kCaptureUse  ? kHighCaptureOrigin
                          : family == kUpdateUse ? kHighUpdateOrigin
                          : family == kTmsUse    ? kHighTmsOrigin
                          : family == kTrstUse   ? kLowTrstOrigin
                          : node.active_low      ? kLowResetOrigin
                                                 : kHighResetOrigin;
    const auto corresponding =
        family == kResetUse ? kResetUse | kTrstUse : family;
    for (const auto& bit : source->second) {
      const auto driver = terminal(bit);
      if (!control_path_error && driver.node < netlist.nodes.size() &&
          netlist.nodes[driver.node].kind == "DataMux" &&
          (origins[bit_id(driver)] & corresponding) &&
          !(control_polarities(origins[bit_id(driver)], driver.inverted) &
            required)) {
        const auto label = family == kCaptureUse  ? "CaptureEn"
                           : family == kUpdateUse ? "UpdateEn"
                           : family == kTmsUse    ? "TMS"
                           : family == kTrstUse   ? "TRST"
                                                  : "reset";
        control_path_error = node.origin + ": " + node.name +
                             ": incompatible " + label + " mux path polarity";
      }
      if (family & kShapeFamilies) mark_shape(driver, family);
    }
  }
  const auto source_origins = [&](const BitReference& source) -> uint16_t {
    return source.node < netlist.nodes.size() &&
                   netlist.nodes[source.node].kind == "DataMux"
               ? origins[bit_id(source)]
               : control_use(classify(source));
  };
  const auto same_source = [](const BitReference& a, const BitReference& b) {
    return a.node == b.node && a.index == b.index && a.inverted == b.inverted &&
           a.virtual_signal == b.virtual_signal &&
           (a.node != std::numeric_limits<size_t>::max() ||
            a.virtual_signal != VirtualSignal::kNone ||
            a.constant == b.constant);
  };
  while (!shape_pending.empty()) {
    const size_t bit = shape_pending.back();
    shape_pending.pop_back();
    const unsigned char families = contexts.uses[bit] & kShapeFamilies;
    contexts.uses[bit] = families;
    const size_t id =
        static_cast<size_t>(std::upper_bound(contexts.offsets.begin(),
                                             contexts.offsets.end(), bit) -
                            contexts.offsets.begin() - 1);
    const auto& node = netlist.nodes[id];
    const size_t position = bit - contexts.offsets[id];
    const auto fail = [&] {
      const auto index = node.left + (node.left <= node.right ? 1 : -1) *
                                         static_cast<int64_t>(position);
      throw std::runtime_error(
          node.origin + ": " + node.name + ": control DataMux bit " +
          std::to_string(index) +
          " requires a corresponding control path and at most two data inputs");
    };
    std::optional<BitReference> first, second;
    unsigned char incoming = 0;
    for (const auto& choice : node.choices) {
      auto source = terminal(choice.source[position]);
      const auto origin = source_origins(source);
      incoming |= origin & kShapeFamilies;
      for (const unsigned char family : {kResetUse, kTmsUse, kTrstUse}) {
        const auto corresponding =
            family == kResetUse ? kResetUse | kTrstUse : family;
        if ((families & family) && (origin & corresponding)) {
          mark_shape(source, family);
        }
      }
      if (source.node == std::numeric_limits<size_t>::max() &&
          source.virtual_signal == VirtualSignal::kNone) {
        if (source.inverted &&
            (source.constant == '0' || source.constant == '1')) {
          source.constant = source.constant == '0' ? '1' : '0';
        }
        source.index = 0;
        source.inverted = false;
      }
      if (!first) {
        first = source;
      } else if (!same_source(*first, source)) {
        if (!second) {
          second = source;
        } else if (!same_source(*second, source)) {
          fail();
        }
      }
    }
    for (const unsigned char family : {kResetUse, kTmsUse, kTrstUse}) {
      const auto corresponding =
          family == kResetUse ? kResetUse | kTrstUse : family;
      if ((families & family) && !(incoming & corresponding)) fail();
    }
  }
  // Retain structural diagnostics before reporting incompatible path polarity.
  if (control_path_error) throw std::runtime_error(*control_path_error);
}

class Elaborator {
 public:
  Elaborator(Design& design,
             const std::map<std::string, ModuleDefinition>& access_link_models,
             const std::map<std::string, std::string>& top_ports)
      : design_(design),
        access_link_models_(access_link_models),
        top_ports_(top_ports) {}

  Netlist Run(const std::string& top) {
    const auto& module = design_.SelectTop(top);
    CheckModels(module);
    netlist_.top_module = module.name();
    for (const auto& entry : design_.modules()) {
      RecordParameters(entry.second);
    }
    AddScope(module, "", std::numeric_limits<size_t>::max(), {}, 0);
    for (const auto& [name, model] : access_link_models_) {
      RecordParameters(model);
      AddScope(model, name, 0, {}, 1);
    }
    IndexPortGroups(&netlist_);
    AddTopPorts();
    Connect();
    const bool restored = RestoreFlatOrigins(&netlist_);
    ValidatePortGroupMembers(netlist_);
    AliasResolver terminal(netlist_);
    if (!restored) {
      InferResetPolarities(terminal);
      InferResets();
    }
    CheckAliasConsumers();
    ConnectAddressedRegisters();
    ValidateInterfaces();
    CheckCombinationalConnections(netlist_);
    CheckSignalSources(netlist_, terminal);
    return std::move(netlist_);
  }

 private:
  void AddTopPorts() {
    static const std::set<std::string> kKinds = {
        "ScanInPort", "ScanOutPort", "TMSPort", "TCKPort", "TRSTPort"};
    for (const auto& [name, kind] : top_ports_) {
      const auto& root = netlist_.scopes[0];
      const auto origin = root.definition.Location(
          root.definition.declaration().name_range.begin);
      if (!kKinds.count(kind) || scope_ids_.count(name) ||
          netlist_.module_parameters.at(netlist_.top_module).count(name)) {
        throw std::runtime_error(origin + ": invalid generated TAP port " +
                                 name);
      }
      if (netlist_.port_groups.count(name)) {
        throw std::runtime_error(origin + ": TAP port " + name +
                                 " must be a scalar " + kind);
      }
      const auto found = netlist_.node_ids.find(name);
      if (found != netlist_.node_ids.end()) {
        const auto& port = netlist_.nodes[found->second];
        if (port.scope != 0 || port.kind != kind || port.Width() != 1) {
          throw std::runtime_error(port.origin + ": TAP port " + name +
                                   " must be a scalar " + kind);
        }
        continue;
      }
      const auto id = netlist_.nodes.size();
      if (id >= kMaxObjects) {
        throw std::runtime_error("ICL object expansion limit exceeded");
      }
      NetlistNode port;
      port.name = name;
      port.kind = kind;
      port.origin = origin + " (generated TAP port " + name + ")";
      netlist_.nodes.push_back(std::move(port));
      netlist_.node_ids.emplace(name, id);
      netlist_.scopes[0].members.emplace(name, id);
      kinds_[{0, kind}].push_back(id);
    }
  }
  void RecordParameters(const ModuleDefinition& module) {
    auto& parameters = netlist_.module_parameters[module.name()];
    for (const auto* node : module.ast->body()) {
      const auto* statement = dynamic_cast<const icl::AstStatement*>(node);
      if (statement &&
          (statement->named_kind() == icl::NamedKind::kParameter ||
           statement->named_kind() == icl::NamedKind::kLocalParameter)) {
        parameters.insert(statement->name());
      }
    }
  }
  void CheckModels(const ModuleDefinition& top) {
    if (access_link_models_.empty()) return;
    std::set<std::string> links;
    std::map<std::string, bool> names;
    for (size_t index : Describe(top).declarations) {
      const auto& declaration = top.document->declarations()[index];
      if (declaration.parent != static_cast<int>(top.declaration_index) &&
          (declaration.parent < 0 ||
           top.document->declarations()[declaration.parent].kind !=
               "OneHotDataGroup")) {
        continue;
      }
      const auto& kind = declaration.kind;
      const bool port =
          kind.size() >= 4 && kind.compare(kind.size() - 4, 4, "Port") == 0;
      const auto inserted = names.emplace(declaration.name, port);
      if (!inserted.second && !(inserted.first->second && port)) {
        throw std::runtime_error(top.Location(declaration.name_range.begin) +
                                 ": duplicate top-level declaration " +
                                 declaration.name);
      }
      if (declaration.kind == "AccessLink" &&
          dynamic_cast<const icl::AstBlock*>(
              Describe(top).nodes.at(declaration.range.begin))) {
        links.insert(declaration.name);
      }
    }
    std::map<std::string, const ModuleDefinition*> definitions;
    for (const auto& [name, module] : design_.modules()) {
      definitions.emplace(name, &module);
    }
    for (const auto& [name, model] : access_link_models_) {
      const auto context = top.Location(top.declaration().name_range.begin) +
                           ": AccessLink " + name + ": ";
      if (!links.count(name)) {
        throw std::runtime_error(context + "unknown top-level AccessLink " +
                                 name);
      }
      if (!model.document || !model.document->ok() || !model.ast ||
          model.declaration_index >= model.document->declarations().size() ||
          model.declaration().kind != "Module") {
        throw std::runtime_error(context + "invalid behavioral module");
      }
      const auto previous = definitions.emplace(model.name(), &model);
      if (!previous.second && previous.first->second->ast != model.ast) {
        throw std::runtime_error(
            context + "conflicting behavioral module identity " + model.name());
      }
      if (!model.instances.empty()) {
        throw std::runtime_error(context +
                                 "behavioral model must be a leaf module");
      }
      for (size_t index : Describe(model).declarations) {
        if (model.document->declarations()[index].kind == "AccessLink") {
          throw std::runtime_error(
              context + "behavioral model cannot contain AccessLink");
        }
      }
    }
  }
  using Parameters = std::map<std::string, Constant>;
  struct Template {
    std::map<size_t, const icl::AstNode*> nodes;
    std::map<std::string, const icl::AstStatement*> parameters;
    std::vector<size_t> declarations;
  };

  const Template& Describe(const ModuleDefinition& module) {
    const auto cached = templates_.find(module.name());
    if (cached != templates_.end()) return cached->second;
    Template result;
    std::vector<const icl::AstNode*> pending{module.ast};
    while (!pending.empty()) {
      const auto* node = pending.back();
      pending.pop_back();
      result.nodes.emplace(module.document->Range(node->span()).begin, node);
      if (const auto* block = dynamic_cast<const icl::AstBlock*>(node)) {
        pending.insert(pending.end(), block->body().begin(),
                       block->body().end());
      }
    }
    for (const auto* node : module.ast->body()) {
      const auto* statement = dynamic_cast<const icl::AstStatement*>(node);
      if (statement &&
          (statement->named_kind() == icl::NamedKind::kParameter ||
           statement->named_kind() == icl::NamedKind::kLocalParameter)) {
        result.parameters.emplace(statement->name(), statement);
      }
    }
    const auto members = module.document->members().find(
        static_cast<int>(module.declaration_index));
    if (members != module.document->members().end()) {
      for (const auto& member : members->second) {
        result.declarations.insert(result.declarations.end(),
                                   member.second.begin(), member.second.end());
      }
    }
    std::sort(result.declarations.begin(), result.declarations.end());
    return templates_.emplace(module.name(), std::move(result)).first->second;
  }

  void ResolveParameters(size_t scope_id, const Template& description,
                         const Parameters& overrides) {
    auto& scope = netlist_.scopes[scope_id];
    const auto& module = scope.definition;
    std::map<std::string, int> types;
    std::set<std::string> type_stack;
    std::function<int(const std::string&)> declared_type;
    declared_type = [&](const std::string& name) -> int {
      const auto found = types.find(name);
      if (found != types.end()) return found->second;
      const auto declared = description.parameters.find(name);
      if (declared == description.parameters.end()) {
        throw std::runtime_error("unknown parameter " + name);
      }
      if (!type_stack.insert(name).second) {
        throw std::runtime_error("parameter type cycle at " + name);
      }
      if (type_stack.size() > kMaxDepth) {
        throw std::runtime_error("parameter type depth limit exceeded");
      }
      const auto& value = *declared->second->value();
      auto atoms = value.atoms;
      if (atoms.empty()) {
        atoms.push_back({value.kind, value.unsized, value.reference});
      }
      int result = -1;
      for (const auto& atom : atoms) {
        const int type = !atom.reference.empty() ? declared_type(atom.reference)
                         : atom.kind == icl::ValueKind::kString ? 0
                         : atom.unsized                         ? 1
                                                                : 2;
        if (result >= 0 && (result != type || type == 1)) {
          throw std::runtime_error(
              "incompatible parameter concatenation types");
        }
        result = type;
      }
      type_stack.erase(name);
      types.emplace(name, result);
      return result;
    };
    for (const auto& override : overrides) {
      const auto declared = description.parameters.find(override.first);
      if (declared == description.parameters.end()) {
        throw std::runtime_error("unknown parameter override " +
                                 override.first);
      }
      if (declared->second->named_kind() == icl::NamedKind::kLocalParameter) {
        throw std::runtime_error("cannot override LocalParameter " +
                                 override.first);
      }
      const int type = override.second.is_string ? 0
                       : override.second.sized   ? 2
                                                 : 1;
      if (declared_type(override.first) != type) {
        throw std::runtime_error(
            "parameter override type does not match declaration: " +
            override.first);
      }
    }
    scope.parameters = overrides;
    std::set<std::string> active;
    std::function<Constant(const std::string&)> resolve;
    resolve = [&](const std::string& name) -> Constant {
      const auto found = scope.parameters.find(name);
      if (found != scope.parameters.end()) return found->second;
      const auto declared = description.parameters.find(name);
      if (declared == description.parameters.end()) {
        throw std::runtime_error("unknown parameter " + name);
      }
      if (!active.insert(name).second) {
        throw std::runtime_error("parameter cycle at " + name);
      }
      if (active.size() > kMaxDepth) {
        throw std::runtime_error("parameter dependency depth limit exceeded");
      }
      auto expression = Tokens(module, *declared->second, scope_id);
      expression.begin += 3;
      --expression.end;
      const auto value =
          EvaluateConstant(SourceTokens(netlist_, scope_id), expression.begin,
                           expression.end, resolve);
      active.erase(name);
      scope.parameters.emplace(name, value);
      return value;
    };
    for (const auto& parameter : description.parameters) {
      resolve(parameter.first);
    }
  }

  void Range(NetlistNode* node, const Expression& expression) {
    const auto& tokens = SourceTokens(netlist_, expression.scope);
    const size_t begin = expression.begin + 2;
    if (begin >= expression.end || tokens[begin].text != "[") return;
    size_t end = begin + 1, colon = expression.end;
    while (end < expression.end && tokens[end].text != "]") {
      if (tokens[end].text == ":") colon = end;
      ++end;
    }
    node->indexed = true;
    node->left =
        netlist_.Integer({expression.scope, begin + 1, std::min(end, colon)});
    node->right = colon < end
                      ? netlist_.Integer({expression.scope, colon + 1, end})
                      : node->left;
    if (node->Width() > kMaxWidth) {
      throw std::runtime_error("object width limit exceeded");
    }
  }

  Attributes ReadAttributes(const ModuleDefinition& module,
                            const icl::AstBlock* block, size_t scope) const {
    Attributes result;
    if (!block) return result;
    for (const auto* item : block->body()) {
      const auto* statement = dynamic_cast<const icl::AstStatement*>(item);
      if (!statement || statement->named_kind() != icl::NamedKind::kAttribute) {
        continue;
      }
      if (statement->name() == kFlatOriginAttribute) {
        if (block != module.ast || scope != 0) {
          throw std::runtime_error(
              "flat origin metadata requires a root module");
        }
        continue;
      }
      if (!statement->value()) {
        result.emplace(statement->name(), std::nullopt);
      } else {
        auto expression = Tokens(module, *item, scope);
        expression.begin += 3;
        --expression.end;
        result.emplace(statement->name(), netlist_.Value(expression));
      }
    }
    return result;
  }

  void AddScope(const ModuleDefinition& module, const std::string& path,
                size_t parent, const Parameters& overrides, size_t depth) {
    if (depth > kMaxDepth || netlist_.scopes.size() >= kMaxObjects) {
      throw std::runtime_error("ICL instance expansion limit exceeded");
    }
    const auto& description = Describe(module);
    const size_t scope_id = netlist_.scopes.size();
    scope_ids_.emplace(path, scope_id);
    netlist_.scopes.push_back({path, module, parent, {}, {}, {}, {}, {}, {}});
    if (parent != std::numeric_limits<size_t>::max()) {
      netlist_.scopes[parent].children.push_back(scope_id);
    }
    try {
      ResolveParameters(scope_id, description, overrides);
      netlist_.scopes[scope_id].attributes =
          ReadAttributes(module, module.ast, scope_id);
      for (size_t index : description.declarations) {
        const auto& declaration = module.document->declarations()[index];
        if (declaration.kind == "Instance" || declaration.kind == "Parameter" ||
            declaration.kind == "LocalParameter" ||
            declaration.kind == "EnumMember" || declaration.kind == "Chain") {
          continue;
        }
        if (scope_id == 0 && declaration.kind == "AccessLink" &&
            access_link_models_.count(declaration.name)) {
          continue;
        }
        const auto* ast = description.nodes.at(declaration.range.begin);
        const size_t node_id = netlist_.nodes.size();
        if (node_id >= kMaxObjects) {
          throw std::runtime_error("ICL object expansion limit exceeded");
        }
        NetlistNode node;
        node.name = Path(path, declaration.name);
        node.kind = declaration.kind;
        node.model = Model(node.kind);
        node.scope = scope_id;
        node.ast = ast;
        node.attributes = ReadAttributes(
            module, dynamic_cast<const icl::AstBlock*>(ast), scope_id);
        node.origin = module.Location(declaration.name_range.begin);
        try {
          const auto expression = Tokens(module, *ast, scope_id);
          Range(&node, expression);
          if (node.kind == "Alias") {
            auto source = expression;
            const auto& tokens = SourceTokens(netlist_, scope_id);
            while (source.begin < source.end &&
                   tokens[source.begin].text != "=") {
              ++source.begin;
            }
            ++source.begin;
            source.end = source.begin;
            while (source.end < expression.end &&
                   tokens[source.end].text != ";" &&
                   tokens[source.end].text != "{") {
              ++source.end;
            }
            node.properties.emplace("Source", source);
          }
          if (node.model == ModelKind::kMux) {
            auto selector = expression;
            const auto& tokens = SourceTokens(netlist_, scope_id);
            selector.begin += 2;
            if (tokens[selector.begin].text == "[") {
              while (tokens[selector.begin].text != "]") ++selector.begin;
              ++selector.begin;
            }
            if (tokens[selector.begin].text == "SelectedBy") ++selector.begin;
            selector.end = selector.begin;
            while (tokens[selector.end].text != "{") ++selector.end;
            node.properties.emplace("SelectedBy", selector);
          }
          if (const auto* block = dynamic_cast<const icl::AstBlock*>(ast);
              block && node.kind != "PortGroup") {
            for (const auto* item : block->body()) {
              if (dynamic_cast<const icl::AstComment*>(item)) continue;
              const auto* statement =
                  dynamic_cast<const icl::AstStatement*>(item);
              if (statement && statement->Logic()) continue;
              auto property = Tokens(module, *item, scope_id);
              const auto& tokens = SourceTokens(netlist_, scope_id);
              if (property.begin == property.end) continue;
              const auto key = tokens[property.begin++].text;
              if (node.model != ModelKind::kEnum &&
                  (key == "Attribute" || key == "Instance" ||
                   key == "DataRegister")) {
                continue;
              }
              if (tokens[property.end - 1].text == ";") --property.end;
              node.properties.emplace(key, property);
            }
          }
          const auto base = node.name;
          auto group = netlist_.port_groups.find(base);
          const auto previous = netlist_.node_ids.find(base);
          if (group != netlist_.port_groups.end() ||
              previous != netlist_.node_ids.end()) {
            if (node.model != ModelKind::kPort || !node.indexed) {
              throw std::runtime_error("duplicate object " + base);
            }
            if (group == netlist_.port_groups.end()) {
              auto& old = netlist_.nodes[previous->second];
              if (old.model != ModelKind::kPort || !old.indexed) {
                throw std::runtime_error("duplicate object " + base);
              }
              const auto old_id = previous->second;
              netlist_.node_ids.erase(previous);
              old.name = PortFunctionName(old);
              netlist_.node_ids.emplace(old.name, old_id);
              auto& members = netlist_.scopes[scope_id].members;
              members.erase(declaration.name);
              members.emplace(
                  old.name.substr(path.empty() ? 0 : path.size() + 1), old_id);
              group =
                  netlist_.port_groups.emplace(base, PortGroup{{old_id}}).first;
            }
            group->second.members.push_back(node_id);
            node.name = PortFunctionName(node);
          }
          if (!netlist_.node_ids.emplace(node.name, node_id).second) {
            throw std::runtime_error("overlapping port functions " + base);
          }
          netlist_.scopes[scope_id].members.emplace(
              node.name.substr(path.empty() ? 0 : path.size() + 1), node_id);
          kinds_[{scope_id, declaration.kind}].push_back(node_id);
          netlist_.nodes.push_back(std::move(node));
        } catch (const std::exception& error) {
          throw std::runtime_error(node.origin + ": " + error.what());
        }
      }
      for (size_t index : module.instances) {
        const auto& declaration = module.document->declarations()[index];
        const auto* ast = description.nodes.at(declaration.range.begin);
        Parameters values;
        if (const auto* block = dynamic_cast<const icl::AstBlock*>(ast)) {
          for (const auto* item : block->body()) {
            const auto* statement =
                dynamic_cast<const icl::AstStatement*>(item);
            if (!statement ||
                statement->named_kind() != icl::NamedKind::kParameter) {
              continue;
            }
            auto expression = Tokens(module, *statement, scope_id);
            expression.begin += 3;
            --expression.end;
            values.emplace(statement->name(), netlist_.Value(expression));
          }
        }
        const size_t child_scope = netlist_.scopes.size();
        AddScope(design_.ResolveInstance(module, index),
                 Path(path, declaration.name), scope_id, values, depth + 1);
        if (const auto* block = dynamic_cast<const icl::AstBlock*>(ast)) {
          netlist_.scopes[child_scope].instance_attributes =
              ReadAttributes(module, block, scope_id);
          for (const auto* item : block->body()) {
            if (dynamic_cast<const icl::AstComment*>(item)) continue;
            auto expression = Tokens(module, *item, scope_id);
            const auto& tokens = SourceTokens(netlist_, scope_id);
            if (tokens[expression.begin].text == "AddressValue") {
              ++expression.begin;
              --expression.end;
              netlist_.scopes[child_scope].address_value =
                  ReadAddressValue(expression);
              continue;
            }
            if (tokens[expression.begin].text != "InputPort") continue;
            const auto& port_name = tokens[expression.begin + 1].text;
            const size_t first = expression.begin;
            while (expression.begin < expression.end &&
                   tokens[expression.begin].text != "=") {
              ++expression.begin;
            }
            const size_t equals = expression.begin++;
            --expression.end;
            bindings_.push_back(
                {Path(netlist_.scopes[child_scope].path, port_name),
                 {scope_id, first + 2, equals},
                 expression});
          }
        }
      }
    } catch (const std::exception& error) {
      throw std::runtime_error(
          module.Location(module.declaration().name_range.begin) +
          ": instance " + (path.empty() ? "<top>" : path) + ": " +
          error.what());
    }
  }

  ScanChain ReadInterfacePart(const NetlistNode& node,
                              const icl::AstBlock& block) const {
    ScanChain result;
    const auto& module = netlist_.scopes[node.scope].definition;
    result.attributes = ReadAttributes(module, &block, node.scope);
    for (const auto* item : block.body()) {
      const auto* statement = dynamic_cast<const icl::AstStatement*>(item);
      if (!statement || statement->named_kind() == icl::NamedKind::kAttribute) {
        continue;
      }
      auto expression = Tokens(module, *item, node.scope);
      const auto& tokens = SourceTokens(netlist_, node.scope);
      const auto keyword = tokens[expression.begin++].text;
      --expression.end;
      if (keyword == "Port") {
        const auto bits = netlist_.Resolve(expression);
        result.ports.insert(result.ports.end(), bits.begin(), bits.end());
      } else if (keyword == "DefaultLoadValue") {
        result.default_load = netlist_.Value(expression);
      }
    }
    return result;
  }

  void ConnectPortGroup(NetlistNode* node) {
    try {
      if (!port_group_bits_remaining_) {
        throw std::runtime_error("member expansion bit limit exceeded");
      }
      const auto& module = netlist_.scopes[node->scope].definition;
      auto expression = Tokens(module, *node->ast, node->scope);
      expression.begin += 3;
      --expression.end;
      node->port_group = netlist_.Resolve(expression, 0, ReferenceUse::kAlias,
                                          port_group_bits_remaining_);
      port_group_bits_remaining_ -= node->port_group.size();
    } catch (const std::runtime_error& error) {
      throw std::runtime_error("PortGroup " + node->name + ": " + error.what());
    }
  }

  void ConnectInterface(NetlistNode* node) const {
    const auto& block = dynamic_cast<const icl::AstBlock&>(*node->ast);
    auto common = ReadInterfacePart(*node, block);
    ScanInterface interface;
    interface.ports = std::move(common.ports);
    interface.default_load = std::move(common.default_load);
    for (const auto* item : block.body()) {
      const auto* chain = dynamic_cast<const icl::AstBlock*>(item);
      if (!chain) continue;
      auto data = ReadInterfacePart(*node, *chain);
      const auto& module = netlist_.scopes[node->scope].definition;
      const auto expression = Tokens(module, *chain, node->scope);
      data.name =
          SourceTokens(netlist_, node->scope)[expression.begin + 1].text;
      interface.chains.push_back(std::move(data));
    }
    node->scan_interface = std::move(interface);
  }

  void ConnectOneHotPorts(NetlistNode* node) {
    const auto& block = dynamic_cast<const icl::AstBlock&>(*node->ast);
    const auto& module = netlist_.scopes[node->scope].definition;
    for (const auto* item : block.body()) {
      if (dynamic_cast<const icl::AstComment*>(item)) continue;
      auto expression = Tokens(module, *item, node->scope);
      const auto& keyword =
          SourceTokens(netlist_, node->scope)[expression.begin].text;
      OneHotSource source;
      const auto append = [&](size_t id) {
        const auto& member = netlist_.nodes[id];
        if (member.Width() > node->Width() - source.bits.size()) {
          throw std::runtime_error("one-hot source width exceeds group width");
        }
        if (member.Width() > one_hot_bits_remaining_) {
          throw std::runtime_error("one-hot source bit limit exceeded");
        }
        one_hot_bits_remaining_ -= member.Width();
        for (size_t i = 0; i < member.Width(); ++i) {
          source.bits.push_back(
              {id,
               member.left + (member.left <= member.right ? 1 : -1) *
                                 static_cast<int64_t>(i),
               'x', false});
        }
      };
      if (keyword == "Port") {
        ++expression.begin;
        --expression.end;
        source.bits = netlist_.Resolve(
            expression, node->kind == "OneHotScanGroup" ? node->Width() : 0,
            ReferenceUse::kConnection, one_hot_bits_remaining_);
        one_hot_bits_remaining_ -= source.bits.size();
      } else if (keyword == "Instance" || keyword == "DataRegister") {
        const auto name = Path(
            netlist_.scopes[node->scope].path,
            SourceTokens(netlist_, node->scope)[expression.begin + 1].text);
        if (keyword == "Instance") {
          source.kind = OneHotSourceKind::kInstance;
          source.object = scope_ids_.at(name);
          for (size_t id : kinds_[{source.object, "DataOutPort"}]) append(id);
        } else {
          source.kind = OneHotSourceKind::kRegister;
          source.object = netlist_.node_ids.at(name);
          if (!netlist_.nodes[source.object].properties.count("AddressValue")) {
            throw std::runtime_error(
                "addressed one-hot register requires AddressValue");
          }
          append(source.object);
        }
      } else {
        continue;
      }
      if (source.bits.size() > node->Width()) {
        throw std::runtime_error("one-hot source width exceeds group width");
      }
      node->one_hot_sources.push_back(std::move(source));
    }
    // IEEE 6.4.15 source order is immaterial. Width order lets graph traversal
    // stop at padded sources instead of doing width * contributor-count work.
    std::stable_sort(node->one_hot_sources.begin(), node->one_hot_sources.end(),
                     [](const auto& a, const auto& b) {
                       return a.bits.size() > b.bits.size();
                     });
  }

  void ValidateInterfaces() {
    std::map<std::pair<size_t, std::string>, std::vector<size_t>> ports;
    size_t tck_bits = 0;
    for (size_t id = 0; id < netlist_.nodes.size(); ++id) {
      const auto& node = netlist_.nodes[id];
      if (node.model == ModelKind::kPort) {
        ports[{node.scope, node.kind}].push_back(id);
      }
      if (node.kind == "ToTCKPort" && !node.inputs.count("Source")) {
        if (node.Width() > inferred_control_bits_remaining_ - tck_bits) {
          throw std::runtime_error(
              node.origin + ": inferred control connection bit limit exceeded");
        }
        tck_bits += node.Width();
      }
    }
    inferred_control_bits_remaining_ -= tck_bits;
    for (auto& [key, candidates] : ports) {
      if (key.second != "TCKPort") continue;
      const auto scalar = std::find_if(
          candidates.begin(), candidates.end(),
          [&](size_t id) { return netlist_.nodes[id].Width() == 1; });
      if (scalar != candidates.end()) {
        std::iter_swap(candidates.begin(), scalar);
      }
    }
    for (auto& node : netlist_.nodes) {
      if (node.kind == "ToTCKPort" && !node.inputs.count("Source")) {
        const auto& candidates = ports[{node.scope, "TCKPort"}];
        if (candidates.empty()) {
          throw std::runtime_error(
              node.origin + ": " + node.name +
              ": source-less ToTCKPort requires a local TCKPort");
        }
        const auto& source = netlist_.nodes[candidates.front()];
        if (source.Width() != 1) {
          throw std::runtime_error(
              node.origin + ": " + node.name +
              ": unsupported implicit TCK mapping; provide an explicit Source");
        }
        node.inputs["Source"].assign(node.Width(),
                                     {candidates.front(), source.left});
      }
      if (node.kind == "ToIRSelectPort") {
        if (ports[{node.scope, "TMSPort"}].size() != 1) {
          throw std::runtime_error(node.origin + ": " + node.name +
                                   ": ToIRSelectPort requires exactly one "
                                   "TMSPort declaration in its module");
        }
        if (ports[{node.scope, "TRSTPort"}].size() > 1) {
          throw std::runtime_error(node.origin + ": " + node.name +
                                   ": ToIRSelectPort requires at most one "
                                   "TRSTPort declaration in its module");
        }
      }
      if (!node.scan_interface) continue;
      try {
        auto& interface = *node.scan_interface;
        std::set<std::pair<size_t, int64_t>> seen;
        const auto count_ports = [&](const std::vector<BitReference>& refs) {
          std::map<std::string, size_t> counts;
          for (const auto& ref : refs) {
            const auto& port = netlist_.nodes.at(ref.node);
            if (port.model != ModelKind::kPort || port.scope != node.scope ||
                !seen.emplace(ref.node, ref.index).second) {
              throw std::runtime_error(
                  "invalid or repeated ScanInterface port");
            }
            ++counts[port.kind];
          }
          return counts;
        };
        auto common = count_ports(interface.ports);
        std::set<std::string> chains;
        for (const auto& chain : interface.chains) {
          auto counts = count_ports(chain.ports);
          if (!chains.insert(chain.name).second || counts.size() != 2 ||
              counts["ScanInPort"] != 1 || counts["ScanOutPort"] != 1) {
            throw std::runtime_error(
                "ScanInterface Chain needs one SI/SO pair");
          }
        }
        if (!interface.chains.empty() &&
            (common.count("ScanInPort") || common.count("ScanOutPort") ||
             interface.default_load)) {
          throw std::runtime_error(
              "ScanInterface mixes Chain and common scan data");
        }

        size_t matches = 0;
        std::vector<BitReference> resolved;
        for (const auto kind :
             {ScanInterfaceKind::kClient, ScanInterfaceKind::kHost,
              ScanInterfaceKind::kClientTap, ScanInterfaceKind::kHostTap}) {
          const bool host = kind == ScanInterfaceKind::kHost ||
                            kind == ScanInterfaceKind::kHostTap;
          const bool tap = kind == ScanInterfaceKind::kClientTap ||
                           kind == ScanInterfaceKind::kHostTap;
          const std::string prefix = host ? "To" : "";
          std::set<std::string> allowed = {"ScanInPort", "ScanOutPort",
                                           prefix + "TCKPort"};
          if (tap) {
            allowed.insert(prefix + "TMSPort");
            allowed.insert(prefix + "TRSTPort");
          } else {
            for (const std::string role :
                 {"Select", "ShiftEn", "CaptureEn", "UpdateEn", "Reset"}) {
              allowed.insert(prefix + role + "Port");
            }
          }
          auto counts = common;
          auto refs = interface.ports;
          if (!tap) {
            for (const std::string role :
                 {"ShiftEn", "CaptureEn", "UpdateEn"}) {
              const auto name = prefix + role + "Port";
              const auto& candidates = ports[{node.scope, name}];
              if (!counts.count(name) && candidates.size() == 1) {
                const auto id = candidates.front();
                const auto& port = netlist_.nodes[id];
                counts[name] = port.Width();
                for (int64_t bit = port.left;;
                     bit += port.left <= port.right ? 1 : -1) {
                  refs.push_back({id, bit});
                  if (bit == port.right) break;
                }
              }
            }
          }
          bool valid = true;
          for (const auto& entry : counts) {
            if (!allowed.count(entry.first) ||
                (entry.first != prefix + "SelectPort" && entry.second > 1)) {
              valid = false;
            }
          }
          if (tap ? counts[prefix + "TMSPort"] != 1
                  : !counts[prefix + "ShiftEnPort"] &&
                        !counts[prefix + "SelectPort"]) {
            valid = false;
          }
          if (host) {
            if (!interface.chains.empty() ||
                counts["ScanInPort"] + counts["ScanOutPort"] == 0) {
              valid = false;
            }
          } else if (interface.chains.empty() && (counts["ScanInPort"] != 1 ||
                                                  counts["ScanOutPort"] != 1)) {
            valid = false;
          }
          if (valid) {
            ++matches;
            interface.kind = kind;
            resolved = std::move(refs);
          }
        }
        if (matches != 1) {
          throw std::runtime_error(
              "ScanInterface must have one valid client/host/TAP type");
        }
        interface.ports = std::move(resolved);
      } catch (const std::exception& error) {
        throw std::runtime_error(node.origin + ": " + node.name + ": " +
                                 error.what());
      }
    }
  }

  Constant ReadAddressValue(const Expression& expression) {
    auto value = netlist_.Value(expression);
    if (value.is_string) {
      throw std::runtime_error("AddressValue must be numeric");
    }
    if (value.data.size() > address_bits_remaining_) {
      throw std::runtime_error("address constant bit limit exceeded");
    }
    address_bits_remaining_ -= value.data.size();
    return value;
  }

  void ConnectAddressedRegisters() {
    std::map<size_t, std::vector<size_t>> data_ports;
    std::set<size_t> members;
    for (size_t id = 0; id < netlist_.nodes.size(); ++id) {
      const auto& node = netlist_.nodes[id];
      if (node.kind == "DataInPort") data_ports[node.scope].push_back(id);
      for (const auto& source : node.one_hot_sources) {
        if (source.kind == OneHotSourceKind::kRegister &&
            (node.kind != "OneHotDataGroup" ||
             netlist_.nodes.at(source.object).scope != node.scope ||
             !members.insert(source.object).second)) {
          throw std::runtime_error(node.origin +
                                   ": invalid addressed register membership");
        }
      }
    }
    for (size_t id = 0; id < netlist_.nodes.size(); ++id) {
      auto& node = netlist_.nodes[id];
      if (node.kind != "DataRegister" ||
          !node.constants.count("AddressValue")) {
        continue;
      }
      try {
        if (!members.count(id)) {
          throw std::runtime_error(
              "addressed register must belong to a OneHotDataGroup");
        }
        for (const std::string property :
             {"WriteDataSource", "WriteEnSource", "ReadDataSource",
              "ReadCallBack", "WriteCallBack"}) {
          if (node.properties.count(property)) {
            throw std::runtime_error(
                "addressed register cannot mix access types");
          }
        }
        auto& bits = node.inputs["WriteDataSource"];
        bits.reserve(node.Width());
        const auto& ports = data_ports[node.scope];
        for (auto port = ports.rbegin();
             port != ports.rend() && bits.size() < node.Width(); ++port) {
          const auto& source = netlist_.nodes[*port];
          for (size_t i = 0; i < source.Width() && bits.size() < node.Width();
               ++i) {
            bits.push_back(
                {*port,
                 source.right + (source.left <= source.right ? -1 : 1) *
                                    static_cast<int64_t>(i),
                 'x', false});
          }
        }
        if (bits.size() != node.Width()) {
          throw std::runtime_error(
              "register exceeds module logical DataIn width");
        }
        std::reverse(bits.begin(), bits.end());
      } catch (const std::exception& error) {
        throw std::runtime_error(node.origin + ": " + node.name + ": " +
                                 error.what());
      }
    }
  }

  void ConnectAddressedData() {
    size_t remaining = 4 * 1024 * 1024;
    for (size_t scope_id = 1; scope_id < netlist_.scopes.size(); ++scope_id) {
      const auto& scope = netlist_.scopes[scope_id];
      if (!scope.address_value) continue;
      const auto& ports = kinds_[{scope_id, "DataInPort"}];
      // Tessent disables defaults for the whole logical bus on any explicit
      // data binding, including a partial binding of just one port.
      if (std::any_of(ports.begin(), ports.end(), [&](size_t id) {
            return netlist_.nodes[id].inputs.count("Source");
          })) {
        continue;
      }
      const auto& sources = kinds_[{scope.parent, "DataInPort"}];
      size_t source_port = 0, source_offset = 0;
      for (size_t id : ports) {
        auto& port = netlist_.nodes[id];
        if (port.Width() > remaining) {
          throw std::runtime_error(
              port.origin + ": " + port.name +
              ": implicit data connection bit limit exceeded");
        }
        remaining -= port.Width();
        auto& bits = port.inputs["Source"];
        bits.reserve(port.Width());
        for (size_t offset = 0; offset < port.Width(); ++offset) {
          if (source_port == sources.size()) {
            throw std::runtime_error(port.origin + ": " + port.name +
                                     ": parent logical DataIn width too small");
          }
          const size_t source_id = sources[source_port];
          const auto& source = netlist_.nodes[source_id];
          const int64_t index =
              source.left + (source.left <= source.right ? 1 : -1) *
                                static_cast<int64_t>(source_offset);
          bits.push_back({source_id, index, 'x', false});
          if (++source_offset == source.Width()) {
            ++source_port;
            source_offset = 0;
          }
        }
      }
    }
  }

  void Connect() {
    for (auto& node : netlist_.nodes) {
      if (node.model != ModelKind::kEnum) continue;
      for (const auto& member : node.properties) {
        auto expression = member.second;
        ++expression.begin;
        node.constants.emplace(member.first, netlist_.Value(expression));
      }
    }
    for (const auto& binding : bindings_) {
      try {
        if (!netlist_.node_ids.count(binding.target) &&
            !netlist_.port_groups.count(binding.target)) {
          throw std::runtime_error("unknown instance input port " +
                                   binding.target);
        }
        const auto targets = netlist_.ResolveSignal(
            binding.target, SelectedRange(netlist_, binding.selection));
        for (const auto& bit : targets) {
          const auto& node = netlist_.nodes[bit.node];
          if (node.model != ModelKind::kPort || !IsInputPort(node.kind)) {
            throw std::runtime_error("InputPort target is not an input port: " +
                                     node.name);
          }
        }
        const auto source = netlist_.Resolve(binding.source, targets.size());
        size_t current = netlist_.nodes.size();
        std::vector<BitReference>* destination = nullptr;
        for (size_t i = 0; i < targets.size(); ++i) {
          const auto& bit = targets[i];
          auto& node = netlist_.nodes[bit.node];
          if (bit.node != current) {
            current = bit.node;
            destination = &node.inputs["Source"];
            if (destination->empty()) {
              destination->resize(
                  node.Width(),
                  {std::numeric_limits<size_t>::max(), 0, '?', false});
            }
          }
          const auto offset = node.left >= node.right ? node.left - bit.index
                                                      : bit.index - node.left;
          auto& target = destination->at(offset);
          if (target.constant != '?') {
            throw std::runtime_error(node.origin +
                                     ": duplicate input binding " + node.name);
          }
          target = source[i];
        }
      } catch (const std::exception& error) {
        const auto& scope = netlist_.scopes[binding.selection.scope];
        const auto& tokens = SourceTokens(netlist_, binding.selection.scope);
        throw std::runtime_error(
            scope.definition.Location(tokens[binding.selection.begin].begin) +
            ": " + error.what());
      }
    }
    ConnectAddressedData();
    for (auto& node : netlist_.nodes) {
      if (node.model == ModelKind::kEnum) continue;
      try {
        if (node.kind == "PortGroup") {
          ConnectPortGroup(&node);
          continue;
        }
        if (node.model == ModelKind::kInterface) {
          ConnectInterface(&node);
          continue;
        }
        if (node.kind == "OneHotScanGroup" || node.kind == "OneHotDataGroup") {
          ConnectOneHotPorts(&node);
          continue;
        }
        node.active_low = ActiveLow(node);
        const auto ref = node.properties.find("RefEnum");
        if (ref != node.properties.end()) {
          const auto& tokens = SourceTokens(netlist_, node.scope);
          const auto name = Path(netlist_.scopes[node.scope].path,
                                 tokens[ref->second.begin].text);
          const auto found = netlist_.node_ids.find(name);
          if (found == netlist_.node_ids.end()) {
            throw std::runtime_error("unknown RefEnum " + name);
          }
          node.enumeration = found->second;
          if (netlist_.nodes[*node.enumeration].model != ModelKind::kEnum) {
            throw std::runtime_error("RefEnum does not name an Enum");
          }
        }
        for (const auto& property : node.properties) {
          const auto& name = property.first;
          if (name == "ReadCallBack" || name == "WriteCallBack") {
            auto callback = ReadRegisterCallback(netlist_, property.second);
            if (name == "ReadCallBack") {
              node.read_callback = std::move(callback);
            } else {
              node.write_callback = std::move(callback);
            }
          }
          if (name == "Period") {
            node.clock_period = ReadClockPeriod(netlist_, property.second);
          }
          if (name == "ResetValue" || name == "DefaultLoadValue" ||
              name == "iApplyEndState") {
            node.constants.emplace(name, ConstantValue(node, property.second));
          }
          if (name == "FreqMultiplier" || name == "FreqDivider") {
            const auto value = netlist_.Value(property.second);
            if (value.AsInteger() <= 0) {
              throw std::runtime_error(name + " must be positive");
            }
            node.constants.emplace(name, value);
          }
          if (name == "AddressValue") {
            node.constants.emplace(name, ReadAddressValue(property.second));
          }
          if (name == "Source" || name == "DifferentialInvOf" ||
              name == "ScanInSource" || name == "CaptureSource" ||
              name == "Enable" || name == "WriteEnSource" ||
              name == "WriteDataSource" || name == "ReadDataSource") {
            const size_t width = name == "ScanInSource" || name == "Enable" ||
                                         name == "WriteEnSource"
                                     ? 1
                                     : node.Width();
            const auto enumeration = name == "CaptureSource"
                                         ? EnumValue(node, property.second)
                                         : std::nullopt;
            if (enumeration) {
              std::vector<BitReference> bits;
              for (char c : enumeration->data) {
                bits.push_back(
                    {std::numeric_limits<size_t>::max(), 0, c, false});
              }
              node.inputs.emplace(name, std::move(bits));
            } else {
              const auto use =
                  name == "ScanInSource" ? ReferenceUse::kScanInput
                  : node.model == ModelKind::kAlias && name == "Source"
                      ? ReferenceUse::kAlias
                      : ReferenceUse::kConnection;
              node.inputs.emplace(
                  name, netlist_.Resolve(property.second, width, use));
            }
          }
        }
        if (node.kind == "ScanOutPort" && node.inputs.count("Source")) {
          for (const auto& bit : node.inputs.at("Source")) {
            if (bit.node == std::numeric_limits<size_t>::max()) continue;
            const auto& source = netlist_.nodes[bit.node];
            if (source.kind == "ScanRegister" && bit.index != source.right) {
              throw std::runtime_error(
                  "ScanOutPort Source must use the rightmost bit of " +
                  source.name);
            }
          }
        }
        if (node.model == ModelKind::kRegister) CheckRegister(node);
        if (node.model == ModelKind::kMux) ConnectMux(&node);
        if (node.model == ModelKind::kLogic) {
          CompileLogic(&netlist_, &node - netlist_.nodes.data(),
                       &logic_bits_remaining_);
        }
      } catch (const std::exception& error) {
        throw std::runtime_error(node.origin + ": " + node.name + ": " +
                                 error.what());
      }
    }
  }

  void CheckAliasConsumers() const {
    std::vector<bool> pdl_only(netlist_.nodes.size());
    std::vector<bool> alias_only(netlist_.nodes.size());
    std::vector<std::vector<size_t>> consumers(netlist_.nodes.size());
    std::vector<size_t> pending;
    for (size_t id = 0; id < netlist_.nodes.size(); ++id) {
      const auto& node = netlist_.nodes[id];
      if (node.model != ModelKind::kAlias) continue;
      for (const auto& bit : node.inputs.at("Source")) {
        if (bit.node == std::numeric_limits<size_t>::max()) continue;
        const auto& source = netlist_.nodes[bit.node];
        if (source.model == ModelKind::kAlias) {
          consumers[bit.node].push_back(id);
        }
        if (source.scope == node.scope) continue;
        if (netlist_.scopes[source.scope].parent != node.scope) {
          pdl_only[id] = true;
        } else if (source.model != ModelKind::kPort ||
                   IsInputPort(source.kind)) {
          alias_only[id] = true;
        }
      }
      if (alias_only[id]) pending.push_back(id);
    }
    for (size_t next = 0; next < pending.size(); ++next) {
      for (size_t id : consumers[pending[next]]) {
        if (alias_only[id]) continue;
        alias_only[id] = true;
        pending.push_back(id);
      }
    }
    if (pending.empty() &&
        std::find(pdl_only.begin(), pdl_only.end(), true) == pdl_only.end()) {
      return;
    }
    for (const auto& node : netlist_.nodes) {
      const auto check = [&](const std::vector<BitReference>& bits) {
        for (const auto& bit : bits) {
          if (bit.node != std::numeric_limits<size_t>::max() &&
              pdl_only[bit.node]) {
            throw std::runtime_error(
                node.origin + ": " + node.name + ": multi-level alias " +
                netlist_.nodes[bit.node].name + " is only available to PDL");
          }
          if (bit.node != std::numeric_limits<size_t>::max() &&
              alias_only[bit.node] && node.model != ModelKind::kAlias) {
            throw std::runtime_error(node.origin + ": " + node.name +
                                     ": internal-instance alias " +
                                     netlist_.nodes[bit.node].name +
                                     " is only available to aliases or PDL");
          }
        }
      };
      for (const auto& input : node.inputs) check(input.second);
      for (const auto& choice : node.choices) check(choice.source);
      for (const auto& instruction : node.logic) check(instruction.signals);
      for (const auto& source : node.one_hot_sources) check(source.bits);
      if (node.scan_interface) {
        check(node.scan_interface->ports);
        for (const auto& chain : node.scan_interface->chains) {
          check(chain.ports);
        }
      }
    }
  }

  void CheckRegister(const NetlistNode& node) const {
    if (node.kind == "ScanRegister" && !node.inputs.count("ScanInSource")) {
      throw std::runtime_error("ScanRegister requires ScanInSource");
    }
    const auto reset = node.constants.find("ResetValue");
    const auto load = node.constants.find("DefaultLoadValue");
    if (reset != node.constants.end() && load != node.constants.end()) {
      for (size_t i = 0; i < node.Width(); ++i) {
        const char a = reset->second.data[i], b = load->second.data[i];
        if (a != 'x' && b != 'x' && a != b) {
          throw std::runtime_error(
              "DefaultLoadValue conflicts with ResetValue");
        }
      }
    }
    const auto ref = node.properties.find("RefEnum");
    if (ref == node.properties.end()) return;
    const auto& tokens = SourceTokens(netlist_, node.scope);
    const auto& table = netlist_.NodeAt(
        Path(netlist_.scopes[node.scope].path, tokens[ref->second.begin].text));
    if (table.model != ModelKind::kEnum) {
      throw std::runtime_error("RefEnum does not name an Enum");
    }
    for (const auto& entry : table.constants) {
      if (entry.second.data.size() != node.Width()) {
        throw std::runtime_error("enum value width mismatch");
      }
    }
  }

  std::optional<Constant> EnumValue(const NetlistNode& node,
                                    const Expression& value) const {
    const auto ref = node.properties.find("RefEnum");
    if (ref == node.properties.end() || value.end != value.begin + 1) {
      return std::nullopt;
    }
    const auto& tokens = SourceTokens(netlist_, node.scope);
    const auto& table = netlist_.NodeAt(
        Path(netlist_.scopes[node.scope].path, tokens[ref->second.begin].text));
    if (table.model != ModelKind::kEnum) {
      throw std::runtime_error("RefEnum does not name an Enum");
    }
    const auto found = table.constants.find(tokens[value.begin].text);
    if (found == table.constants.end()) return std::nullopt;
    if (found->second.data.size() != node.Width()) {
      throw std::runtime_error("enum value width mismatch");
    }
    return found->second;
  }

  Constant ConstantValue(const NetlistNode& node,
                         const Expression& expression) const {
    const auto value = EnumValue(node, expression);
    return value ? *value : netlist_.Value(expression, node.Width());
  }

  void ConnectMux(NetlistNode* node) {
    std::vector<BitReference> selector;
    try {
      selector =
          netlist_.Resolve(node->properties.at("SelectedBy"), 0,
                           ReferenceUse::kConnection, mux_bits_remaining_);
    } catch (const std::runtime_error& error) {
      if (mux_bits_remaining_ < kMaxWidth &&
          std::string(error.what()) == "signal operand bit limit exceeded") {
        throw std::runtime_error("mux expansion bit limit exceeded");
      }
      throw;
    }
    const size_t width = selector.size();
    SpendMuxBits(width, &mux_bits_remaining_);
    node->inputs.emplace("SelectedBy", std::move(selector));
    const auto& module = netlist_.scopes[node->scope].definition;
    const auto& tokens = SourceTokens(netlist_, node->scope);
    const auto* block = dynamic_cast<const icl::AstBlock*>(node->ast);
    for (const auto* item : block->body()) {
      if (dynamic_cast<const icl::AstComment*>(item)) continue;
      const auto* statement = dynamic_cast<const icl::AstStatement*>(item);
      if (!statement || statement->named_kind() == icl::NamedKind::kAttribute) {
        continue;
      }
      auto expression = Tokens(module, *item, node->scope);
      --expression.end;
      size_t colon = expression.begin;
      while (colon < expression.end && tokens[colon].text != ":") ++colon;
      MuxChoice choice;
      for (size_t begin = expression.begin; begin < colon;) {
        size_t end = begin;
        while (end < colon && tokens[end].text != "|") ++end;
        SpendMuxBits(width, &mux_bits_remaining_);
        auto code = netlist_.Value({node->scope, begin, end}, width);
        if (code.is_string) {
          throw std::runtime_error("mux selector code must be numeric");
        }
        choice.codes.push_back(std::move(code));
        begin = end + 1;
      }
      SpendMuxBits(node->Width(), &mux_bits_remaining_);
      choice.source = netlist_.Resolve({node->scope, colon + 1, expression.end},
                                       node->Width());
      node->choices.push_back(std::move(choice));
    }
  }

  bool ActiveLow(const NetlistNode& node) const {
    const auto polarity = node.properties.find("ActivePolarity");
    if (polarity != node.properties.end()) {
      return netlist_.Integer(polarity->second) == 0;
    }
    return node.kind == "TRSTPort" || node.kind == "ToTRSTPort";
  }

  BitReference ResetSource(size_t scope_id, const std::string& input,
                           const std::string& output, const std::string& role) {
    const auto key = std::make_pair(scope_id, input);
    auto cached = reset_candidates_.find(key);
    if (cached == reset_candidates_.end()) {
      auto candidates = kinds_[key];
      if (candidates.empty()) {
        for (size_t child : netlist_.scopes[scope_id].children) {
          const auto& ports = kinds_[{child, output}];
          candidates.insert(candidates.end(), ports.begin(), ports.end());
        }
      }
      cached = reset_candidates_.emplace(key, std::move(candidates)).first;
    }
    const auto& candidates = cached->second;
    if (candidates.size() > 1) throw std::runtime_error("ambiguous " + role);
    if (candidates.empty()) {
      if (input == "TMSPort") {
        throw std::runtime_error("missing source of " + role);
      }
      BitReference source;
      source.virtual_signal = input == "TRSTPort"
                                  ? VirtualSignal::kAsyncGlobalReset
                                  : VirtualSignal::kGlobalReset;
      return source;
    }
    const auto& source = netlist_.nodes[candidates[0]];
    if (source.Width() != 1) throw std::runtime_error("ambiguous " + role);
    return {candidates[0], source.left, 'x', source.active_low};
  }

  void InferResetPolarities(AliasResolver& terminal) {
    // Explicit parent bindings dictate omitted defaults; implied wiring
    // normalizes.
    for (size_t scope = 0; scope < netlist_.scopes.size(); ++scope) {
      const auto found = kinds_.find({scope, "ResetPort"});
      if (found == kinds_.end()) continue;
      for (size_t id : found->second) {
        auto& node = netlist_.nodes[id];
        const auto source = node.inputs.find("Source");
        if (source == node.inputs.end() ||
            node.properties.count("ActivePolarity")) {
          continue;
        }
        std::optional<bool> polarity;
        for (const auto& bit : source->second) {
          const auto driver = ResetPolarity(netlist_, terminal, bit);
          if (!driver) continue;
          if (polarity && *polarity != *driver) {
            throw std::runtime_error(node.origin + ": " + node.name +
                                     ": conflicting reset source polarity");
          }
          polarity = driver;
        }
        if (polarity) node.active_low = *polarity;
      }
    }
  }

  void InferResets() {
    size_t inferred_bits = 0;
    size_t tck_bits = 0;
    // Bound implicit expansion before allocating any per-bit source buffers.
    for (const auto& node : netlist_.nodes) {
      if (node.inputs.count("Source")) continue;
      const bool input = node.kind == "ResetPort" || node.kind == "TRSTPort" ||
                         node.kind == "TMSPort";
      const bool output = node.kind == "ToResetPort" ||
                          node.kind == "ToTRSTPort" ||
                          node.kind == "ToTMSPort" || node.kind == "ToTCKPort";
      if (!output && (!input || netlist_.scopes[node.scope].parent ==
                                    std::numeric_limits<size_t>::max())) {
        continue;
      }
      if (node.Width() > inferred_control_bits_remaining_ - inferred_bits) {
        throw std::runtime_error(
            node.origin + ": inferred control connection bit limit exceeded");
      }
      inferred_bits += node.Width();
      if (node.kind == "ToTCKPort") tck_bits += node.Width();
    }
    // ToTCK wiring uses the restored port index; debit only early allocations.
    inferred_control_bits_remaining_ -= inferred_bits - tck_bits;
    for (auto& node : netlist_.nodes) {
      try {
        if ((node.kind == "ScanRegister" || node.kind == "DataRegister") &&
            node.constants.count("ResetValue")) {
          node.reset = ResetSource(node.scope, "ResetPort", "ToResetPort",
                                   "register reset signal");
        }
        if (node.inputs.count("Source")) continue;
        for (const std::string type : {"Reset", "TRST", "TMS"}) {
          const auto input = type + "Port", output = "To" + type + "Port";
          if (node.kind != input && node.kind != output) continue;
          const auto parent = netlist_.scopes[node.scope].parent;
          if (node.kind == input &&
              parent == std::numeric_limits<size_t>::max()) {
            continue;
          }
          auto source = ResetSource(node.kind == input ? parent : node.scope,
                                    input, output, "source of " + node.name);
          source.inverted = source.inverted != node.active_low;
          node.inputs["Source"].assign(node.Width(), source);
        }
      } catch (const std::exception& error) {
        throw std::runtime_error(node.origin + ": " + error.what());
      }
    }
  }

  Design& design_;
  const std::map<std::string, ModuleDefinition>& access_link_models_;
  const std::map<std::string, std::string>& top_ports_;
  Netlist netlist_;
  size_t logic_bits_remaining_ = 4 * 1024 * 1024;
  size_t address_bits_remaining_ = kMaxAddressBits;
  size_t one_hot_bits_remaining_ = 4 * 1024 * 1024;
  size_t port_group_bits_remaining_ = kMaxPortGroupBits;
  size_t mux_bits_remaining_ = kMaxMuxBits;
  size_t inferred_control_bits_remaining_ = 4 * 1024 * 1024;
  std::map<std::string, Template> templates_;
  std::map<std::string, size_t> scope_ids_;
  std::map<std::pair<size_t, std::string>, std::vector<size_t>> kinds_;
  std::map<std::pair<size_t, std::string>, std::vector<size_t>>
      reset_candidates_;
  struct Binding {
    std::string target;
    Expression selection, source;
  };
  std::vector<Binding> bindings_;
};
}  // namespace

std::vector<ScanInterfaceBinding> EffectiveScanInterfaces(
    const Netlist& netlist) {
  std::vector<ScanInterfaceBinding> result;
  std::set<size_t> explicit_scopes;
  std::vector<std::map<std::string, std::vector<size_t>>> ports(
      netlist.scopes.size());
  for (size_t id = 0; id < netlist.nodes.size(); ++id) {
    const auto& node = netlist.nodes[id];
    if (node.scan_interface) {
      result.push_back({node.scope, *node.scan_interface, node.origin});
      explicit_scopes.insert(node.scope);
    } else if (node.model == ModelKind::kPort) {
      ports.at(node.scope)[node.kind].push_back(id);
    }
  }
  for (size_t scope = 0; scope < ports.size(); ++scope) {
    if (explicit_scopes.count(scope)) continue;
    const auto scalar = [&](const std::string& kind) {
      const auto found = ports[scope].find(kind);
      return found != ports[scope].end() && found->second.size() == 1 &&
             netlist.nodes[found->second[0]].Width() == 1;
    };
    if (!scalar("ScanInPort") || !scalar("ScanOutPort") ||
        (!scalar("SelectPort") && !scalar("ShiftEnPort") &&
         !scalar("TMSPort"))) {
      continue;
    }
    ScanInterface interface;
    interface.kind = scalar("TMSPort") ? ScanInterfaceKind::kClientTap
                                       : ScanInterfaceKind::kClient;
    const std::vector<std::string> kinds =
        scalar("TMSPort")
            ? std::vector<std::string>{"ScanInPort", "ScanOutPort", "TCKPort",
                                       "TMSPort", "TRSTPort"}
            : std::vector<std::string>{
                  "ScanInPort",  "ScanOutPort",   "TCKPort",      "SelectPort",
                  "ShiftEnPort", "CaptureEnPort", "UpdateEnPort", "ResetPort"};
    for (const auto& kind : kinds) {
      if (scalar(kind)) {
        const size_t id = ports[scope].at(kind)[0];
        interface.ports.push_back({id, netlist.nodes[id].left});
      }
    }
    result.push_back({scope, std::move(interface),
                      netlist.nodes[ports[scope].at("ScanInPort")[0]].origin});
  }
  return result;
}

std::vector<std::string> UncoveredMuxCodes(const NetlistNode& node) {
  const size_t width = node.inputs.at("SelectedBy").size();
  size_t remaining = 4 * 1024 * 1024;
  const auto spend = [&]() {
    if (width > remaining) {
      throw std::runtime_error("mux coverage resource limit exceeded");
    }
    remaining -= width;
  };
  spend();
  std::vector<std::string> uncovered{std::string(width, 'x')};
  for (const auto& choice : node.choices) {
    for (const auto& code : choice.codes) {
      std::vector<std::string> next;
      for (auto& cube : uncovered) {
        spend();
        bool overlaps = true;
        for (size_t bit = 0; bit < width; ++bit) {
          if (cube[bit] != 'x' && code.data.at(bit) != 'x' &&
              cube[bit] != code.data[bit]) {
            overlaps = false;
            break;
          }
        }
        if (!overlaps) {
          next.push_back(std::move(cube));
          continue;
        }
        spend();
        for (size_t bit = 0; bit < width; ++bit) {
          if (cube[bit] == 'x' && code.data[bit] != 'x') {
            spend();
            next.push_back(cube);
            next.back()[bit] = code.data[bit] == '0' ? '1' : '0';
            cube[bit] = code.data[bit];
          }
        }
      }
      uncovered = std::move(next);
      if (uncovered.empty()) return uncovered;
    }
  }
  return uncovered;
}

bool IsInputPort(const std::string& kind) {
  return kind == "ScanInPort" || kind == "DataInPort" || kind == "ResetPort" ||
         kind == "TRSTPort" || kind == "TMSPort" || kind == "SelectPort" ||
         kind == "ShiftEnPort" || kind == "CaptureEnPort" ||
         kind == "UpdateEnPort" || kind == "TCKPort" || kind == "ClockPort" ||
         kind == "AddressPort" || kind == "ReadEnPort" || kind == "WriteEnPort";
}

void CheckCombinationalConnections(const Netlist& netlist) {
  // Signal-level cycles may be acyclic between bits; registers cut the graph.
  constexpr size_t kMaxGraphBits = 16 * 1024 * 1024;
  size_t mux_bits_remaining = kMaxMuxBits;
  std::vector<size_t> offsets{0};
  for (const auto& node : netlist.nodes) {
    if (node.model == ModelKind::kMux) {
      try {
        SpendMuxBits(node.inputs.at("SelectedBy").size(), &mux_bits_remaining);
        for (const auto& choice : node.choices) {
          SpendMuxBits(choice.source.size(), &mux_bits_remaining);
          for (const auto& code : choice.codes) {
            SpendMuxBits(code.data.size(), &mux_bits_remaining);
          }
        }
      } catch (const std::runtime_error& error) {
        throw std::runtime_error(node.origin + ": " + node.name + ": " +
                                 error.what());
      }
    }
    if (node.Width() > kMaxGraphBits - offsets.back()) {
      throw std::runtime_error("ICL bit dependency graph limit exceeded");
    }
    offsets.push_back(offsets.back() + node.Width());
  }
  const size_t real_bits = offsets.back();
  std::vector<size_t> selector_vertices(netlist.nodes.size(),
                                        std::numeric_limits<size_t>::max());
  std::vector<size_t> selector_owners;
  for (size_t id = 0; id < netlist.nodes.size(); ++id) {
    const auto& node = netlist.nodes[id];
    if (node.model == ModelKind::kMux && node.Width() > 1 &&
        node.inputs.at("SelectedBy").size() > 1) {
      selector_vertices[id] = real_bits + selector_owners.size();
      selector_owners.push_back(id);
    }
  }
  const auto owner = [&](size_t bit) {
    if (bit >= real_bits) return selector_owners[bit - real_bits];
    return static_cast<size_t>(
        std::upper_bound(offsets.begin(), offsets.end(), bit) -
        offsets.begin() - 1);
  };
  const auto bit_id = [&](const BitReference& ref) {
    const auto& source = netlist.nodes.at(ref.node);
    const size_t index = static_cast<size_t>(source.left >= source.right
                                                 ? source.left - ref.index
                                                 : ref.index - source.left);
    return offsets[ref.node] + index;
  };
  const auto dependencies = [&](size_t bit) {
    std::vector<size_t> result;
    const size_t id = owner(bit);
    const auto& node = netlist.nodes[id];
    if (node.model == ModelKind::kRegister) return result;
    const auto append = [&](const BitReference& ref) {
      if (ref.node == std::numeric_limits<size_t>::max()) return;
      result.push_back(bit_id(ref));
    };
    if (bit >= real_bits) {
      for (const auto& ref : node.inputs.at("SelectedBy")) append(ref);
    } else {
      const size_t position = bit - offsets[id];
      for (const auto& input : node.inputs) {
        if (input.first == "SelectedBy" &&
            selector_vertices[id] != std::numeric_limits<size_t>::max()) {
          result.push_back(selector_vertices[id]);
          continue;
        }
        if (input.first != "SelectedBy" &&
            input.second.size() == node.Width()) {
          append(input.second[position]);
        } else {
          for (const auto& ref : input.second) append(ref);
        }
      }
      for (const auto& choice : node.choices) {
        append(choice.source.at(position));
      }
      for (size_t source = 0; source < node.one_hot_sources.size(); ++source) {
        if (position <
            node.Width() - node.one_hot_sources[source].bits.size()) {
          break;
        }
        append(node.OneHotSourceBit(source, position));
      }
    }
    std::sort(result.begin(), result.end());
    result.erase(std::unique(result.begin(), result.end()), result.end());
    return result;
  };
  struct Visit {
    size_t bit, next;
    std::vector<size_t> dependencies;
  };
  std::vector<unsigned char> state(real_bits + selector_owners.size());
  const bool has_groups = std::any_of(
      netlist.nodes.begin(), netlist.nodes.end(),
      [](const NetlistNode& node) { return !node.one_hot_sources.empty(); });
  std::vector<unsigned char> enabled_paths(has_groups ? offsets.back() : 0);
  const auto enabled = [&](const BitReference& ref) -> unsigned char {
    return ref.node == std::numeric_limits<size_t>::max()
               ? 0
               : enabled_paths[bit_id(ref)];
  };
  const auto propagate_enable = [&](size_t bit) {
    if (bit >= real_bits) return;
    const size_t id = owner(bit);
    const auto& node = netlist.nodes[id];
    const auto position = bit - offsets[id];
    if (node.kind == "OneHotScanGroup" ||
        (node.kind == "ScanOutPort" && node.inputs.count("Enable"))) {
      enabled_paths[bit] = 1;
    } else if (node.kind == "OneHotDataGroup" ||
               (node.kind == "DataOutPort" &&
                (node.inputs.count("Enable") ||
                 netlist.scopes[node.scope].address_value))) {
      enabled_paths[bit] = 2;
    } else if (node.model == ModelKind::kAlias || node.kind == "ScanOutPort" ||
               node.kind == "ScanInPort" || node.kind == "DataOutPort") {
      const auto source = node.inputs.find("Source");
      const unsigned char mask = node.model == ModelKind::kAlias ? 3
                                 : node.kind == "DataOutPort"    ? 2
                                                                 : 1;
      enabled_paths[bit] = source == node.inputs.end()
                               ? 0
                               : enabled(source->second.at(position)) & mask;
    } else if (node.kind == "ScanMux") {
      enabled_paths[bit] =
          std::any_of(node.choices.begin(), node.choices.end(),
                      [&](const MuxChoice& choice) {
                        return (enabled(choice.source.at(position)) & 1) != 0;
                      });
    }
  };
  std::vector<Visit> stack;
  for (size_t root = 0; root < state.size(); ++root) {
    if (state[root]) continue;
    state[root] = 1;
    stack.push_back({root, 0, dependencies(root)});
    while (!stack.empty()) {
      auto& frame = stack.back();
      if (frame.next == frame.dependencies.size()) {
        if (has_groups) propagate_enable(frame.bit);
        state[frame.bit] = 2;
        stack.pop_back();
        continue;
      }
      const size_t next = frame.dependencies[frame.next++];
      if (state[next] == 1) {
        const auto& node = netlist.nodes[owner(next)];
        throw std::runtime_error(
            node.origin + ": combinational connection cycle at " + node.name);
      }
      if (!state[next]) {
        state[next] = 1;
        stack.push_back({next, 0, dependencies(next)});
      }
    }
  }
  for (const auto& node : netlist.nodes) {
    const unsigned char mask = node.kind == "OneHotDataGroup" ? 2 : 1;
    for (const auto& source : node.one_hot_sources) {
      if (source.kind == OneHotSourceKind::kRegister) continue;
      if (!std::all_of(source.bits.begin(), source.bits.end(),
                       [&](const BitReference& bit) {
                         return (enabled(bit) & mask) != 0;
                       })) {
        throw std::runtime_error(node.origin + ": " + node.name +
                                 ": group source needs an enabled " +
                                 (mask == 2
                                      ? "DataOutPort or OneHotDataGroup"
                                      : "ScanOutPort or OneHotScanGroup") +
                                 " in its data fan-in");
      }
    }
  }
}

BitReference NetlistNode::OneHotSourceBit(size_t source,
                                          size_t position) const {
  const auto& bits = one_hot_sources.at(source).bits;
  if (position >= Width() || bits.size() > Width()) {
    throw std::out_of_range("invalid one-hot source bit position or width");
  }
  // Tessent 2026.2 Port contribution: right-align, unknown leading positions.
  const size_t padding = Width() - bits.size();
  return position < padding ? BitReference{} : bits.at(position - padding);
}

std::string PortFunctionName(const NetlistNode& node) {
  auto name = node.BaseName() + "[" + std::to_string(node.left);
  if (node.left != node.right) name += ":" + std::to_string(node.right);
  return name + "]";
}

void IndexPortGroups(Netlist* netlist) {
  for (auto& [name, group] : netlist->port_groups) {
    if (group.members.size() < 2 || netlist->node_ids.count(name)) {
      throw std::runtime_error("invalid segmented port identity " + name);
    }
    std::optional<bool> ascending;
    for (size_t id : group.members) {
      const auto& node = netlist->nodes.at(id);
      if (node.model != ModelKind::kPort || !node.indexed ||
          node.BaseName() != name || node.name != PortFunctionName(node) ||
          node.scope != netlist->nodes.at(group.members.front()).scope ||
          std::min(node.left, node.right) < 0) {
        throw std::runtime_error("invalid segmented port function " + name);
      }
      if (node.left == node.right) continue;
      const bool direction = node.left < node.right;
      if (ascending && *ascending != direction) {
        throw std::runtime_error("segmented port direction conflict " + name);
      }
      ascending = direction;
    }
    std::sort(group.members.begin(), group.members.end(),
              [&](size_t a, size_t b) {
                const auto& x = netlist->nodes[a];
                const auto& y = netlist->nodes[b];
                return std::min(x.left, x.right) < std::min(y.left, y.right);
              });
    const auto& first = netlist->nodes[group.members.front()];
    const auto lower = std::min(first.left, first.right);
    auto upper = std::max(first.left, first.right);
    for (size_t i = 1; i < group.members.size(); ++i) {
      const auto& node = netlist->nodes[group.members[i]];
      const auto next = std::min(node.left, node.right);
      if (next <= upper) {
        throw std::runtime_error("overlapping port functions " + name);
      }
      if (next - upper != 1) {
        throw std::runtime_error("segmented port must be contiguous " + name);
      }
      upper = std::max(node.left, node.right);
    }
    if (static_cast<uint64_t>(upper - lower) >= kMaxWidth) {
      throw std::runtime_error("segmented port width limit exceeded " + name);
    }
    group.left = ascending.value_or(false) ? lower : upper;
    group.right = ascending.value_or(false) ? upper : lower;
  }
}

void ValidatePortGroupMembers(const Netlist& netlist) {
  size_t remaining = kMaxPortGroupBits;
  for (const auto& node : netlist.nodes) {
    if (node.kind != "PortGroup" && node.port_group.empty()) continue;
    const auto reject = [&](const std::string& message) {
      throw std::runtime_error(node.origin + ": PortGroup " + node.name + ": " +
                               message);
    };
    if (node.kind != "PortGroup" || node.model != ModelKind::kGroup ||
        node.scope >= netlist.scopes.size() || node.indexed || node.left != 0 ||
        node.right != 0 || node.active_low || node.port_group.empty() ||
        !node.properties.empty() || !node.attributes.empty() ||
        !node.inputs.empty() || !node.constants.empty() || node.reset ||
        !node.choices.empty() || !node.one_hot_sources.empty() ||
        !node.logic.empty() || node.scan_interface || node.enumeration ||
        node.clock_period || node.read_callback || node.write_callback) {
      reject("invalid annotation representation");
    }
    if (node.port_group.size() > std::min(remaining, kMaxWidth)) {
      reject("member expansion bit limit exceeded");
    }
    remaining -= node.port_group.size();
    for (const auto& bit : node.port_group) {
      if (bit.node >= netlist.nodes.size() || bit.inverted ||
          bit.virtual_signal != VirtualSignal::kNone) {
        reject("member must name a local port");
      }
      const auto& port = netlist.nodes[bit.node];
      if (port.model != ModelKind::kPort || port.kind.size() < 4 ||
          port.kind.compare(port.kind.size() - 4, 4, "Port") != 0 ||
          port.scope != node.scope ||
          (!port.indexed &&
           (port.left != 0 || port.right != 0 || bit.index != 0)) ||
          bit.index < std::min(port.left, port.right) ||
          bit.index > std::max(port.left, port.right)) {
        reject("member must name a valid local port bit");
      }
    }
  }
}

std::vector<BitReference> Netlist::ResolveSignal(
    const std::string& name,
    std::optional<std::pair<int64_t, int64_t>> selection,
    size_t max_bits) const {
  const auto single = node_ids.find(name);
  const auto grouped = port_groups.find(name);
  if (single == node_ids.end() && grouped == port_groups.end()) {
    throw std::runtime_error("unknown signal " + name);
  }
  const auto* node =
      single == node_ids.end() ? nullptr : &nodes.at(single->second);
  const auto left = node ? node->left : grouped->second.left;
  const auto right = node ? node->right : grouped->second.right;
  const auto range = selection.value_or(std::make_pair(left, right));
  if (selection && node && !node->indexed) {
    throw std::runtime_error("invalid vector selection on " + name);
  }
  if (range.first < std::min(left, right) ||
      range.second < std::min(left, right) ||
      range.first > std::max(left, right) ||
      range.second > std::max(left, right)) {
    throw std::runtime_error("signal index out of range for " + name);
  }
  const auto width = static_cast<size_t>(std::max(range.first, range.second) -
                                         std::min(range.first, range.second)) +
                     1;
  if (width > std::min(max_bits, kMaxWidth)) {
    throw std::runtime_error("signal operand bit limit exceeded");
  }
  std::vector<BitReference> result;
  result.reserve(width);
  const int step = range.first <= range.second ? 1 : -1;
  if (node) {
    for (int64_t index = range.first;; index += step) {
      result.push_back({single->second, index});
      if (index == range.second) break;
    }
    return result;
  }
  const auto& members = grouped->second.members;
  auto member =
      std::lower_bound(members.begin(), members.end(), range.first,
                       [&](size_t id, int64_t index) {
                         const auto& source = nodes[id];
                         return std::max(source.left, source.right) < index;
                       });
  auto index = range.first;
  while (true) {
    const auto& source = nodes[*member];
    const auto last =
        step > 0 ? std::min(range.second, std::max(source.left, source.right))
                 : std::max(range.second, std::min(source.left, source.right));
    for (;;) {
      result.push_back({*member, index});
      if (index == last) break;
      index += step;
    }
    if (index == range.second) break;
    index += step;
    member += step;
  }
  return result;
}

const NetlistNode& Netlist::NodeAt(const std::string& path) const {
  const auto found = node_ids.find(path);
  if (found == node_ids.end()) {
    throw std::runtime_error("unknown ICL object " + path);
  }
  return nodes.at(found->second);
}

Expression Netlist::ExpressionAt(size_t scope, icl::SourceSpan span) const {
  const auto& module = scopes.at(scope).definition;
  const auto range = module.document->Range(span);
  return {scope, TokenAt(module, range.begin), TokenAt(module, range.end)};
}

int64_t Netlist::Integer(const Expression& expression) const {
  const auto& scope = scopes.at(expression.scope);
  return EvaluateInteger(SourceTokens(*this, expression.scope),
                         expression.begin, expression.end,
                         [&](const std::string& name) {
                           return Parameter(scope, name).AsInteger();
                         });
}

Constant Netlist::Value(const Expression& expression, size_t width) const {
  const auto& scope = scopes.at(expression.scope);
  return EvaluateConstant(
      SourceTokens(*this, expression.scope), expression.begin, expression.end,
      [&](const std::string& name) { return Parameter(scope, name); }, width);
}

std::vector<BitReference> Netlist::Resolve(const Expression& expression,
                                           size_t width, ReferenceUse use,
                                           size_t max_bits) const {
  const bool scan_input = use == ReferenceUse::kScanInput;
  max_bits = std::min(max_bits, kMaxWidth);
  if (width > max_bits) {
    throw std::runtime_error("signal operand bit limit exceeded");
  }
  const auto& tokens = SourceTokens(*this, expression.scope);
  const auto& scope = scopes.at(expression.scope);
  struct Segment {
    std::vector<BitReference> bits;
    Constant value;
    bool literal = false, inverted = false;
  };
  std::vector<Segment> segments;
  size_t known = 0, unsized = 0;
  for (size_t first = expression.begin; first < expression.end;) {
    size_t end = first;
    while (end < expression.end && tokens[end].text != ",") ++end;
    Segment segment;
    const bool has_inversion = tokens[first].text == "~";
    while (first < end && tokens[first].text == "~") {
      segment.inverted = !segment.inverted;
      ++first;
    }
    if (first == end) throw std::runtime_error("empty signal reference");
    if (std::isalpha(static_cast<unsigned char>(tokens[first].text[0]))) {
      std::string name = tokens[first++].text;
      while (first + 1 < end && tokens[first].text == ".") {
        name += "." + tokens[first + 1].text;
        first += 2;
      }
      const auto path = Path(scope.path, name);
      std::optional<std::pair<int64_t, int64_t>> range;
      range = SelectedRange(*this, {expression.scope, first, end});
      if (!range && scan_input) {
        const auto id = node_ids.find(path);
        if (id != node_ids.end() && nodes[id->second].kind == "ScanRegister" &&
            nodes[id->second].indexed) {
          range = {nodes[id->second].right, nodes[id->second].right};
        }
      }
      segment.bits = ResolveSignal(path, range, max_bits - known);
      for (auto& bit : segment.bits) {
        const auto& source = nodes[bit.node];
        if (source.model == ModelKind::kEnum ||
            source.model == ModelKind::kInterface ||
            source.model == ModelKind::kAccessLink ||
            source.kind == "PortGroup") {
          throw std::runtime_error(source.name +
                                   " is not a data signal or wire");
        }
        if (use != ReferenceUse::kAlias && source.scope != expression.scope &&
            (source.model != ModelKind::kPort || IsInputPort(source.kind))) {
          throw std::runtime_error(
              scope.definition.Location(tokens[expression.begin].begin) +
              ": hierarchical signal " + source.name +
              " must name an output port");
        }
        if (scan_input && source.kind == "ScanRegister" &&
            bit.index != source.right) {
          throw std::runtime_error(
              "ScanInSource must use the rightmost bit of " + source.name);
        }
        bit.inverted = segment.inverted;
      }
      known += segment.bits.size();
    } else {
      segment.literal = true;
      segment.value = Value({expression.scope, first, end});
      if (segment.value.is_string) {
        throw std::runtime_error("string used as signal");
      }
      if (has_inversion && !segment.value.sized) {
        throw std::runtime_error("unsized signal element cannot be inverted");
      }
      if (segment.value.sized) {
        known += segment.value.data.size();
      } else {
        ++unsized;
      }
    }
    if (known > max_bits) {
      throw std::runtime_error("signal operand bit limit exceeded");
    }
    segments.push_back(std::move(segment));
    first = end + 1;
  }
  if (unsized > 1 || (unsized && !width)) {
    throw std::runtime_error("unsized signal requires a target width");
  }
  if (width && (known > width || (!unsized && known != width))) {
    throw std::runtime_error("source width does not match target width");
  }
  std::vector<BitReference> result;
  for (auto& segment : segments) {
    if (!segment.literal) {
      result.insert(result.end(), segment.bits.begin(), segment.bits.end());
      continue;
    }
    auto bits = segment.value.data;
    if (!segment.value.sized) {
      const size_t target = width - known;
      bits = segment.value.Sized(target).data;
    }
    for (char bit : bits) {
      if (segment.inverted) bit = bit == '0' ? '1' : bit == '1' ? '0' : 'x';
      result.push_back({std::numeric_limits<size_t>::max(), 0, bit, false});
    }
  }
  return result;
}

Netlist Elaborate(Design& design, const std::string& top) {
  return Elaborate(design, top, {});
}

Netlist Elaborate(
    Design& design, const std::string& top,
    const std::map<std::string, ModuleDefinition>& access_link_models,
    const std::map<std::string, std::string>& top_ports) {
  return Elaborator(design, access_link_models, top_ports).Run(top);
}
}  // namespace pattern
