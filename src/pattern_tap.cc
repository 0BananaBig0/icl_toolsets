// Copyright 2026 The ICL Toolsets Authors.
// SPDX-License-Identifier: MIT
#include "pattern_tap.h"

#include <algorithm>
#include <optional>
#include <set>
#include <stdexcept>
#include <utility>
#include <vector>

namespace pattern {
namespace {
void ValidateTapInput(char value) {
  if (value != '0' && value != '1' && value != 'x') {
    throw std::invalid_argument("TAP input must be 0, 1 or x");
  }
}

char StateActive(uint16_t states, uint16_t mask) {
  if (!(states & mask)) return '0';
  return (states & mask) == states ? '1' : 'x';
}

char ClockAction(uint16_t states, TapState state, char trst) {
  const char active = StateActive(states, TapStateBit(state));
  if (trst == '0' || active == '0') return '0';
  return trst == 'x' ? 'x' : active;
}

constexpr size_t kMaxModelBytes = 16 * 1024 * 1024;
constexpr size_t kMaxActiveBits = 1024 * 1024;
struct SignalRange {
  bool indexed = false;
  int64_t left = 0, right = 0;
};
struct Signal : SignalRange {
  std::vector<std::vector<size_t>> instructions;
};
bool Identifier(const std::string& name) {
  const auto letter = [](char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
  };
  if (name.empty() || !letter(name[0])) return false;
  for (char c : name) {
    if (!letter(c) && !(c >= '0' && c <= '9') && c != '_') return false;
  }
  return true;
}
class Builder {
 public:
  explicit Builder(const AccessLink& link) : link_(link) {}
  TapDatapathModel Run() {
    if (!link_.bsdl) throw std::runtime_error("missing bound BSDL metadata");
    ReadSignalRanges();
    const auto input = Fresh("tdi");
    const auto reg = Fresh("INSTRUCTION");
    const auto width = std::to_string(link_.bsdl->instruction_width);
    Append("Module IclPatternTapDatapath {\nScanInPort " + input + ";\n");
    Append("ScanRegister " + reg + "[" +
           std::to_string(link_.bsdl->instruction_width - 1) +
           ":0] { ScanInSource " + input + "; CaptureSource " + width + "'b" +
           link_.bsdl->capture + "; }\n");
    std::map<std::string, std::string> decoders;
    std::vector<std::string> decoder_names;
    size_t remaining_bits = kMaxActiveBits;
    for (const auto& instruction : link_.instructions) {
      const auto name = Fresh("instruction_select");
      std::vector<std::string> alternatives;
      for (const auto& code : link_.bsdl->opcodes.at(instruction.name)) {
        alternatives.push_back(reg + " == " + width + "'b" + code);
      }
      Append("LogicSignal " + name + " { " + Or(std::move(alternatives)) +
             "; }\n");
      decoders.emplace(instruction.name, name);
      const size_t instruction_id = decoder_names.size();
      decoder_names.push_back(name);
      std::set<std::string> seen;
      for (const auto& signal : instruction.active_signals) {
        if (!seen.insert(signal.text).second) continue;
        const auto& range = ranges_.at(signal.text);
        auto& output = signals_.at(BaseName(signal.text));
        const auto lower = std::min(output.left, output.right);
        const auto first = std::min(range.left, range.right);
        const size_t width = std::max(range.left, range.right) - first + 1;
        if (width > remaining_bits) {
          throw std::runtime_error(signal.origin +
                                   ": ActiveSignal selection limit exceeded");
        }
        remaining_bits -= width;
        for (size_t offset = 0; offset < width; ++offset) {
          auto& selected = output.instructions[first - lower + offset];
          if (!selected.empty() && selected.back() == instruction_id) continue;
          selected.push_back(instruction_id);
        }
      }
    }
    std::map<std::vector<size_t>, std::string> shared_sources;
    for (const auto& entry : signals_) {
      const auto& signal = entry.second;
      std::string sources;
      for (size_t offset = 0; offset < signal.instructions.size(); ++offset) {
        const size_t index = signal.left > signal.right
                                 ? signal.instructions.size() - 1 - offset
                                 : offset;
        const auto& selected = signal.instructions[index];
        std::string source = "1'b0";
        if (selected.size() == 1) {
          source = decoder_names[selected.front()];
        } else if (!selected.empty()) {
          auto found = shared_sources.find(selected);
          if (found == shared_sources.end()) {
            std::vector<std::string> terms;
            for (size_t id : selected) terms.push_back(decoder_names[id]);
            source = Fresh("active_signal");
            Append("LogicSignal " + source + " { " + Or(std::move(terms)) +
                   "; }\n");
            found = shared_sources.emplace(selected, source).first;
          }
          source = found->second;
        }
        const size_t separator = sources.empty() ? 0 : 1;
        if (source.size() + separator > kMaxModelBytes - sources.size()) {
          throw std::runtime_error("ActiveSignal source text limit exceeded");
        }
        if (separator) sources += ",";
        sources += source;
      }
      const auto range = signal.indexed
                             ? "[" + std::to_string(signal.left) + ":" +
                                   std::to_string(signal.right) + "]"
                             : "";
      Append("DataOutPort " + entry.first + range + " { Source " + sources +
             "; }\n");
    }
    const auto ir_select = Fresh("ir_select");
    const auto output = Fresh("tdo");
    std::map<std::string, std::string> registers, returns;
    BuildDataPath(input, reg, width, ir_select, output, &registers, &returns);
    Append("}\n");
    Design design;
    design.AddIcl(link_.origin + " (TAP datapath)", text_);
    TapDatapathModel model;
    model.netlist = Elaborate(design, "IclPatternTapDatapath");
    model.instruction_register = model.netlist.node_ids.at(reg);
    model.scan_input = model.netlist.node_ids.at(input);
    model.scan_output = model.netlist.node_ids.at(output);
    model.ir_select = model.netlist.node_ids.at(ir_select);
    for (const auto& entry : registers) {
      model.data_registers.emplace(entry.first,
                                   model.netlist.node_ids.at(entry.second));
    }
    for (const auto& entry : returns) {
      model.scan_returns.emplace(entry.first,
                                 model.netlist.node_ids.at(entry.second));
    }
    for (const auto& decoder : decoders) {
      model.instruction_selects.emplace(
          decoder.first, model.netlist.node_ids.at(decoder.second));
    }
    return model;
  }

 private:
  void ValidateRoutes(const std::map<std::string, std::string>& routes,
                      const std::string& kind = "data-register routes") const {
    struct Route {
      std::string source, instruction;
    };
    std::map<std::string, Route> seen;
    std::vector<decltype(seen)::const_iterator> masked;
    size_t work = kMaxModelBytes;
    const auto spend = [&] {
      if (!work) {
        throw std::runtime_error(link_.bsdl->origin +
                                 ": TAP opcode overlap work limit exceeded");
      }
      --work;
    };
    for (const auto& opcode : link_.bsdl->opcodes) {
      const auto route = routes.find(opcode.first);
      if (route == routes.end()) continue;
      const auto& source = route->second;
      const auto conflict = [&](const Route& prior) {
        throw std::runtime_error(link_.bsdl->origin + ": conflicting " + kind +
                                 " for " + prior.instruction + " and " +
                                 opcode.first);
      };
      for (const auto& code : opcode.second) {
        const auto duplicate = seen.find(code);
        if (duplicate != seen.end()) {
          if (duplicate->second.source != source) conflict(duplicate->second);
          continue;
        }
        const auto compare = [&](const auto& prior) {
          spend();
          if (prior.second.source == source) return;
          for (size_t bit = 0; bit < code.size(); ++bit) {
            spend();
            if (code[bit] != 'x' && prior.first[bit] != 'x' &&
                code[bit] != prior.first[bit]) {
              return;
            }
          }
          conflict(prior.second);
        };
        const bool mask = code.find('x') != std::string::npos;
        if (mask) {
          for (const auto& prior : seen) compare(prior);
        } else {
          for (const auto& prior : masked) compare(*prior);
        }
        const auto entry =
            seen.emplace(code, Route{source, opcode.first}).first;
        if (mask) masked.push_back(entry);
      }
    }
  }
  std::string DeviceCapture(const std::string& instruction_reg,
                            const std::string& instruction_width,
                            const std::map<std::string, std::string>& returns) {
    std::map<std::string, std::string> captures;
    for (const auto& opcode : link_.bsdl->opcodes) {
      if (returns.count(opcode.first)) continue;
      const auto association = link_.bsdl->registers.find(opcode.first);
      const bool idcode = opcode.first == "IDCODE";
      const bool usercode = opcode.first == "USERCODE";
      if (!idcode && !usercode &&
          (association == link_.bsdl->registers.end() ||
           association->second.name != "DEVICE_ID")) {
        continue;
      }
      std::string capture(32, 'x');
      if (idcode || usercode) {
        const auto& value =
            usercode ? link_.bsdl->usercode : link_.bsdl->idcode;
        if (!value) {
          throw std::runtime_error(
              link_.bsdl->origin + ": missing " +
              (usercode ? "USERCODE_REGISTER" : "IDCODE_REGISTER"));
        }
        capture = *value;
      }
      captures.emplace(opcode.first, std::move(capture));
    }
    ValidateRoutes(captures, "identification capture values");
    const auto& first = captures.begin()->second;
    if (std::all_of(captures.begin(), captures.end(),
                    [&](const auto& entry) { return entry.second == first; })) {
      return "32'b" + first;
    }
    const auto name = Fresh("device_capture");
    Append("DataMux " + name + "[31:0] SelectedBy " + instruction_reg + " {\n");
    for (const auto& entry : captures) {
      bool first = true;
      for (const auto& code : link_.bsdl->opcodes.at(entry.first)) {
        Append((first ? "" : " | ") + instruction_width + "'b" + code);
        first = false;
      }
      Append(" : 32'b" + entry.second + ";\n");
    }
    Append("}\n");
    return name;
  }
  void BuildDataPath(const std::string& input,
                     const std::string& instruction_reg,
                     const std::string& instruction_width,
                     const std::string& ir_select, const std::string& output,
                     std::map<std::string, std::string>* registers,
                     std::map<std::string, std::string>* returns) {
    std::map<std::vector<std::pair<std::string, std::string>>, std::string>
        chains;
    for (const auto& instruction : link_.instructions) {
      if (instruction.interfaces.empty()) continue;
      std::vector<std::pair<std::string, std::string>> chain;
      for (const auto& interface : instruction.interfaces) {
        chain.emplace_back(interface.instance, interface.name);
      }
      auto found = chains.find(chain);
      if (found == chains.end()) {
        const auto name = Fresh("scan_return");
        Append("ScanInPort " + name + ";\n");
        found = chains.emplace(std::move(chain), name).first;
      }
      returns->emplace(instruction.name, found->second);
    }
    std::map<std::string, size_t> widths;
    std::map<std::string, std::string> routes;
    size_t remaining_bits = kMaxActiveBits;
    for (const auto& opcode : link_.bsdl->opcodes) {
      // IEEE 1149.1-2001 B.8.13.3(a): these associations may be omitted,
      // but explicit ones must agree with the standard instruction.
      static const std::map<std::string, std::string> kStandardRegisters = {
          {"BYPASS", "BYPASS"},     {"CLAMP", "BYPASS"},
          {"HIGHZ", "BYPASS"},      {"EXTEST", "BOUNDARY"},
          {"INTEST", "BOUNDARY"},   {"PRELOAD", "BOUNDARY"},
          {"SAMPLE", "BOUNDARY"},   {"IDCODE", "DEVICE_ID"},
          {"USERCODE", "DEVICE_ID"}};
      const auto standard = kStandardRegisters.find(opcode.first);
      const auto association = link_.bsdl->registers.find(opcode.first);
      if (standard != kStandardRegisters.end() &&
          association != link_.bsdl->registers.end()) {
        if (association->second.name != standard->second) {
          throw std::runtime_error(
              link_.bsdl->origin + ": instruction " + opcode.first +
              " requires mandated register " + standard->second);
        }
        const auto expected_width =
            standard->second == "BOUNDARY"
                ? link_.bsdl->boundary_width
                : std::optional<size_t>(standard->second == "BYPASS" ? 1 : 32);
        if (association->second.width &&
            association->second.width != expected_width) {
          throw std::runtime_error(link_.bsdl->origin +
                                   ": inconsistent width for BSDL register " +
                                   standard->second);
        }
      }
      const auto returned = returns->find(opcode.first);
      if (returned != returns->end()) {
        routes.emplace(opcode.first, returned->second);
        continue;
      }
      if (association == link_.bsdl->registers.end() &&
          standard == kStandardRegisters.end()) {
        throw std::runtime_error(
            link_.bsdl->origin +
            ": no data-register association for instruction " + opcode.first);
      }
      const auto reg = association == link_.bsdl->registers.end()
                           ? BsdlRegister{standard->second, std::nullopt}
                           : association->second;
      auto width = reg.width;
      std::string capture;
      if (reg.name == "BYPASS") {
        width = 1;
        capture = "1'b0";
      } else if (reg.name == "BOUNDARY") {
        width = link_.bsdl->boundary_width;
      } else if (reg.name == "DEVICE_ID") {
        width = 32;
      }
      if (!width || !*width || (reg.width && reg.width != width)) {
        throw std::runtime_error(
            link_.bsdl->origin +
            ": missing or inconsistent width for BSDL register " + reg.name);
      }
      auto found = registers->find(reg.name);
      if (found == registers->end()) {
        if (*width > remaining_bits) {
          throw std::runtime_error(
              link_.bsdl->origin +
              ": TAP data-register width limit exceeded: " + reg.name);
        }
        remaining_bits -= *width;
        if (reg.name == "DEVICE_ID") {
          capture = DeviceCapture(instruction_reg, instruction_width, *returns);
        }
        const auto name = Fresh(reg.name);
        Append("ScanRegister " + name + "[" + std::to_string(*width - 1) +
               ":0] { ScanInSource " + input + ";");
        if (!capture.empty()) {
          Append(" CaptureSource " + capture + ";");
        }
        Append(" }\n");
        found = registers->emplace(reg.name, name).first;
        widths.emplace(reg.name, *width);
      } else if (widths.at(reg.name) != *width) {
        throw std::runtime_error(link_.bsdl->origin +
                                 ": inconsistent width for BSDL register " +
                                 reg.name);
      }
      routes.emplace(opcode.first, found->second + "[0]");
    }
    ValidateRoutes(routes);
    const auto bypass = registers->find("BYPASS");
    if (bypass == registers->end()) {
      throw std::runtime_error(
          link_.bsdl->origin +
          ": missing BYPASS register for unassigned opcodes");
    }
    std::set<std::string> known_codes;
    for (const auto& opcode : link_.bsdl->opcodes) {
      known_codes.insert(opcode.second.begin(), opcode.second.end());
    }
    std::vector<std::string> known_terms;
    for (const auto& code : known_codes) {
      known_terms.push_back(instruction_reg + " == " + instruction_width +
                            "'b" + code);
    }
    const auto known = Fresh("known_instruction");
    Append("LogicSignal " + known + " { " + Or(std::move(known_terms)) +
           "; }\n");
    const auto data_mux = Fresh("data_mux");
    const auto fallback = Fresh("unassigned_mux");
    const auto ir_mux = Fresh("ir_mux");
    Append("ScanMux " + data_mux + " SelectedBy " + instruction_reg + " {\n");
    for (const auto& opcode : link_.bsdl->opcodes) {
      bool first = true;
      for (const auto& code : opcode.second) {
        Append((first ? "" : " | ") + instruction_width + "'b" + code);
        first = false;
      }
      Append(" : " + routes.at(opcode.first) + ";\n");
    }
    // IEEE 1149.1-2001 8.1.1(d). Decode coverage instead of enumerating the
    // exponentially large complement of literal/masked opcode patterns.
    Append("}\nScanMux " + fallback + " SelectedBy " + known + " { 1'b0 : " +
           bypass->second + "[0]; 1'b1 : " + data_mux + "; }\nDataInPort " +
           ir_select + ";\nScanMux " + ir_mux + " SelectedBy " + ir_select +
           " { 1'b1 : " + instruction_reg + "[0]; 1'b0 : " + fallback +
           "; }\nScanOutPort " + output + " { Source " + ir_mux + "; }\n");
  }
  static std::string BaseName(const std::string& text) {
    return text.substr(0, text.find('['));
  }
  void ReadSignalRanges() {
    // Batch range/parameter evaluation through the shared frontend, not a new
    // ActiveSignal expression parser. Temporary ports carry only their shape.
    Append("Module IclPatternSignalShapes {\n");
    std::map<std::string, std::string> origins;
    if (link_.module) {
      for (const auto* node : link_.module->ast->body()) {
        const auto* statement = dynamic_cast<const icl::AstStatement*>(node);
        if (!statement ||
            (statement->named_kind() != icl::NamedKind::kParameter &&
             statement->named_kind() != icl::NamedKind::kLocalParameter)) {
          continue;
        }
        names_.insert(statement->name());
        const auto range = link_.module->document->Range(node->span());
        Append(link_.module->document->source().substr(
                   range.begin, range.end - range.begin) +
               "\n");
      }
    }
    std::map<std::string, std::string> shape_names;
    for (const auto& instruction : link_.instructions) {
      for (const auto& signal : instruction.active_signals) {
        const auto base = BaseName(signal.text);
        if (!Identifier(base)) {
          throw std::runtime_error(signal.origin +
                                   ": invalid ActiveSignal name");
        }
        names_.insert(base);
        if (shape_names.count(signal.text)) continue;
        const auto name = Fresh("active_range");
        shape_names.emplace(signal.text, name);
        origins.emplace(name, signal.origin);
        Append("DataInPort " + name + signal.text.substr(base.size()) + ";\n");
      }
    }
    Append("}\n");
    Design design;
    design.AddIcl(link_.origin + " (ActiveSignal ranges)", text_);
    Netlist shapes;
    try {
      shapes = Elaborate(design, "IclPatternSignalShapes");
    } catch (const std::runtime_error& error) {
      std::string message = error.what();
      const auto& module = design.SelectTop("IclPatternSignalShapes");
      for (const auto& declaration : module.document->declarations()) {
        const auto origin = origins.find(declaration.name);
        if (origin == origins.end()) continue;
        const auto generated =
            module.Location(declaration.name_range.begin) + ":";
        const auto at = message.find(generated);
        if (at != std::string::npos) {
          message.replace(at, generated.size(), origin->second + ":");
          break;
        }
      }
      throw std::runtime_error(message);
    }
    size_t remaining_bits = kMaxActiveBits;
    for (const auto& instruction : link_.instructions) {
      for (const auto& signal : instruction.active_signals) {
        if (ranges_.count(signal.text)) continue;
        const auto& node = shapes.NodeAt(shape_names.at(signal.text));
        ranges_.emplace(signal.text,
                        SignalRange{node.indexed, node.left, node.right});
        const auto base = BaseName(signal.text);
        auto found = signals_.find(base);
        if (found == signals_.end()) {
          Signal output;
          output.indexed = node.indexed;
          output.left = node.left;
          output.right = node.right;
          signals_.emplace(base, std::move(output));
        } else {
          auto& output = found->second;
          if (output.indexed != node.indexed) {
            throw std::runtime_error(
                signal.origin +
                ": scalar/indexed ActiveSignal conflict: " + base);
          }
          const auto lower =
              std::min({output.left, output.right, node.left, node.right});
          const auto upper =
              std::max({output.left, output.right, node.left, node.right});
          const bool descending = output.left >= output.right;
          output.left = descending ? upper : lower;
          output.right = descending ? lower : upper;
        }
      }
    }
    for (auto& entry : signals_) {
      auto& signal = entry.second;
      const auto width =
          static_cast<size_t>(std::max(signal.left, signal.right) -
                              std::min(signal.left, signal.right)) +
          1;
      if (width > remaining_bits) {
        throw std::runtime_error("ActiveSignal width limit exceeded: " +
                                 entry.first);
      }
      remaining_bits -= width;
      signal.instructions.resize(width);
    }
    text_.clear();
  }
  std::string Fresh(const std::string& base) {
    auto name = base;
    while (!names_.insert(name).second) {
      name = base + "_" + std::to_string(next_name_++);
    }
    return name;
  }
  void Append(const std::string& text) {
    if (text.size() > kMaxModelBytes - text_.size()) {
      throw std::runtime_error("TAP instruction model text limit exceeded");
    }
    text_ += text;
  }
  static std::string Or(std::vector<std::string> terms) {
    if (terms.empty()) throw std::runtime_error("instruction has no opcode");
    while (terms.size() > 1) {
      std::vector<std::string> merged;
      for (size_t i = 0; i < terms.size(); i += 2) {
        if (i + 1 == terms.size()) {
          merged.push_back(std::move(terms[i]));
        } else {
          merged.push_back("(" + terms[i] + ") || (" + terms[i + 1] + ")");
        }
      }
      terms = std::move(merged);
    }
    return std::move(terms.front());
  }
  const AccessLink& link_;
  std::set<std::string> names_;
  std::map<std::string, SignalRange> ranges_;
  std::map<std::string, Signal> signals_;
  std::string text_;
  size_t next_name_ = 0;
};
}  // namespace
TapActions TapController::SetTrst(char value) {
  ValidateTapInput(value);
  trst_ = value;
  if (value == '0') states_ = TapStateBit(TapState::kTestLogicReset);
  if (value == 'x') states_ |= TapStateBit(TapState::kTestLogicReset);
  TapActions actions;
  actions.reset_ir = value == '0' ? '1' : value == '1' ? '0' : 'x';
  return actions;
}

TapActions TapController::RisingEdge(char tms) {
  ValidateTapInput(tms);
  using S = TapState;
  TapActions actions;
  actions.capture_ir = ClockAction(states_, S::kCaptureIr, trst_);
  actions.shift_ir = ClockAction(states_, S::kShiftIr, trst_);
  actions.capture_dr = ClockAction(states_, S::kCaptureDr, trst_);
  actions.shift_dr = ClockAction(states_, S::kShiftDr, trst_);
  // IEEE 1149.1-2001 Figure 6-1, indexed in TapState declaration order.
  static constexpr S kNext[][2] = {
      {S::kRunTestIdle, S::kTestLogicReset},
      {S::kRunTestIdle, S::kSelectDr},
      {S::kCaptureDr, S::kSelectIr},
      {S::kShiftDr, S::kExit1Dr},
      {S::kShiftDr, S::kExit1Dr},
      {S::kPauseDr, S::kUpdateDr},
      {S::kPauseDr, S::kExit2Dr},
      {S::kShiftDr, S::kUpdateDr},
      {S::kRunTestIdle, S::kSelectDr},
      {S::kCaptureIr, S::kTestLogicReset},
      {S::kShiftIr, S::kExit1Ir},
      {S::kShiftIr, S::kExit1Ir},
      {S::kPauseIr, S::kUpdateIr},
      {S::kPauseIr, S::kExit2Ir},
      {S::kShiftIr, S::kUpdateIr},
      {S::kRunTestIdle, S::kSelectDr},
  };
  uint16_t next = 0;
  for (unsigned state = 0; state < 16; ++state) {
    if (!(states_ & (uint16_t{1} << state))) continue;
    if (tms != '1') next |= TapStateBit(kNext[state][0]);
    if (tms != '0') next |= TapStateBit(kNext[state][1]);
  }
  states_ = next;
  actions.reset_ir = SetTrst(trst_).reset_ir;
  return actions;
}

TapActions TapController::FallingEdge() {
  using S = TapState;
  TapActions actions;
  actions.update_ir = ClockAction(states_, S::kUpdateIr, trst_);
  actions.update_dr = ClockAction(states_, S::kUpdateDr, trst_);
  actions.reset_ir = StateActive(states_, TapStateBit(S::kTestLogicReset));
  tdo_enabled_ =
      StateActive(states_, TapStateBit(S::kShiftIr) | TapStateBit(S::kShiftDr));
  ir_select_ = states_ == TapStateBit(S::kShiftIr)   ? '1'
               : states_ == TapStateBit(S::kShiftDr) ? '0'
                                                     : 'x';
  return actions;
}

TapDatapathModel BuildTapDatapathModel(const AccessLink& link) {
  try {
    return Builder(link).Run();
  } catch (const std::exception& error) {
    throw std::runtime_error(link.origin + ": AccessLink " + link.name + ": " +
                             error.what());
  }
}
}  // namespace pattern
