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

bool InputPort(const std::string& kind) {
  return kind == "ScanInPort" || kind == "DataInPort" || kind == "ResetPort" ||
         kind == "TRSTPort" || kind == "TMSPort" || kind == "SelectPort" ||
         kind == "ShiftEnPort" || kind == "CaptureEnPort" ||
         kind == "UpdateEnPort" || kind == "TCKPort" || kind == "ClockPort" ||
         kind == "AddressPort" || kind == "ReadEnPort" || kind == "WriteEnPort";
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
  Json::Elements scopes, nodes, modules;
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
  for (const auto& node : netlist.nodes) {
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
         Json::Bool(InputPort(node.kind) && node.scope != 0 &&
                    !node.inputs.count("Source")),
         node.reset ? Reference(netlist, *node.reset) : Json::Null(),
         Json::Array(std::move(special)), Json::Bool(node.active_low)}));
  }
  return Json::Object({{"version", Json::Int(1)},
                       {"modules", Json::Array(std::move(modules))},
                       {"scopes", Json::Array(std::move(scopes))},
                       {"nodes", Json::Array(std::move(nodes))}});
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
  const bool internal_input = InputPort(node.kind) && node.scope != 0;
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
    if (node.has_addressed_contributors) {
      throw std::runtime_error(
          node.origin +
          ": addressed one-hot group serialization is not implemented");
    }
    for (const auto& source : node.one_hot_sources) {
      result += "Port " + Signals(netlist, source) + ";\n";
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
  for (const auto& node : netlist.nodes) text += Declaration(netlist, node);
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
    const bool internal_input = scope != 0 && InputPort(kind);
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
  return true;
}
}  // namespace pattern
