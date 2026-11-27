// Copyright 2026 The ICL Toolsets Authors.
// SPDX-License-Identifier: MIT
#include "pattern/pattern_state.h"

#include <algorithm>
#include <set>
#include <stdexcept>
#include <utility>

#include "pattern/pattern_data.h"
#include "pattern/pattern_logic.h"

namespace pattern {
namespace {
constexpr size_t kMissing = std::numeric_limits<size_t>::max();
char Invert(char value) {
  return value == '0' ? '1' : value == '1' ? '0' : 'x';
}
char Or(char a, char b) {
  return a == '1' || b == '1' ? '1' : a == '0' && b == '0' ? '0' : 'x';
}
char And(char a, char b) {
  return a == '0' || b == '0' ? '0' : a == '1' && b == '1' ? '1' : 'x';
}
void Validate(const std::string& bits, size_t width) {
  if (bits.size() != width ||
      bits.find_first_not_of("01x") != std::string::npos) {
    throw std::runtime_error(
        "state value requires matching width and 0/1/x bits");
  }
}
std::string Join(const std::string& a, const std::string& b) {
  auto result = a;
  for (size_t i = 0; i < result.size(); ++i) {
    if (result[i] != b.at(i)) {
      result[i] = 'x';
    }
  }
  return result;
}
std::string Choose(char enable, const std::string& value,
                   const std::string& previous) {
  return enable == '1'   ? value
         : enable == '0' ? previous
                         : Join(value, previous);
}
size_t Offset(const NetlistNode& node, int64_t index) {
  if (index < std::min(node.left, node.right) ||
      index > std::max(node.left, node.right)) {
    throw std::runtime_error("state bit index outside declared range");
  }
  return node.left >= node.right ? node.left - index : index - node.left;
}
}  // namespace

// Iterative dependency evaluation avoids C++ stack depth proportional to serial
// chain length. Each node/mode is evaluated once per event snapshot.
class SignalEvaluator {
 public:
  explicit SignalEvaluator(const NetworkState& state)
      : state_(state), netlist_(state.netlist_), one_hot_(netlist_) {}
  char Read(const BitReference& bit, bool scan = false) {
    char result = 'x';
    if (bit.node == kMissing) {
      result = bit.virtual_signal == VirtualSignal::kGlobalReset
                   ? state_.global_reset_
               : bit.virtual_signal == VirtualSignal::kAsyncGlobalReset
                   ? state_.async_reset_
               : bit.constant == '?' ? 'x'
                                     : bit.constant;
    } else {
      result = Value(bit.node, scan)
                   .at(Offset(netlist_.nodes.at(bit.node), bit.index));
    }
    return bit.inverted ? Invert(result) : result;
  }
  std::string Bits(const std::vector<BitReference>& bits, bool scan = false) {
    std::string result;
    result.reserve(bits.size());
    for (const auto& bit : bits) {
      result += Read(bit, scan);
    }
    return result;
  }
  const std::string& Value(size_t id, bool scan) {
    const Key requested{id, scan};
    const auto cached = cache_.find(requested);
    if (cached != cache_.end()) {
      return cached->second;
    }
    std::vector<std::pair<Key, bool>> stack{{requested, false}};
    while (!stack.empty()) {
      const auto [key, expanded] = stack.back();
      stack.pop_back();
      if (cache_.count(key)) {
        continue;
      }
      const auto& node = netlist_.nodes.at(key.first);
      if (expanded) {
        cache_.emplace(key, Compute(node, key));
        visiting_.erase(key);
        continue;
      }
      if (!visiting_.insert(key).second) {
        throw std::runtime_error(node.origin + ": cyclic state dependency");
      }
      stack.emplace_back(key, true);
      if (node.model == ModelKind::kRegister ||
          !state_.driven_.at(key.first).empty()) {
        continue;
      }
      std::set<Key> dependencies;
      const bool serial =
          key.second || node.kind == "ScanMux" || node.kind == "ScanInPort" ||
          node.kind == "ScanOutPort" || node.kind == "OneHotScanGroup";
      const auto add = [&](const BitReference& bit, bool mode) {
        if (bit.node != kMissing) {
          dependencies.emplace(bit.node, mode);
        }
      };
      for (const auto& [name, bits] : node.inputs) {
        for (const auto& bit : bits) {
          add(bit, name == "Source" ? serial : false);
        }
      }
      for (const auto& choice : node.choices) {
        for (const auto& bit : choice.source) {
          add(bit, serial);
        }
      }
      for (const auto& source : node.one_hot_sources) {
        for (const auto& bit : source.bits) add(bit, serial);
        for (const auto& condition : one_hot_.Resolve(source)) {
          add(condition.enable, false);
          if (condition.signals) {
            for (const auto& bit : *condition.signals) add(bit, false);
          }
        }
      }
      for (const auto& term : node.logic) {
        for (const auto& bit : term.signals) {
          add(bit, false);
        }
      }
      for (const auto& dependency : dependencies) {
        if (!cache_.count(dependency)) {
          stack.emplace_back(dependency, false);
        }
      }
    }
    return cache_.at(requested);
  }

  char Active(const AddressRequirement& condition) {
    char enabled = Read(condition.enable);
    if (!condition.signals) return enabled;
    const auto actual = Bits(*condition.signals);
    auto wanted = condition.value.data;
    wanted.insert(0, actual.size() - wanted.size(), '0');
    for (size_t i = 0; i < actual.size(); ++i) {
      enabled = And(enabled, actual[i] == 'x'         ? 'x'
                             : actual[i] == wanted[i] ? '1'
                                                      : '0');
    }
    return enabled;
  }

 private:
  using Key = std::pair<size_t, bool>;
  std::string Compute(const NetlistNode& node, Key key) {
    if (node.model == ModelKind::kRegister) {
      return key.second && node.kind == "ScanRegister"
                 ? state_.shift_.at(key.first)
                 : state_.update_.at(key.first);
    }
    if (!state_.driven_.at(key.first).empty()) {
      return state_.driven_.at(key.first);
    }
    const std::string unknown(node.Width(), 'x');
    if (node.model == ModelKind::kLogic) {
      return std::string(1, EvaluateLogic(node, [&](auto bit) {
                           // EvaluateLogic applies each reference's inversion
                           // itself.
                           bit.inverted = false;
                           return Read(bit);
                         }));
    }
    const bool serial =
        key.second || node.kind == "ScanMux" || node.kind == "ScanInPort" ||
        node.kind == "ScanOutPort" || node.kind == "OneHotScanGroup";
    std::string value = unknown;
    if (node.model == ModelKind::kMux) {
      const auto selected = Bits(node.inputs.at("SelectedBy"));
      const auto compatible = [&](const std::string& code) {
        for (size_t bit = 0; bit < selected.size(); ++bit) {
          if (selected[bit] != 'x' && code.at(bit) != 'x' &&
              selected[bit] != code[bit]) {
            return false;
          }
        }
        return true;
      };
      if (selected.find('x') != std::string::npos) {
        for (const auto& hole : UncoveredMuxCodes(node)) {
          if (compatible(hole)) return unknown;
        }
      }
      bool found = false;
      for (const auto& choice : node.choices) {
        bool matches = false;
        for (const auto& code : choice.codes) {
          matches = matches || compatible(code.data);
        }
        if (matches) {
          const auto next = Bits(choice.source, serial);
          value = found ? Join(value, next) : next;
          found = true;
        }
      }
    } else if (node.model == ModelKind::kGroup) {
      size_t selected = 0;
      bool uncertain = false;
      for (size_t source = 0; source < node.one_hot_sources.size(); ++source) {
        char enable = '0';
        for (const auto& condition :
             one_hot_.Resolve(node.one_hot_sources[source])) {
          enable = Or(enable, Active(condition));
        }
        uncertain = uncertain || enable == 'x';
        if (enable != '1') continue;
        ++selected;
        node.OneHotSourceBit(source, 0);
        value = Bits(node.one_hot_sources[source].bits, serial);
      }
      if (selected != 1 || uncertain) {
        value = unknown;
      } else {
        value.insert(0, node.Width() - value.size(), 'x');
      }
    } else {
      const auto source = node.inputs.find("Source");
      if (source != node.inputs.end()) {
        value = Bits(source->second, serial);
      }
    }
    const auto enabled = node.inputs.find("Enable");
    if (enabled != node.inputs.end()) {
      value = Choose(Read(enabled->second.at(0)), value, unknown);
    }
    return value;
  }
  const NetworkState& state_;
  const Netlist& netlist_;
  OneHotReadResolver one_hot_;
  std::map<Key, std::string> cache_;
  std::set<Key> visiting_;
};

void ValidateStateTarget(const Netlist& netlist, size_t id) {
  const auto& node = netlist.nodes.at(id);
  if (node.model != ModelKind::kRegister &&
      !(node.model == ModelKind::kPort && node.scope == 0 &&
        (node.kind == "DataInPort" || node.kind == "SelectPort") &&
        !node.inputs.count("Source"))) {
    throw std::runtime_error(node.origin + ": state assumption for " +
                             node.name +
                             " requires a ScanRegister/DataRegister or "
                             "top-level DataInPort/SelectPort");
  }
}

void ValidateStateAssumption(const Netlist& netlist, size_t id) {
  ValidateStateTarget(netlist, id);
  for (const auto& [name, tap] : netlist.taps) {
    if (id == tap.instruction_register) {
      throw std::runtime_error(
          "cannot assume controller-owned instruction state");
    }
  }
}

NetworkState::NetworkState(const Netlist& netlist)
    : netlist_(netlist),
      update_(netlist.nodes.size()),
      shift_(netlist.nodes.size()),
      driven_(netlist.nodes.size()) {
  size_t remaining = 16 * 1024 * 1024;
  for (size_t id = 0; id < netlist.nodes.size(); ++id) {
    const auto& node = netlist.nodes[id];
    if (node.Width() > remaining) {
      throw std::runtime_error("network state bit limit exceeded");
    }
    remaining -= node.Width();
    has_data_registers_ =
        has_data_registers_ ||
        (node.kind == "DataRegister" && node.inputs.count("WriteDataSource"));
    if (node.model == ModelKind::kRegister) {
      update_[id].assign(node.Width(), 'x');
      if (node.kind == "ScanRegister") {
        shift_[id] = update_[id];
      }
    }
  }
}
bool NetworkState::SameState(const NetworkState& state) const {
  return &netlist_ == &state.netlist_ && update_ == state.update_ &&
         shift_ == state.shift_ && driven_ == state.driven_ &&
         updated_ == state.updated_ && global_reset_ == state.global_reset_ &&
         async_reset_ == state.async_reset_ &&
         reset_events_ == state.reset_events_ &&
         control_events_ == state.control_events_;
}
std::set<size_t> NetworkState::TakeResetEvents() {
  std::set<size_t> events;
  events.swap(reset_events_);
  return events;
}
NetworkState::ControlEvents NetworkState::TakeControlEvents() {
  ControlEvents events;
  events.swap(control_events_);
  return events;
}
void NetworkState::Restore(const NetworkState& state) {
  if (&netlist_ != &state.netlist_) {
    throw std::runtime_error("cannot restore state from a different netlist");
  }
  update_ = state.update_;
  shift_ = state.shift_;
  driven_ = state.driven_;
  updated_ = state.updated_;
  reset_events_ = state.reset_events_;
  control_events_ = state.control_events_;
  global_reset_ = state.global_reset_;
  async_reset_ = state.async_reset_;
}
void NetworkState::Drive(size_t id, const std::string& value) {
  const auto& node = netlist_.nodes.at(id);
  Validate(value, node.Width());
  if (node.model != ModelKind::kPort || node.inputs.count("Source") ||
      !IsInputPort(node.kind)) {
    throw std::runtime_error(node.origin +
                             ": cannot drive connected or non-input node " +
                             node.name);
  }
  driven_[id] = value;
}
void NetworkState::Assume(size_t id, const std::string& value) {
  ValidateStateAssumption(netlist_, id);
  if (netlist_.nodes[id].model == ModelKind::kRegister) {
    Validate(value, netlist_.nodes[id].Width());
    update_[id] = value;
  } else {
    Drive(id, value);
  }
}
std::string NetworkState::Value(size_t node, bool scan) const {
  return SignalEvaluator(*this).Value(node, scan);
}
char NetworkState::Read(const BitReference& bit, bool scan) const {
  return SignalEvaluator(*this).Read(bit, scan);
}
std::string NetworkState::ReadBits(const std::vector<BitReference>& bits,
                                   bool scan) const {
  return SignalEvaluator(*this).Bits(bits, scan);
}
bool NetworkState::IsDriven(size_t node) const {
  return !driven_.at(node).empty();
}
void NetworkState::SetGlobalReset(char synchronous, char asynchronous) {
  Validate({synchronous, asynchronous}, 2);
  global_reset_ = synchronous;
  async_reset_ = asynchronous;
}
void NetworkState::SetRegister(size_t node, const std::string& shift,
                               const std::string& update) {
  const auto& object = netlist_.nodes.at(node);
  if (object.kind != "ScanRegister") {
    throw std::runtime_error("controller state requires ScanRegister");
  }
  Validate(shift, object.Width());
  Validate(update, object.Width());
  shift_[node] = shift;
  update_[node] = update;
}
const DataRegisterPlan& NetworkState::DataRegisters() const {
  if (!data_registers_) {
    data_registers_ = std::make_shared<DataRegisterPlan>(netlist_);
  }
  return *data_registers_;
}
void NetworkState::ApplyInputs(const std::map<size_t, std::string>& values,
                               const Check& check) {
  if (has_data_registers_) DataRegisters();
  NetworkState next(*this);
  for (const auto& [id, value] : values) next.Drive(id, value);
  next.Reset();
  next.UpdateDataRegisters('1', check);
  Restore(next);
}
void NetworkState::UpdateDataRegisters(char event, const Check& check) {
  if (!has_data_registers_) return;
  const auto& plan = DataRegisters();
  const auto updated = [&](size_t id, SignalEvaluator& values) {
    const auto& node = netlist_.nodes[id];
    const auto& activation = plan.writes.at(id);
    const char enabled = And(event, values.Active(activation));
    const auto load = values.Bits(node.inputs.at("WriteDataSource"));
    const char reset = node.reset ? values.Read(*node.reset) : '0';
    auto next = Choose(enabled, load, update_[id]);
    if (enabled == '1' && reset == '0' && load.find('x') == std::string::npos) {
      control_events_[id] = load;
    }
    if (node.reset) {
      if (reset == '1') {
        reset_events_.insert(id);
        control_events_[id] = std::nullopt;
      }
      next = Choose(reset, node.constants.at("ResetValue").data, next);
    }
    return next;
  };
  size_t remaining = kDataRegisterFeedbackBits;
  for (const auto& group : plan.groups) {
    if (!group.feedback) {
      SignalEvaluator values(*this);
      const size_t id = group.nodes.front();
      update_[id] = updated(id, values);
      continue;
    }
    std::string reset_levels;
    SignalEvaluator before(*this);
    for (const auto& reset : group.external_resets) {
      reset_levels += before.Read(reset);
    }
    bool stable = false;
    for (size_t round = 0; round < kDataRegisterDeltaLimit; ++round) {
      for (size_t id : group.nodes) {
        const auto width = netlist_.nodes[id].Width();
        if (width > remaining) {
          throw std::runtime_error(
              "data-register feedback bit-work limit; fixed point not proved");
        }
        remaining -= width;
      }
      SignalEvaluator values(*this);
      std::vector<std::string> next;
      next.reserve(group.nodes.size());
      stable = true;
      for (size_t id : group.nodes) {
        next.push_back(updated(id, values));
        stable = stable && next.back() == update_[id];
      }
      for (size_t i = 0; i < group.nodes.size(); ++i) {
        update_[group.nodes[i]] = std::move(next[i]);
      }
      if (check && !stable) check(*this);
      ResetFeedback(group, &remaining, check);
      if (stable) break;
      SignalEvaluator after(*this);
      for (size_t i = 0; i < group.external_resets.size(); ++i) {
        if (after.Read(group.external_resets[i]) != reset_levels[i]) {
          throw std::runtime_error(
              "feedback reset event is not supported during data settling");
        }
      }
    }
    if (!stable) {
      throw std::runtime_error(
          "data-register feedback delta limit; fixed point not proved");
    }
  }
  Reset();
}
void NetworkState::Clock(const std::map<size_t, ScanControl>& controls,
                         bool rising, const Check& check) {
  char data_update = '0';
  if (has_data_registers_ && !rising) {
    for (const auto& [id, control] : controls) {
      data_update = Or(data_update, And(control.select, control.update));
    }
  }
  if (data_update == '0') {
    ClockRegisters(controls, rising);
    return;
  }
  DataRegisters();
  NetworkState next(*this);
  next.ClockRegisters(controls, rising);
  next.UpdateDataRegisters(data_update, check);
  Restore(next);
}
void NetworkState::ShiftComplete(
    const std::map<size_t, std::string>& loads,
    const std::map<size_t, ScanControl>& controls) {
  if (loads.empty() || loads.size() != controls.size()) {
    throw std::runtime_error(
        "complete shift requires matching loads and controls");
  }
  for (const auto& [id, bits] : loads) {
    const auto& node = netlist_.nodes.at(id);
    const auto control = controls.find(id);
    if (node.kind != "ScanRegister" || control == controls.end() ||
        control->second.capture != '0' || control->second.shift != '1' ||
        control->second.update != '0' || control->second.select != '1' ||
        (node.reset && Read(*node.reset) != '0')) {
      throw std::runtime_error(
          "complete shift requires active shifting and inactive resets");
    }
    Validate(bits, node.Width());
  }
  // A real native edge retires update shadows before replacing the fully
  // overwritten shift data. The caller drives the final serial input first.
  Clock(controls, true);
  for (const auto& [id, bits] : loads) SetRegister(id, bits, Value(id));
}
void NetworkState::ClockRegisters(const std::map<size_t, ScanControl>& controls,
                                  bool rising) {
  SignalEvaluator values(*this);
  std::map<size_t, std::string> next_shift, next_update;
  ControlEvents events;
  std::set<size_t> updated;
  for (const auto& [id, control] : controls) {
    const auto& node = netlist_.nodes.at(id);
    if (node.kind != "ScanRegister") {
      throw std::runtime_error("scan event targets non-scan register");
    }
    Validate({control.capture, control.shift, control.update, control.select},
             4);
    const char capture = And(control.select, control.capture),
               shift = And(control.select, control.shift);
    if (And(control.select, control.update) != '0') {
      updated.insert(id);
    }
    if (rising) {
      if (capture == '1' && shift == '1') {
        throw std::runtime_error(
            node.origin +
            ": simultaneous effective CaptureEn and ShiftEn is ambiguous");
      }
      auto next = shift_[id];
      if (capture != '0') {
        const auto found = node.inputs.find("CaptureSource");
        const auto captured = found == node.inputs.end()
                                  ? std::string(node.Width(), 'x')
                                  : values.Bits(found->second);
        next = Choose(capture, captured, next);
      }
      if (shift != '0') {
        const auto shifted =
            std::string(
                1, values.Read(node.inputs.at("ScanInSource").at(0), true)) +
            shift_[id].substr(0, node.Width() - 1);
        next = Choose(shift, shifted, next);
      }
      next_shift.emplace(id, std::move(next));
    } else {
      const char enabled = And(control.select, control.update);
      const char reset = node.reset ? values.Read(*node.reset) : '0';
      auto next = Choose(enabled, shift_[id], update_[id]);
      if (enabled == '1' && reset == '0' &&
          shift_[id].find('x') == std::string::npos) {
        events[id] = shift_[id];
      }
      if (node.reset) {
        if (reset == '1') {
          reset_events_.insert(id);
          events[id] = std::nullopt;
        }
        next = Choose(reset, node.constants.at("ResetValue").data, next);
      }
      next_update.emplace(id, std::move(next));
    }
  }
  for (size_t id : updated_) {
    if (updated.count(id)) {
      continue;
    }
    const auto found = controls.find(id);
    if (!rising || found == controls.end() ||
        (And(found->second.select, found->second.capture) == '0' &&
         And(found->second.select, found->second.shift) == '0')) {
      next_shift[id].assign(netlist_.nodes.at(id).Width(), 'x');
    }
  }
  updated_ = std::move(updated);
  for (auto& [id, event] : events) control_events_[id] = std::move(event);
  for (auto& [id, value] : next_shift) {
    shift_[id] = std::move(value);
  }
  for (auto& [id, value] : next_update) {
    update_[id] = std::move(value);
  }
  Reset();
}
void NetworkState::ResetFeedback(const DataRegisterGroup& group,
                                 size_t* remaining, const Check& check) {
  std::set<size_t> pending(group.reset_targets.begin(),
                           group.reset_targets.end());
  while (!pending.empty()) {
    SignalEvaluator values(*this);
    std::map<size_t, std::string> next;
    for (size_t id : pending) {
      const auto& node = netlist_.nodes[id];
      if (node.Width() > *remaining) {
        throw std::runtime_error("data-register feedback reset bit-work limit");
      }
      *remaining -= node.Width();
      const char reset = values.Read(*node.reset);
      if (reset == '1') {
        reset_events_.insert(id);
        control_events_[id] = std::nullopt;
      }
      auto value =
          Choose(reset, node.constants.at("ResetValue").data, update_[id]);
      if (value != update_[id]) next.emplace(id, std::move(value));
    }
    pending.clear();
    for (auto& [id, value] : next) {
      update_[id] = std::move(value);
      const auto consumers = group.reset_consumers.find(id);
      if (consumers != group.reset_consumers.end()) {
        pending.insert(consumers->second.begin(), consumers->second.end());
      }
    }
    if (check && !next.empty()) check(*this);
  }
}

void NetworkState::Reset() {
  // Reset controls may themselves be register outputs. Propagate to a fixed
  // point without inventing a reset or prioritizing a source by traversal
  // order.
  size_t resets = 0;
  for (const auto& node : netlist_.nodes) resets += node.reset.has_value();
  // Each vector changes at most twice; retain the final event-observation pass.
  for (size_t pass = 0; pass <= 2 * resets; ++pass) {
    SignalEvaluator values(*this);
    std::map<size_t, std::string> next;
    for (size_t id = 0; id < netlist_.nodes.size(); ++id) {
      const auto& node = netlist_.nodes[id];
      if (!node.reset) {
        continue;
      }
      const char reset = values.Read(*node.reset);
      if (reset == '1') {
        reset_events_.insert(id);
        control_events_[id] = std::nullopt;
      }
      const auto value =
          Choose(reset, node.constants.at("ResetValue").data, update_[id]);
      if (value != update_[id]) {
        next.emplace(id, value);
      }
    }
    if (next.empty()) {
      return;
    }
    for (auto& [id, value] : next) {
      update_[id] = std::move(value);
    }
  }
  throw std::runtime_error("reset network did not converge");
}
}  // namespace pattern
