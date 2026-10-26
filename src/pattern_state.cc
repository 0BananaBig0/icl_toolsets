// Copyright 2026 The ICL Toolsets Authors.
// SPDX-License-Identifier: MIT
#include "pattern_state.h"

#include <algorithm>
#include <set>
#include <stdexcept>
#include <utility>

#include "pattern_logic.h"

namespace pattern {
namespace {
constexpr size_t kMissing = std::numeric_limits<size_t>::max();
char Invert(char value) {
  return value == '0' ? '1' : value == '1' ? '0' : 'x';
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
      : state_(state), netlist_(state.netlist_) {}
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
      const bool serial = key.second || node.kind == "ScanMux" ||
                          node.kind == "ScanInPort" ||
                          node.kind == "ScanOutPort";
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
    const bool serial = key.second || node.kind == "ScanMux" ||
                        node.kind == "ScanInPort" || node.kind == "ScanOutPort";
    std::string value = unknown;
    if (node.model == ModelKind::kMux) {
      const auto selected = Bits(node.inputs.at("SelectedBy"));
      // An uncertain selector can include an unmapped encoding.
      if (selected.find('x') != std::string::npos) {
        return unknown;
      }
      bool found = false;
      for (const auto& choice : node.choices) {
        bool matches = false;
        for (const auto& code : choice.codes) {
          bool match = true;
          for (size_t i = 0; i < selected.size(); ++i) {
            if (code.data.at(i) != 'x' && code.data[i] != selected[i]) {
              match = false;
            }
          }
          matches = matches || match;
        }
        if (matches) {
          const auto next = Bits(choice.source, serial);
          value = found ? Join(value, next) : next;
          found = true;
        }
      }
    } else if (node.model == ModelKind::kGroup) {
      throw std::runtime_error(node.origin +
                               ": one-hot state execution is not implemented");
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
  std::map<Key, std::string> cache_;
  std::set<Key> visiting_;
};

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
    if (node.model == ModelKind::kRegister) {
      update_[id].assign(node.Width(), 'x');
      if (node.kind == "ScanRegister") {
        shift_[id] = update_[id];
      }
    }
  }
}
void NetworkState::Restore(const NetworkState& state) {
  if (&netlist_ != &state.netlist_) {
    throw std::runtime_error("cannot restore state from a different netlist");
  }
  update_ = state.update_;
  shift_ = state.shift_;
  driven_ = state.driven_;
  updated_ = state.updated_;
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
std::string NetworkState::Value(size_t node, bool scan) const {
  return SignalEvaluator(*this).Value(node, scan);
}
char NetworkState::Read(const BitReference& bit, bool scan) const {
  return SignalEvaluator(*this).Read(bit, scan);
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
void NetworkState::Clock(const std::map<size_t, ScanControl>& controls,
                         bool rising) {
  SignalEvaluator values(*this);
  std::map<size_t, std::string> next_shift, next_update;
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
      auto next =
          Choose(And(control.select, control.update), shift_[id], update_[id]);
      if (node.reset) {
        next = Choose(values.Read(*node.reset),
                      node.constants.at("ResetValue").data, next);
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
  for (auto& [id, value] : next_shift) {
    shift_[id] = std::move(value);
  }
  for (auto& [id, value] : next_update) {
    update_[id] = std::move(value);
  }
  Reset();
}
void NetworkState::Reset() {
  // Reset controls may themselves be register outputs. Propagate to a fixed
  // point without inventing a reset or prioritizing a source by traversal
  // order.
  for (size_t pass = 0; pass <= netlist_.nodes.size(); ++pass) {
    SignalEvaluator values(*this);
    std::map<size_t, std::string> next;
    for (size_t id = 0; id < netlist_.nodes.size(); ++id) {
      const auto& node = netlist_.nodes[id];
      if (!node.reset) {
        continue;
      }
      const auto value =
          Choose(values.Read(*node.reset), node.constants.at("ResetValue").data,
                 update_[id]);
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
