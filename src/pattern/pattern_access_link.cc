// Copyright 2026 The ICL Toolsets Authors.
// SPDX-License-Identifier: MIT
#include "pattern/pattern_access_link.h"

#include <algorithm>
#include <map>
#include <set>
#include <stdexcept>
#include <utility>

namespace pattern {
namespace {
using TokenIterator = std::vector<icl::SourceToken>::const_iterator;
std::pair<TokenIterator, TokenIterator> Tokens(const ModuleDefinition& module,
                                               const icl::AstNode& ast) {
  const auto range = module.document->Range(ast.span());
  const auto& tokens = module.document->driver().unit().tokens();
  const auto at = [&](size_t offset) {
    return std::lower_bound(tokens.begin(), tokens.end(), offset,
                            [](const icl::SourceToken& token, size_t byte) {
                              return token.begin < byte;
                            });
  };
  return {at(range.begin), at(range.end)};
}
std::string Kind(const ModuleDefinition& module, const icl::AstNode& ast) {
  if (dynamic_cast<const icl::AstComment*>(&ast)) return "";
  return Tokens(module, ast).first->text;
}
std::string Origin(const ModuleDefinition& module, const icl::AstNode& ast) {
  return module.Location(module.document->Range(ast.span()).begin);
}
std::string Text(const ModuleDefinition& module, const icl::AstNode& ast) {
  const auto tokens = Tokens(module, ast);
  std::string result;
  for (auto token = tokens.first; token != tokens.second; ++token) {
    if (token->text != ";") result += token->text;
  }
  return result;
}
const icl::Declaration* Member(const ModuleDefinition& module,
                               const std::string& name) {
  const auto scope = module.document->members().find(module.declaration_index);
  if (scope == module.document->members().end()) return nullptr;
  const auto found = scope->second.find(name);
  if (found == scope->second.end()) return nullptr;
  const icl::Declaration* result = nullptr;
  for (size_t index : found->second) {
    const auto& declaration = module.document->declarations()[index];
    if (declaration.parent != static_cast<int>(module.declaration_index) &&
        (declaration.parent < 0 ||
         module.document->declarations()[declaration.parent].kind !=
             "OneHotDataGroup")) {
      continue;
    }
    if (result) {
      throw std::runtime_error(module.Location(declaration.name_range.begin) +
                               ": duplicate declaration " + name);
    }
    result = &declaration;
  }
  return result;
}

class Binder {
 public:
  Binder(const Design& design, const ModuleDefinition& top)
      : design_(design), top_(design.modules().at(top.name())) {
    std::vector<const icl::AstNode*> pending(top_.ast->body().begin(),
                                             top_.ast->body().end());
    while (!pending.empty()) {
      const auto* node = pending.back();
      pending.pop_back();
      top_nodes_.emplace(top_.document->Range(node->span()).begin, node);
      if (Kind(top_, *node) == "OneHotDataGroup") {
        const auto& body = dynamic_cast<const icl::AstBlock&>(*node).body();
        pending.insert(pending.end(), body.begin(), body.end());
      }
    }
    for (size_t index : top_.instances) {
      const auto& declaration = top_.document->declarations()[index];
      if (!instances_.emplace(declaration.name, index).second) {
        throw std::runtime_error(top_.Location(declaration.name_range.begin) +
                                 ": duplicate instance " + declaration.name);
      }
      const auto* block = dynamic_cast<const icl::AstBlock*>(
          top_nodes_.at(declaration.range.begin));
      if (!block) continue;
      for (const auto* child : block->body()) {
        if (Kind(top_, *child) == "InputPort") {
          input_bindings_[index].emplace((Tokens(top_, *child).first + 1)->text,
                                         child);
        }
      }
    }
  }
  std::vector<AccessLink> Run() {
    CheckPlacement();
    std::vector<AccessLink> links;
    for (const auto* node : top_.ast->body()) {
      if (Kind(top_, *node) != "AccessLink") continue;
      const auto& block = dynamic_cast<const icl::AstBlock&>(*node);
      const auto begin = Tokens(top_, block).first;
      AccessLink link;
      link.module = &top_;
      link.name = (begin + 1)->text;
      link.standard = (begin + 3)->text;
      link.origin = Origin(top_, block);
      try {
        for (const auto* child : block.body()) {
          if (dynamic_cast<const icl::AstStatement*>(child) &&
              Kind(top_, *child) == "BSDLEntity") {
            link.bsdl =
                &design_.ResolveBsdl((Tokens(top_, *child).first + 1)->text);
            CheckTapPorts(*link.bsdl);
          } else if (const auto* instruction =
                         dynamic_cast<const icl::AstBlock*>(child)) {
            auto bound = ReadInstruction(*instruction);
            if (!link.bsdl->opcodes.count(bound.name)) {
              throw std::runtime_error(
                  bound.origin + ": unknown BSDL instruction " + bound.name);
            }
            link.instructions.push_back(std::move(bound));
          }
        }
      } catch (const std::exception& error) {
        throw std::runtime_error(link.origin + ": AccessLink " + link.name +
                                 ": " + error.what());
      }
      links.push_back(std::move(link));
    }
    return links;
  }

 private:
  void CheckTapPorts(const BsdlTap& tap) const {
    static const std::map<std::string, std::string> kKinds = {
        {"tap_scan_in", "ScanInPort"},
        {"tap_scan_out", "ScanOutPort"},
        {"tap_scan_mode", "TMSPort"},
        {"tap_scan_clock", "TCKPort"},
        {"tap_scan_reset", "TRSTPort"}};
    for (const auto& [role, name] : tap.ports) {
      const auto* declaration = Member(top_, name);
      if (!declaration) continue;
      const auto origin = top_.Location(declaration->name_range.begin);
      if (declaration->kind != kKinds.at(role)) {
        throw std::runtime_error(origin + ": TAP port " + name +
                                 " requires a unique " + kKinds.at(role));
      }
      if (role != "tap_scan_out") continue;
      const auto* block = dynamic_cast<const icl::AstBlock*>(
          top_nodes_.at(declaration->range.begin));
      if (!block) continue;
      for (const auto* child : block->body()) {
        if (Kind(top_, *child) == "Source") {
          throw std::runtime_error(Origin(top_, *child) + ": TAP port " + name +
                                   " must not have a Source");
        }
      }
    }
  }
  struct InterfaceDefinition {
    const icl::AstBlock* ast;
    size_t chains;
    std::set<std::string> scan_inputs;
  };
  void ReadScanInputs(const ModuleDefinition& module,
                      const icl::AstBlock& block,
                      std::set<std::string>* inputs) const {
    for (const auto* item : block.body()) {
      const auto kind = Kind(module, *item);
      if (kind == "Chain") {
        ReadScanInputs(module, dynamic_cast<const icl::AstBlock&>(*item),
                       inputs);
      } else if (kind == "Port") {
        const auto name = (Tokens(module, *item).first + 1)->text;
        const auto* declaration = Member(module, name);
        if (declaration && declaration->kind == "ScanInPort") {
          inputs->insert(name);
        }
      }
    }
  }
  void CheckPlacement() const {
    std::set<const ModuleDefinition*> seen;
    std::vector<const ModuleDefinition*> pending{&top_};
    while (!pending.empty()) {
      const auto* module = pending.back();
      pending.pop_back();
      if (!seen.insert(module).second) continue;
      for (const auto* node : module->ast->body()) {
        if (Kind(*module, *node) != "AccessLink") continue;
        if (!dynamic_cast<const icl::AstBlock*>(node)) {
          throw std::runtime_error(Origin(*module, *node) +
                                   ": unsupported generic AccessLink");
        }
        if (module != &top_) {
          throw std::runtime_error(
              Origin(*module, *node) +
              ": IEEE 1149.1 AccessLink requires the top-level module");
        }
      }
      for (size_t index : module->instances) {
        pending.push_back(&design_.ResolveInstance(*module, index));
      }
    }
  }
  AccessLinkInstruction ReadInstruction(const icl::AstBlock& block) {
    AccessLinkInstruction result;
    result.name = Kind(top_, block);
    result.origin = Origin(top_, block);
    for (const auto* child : block.body()) {
      const auto* selected = dynamic_cast<const icl::AstBlock*>(child);
      if (!selected) continue;
      const bool interfaces = Kind(top_, *selected) == "ScanInterface";
      for (const auto* item : selected->body()) {
        if (dynamic_cast<const icl::AstComment*>(item)) continue;
        if (interfaces) {
          result.interfaces.push_back(ReadInterface(*item));
        } else {
          result.active_signals.push_back(
              {Text(top_, *item), Origin(top_, *item)});
        }
      }
    }
    return result;
  }
  AccessLinkInterface ReadInterface(const icl::AstNode& node) {
    const auto tokens = Tokens(top_, node);
    AccessLinkInterface result;
    result.instance = tokens.first->text;
    result.origin = Origin(top_, node);
    const auto instance = instances_.find(result.instance);
    if (instance == instances_.end()) {
      throw std::runtime_error(result.origin + ": unknown instance " +
                               result.instance);
    }
    result.module = &design_.ResolveInstance(top_, instance->second);
    auto cached = interfaces_.find(result.module);
    if (cached == interfaces_.end()) {
      std::map<std::string, InterfaceDefinition> interfaces;
      for (const auto* child : result.module->ast->body()) {
        if (Kind(*result.module, *child) == "ScanInterface") {
          const auto& block = dynamic_cast<const icl::AstBlock&>(*child);
          size_t chains = 0;
          for (const auto* item : block.body()) {
            if (Kind(*result.module, *item) == "Chain") ++chains;
          }
          const auto name = (Tokens(*result.module, *child).first + 1)->text;
          std::set<std::string> inputs;
          ReadScanInputs(*result.module, block, &inputs);
          if (!interfaces
                   .emplace(name, InterfaceDefinition{&block, chains,
                                                      std::move(inputs)})
                   .second) {
            throw std::runtime_error(Origin(*result.module, *child) +
                                     ": duplicate ScanInterface " + name);
          }
        }
      }
      cached = interfaces_.emplace(result.module, std::move(interfaces)).first;
    }
    const auto& interfaces = cached->second;
    if ((tokens.first + 1)->text == ".") {
      result.name = (tokens.first + 2)->text;
    } else {
      if (interfaces.size() > 1) {
        throw std::runtime_error(
            result.origin + ": ambiguous ScanInterface for " + result.instance);
      }
      if (!interfaces.empty()) result.name = interfaces.begin()->first;
    }
    const auto selected = interfaces.find(result.name);
    if (selected == interfaces.end()) {
      throw std::runtime_error(result.origin + ": unknown ScanInterface " +
                               result.instance + "." + result.name);
    }
    result.ast = selected->second.ast;
    if (selected->second.chains > 1) {
      throw std::runtime_error(
          result.origin +
          ": selected interface must have exactly one scan chain");
    }
    const auto bindings = input_bindings_.find(instance->second);
    if (bindings != input_bindings_.end()) {
      for (const auto& input : selected->second.scan_inputs) {
        const auto bound = bindings->second.find(input);
        if (bound != bindings->second.end()) {
          throw std::runtime_error(
              Origin(top_, *bound->second) + ": " + result.instance + "." +
              input + " already has an InputPort binding (selected at " +
              result.origin + ")");
        }
      }
    }
    return result;
  }
  const Design& design_;
  const ModuleDefinition& top_;
  std::map<std::string, size_t> instances_;
  std::map<size_t, const icl::AstNode*> top_nodes_;
  std::map<size_t, std::map<std::string, const icl::AstNode*>> input_bindings_;
  std::map<const ModuleDefinition*, std::map<std::string, InterfaceDefinition>>
      interfaces_;
};
}  // namespace
std::vector<AccessLink> BindAccessLinks(const Design& design,
                                        const ModuleDefinition& top) {
  return Binder(design, top).Run();
}
}  // namespace pattern
