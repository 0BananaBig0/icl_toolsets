// Copyright 2026 The ICL Toolsets Authors.
// SPDX-License-Identifier: MIT
#include "pattern_netlist.h"

#include <algorithm>
#include <cctype>
#include <functional>
#include <set>
#include <stdexcept>
#include <utility>

#include "pattern_logic.h"

namespace pattern {
namespace {
constexpr size_t kMaxObjects = 1000000;
constexpr size_t kMaxWidth = 1024 * 1024;
constexpr size_t kMaxDepth = 128;

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

std::pair<int64_t, int64_t> SelectRange(const Netlist& netlist,
                                        const NetlistNode& node,
                                        const Expression& selection) {
  if (selection.begin == selection.end) return {node.left, node.right};
  const auto& tokens = SourceTokens(netlist, selection.scope);
  if (!node.indexed || tokens[selection.begin].text != "[" ||
      tokens[selection.end - 1].text != "]") {
    throw std::runtime_error("invalid vector selection on " + node.name);
  }
  size_t colon = selection.begin + 1;
  while (colon < selection.end - 1 && tokens[colon].text != ":") ++colon;
  const int64_t left =
      netlist.Integer({selection.scope, selection.begin + 1, colon});
  const int64_t right =
      colon < selection.end - 1
          ? netlist.Integer({selection.scope, colon + 1, selection.end - 1})
          : left;
  const auto lower = std::min(node.left, node.right),
             upper = std::max(node.left, node.right);
  if (left < lower || right < lower || left > upper || right > upper) {
    throw std::runtime_error("signal index out of range for " + node.name);
  }
  return {left, right};
}

class Elaborator {
 public:
  explicit Elaborator(Design& design) : design_(design) {}

  Netlist Run(const std::string& top) {
    const auto& module = design_.SelectTop(top);
    netlist_.top_module = module.name();
    AddScope(module, "", std::numeric_limits<size_t>::max(), {}, 0);
    Connect();
    InferResets();
    CheckCombinationalCycles();
    return std::move(netlist_);
  }

 private:
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

  void AddScope(const ModuleDefinition& module, const std::string& path,
                size_t parent, const Parameters& overrides, size_t depth) {
    if (depth > kMaxDepth || netlist_.scopes.size() >= kMaxObjects) {
      throw std::runtime_error("ICL instance expansion limit exceeded");
    }
    const auto& description = Describe(module);
    const size_t scope_id = netlist_.scopes.size();
    netlist_.scopes.push_back({path, module, parent, {}, {}, {}});
    if (parent != std::numeric_limits<size_t>::max()) {
      netlist_.scopes[parent].children.push_back(scope_id);
    }
    try {
      ResolveParameters(scope_id, description, overrides);
      for (size_t index : description.declarations) {
        const auto& declaration = module.document->declarations()[index];
        if (declaration.kind == "Instance" || declaration.kind == "Parameter" ||
            declaration.kind == "LocalParameter" ||
            declaration.kind == "EnumMember" || declaration.kind == "Chain") {
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
          if (const auto* block = dynamic_cast<const icl::AstBlock*>(ast)) {
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
          if (!netlist_.node_ids.emplace(node.name, node_id).second) {
            throw std::runtime_error("duplicate object " + node.name);
          }
          netlist_.scopes[scope_id].members.emplace(declaration.name, node_id);
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
          for (const auto* item : block->body()) {
            if (dynamic_cast<const icl::AstComment*>(item)) continue;
            auto expression = Tokens(module, *item, scope_id);
            const auto& tokens = SourceTokens(netlist_, scope_id);
            if (tokens[expression.begin].text != "InputPort") continue;
            const auto& port_name = tokens[expression.begin + 1].text;
            const auto found =
                netlist_.scopes[child_scope].members.find(port_name);
            if (found == netlist_.scopes[child_scope].members.end()) {
              throw std::runtime_error("unknown instance input port " +
                                       port_name);
            }
            const auto& port = netlist_.nodes[found->second];
            if (port.model != ModelKind::kPort ||
                port.kind.rfind("To", 0) == 0 || port.kind == "DataOutPort" ||
                port.kind == "ScanOutPort") {
              throw std::runtime_error(
                  "InputPort target is not an input port: " + port.name);
            }
            const size_t first = expression.begin;
            while (expression.begin < expression.end &&
                   tokens[expression.begin].text != "=") {
              ++expression.begin;
            }
            const size_t equals = expression.begin++;
            --expression.end;
            bindings_.push_back(
                {found->second, {scope_id, first + 2, equals}, expression});
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
      auto& node = netlist_.nodes[binding.node];
      const auto range = SelectRange(netlist_, node, binding.selection);
      const size_t width =
          static_cast<size_t>(std::max(range.first, range.second) -
                              std::min(range.first, range.second)) +
          1;
      const auto source = netlist_.Resolve(binding.source, width);
      auto& destination = node.inputs["Source"];
      if (destination.empty()) {
        destination.resize(node.Width(),
                           {std::numeric_limits<size_t>::max(), 0, '?', false});
      }
      int64_t bit = range.first;
      for (size_t i = 0; i < width; ++i) {
        const size_t offset = static_cast<size_t>(
            node.left >= node.right ? node.left - bit : bit - node.left);
        auto& target = destination.at(offset);
        if (target.constant != '?') {
          throw std::runtime_error(node.origin + ": duplicate input binding " +
                                   node.name);
        }
        target = source[i];
        if (i + 1 < width) bit += range.first <= range.second ? 1 : -1;
      }
    }
    for (auto& node : netlist_.nodes) {
      if (node.model == ModelKind::kEnum) continue;
      try {
        for (const auto& property : node.properties) {
          const auto& name = property.first;
          if (name == "ResetValue" || name == "DefaultLoadValue") {
            node.constants.emplace(name, ConstantValue(node, property.second));
          }
          if (name == "Source" || name == "ScanInSource" ||
              name == "CaptureSource" || name == "Enable" ||
              name == "WriteEnSource" || name == "WriteDataSource" ||
              name == "ReadDataSource") {
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
              node.inputs.emplace(name,
                                  netlist_.Resolve(property.second, width,
                                                   name == "ScanInSource"));
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
    auto selector = netlist_.Resolve(node->properties.at("SelectedBy"));
    const size_t width = selector.size();
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
        choice.codes.push_back(
            netlist_.Value({node->scope, begin, end}, width));
        begin = end + 1;
      }
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
    return {candidates[0], source.left, 'x', ActiveLow(source)};
  }

  void InferResets() {
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
          source.inverted = source.inverted != ActiveLow(node);
          node.inputs["Source"] = {source};
        }
      } catch (const std::exception& error) {
        throw std::runtime_error(node.origin + ": " + error.what());
      }
    }
  }

  void CheckCombinationalCycles() const {
    // Signal-level cycles may be acyclic between bits; registers cut the graph.
    constexpr size_t kMaxGraphBits = 16 * 1024 * 1024;
    std::vector<size_t> offsets{0};
    for (const auto& node : netlist_.nodes) {
      if (node.Width() > kMaxGraphBits - offsets.back()) {
        throw std::runtime_error("ICL bit dependency graph limit exceeded");
      }
      offsets.push_back(offsets.back() + node.Width());
    }
    const auto owner = [&](size_t bit) {
      return static_cast<size_t>(
          std::upper_bound(offsets.begin(), offsets.end(), bit) -
          offsets.begin() - 1);
    };
    const auto dependencies = [&](size_t bit) {
      std::vector<size_t> result;
      const size_t id = owner(bit);
      const auto& node = netlist_.nodes[id];
      if (node.model == ModelKind::kRegister) return result;
      const size_t position = bit - offsets[id];
      const auto append = [&](const BitReference& ref) {
        if (ref.node == std::numeric_limits<size_t>::max()) return;
        const auto& source = netlist_.nodes[ref.node];
        const size_t index = static_cast<size_t>(source.left >= source.right
                                                     ? source.left - ref.index
                                                     : ref.index - source.left);
        result.push_back(offsets[ref.node] + index);
      };
      for (const auto& input : node.inputs) {
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
      std::sort(result.begin(), result.end());
      result.erase(std::unique(result.begin(), result.end()), result.end());
      return result;
    };
    struct Visit {
      size_t bit, next;
      std::vector<size_t> dependencies;
    };
    std::vector<unsigned char> state(offsets.back());
    std::vector<Visit> stack;
    for (size_t root = 0; root < state.size(); ++root) {
      if (state[root]) continue;
      state[root] = 1;
      stack.push_back({root, 0, dependencies(root)});
      while (!stack.empty()) {
        auto& frame = stack.back();
        if (frame.next == frame.dependencies.size()) {
          state[frame.bit] = 2;
          stack.pop_back();
          continue;
        }
        const size_t next = frame.dependencies[frame.next++];
        if (state[next] == 1) {
          const auto& node = netlist_.nodes[owner(next)];
          throw std::runtime_error(
              node.origin + ": combinational connection cycle at " + node.name);
        }
        if (!state[next]) {
          state[next] = 1;
          stack.push_back({next, 0, dependencies(next)});
        }
      }
    }
  }

  Design& design_;
  Netlist netlist_;
  size_t logic_bits_remaining_ = 4 * 1024 * 1024;
  std::map<std::string, Template> templates_;
  std::map<std::pair<size_t, std::string>, std::vector<size_t>> kinds_;
  std::map<std::pair<size_t, std::string>, std::vector<size_t>>
      reset_candidates_;
  struct Binding {
    size_t node;
    Expression selection, source;
  };
  std::vector<Binding> bindings_;
};
}  // namespace

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
                                           size_t width, bool scan_input,
                                           size_t max_bits) const {
  max_bits = std::min(max_bits, kMaxWidth);
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
      const auto id = node_ids.find(Path(scope.path, name));
      if (id == node_ids.end()) {
        throw std::runtime_error("unknown signal " + Path(scope.path, name));
      }
      const auto& source = nodes[id->second];
      if (source.model == ModelKind::kEnum ||
          source.model == ModelKind::kInterface ||
          source.model == ModelKind::kAccessLink ||
          source.kind == "PortGroup") {
        throw std::runtime_error(source.name + " is not a data signal or wire");
      }
      const auto range =
          scan_input && first == end && source.kind == "ScanRegister"
              ? std::make_pair(source.right, source.right)
              : SelectRange(*this, source, {expression.scope, first, end});
      const auto left = range.first, right = range.second;
      const auto length =
          static_cast<size_t>(std::max(left, right) - std::min(left, right)) +
          1;
      if (length > max_bits - known) {
        throw std::runtime_error("signal operand bit limit exceeded");
      }
      if (scan_input && source.kind == "ScanRegister" &&
          (left != source.right || right != source.right)) {
        throw std::runtime_error("ScanInSource must use the rightmost bit of " +
                                 source.name);
      }
      for (int64_t bit = left;; bit += left <= right ? 1 : -1) {
        segment.bits.push_back({id->second, bit, 'x', segment.inverted});
        if (bit == right) break;
      }
      known += segment.bits.size();
    } else {
      segment.literal = true;
      segment.value = Value({expression.scope, first, end});
      if (segment.value.is_string) {
        throw std::runtime_error("string used as signal");
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
  return Elaborator(design).Run(top);
}
}  // namespace pattern
