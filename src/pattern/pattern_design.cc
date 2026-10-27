// Copyright 2026 The ICL Toolsets Authors.
// SPDX-License-Identifier: MIT
#include "pattern/pattern_design.h"

#include <algorithm>
#include <set>
#include <stdexcept>
#include <utility>

namespace pattern {
namespace {
std::string Qualified(const std::string& name_space, const std::string& name) {
  return name_space.empty() ? name : name_space + "::" + name;
}
std::string BsdlKey(std::string entity) {
  for (auto& c : entity) {
    if (c >= 'A' && c <= 'Z') c += 'a' - 'A';
  }
  return entity;
}
}  // namespace

std::string ModuleDefinition::name() const {
  return Qualified(declaration().name_space, declaration().name);
}

std::string ModuleDefinition::Location(size_t byte) const {
  const auto& lines = document->line_starts();
  byte = std::min(byte, document->source().size());
  const auto line = std::upper_bound(lines.begin(), lines.end(), byte);
  return path + ":" + std::to_string(line - lines.begin()) + ":" +
         std::to_string(byte - *(line - 1) + 1);
}

void Design::AddIcl(const std::string& path, const std::string& text) {
  const auto document = std::make_shared<icl::ParsedDocument>(text);
  if (!document->ok()) {
    const auto& driver = document->driver();
    throw std::runtime_error(
        path + ":" + std::to_string(driver.error_line()) + ":" +
        std::to_string(std::max(1, driver.error_span().first_column)) + ": " +
        driver.error_message());
  }
  std::map<size_t, const icl::AstBlock*> blocks;
  for (const auto* node : document->driver().unit().top_level()) {
    if (const auto* block = dynamic_cast<const icl::AstBlock*>(node)) {
      blocks.emplace(document->Range(block->span()).begin, block);
    }
  }
  std::map<size_t, ModuleDefinition> pending;
  const auto& declarations = document->declarations();
  for (size_t i = 0; i < declarations.size(); ++i) {
    const auto& declaration = declarations[i];
    if (declaration.kind == "Module") {
      pending.emplace(
          i, ModuleDefinition{
                 path, document, i, blocks.at(declaration.range.begin), {}});
    } else if (declaration.kind == "Instance" && declaration.module >= 0) {
      pending.at(static_cast<size_t>(declaration.module))
          .instances.push_back(i);
    }
  }
  for (auto& entry : pending) {
    auto& module = entry.second;
    const auto key = module.name();
    const auto previous = modules_.find(key);
    if (previous != modules_.end()) {
      const auto& old = previous->second;
      warnings_.push_back(
          module.Location(module.declaration().name_range.begin) +
          ": warning: module " + key + " overrides " +
          old.Location(old.declaration().name_range.begin));
    }
    modules_.insert_or_assign(key, std::move(module));
  }
}

void Design::AddBsdl(const std::string& path, const std::string& text,
                     const std::string& entity) {
  auto tap = ReadBsdlTap(path, text, entity);
  const auto key = BsdlKey(tap.entity);
  const auto previous = bsdl_taps_.find(key);
  if (previous != bsdl_taps_.end()) {
    throw std::runtime_error(tap.origin + ": BSDL entity " + tap.entity +
                             " already loaded at " + previous->second.origin);
  }
  bsdl_taps_.emplace(key, std::move(tap));
}

const BsdlTap& Design::ResolveBsdl(const std::string& entity) const {
  const auto found = bsdl_taps_.find(BsdlKey(entity));
  if (found == bsdl_taps_.end()) {
    throw std::runtime_error("no BSDL model loaded for entity " + entity);
  }
  return found->second;
}

const ModuleDefinition& Design::ResolveInstance(
    const ModuleDefinition& owner, size_t declaration_index) const {
  const auto& instance = owner.document->declarations().at(declaration_index);
  if (instance.kind != "Instance" ||
      instance.module != static_cast<int>(owner.declaration_index)) {
    throw std::runtime_error("instance does not belong to module " +
                             owner.name());
  }
  const auto target =
      Qualified(instance.target_namespace, instance.target_name);
  const auto found = modules_.find(target);
  if (found == modules_.end()) {
    throw std::runtime_error(owner.Location(instance.name_range.begin) +
                             ": instance " + owner.name() + "." +
                             instance.name + " references undefined module " +
                             target);
  }
  return found->second;
}

void Design::CheckHierarchy(const ModuleDefinition& top) const {
  struct Frame {
    const ModuleDefinition* module;
    size_t next = 0;
  };
  std::vector<Frame> stack{{&top}};
  std::map<const ModuleDefinition*, int> state{{&top, 1}};
  while (!stack.empty()) {
    auto& frame = stack.back();
    if (frame.next == frame.module->instances.size()) {
      state[frame.module] = 2;
      stack.pop_back();
      continue;
    }
    const size_t index = frame.module->instances[frame.next++];
    const auto& child = ResolveInstance(*frame.module, index);
    if (state[&child] == 2) continue;
    if (state[&child] == 1) {
      std::string chain;
      for (const auto& ancestor : stack) {
        chain += ancestor.module->name() + " -> ";
      }
      const auto& instance = frame.module->document->declarations()[index];
      throw std::runtime_error(
          frame.module->Location(instance.name_range.begin) +
          ": recursive module cycle: " + chain + child.name());
    }
    state[&child] = 1;
    stack.push_back({&child});
  }
}

const ModuleDefinition& Design::SelectTop(const std::string& name) {
  if (modules_.empty()) {
    throw std::runtime_error("ICL design contains no modules");
  }
  const ModuleDefinition* top = nullptr;
  if (!name.empty()) {
    if (name.find("::") != std::string::npos) {
      const auto found =
          modules_.find(name.rfind("::", 0) == 0 ? name.substr(2) : name);
      if (found != modules_.end()) top = &found->second;
    } else {
      for (const auto& entry : modules_) {
        if (entry.second.declaration().name != name) continue;
        if (top) {
          throw std::runtime_error("ambiguous top module " + name +
                                   "; qualify --top_module with its namespace");
        }
        top = &entry.second;
      }
    }
    if (!top) throw std::runtime_error("unknown top module " + name);
  } else {
    std::set<std::string> instantiated;
    for (const auto& entry : modules_) {
      for (size_t index : entry.second.instances) {
        instantiated.insert(ResolveInstance(entry.second, index).name());
      }
    }
    std::vector<const ModuleDefinition*> candidates;
    for (const auto& entry : modules_) {
      if (!instantiated.count(entry.first)) candidates.push_back(&entry.second);
    }
    if (candidates.empty()) {
      CheckHierarchy(modules_.begin()->second);
      throw std::runtime_error("no top module: recursive module cycle");
    }
    if (candidates.size() != 1) {
      std::string choices;
      for (const auto* candidate : candidates) {
        choices += " " + candidate->name();
      }
      throw std::runtime_error("multiple top modules:" + choices +
                               "; specify --top_module");
    }
    top = candidates.front();
  }
  CheckHierarchy(*top);
  if (name.empty()) {
    warnings_.push_back("warning: automatically selected top module " +
                        top->name());
  }
  return *top;
}
}  // namespace pattern
