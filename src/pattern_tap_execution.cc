// Copyright 2026 The ICL Toolsets Authors.
// SPDX-License-Identifier: MIT
#include "pattern_tap_execution.h"

#include <algorithm>
#include <stdexcept>
#include <tuple>
#include <utility>

namespace pattern {
namespace {
constexpr size_t kMissing = std::numeric_limits<size_t>::max();
char And(char a, char b) {
  return a == '0' || b == '0' ? '0' : a == '1' && b == '1' ? '1' : 'x';
}
char Or(char a, char b) {
  return a == '1' || b == '1' ? '1' : a == '0' && b == '0' ? '0' : 'x';
}
char Match(const std::string& bits, const std::string& code) {
  char result = '1';
  for (size_t i = 0; i < bits.size(); ++i) {
    if (code.at(i) == 'x') {
      continue;
    }
    if (bits[i] == 'x') {
      result = 'x';
    } else if (bits[i] != code[i]) {
      return '0';
    }
  }
  return result;
}
}  // namespace
TapExecution::TapExecution(const Netlist& netlist)
    : netlist_(netlist), state_(netlist) {
  for (const auto& [name, tap] : netlist.taps) {
    controllers_.emplace(name, TapInstructionRegister(tap.bsdl));
    Synchronize(name);
    std::map<size_t, char> owned;
    for (const auto& [instruction, id] : tap.scan_returns) {
      Trace({id, netlist.nodes.at(id).left}, '1', &owned, true);
    }
    for (const auto& [reg, id] : tap.data_registers) {
      owned[id] = '1';
    }
    for (const auto& [id, enabled] : owned) {
      owned_registers_[tap.instruction_register].insert(id);
    }
  }
  // Associate only SelectPorts listed by an interface containing this register.
  // A stand-alone SelectPort has DataInPort semantics (IEEE 6.4.6.11).
  for (const auto& interface : EffectiveScanInterfaces(netlist)) {
    const auto bind = [&](const std::vector<BitReference>& chain) {
      std::map<size_t, char> members;
      std::set<std::pair<size_t, int64_t>> stops;
      for (const auto& port : chain) {
        if (netlist.nodes.at(port.node).kind == "ScanInPort") {
          stops.emplace(port.node, port.index);
        }
      }
      for (const auto& port : chain) {
        if (netlist.nodes.at(port.node).kind == "ScanOutPort") {
          Trace(port, '1', &members, true, stops);
        }
      }
      for (const auto& [id, selected] : members) {
        if (netlist.nodes[id].scope != interface.scope) {
          continue;
        }
        InterfacePorts controls;
        for (const auto& port : chain) {
          const auto& node = netlist.nodes.at(port.node);
          controls[node.kind].insert({port.node, port.index});
        }
        for (const auto& port : interface.interface.ports) {
          const auto& node = netlist.nodes.at(port.node);
          controls[node.kind].insert({port.node, port.index});
        }
        register_ports_[id].push_back(std::move(controls));
      }
    };
    if (interface.interface.chains.empty()) {
      bind(interface.interface.ports);
    } else {
      for (const auto& chain : interface.interface.chains) {
        bind(chain.ports);
      }
    }
  }
}
void TapExecution::Synchronize(const std::string& name) {
  const auto& tap = netlist_.taps.at(name);
  const auto& controller = controllers_.at(name);
  auto code = controller.InstructionCodes().at(0);
  for (const auto& possible : controller.InstructionCodes()) {
    for (size_t i = 0; i < code.size(); ++i) {
      if (code[i] != possible[i]) {
        code[i] = 'x';
      }
    }
  }
  state_.SetRegister(tap.instruction_register, controller.ShiftBits(), code);
  state_.Drive(tap.ir_select,
               std::string(1, controller.Controller().IrSelect()));
}
void TapExecution::Drive(const std::string& port, const std::string& value) {
  const size_t id = netlist_.node_ids.at(port);
  state_.Drive(id, value);
  for (const auto& [name, tap] : netlist_.taps) {
    const auto reset = tap.pins.find("tap_scan_reset");
    if (reset != tap.pins.end() && reset->second == id) {
      controllers_.at(name).SetTrst(value.at(0));
      Synchronize(name);
    }
  }
  GlobalReset();
  state_.Reset();
}
void TapExecution::GlobalReset() {
  // Virtual reset has no unique owner in a multi-TAP design.
  if (controllers_.size() == 1) {
    const auto& [name, controller] = *controllers_.begin();
    const auto states = controller.Controller().PossibleStates();
    const auto reset = TapStateBit(TapState::kTestLogicReset);
    char async = '0';
    const auto& tap = netlist_.taps.at(name);
    const auto pin = tap.pins.find("tap_scan_reset");
    if (pin != tap.pins.end()) {
      const char trst = state_.Value(pin->second).at(0);
      async = trst == '0' ? '1' : trst == '1' ? '0' : 'x';
    }
    state_.SetGlobalReset(states == reset  ? '1'
                          : states & reset ? 'x'
                                           : '0',
                          async);
  }
}
std::string TapExecution::Value(const std::string& path, bool scan) const {
  return state_.Value(netlist_.node_ids.at(path), scan);
}
void TapExecution::Trace(
    BitReference output, char selected, std::map<size_t, char>* active,
    bool all, const std::set<std::pair<size_t, int64_t>>& stops) const {
  std::vector<std::pair<BitReference, char>> pending{{output, selected}};
  std::map<std::pair<size_t, int64_t>, char> visited;
  while (!pending.empty()) {
    const auto [bit, gate] = pending.back();
    pending.pop_back();
    if (gate == '0' || bit.node == kMissing) {
      continue;
    }
    const auto key = std::make_pair(bit.node, bit.index);
    if (stops.count(key)) {
      continue;
    }
    const auto old = visited.find(key);
    if (old != visited.end() && (old->second == gate || old->second == '1')) {
      continue;
    }
    visited[key] = gate;
    const auto& node = netlist_.nodes.at(bit.node);
    const size_t offset =
        node.left >= node.right ? node.left - bit.index : bit.index - node.left;
    if (node.kind == "ScanRegister") {
      auto found = active->find(bit.node);
      if (found == active->end()) {
        active->emplace(bit.node, gate);
      } else {
        found->second = Or(found->second, gate);
      }
      pending.emplace_back(node.inputs.at("ScanInSource").at(0), gate);
    } else if (node.model == ModelKind::kMux) {
      std::string selector;
      if (!all) {
        for (const auto& ref : node.inputs.at("SelectedBy")) {
          selector += state_.Read(ref);
        }
      }
      for (const auto& choice : node.choices) {
        char matches = all ? '1' : '0';
        if (!all) {
          for (const auto& code : choice.codes) {
            matches = Or(matches, Match(selector, code.data));
          }
        }
        pending.emplace_back(choice.source.at(offset), And(gate, matches));
      }
    } else if (node.kind == "OneHotScanGroup") {
      for (size_t i = 0; i < node.one_hot_sources.size(); ++i) {
        pending.emplace_back(node.OneHotSourceBit(i, offset), gate);
      }
    } else {
      const auto input = node.inputs.find("Source");
      if (input != node.inputs.end()) {
        pending.emplace_back(input->second.at(offset), gate);
      }
    }
  }
}
std::map<size_t, char> TapExecution::Active(const TapRuntimeModel& tap) const {
  std::map<size_t, char> active;
  for (const auto& [instruction, id] : tap.scan_returns) {
    const auto& port = netlist_.nodes.at(id);
    Trace({id, port.left},
          state_.Value(tap.instruction_selects.at(instruction)).at(0), &active);
  }
  const auto codes = state_.Value(tap.instruction_register);
  static const std::map<std::string, std::string> kStandard = {
      {"BYPASS", "BYPASS"},     {"CLAMP", "BYPASS"},
      {"HIGHZ", "BYPASS"},      {"EXTEST", "BOUNDARY"},
      {"INTEST", "BOUNDARY"},   {"PRELOAD", "BOUNDARY"},
      {"SAMPLE", "BOUNDARY"},   {"IDCODE", "DEVICE_ID"},
      {"USERCODE", "DEVICE_ID"}};
  char matched = '0';
  for (const auto& [instruction, alternatives] : tap.bsdl.opcodes) {
    char select = '0';
    for (const auto& code : alternatives) {
      select = Or(select, Match(codes, code));
    }
    matched = Or(matched, select);
    if (tap.scan_returns.count(instruction)) {
      continue;
    }
    const auto association = tap.bsdl.registers.find(instruction);
    const auto standard = kStandard.find(instruction);
    if (association == tap.bsdl.registers.end() &&
        standard == kStandard.end()) {
      throw std::runtime_error("missing TAP register association");
    }
    const auto reg = association == tap.bsdl.registers.end()
                         ? standard->second
                         : association->second.name;
    const size_t id = tap.data_registers.at(reg);
    active[id] = Or(active.count(id) ? active[id] : '0', select);
  }
  if (matched != '1') {
    const size_t bypass = tap.data_registers.at("BYPASS");
    active[bypass] = Or(active.count(bypass) ? active[bypass] : '0',
                        matched == '0' ? '1' : 'x');
  }
  return active;
}
std::map<size_t, ScanControl> TapExecution::Controls(
    const TapRuntimeModel& tap, const TapActions& actions,
    const std::map<std::pair<size_t, int64_t>, char>& clocks) const {
  const auto active = Active(tap);
  std::map<size_t, ScanControl> result;
  for (size_t id : owned_registers_.at(tap.instruction_register)) {
    const auto selected_entry = active.find(id);
    const char selected =
        selected_entry == active.end() ? '0' : selected_entry->second;
    if (id == tap.instruction_register) {
      continue;
    }
    ScanControl combined{'0', '0', '0', '1'};
    const auto accumulate = [&](const InterfacePorts& interface) {
      const auto control = [&](const std::string& kind, char fallback) {
        const auto ports = interface.find(kind);
        if (ports == interface.end()) {
          return fallback;
        }
        char value = '1';
        for (const auto& [port, index] : ports->second) {
          const auto& node = netlist_.nodes.at(port);
          const char driven =
              (node.inputs.count("Source") || state_.IsDriven(port))
                  ? state_.Read({port, index})
                  : fallback;
          value = And(value, driven);
        }
        return value;
      };
      char enabled = control("SelectPort", selected);
      const auto clock_ports = interface.find("TCKPort");
      if (clock_ports != interface.end()) {
        for (const auto& pin : clock_ports->second) {
          enabled = And(enabled, clocks.at(pin));
        }
      }
      combined.capture =
          Or(combined.capture,
             And(enabled, control("CaptureEnPort", actions.capture_dr)));
      combined.shift =
          Or(combined.shift,
             And(enabled, control("ShiftEnPort", actions.shift_dr)));
      combined.update =
          Or(combined.update,
             And(enabled, control("UpdateEnPort", actions.update_dr)));
    };
    const auto found = register_ports_.find(id);
    if (found == register_ports_.end()) {
      accumulate({});
    } else {
      for (const auto& interface : found->second) {
        accumulate(interface);
      }
    }
    result.emplace(id, combined);
  }
  return result;
}
void TapExecution::Edge(const std::string& clock, bool rising) {
  const size_t clock_id = netlist_.node_ids.at(clock);
  if (netlist_.nodes.at(clock_id).kind != "TCKPort") {
    throw std::runtime_error("TAP edge requires a TCKPort");
  }
  // The API reports an observed external edge, establishing both levels.
  state_.Drive(clock_id, rising ? "0" : "1");
  std::map<std::pair<size_t, int64_t>, char> clocks;
  for (const auto& [reg, interfaces] : register_ports_) {
    for (const auto& ports : interfaces) {
      const auto found = ports.find("TCKPort");
      if (found == ports.end()) {
        continue;
      }
      for (const auto& pin : found->second) {
        if (!clocks.count(pin)) {
          clocks.emplace(pin, state_.Read({pin.first, pin.second}));
        }
      }
    }
  }
  state_.Drive(clock_id, rising ? "1" : "0");
  for (auto& [pin, before] : clocks) {
    const char after = state_.Read({pin.first, pin.second});
    const char from = rising ? '0' : '1', to = rising ? '1' : '0';
    if (before == to && after == from) {
      throw std::runtime_error(netlist_.nodes.at(pin.first).origin +
                               ": inverted child TCK phase is not supported");
    }
    before = before == to || after == from   ? '0'
             : before == from && after == to ? '1'
                                             : 'x';
  }
  std::map<size_t, ScanControl> controls;
  std::vector<std::string> changed;
  for (const auto& [name, tap] : netlist_.taps) {
    if (tap.pins.at("tap_scan_clock") != clock_id) {
      continue;
    }
    auto& controller = controllers_.at(name);
    const auto actions =
        rising ? controller.RisingEdge(
                     state_.Value(tap.pins.at("tap_scan_mode")).at(0),
                     state_.Value(tap.pins.at("tap_scan_in")).at(0))
               : controller.FallingEdge();
    state_.Drive(tap.controls.at("capture"),
                 std::string(1, actions.capture_dr));
    state_.Drive(tap.controls.at("shift"), std::string(1, actions.shift_dr));
    state_.Drive(tap.controls.at("update"), std::string(1, actions.update_dr));
    for (const auto& [id, events] : Controls(tap, actions, clocks)) {
      if (!controls.emplace(id, events).second) {
        throw std::runtime_error(netlist_.nodes.at(id).origin +
                                 ": register controlled by multiple TAPs");
      }
    }
    changed.push_back(name);
  }
  if (changed.empty()) {
    throw std::runtime_error("clock does not drive any TAP");
  }
  GlobalReset();
  state_.Clock(controls, rising);
  for (const auto& name : changed) {
    Synchronize(name);
  }
  state_.Reset();
}
void TapExecution::RisingEdge(const std::string& clock) { Edge(clock, true); }
void TapExecution::FallingEdge(const std::string& clock) { Edge(clock, false); }
}  // namespace pattern
