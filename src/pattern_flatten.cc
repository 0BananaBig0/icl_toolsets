// Copyright 2026 The ICL Toolsets Authors.
// SPDX-License-Identifier: MIT
#include "pattern_flatten.h"

#include <algorithm>
#include <charconv>
#include <set>
#include <stdexcept>
#include <utility>

#include "format_job.h"
#include "json.h"

namespace pattern {
namespace {
using icl::Json;
constexpr const char* kOrigins = kFlatOriginAttribute;
constexpr size_t kMissing = std::numeric_limits<size_t>::max();
constexpr size_t kMaxMetadataBytes = 16 * 1024 * 1024;

std::string Quote(const std::string& text) {
  std::string result = "\"";
  for (char c : text) {
    if (c == '\\' || c == '"') result += '\\';
    result += c;
  }
  return result + '"';
}

std::string FlatName(const std::string& name) {
  constexpr char kHex[] = "0123456789abcdef";
  std::string result = "n";
  for (unsigned char c : name) {
    result += kHex[c >> 4];
    result += kHex[c & 15];
  }
  return result;
}

std::string Literal(const Constant& value) {
  if (value.is_string) return Quote(value.data);
  return (value.sized ? std::to_string(value.data.size()) : "") + "'b" +
         value.data;
}

std::string Range(const NetlistNode& node) {
  return node.indexed ? "[" + std::to_string(node.left) + ":" +
                            std::to_string(node.right) + "]"
                      : "";
}

std::string Signals(const Netlist& netlist,
                    const std::vector<BitReference>& bits) {
  std::string result;
  for (size_t i = 0; i < bits.size();) {
    if (i) result += ", ";
    const auto& bit = bits[i];
    if (bit.node == kMissing) {
      char c = bit.constant;
      if (c == '?' || bit.virtual_signal != VirtualSignal::kNone) c = 'x';
      if (bit.inverted) c = c == '0' ? '1' : c == '1' ? '0' : 'x';
      result += std::string("1'b") + c;
      ++i;
      continue;
    }
    const auto& node = netlist.nodes.at(bit.node);
    if (bit.inverted) result += '~';
    result += FlatName(node.name);
    size_t end = i + 1;
    int direction = 0;
    if (end < bits.size() && bits[end].node == bit.node &&
        bits[end].inverted == bit.inverted) {
      if (bits[end].index == bit.index - 1) direction = -1;
      if (bit.index != std::numeric_limits<int64_t>::max() &&
          bits[end].index == bit.index + 1) {
        direction = 1;
      }
    }
    if (node.indexed && direction) {
      while (end < bits.size() && bits[end].node == bit.node &&
             bits[end].inverted == bit.inverted &&
             bits[end].index - bits[end - 1].index == direction) {
        ++end;
      }
    }
    if (node.indexed) {
      result += "[" + std::to_string(bit.index);
      if (end > i + 1) result += ":" + std::to_string(bits[end - 1].index);
      result += "]";
    }
    i = end;
  }
  return result;
}

void Logic(const Netlist& netlist, const NetlistNode& node, size_t index,
           std::string* result) {
  const auto& term = node.logic.at(index);
  if (term.operation == LogicOperation::kLiteral) {
    *result += Literal(term.literal);
    return;
  }
  if (term.operation == LogicOperation::kSignal) {
    *result += Signals(netlist, term.signals);
    return;
  }
  const bool unary = term.operation == LogicOperation::kInvert ||
                     term.operation == LogicOperation::kNot ||
                     term.operation == LogicOperation::kReduceAnd ||
                     term.operation == LogicOperation::kReduceOr ||
                     term.operation == LogicOperation::kReduceXor;
  const std::string op = term.operation == LogicOperation::kInvert ? "~"
                         : term.operation == LogicOperation::kNot  ? "!"
                         : term.operation == LogicOperation::kAnd ||
                                 term.operation == LogicOperation::kReduceAnd
                             ? "&"
                         : term.operation == LogicOperation::kOr ||
                                 term.operation == LogicOperation::kReduceOr
                             ? "|"
                         : term.operation == LogicOperation::kXor ||
                                 term.operation == LogicOperation::kReduceXor
                             ? "^"
                         : term.operation == LogicOperation::kEqual    ? "=="
                         : term.operation == LogicOperation::kNotEqual ? "!="
                                                                       : ",";
  if (unary) *result += op;
  *result += "(";
  Logic(netlist, node, term.left, result);
  if (!unary) {
    *result += ") " + op + " (";
    Logic(netlist, node, term.right, result);
  }
  *result += ")";
}

Json Value(const Constant& value) {
  return Json::Array({Json::Bool(value.is_string), Json::Bool(value.sized),
                      Json::String(value.data)});
}

Json Reference(const Netlist& netlist, const BitReference& bit) {
  return Json::Array(
      {Json::String(bit.node == kMissing
                        ? ""
                        : FlatName(netlist.nodes.at(bit.node).name)),
       Json::String(std::to_string(bit.index)), Json::Bool(bit.inverted),
       Json::Int(static_cast<int>(bit.virtual_signal))});
}

Json AttributeValues(const Attributes& attributes) {
  Json::Elements result;
  for (const auto& item : attributes) {
    result.push_back(
        Json::Array({Json::String(item.first),
                     item.second ? Value(*item.second) : Json::Null()}));
  }
  return Json::Array(std::move(result));
}

Json Metadata(const Netlist& netlist) {
  Json::Elements scopes, nodes, modules, contributors;
  for (const auto& module : netlist.module_parameters) {
    Json::Elements parameters;
    for (const auto& parameter : module.second) {
      parameters.push_back(Json::String(parameter));
    }
    modules.push_back(Json::Array(
        {Json::String(module.first), Json::Array(std::move(parameters))}));
  }
  for (const auto& scope : netlist.scopes) {
    Json::Elements parameters;
    for (const auto& item : scope.parameters) {
      parameters.push_back(
          Json::Array({Json::String(item.first), Value(item.second)}));
    }
    Json::Elements fields{
        Json::String(scope.path), Json::String(scope.definition.name()),
        Json::Array(std::move(parameters)), AttributeValues(scope.attributes)};
    if (!scope.instance_attributes.empty() || scope.address_value) {
      fields.push_back(AttributeValues(scope.instance_attributes));
      fields.push_back(scope.address_value ? Value(*scope.address_value)
                                           : Json::Null());
    }
    scopes.push_back(Json::Array(std::move(fields)));
  }
  // Nested registers can change node IDs when width-sorted groups are parsed.
  for (const auto& entry : netlist.node_ids) {
    const auto& node = netlist.nodes.at(entry.second);
    for (size_t i = 0; i < node.one_hot_sources.size(); ++i) {
      const auto& source = node.one_hot_sources[i];
      if (source.kind == OneHotSourceKind::kInstance) {
        contributors.push_back(
            Json::Array({Json::String(node.name), Json::Int(i),
                         Json::String(netlist.scopes.at(source.object).path)}));
      }
    }
    Json::Elements special;
    for (const auto& input : node.inputs) {
      if (input.first == "LogicDependencies") continue;
      for (size_t i = 0; i < input.second.size(); ++i) {
        const auto& bit = input.second[i];
        if (bit.constant != '?' && bit.virtual_signal == VirtualSignal::kNone) {
          continue;
        }
        special.push_back(Json::Array(
            {Json::String(input.first), Json::Int(i),
             Json::Int(static_cast<int>(bit.virtual_signal)),
             Json::Bool(bit.constant == '?'), Json::Bool(bit.inverted)}));
      }
    }
    nodes.push_back(Json::Array(
        {Json::String(FlatName(node.name)), Json::String(node.name),
         Json::Int(node.scope), Json::String(node.kind),
         Json::Bool(IsInputPort(node.kind) && node.scope != 0 &&
                    !node.inputs.count("Source")),
         node.reset ? Reference(netlist, *node.reset) : Json::Null(),
         Json::Array(std::move(special)), Json::Bool(node.active_low)}));
  }
  Json::Members fields{{"version", Json::Int(1)},
                       {"modules", Json::Array(std::move(modules))},
                       {"scopes", Json::Array(std::move(scopes))},
                       {"nodes", Json::Array(std::move(nodes))}};
  if (!contributors.empty()) {
    fields.emplace_back("instance_contributors",
                        Json::Array(std::move(contributors)));
  }
  if (!netlist.taps.empty()) {
    Json::Elements taps;
    const auto name = [&](size_t id) {
      return Json::String(netlist.nodes.at(id).name);
    };
    const auto mapping = [&](const std::map<std::string, size_t>& nodes) {
      Json::Elements entries;
      for (const auto& [key, id] : nodes) {
        entries.push_back(Json::Array({Json::String(key), name(id)}));
      }
      return Json::Array(std::move(entries));
    };
    for (const auto& [path, tap] : netlist.taps) {
      taps.push_back(Json::Array(
          {Json::String(path), Json::String(tap.bsdl.source_path),
           Json::String(tap.bsdl.entity), Json::String(tap.bsdl.source),
           Json::Array({name(tap.instruction_register), name(tap.scan_input),
                        name(tap.scan_output), name(tap.ir_select)}),
           mapping(tap.instruction_selects), mapping(tap.data_registers),
           mapping(tap.scan_returns), mapping(tap.controls)}));
    }
    fields.emplace_back("taps", Json::Array(std::move(taps)));
  }
  return Json::Object(std::move(fields));
}

std::string RenderAttributes(const Attributes& attributes) {
  std::string result;
  for (const auto& item : attributes) {
    result += "Attribute " + item.first;
    if (item.second) result += " = " + Literal(*item.second);
    result += ";\n";
  }
  return result;
}

std::string Declaration(const Netlist& netlist, const NetlistNode& node) {
  if ((node.model == ModelKind::kGroup && node.kind != "OneHotScanGroup" &&
       node.kind != "OneHotDataGroup") ||
      node.model == ModelKind::kAccessLink) {
    throw std::runtime_error(node.origin +
                             ": flat serialization is not implemented for " +
                             node.kind);
  }
  const bool internal_input = IsInputPort(node.kind) && node.scope != 0;
  std::string result = (internal_input ? "DataOutPort" : node.kind) + " " +
                       FlatName(node.name) + Range(node);
  if (node.model == ModelKind::kAlias) {
    const auto source = node.inputs.find("Source");
    result += " = " + (source == node.inputs.end()
                           ? std::to_string(node.Width()) + "'b" +
                                 std::string(node.Width(), 'x')
                           : Signals(netlist, source->second));
  }
  if (node.model == ModelKind::kMux) {
    result += " SelectedBy " + Signals(netlist, node.inputs.at("SelectedBy"));
  }
  result += " {\n" + RenderAttributes(node.attributes);
  if (internal_input) {
    const auto source = node.inputs.find("Source");
    result +=
        "Source " +
        (source == node.inputs.end() ? std::to_string(node.Width()) + "'b" +
                                           std::string(node.Width(), 'x')
                                     : Signals(netlist, source->second)) +
        ";\n";
  }
  if (node.kind == "OneHotScanGroup" || node.kind == "OneHotDataGroup") {
    for (const auto& source : node.one_hot_sources) {
      if (source.kind == OneHotSourceKind::kRegister) {
        result += Declaration(netlist, netlist.nodes.at(source.object));
      } else {
        if (source.bits.empty()) {
          throw std::runtime_error(node.origin +
                                   ": addressed one-hot instance without "
                                   "DataOutPorts cannot be serialized");
        }
        result += "Port " + Signals(netlist, source.bits) + ";\n";
      }
    }
  } else if (node.scan_interface) {
    const auto append_ports = [&](const std::vector<BitReference>& ports,
                                  const std::optional<Constant>& load) {
      for (const auto& port : ports) {
        result += "Port " + Signals(netlist, {port}) + ";\n";
      }
      if (load) result += "DefaultLoadValue " + Literal(*load) + ";\n";
    };
    append_ports(node.scan_interface->ports, node.scan_interface->default_load);
    for (const auto& chain : node.scan_interface->chains) {
      result +=
          "Chain " + chain.name + " {\n" + RenderAttributes(chain.attributes);
      append_ports(chain.ports, chain.default_load);
      result += "}\n";
    }
  } else if (node.model == ModelKind::kLogic) {
    result += "(";
    Logic(netlist, node, node.logic.size() - 1, &result);
    result += ");\n";
  } else if (node.model == ModelKind::kMux) {
    for (const auto& choice : node.choices) {
      for (size_t i = 0; i < choice.codes.size(); ++i) {
        if (i) result += " | ";
        result += Literal(choice.codes[i]);
      }
      result += ": " + Signals(netlist, choice.source) + ";\n";
    }
  } else if (node.model == ModelKind::kEnum) {
    for (const auto& value : node.constants) {
      result += value.first + " = " + Literal(value.second) + ";\n";
    }
  } else {
    for (const auto& property : node.properties) {
      const auto& name = property.first;
      if (name == "Source" &&
          (internal_input || node.model == ModelKind::kAlias)) {
        continue;
      }
      if (name == "ActivePolarity" && internal_input) continue;
      const auto bits = node.inputs.find(name);
      const auto value = node.constants.find(name);
      if (bits != node.inputs.end()) {
        result += name + " " + Signals(netlist, bits->second) + ";\n";
      } else if (value != node.constants.end()) {
        result += name + " " + Literal(value->second) + ";\n";
      } else if (name == "RefEnum" && node.enumeration) {
        result += "RefEnum " + FlatName(netlist.nodes[*node.enumeration].name) +
                  ";\n";
      } else if (name == "ActivePolarity") {
        result += "ActivePolarity " +
                  std::to_string(netlist.Integer(property.second)) + ";\n";
      } else {
        throw std::runtime_error(node.origin +
                                 ": flat property is not implemented: " + name);
      }
    }
    // Inferred reset forwarding does not have an explicit AST property.
    if (!internal_input && node.model != ModelKind::kAlias &&
        node.inputs.count("Source") && !node.properties.count("Source")) {
      result += "Source " + Signals(netlist, node.inputs.at("Source")) + ";\n";
    }
  }
  return result + "}\n";
}

const Json::Elements& Array(const Json& json, size_t count = kMissing) {
  if (json.type() != Json::Type::kArray ||
      (count != kMissing && json.elements().size() != count)) {
    throw std::runtime_error("invalid flat origin array");
  }
  return json.elements();
}
std::string String(const Json& json) {
  if (json.type() != Json::Type::kString) {
    throw std::runtime_error("invalid flat origin string");
  }
  return json.string_value();
}
bool Boolean(const Json& json) {
  if (json.type() != Json::Type::kBool) {
    throw std::runtime_error("invalid flat origin boolean");
  }
  return json.bool_value();
}
size_t Index(const Json& json, size_t maximum) {
  const auto value = json.int_value(-1);
  if (json.type() != Json::Type::kNumber || value < 0 ||
      static_cast<uint64_t>(value) >= maximum ||
      json.Serialize() != std::to_string(value)) {
    throw std::runtime_error("invalid flat origin index");
  }
  return static_cast<size_t>(value);
}
bool Identifier(const std::string& text) {
  const auto letter = [](char c) {
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
  };
  if (text.empty() || !letter(text[0])) return false;
  return std::all_of(text.begin(), text.end(), [&](char c) {
    return letter(c) || (c >= '0' && c <= '9') || c == '_';
  });
}
std::string ModuleText(const std::string& name) {
  const auto colon = name.find("::");
  if (colon == std::string::npos) {
    if (!Identifier(name)) throw std::runtime_error("invalid flat module name");
    return "Module " + name + " {}";
  }
  const auto ns = name.substr(0, colon), module = name.substr(colon + 2);
  if (!Identifier(ns) || !Identifier(module)) {
    throw std::runtime_error("invalid flat qualified module name");
  }
  return "NameSpace " + ns + "; Module " + module + " {}";
}
Constant ReadValue(const Json& json) {
  const auto& fields = Array(json, 3);
  Constant result{Boolean(fields[0]), Boolean(fields[1]), String(fields[2])};
  if (result.data.size() > 1024 * 1024 ||
      (!result.is_string &&
       (result.data.empty() ||
        result.data.find_first_not_of("01x") != std::string::npos))) {
    throw std::runtime_error("invalid flat parameter value");
  }
  return result;
}

Attributes ReadAttributes(const Json& json) {
  Attributes result;
  for (const auto& attribute : Array(json)) {
    const auto& parts = Array(attribute, 2);
    const auto name = String(parts[0]);
    if (!Identifier(name) || name == kOrigins ||
        !result
             .emplace(name, parts[1].is_null()
                                ? std::nullopt
                                : std::optional<Constant>(ReadValue(parts[1])))
             .second) {
      throw std::runtime_error("invalid flat scope attribute");
    }
  }
  return result;
}
BitReference ReadReference(const Netlist& netlist, const Json& json) {
  const auto& fields = Array(json, 4);
  BitReference bit;
  const auto name = String(fields[0]);
  if (!name.empty()) bit.node = netlist.node_ids.at(name);
  const auto index = String(fields[1]);
  const auto parsed =
      std::from_chars(index.data(), index.data() + index.size(), bit.index);
  if (parsed.ec != std::errc{} || parsed.ptr != index.data() + index.size() ||
      bit.index < 0) {
    throw std::runtime_error("invalid flat signal index");
  }
  bit.inverted = Boolean(fields[2]);
  bit.virtual_signal = static_cast<VirtualSignal>(Index(fields[3], 3));
  if (bit.node == kMissing) {
    if (bit.virtual_signal == VirtualSignal::kNone || bit.index != 0) {
      throw std::runtime_error("invalid flat reset source");
    }
  } else {
    const auto& source = netlist.nodes[bit.node];
    if (source.model == ModelKind::kEnum ||
        source.model == ModelKind::kInterface ||
        source.model == ModelKind::kGroup ||
        source.model == ModelKind::kAccessLink ||
        bit.virtual_signal != VirtualSignal::kNone ||
        bit.index < std::min(source.left, source.right) ||
        bit.index > std::max(source.left, source.right)) {
      throw std::runtime_error("invalid flat reset reference");
    }
  }
  return bit;
}
void RestoreTaps(Netlist* netlist, const Json& json) {
  const auto* entries = json.Find("taps");
  if (!entries) {
    return;
  }
  static const std::map<std::string, std::string> kPinKinds = {
      {"tap_scan_in", "ScanInPort"},
      {"tap_scan_out", "ScanOutPort"},
      {"tap_scan_mode", "TMSPort"},
      {"tap_scan_clock", "TCKPort"},
      {"tap_scan_reset", "TRSTPort"}};
  std::set<size_t> used;
  for (const auto& entry : Array(*entries)) {
    const auto& fields = Array(entry, 9);
    const auto path = String(fields[0]);
    if (!Identifier(path) || netlist->taps.count(path)) {
      throw std::runtime_error("invalid or duplicate flat TAP identity");
    }
    TapRuntimeModel tap;
    tap.bsdl =
        ReadBsdlTap(String(fields[1]), String(fields[3]), String(fields[2]));
    const auto node_id = [&](const Json& value, const std::string& kind,
                             size_t width) {
      const auto found = netlist->node_ids.find(String(value));
      if (found == netlist->node_ids.end()) {
        throw std::runtime_error("missing flat TAP node");
      }
      const auto& node = netlist->nodes.at(found->second);
      if (node.kind != kind || (width && node.Width() != width) ||
          netlist->scopes.at(node.scope).path != path ||
          !used.insert(found->second).second) {
        throw std::runtime_error("invalid flat TAP node kind, width or scope");
      }
      return found->second;
    };
    const auto& nodes = Array(fields[4], 4);
    tap.instruction_register =
        node_id(nodes[0], "ScanRegister", tap.bsdl.instruction_width);
    tap.scan_input = node_id(nodes[1], "ScanInPort", 1);
    tap.scan_output = node_id(nodes[2], "ScanOutPort", 1);
    tap.ir_select = node_id(nodes[3], "DataInPort", 1);
    const auto& ir = netlist->nodes.at(tap.instruction_register);
    const auto capture = ir.inputs.find("CaptureSource");
    std::string capture_bits;
    if (capture != ir.inputs.end()) {
      for (const auto& bit : capture->second) {
        capture_bits += bit.node == kMissing && !bit.inverted &&
                                bit.virtual_signal == VirtualSignal::kNone
                            ? bit.constant
                            : '?';
      }
    }
    if (capture_bits != tap.bsdl.capture) {
      throw std::runtime_error("flat TAP capture conflicts with BSDL");
    }
    const auto read_map = [&](const Json& value, const std::string& kind,
                              size_t width,
                              std::map<std::string, size_t>* result) {
      // Several instruction aliases may share a data register/return node.
      std::map<std::string, size_t> seen;
      for (const auto& item : Array(value)) {
        const auto& pair = Array(item, 2);
        const auto key = String(pair[0]), name = String(pair[1]);
        if ((kind != "ScanRegister" && !tap.bsdl.opcodes.count(key)) ||
            result->count(key)) {
          throw std::runtime_error("invalid flat TAP instruction mapping");
        }
        auto found = seen.find(name);
        const size_t id =
            found == seen.end() ? node_id(pair[1], kind, width) : found->second;
        seen.emplace(name, id);
        result->emplace(key, id);
      }
    };
    read_map(fields[5], "LogicSignal", 1, &tap.instruction_selects);
    read_map(fields[6], "ScanRegister", 0, &tap.data_registers);
    read_map(fields[7], "ScanInPort", 1, &tap.scan_returns);
    for (const auto& [instruction, id] : tap.scan_returns) {
      if (!tap.instruction_selects.count(instruction)) {
        throw std::runtime_error("missing flat TAP instruction decoder");
      }
    }
    for (const auto& [name, id] : tap.data_registers) {
      std::optional<size_t> width;
      if (name == "BYPASS") {
        width = 1;
      }
      if (name == "BOUNDARY") {
        width = tap.bsdl.boundary_width;
      }
      if (name == "DEVICE_ID") {
        width = 32;
      }
      for (const auto& [instruction, reg] : tap.bsdl.registers) {
        if (reg.name == name && reg.width) {
          if (width && width != reg.width) {
            throw std::runtime_error("conflicting flat TAP register widths");
          }
          width = reg.width;
        }
      }
      if (!width || netlist->nodes.at(id).Width() != *width) {
        throw std::runtime_error("invalid flat TAP data register");
      }
    }
    for (const auto& entry : Array(fields[8], 3)) {
      const auto& pair = Array(entry, 2);
      const auto key = String(pair[0]);
      if ((key != "capture" && key != "shift" && key != "update") ||
          !tap.controls.emplace(key, node_id(pair[1], "DataInPort", 1))
               .second) {
        throw std::runtime_error("invalid flat TAP control boundary");
      }
    }
    for (const auto& [role, name] : tap.bsdl.ports) {
      const auto found = netlist->node_ids.find(name);
      if (found == netlist->node_ids.end()) {
        throw std::runtime_error("missing flat TAP pin");
      }
      const auto& node = netlist->nodes.at(found->second);
      if (node.scope || node.kind != kPinKinds.at(role) || node.Width() != 1) {
        throw std::runtime_error("invalid flat TAP pin kind or width");
      }
      tap.pins.emplace(role, found->second);
    }
    const auto connected = [&](size_t destination, size_t source) {
      const auto& inputs = netlist->nodes.at(destination).inputs;
      const auto found = inputs.find("Source");
      if (found == inputs.end() || found->second.size() != 1) {
        return false;
      }
      const auto& bit = found->second[0];
      return bit.node == source &&
             bit.index == netlist->nodes.at(source).left && !bit.inverted &&
             bit.virtual_signal == VirtualSignal::kNone;
    };
    if (!connected(tap.scan_input, tap.pins.at("tap_scan_in")) ||
        !connected(tap.pins.at("tap_scan_out"), tap.scan_output)) {
      throw std::runtime_error(
          "flat TAP pin connection conflicts with metadata");
    }
    netlist->taps.emplace(path, std::move(tap));
  }
}
}  // namespace

std::string SerializeIcl(const Netlist& netlist) {
  const auto metadata = Metadata(netlist).Serialize();
  if (metadata.size() > kMaxMetadataBytes) {
    throw std::runtime_error("flat origin metadata limit exceeded");
  }
  std::string text = ModuleText(netlist.top_module);
  text.erase(text.size() - 1);
  text +=
      "\nAttribute " + std::string(kOrigins) + " = " + Quote(metadata) + ";\n";
  text += RenderAttributes(netlist.scopes[0].attributes);
  std::set<size_t> nested;
  for (const auto& node : netlist.nodes) {
    for (const auto& source : node.one_hot_sources) {
      if (source.kind == OneHotSourceKind::kRegister &&
          !nested.insert(source.object).second) {
        throw std::runtime_error(
            "DataRegister belongs to multiple one-hot groups");
      }
    }
  }
  for (size_t id = 0; id < netlist.nodes.size(); ++id) {
    if (!nested.count(id)) text += Declaration(netlist, netlist.nodes[id]);
  }
  text += "}\n";
  const auto formatted = icl::FormatDocument(text, {});
  if (!formatted.ok) {
    throw std::runtime_error("invalid serialized ICL: " +
                             formatted.error_message);
  }
  return formatted.text;
}

bool RestoreFlatOrigins(Netlist* netlist) {
  const auto root = netlist->scopes.at(0).definition;
  const icl::AstStatement* metadata = nullptr;
  for (const auto* item : root.ast->body()) {
    const auto* statement = dynamic_cast<const icl::AstStatement*>(item);
    if (statement && statement->named_kind() == icl::NamedKind::kAttribute &&
        statement->name() == kOrigins) {
      metadata = statement;
    }
  }
  if (!metadata) return false;
  if (netlist->scopes.size() != 1) {
    throw std::runtime_error(
        "flat origin metadata requires a flattened module");
  }
  auto expression = netlist->ExpressionAt(0, metadata->span());
  expression.begin += 3;
  --expression.end;
  const auto value = EvaluateConstant(
      root.document->driver().unit().tokens(), expression.begin, expression.end,
      [&](const std::string& name) {
        return netlist->scopes[0].parameters.at(name);
      },
      0, kMaxMetadataBytes);
  if (!value.is_string || value.data.size() > kMaxMetadataBytes) {
    throw std::runtime_error("invalid flat origin metadata value");
  }
  Json json;
  std::string error;
  if (!Json::Parse(value.data, &json, &error)) {
    throw std::runtime_error("invalid flat origin metadata: " + error);
  }
  const auto *version = json.Find("version"), *scope_json = json.Find("scopes"),
             *node_json = json.Find("nodes"), *modules = json.Find("modules");
  if (!version || Index(*version, 2) != 1 || !scope_json || !node_json ||
      !modules) {
    throw std::runtime_error("unsupported flat origin metadata version");
  }
  const auto& scope_data = Array(*scope_json);
  const auto& node_data = Array(*node_json, netlist->nodes.size());
  if (scope_data.empty() || scope_data.size() > 1000000) {
    throw std::runtime_error("invalid flat scope count");
  }
  std::vector<InstanceScope> scopes;
  std::map<std::string, size_t> scope_ids;
  std::vector<size_t> depths;
  std::map<std::string, std::set<std::string>> module_parameters;
  size_t address_bits_remaining = kMaxAddressBits;
  const auto count_address = [&](const Constant& value) {
    if (value.data.size() > address_bits_remaining) {
      throw std::runtime_error("flat address constant bit limit exceeded");
    }
    address_bits_remaining -= value.data.size();
  };
  for (const auto& node : netlist->nodes) {
    const auto address = node.constants.find("AddressValue");
    if (address != node.constants.end()) count_address(address->second);
  }
  for (const auto& entry : Array(*modules)) {
    const auto& fields = Array(entry, 2);
    const auto name = String(fields[0]);
    ModuleText(name);
    if (!module_parameters.emplace(name, std::set<std::string>{}).second) {
      throw std::runtime_error("duplicate flat module binding");
    }
    for (const auto& parameter : Array(fields[1])) {
      const auto parameter_name = String(parameter);
      if (!Identifier(parameter_name) ||
          !module_parameters.at(name).insert(parameter_name).second) {
        throw std::runtime_error("invalid flat module parameter");
      }
    }
  }
  for (const auto& item : scope_data) {
    const auto& fields = Array(item);
    if (fields.size() != 4 && fields.size() != 6) {
      throw std::runtime_error("invalid flat scope fields");
    }
    InstanceScope scope;
    scope.path = String(fields[0]);
    const auto module_name = String(fields[1]);
    const auto module_text = ModuleText(module_name);
    if (!module_parameters.count(module_name)) {
      throw std::runtime_error("missing flat module binding");
    }
    if (!scope_ids.emplace(scope.path, scopes.size()).second) {
      throw std::runtime_error("duplicate flat scope");
    }
    if (scopes.empty()) {
      if (!scope.path.empty() || module_name != netlist->top_module) {
        throw std::runtime_error("invalid flat root scope");
      }
      scope.definition = root;
      depths.push_back(0);
    } else {
      const auto dot = scope.path.rfind('.');
      const auto parent_path =
          dot == std::string::npos ? "" : scope.path.substr(0, dot);
      const auto child =
          dot == std::string::npos ? scope.path : scope.path.substr(dot + 1);
      if (!Identifier(child) || !scope_ids.count(parent_path)) {
        throw std::runtime_error("invalid flat scope path");
      }
      scope.parent = scope_ids.at(parent_path);
      const auto depth = depths.at(scope.parent) + 1;
      if (depth > 128) {
        throw std::runtime_error("flat scope depth limit exceeded");
      }
      depths.push_back(depth);
      scopes.at(scope.parent).children.push_back(scopes.size());
      Design binding;
      binding.AddIcl(root.path, module_text);
      scope.definition = binding.modules().at(module_name);
    }
    for (const auto& parameter : Array(fields[2])) {
      const auto& parts = Array(parameter, 2);
      const auto name = String(parts[0]);
      if (!module_parameters.at(module_name).count(name) ||
          !scope.parameters.emplace(name, ReadValue(parts[1])).second) {
        throw std::runtime_error("invalid flat parameter name");
      }
    }
    if (scope.parameters.size() != module_parameters.at(module_name).size()) {
      throw std::runtime_error("missing flat parameter value");
    }
    scope.attributes = ReadAttributes(fields[3]);
    if (fields.size() == 6) {
      if (scopes.empty()) {
        throw std::runtime_error("flat root cannot have instance properties");
      }
      scope.instance_attributes = ReadAttributes(fields[4]);
      if (!fields[5].is_null()) {
        scope.address_value = ReadValue(fields[5]);
        if (scope.address_value->is_string) {
          throw std::runtime_error(
              "flat instance AddressValue must be numeric");
        }
        count_address(*scope.address_value);
      }
    }
    if (scopes.empty() &&
        AttributeValues(scope.attributes).Serialize() !=
            AttributeValues(netlist->scopes[0].attributes).Serialize()) {
      throw std::runtime_error("flat root attributes conflict with metadata");
    }
    scopes.push_back(std::move(scope));
  }
  std::map<std::string, size_t> names;
  std::set<size_t> seen;
  for (const auto& item : node_data) {
    const auto& fields = Array(item, 8);
    const auto flat_name = String(fields[0]), original_name = String(fields[1]),
               kind = String(fields[3]);
    const size_t scope = Index(fields[2], scopes.size());
    const auto prefix =
        scopes[scope].path.empty() ? "" : scopes[scope].path + ".";
    if (original_name.compare(0, prefix.size(), prefix) != 0 ||
        !Identifier(original_name.substr(prefix.size())) ||
        FlatName(original_name) != flat_name) {
      throw std::runtime_error("invalid flat object identity");
    }
    const size_t id = netlist->node_ids.at(flat_name);
    auto& node = netlist->nodes[id];
    const bool internal_input = scope != 0 && IsInputPort(kind);
    if (!seen.insert(id).second || !names.emplace(original_name, id).second ||
        node.kind != (internal_input ? "DataOutPort" : kind)) {
      throw std::runtime_error("invalid flat object kind or duplicate");
    }
    node.name = original_name;
    node.scope = scope;
    const bool active_low = Boolean(fields[7]);
    if (!internal_input && node.active_low != active_low) {
      throw std::runtime_error("flat polarity conflicts with ICL");
    }
    node.active_low = active_low;
    if (internal_input) {
      node.kind = kind;
      node.model = ModelKind::kPort;
      node.properties.erase("Source");
    }
    if (Boolean(fields[4])) {
      if (!internal_input) {
        throw std::runtime_error("invalid flat unconnected port");
      }
      const auto& bits = node.inputs.at("Source");
      if (std::any_of(bits.begin(), bits.end(), [](const BitReference& bit) {
            return bit.node != kMissing || bit.constant != 'x';
          })) {
        throw std::runtime_error("flat unconnected port has a driver");
      }
      node.inputs.erase("Source");
    }
    node.reset =
        fields[5].is_null()
            ? std::nullopt
            : std::optional<BitReference>(ReadReference(*netlist, fields[5]));
    if (node.reset.has_value() != (node.model == ModelKind::kRegister &&
                                   node.constants.count("ResetValue") != 0)) {
      throw std::runtime_error("flat reset association conflicts with ICL");
    }
    std::set<std::pair<std::string, size_t>> special_bits;
    for (const auto& special : Array(fields[6])) {
      const auto& parts = Array(special, 5);
      const auto property = String(parts[0]);
      auto& input = node.inputs.at(property);
      const auto offset = Index(parts[1], input.size());
      if (!special_bits.emplace(property, offset).second) {
        throw std::runtime_error("duplicate flat virtual connection");
      }
      auto& bit = input.at(offset);
      if (bit.node != kMissing || bit.constant != 'x') {
        throw std::runtime_error("invalid flat virtual connection");
      }
      bit.virtual_signal = static_cast<VirtualSignal>(Index(parts[2], 3));
      const bool missing = Boolean(parts[3]);
      if (missing == (bit.virtual_signal != VirtualSignal::kNone)) {
        throw std::runtime_error("invalid flat special connection kind");
      }
      if (missing) bit.constant = '?';
      bit.inverted = Boolean(parts[4]);
    }
    scopes[scope].members.emplace(original_name.substr(prefix.size()), id);
  }
  netlist->node_ids = std::move(names);
  netlist->scopes = std::move(scopes);
  netlist->module_parameters = std::move(module_parameters);
  if (const auto* contributors = json.Find("instance_contributors")) {
    std::map<size_t, std::vector<size_t>> outputs;
    for (size_t id = 0; id < netlist->nodes.size(); ++id) {
      const auto& node = netlist->nodes[id];
      if (node.kind == "DataOutPort") outputs[node.scope].push_back(id);
    }
    std::set<size_t> used;
    for (const auto& entry : Array(*contributors)) {
      const auto& fields = Array(entry, 3);
      const auto group_id = netlist->node_ids.find(String(fields[0]));
      const auto scope_id = scope_ids.find(String(fields[2]));
      if (group_id == netlist->node_ids.end() || scope_id == scope_ids.end()) {
        throw std::runtime_error("invalid flat instance contributor identity");
      }
      auto& group = netlist->nodes.at(group_id->second);
      const size_t index = Index(fields[1], group.one_hot_sources.size());
      const size_t scope = scope_id->second;
      auto& source = group.one_hot_sources[index];
      if (group.kind != "OneHotDataGroup" ||
          source.kind != OneHotSourceKind::kPort ||
          !used.insert(scope).second ||
          netlist->scopes[scope].parent != group.scope) {
        throw std::runtime_error("invalid flat instance contributor");
      }
      size_t offset = 0;
      for (size_t id : outputs[scope]) {
        const auto& port = netlist->nodes[id];
        for (size_t i = 0; i < port.Width(); ++i) {
          const int64_t bit = port.left + (port.left <= port.right ? 1 : -1) *
                                              static_cast<int64_t>(i);
          if (offset >= source.bits.size() || source.bits[offset].node != id ||
              source.bits[offset].index != bit ||
              source.bits[offset].inverted ||
              source.bits[offset].virtual_signal != VirtualSignal::kNone) {
            throw std::runtime_error(
                "flat instance contributor conflicts with ICL");
          }
          ++offset;
        }
      }
      if (!offset || offset != source.bits.size()) {
        throw std::runtime_error("invalid flat instance contributor width");
      }
      source.kind = OneHotSourceKind::kInstance;
      source.object = scope;
    }
  }
  RestoreTaps(netlist, json);
  return true;
}
}  // namespace pattern
