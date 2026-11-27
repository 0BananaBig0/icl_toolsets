// Copyright 2026 The ICL Toolsets Authors.
// SPDX-License-Identifier: MIT
#include "pattern/pattern_data.h"

#include <deque>
#include <set>
#include <stdexcept>
#include <tuple>

#include "utility/strong_components.h"

namespace pattern {
namespace {
void AppendDependencySources(const NetlistNode& node, int64_t index,
                             bool controls,
                             std::vector<BitReference>* pending) {
  const size_t position =
      node.left >= node.right ? node.left - index : index - node.left;
  for (const auto& [role, bits] : node.inputs) {
    if (role != "SelectedBy" && bits.size() == node.Width()) {
      pending->push_back(bits.at(position));
    } else if (controls) {
      pending->insert(pending->end(), bits.begin(), bits.end());
    }
  }
  for (const auto& choice : node.choices) {
    pending->push_back(choice.source.at(position));
  }
  for (size_t source = 0; source < node.one_hot_sources.size(); ++source) {
    pending->push_back(node.OneHotSourceBit(source, position));
  }
  if (controls) {
    for (const auto& term : node.logic) {
      pending->insert(pending->end(), term.signals.begin(), term.signals.end());
    }
  }
}
}  // namespace
DataReadRoutes::DataReadRoutes(const Netlist& netlist, DataReadMode mode) {
  constexpr size_t kMissing = std::numeric_limits<size_t>::max();
  struct Pending {
    BitReference bit;
    DataReadRoute route;
    bool direct = true;
  };
  size_t remaining = 262144;
  for (size_t id = 0; id < netlist.nodes.size(); ++id) {
    const auto& output = netlist.nodes[id];
    const bool scan_capture =
        output.kind == "ScanRegister" && output.inputs.count("CaptureSource");
    const bool primary = output.scope == 0 && output.kind == "DataOutPort";
    if ((!scan_capture || mode == DataReadMode::kPrimary) &&
        (!primary || mode == DataReadMode::kScan)) {
      continue;
    }
    for (size_t offset = 0; offset < output.Width(); ++offset) {
      const auto index = output.left + (output.left <= output.right
                                            ? static_cast<int64_t>(offset)
                                            : -static_cast<int64_t>(offset));
      const auto source = scan_capture
                              ? output.inputs.at("CaptureSource").at(offset)
                              : BitReference{id, index};
      std::vector<Pending> pending{{source, {{id, index}, {}}}};
      while (!pending.empty()) {
        auto item = std::move(pending.back());
        pending.pop_back();
        if (!remaining-- || item.route.guards.size() > 256) {
          throw std::runtime_error("data read route resource limit exceeded");
        }
        const auto bit = item.bit;
        if (bit.node == kMissing) continue;
        const auto& node = netlist.nodes.at(bit.node);
        const auto position = node.left >= node.right ? node.left - bit.index
                                                      : bit.index - node.left;
        item.route.output.inverted = item.route.output.inverted != bit.inverted;
        if (node.inputs.count("Enable")) {
          item.route.guards.push_back({bit.node, kMissing});
        }
        if (item.direct &&
            (node.kind == "DataRegister" || node.kind == "DataOutPort")) {
          routes[{bit.node, bit.index}].push_back(item.route);
        }
        if (node.model == ModelKind::kRegister) {
          storage[{id, index}].insert(bit.node);
          continue;
        }
        if (node.kind == "DataMux" || node.kind == "OneHotDataGroup") {
          const size_t count = node.kind == "DataMux"
                                   ? node.choices.size()
                                   : node.one_hot_sources.size();
          for (size_t choice = 0; choice < count; ++choice) {
            auto route = item.route;
            route.guards.push_back({bit.node, choice});
            const auto source = node.kind == "DataMux"
                                    ? node.choices[choice].source.at(position)
                                    : node.OneHotSourceBit(choice, position);
            pending.push_back({source, std::move(route), item.direct});
          }
        } else if (node.model == ModelKind::kAlias ||
                   node.kind == "DataInPort" || node.kind == "DataOutPort") {
          const auto source = node.inputs.find("Source");
          if (source != node.inputs.end()) {
            pending.push_back({source->second.at(position),
                               std::move(item.route), item.direct});
          }
        } else if (node.model == ModelKind::kLogic) {
          // Logic outputs are observable, but are not transparent routes for
          // register comparisons. Their storage must still remain unchanged.
          for (const auto& term : node.logic) {
            for (const auto& source : term.signals) {
              pending.push_back({source, item.route, false});
            }
          }
        }
      }
    }
  }
}
DataRegisterPlan::DataRegisterPlan(const Netlist& netlist) {
  AddressResolver resolver(netlist);
  using BitKey = std::tuple<size_t, int64_t, bool, char, VirtualSignal>;
  std::map<std::pair<std::vector<BitKey>, std::string>, size_t> addresses;
  for (size_t id = 0; id < netlist.nodes.size(); ++id) {
    const auto& node = netlist.nodes[id];
    if (node.kind != "DataRegister" || !node.inputs.count("WriteDataSource")) {
      continue;
    }
    if (writes.size() >= 4096) {
      throw std::runtime_error("data-register count limit exceeded");
    }
    if (node.constants.count("AddressValue")) {
      auto access = resolver.Resolve(node.name, AddressOperation::kWrite);
      std::vector<BitKey> key;
      for (const auto& bit : *access.signals) {
        key.emplace_back(bit.node, bit.index, bit.inverted, bit.constant,
                         bit.virtual_signal);
      }
      const auto& enable = access.enable;
      key.emplace_back(enable.node, enable.index, enable.inverted,
                       enable.constant, enable.virtual_signal);
      auto code = access.value.data;
      code.insert(0, access.signals->size() - code.size(), '0');
      const auto previous = addresses.emplace(
          std::make_pair(std::move(key), std::move(code)), id);
      if (!previous.second) {
        throw std::runtime_error(
            node.origin + ": ambiguous data-register address for " + node.name +
            " and " + netlist.nodes[previous.first->second].name);
      }
      writes.emplace(id, std::move(access));
    } else {
      const auto enable = node.inputs.find("WriteEnSource");
      writes.emplace(
          id, AddressRequirement{
                  nullptr,
                  {},
                  enable == node.inputs.end()
                      ? BitReference{std::numeric_limits<size_t>::max(), 0, '1'}
                      : enable->second.at(0)});
    }
  }
  std::map<size_t, std::set<size_t>> dependencies;
  std::map<size_t, std::vector<size_t>> consumers;
  size_t remaining = 4 * 1024 * 1024;
  OneHotReadResolver one_hot(netlist, &remaining);
  std::set<BitKey> implicit_seen;
  for (const auto& [id, activation] : writes) {
    const auto& node = netlist.nodes[id];
    std::vector<BitReference> pending = node.inputs.at("WriteDataSource");
    pending.push_back(activation.enable);
    if (activation.signals) {
      pending.insert(pending.end(), activation.signals->begin(),
                     activation.signals->end());
    }
    if (node.reset) pending.push_back(*node.reset);
    const auto implicit_control = [&](const BitReference& bit) {
      pending.push_back(bit);
      if (bit.node != std::numeric_limits<size_t>::max() &&
          implicit_seen
              .emplace(bit.node, bit.index, bit.inverted, bit.constant,
                       bit.virtual_signal)
              .second) {
        implicit_controls.push_back(bit);
      }
    };
    std::set<std::pair<size_t, int64_t>> visited;
    std::set<size_t> expanded_controls;
    auto& sources = dependencies[id];
    while (!pending.empty()) {
      const auto ref = pending.back();
      pending.pop_back();
      if (!remaining--) {
        throw std::runtime_error(
            "data-register dependency resource limit exceeded");
      }
      if (ref.node == std::numeric_limits<size_t>::max() ||
          !visited.emplace(ref.node, ref.index).second) {
        continue;
      }
      const auto& source = netlist.nodes.at(ref.node);
      if (source.model == ModelKind::kRegister) {
        if (writes.count(ref.node)) sources.insert(ref.node);
        if (source.kind == "ScanRegister") scan_inputs[id].insert(ref.node);
        continue;
      }
      if (source.scope == 0 && IsInputPort(source.kind) &&
          !source.inputs.count("Source")) {
        primary_inputs[id].insert(ref.node);
      }
      // Shared controls are expanded once per node, independent of word width.
      const bool controls = expanded_controls.insert(ref.node).second;
      AppendDependencySources(source, ref.index, controls, &pending);
      if (controls) {
        for (const auto& contributor : source.one_hot_sources) {
          for (const auto& condition : one_hot.Resolve(contributor)) {
            implicit_control(condition.enable);
            if (condition.signals) {
              for (const auto& bit : *condition.signals) implicit_control(bit);
            }
          }
        }
      }
    }
    for (size_t source : sources) consumers[source].push_back(id);
  }
  std::set<std::pair<std::string, size_t>> ready;
  std::map<size_t, size_t> waiting;
  for (const auto& [id, sources] : dependencies) {
    waiting[id] = sources.size();
    if (sources.empty()) ready.emplace(netlist.nodes[id].name, id);
  }
  while (!ready.empty()) {
    const auto id = ready.begin()->second;
    ready.erase(ready.begin());
    order.push_back(id);
    for (size_t consumer : consumers[id]) {
      primary_inputs[consumer].insert(primary_inputs[id].begin(),
                                      primary_inputs[id].end());
      scan_inputs[consumer].insert(scan_inputs[id].begin(),
                                   scan_inputs[id].end());
      if (--waiting[consumer] == 0) {
        ready.emplace(netlist.nodes[consumer].name, consumer);
      }
    }
  }
  if (order.size() == writes.size()) {
    for (size_t id : order) groups.push_back({{id}, false});
    return;
  }
  std::vector<size_t> vertices;
  for (const auto& [id, access] : writes) vertices.push_back(id);
  const auto components =
      toolsets::StrongComponents(vertices, dependencies, consumers);
  std::map<size_t, size_t> membership;
  order.clear();
  for (size_t group = 0; group < components.size(); ++group) {
    const auto& members = components[group];
    for (size_t id : members) membership[id] = group;
    std::set<size_t> primary, scan;
    for (size_t id : members) {
      primary.insert(primary_inputs[id].begin(), primary_inputs[id].end());
      scan.insert(scan_inputs[id].begin(), scan_inputs[id].end());
      for (size_t parent : dependencies.at(id)) {
        if (membership.at(parent) == group) continue;
        primary.insert(primary_inputs[parent].begin(),
                       primary_inputs[parent].end());
        scan.insert(scan_inputs[parent].begin(), scan_inputs[parent].end());
      }
    }
    for (size_t id : members) {
      primary_inputs[id] = primary;
      scan_inputs[id] = scan;
    }
    groups.push_back(
        {members,
         members.size() > 1 ||
             dependencies.at(members.front()).count(members.front()) != 0});
    order.insert(order.end(), members.begin(), members.end());
  }
  std::map<size_t, std::set<BitKey>> reset_seen;
  std::map<size_t, std::set<size_t>> reset_consumers;
  for (size_t target = 0; target < netlist.nodes.size(); ++target) {
    const auto& node = netlist.nodes[target];
    if (!node.reset) continue;
    std::vector<BitReference> pending{*node.reset};
    std::set<std::pair<size_t, int64_t>> visited;
    std::set<size_t> expanded_controls;
    while (!pending.empty()) {
      const auto ref = pending.back();
      pending.pop_back();
      if (!remaining--) {
        throw std::runtime_error(
            "data-register reset dependency resource limit exceeded");
      }
      if (ref.node == std::numeric_limits<size_t>::max() ||
          !visited.emplace(ref.node, ref.index).second) {
        continue;
      }
      const auto& source = netlist.nodes.at(ref.node);
      if (source.model == ModelKind::kRegister) {
        reset_consumers[ref.node].insert(target);
        const auto found = membership.find(ref.node);
        if (found != membership.end() && groups[found->second].feedback &&
            (!membership.count(target) ||
             membership.at(target) != found->second)) {
          const auto reset = *node.reset;
          if (reset_seen[found->second]
                  .emplace(reset.node, reset.index, reset.inverted,
                           reset.constant, reset.virtual_signal)
                  .second) {
            groups[found->second].external_resets.push_back(reset);
          }
        }
        continue;
      }
      const bool controls = expanded_controls.insert(ref.node).second;
      AppendDependencySources(source, ref.index, controls, &pending);
      if (controls) {
        for (const auto& contributor : source.one_hot_sources) {
          for (const auto& condition : one_hot.Resolve(contributor)) {
            pending.push_back(condition.enable);
            if (condition.signals) {
              pending.insert(pending.end(), condition.signals->begin(),
                             condition.signals->end());
            }
          }
        }
      }
    }
  }
  const std::set<std::string> ordinary_ports{
      "DataInPort", "DataOutPort", "ScanInPort", "ScanOutPort",
      "ResetPort",  "AddressPort", "ReadEnPort", "WriteEnPort"};
  if (!netlist.taps.empty()) return;
  for (const auto& node : netlist.nodes) {
    if (node.model == ModelKind::kInterface ||
        node.model == ModelKind::kAccessLink ||
        (node.model == ModelKind::kPort && !ordinary_ports.count(node.kind))) {
      return;
    }
  }
  // Saturate at two origins: ambiguity cannot become unique downstream.
  std::map<size_t, std::set<size_t>> owners;
  std::deque<size_t> pending;
  for (const auto& [id, group] : membership) {
    owners[id].insert(group);
    pending.push_back(id);
  }
  while (!pending.empty()) {
    const auto id = pending.front();
    pending.pop_front();
    const auto edges = reset_consumers.find(id);
    if (edges == reset_consumers.end()) continue;
    for (size_t target : edges->second) {
      auto& origins = owners[target];
      const auto previous = origins.size();
      for (size_t origin : owners.at(id)) {
        if (origins.size() == 2) break;
        origins.insert(origin);
      }
      if (origins.size() != previous) pending.push_back(target);
    }
  }
  std::set<size_t> data_consumers;
  for (const auto& [id, sources] : scan_inputs) {
    data_consumers.insert(sources.begin(), sources.end());
  }
  for (size_t index = 0; index < groups.size(); ++index) {
    auto& group = groups[index];
    if (!group.feedback) continue;
    std::set<size_t> targets;
    std::vector<size_t> todo = group.nodes;
    bool qualified = true;
    while (!todo.empty() && qualified) {
      const auto id = todo.back();
      todo.pop_back();
      const auto edges = reset_consumers.find(id);
      if (edges == reset_consumers.end()) continue;
      for (size_t target : edges->second) {
        if (membership.count(target) && membership.at(target) == index) {
          continue;
        }
        if (owners.at(target).size() != 1 ||
            *owners.at(target).begin() != index ||
            netlist.nodes[target].kind != "ScanRegister" ||
            data_consumers.count(target)) {
          qualified = false;
          break;
        }
        if (targets.insert(target).second) todo.push_back(target);
      }
    }
    if (!qualified || targets.empty()) continue;
    std::map<size_t, size_t> waiting;
    for (size_t id : targets) waiting[id] = 0;
    for (size_t id : targets) {
      const auto edges = reset_consumers.find(id);
      if (edges == reset_consumers.end()) continue;
      for (size_t target : edges->second) {
        if (targets.count(target)) ++waiting.at(target);
      }
    }
    std::vector<size_t> ready;
    for (const auto& [id, count] : waiting) {
      if (!count) ready.push_back(id);
    }
    size_t visited = 0;
    while (!ready.empty()) {
      const auto id = ready.back();
      ready.pop_back();
      ++visited;
      const auto edges = reset_consumers.find(id);
      if (edges == reset_consumers.end()) continue;
      for (size_t target : edges->second) {
        if (targets.count(target) && !--waiting.at(target)) {
          ready.push_back(target);
        }
      }
    }
    if (visited != targets.size()) continue;
    group.reset_targets.assign(targets.begin(), targets.end());
    for (size_t id : targets) {
      const auto edges = reset_consumers.find(id);
      if (edges == reset_consumers.end()) continue;
      for (size_t target : edges->second) {
        if (targets.count(target)) group.reset_consumers[id].push_back(target);
      }
    }
    group.external_resets.clear();
    feedback_reset_targets.insert(targets.begin(), targets.end());
  }
}
const std::vector<AddressRequirement>& OneHotReadResolver::Resolve(
    const OneHotSource& source) {
  const auto cached = cache_.find(&source);
  if (cached != cache_.end()) return cached->second;
  std::vector<BitReference> pending;
  std::set<size_t> visited;
  std::vector<AddressRequirement> conditions;
  size_t remaining = 4 * 1024 * 1024;
  const auto append = [&](const std::vector<BitReference>& bits) {
    if (bits.size() > remaining || (budget_ && bits.size() > *budget_)) {
      if (budget_) *budget_ = 0;
      throw std::runtime_error(
          "one-hot read dependency resource limit exceeded");
    }
    remaining -= bits.size();
    if (budget_) *budget_ -= bits.size();
    pending.insert(pending.end(), bits.begin(), bits.end());
  };
  append(source.bits);
  while (!pending.empty()) {
    const auto ref = pending.back();
    pending.pop_back();
    if (ref.node == std::numeric_limits<size_t>::max()) {
      throw std::runtime_error("one-hot contributor has no selectable source");
    }
    if (!visited.insert(ref.node).second) continue;
    const auto& node = netlist_.nodes.at(ref.node);
    if (node.kind == "DataRegister" && node.constants.count("AddressValue")) {
      if (!addresses_) addresses_ = std::make_unique<AddressResolver>(netlist_);
      conditions.push_back(
          addresses_->Resolve(node.name, AddressOperation::kRead));
      continue;
    }
    if (node.kind == "OneHotDataGroup" || node.kind == "OneHotScanGroup") {
      for (const auto& nested : node.one_hot_sources) {
        append(nested.bits);
      }
      continue;
    }
    if (node.kind == "DataOutPort" || node.kind == "ScanOutPort") {
      const auto enable = node.inputs.find("Enable");
      if (enable != node.inputs.end()) {
        conditions.push_back({nullptr, {}, enable->second.at(0)});
        continue;
      }
    }
    if (node.kind == "DataOutPort" || node.kind == "ScanOutPort" ||
        node.kind == "ScanInPort" || node.model == ModelKind::kAlias) {
      const auto input = node.inputs.find("Source");
      if (input != node.inputs.end()) {
        append(input->second);
        continue;
      }
    }
    throw std::runtime_error(node.origin +
                             ": one-hot Port execution requires a uniformly "
                             "enabled output-port path or addressed register");
  }
  if (conditions.empty()) {
    throw std::runtime_error("one-hot contributor has no selection condition");
  }
  return cache_.emplace(&source, std::move(conditions)).first->second;
}
}  // namespace pattern
