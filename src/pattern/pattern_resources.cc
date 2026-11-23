// Copyright 2026 The ICL Toolsets Authors.
// SPDX-License-Identifier: MIT
#include "pattern/pattern_resources.h"

#include <algorithm>
#include <stdexcept>
#include <utility>

#include "pattern/pattern_access_queue.h"
#include "pattern/pattern_values.h"
namespace pattern {
ResourceReservations::ResourceReservations(const Netlist& netlist)
    : netlist_(netlist), resources_(netlist.scopes.size()) {
  for (size_t i = 0; i < netlist.scopes.size(); ++i) {
    scopes_.emplace(netlist.scopes[i].path, i);
  }
  for (size_t i = 0; i < netlist.nodes.size(); ++i) {
    const auto& node = netlist.nodes[i];
    if (IsReservableResource(node)) resources_.at(node.scope).push_back(i);
  }
}
void ResourceReservations::Reset() {
  for (auto& owner : owners_) owner.clear();
  counts_.clear();
  values_.clear();
}
std::map<size_t, std::string> ResourceReservations::CallbackConstraints(
    const Operation& operation, const QueuedAccesses& pending,
    const ScanScheduler* established) const {
  std::map<size_t, std::string> result;
  if (operation.callback_preserved.empty()) return result;
  const auto [written, values] = pending.WriteModel();
  for (const auto& [id, desired] : operation.callback_preserved) {
    if (id >= netlist_.nodes.size() ||
        desired.size() != netlist_.nodes[id].Width() ||
        desired.find_first_not_of("01x") != std::string::npos) {
      throw std::runtime_error("invalid callback caller goal mask");
    }
    const auto& node = netlist_.nodes[id];
    const auto current =
        established ? established->Value(node.name) : std::string{};
    for (size_t i = 0; i < desired.size(); ++i) {
      if (desired[i] == 'x') continue;
      const auto index = node.left >= node.right
                             ? node.left - static_cast<int64_t>(i)
                             : node.left + static_cast<int64_t>(i);
      const auto key = std::make_pair(id, index);
      const auto found = std::lower_bound(
          written.begin(), written.end(), key,
          [](const auto& bit, const auto& wanted) {
            return std::make_pair(bit.node, bit.index) < wanted;
          });
      if (found != written.end() && found->node == id &&
          found->index == index && values[found - written.begin()] != 'x') {
        if (values[found - written.begin()] != desired[i]) {
          throw std::runtime_error(
              "queued write conflicts with callback caller goal " + node.name);
        }
        // A future write cannot release a caller goal already established.
        if (!established || current.at(i) != desired[i]) continue;
      }
      auto [entry, inserted] = result.try_emplace(id, node.Width(), 'x');
      entry->second[i] = desired[i];
    }
  }
  return result;
}
std::optional<std::map<size_t, std::string>>
ResourceReservations::CallbackValues(const Operation& operation,
                                     const QueuedAccesses& pending) const {
  const auto caller = CallbackConstraints(operation, pending);
  if (caller.empty()) return std::nullopt;
  auto result = values_;
  for (const auto& [id, mask] : caller) {
    auto [entry, inserted] = result.try_emplace(id, mask.size(), 'x');
    for (size_t i = 0; i < mask.size(); ++i) {
      if (mask[i] == 'x') continue;
      if (entry->second[i] != 'x' && entry->second[i] != mask[i]) {
        throw std::runtime_error(
            "reserved resource conflicts with callback caller goal " +
            netlist_.nodes[id].name);
      }
      entry->second[i] = mask[i];
    }
  }
  return result;
}
void ResourceReservations::Enter(const std::vector<std::string>& call_stack) {
  size_t common = 0;
  while (common < calls_.size() && common < call_stack.size() &&
         calls_[common] == call_stack[common]) {
    ++common;
  }
  while (owners_.size() > common + 1) {
    for (const auto& [id, mask] : owners_.back()) {
      auto& counts = counts_.at(id);
      for (size_t i = 0; i < mask.size(); ++i) {
        if (mask[i] == '1') --counts[i];
      }
      Prune(id);
    }
    owners_.pop_back();
  }
  owners_.resize(call_stack.size() + 1);
  calls_ = call_stack;
}
void ResourceReservations::Prune(size_t id) {
  const auto& counts = counts_.at(id);
  const auto value = values_.find(id);
  if (value != values_.end()) {
    for (size_t i = 0; i < counts.size(); ++i) {
      if (!counts[i]) value->second[i] = 'x';
    }
    if (value->second.find_first_not_of('x') == std::string::npos) {
      values_.erase(value);
    }
  }
  if (std::all_of(counts.begin(), counts.end(),
                  [](size_t count) { return count == 0; })) {
    counts_.erase(id);
  }
}
std::map<size_t, std::string> ResourceReservations::Targets(
    const std::string& name) const {
  std::map<size_t, std::string> result;
  const auto scope = scopes_.find(name);
  if (scope == scopes_.end()) {
    for (const auto& bit : ResolvePdlTarget(netlist_, name).bits) {
      if (bit.node >= netlist_.nodes.size() ||
          !IsReservableResource(netlist_.nodes[bit.node])) {
        throw std::runtime_error(
            "resource reservation requires a register, port or instance: " +
            name);
      }
      const auto& node = netlist_.nodes[bit.node];
      auto [entry, inserted] = result.try_emplace(bit.node, node.Width(), '0');
      entry->second.at(node.left >= node.right ? node.left - bit.index
                                               : bit.index - node.left) = '1';
    }
    return result;
  }
  std::vector<size_t> pending{scope->second};
  while (!pending.empty()) {
    const auto id = pending.back();
    pending.pop_back();
    for (size_t reg : resources_[id]) {
      result.emplace(reg, std::string(netlist_.nodes[reg].Width(), '1'));
    }
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
  for (const auto& [id, mask] : Targets(operation.arguments[0])) {
    if (operation.command == "iTake") {
      auto& owner = persistent ? owners_.front() : owners_.back();
      auto [entry, inserted] = owner.try_emplace(id, mask.size(), '0');
      auto [count, added] = counts_.try_emplace(id, mask.size(), 0);
      for (size_t i = 0; i < mask.size(); ++i) {
        if (mask[i] == '1' && entry->second[i] == '0') {
          entry->second[i] = '1';
          ++count->second[i];
        }
      }
    } else if (counts_.count(id)) {
      // Explicit release propagates to inherited owners; return does not.
      for (auto& owner : owners_) {
        const auto entry = owner.find(id);
        if (entry == owner.end()) continue;
        for (size_t i = 0; i < mask.size(); ++i) {
          if (mask[i] == '1' && entry->second[i] == '1') {
            entry->second[i] = '0';
            --counts_.at(id)[i];
          }
        }
        if (entry->second.find('1') == std::string::npos) owner.erase(entry);
      }
      Prune(id);
    }
  }
}
void ResourceReservations::Assigned(const std::vector<Operation>& operations,
                                    const ScanScheduler& scheduler) {
  if (counts_.empty()) return;
  std::map<size_t, std::string> written;
  for (const auto& operation : operations) {
    if (operation.command != "iWrite" && operation.command != "iState") {
      continue;
    }
    for (const auto& bit :
         ResolvePdlTarget(netlist_, operation.arguments.at(0)).bits) {
      const auto found = counts_.find(bit.node);
      if (found == counts_.end()) continue;
      const auto& node = netlist_.nodes[bit.node];
      const size_t offset = node.left >= node.right ? node.left - bit.index
                                                    : bit.index - node.left;
      if (found->second.at(offset)) {
        auto [entry, inserted] =
            written.try_emplace(bit.node, node.Width(), 'x');
        entry->second[offset] = '1';
      }
    }
  }
  for (const auto& [id, mask] : written) {
    Assign(id, mask, scheduler);
  }
}
void ResourceReservations::AssignedScans(const std::set<size_t>& registers,
                                         const ScanScheduler& scheduler) {
  for (const auto id : registers) {
    const auto found = values_.find(id);
    if (found != values_.end()) Assign(id, found->second, scheduler);
  }
}
void ResourceReservations::Assign(size_t id, const std::string& mask,
                                  const ScanScheduler& scheduler) {
  const auto& name = netlist_.nodes[id].name;
  const auto value = scheduler.Value(name);
  auto [entry, inserted] = values_.try_emplace(id, mask.size(), 'x');
  for (size_t i = 0; i < mask.size(); ++i) {
    if (mask[i] == 'x') continue;
    if (value[i] != '0' && value[i] != '1') {
      throw std::runtime_error("applied reserved resource is unknown: " + name);
    }
    entry->second[i] = value[i];
  }
}
std::map<size_t, std::string> ResourceReservations::UnwrittenBits() const {
  std::map<size_t, std::string> result;
  for (const auto& [id, counts] : counts_) {
    std::string mask(counts.size(), '0');
    const auto value = values_.find(id);
    for (size_t i = 0; i < counts.size(); ++i) {
      if (counts[i] && (value == values_.end() || value->second[i] == 'x')) {
        mask[i] = '1';
      }
    }
    if (mask.find('1') != std::string::npos) {
      result.emplace(id, std::move(mask));
    }
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
      const auto found = counts_.find(bit.node);
      if (found == counts_.end()) continue;
      const auto& node = netlist_.nodes[bit.node];
      const size_t offset = node.left >= node.right ? node.left - bit.index
                                                    : bit.index - node.left;
      const auto value = values_.find(bit.node);
      if (found->second.at(offset) &&
          (value == values_.end() || value->second[offset] == 'x')) {
        throw std::runtime_error(
            "local-reset insertion would lock taken-but-unwritten resource " +
            node.name);
      }
    }
  }
}
}  // namespace pattern
