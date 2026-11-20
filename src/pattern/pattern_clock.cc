// Copyright 2026 The ICL Toolsets Authors.
// SPDX-License-Identifier: MIT
#include "pattern/pattern_clock.h"

#include <algorithm>
#include <charconv>
#include <limits>
#include <numeric>
#include <stdexcept>
#include <vector>

#include "pattern/pattern_flatten.h"
#include "pattern/pattern_values.h"

namespace pattern {
namespace {
bool ClockPort(const NetlistNode& node) {
  return node.kind == "ClockPort" || node.kind == "ToClockPort";
}
size_t Offset(const NetlistNode& node, int64_t index) {
  return static_cast<size_t>(node.left >= node.right ? node.left - index
                                                     : index - node.left);
}
int64_t Index(const NetlistNode& node, size_t offset) {
  return node.left +
         (node.left <= node.right ? 1 : -1) * static_cast<int64_t>(offset);
}
uint64_t Factor(const NetlistNode& node, const std::string& name) {
  const auto found = node.constants.find(name);
  return found == node.constants.end() ? 1 : found->second.AsInteger();
}
uint64_t Product(uint64_t a, uint64_t b) {
  if (a > std::numeric_limits<uint64_t>::max() / b) {
    throw std::runtime_error("system-clock frequency ratio limit exceeded");
  }
  return a * b;
}
}  // namespace
const SystemClocks::Definition* SystemClocks::DefinitionAt(
    Bit bit, size_t version) const {
  const auto found = overrides_.find(bit);
  if (found == overrides_.end()) return nullptr;
  const auto& history = found->second;
  const auto next = std::upper_bound(history.begin(), history.end(), version,
                                     [](size_t value, const Definition& entry) {
                                       return value < entry.version;
                                     });
  return next == history.begin() ? nullptr : &*std::prev(next);
}
void SystemClocks::CheckCycle(Bit target, BitReference source) const {
  std::vector<BitReference> pending{source};
  std::set<Bit> visited;
  while (!pending.empty()) {
    const auto bit = pending.back();
    pending.pop_back();
    if (bit.node >= netlist_.nodes.size()) continue;
    const Bit key{bit.node, bit.index};
    if (key == target) throw std::runtime_error("cyclic system-clock override");
    if (!visited.insert(key).second) continue;
    if (const auto* definition = DefinitionAt(key, version_)) {
      pending.push_back(definition->source);
      continue;
    }
    const auto& node = netlist_.nodes[bit.node];
    const size_t offset = Offset(node, bit.index);
    for (const auto* role : {"Source", "DifferentialInvOf"}) {
      const auto input = node.inputs.find(role);
      if (input != node.inputs.end()) {
        pending.push_back(input->second.at(offset));
      }
    }
    for (const auto& choice : node.choices) {
      pending.push_back(choice.source.at(offset));
    }
  }
}
void SystemClocks::Override(const Operation& operation, z3::context& context) {
  const auto& args = operation.arguments;
  if (args.empty() || args.size() % 2 == 0) {
    throw std::runtime_error("invalid iClockOverride arguments");
  }
  const auto target = Target(ClockTargetName(args[0]));
  const auto& node = netlist_.nodes[target.first];
  if (node.kind != "ToClockPort") {
    throw std::runtime_error(
        "iClockOverride target requires a ToClockPort bit");
  }
  if (node.properties.count("DifferentialInvOf")) {
    throw std::runtime_error(
        "oscillator/differential clock override is unsupported");
  }
  Definition definition;
  definition.version = version_ + 1;
  definition.multiplier = Factor(node, "FreqMultiplier");
  definition.divider = Factor(node, "FreqDivider");
  const auto source = node.inputs.find("Source");
  if (source != node.inputs.end()) {
    definition.source = source->second.at(Offset(node, target.second));
  }
  std::set<std::string> options;
  for (size_t i = 1; i < args.size(); i += 2) {
    const auto& option = args[i];
    if (!options.insert(option).second) {
      throw std::runtime_error("duplicate iClockOverride option");
    }
    if (option == "-source") {
      const auto bit = Target(ClockTargetName(args[i + 1]));
      definition.source = {bit.first, bit.second};
    } else if (option == "-freqMultiplier" || option == "-freqDivider") {
      const auto bits = PdlNumber(context, args[i + 1], 64);
      uint64_t value = 0;
      for (char bit : bits) {
        if (bit == 'x') {
          throw std::runtime_error(
              "clock override factor must be known and positive");
        }
        value = (value << 1) | (bit == '1');
      }
      if (!value) {
        throw std::runtime_error("clock override factor must be positive");
      }
      (option == "-freqMultiplier" ? definition.multiplier
                                   : definition.divider) = value;
    } else {
      throw std::runtime_error("unknown iClockOverride option: " + option);
    }
  }
  // Factor-only updates may leave only an earlier checked oscillator usable.
  if (node.clock_period && options.empty()) {
    throw std::runtime_error("oscillator override requires explicit -source");
  }
  if (!node.clock_period && source == node.inputs.end() &&
      !options.count("-source")) {
    throw std::runtime_error("clock override requires a source");
  }
  BuildSafety();
  if (unsafe_.count(target)) {
    throw std::runtime_error("clock override target has unsupported fanout: " +
                             Name(target));
  }
  CheckCycle(target, definition.source);
  overrides_[target].push_back(definition);
  version_ = definition.version;
  if (definition.source.node < netlist_.nodes.size()) {
    fanout_[{definition.source.node, definition.source.index}].insert(target);
  }
}
void SystemClocks::Invalidate() {
  routes_.clear();
  values_.clear();
}
std::string SystemClocks::ClockTargetName(const std::string& text) const {
  const auto open = text.find_first_of("[(");
  const auto name = text.substr(0, open);
  if (netlist_.node_ids.count(name) || netlist_.port_groups.count(name) ||
      name.size() < 3 || name[0] != 'n' || name.size() % 2 == 0 ||
      name.find_first_not_of("0123456789abcdef", 1) != std::string::npos) {
    return text;
  }
  std::string original;
  original.reserve((name.size() - 1) / 2);
  for (size_t i = 1; i < name.size(); i += 2) {
    unsigned int byte = 0;
    std::from_chars(name.data() + i, name.data() + i + 2, byte, 16);
    original.push_back(static_cast<char>(byte));
  }
  const auto found = netlist_.node_ids.find(original);
  if (found == netlist_.node_ids.end() ||
      !ClockPort(netlist_.nodes.at(found->second))) {
    return text;
  }
  if (!flat_origins_.has_value()) {
    flat_origins_ = false;
    if (!netlist_.scopes.empty() && netlist_.scopes[0].definition.ast) {
      for (const auto* item : netlist_.scopes[0].definition.ast->body()) {
        const auto* attribute = dynamic_cast<const icl::AstStatement*>(item);
        if (attribute &&
            attribute->named_kind() == icl::NamedKind::kAttribute &&
            attribute->name() == kFlatOriginAttribute) {
          flat_origins_ = true;
          break;
        }
      }
    }
  }
  if (!*flat_origins_) return text;
  if (open == std::string::npos) return original;
  const auto& node = netlist_.nodes[found->second];
  const auto mapped = node.BaseName() + text.substr(open);
  if (node.BaseName() != original) {
    const auto target = ResolvePdlTarget(netlist_, mapped);
    if (std::any_of(
            target.bits.begin(), target.bits.end(),
            [&](const auto& bit) { return bit.node != found->second; })) {
      throw std::runtime_error("clock index outside flattened port function " +
                               original);
    }
  }
  return mapped;
}
SystemClocks::Bit SystemClocks::Target(const std::string& name) const {
  const auto target = ResolvePdlTarget(netlist_, name);
  if (target.bits.size() != 1 || target.declaration >= netlist_.nodes.size() ||
      !ClockPort(netlist_.nodes.at(target.declaration))) {
    throw std::runtime_error(
        "system-clock target requires one ClockPort or "
        "ToClockPort bit");
  }
  return {target.bits[0].node, target.bits[0].index};
}
std::string SystemClocks::Name(Bit bit) const {
  const auto& node = netlist_.nodes.at(bit.first);
  return node.BaseName() +
         (node.indexed ? "(" + std::to_string(bit.second) + ")" : "");
}
BitReference SystemClocks::Source(Bit bit, const ScanScheduler& scheduler,
                                  size_t version) {
  if (const auto* definition = DefinitionAt(bit, version)) {
    return definition->source;
  }
  const auto& node = netlist_.nodes.at(bit.first);
  const size_t offset = Offset(node, bit.second);
  const auto differential = node.inputs.find("DifferentialInvOf");
  if (differential != node.inputs.end()) {
    auto source = differential->second.at(offset);
    source.inverted = !source.inverted;
    return source;
  }
  if (node.kind == "ClockMux") {
    std::string selected;
    for (const auto& signal : node.inputs.at("SelectedBy")) {
      char value = signal.constant;
      if (signal.node < netlist_.nodes.size()) {
        auto found = values_.find(signal.node);
        if (found == values_.end()) {
          found =
              values_
                  .emplace(signal.node,
                           scheduler.Value(netlist_.nodes[signal.node].name))
                  .first;
        }
        value =
            found->second.at(Offset(netlist_.nodes[signal.node], signal.index));
      }
      if (value != '0' && value != '1') {
        throw std::runtime_error(node.origin +
                                 ": unknown clock selector: " + node.name);
      }
      selected += signal.inverted ? (value == '0' ? '1' : '0') : value;
    }
    for (const auto& choice : node.choices) {
      for (const auto& code : choice.codes) {
        bool match = true;
        for (size_t i = 0; i < selected.size(); ++i) {
          if (code.data[i] != 'x' && code.data[i] != selected[i]) match = false;
        }
        if (match) return choice.source.at(offset);
      }
    }
    throw std::runtime_error(node.origin +
                             ": uncovered clock selector: " + node.name);
  }
  const auto source = node.inputs.find("Source");
  if ((!ClockPort(node) && node.model != ModelKind::kAlias) ||
      source == node.inputs.end()) {
    throw std::runtime_error(
        node.origin + ": no supported system-clock source: " + node.name);
  }
  return source->second.at(offset);
}
SystemClocks::Route SystemClocks::Resolve(Bit bit,
                                          const ScanScheduler& scheduler,
                                          size_t version) {
  // shortcut: one cached version; expand if 4096-stage/1024-wait runs exceed
  // 45s.
  if (cache_version_ != version) {
    routes_.clear();
    cache_version_ = version;
  }
  struct Frame {
    Bit bit;
    std::vector<BitReference> sources;
  };
  std::vector<Frame> pending;
  std::set<Bit> active;
  const auto push = [&](Bit next) {
    if (routes_.count(next)) return;
    if (!active.insert(next).second) {
      throw std::runtime_error("cyclic system-clock path");
    }
    const auto& node = netlist_.nodes.at(next.first);
    if ((node.kind == "ClockPort" && node.scope == 0 &&
         !node.inputs.count("Source") &&
         !node.inputs.count("DifferentialInvOf")) ||
        (node.kind == "ToClockPort" && node.clock_period &&
         !DefinitionAt(next, version))) {
      Route route;
      route.root = route.phase_origin = next;
      routes_.emplace(next, route);
      active.erase(next);
      return;
    }
    Frame frame{next, {Source(next, scheduler, version)}};
    if (node.kind == "ClockPort" && node.inputs.count("DifferentialInvOf") &&
        node.inputs.count("Source")) {
      frame.sources.push_back(
          node.inputs.at("Source").at(Offset(node, next.second)));
    }
    pending.push_back(std::move(frame));
  };
  push(bit);
  while (!pending.empty()) {
    const auto& frame = pending.back();
    const auto current = frame.bit;
    const auto& node = netlist_.nodes.at(current.first);
    bool ready = true;
    for (const auto& source : frame.sources) {
      if (source.node >= netlist_.nodes.size()) {
        throw std::runtime_error(node.origin + ": inactive system-clock path");
      }
      const Bit dependency{source.node, source.index};
      if (!routes_.count(dependency)) {
        push(dependency);
        ready = false;
        break;
      }
    }
    if (!ready) continue;
    const auto& source = frame.sources.front();
    auto route = routes_.at({source.node, source.index});
    route.inverted = route.inverted != source.inverted;
    if (frame.sources.size() == 2) {
      const auto& actual = frame.sources.back();
      const auto& binding = routes_.at({actual.node, actual.index});
      if (route.root != binding.root ||
          route.multiplier != binding.multiplier ||
          route.divider != binding.divider ||
          route.phase_origin != binding.phase_origin ||
          route.inverted != (binding.inverted != actual.inverted)) {
        throw std::runtime_error(node.origin +
                                 ": bound differential clock inputs are not "
                                 "provably complementary: " +
                                 node.name);
      }
    }
    const auto* definition = DefinitionAt(current, version);
    uint64_t multiplier =
        definition ? definition->multiplier : Factor(node, "FreqMultiplier");
    uint64_t divider =
        definition ? definition->divider : Factor(node, "FreqDivider");
    const auto common = std::gcd(multiplier, divider);
    multiplier /= common;
    divider /= common;
    const auto a = std::gcd(multiplier, route.divider);
    const auto b = std::gcd(divider, route.multiplier);
    route.multiplier = Product(route.multiplier / b, multiplier / a);
    route.divider = Product(route.divider / a, divider / b);
    if (multiplier != divider) {
      // Frequency conversion alone establishes no phase relation to its input.
      route.phase_origin = current;
      route.inverted = false;
    }
    routes_.emplace(current, route);
    active.erase(current);
    pending.pop_back();
  }
  return routes_.at(bit);
}
void SystemClocks::BuildSafety() {
  if (safety_ready_) return;
  std::map<Bit, std::vector<Bit>> upstream;
  const auto mark = [&](const BitReference& bit) {
    if (bit.node < netlist_.nodes.size()) unsafe_.emplace(bit.node, bit.index);
  };
  for (size_t id = 0; id < netlist_.nodes.size(); ++id) {
    const auto& node = netlist_.nodes[id];
    const auto edges = [&](const std::vector<BitReference>& bits,
                           bool routing) {
      for (size_t i = 0; i < bits.size(); ++i) {
        const auto& source = bits[i];
        if (source.node >= netlist_.nodes.size()) continue;
        if (routing) {
          const Bit destination{id, Index(node, i)};
          upstream[destination].emplace_back(source.node, source.index);
          fanout_[{source.node, source.index}].insert(destination);
        }
        if (!routing) mark(source);
      }
    };
    for (const auto& [role, bits] : node.inputs) {
      edges(bits, (role == "Source" || role == "DifferentialInvOf") &&
                      (ClockPort(node) || node.model == ModelKind::kAlias));
    }
    for (const auto& choice : node.choices) {
      edges(choice.source, node.kind == "ClockMux");
    }
    for (const auto& instruction : node.logic) {
      edges(instruction.signals, false);
    }
    for (const auto& source : node.one_hot_sources) edges(source.bits, false);
    if (node.reset) mark(*node.reset);
    if (node.scan_interface) {
      edges(node.scan_interface->ports, false);
      for (const auto& chain : node.scan_interface->chains) {
        edges(chain.ports, false);
      }
    }
  }
  std::vector<Bit> pending(unsafe_.begin(), unsafe_.end());
  for (size_t next = 0; next < pending.size(); ++next) {
    const auto found = upstream.find(pending[next]);
    if (found == upstream.end()) continue;
    for (const auto& source : found->second) {
      if (unsafe_.insert(source).second) pending.push_back(source);
    }
  }
  safety_ready_ = true;
}
void SystemClocks::CheckTimeWait(
    const std::map<size_t, std::string>& preserved) {
  if (!time_ready_) {
    if (!netlist_.taps.empty() ||
        std::any_of(netlist_.nodes.begin(), netlist_.nodes.end(),
                    [](const NetlistNode& node) {
                      return node.model == ModelKind::kRegister ||
                             node.model == ModelKind::kInterface ||
                             node.model == ModelKind::kAccessLink;
                    })) {
      throw std::runtime_error(
          "iRunLoop -time requires absolute period models and clock-event "
          "support for stateful or opaque hardware");
    }
    BuildSafety();
    for (const auto& bit : unsafe_) {
      const auto& node = netlist_.nodes.at(bit.first);
      if (ClockPort(node) || node.kind == "TCKPort") {
        throw std::runtime_error(
            "absolute-time wait has unsupported clock fanout: " + Name(bit));
      }
    }
    for (const auto& node : netlist_.nodes) {
      if (node.kind == "ToTCKPort" ||
          (node.kind == "TCKPort" && node.inputs.count("Source"))) {
        throw std::runtime_error(
            "absolute-time wait has unsupported test-clock forwarding: " +
            node.name);
      }
    }
    time_ready_ = true;
  }
  for (const auto& [id, value] : preserved) {
    const auto& node = netlist_.nodes.at(id);
    if (!ClockPort(node) && node.kind != "TCKPort") continue;
    const auto bit = value.find_first_of("01");
    if (bit != std::string::npos) {
      throw std::runtime_error(
          "absolute-time wait conflicts with held clock: " +
          Name({id, Index(node, bit)}));
    }
  }
}
std::string SystemClocks::Check(const std::string& target,
                                const ScanScheduler& scheduler) {
  const auto bit = Target(ClockTargetName(target));
  const auto route = Resolve(bit, scheduler, version_);
  BuildSafety();
  if (unsafe_.count(route.root)) {
    throw std::runtime_error("system-clock root has unsupported fanout: " +
                             Name(route.root));
  }
  checked_[bit] = {route, version_};
  return Name(route.root);
}
std::pair<std::string, uint64_t> SystemClocks::Wait(
    const std::string& target, uint64_t cycles, const ScanScheduler& scheduler,
    const std::map<size_t, std::string>& preserved) {
  const auto bit = Target(ClockTargetName(target));
  const auto checked = checked_.find(bit);
  if (checked == checked_.end()) {
    throw std::runtime_error("system-clock wait requires prior iClock");
  }
  const auto route = Resolve(bit, scheduler, checked->second.version);
  const auto& previous = checked->second.route;
  if (route.root != previous.root || route.multiplier != previous.multiplier ||
      route.divider != previous.divider) {
    throw std::runtime_error(
        "system-clock source or ratio changed since iClock; repeat iClock");
  }
  if (cycles) {
    std::set<Bit> held;
    for (const auto& [id, value] : preserved) {
      const auto& node = netlist_.nodes.at(id);
      if (!ClockPort(node)) continue;
      for (size_t i = 0; i < value.size(); ++i) {
        if (value[i] == '0' || value[i] == '1') {
          held.emplace(id, Index(node, i));
        }
      }
    }
    if (!held.empty()) {
      std::vector<Bit> pending{route.root};
      std::set<Bit> visited{route.root};
      for (size_t next = 0; next < pending.size(); ++next) {
        const auto current = pending[next];
        if (held.count(current) &&
            Resolve(current, scheduler, version_).root == route.root) {
          throw std::runtime_error(
              "system-clock wait conflicts with held clock: " + Name(current));
        }
        const auto found = fanout_.find(current);
        if (found == fanout_.end()) continue;
        for (const auto& output : found->second) {
          if (visited.insert(output).second) pending.push_back(output);
        }
      }
    }
  }
  const __uint128_t product = static_cast<__uint128_t>(cycles) * route.divider;
  const auto count =
      product / route.multiplier + (product % route.multiplier != 0);
  if (count > std::numeric_limits<uint64_t>::max()) {
    throw std::runtime_error("system-clock cycle count limit exceeded");
  }
  return {Name(route.root), static_cast<uint64_t>(count)};
}
}  // namespace pattern
