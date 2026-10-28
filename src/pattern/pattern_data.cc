// Copyright 2026 The ICL Toolsets Authors.
// SPDX-License-Identifier: MIT
#include "pattern/pattern_data.h"

#include <set>
#include <stdexcept>
#include <tuple>

namespace pattern {
DataReadRoutes::DataReadRoutes(const Netlist& netlist, bool scan_capture) {
  constexpr size_t kMissing = std::numeric_limits<size_t>::max();
  struct Pending {
    BitReference bit;
    DataReadRoute route;
    bool direct = true;
  };
  size_t remaining = 262144;
  for (size_t id = 0; id < netlist.nodes.size(); ++id) {
    const auto& output = netlist.nodes[id];
    if (scan_capture ? (output.kind != "ScanRegister" ||
                        !output.inputs.count("CaptureSource"))
                     : (output.scope != 0 || output.kind != "DataOutPort")) {
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
          if (node.kind == "DataRegister") {
            storage[{id, index}].insert(bit.node);
          }
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
  std::map<size_t, std::set<size_t>> dependencies, consumers;
  size_t remaining = 4 * 1024 * 1024;
  for (const auto& [id, activation] : writes) {
    const auto& node = netlist.nodes[id];
    std::vector<BitReference> pending = node.inputs.at("WriteDataSource");
    pending.push_back(activation.enable);
    if (activation.signals) {
      pending.insert(pending.end(), activation.signals->begin(),
                     activation.signals->end());
    }
    if (node.reset) pending.push_back(*node.reset);
    std::set<size_t> visited;
    auto& sources = dependencies[id];
    while (!pending.empty()) {
      const auto ref = pending.back();
      pending.pop_back();
      if (!remaining--) {
        throw std::runtime_error(
            "data-register dependency resource limit exceeded");
      }
      if (ref.node == std::numeric_limits<size_t>::max() ||
          !visited.insert(ref.node).second) {
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
      for (const auto& [name, bits] : source.inputs) {
        pending.insert(pending.end(), bits.begin(), bits.end());
      }
      for (const auto& choice : source.choices) {
        pending.insert(pending.end(), choice.source.begin(),
                       choice.source.end());
      }
      for (const auto& source : source.one_hot_sources) {
        pending.insert(pending.end(), source.bits.begin(), source.bits.end());
      }
      for (const auto& term : source.logic) {
        pending.insert(pending.end(), term.signals.begin(), term.signals.end());
      }
    }
    for (size_t source : sources) consumers[source].insert(id);
  }
  std::set<std::pair<std::string, size_t>> ready;
  for (const auto& [id, sources] : dependencies) {
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
      dependencies[consumer].erase(id);
      if (dependencies[consumer].empty()) {
        ready.emplace(netlist.nodes[consumer].name, consumer);
      }
    }
  }
  if (order.size() != writes.size()) {
    throw std::runtime_error(
        "cyclic data-register update dependencies are not supported");
  }
}
const std::vector<AddressRequirement>& OneHotReadResolver::Resolve(
    const OneHotSource& source) {
  const auto cached = cache_.find(&source);
  if (cached != cache_.end()) return cached->second;
  std::vector<BitReference> pending = source.bits;
  std::set<size_t> visited;
  std::vector<AddressRequirement> conditions;
  size_t remaining = 4 * 1024 * 1024;
  while (!pending.empty()) {
    const auto ref = pending.back();
    pending.pop_back();
    if (!remaining--) {
      throw std::runtime_error(
          "one-hot read dependency resource limit exceeded");
    }
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
    if (node.kind == "OneHotDataGroup") {
      for (const auto& nested : node.one_hot_sources) {
        pending.insert(pending.end(), nested.bits.begin(), nested.bits.end());
      }
      continue;
    }
    if (node.kind == "DataOutPort") {
      const auto enable = node.inputs.find("Enable");
      if (enable != node.inputs.end()) {
        conditions.push_back({nullptr, {}, enable->second.at(0)});
        continue;
      }
    }
    if (node.kind == "DataOutPort" || node.model == ModelKind::kAlias) {
      const auto input = node.inputs.find("Source");
      if (input != node.inputs.end()) {
        pending.insert(pending.end(), input->second.begin(),
                       input->second.end());
        continue;
      }
    }
    throw std::runtime_error(node.origin +
                             ": one-hot Port execution requires a uniformly "
                             "enabled DataOutPort path or addressed register");
  }
  if (conditions.empty()) {
    throw std::runtime_error("one-hot contributor has no selection condition");
  }
  return cache_.emplace(&source, std::move(conditions)).first->second;
}
}  // namespace pattern
