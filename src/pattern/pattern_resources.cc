// Copyright 2026 The ICL Toolsets Authors.
// SPDX-License-Identifier: MIT
#include "pattern/pattern_resources.h"

#include <stdexcept>

#include "pattern/pattern_values.h"
namespace pattern {
ResourceReservations::ResourceReservations(const Netlist& netlist)
    : netlist_(netlist), registers_(netlist.scopes.size()) {
  for (size_t i = 0; i < netlist.scopes.size(); ++i) {
    scopes_.emplace(netlist.scopes[i].path, i);
  }
  for (size_t i = 0; i < netlist.nodes.size(); ++i) {
    const auto& node = netlist.nodes[i];
    if (node.kind == "ScanRegister") registers_.at(node.scope).push_back(i);
  }
}
void ResourceReservations::Enter(const std::vector<std::string>& call_stack) {
  size_t common = 0;
  while (common < calls_.size() && common < call_stack.size() &&
         calls_[common] == call_stack[common]) {
    ++common;
  }
  while (owners_.size() > common + 1) {
    for (size_t id : owners_.back()) {
      if (--counts_.at(id) == 0) {
        counts_.erase(id);
        values_.erase(id);
      }
    }
    owners_.pop_back();
  }
  owners_.resize(call_stack.size() + 1);
  calls_ = call_stack;
}
std::set<size_t> ResourceReservations::Targets(const std::string& name) const {
  const auto node = netlist_.node_ids.find(name);
  if (node != netlist_.node_ids.end()) {
    if (netlist_.nodes[node->second].kind == "ScanRegister") {
      return {node->second};
    }
    throw std::runtime_error("resource reservation requires a ScanRegister: " +
                             name);
  }
  const auto scope = scopes_.find(name);
  if (scope == scopes_.end()) {
    throw std::runtime_error("unknown or unsupported resource " + name);
  }
  std::set<size_t> result;
  std::vector<size_t> pending{scope->second};
  while (!pending.empty()) {
    const auto id = pending.back();
    pending.pop_back();
    result.insert(registers_[id].begin(), registers_[id].end());
    const auto& children = netlist_.scopes[id].children;
    pending.insert(pending.end(), children.begin(), children.end());
  }
  return result;
}
void ResourceReservations::Change(const Operation& operation) {
  const auto& args = operation.arguments;
  const bool persistent = operation.command == "iTake" && args.size() == 2 &&
                          args[1] == "-persistent";
  if ((!persistent && args.size() != 1) ||
      (!args[0].empty() && args[0][0] == '-') ||
      (operation.command == "iRelease" && args[0].empty())) {
    throw std::runtime_error("invalid resource reservation arguments");
  }
  for (size_t id : Targets(operation.arguments[0])) {
    if (operation.command == "iTake") {
      auto& owner = persistent ? owners_.front() : owners_.back();
      if (owner.insert(id).second) ++counts_[id];
    } else {
      // Explicit release propagates to inherited owners; return does not.
      for (auto& owner : owners_) owner.erase(id);
      counts_.erase(id);
      values_.erase(id);
    }
  }
}
void ResourceReservations::Assigned(const std::vector<Operation>& operations,
                                    const ScanScheduler& scheduler) {
  if (counts_.empty()) return;
  std::set<size_t> written;
  for (const auto& operation : operations) {
    if (operation.command != "iWrite" && operation.command != "iState") {
      continue;
    }
    for (const auto& bit :
         ResolvePdlTarget(netlist_, operation.arguments.at(0)).bits) {
      if (counts_.count(bit.node)) written.insert(bit.node);
    }
  }
  for (size_t id : written) {
    const auto& name = netlist_.nodes[id].name;
    const auto value = scheduler.Value(name);
    if (value.find_first_not_of("01") != std::string::npos) {
      throw std::runtime_error("applied reserved resource is unknown: " + name);
    }
    values_.insert_or_assign(id, value);
  }
}
std::set<size_t> ResourceReservations::UnwrittenRegisters() const {
  std::set<size_t> result;
  for (const auto& [id, count] : counts_) {
    if (!values_.count(id)) result.insert(id);
  }
  return result;
}
void ResourceReservations::CheckAutomaticWrites(
    const std::vector<Operation>& operations) const {
  if (counts_.empty()) return;
  for (const auto& operation : operations) {
    if (operation.command != "iWrite") continue;
    for (const auto& bit :
         ResolvePdlTarget(netlist_, operation.arguments.at(0)).bits) {
      if (counts_.count(bit.node) && !values_.count(bit.node)) {
        throw std::runtime_error(
            "local-reset insertion would lock taken-but-unwritten resource " +
            netlist_.nodes.at(bit.node).name);
      }
    }
  }
}
}  // namespace pattern
