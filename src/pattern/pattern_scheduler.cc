// Copyright 2026 The ICL Toolsets Authors.
// SPDX-License-Identifier: MIT
#include "pattern/pattern_scheduler.h"

#include <algorithm>
#include <chrono>
#include <functional>
#include <set>
#include <stdexcept>
#include <tuple>
#include <utility>

#include "pattern/pattern_data.h"
#include "pattern/pattern_state.h"
#include "pattern/pattern_symbolic.h"
#include "pattern/pattern_tap_execution.h"
#include "pattern/pattern_values.h"

namespace pattern {
namespace {
constexpr size_t kMissing = std::numeric_limits<size_t>::max();
constexpr size_t kMaxSteps = 32;
using Values = std::map<size_t, SymbolicValue>;
using Key = std::pair<size_t, int64_t>;

void MinimizeBits(z3::optimize* solver, const z3::expr& bits) {
  // Bound soft-constraint weights while preserving unsigned objective order.
  for (unsigned high = bits.get_sort().bv_size(); high;) {
    const unsigned low = high > 32 ? high - 32 : 0;
    solver->minimize(bits.extract(high - 1, low));
    high = low;
  }
}

class ScheduleLimit : public std::runtime_error {
 public:
  using std::runtime_error::runtime_error;
};
size_t Offset(const NetlistNode& node, int64_t index) {
  return node.left >= node.right ? node.left - index : index - node.left;
}
z3::expr Known(const SymbolicValue& value, bool one) {
  return value.known == value.bits.ctx().bv_val(1, 1) &&
         value.bits == value.bits.ctx().bv_val(one ? 1 : 0, 1);
}
// Unknown bits in a reservation are unprotected, not required unknown state.
z3::expr Matches(const SymbolicValue& value, const SymbolicValue& mask) {
  return (value.known & mask.known) == mask.known &&
         (value.bits & mask.known) == mask.bits;
}
std::string Masked(const std::string& value, const std::string& mask) {
  auto result = mask;
  for (size_t i = 0; i < mask.size(); ++i) {
    if (mask[i] != 'x') result[i] = value.at(i);
  }
  return result;
}
SymbolicValue Choose(const z3::expr& choice, const SymbolicValue& a,
                     const SymbolicValue& b) {
  return {z3::ite(choice, a.bits, b.bits), z3::ite(choice, a.known, b.known)};
}
SymbolicValue BooleanValue(const z3::expr& value) {
  auto& context = value.ctx();
  return {z3::ite(value, context.bv_val(1, 1), context.bv_val(0, 1)),
          context.bv_val(1, 1)};
}
SymbolicValue And(const SymbolicValue& a, const SymbolicValue& b) {
  const auto known =
      (a.known & b.known) | (a.known & ~a.bits) | (b.known & ~b.bits);
  return {a.bits & b.bits & known, known};
}
SymbolicValue Or(const SymbolicValue& a, const SymbolicValue& b) {
  const auto known =
      (a.known & b.known) | (a.known & a.bits) | (b.known & b.bits);
  return {(a.bits | b.bits) & known, known};
}
SymbolicValue ResetValue(const SymbolicValue& control,
                         const SymbolicValue& reset,
                         const SymbolicValue& previous) {
  const auto known =
      reset.known & previous.known & ~(reset.bits ^ previous.bits);
  auto& context = control.bits.ctx();
  return Choose(control.known == context.bv_val(1, 1),
                Choose(control.bits == context.bv_val(1, 1), reset, previous),
                {previous.bits & known, known});
}
std::string Decode(const z3::model& model, const SymbolicValue& value) {
  std::string bits;
  for (unsigned i = value.bits.get_sort().bv_size(); i > 0; --i) {
    bits +=
        model.eval(value.known.extract(i - 1, i - 1), true)
                    .get_numeral_uint() == 0
            ? 'x'
        : model.eval(value.bits.extract(i - 1, i - 1), true).get_numeral_uint()
            ? '1'
            : '0';
  }
  return bits;
}
}  // namespace

class ScanScheduler::Impl {
 public:
  explicit Impl(const Netlist& netlist)
      : netlist_(netlist), state_(netlist), one_hot_(netlist) {
    if (!netlist.taps.empty()) {
      tap_ = &netlist.taps.begin()->second;
      for (const auto& [name, tap] : netlist.taps) {
        instruction_registers_.emplace(tap.instruction_register, &tap);
        for (const auto& [role, id] : tap.pins) tap_pins_.insert(id);
        for (const auto& [reg, id] : tap.data_registers) {
          tap_data_registers_.insert(id);
        }
      }
      execution_ = std::make_unique<TapExecution>(netlist);
      state_.Restore(execution_->State());
    }
    const auto interfaces = EffectiveScanInterfaces(netlist);
    std::vector<const ScanInterface*> clients;
    for (const auto& binding : interfaces) {
      if (binding.scope == 0 &&
          binding.interface.kind == ScanInterfaceKind::kClient) {
        clients.push_back(&binding.interface);
      }
    }
    if (clients.size() > 1) {
      for (const auto* client : clients) {
        const size_t id = clients_.size();
        clients_.emplace_back();
        client_route_counts_.push_back(0);
        for (const auto& port : client->ports) {
          const Key key{port.node, port.index};
          const auto& kind = netlist.nodes.at(port.node).kind;
          if (kind == "SelectPort") {
            clients_.back().push_back(key);
            client_selectors_[key];
          }
        }
        const auto add_route = [&](const std::vector<BitReference>& ports) {
          Key input, output;
          for (const auto& port : ports) {
            const auto& kind = netlist.nodes.at(port.node).kind;
            if (kind == "ScanInPort") input = {port.node, port.index};
            if (kind == "ScanOutPort") output = {port.node, port.index};
          }
          client_routes_[output].emplace_back(id, input);
          client_inputs_[input].push_back(id);
          ++client_route_counts_.back();
        };
        if (client->chains.empty()) {
          add_route(client->ports);
        } else {
          for (const auto& chain : client->chains) add_route(chain.ports);
        }
        if (clients_.back().empty()) {
          for (const auto& port : client->ports) {
            if (netlist.nodes.at(port.node).kind == "ShiftEnPort") {
              const Key key{port.node, port.index};
              clients_.back().push_back(key);
              client_selectors_[key];
            }
          }
        }
      }
      // A fallback ShiftEn may also belong to a client with explicit selects.
      for (size_t id = 0; id < clients.size(); ++id) {
        for (const auto& port : clients[id]->ports) {
          const auto selector = client_selectors_.find({port.node, port.index});
          if (selector != client_selectors_.end()) {
            selector->second.push_back(id);
          }
        }
      }
    }
    std::map<Key, Key> chain_inputs;
    for (const auto& binding : interfaces) {
      if (!clients_.empty() || binding.scope != 0 ||
          binding.interface.kind != ScanInterfaceKind::kClient ||
          binding.interface.chains.size() < 2) {
        continue;
      }
      parallel_chains_ = true;
      for (const auto& chain : binding.interface.chains) {
        Key input, output;
        for (const auto& port : chain.ports) {
          if (netlist.nodes[port.node].kind == "ScanInPort") {
            input = {port.node, port.index};
          } else {
            output = {port.node, port.index};
          }
        }
        chain_inputs.emplace(output, input);
      }
    }
    if (netlist.taps.size() > 1) {
      parallel_chains_ = true;
      for (const auto& [name, tap] : netlist.taps) {
        const auto output = tap.pins.at("tap_scan_out");
        const auto input = tap.pins.at("tap_scan_in");
        chain_inputs.emplace(Key{output, netlist.nodes[output].left},
                             Key{input, netlist.nodes[input].left});
      }
    }
    for (size_t id = 0; id < netlist.nodes.size(); ++id) {
      const auto& node = netlist.nodes[id];
      if (tap_ && node.scope == 0 && node.kind == "TCKPort" &&
          !node.inputs.count("Source")) {
        tap_clocks_.insert(id);
      }
      if (node.model == ModelKind::kRegister || node.kind == "DataInPort" ||
          node.kind == "DataOutPort" || ParallelControl(node) ||
          PrimaryInput(node)) {
        if (node.kind == "ScanRegister") registers_.push_back(id);
        auto initial = std::string(node.Width(), '0');
        for (const auto* property : {"ResetValue", "DefaultLoadValue"}) {
          const auto found = node.constants.find(property);
          if (found != node.constants.end()) {
            for (size_t i = 0; i < initial.size(); ++i) {
              if (found->second.data.at(i) != 'x') {
                initial[i] = found->second.data[i];
              }
            }
          }
        }
        fill_[id] = std::move(initial);
      }
      if (node.kind == "ScanOutPort" && node.scope == 0) {
        for (int64_t index = node.left;;
             index += node.left <= node.right ? 1 : -1) {
          if ((!parallel_chains_ || chain_inputs.count({id, index})) &&
              (clients_.empty() || client_routes_.count({id, index}))) {
            roots_.push_back(Add({id, index}));
          }
          if (index == node.right) break;
        }
      }
    }
    if (registers_.size() > 4096 || graph_.size() > 65536) {
      throw std::runtime_error(
          "scan scheduler structural resource limit exceeded");
    }
    std::sort(roots_.begin(), roots_.end(), [&](size_t a, size_t b) {
      const auto& left = graph_[a].bit;
      const auto& right = graph_[b].bit;
      return std::make_pair(netlist.nodes[left.node].name, left.index) <
             std::make_pair(netlist.nodes[right.node].name, right.index);
    });
    if (parallel_chains_) {
      CheckIndependentChains(chain_inputs);
    } else if (clients_.empty() && roots_.size() > 1) {
      // Forwarded observations of one path do not create extra clock events.
      std::vector<size_t> paths(graph_.size()), physical(graph_.size());
      for (size_t id : order_) {
        const auto& vertex = graph_[id];
        paths[id] = physical[id] = id;
        if (vertex.bit.node != kMissing && vertex.edges.size() == 1 &&
            vertex.edges.front().choice == kMissing) {
          const auto& node = netlist.nodes[vertex.bit.node];
          if (node.kind != "ScanRegister") {
            physical[id] = physical[vertex.edges.front().next];
            if (!node.inputs.count("Enable")) {
              paths[id] = paths[vertex.edges.front().next];
            }
          }
        }
      }
      std::set<size_t> observed;
      roots_.erase(std::remove_if(roots_.begin(), roots_.end(),
                                  [&](size_t root) {
                                    return !observed.insert(paths[root]).second;
                                  }),
                   roots_.end());
      // Gated observations remain alternatives, with their enables enforced.
      for (size_t root : roots_) {
        if (physical[root] != physical[roots_.front()]) {
          throw std::runtime_error(
              "multiple scan outputs require shared-control transaction "
              "scheduling");
        }
      }
    }
    BuildEndStates();
    initial_fill_ = fill_;
    BuildResetOrder();
    for (const auto& node : netlist.nodes) {
      if (node.reset) AddControl(*node.reset);
      for (const auto* property : {"SelectedBy", "Enable"}) {
        const auto found = node.inputs.find(property);
        if (found != node.inputs.end()) {
          for (const auto& ref : found->second) AddControl(ref);
        }
      }
    }
    if (tap_) {
      for (const auto& [id, tap] : instruction_registers_) {
        control_masks_[id] = std::string(netlist.nodes[id].Width(), '1');
      }
      for (const auto& [reg, interfaces] : execution_->RegisterInterfaces()) {
        for (const auto& ports : interfaces) {
          for (const auto& [kind, bits] : ports) {
            if (kind == "ScanInPort" || kind == "ScanOutPort") continue;
            for (const auto& [id, index] : bits) AddControl({id, index});
          }
        }
      }
      return;
    }
    std::set<size_t> serial_scopes;
    for (const auto& vertex : graph_) {
      if (vertex.bit.node != kMissing) {
        serial_scopes.insert(netlist.nodes.at(vertex.bit.node).scope);
      }
    }
    std::set<size_t> bound_controls;
    const std::map<std::string, std::string> roles = {
        {"SelectPort", "select"},
        {"CaptureEnPort", "capture"},
        {"ShiftEnPort", "shift"},
        {"UpdateEnPort", "update"},
        {"TCKPort", "clock"}};
    for (const auto& binding : interfaces) {
      if (binding.interface.kind != ScanInterfaceKind::kClient) {
        throw std::runtime_error(
            binding.origin +
            ": interface controls require event scheduling integration");
      }
      for (const auto& port : binding.interface.ports) {
        const auto& node = netlist.nodes.at(port.node);
        const auto role = roles.find(node.kind);
        if (role == roles.end()) continue;
        bound_controls.insert(port.node);
        if (node.kind == "SelectPort") {
          idle_selects_.insert({port.node, port.index});
        }
        AddControl(port);
        client_controls_ = true;
        if (node.kind == "TCKPort") {
          client_clocks_.insert({port.node, port.index});
        }
        if (!node.inputs.count("Source") &&
            (node.scope == 0 || node.kind == "TCKPort")) {
          client_pins_.emplace(port.node, role->second);
          client_pin_bits_[port.node].insert(Offset(node, port.index));
        }
      }
    }
    // Vector ports without a client interface must not hide declared controls.
    for (size_t id = 0; id < netlist.nodes.size(); ++id) {
      const auto& node = netlist.nodes[id];
      if (serial_scopes.count(node.scope) &&
          (roles.count(node.kind) || node.kind == "TMSPort" ||
           node.kind == "TRSTPort") &&
          !bound_controls.count(id)) {
        throw std::runtime_error(
            node.origin +
            ": interface controls require event scheduling integration");
      }
    }
    if (client_controls_) {
      register_ports_ = TapExecution(netlist).RegisterInterfaces();
    }
  }

  void CheckIndependentChains(const std::map<Key, Key>& inputs) const {
    std::vector<size_t> owner(graph_.size(), kMissing);
    for (size_t root : roots_) owner[root] = root;
    for (auto it = order_.rbegin(); it != order_.rend(); ++it) {
      const size_t id = *it;
      if (owner[id] == kMissing) continue;
      const auto& vertex = graph_[id];
      if (vertex.input) {
        const auto& output = graph_[owner[id]].bit;
        if (inputs.at({output.node, output.index}) !=
            Key{vertex.bit.node, vertex.bit.index}) {
          throw std::runtime_error(
              "scan chain does not reach its declared input");
        }
      }
      for (const auto& edge : vertex.edges) {
        if (owner[edge.next] != kMissing && owner[edge.next] != owner[id]) {
          throw std::runtime_error(
              "shared scan inputs or storage require overlapping-event "
              "scheduling");
        }
        owner[edge.next] = owner[id];
      }
    }
  }

  Impl(const Impl& other) : Impl(other.netlist_) {
    state_.Restore(other.state_);
    fill_ = other.fill_;
    global_resets_ = other.global_resets_;
    tap_ready_ = other.tap_ready_;
    control_masks_ = other.control_masks_;
    data_control_masks_ = other.data_control_masks_;
    control_seen_ = other.control_seen_;
    if (other.execution_) {
      execution_ = std::make_unique<TapExecution>(*other.execution_);
    }
  }
  BitReference EndStateSource(BitReference bit) const {
    std::set<Key> visited;
    while (bit.node != kMissing) {
      const auto& node = netlist_.nodes.at(bit.node);
      if (!visited.emplace(bit.node, bit.index).second) {
        throw std::runtime_error("cyclic iApplyEndState source");
      }
      if (node.kind == "ScanRegister" || node.kind == "DataRegister" ||
          PrimaryInput(node)) {
        return bit;
      }
      if (node.model != ModelKind::kAlias && node.kind != "DataInPort" &&
          node.kind != "DataOutPort") {
        throw std::runtime_error("unsuitable iApplyEndState source " +
                                 node.name);
      }
      if (node.inputs.count("Enable") ||
          node.properties.count("EnableSource")) {
        throw std::runtime_error("gated iApplyEndState source " + node.name);
      }
      const auto source = node.inputs.find("Source");
      if (source == node.inputs.end()) {
        throw std::runtime_error("unconnected iApplyEndState source " +
                                 node.name);
      }
      auto next = source->second.at(Offset(node, bit.index));
      next.inverted = next.inverted != bit.inverted;
      bit = next;
    }
    return bit;
  }
  void BuildEndStates() {
    for (const auto& node : netlist_.nodes) {
      const auto directive = node.constants.find("iApplyEndState");
      if (directive == node.constants.end()) continue;
      try {
        const auto target = ResolvePdlTarget(netlist_, node.name);
        const auto& bits = directive->second.data;
        if (bits.size() != target.bits.size() ||
            bits.find_first_not_of("01") != std::string::npos) {
          throw std::runtime_error("requires matching width and known bits");
        }
        for (size_t i = 0; i < bits.size(); ++i) {
          const auto ref = EndStateSource(target.bits[i]);
          const char value =
              ref.inverted ? (bits[i] == '1' ? '0' : '1') : bits[i];
          if (ref.node == kMissing) {
            if (ref.virtual_signal != VirtualSignal::kNone ||
                ref.constant != value) {
              throw std::runtime_error("incompatible constant source");
            }
            continue;
          }
          const auto& source = netlist_.nodes.at(ref.node);
          const size_t offset = Offset(source, ref.index);
          if (source.model == ModelKind::kRegister) {
            const auto reset = source.constants.find("ResetValue");
            if (reset == source.constants.end() ||
                reset->second.data.at(offset) != value) {
              throw std::runtime_error(
                  "requires a matching register ResetValue");
            }
          }
          if (!fill_.count(ref.node)) {
            throw std::runtime_error(
                "DataRegister execution is not implemented");
          }
          auto& values = end_states_.try_emplace(ref.node, source.Width(), 'x')
                             .first->second;
          if (values[offset] != 'x' && values[offset] != value) {
            throw std::runtime_error("conflicting directives");
          }
          values[offset] = value;
        }
      } catch (const std::exception& error) {
        throw std::runtime_error(node.origin + ": iApplyEndState on " +
                                 node.name + ": " + error.what());
      }
    }
  }
  std::map<size_t, std::string> EndGoals(
      const std::vector<Operation>& operations) {
    auto result = end_states_;
    if (result.empty()) return result;
    for (const auto& operation : operations) {
      if (operation.command != "iWrite" || operation.arguments.size() != 2) {
        continue;
      }
      const auto target = ResolvePdlTarget(netlist_, operation.arguments[0]);
      const auto bits =
          PdlDataValue(context_, netlist_, target, operation.arguments[1]);
      for (size_t i = 0; i < bits.size(); ++i) {
        if (bits[i] == 'x') continue;
        // Only direct data paths can override an end-state obligation.
        BitReference ref;
        try {
          ref = EndStateSource(target.bits[i]);
        } catch (const std::runtime_error&) {
          continue;
        }
        const auto found = result.find(ref.node);
        if (found != result.end()) {
          found->second.at(Offset(netlist_.nodes[ref.node], ref.index)) = 'x';
        }
      }
    }
    return result;
  }
  std::string DefaultFill(size_t id) const {
    auto result = fill_.at(id);
    if (end_states_.empty()) return result;
    const auto& node = netlist_.nodes[id];
    for (size_t i = 0; i < result.size(); ++i) {
      const auto index =
          node.left + (node.left <= node.right ? static_cast<int64_t>(i)
                                               : -static_cast<int64_t>(i));
      BitReference ref;
      try {
        ref = EndStateSource({id, index});
      } catch (const std::runtime_error&) {
        continue;
      }
      const auto found = end_states_.find(ref.node);
      if (found == end_states_.end()) continue;
      const char value =
          found->second.at(Offset(netlist_.nodes[ref.node], ref.index));
      if (value != 'x') {
        result[i] = ref.inverted ? (value == '1' ? '0' : '1') : value;
      }
    }
    return result;
  }
  bool NeedsEndState(const std::map<size_t, std::string>& values) const {
    for (const auto& [id, bits] : values) {
      const auto actual = state_.Value(id);
      for (size_t i = 0; i < bits.size(); ++i) {
        if (bits[i] != 'x' && bits[i] != actual[i]) return true;
      }
    }
    return false;
  }
  void Cycle(TapExecution* run, char tms,
             const std::map<size_t, std::string>& serial = {},
             const std::function<void(const NetworkState&)>& check = {}) const {
    auto pins = serial;
    for (const auto& [name, tap] : netlist_.taps) {
      pins.emplace(tap.pins.at("tap_scan_mode"), std::string(1, tms));
      pins.emplace(tap.pins.at("tap_scan_in"), "0");
    }
    run->Drive(pins);
    run->RisingEdge(tap_clocks_);
    if (check) check(run->State());
    run->FallingEdge(tap_clocks_);
    if (check) check(run->State());
  }
  void Reset(bool synchronous) {
    if (tap_) {
      auto run = std::make_unique<TapExecution>(*execution_);
      std::map<size_t, std::string> asserted, released, trst;
      for (size_t id = 0; id < netlist_.nodes.size(); ++id) {
        const auto& node = netlist_.nodes[id];
        if (node.scope == 0 && node.kind == "ResetPort" &&
            !node.inputs.count("Source")) {
          asserted.emplace(
              id, std::string(node.Width(), node.active_low ? '0' : '1'));
          released.emplace(
              id, std::string(node.Width(), node.active_low ? '1' : '0'));
        }
      }
      bool clock_reset = synchronous;
      for (const auto& [name, tap] : netlist_.taps) {
        const auto reset = tap.pins.find("tap_scan_reset");
        if (reset == tap.pins.end()) {
          clock_reset = true;
        } else {
          if (!synchronous) asserted.emplace(reset->second, "0");
          trst.emplace(reset->second, "1");
        }
      }
      if (!asserted.empty()) run->Drive(asserted);
      if (!trst.empty()) run->Drive(trst);
      if (clock_reset) {
        for (int i = 0; i < 5; ++i) Cycle(run.get(), '1');
      }
      auto fill = initial_fill_;
      // IR reset is driven by the controller, not an ICL ResetValue property.
      for (const auto& [id, tap] : instruction_registers_) {
        fill[id] = run->State().Value(id);
      }
      for (size_t id : registers_) {
        const auto& node = netlist_.nodes[id];
        if (node.reset && run->State().Read(*node.reset) == '1') {
          const auto& bits = node.constants.at("ResetValue").data;
          for (size_t i = 0; i < bits.size(); ++i) {
            if (bits[i] != 'x') fill[id][i] = bits[i];
          }
        }
      }
      if (!released.empty()) run->Drive(released);
      Cycle(run.get(), '0');
      state_.Restore(run->State());
      execution_ = std::move(run);
      fill_ = std::move(fill);
      global_resets_ = "00";
      tap_ready_ = true;
      return;
    }
    std::vector<size_t> pins;
    for (size_t id = 0; id < netlist_.nodes.size(); ++id) {
      const auto& node = netlist_.nodes[id];
      if (node.scope == 0 && node.kind == "ResetPort" &&
          !node.inputs.count("Source")) {
        state_.Drive(id,
                     std::string(node.Width(), node.active_low ? '0' : '1'));
        pins.push_back(id);
      }
    }
    state_.SetGlobalReset('1', '1');
    state_.Reset();
    fill_ = initial_fill_;
    for (size_t id : registers_) {
      const auto& node = netlist_.nodes[id];
      if (node.reset && state_.Read(*node.reset) == '1') {
        const auto& value = node.constants.at("ResetValue").data;
        for (size_t i = 0; i < value.size(); ++i) {
          if (value[i] != 'x') fill_[id][i] = value[i];
        }
      }
    }
    for (size_t id : pins) {
      state_.Drive(id, std::string(netlist_.nodes[id].Width(),
                                   netlist_.nodes[id].active_low ? '1' : '0'));
    }
    state_.SetGlobalReset('0', '0');
    global_resets_ = "00";
    state_.Reset();
  }
  void ClientIdleCycle(NetworkState* state,
                       const std::function<void(const NetworkState&)>& check) {
    const std::vector<TapExecution::InterfacePorts> empty;
    for (bool rising : {true, false}) {
      std::map<Key, char> clocks;
      for (const auto& pin : client_clocks_) {
        clocks.emplace(pin, state->Read({pin.first, pin.second}));
      }
      for (const auto& [id, role] : client_pins_) {
        if (role != "clock") continue;
        auto bits = state->Value(id);
        for (size_t offset : client_pin_bits_.at(id)) {
          bits[offset] = rising ? '1' : '0';
        }
        state->Drive(id, bits);
      }
      for (auto& [pin, before] : clocks) {
        const char after = state->Read({pin.first, pin.second});
        const char from = rising ? '0' : '1', to = rising ? '1' : '0';
        if (before == to && after == from) {
          throw std::runtime_error(
              "inverted client TCK phase is not supported");
        }
        before = before == to || after == from   ? '0'
                 : before == from && after == to ? '1'
                                                 : 'x';
      }
      check(*state);
      std::map<size_t, ScanControl> controls;
      std::optional<std::set<size_t>> active;
      for (size_t id : registers_) {
        const auto found = register_ports_.find(id);
        const auto& ports =
            found == register_ports_.end() ? empty : found->second;
        const auto off = EffectiveScanControl(netlist_, *state, ports,
                                              {'0', '0', '0', '0'}, clocks);
        const auto on = EffectiveScanControl(netlist_, *state, ports,
                                             {'0', '0', '0', '1'}, clocks);
        if (off.capture == on.capture && off.shift == on.shift &&
            off.update == on.update) {
          controls.emplace(id, off);
          continue;
        }
        if (!active) {
          active = ActivePath(*state);
          bool selected = active.has_value();
          for (const auto& [port, index] : idle_selects_) {
            const auto& node = netlist_.nodes.at(port);
            if (node.inputs.count("Source") || state->IsDriven(port)) {
              selected = selected && state->Read({port, index}) == '1';
            }
          }
          if (!selected) {
            throw std::runtime_error(
                "standalone idle implicit selection requires one proven route "
                "and known-active explicit interface selects");
          }
        }
        controls.emplace(id, active->count(id) ? on : off);
      }
      state->Clock(controls, rising);
      check(*state);
    }
  }

  void Idle(uint64_t cycles, const std::map<size_t, std::string>& preserved) {
    if (cycles == 0) return;
    if (tap_ && !tap_ready_) {
      throw std::runtime_error("TCK idle replay requires explicit iReset");
    }
    auto run =
        execution_ ? std::make_unique<TapExecution>(*execution_) : nullptr;
    auto standalone = run ? nullptr : std::make_unique<NetworkState>(state_);
    if (run) {
      run->TakeResetEvents();
      run->TakeControlEvents();
    } else {
      standalone->TakeResetEvents();
      standalone->TakeControlEvents();
    }
    const auto snapshot = [&]() -> const NetworkState& {
      return run ? run->State() : *standalone;
    };
    auto fill = fill_;
    auto held = preserved;
    const auto check = [&](const NetworkState& state) {
      CheckPreserved(state, held, preserved);
    };
    if (standalone) {
      for (const auto& [id, role] : client_pins_) {
        if (role == "select") continue;
        auto bits = standalone->Value(id);
        for (size_t offset : client_pin_bits_.at(id)) bits[offset] = '0';
        standalone->Drive(id, bits);
      }
      standalone->Reset();
      check(*standalone);
    }
    size_t work = netlist_.nodes.size();
    for (const auto& node : netlist_.nodes) work += node.Width();
    const size_t limit = std::min<size_t>(
        4096,
        std::max<size_t>(2, 16 * 1024 * 1024 / std::max<size_t>(1, work)));
    const auto start = std::chrono::steady_clock::now();
    for (size_t step = 0; cycles != 0; ++step, --cycles) {
      if (step == limit ||
          std::chrono::steady_clock::now() - start > std::chrono::seconds(15)) {
        throw std::runtime_error(
            "TCK idle replay resource limit before a stable state");
      }
      const NetworkState previous(snapshot());
      if (run) {
        Cycle(run.get(), '0', {}, check);
      } else {
        ClientIdleCycle(standalone.get(), check);
      }
      const auto resets =
          run ? run->TakeResetEvents() : standalone->TakeResetEvents();
      for (size_t id : resets) {
        const auto& node = netlist_.nodes[id];
        if (node.kind == "ScanRegister") {
          fill[id] = initial_fill_.at(id);
          const auto& bits = node.constants.at("ResetValue").data;
          for (size_t i = 0; i < bits.size(); ++i) {
            if (bits[i] != 'x') fill[id][i] = bits[i];
          }
        }
      }
      const auto events =
          run ? run->TakeControlEvents() : standalone->TakeControlEvents();
      for (const auto& [id, event] : events) {
        if (netlist_.nodes[id].kind == "DataRegister") {
          fill[id] = event ? *event : initial_fill_.at(id);
        }
      }
      // Every cycle starts/ends at the same idle protocol levels.
      // An identical complete state is a fixed point for all remaining cycles.
      if (snapshot().SameState(previous)) break;
    }
    state_.Restore(snapshot());
    execution_ = std::move(run);
    fill_ = std::move(fill);
  }
  void Assume(const std::string& target, const std::string& bits) {
    const auto resolved = ResolvePdlTarget(netlist_, target);
    if (bits.size() != resolved.bits.size() ||
        bits.find_first_not_of("01x") != std::string::npos) {
      throw std::runtime_error(
          "state assumption requires width-matched 0/1/x bits");
    }
    std::map<size_t, std::string> changed;
    for (size_t i = 0; i < bits.size(); ++i) {
      const auto& ref = resolved.bits[i];
      const auto& node = netlist_.nodes.at(ref.node);
      ValidateStateAssumption(netlist_, ref.node);
      auto entry = changed.find(ref.node);
      if (entry == changed.end()) {
        entry = changed.emplace(ref.node, state_.Value(ref.node)).first;
      }
      char bit = bits[i];
      if (ref.inverted && bit != 'x') bit = bit == '1' ? '0' : '1';
      entry->second.at(Offset(node, ref.index)) = bit;
    }
    for (const auto& [id, value] : changed) {
      if (execution_) execution_->Assume(id, value);
      state_.Assume(id, value);
      if (fill_.count(id)) {
        for (size_t i = 0; i < value.size(); ++i) {
          if (value[i] != 'x') fill_.at(id)[i] = value[i];
        }
      }
    }
  }
  std::string Value(const std::string& target) const {
    std::string value;
    for (const auto& bit : ResolvePdlTarget(netlist_, target).bits) {
      value += state_.Read(bit);
    }
    return value;
  }
  std::vector<ScanStep> Apply(const std::vector<Operation>& operations,
                              bool together,
                              const std::map<size_t, std::string>& preserved,
                              bool explicit_writes) {
    const auto previous_fill = fill_;
    try {
      for (const auto& [id, bits] : end_states_) {
        for (size_t i = 0; i < bits.size(); ++i) {
          if (bits[i] != 'x') fill_.at(id)[i] = bits[i];
        }
      }
      const bool events = (tap_ || client_controls_) && NeedsScanEvents();
      return Solve(operations, together, kMaxSteps, {}, true, preserved,
                   explicit_writes, events);
    } catch (const std::exception& error) {
      fill_ = previous_fill;
      std::string message = together ? "iApply -together: " : "";
      message += error.what();
      for (const auto& [id, value] : preserved) {
        message += "\n  reserved " + netlist_.nodes.at(id).name + " = " + value;
      }
      for (const auto& operation : operations) {
        message += "\n  " + operation.origin + ": " + operation.command;
        for (const auto& arg : operation.arguments) message += " " + arg;
        for (const auto& frame : operation.call_stack) {
          message += "\n    while " + frame;
        }
      }
      throw std::runtime_error(message);
    }
  }

  std::vector<std::vector<Operation>> ResetPrerequisites(
      const std::vector<Operation>& operations,
      const std::map<size_t, std::string>& preserved,
      const std::map<size_t, std::string>& unwritten) {
    auto targets = ResetTargets(operations);
    std::set<size_t> controls;
    std::vector<size_t> pending(targets.begin(), targets.end());
    while (!pending.empty()) {
      const auto id = pending.back();
      pending.pop_back();
      const auto found = reset_dependencies_.find(id);
      if (found == reset_dependencies_.end()) continue;
      for (size_t parent : found->second) {
        if (controls.insert(parent).second) pending.push_back(parent);
      }
    }
    if (controls.empty()) return {};
    targets.insert(controls.begin(), controls.end());
    z3::optimize solver(context_);
    z3::params parameters(context_);
    parameters.set("timeout", 2000U);
    solver.set(parameters);
    Values assignments;
    std::map<size_t, SymbolicValue> choices;
    std::vector<z3::expr> tie_breaks;
    auto changes = context_.int_val(0);
    size_t total_bits = 0;
    for (const auto& [name, id] : netlist_.node_ids) {
      const auto& node = netlist_.nodes[id];
      if (controls.count(id) && node.kind == "DataRegister" &&
          !state_.DataRegisters().writes.count(id)) {
        assignments.emplace(id,
                            SymbolicValue::Literal(context_, state_.Value(id)));
      } else if (controls.count(id)) {
        if ((node.kind != "ScanRegister" && node.kind != "DataRegister") ||
            instruction_registers_.count(id)) {
          throw std::runtime_error(
              "local-reset control is not a writable register: " + name);
        }
        total_bits += node.Width();
        if (total_bits > 4096) {
          throw std::runtime_error("local-reset solver bit limit exceeded");
        }
        const auto bits =
            context_.bv_const(("reset_" + name).c_str(), node.Width());
        auto known =
            SymbolicValue::Literal(context_, std::string(node.Width(), '1'))
                .bits;
        const auto previous =
            SymbolicValue::Literal(context_, state_.Value(id));
        const auto frozen = unwritten.find(id);
        if (frozen != unwritten.end()) {
          const auto mask = SymbolicValue::Literal(context_, frozen->second);
          known = (known & ~mask.bits) | (previous.known & mask.bits);
          solver.add((bits & mask.bits) == (previous.bits & mask.bits));
        }
        const SymbolicValue value{bits, known};
        assignments.emplace(id, value);
        choices.emplace(id, value);
        const auto held = preserved.find(id);
        if (held != preserved.end()) {
          solver.add(
              Matches(value, SymbolicValue::Literal(context_, held->second)));
        }
        changes =
            changes + z3::ite(bits == previous.bits && previous.known == known,
                              context_.int_val(0), context_.int_val(1));
        const auto fill = SymbolicValue::Literal(context_, fill_.at(id));
        tie_breaks.push_back(bits ^ fill.bits);
      } else if (node.model == ModelKind::kRegister || state_.IsDriven(id)) {
        assignments.emplace(id,
                            SymbolicValue::Literal(context_, state_.Value(id)));
      }
    }
    SymbolicNetwork values(netlist_, context_, std::move(assignments),
                           global_resets_);
    for (size_t id : targets) {
      const auto& node = netlist_.nodes[id];
      if (node.reset) solver.add(Known(values.Read(*node.reset), false));
    }
    solver.minimize(changes);
    for (const auto& tie : tie_breaks) solver.minimize(tie);
    const auto result = solver.check();
    if (result != z3::sat) {
      std::string message =
          result == z3::unsat
              ? "local-reset requirements cannot be released together before "
                "this write/call"
              : "local-reset solver limit; release feasibility not proved";
      for (size_t id : controls) {
        if (netlist_.nodes[id].kind == "DataRegister" && !choices.count(id)) {
          message += "\n  fixed DataRegister reset control " +
                     netlist_.nodes[id].name + " = " + state_.Value(id);
        }
      }
      for (const auto& [id, value] : preserved) {
        if (controls.count(id)) {
          message +=
              "\n  reserved " + netlist_.nodes.at(id).name + " = " + value;
        }
      }
      for (const auto& [id, mask] : unwritten) {
        if (controls.count(id)) {
          message +=
              "\n  taken-but-unwritten resource " + netlist_.nodes.at(id).name;
        }
      }
      throw std::runtime_error(message);
    }
    const auto model = solver.get_model();
    std::map<size_t, std::string> replacements;
    for (const auto& [id, choice] : choices) {
      const auto bits = Decode(model, choice);
      if (bits != state_.Value(id)) replacements.emplace(id, bits);
    }
    std::map<size_t, size_t> levels;
    for (size_t group = 0; group < reset_components_.size(); ++group) {
      const auto& component = reset_components_[group];
      if (std::none_of(component.begin(), component.end(),
                       [&](size_t id) { return replacements.count(id); })) {
        continue;
      }
      size_t level = 0;
      for (size_t id : component) {
        for (size_t parent : reset_dependencies_[id]) {
          const auto found = levels.find(reset_component_.at(parent));
          if (found != levels.end()) level = std::max(level, found->second + 1);
        }
      }
      levels.emplace(group, level);
    }
    std::map<size_t, std::vector<Operation>> layers;
    for (size_t id : reset_order_) {
      if (!replacements.count(id)) continue;
      const auto& bits = replacements.at(id);
      const size_t level = levels.at(reset_component_.at(id));
      const auto& node = netlist_.nodes[id];
      const auto frozen = unwritten.find(id);
      if (frozen == unwritten.end()) {
        layers[level].push_back(
            {"iWrite", {node.name, "0b" + bits}, node.origin, {}});
      } else {
        // Do not turn a taken-but-unwritten bit into a protected assignment.
        for (size_t begin = 0; begin < bits.size();) {
          if (frozen->second[begin] == '1') {
            ++begin;
            continue;
          }
          size_t end = begin + 1;
          while (end < bits.size() && frozen->second[end] == '0') ++end;
          const auto index = [&](size_t offset) {
            return node.left + (node.left <= node.right
                                    ? static_cast<int64_t>(offset)
                                    : -static_cast<int64_t>(offset));
          };
          std::string name = node.name + "[" + std::to_string(index(begin));
          if (end > begin + 1) name += ":" + std::to_string(index(end - 1));
          name += "]";
          layers[level].push_back(
              {"iWrite",
               {name, "0b" + bits.substr(begin, end - begin)},
               node.origin,
               {}});
          begin = end;
        }
      }
    }
    std::vector<std::vector<Operation>> groups;
    for (auto& [level, group] : layers) {
      std::sort(group.begin(), group.end(), [](const auto& a, const auto& b) {
        return a.arguments[0] < b.arguments[0];
      });
      groups.push_back(std::move(group));
    }
    return groups;
  }
  void CheckLocalResets(const std::vector<Operation>& operations) const {
    for (size_t id : ResetTargets(operations)) {
      const auto found = reset_dependencies_.find(id);
      const auto& node = netlist_.nodes[id];
      if (found != reset_dependencies_.end() && !found->second.empty() &&
          node.reset && state_.Read(*node.reset) != '0') {
        throw std::runtime_error(
            node.origin + ": local-reset remains active or unknown for " +
            node.name +
            "; cannot repair this access without changing a procedure or "
            "pending group");
      }
    }
  }

 private:
  std::map<size_t, std::string> DesiredPreserved(
      const std::map<size_t, std::string>& preserved,
      const std::map<size_t, std::string>& end_goals, bool explicit_writes,
      const std::vector<Operation>& operations) {
    auto desired = preserved;
    for (const auto& [id, value] : preserved) {
      const auto& node = netlist_.nodes.at(id);
      if (!IsReservableResource(node) || value.size() != node.Width() ||
          value.find_first_not_of("01x") != std::string::npos) {
        throw std::runtime_error("invalid reserved resource value");
      }
    }
    // Physical goals may include mapped child enables. Only original logical
    // assignments may replace taken bits; a child write is not a parent write.
    if (explicit_writes && !preserved.empty()) {
      for (const auto& operation : operations) {
        if (operation.command != "iWrite") continue;
        const auto target =
            ResolvePdlTarget(netlist_, operation.arguments.at(0));
        if (std::none_of(
                target.bits.begin(), target.bits.end(),
                [&](const auto& bit) { return desired.count(bit.node); })) {
          continue;
        }
        const auto bits =
            PdlDataValue(context_, netlist_, target, operation.arguments.at(1));
        for (size_t i = 0; i < bits.size(); ++i) {
          const auto& ref = target.bits[i];
          if (!desired.count(ref.node) || bits[i] == 'x') continue;
          auto& value = desired.at(ref.node).at(
              Offset(netlist_.nodes[ref.node], ref.index));
          if (value != 'x') {
            value = ref.inverted ? (bits[i] == '1' ? '0' : '1') : bits[i];
          }
        }
      }
    }
    for (const auto& [id, value] : desired) {
      const auto& node = netlist_.nodes.at(id);
      const auto defaults = end_goals.find(id);
      if (defaults != end_goals.end()) {
        for (size_t i = 0; i < value.size(); ++i) {
          if (value[i] != 'x' && defaults->second[i] != 'x' &&
              defaults->second[i] != value[i]) {
            throw std::runtime_error(
                node.origin + ": iApplyEndState conflicts with reserved " +
                node.name);
          }
        }
      }
    }
    return desired;
  }
  void ConstrainPreserved(z3::optimize& solver, SymbolicNetwork& state,
                          const std::map<size_t, std::string>& preserved,
                          const std::map<size_t, std::string>& desired,
                          std::map<size_t, z3::expr>& changed) {
    for (const auto& [id, previous] : preserved) {
      const auto old = SymbolicValue::Literal(context_, previous);
      const auto wanted = SymbolicValue::Literal(context_, desired.at(id));
      const auto value = state.Value(id);
      solver.add((!changed.at(id) && Matches(value, old)) ||
                 Matches(value, wanted));
      changed.at(id) = changed.at(id) || Matches(value, wanted);
    }
  }
  void CheckPreserved(const NetworkState& state,
                      std::map<size_t, std::string>& held,
                      const std::map<size_t, std::string>& desired) const {
    for (auto& [id, previous] : held) {
      const auto actual = Masked(state.Value(id), previous);
      if (actual != previous && actual != desired.at(id)) {
        throw std::runtime_error("concrete replay changed reserved resource " +
                                 netlist_.nodes.at(id).name);
      }
      previous = actual;
    }
  }
  std::set<size_t> ResetTargets(
      const std::vector<Operation>& operations) const {
    std::set<size_t> targets, aliases;
    bool canceled = false;
    for (auto it = operations.rbegin(); it != operations.rend(); ++it) {
      const auto& operation = *it;
      if (operation.command == "iReset") canceled = true;
      if (operation.command == "iApply") canceled = false;
      if (canceled) continue;
      if (operation.command != "iWrite" && operation.command != "iRead") {
        continue;
      }
      const auto target = ResolvePdlTarget(netlist_, operation.arguments.at(0));
      for (const auto& bit : target.bits) {
        if (bit.node != kMissing && netlist_.nodes[bit.node].reset) {
          targets.insert(bit.node);
        }
      }
      aliases.insert(target.access_together.begin(),
                     target.access_together.end());
    }
    std::vector<size_t> pending(aliases.begin(), aliases.end());
    while (!pending.empty()) {
      const auto id = pending.back();
      pending.pop_back();
      const auto target = ResolvePdlTarget(netlist_, netlist_.nodes[id].name);
      for (size_t alias : target.access_together) {
        if (aliases.insert(alias).second) pending.push_back(alias);
      }
      for (const auto& bit : target.bits) {
        if (bit.node != kMissing && netlist_.nodes[bit.node].reset) {
          targets.insert(bit.node);
        }
      }
    }
    return targets;
  }
  static bool PrimaryInput(const NetlistNode& node) {
    return node.scope == 0 &&
           (node.kind == "DataInPort" || node.kind == "AddressPort" ||
            node.kind == "WriteEnPort" || node.kind == "ReadEnPort") &&
           !node.inputs.count("Source");
  }
  static bool ParallelControl(const NetlistNode& node) {
    return node.kind == "AddressPort" || node.kind == "ReadEnPort" ||
           node.kind == "WriteEnPort";
  }
  static bool DataPort(const NetlistNode& node) {
    return node.kind == "DataInPort" || node.kind == "DataOutPort" ||
           ParallelControl(node);
  }
  bool FixedOneHotPadding(BitReference ref) const {
    std::set<Key> visited;
    while (ref.node != kMissing &&
           visited.insert({ref.node, ref.index}).second) {
      const auto& node = netlist_.nodes.at(ref.node);
      const size_t offset = Offset(node, ref.index);
      if (node.kind == "OneHotDataGroup") {
        return !node.one_hot_sources.empty() &&
               std::all_of(node.one_hot_sources.begin(),
                           node.one_hot_sources.end(),
                           [&](const OneHotSource& source) {
                             return source.kind == OneHotSourceKind::kPort &&
                                    offset < node.Width() - source.bits.size();
                           });
      }
      const auto source = node.inputs.find("Source");
      if (source == node.inputs.end()) break;
      ref = source->second.at(offset);
    }
    return false;
  }
  std::string WriteFill(size_t id) const {
    auto goal = DefaultFill(id);
    const auto& node = netlist_.nodes[id];
    for (size_t bit = 0; bit < goal.size(); ++bit) {
      const int64_t index =
          node.left + (node.left <= node.right ? static_cast<int64_t>(bit)
                                               : -static_cast<int64_t>(bit));
      // Tessent permits X on structurally padded bits; disabled ports are
      // not padding and retain their distinct 0/1 controlling state.
      if (FixedOneHotPadding({id, index})) goal[bit] = 'x';
    }
    return goal;
  }
  BitReference WriteReference(
      BitReference ref, std::unique_ptr<AddressResolver>* controls) const {
    if (ref.node == kMissing) return ref;
    const auto& node = netlist_.nodes.at(ref.node);
    if (node.scope != 0 &&
        (node.kind == "ReadEnPort" || node.kind == "WriteEnPort")) {
      if (!*controls) *controls = std::make_unique<AddressResolver>(netlist_);
      auto terminal = (*controls)->ResolveEnable(
          node.scope, node.kind == "ReadEnPort" ? AddressOperation::kRead
                                                : AddressOperation::kWrite);
      terminal.inverted = terminal.inverted != ref.inverted;
      return terminal;
    }
    return ref;
  }

  std::vector<ScanStep> SolvePrimaryWrites(
      const std::vector<Operation>& operations, bool together,
      const std::map<size_t, std::string>& preserved, bool explicit_writes) {
    const auto& data_plan = state_.DataRegisters();
    std::map<size_t, std::string> goals;
    std::map<size_t, std::map<int64_t, char>> reads;
    std::unique_ptr<AddressResolver> controls;
    for (const auto& operation : operations) {
      const bool read = operation.command == "iRead";
      if ((!read && operation.command != "iWrite") ||
          operation.arguments.empty() || operation.arguments.size() > 2 ||
          (!read && operation.arguments.size() != 2)) {
        throw std::runtime_error(
            "primary-input scheduling requires literal iWrite/iRead "
            "target/value");
      }
      const auto target = ResolvePdlTarget(netlist_, operation.arguments[0]);
      if (!target.access_together.empty()) {
        throw std::runtime_error(
            "AccessTogether primary-port groups are not implemented");
      }
      const auto bits = operation.arguments.size() == 1
                            ? std::string(target.bits.size(), 'x')
                            : PdlDataValue(context_, netlist_, target,
                                           operation.arguments[1]);
      for (size_t i = 0; i < bits.size(); ++i) {
        auto ref = target.bits[i];
        if (!read) ref = WriteReference(ref, &controls);
        if (read) {
          if (ref.node == kMissing ||
              (netlist_.nodes.at(ref.node).kind != "DataRegister" &&
               netlist_.nodes.at(ref.node).kind != "DataOutPort")) {
            throw std::runtime_error(
                "primary read requires a DataRegister or DataOutPort");
          }
          const char value = ref.inverted && bits[i] != 'x'
                                 ? (bits[i] == '1' ? '0' : '1')
                                 : bits[i];
          reads[ref.node][ref.index] = value;
          continue;
        }
        if (ref.node == kMissing ||
            (netlist_.nodes.at(ref.node).kind != "DataInPort" &&
             netlist_.nodes.at(ref.node).kind != "DataOutPort" &&
             !ParallelControl(netlist_.nodes.at(ref.node)) &&
             !PrimaryInput(netlist_.nodes.at(ref.node)) &&
             !data_plan.writes.count(ref.node))) {
          throw std::runtime_error(
              "primary write requires a data port or its Alias");
        }
        auto found = goals.find(ref.node);
        if (found == goals.end()) {
          auto goal = WriteFill(ref.node);
          found = goals.emplace(ref.node, std::move(goal)).first;
        }
        if (bits[i] != 'x') {
          found->second.at(Offset(netlist_.nodes[ref.node], ref.index)) =
              ref.inverted ? (bits[i] == '1' ? '0' : '1') : bits[i];
        }
      }
    }
    const DataReadRoutes* routes = nullptr;
    std::map<size_t, std::set<size_t>> pending_reads;
    if (!reads.empty()) {
      if (!data_read_routes_) {
        data_read_routes_ = std::make_unique<DataReadRoutes>(netlist_);
      }
      routes = data_read_routes_.get();
      for (const auto& [id, bits] : reads) {
        for (const auto& [index, expected] : bits) {
          if (!routes->routes.count({id, index})) {
            throw std::runtime_error("unobservable data read target " +
                                     netlist_.nodes[id].name);
          }
          if (netlist_.nodes[id].kind == "DataRegister") {
            pending_reads[id].insert(id);
          } else {
            for (const auto& route : routes->routes.at({id, index})) {
              const auto found =
                  routes->storage.find({route.output.node, route.output.index});
              if (found != routes->storage.end()) {
                for (size_t reg : found->second) pending_reads[reg].insert(id);
              }
            }
          }
        }
      }
    }
    struct ReadChoice {
      Key bit;
      size_t route;
      z3::expr selected;
    };
    struct PrimaryFrame {
      Values state, fill, activation;
      std::map<size_t, z3::expr> drives;
      std::vector<ReadChoice> reads;
    };
    std::vector<size_t> pins;
    size_t total_bits = 0;
    for (const auto& [name, id] : netlist_.node_ids) {
      if (!PrimaryInput(netlist_.nodes[id])) continue;
      pins.push_back(id);
      total_bits += netlist_.nodes[id].Width();
    }
    if (pins.size() > 256 || total_bits > 4096) {
      throw std::runtime_error(
          "primary-input scheduler resource limit exceeded");
    }
    const auto end_goals = EndGoals(operations);
    std::set<size_t> explicit_goals;
    for (const auto& [id, goal] : goals) explicit_goals.insert(id);
    const auto desired_preserved =
        DesiredPreserved(preserved, end_goals, explicit_writes, operations);
    const auto started = std::chrono::steady_clock::now();
    const size_t limit =
        data_plan.order.empty() && !data_reset_dependencies_ ? 1 : kMaxSteps;
    for (size_t horizon = 1; horizon <= limit; ++horizon) {
      if (std::chrono::steady_clock::now() - started >
              std::chrono::seconds(15) ||
          horizon * (netlist_.nodes.size() + total_bits) > 262144) {
        throw std::runtime_error(
            "primary-input scheduling resource limit; reachability not proved");
      }
      if (data_reset_dependencies_ && horizon * reset_work_ > 1048576) {
        throw std::runtime_error(
            "reset resource limit; fixed point not proved");
      }
      z3::optimize solver(context_);
      z3::params parameters(context_);
      parameters.set("timeout", 2000U);
      solver.set(parameters);
      std::map<size_t, z3::expr> reserved_changed;
      for (const auto& [id, value] : preserved) {
        reserved_changed.emplace(id, context_.bool_val(false));
      }
      Values state, fills;
      for (size_t id = 0; id < netlist_.nodes.size(); ++id) {
        if (netlist_.nodes[id].model == ModelKind::kRegister ||
            PrimaryInput(netlist_.nodes[id])) {
          state.emplace(id, SymbolicValue::Literal(context_, state_.Value(id)));
          if (data_plan.writes.count(id)) {
            fills.emplace(id, SymbolicValue::Literal(context_, fill_.at(id)));
          }
        }
      }
      std::map<size_t, z3::expr> touched;
      for (const auto& [id, goal] : goals) {
        bool satisfied = data_plan.writes.count(id);
        const auto actual = state_.Value(id);
        for (size_t bit = 0; bit < goal.size(); ++bit) {
          satisfied =
              satisfied && (goal[bit] == 'x' || goal[bit] == actual[bit]);
        }
        touched.emplace(id, context_.bool_val(satisfied));
      }
      std::map<size_t, z3::expr> read_done;
      for (const auto& [id, bits] : reads) {
        read_done.emplace(id, context_.bool_val(false));
      }
      auto simultaneous = context_.bool_val(false);
      auto count = context_.int_val(0);
      std::vector<z3::expr> deviations, data_deviations;
      std::vector<PrimaryFrame> frames;
      for (size_t time = 0; time < horizon; ++time) {
        PrimaryFrame frame;
        Values next = state;
        auto any_drive = context_.bool_val(false);
        for (size_t id : pins) {
          const auto& node = netlist_.nodes[id];
          const auto suffix = node.name + "_" + std::to_string(time);
          const auto drive = context_.bool_const(("drive_" + suffix).c_str());
          const auto bits =
              context_.bv_const(("pin_" + suffix).c_str(), node.Width());
          const auto known = ~context_.bv_val(0, node.Width());
          next.insert_or_assign(id, Choose(drive, {bits, known}, state.at(id)));
          frame.drives.emplace(id, drive);
          any_drive = any_drive || drive;
          count =
              count + z3::ite(drive, context_.int_val(1), context_.int_val(0));
          deviations.push_back(
              bits ^ SymbolicValue::Literal(context_, fill_.at(id)).bits);
          if (touched.count(id)) touched.at(id) = touched.at(id) || drive;
        }
        Values before_data;
        if (data_reset_dependencies_) {
          SettleResets(&next, &fills, &any_drive, &solver);
          before_data = next;
        }
        for (size_t id : data_plan.order) {
          const auto& node = netlist_.nodes[id];
          const auto suffix = node.name + "_" + std::to_string(time);
          next.insert_or_assign(
              id, SymbolicValue{context_.bv_const(("data_" + suffix).c_str(),
                                                  node.Width()),
                                context_.bv_const(("known_" + suffix).c_str(),
                                                  node.Width())});
        }
        auto values = std::make_unique<SymbolicNetwork>(netlist_, context_,
                                                        next, global_resets_);
        for (size_t id : data_plan.order) {
          const auto& node = netlist_.nodes[id];
          const auto active =
              Choose(any_drive, values->Active(data_plan.writes.at(id)),
                     SymbolicValue::Literal(context_, "0"));
          const auto load = values->Bits(node.inputs.at("WriteDataSource"));
          auto updated = ResetValue(
              active, load,
              data_reset_dependencies_ ? before_data.at(id) : state.at(id));
          auto enabled = Known(active, true);
          auto definite_event = enabled || Known(active, false);
          if (node.reset) {
            const auto reset = values->Read(*node.reset);
            updated =
                ResetValue(reset,
                           SymbolicValue::Literal(
                               context_, node.constants.at("ResetValue").data),
                           updated);
            enabled = enabled && Known(reset, false);
            definite_event = definite_event || Known(reset, true);
          }
          // A read-only frame emits no event, including no reset evaluation.
          updated = Choose(any_drive, updated, state.at(id));
          // An emitted input event must not leave unrequested storage exposed
          // to an unknown write-enable (and silently destroy known values).
          solver.add(definite_event);
          solver.add(next.at(id).bits == updated.bits &&
                     next.at(id).known == updated.known);
          solver.add(z3::implies(
              enabled, load.known == ~context_.bv_val(0, node.Width())));
          const auto inputs = data_plan.primary_inputs.find(id);
          const bool explicit_pins =
              inputs != data_plan.primary_inputs.end() &&
              std::any_of(inputs->second.begin(), inputs->second.end(),
                          [&](size_t pin) { return goals.count(pin); });
          if (!explicit_pins) {
            data_deviations.push_back(
                z3::ite(enabled, load.bits ^ fills.at(id).bits,
                        context_.bv_val(0, node.Width())));
          }
          auto fill = Choose(enabled, load, fills.at(id));
          if (node.reset) {
            fill = Choose(
                any_drive && Known(values->Read(*node.reset), true),
                SymbolicValue::Literal(context_, initial_fill_.at(id)), fill);
          }
          fills.at(id) = fill;
          frame.activation.emplace(id, active);
          if (touched.count(id)) {
            auto requested_drive = context_.bool_val(false);
            const auto inputs = data_plan.primary_inputs.find(id);
            if (inputs != data_plan.primary_inputs.end()) {
              for (size_t pin : inputs->second) {
                if (frame.drives.count(pin)) {
                  requested_drive = requested_drive || frame.drives.at(pin);
                }
              }
            }
            touched.at(id) = touched.at(id) || (enabled && requested_drive);
          }
        }
        if (data_reset_dependencies_) {
          SettleResets(&next, &fills, &any_drive, &solver);
          values = std::make_unique<SymbolicNetwork>(netlist_, context_, next,
                                                     global_resets_);
        }
        for (const auto& [id, readers] : pending_reads) {
          for (size_t read : readers) {
            solver.add(
                z3::implies(!read_done.at(read),
                            next.at(id).bits == state.at(id).bits &&
                                next.at(id).known == state.at(id).known));
          }
        }
        auto all_reads = context_.bool_val(true);
        std::map<std::pair<Key, char>, z3::expr> output_comparisons;
        for (const auto& [id, bits] : reads) {
          const auto observe = context_.bool_const(
              ("observe_" + std::to_string(id) + "_" + std::to_string(time))
                  .c_str());
          solver.add(z3::implies(observe, !read_done.at(id)));
          all_reads = all_reads && observe;
          for (const auto& [index, expected] : bits) {
            const Key key{id, index};
            const auto& choices = routes->routes.at(key);
            auto selected_count = context_.int_val(0);
            for (size_t route = 0; route < choices.size(); ++route) {
              const auto& path = choices[route];
              const auto selected = context_.bool_const(
                  ("read_route_" + std::to_string(id) + "_" +
                   std::to_string(index) + "_" + std::to_string(time) + "_" +
                   std::to_string(route))
                      .c_str());
              auto valid = context_.bool_val(true);
              for (const auto& guard : path.guards) {
                valid = valid && DataReadGuardActive(*values, guard);
              }
              solver.add(z3::implies(selected, valid));
              selected_count =
                  selected_count +
                  z3::ite(selected, context_.int_val(1), context_.int_val(0));
              frame.reads.push_back({key, route, selected});
              if (expected != 'x') {
                const char compare = path.output.inverted
                                         ? (expected == '1' ? '0' : '1')
                                         : expected;
                const auto physical = std::make_pair(
                    Key{path.output.node, path.output.index}, compare);
                const auto found = output_comparisons.find(physical);
                if (found == output_comparisons.end()) {
                  output_comparisons.emplace(physical, selected);
                } else {
                  found->second = found->second || selected;
                }
              }
            }
            solver.add(selected_count == z3::ite(observe, context_.int_val(1),
                                                 context_.int_val(0)));
          }
          read_done.at(id) = read_done.at(id) || observe;
        }
        for (const auto& [key, selected] : output_comparisons) {
          const auto opposite = output_comparisons.find(
              {key.first, key.second == '1' ? '0' : '1'});
          if (opposite != output_comparisons.end()) {
            solver.add(!(selected && opposite->second));
          }
        }
        auto complete = context_.bool_val(true);
        for (const auto& [id, goal] : goals) {
          const auto wanted = SymbolicValue::Literal(context_, goal);
          const auto actual = values->Value(id);
          complete = complete &&
                     (actual.known & wanted.known) == wanted.known &&
                     (actual.bits & wanted.known) == wanted.bits;
          if (frame.drives.count(id)) {
            complete = complete && frame.drives.at(id);
          }
          if (frame.activation.count(id)) {
            complete = complete && Known(frame.activation.at(id), true);
          }
        }
        simultaneous = simultaneous || (complete && all_reads);
        ConstrainPreserved(solver, *values, preserved, desired_preserved,
                           reserved_changed);
        frame.state = next;
        frame.fill = fills;
        frames.push_back(std::move(frame));
        state = std::move(next);
      }
      SymbolicNetwork final(netlist_, context_, state, global_resets_);
      for (const auto& [id, done] : read_done) solver.add(done);
      for (const auto& [id, goal] : goals) {
        const auto wanted = SymbolicValue::Literal(context_, goal);
        const auto actual = final.Value(id);
        solver.add((actual.known & wanted.known) == wanted.known &&
                   (actual.bits & wanted.known) == wanted.bits);
        if (PrimaryInput(netlist_.nodes[id]) || data_plan.writes.count(id)) {
          solver.add(touched.at(id));
        }
      }
      for (const auto& [id, bits] : end_goals) {
        const auto wanted = SymbolicValue::Literal(context_, bits);
        const auto actual = final.Value(id);
        solver.add((actual.known & wanted.known) == wanted.known &&
                   (actual.bits & wanted.known) == wanted.bits);
      }
      if (together) solver.add(simultaneous);
      for (const auto& deviation : data_deviations) {
        MinimizeBits(&solver, deviation);
      }
      solver.minimize(count);
      for (const auto& deviation : deviations) MinimizeBits(&solver, deviation);
      const auto status = solver.check();
      if (status == z3::unsat) continue;
      if (status != z3::sat) {
        throw std::runtime_error("primary-input solver resource limit reached");
      }
      const auto model = solver.get_model();
      NetworkState replay(state_);
      auto replayed_preserved = preserved;
      auto fill = fill_;
      std::vector<ScanStep> steps;
      for (const auto& frame : frames) {
        replay.TakeControlEvents();
        ScanStep step;
        step.together = together;
        std::map<size_t, std::string> driven;
        for (const auto& [id, drive] : frame.drives) {
          if (!model.eval(drive, true).is_true()) continue;
          const auto bits = Decode(model, frame.state.at(id));
          driven.emplace(id, bits);
          fill.at(id) = bits;
          step.writes.emplace(netlist_.nodes[id].name, bits);
        }
        if (!driven.empty()) replay.ApplyInputs(driven);
        const auto control_events = replay.TakeControlEvents();
        CheckPreserved(replay, replayed_preserved, desired_preserved);
        Values concrete;
        for (const auto& [id, value] : frame.state) {
          concrete.emplace(id,
                           SymbolicValue::Literal(context_, replay.Value(id)));
        }
        SymbolicNetwork observed(netlist_, context_, std::move(concrete),
                                 global_resets_);
        for (const auto& choice : frame.reads) {
          if (!model.eval(choice.selected, true).is_true()) continue;
          const auto& route = routes->routes.at(choice.bit).at(choice.route);
          for (const auto& guard : route.guards) {
            if (!DataReadGuardActive(observed, guard).simplify().is_true()) {
              throw std::runtime_error(
                  "symbolic/concrete data read route mismatch");
            }
          }
          char expected = reads.at(choice.bit.first).at(choice.bit.second);
          if (expected != 'x' && route.output.inverted) {
            expected = expected == '1' ? '0' : '1';
          }
          const auto& output = netlist_.nodes[route.output.node];
          auto& comparisons =
              step.expected.try_emplace(output.name, output.Width(), 'x')
                  .first->second;
          if (expected != 'x') {
            comparisons.at(Offset(output, route.output.index)) = expected;
          }
        }
        for (const auto& [id, value] : frame.state) {
          if (replay.Value(id) != Decode(model, value)) {
            throw std::runtime_error(
                "symbolic/concrete primary replay mismatch at " +
                netlist_.nodes[id].name);
          }
        }
        for (size_t id : data_plan.order) {
          const auto event = control_events.find(id);
          if (event != control_events.end()) {
            fill.at(id) = event->second ? *event->second : initial_fill_.at(id);
          }
          if (fill.at(id) != Decode(model, frame.fill.at(id))) {
            throw std::runtime_error(
                "symbolic/concrete primary data fill mismatch");
          }
        }
        AppendDataEndOverrides(replay, &step);
        if (!step.writes.empty() || !step.expected.empty()) {
          steps.push_back(std::move(step));
        }
      }
      for (const auto& [id, goal] : goals) {
        const auto actual = replay.Value(id);
        for (size_t i = 0; i < goal.size(); ++i) {
          if (goal[i] != 'x' && actual[i] != goal[i]) {
            throw std::runtime_error(
                "symbolic/concrete primary goal mismatch at " +
                netlist_.nodes[id].name);
          }
          if (goal[i] != 'x') fill.at(id)[i] = goal[i];
        }
      }
      state_.Restore(replay);
      fill_ = std::move(fill);
      return steps;
    }
    throw std::runtime_error(
        "cannot justify primary-input write requirements within the scan "
        "budget");
  }

  void AppendDataEndOverrides(const NetworkState& replay,
                              ScanStep* step) const {
    if (!step->writes.empty()) {
      // Preserve temporary data-register overrides when flat PDL writes
      // the physical ports instead of naming the abstract register.
      for (const auto& [id, defaults] : end_states_) {
        if (!state_.DataRegisters().writes.count(id) &&
            !PrimaryInput(netlist_.nodes[id])) {
          continue;
        }
        const auto bits = replay.Value(id);
        bool overridden = false;
        for (size_t bit = 0; bit < bits.size(); ++bit) {
          overridden =
              overridden || (defaults[bit] != 'x' && bits[bit] != 'x' &&
                             defaults[bit] != bits[bit]);
        }
        if (overridden) step->writes.emplace(netlist_.nodes[id].name, bits);
      }
    }
  }
  z3::expr DataReadGuardActive(SymbolicNetwork& values,
                               const DataReadGuard& guard) {
    const auto& node = netlist_.nodes[guard.node];
    if (guard.choice == kMissing) {
      return Known(values.Read(node.inputs.at("Enable").at(0)), true);
    }
    auto valid = context_.bool_val(true);
    if (node.kind == "OneHotDataGroup") {
      for (size_t i = 0; i < node.one_hot_sources.size(); ++i) {
        valid = valid && Known(values.OneHotActive(node.one_hot_sources[i]),
                               i == guard.choice);
      }
    } else {
      const auto selector = values.Bits(node.inputs.at("SelectedBy"));
      valid = selector.known ==
              ~context_.bv_val(0, selector.bits.get_sort().bv_size());
      for (size_t i = 0; i < node.choices.size(); ++i) {
        auto match = context_.bool_val(false);
        for (const auto& code : node.choices[i].codes) {
          const auto wanted = SymbolicValue::Literal(context_, code.data);
          match = match || ((selector.bits ^ wanted.bits) & wanted.known) ==
                               context_.bv_val(0, code.data.size());
        }
        valid = valid && (i == guard.choice ? match : !match);
      }
    }
    return valid;
  }

  struct Edge {
    size_t next, choice;
    bool inverted;
  };
  struct Vertex {
    BitReference bit;
    std::vector<Edge> edges;
    bool input = false;
  };
  size_t Add(BitReference bit) {
    // Serial graph expansion is iterative: real corpora have long scan chains.
    const Key requested{bit.node, bit.index};
    if (ids_.count(requested)) return ids_.at(requested);
    std::vector<std::pair<BitReference, bool>> pending{{bit, false}};
    std::set<Key> visiting;
    while (!pending.empty()) {
      const auto [current, expanded] = pending.back();
      pending.pop_back();
      const Key key{current.node, current.index};
      if (expanded) {
        visiting.erase(key);
        order_.push_back(ids_.at(key));
        continue;
      }
      if (visiting.count(key)) {
        throw std::runtime_error("cyclic serial scan path");
      }
      if (ids_.count(key)) continue;
      const size_t id = graph_.size();
      ids_[key] = id;
      graph_.push_back({current, {}, false});
      if (graph_.size() > 65536) {
        throw std::runtime_error("scan graph resource limit exceeded");
      }
      visiting.insert(key);
      pending.emplace_back(current, true);
      if (current.node == kMissing) continue;
      const auto& node = netlist_.nodes.at(current.node);
      std::vector<std::pair<BitReference, size_t>> sources;
      if (node.kind == "ScanRegister") {
        if (current.index != node.right) {
          throw std::runtime_error(node.origin +
                                   ": intermediate serial taps require "
                                   "overlapping-storage scheduling");
        }
        sources.emplace_back(node.inputs.at("ScanInSource").at(0), kMissing);
      } else if (node.kind == "ScanMux") {
        for (size_t c = 0; c < node.choices.size(); ++c) {
          sources.emplace_back(
              node.choices[c].source.at(Offset(node, current.index)), c);
        }
      } else if (node.kind == "OneHotScanGroup") {
        for (size_t c = 0; c < node.one_hot_sources.size(); ++c) {
          sources.emplace_back(
              node.OneHotSourceBit(c, Offset(node, current.index)), c);
        }
      } else if (node.inputs.count("Source")) {
        sources.emplace_back(
            node.inputs.at("Source").at(Offset(node, current.index)), kMissing);
      } else if (node.kind == "ScanInPort" && node.scope == 0) {
        graph_[id].input = true;
      }
      // Allocate edges on the expanded pass after source identities exist.
      source_edges_[id] = sources;
      for (const auto& [source, choice] : sources) {
        pending.emplace_back(source, false);
      }
    }
    for (const auto& [id, sources] : source_edges_) {
      if (!graph_[id].edges.empty()) continue;
      for (const auto& [source, choice] : sources) {
        graph_[id].edges.push_back(
            {ids_.at({source.node, source.index}), choice, source.inverted});
      }
    }
    return ids_.at(requested);
  }
  void AddControl(BitReference initial) {
    std::vector<BitReference> pending{initial};
    while (!pending.empty()) {
      const auto bit = pending.back();
      pending.pop_back();
      if (netlist_.taps.size() > 1 &&
          bit.virtual_signal != VirtualSignal::kNone) {
        throw std::runtime_error(
            "multiple TAPs require explicit reset connectivity");
      }
      if (bit.node == kMissing ||
          !control_seen_.insert({bit.node, bit.index}).second) {
        continue;
      }
      const auto& node = netlist_.nodes.at(bit.node);
      if (node.kind == "DataRegister") {
        auto& mask =
            data_control_masks_.try_emplace(bit.node, node.Width(), '0')
                .first->second;
        mask.at(Offset(node, bit.index)) = '1';
      }
      if (node.kind == "ScanRegister") {
        auto entry =
            control_masks_.emplace(bit.node, std::string(node.Width(), '0'))
                .first;
        entry->second.at(Offset(node, bit.index)) = '1';
        continue;
      }
      for (const auto& [name, refs] : node.inputs) {
        if (name == "Source") {
          pending.push_back(refs.at(Offset(node, bit.index)));
        } else {
          pending.insert(pending.end(), refs.begin(), refs.end());
        }
      }
      for (const auto& choice : node.choices) {
        pending.push_back(choice.source.at(Offset(node, bit.index)));
      }
      for (size_t source = 0; source < node.one_hot_sources.size(); ++source) {
        pending.push_back(
            node.OneHotSourceBit(source, Offset(node, bit.index)));
      }
      for (const auto& instruction : node.logic) {
        pending.insert(pending.end(), instruction.signals.begin(),
                       instruction.signals.end());
      }
    }
  }
  void BuildResetOrder() {
    std::vector<size_t> storage;
    for (size_t id = 0; id < netlist_.nodes.size(); ++id) {
      if (netlist_.nodes[id].model == ModelKind::kRegister) {
        storage.push_back(id);
      }
    }
    size_t budget = 4 * 1024 * 1024;
    OneHotReadResolver one_hot(netlist_, &budget);
    auto& dependencies = reset_dependencies_;
    for (size_t id : storage) {
      const auto& node = netlist_.nodes[id];
      if (!node.reset) continue;
      std::vector<size_t> pending;
      if (node.reset->node != kMissing) pending.push_back(node.reset->node);
      std::set<size_t> seen;
      while (!pending.empty()) {
        if (!budget--) {
          throw std::runtime_error("reset dependency resource limit exceeded");
        }
        const auto next = pending.back();
        pending.pop_back();
        if (!seen.insert(next).second) continue;
        const auto& source = netlist_.nodes.at(next);
        if (source.model == ModelKind::kRegister) {
          dependencies[id].insert(next);
          data_reset_dependencies_ =
              data_reset_dependencies_ || source.kind == "DataRegister";
          continue;
        }
        for (const auto& [name, refs] : source.inputs) {
          for (const auto& ref : refs) {
            if (ref.node != kMissing) pending.push_back(ref.node);
          }
        }
        for (const auto& choice : source.choices) {
          for (const auto& ref : choice.source) {
            if (ref.node != kMissing) pending.push_back(ref.node);
          }
        }
        for (const auto& contributor : source.one_hot_sources) {
          for (const auto& bit : contributor.bits) {
            if (bit.node != kMissing) pending.push_back(bit.node);
          }
          for (const auto& condition : one_hot.Resolve(contributor)) {
            if (condition.enable.node != kMissing) {
              pending.push_back(condition.enable.node);
            }
            if (condition.signals) {
              for (const auto& bit : *condition.signals) {
                if (bit.node != kMissing) pending.push_back(bit.node);
              }
            }
          }
        }
        for (const auto& instruction : source.logic) {
          for (const auto& ref : instruction.signals) {
            if (ref.node != kMissing) pending.push_back(ref.node);
          }
        }
      }
    }
    std::map<size_t, size_t> remaining;
    std::map<size_t, std::vector<size_t>> children;
    std::set<size_t> ready;
    for (size_t id : storage) {
      remaining[id] = dependencies[id].size();
      if (remaining[id] == 0) ready.insert(id);
      for (size_t parent : dependencies[id]) children[parent].push_back(id);
    }
    // Visit each dependency once, even for reverse-declared reset chains.
    while (!ready.empty()) {
      const size_t id = *ready.begin();
      ready.erase(ready.begin());
      reset_order_.push_back(id);
      for (size_t child : children[id]) {
        if (--remaining[child] == 0) ready.insert(child);
      }
    }
    feedback_resets_ = reset_order_.size() != storage.size();
    const bool feedback = feedback_resets_;
    if (!feedback) {
      for (size_t id : reset_order_) reset_components_.push_back({id});
    } else {
      // Iterative Kosaraju keeps graph depth off the C++ call stack.
      std::set<size_t> visited;
      std::vector<size_t> finished;
      for (size_t root : storage) {
        std::vector<std::pair<size_t, bool>> pending{{root, false}};
        while (!pending.empty()) {
          const auto [id, expanded] = pending.back();
          pending.pop_back();
          if (expanded) {
            finished.push_back(id);
          } else if (visited.insert(id).second) {
            pending.emplace_back(id, true);
            for (size_t child : children[id]) {
              pending.emplace_back(child, false);
            }
          }
        }
      }
      visited.clear();
      for (auto root = finished.rbegin(); root != finished.rend(); ++root) {
        if (!visited.insert(*root).second) continue;
        std::vector<size_t> component, pending{*root};
        while (!pending.empty()) {
          const size_t id = pending.back();
          pending.pop_back();
          component.push_back(id);
          for (size_t parent : dependencies[id]) {
            if (visited.insert(parent).second) pending.push_back(parent);
          }
        }
        std::sort(component.begin(), component.end());
        reset_components_.push_back(std::move(component));
      }
      reset_order_.clear();
      for (const auto& component : reset_components_) {
        reset_order_.insert(reset_order_.end(), component.begin(),
                            component.end());
      }
    }
    std::vector<size_t> depths(reset_components_.size());
    size_t bits = netlist_.nodes.size();
    for (size_t group = 0; group < reset_components_.size(); ++group) {
      const auto& component = reset_components_[group];
      const bool cyclic =
          component.size() > 1 ||
          dependencies[component.front()].count(component.front());
      for (size_t id : component) reset_component_[id] = group;
      size_t depth = 0;
      for (size_t id : component) {
        bits += netlist_.nodes[id].Width();
        for (size_t parent : dependencies[id]) {
          const size_t ancestor = reset_component_.at(parent);
          if (ancestor != group) depth = std::max(depth, depths.at(ancestor));
        }
      }
      // A reset-only vector moves at most V -> join(V, R) -> R.
      depths[group] = depth + (cyclic ? 2 * component.size()
                               : netlist_.nodes[component.front()].reset ? 1
                                                                         : 0);
      reset_passes_ = std::max(reset_passes_, depths[group]);
    }
    // Observe reset/fill events after the final changing feedback snapshot.
    if (feedback) ++reset_passes_;
    reset_work_ = reset_passes_ * bits;
  }

  z3::expr Match(SymbolicNetwork& values, const NetlistNode& node,
                 size_t choice) {
    if (node.kind == "OneHotScanGroup") {
      auto match = context_.bool_val(true);
      for (size_t i = 0; i < node.one_hot_sources.size(); ++i) {
        match = match && Known(values.OneHotActive(node.one_hot_sources[i]),
                               i == choice);
      }
      return match;
    }
    auto match = context_.bool_val(false);
    for (const auto& code : node.choices.at(choice).codes) {
      auto term = context_.bool_val(true);
      for (size_t i = 0; i < code.data.size(); ++i) {
        if (code.data[i] != 'x') {
          term = term && Known(values.Read(node.inputs.at("SelectedBy").at(i)),
                               code.data[i] == '1');
        }
      }
      match = match || term;
    }
    return match;
  }
  bool Match(const NetworkState& values, const NetlistNode& node,
             size_t choice) {
    if (node.kind == "OneHotScanGroup") {
      for (size_t i = 0; i < node.one_hot_sources.size(); ++i) {
        char active = '0';
        for (const auto& condition :
             one_hot_.Resolve(node.one_hot_sources[i])) {
          const char bit = values.Read(condition.enable);
          active = active == '1' || bit == '1'   ? '1'
                   : active == 'x' || bit == 'x' ? 'x'
                                                 : '0';
        }
        if (active != (i == choice ? '1' : '0')) return false;
      }
      return true;
    }
    for (const auto& code : node.choices.at(choice).codes) {
      bool match = true;
      for (size_t bit = 0; bit < code.data.size(); ++bit) {
        if (code.data[bit] != 'x' &&
            values.Read(node.inputs.at("SelectedBy").at(bit)) !=
                code.data[bit]) {
          match = false;
        }
      }
      if (match) return true;
    }
    return false;
  }
  struct CaptureComparison {
    BitReference output;
    char expected;
    z3::expr selected;
  };
  struct Frame {
    Values state, loads, fill;
    std::vector<CaptureComparison> comparisons;
    std::optional<SymbolicValue> instruction;
    std::optional<z3::expr> parallel;
    std::optional<z3::expr> clocks;
    std::map<size_t, z3::expr> drives;
    std::map<size_t, z3::expr> active, read, needs_write;
    std::map<Key, std::pair<z3::expr, z3::expr>> inputs;
    std::vector<z3::expr> selected, clients;
  };
  void SettleResets(Values* state, Values* fills = nullptr,
                    const z3::expr* event = nullptr,
                    z3::optimize* solver = nullptr) {
    if (event && event->is_false()) return;
    if (reset_work_ > 1048576) {
      throw std::runtime_error("reset resource limit; fixed point not proved");
    }
    for (size_t pass = 0; pass < reset_passes_; ++pass) {
      SymbolicNetwork values(netlist_, context_, *state, global_resets_);
      auto next = *state;
      bool changed = false;
      const auto settle = [&](size_t id) {
        const auto& node = netlist_.nodes[id];
        if (!node.reset) return;
        const auto reset = event ? Choose(*event, values.Read(*node.reset),
                                          SymbolicValue::Literal(context_, "0"))
                                 : values.Read(*node.reset);
        const auto& bits = node.constants.at("ResetValue").data;
        auto updated = ResetValue(reset, SymbolicValue::Literal(context_, bits),
                                  state->at(id));
        updated.bits = updated.bits.simplify();
        updated.known = updated.known.simplify();
        changed = changed || !z3::eq(updated.bits, state->at(id).bits) ||
                  !z3::eq(updated.known, state->at(id).known);
        if (solver && feedback_resets_) {
          const auto bind = [&](z3::expr value, const z3::expr& previous) {
            if (value.is_numeral() || z3::eq(value, previous)) return value;
            // Name snapshots so cyclic reset expressions do not keep expanding.
            const z3::expr named(context_, Z3_mk_fresh_const(context_, "reset",
                                                             value.get_sort()));
            solver->add(named == value);
            return named;
          };
          updated.bits = bind(updated.bits, state->at(id).bits);
          updated.known = bind(updated.known, state->at(id).known);
        }
        next.at(id) = std::move(updated);
        if (fills && fills->count(id)) {
          auto initial = initial_fill_.at(id);
          for (size_t bit = 0; bit < bits.size(); ++bit) {
            if (bits[bit] != 'x') initial[bit] = bits[bit];
          }
          auto fill =
              Choose(Known(reset, true),
                     SymbolicValue::Literal(context_, initial), fills->at(id));
          fill.bits = fill.bits.simplify();
          fill.known = fill.known.simplify();
          fills->at(id) = std::move(fill);
        }
      };
      // Match concrete simultaneous passes, including transient assertions.
      for (size_t id : reset_order_) settle(id);
      *state = std::move(next);
      if (!changed) break;
    }
  }
  void SelectClients(Frame& frame, size_t time) {
    for (size_t i = 0; i < clients_.size(); ++i) {
      frame.clients.push_back(context_.bool_const(
          ("client_" + std::to_string(time) + "_" + std::to_string(i))
              .c_str()));
    }
  }
  void ClientSelections(Values* pins, const Frame& frame,
                        const std::string& phase) {
    for (const auto& [key, clients] : client_selectors_) {
      auto active = context_.bool_val(false);
      for (size_t id : clients) active = active || frame.clients.at(id);
      if (netlist_.nodes.at(key.first).kind == "ShiftEnPort" &&
          phase != "shift") {
        active = context_.bool_val(false);
      }
      const auto bit = BooleanValue(active);
      auto& value = pins->at(key.first);
      const auto& node = netlist_.nodes.at(key.first);
      const auto width = static_cast<unsigned>(node.Width());
      const auto shift =
          context_.bv_val(width - 1 - Offset(node, key.second), width);
      const auto mask = z3::shl(context_.bv_val(1, width), shift);
      const SymbolicValue selected{
          (value.bits & ~mask) | z3::shl(z3::zext(bit.bits, width - 1), shift),
          value.known | mask};
      value =
          frame.parallel ? Choose(!*frame.parallel, selected, value) : selected;
    }
  }
  void SelectScan(z3::optimize& solver, SymbolicNetwork& values, size_t time,
                  Frame& frame) {
    if (!clients_.empty()) {
      auto count = context_.int_val(0);
      for (size_t i = 0; i < clients_.size(); ++i) {
        count = count + z3::ite(frame.clients.at(i), context_.int_val(1),
                                context_.int_val(0));
        auto enabled = context_.bool_val(true);
        for (const auto& [node, index] : clients_[i]) {
          if (netlist_.nodes.at(node).kind == "ShiftEnPort") {
            auto selected = context_.bool_val(false);
            for (size_t client : client_selectors_.at({node, index})) {
              selected = selected || frame.clients.at(client);
            }
            enabled = enabled && selected;
          } else {
            enabled = enabled && Known(values.Read({node, index}), true);
          }
        }
        solver.add(z3::implies(
            frame.parallel ? !*frame.parallel : context_.bool_val(true),
            enabled == frame.clients.at(i)));
      }
      solver.add(count == (frame.parallel
                               ? z3::ite(*frame.parallel, context_.int_val(0),
                                         context_.int_val(1))
                               : context_.int_val(1)));
    }
    const bool paired_inputs =
        std::any_of(client_route_counts_.begin(), client_route_counts_.end(),
                    [](size_t count) { return count > 1; });
    std::vector<z3::expr> viable, observed, lengths, input_lengths, sources;
    for (size_t i = 0; i < graph_.size(); ++i) {
      if (paired_inputs) {
        sources.push_back(
            graph_[i].input
                ? context_.int_val(i)
                : context_.int_const(("scan_input_" + std::to_string(time) +
                                      "_" + std::to_string(i))
                                         .c_str()));
      }
      viable.push_back(context_.bool_val(false));
      observed.push_back(context_.bool_val(false));
      lengths.push_back(context_.int_val(0));
      input_lengths.push_back(context_.int_val(0));
    }
    std::vector<std::vector<z3::expr>> gates(graph_.size());
    for (size_t id : order_) {
      const auto& vertex = graph_[id];
      viable[id] = context_.bool_val(vertex.input);
      for (const auto& edge : vertex.edges) {
        const auto gate =
            edge.choice == kMissing
                ? context_.bool_val(true)
                : Match(values, netlist_.nodes.at(vertex.bit.node),
                        edge.choice);
        gates[id].push_back(gate);
        viable[id] = viable[id] || (gate && viable[edge.next]);
        lengths[id] = z3::ite(gate, lengths[edge.next], lengths[id]);
      }
      if (vertex.bit.node != kMissing) {
        const auto& node = netlist_.nodes.at(vertex.bit.node);
        if (node.kind == "ScanRegister") {
          lengths[id] = lengths[id] + context_.int_val(node.Width());
        }
        const auto enable = node.inputs.find("Enable");
        if (enable != node.inputs.end()) {
          viable[id] =
              viable[id] && Known(values.Read(enable->second.at(0)), true);
        }
      }
      // Overlapping selectors do not define a unique serial transaction.
      for (size_t a = 0; a < gates[id].size(); ++a) {
        for (size_t b = a + 1; b < gates[id].size(); ++b) {
          viable[id] = viable[id] && !(gates[id][a] && gates[id][b]);
        }
      }
    }
    auto count = context_.int_val(0);
    for (size_t i = 0; i < roots_.size(); ++i) {
      auto selected = context_.bool_const(
          ("scan_" + std::to_string(time) + "_" + std::to_string(i)).c_str());
      frame.selected.push_back(selected);
      if (!clients_.empty()) {
        const auto& bit = graph_[roots_[i]].bit;
        auto enabled = context_.bool_val(false);
        for (const auto& [client, input] :
             client_routes_.at({bit.node, bit.index})) {
          enabled = enabled || frame.clients.at(client);
          if (paired_inputs) {
            const auto found = ids_.find(input);
            const auto source = found == ids_.end()
                                    ? context_.int_val(-1)
                                    : context_.int_val(found->second);
            solver.add(z3::implies(frame.clients.at(client),
                                   sources[roots_[i]] == source));
          }
        }
        solver.add(selected == enabled);
      }
      solver.add(z3::implies(selected, viable[roots_[i]]));
      observed[roots_[i]] = observed[roots_[i]] || selected;
      input_lengths[roots_[i]] = lengths[roots_[i]];
      count =
          count + z3::ite(selected, context_.int_val(1), context_.int_val(0));
    }
    auto chains = context_.int_val(parallel_chains_ ? roots_.size() : 1);
    if (paired_inputs) {
      chains = context_.int_val(0);
      for (size_t i = 0; i < clients_.size(); ++i) {
        chains = chains + z3::ite(frame.clients.at(i),
                                  context_.int_val(client_route_counts_[i]),
                                  context_.int_val(0));
      }
    }
    solver.add(count == (frame.parallel ? z3::ite(*frame.parallel,
                                                  context_.int_val(0), chains)
                                        : chains));
    auto clocks = context_.int_val(0);
    for (size_t i = 0; i < roots_.size(); ++i) {
      clocks = z3::ite(frame.selected[i] && lengths[roots_[i]] > clocks,
                       lengths[roots_[i]], clocks);
    }
    frame.clocks = clocks;
    for (auto it = order_.rbegin(); it != order_.rend(); ++it) {
      const size_t id = *it;
      const auto& vertex = graph_[id];
      if (vertex.input) {
        const Key key{vertex.bit.node, vertex.bit.index};
        frame.inputs.emplace(key,
                             std::make_pair(observed[id], input_lengths[id]));
        if (!clients_.empty() && !paired_inputs) {
          auto allowed = context_.bool_val(false);
          const auto clients = client_inputs_.find(key);
          if (clients != client_inputs_.end()) {
            for (size_t client : clients->second) {
              allowed = allowed || frame.clients.at(client);
            }
          }
          solver.add(z3::implies(observed[id], allowed));
        }
      }
      if (vertex.bit.node != kMissing &&
          netlist_.nodes.at(vertex.bit.node).kind == "ScanRegister") {
        const auto old = frame.active.find(vertex.bit.node);
        if (old == frame.active.end()) {
          frame.active.emplace(vertex.bit.node, observed[id]);
        } else {
          old->second = old->second || observed[id];
        }
      }
      for (size_t e = 0; e < vertex.edges.size(); ++e) {
        if (paired_inputs) {
          solver.add(z3::implies(observed[id] && gates[id][e],
                                 sources[id] == sources[vertex.edges[e].next]));
        }
        input_lengths[vertex.edges[e].next] =
            z3::ite(observed[id] && gates[id][e], input_lengths[id],
                    input_lengths[vertex.edges[e].next]);
        observed[vertex.edges[e].next] =
            observed[vertex.edges[e].next] || (observed[id] && gates[id][e]);
      }
    }
  }
  std::optional<std::set<size_t>> ActivePath(const NetworkState& snapshot) {
    if (roots_.size() != 1) return std::nullopt;
    std::set<size_t> active;
    size_t vertex = roots_.front();
    while (!graph_[vertex].input) {
      const auto& current = graph_[vertex];
      if (current.bit.node == kMissing) return std::nullopt;
      const auto& node = netlist_.nodes[current.bit.node];
      const auto enable = node.inputs.find("Enable");
      if (enable != node.inputs.end() &&
          snapshot.Read(enable->second.at(0)) != '1') {
        return std::nullopt;
      }
      if (node.kind == "ScanRegister") active.insert(current.bit.node);
      size_t selected = kMissing;
      for (size_t e = 0; e < current.edges.size(); ++e) {
        const auto& edge = current.edges[e];
        const bool matches =
            edge.choice == kMissing || Match(snapshot, node, edge.choice);
        if (matches) {
          if (selected != kMissing) return std::nullopt;
          selected = e;
        }
      }
      if (selected == kMissing) return std::nullopt;
      vertex = current.edges[selected].next;
    }
    return active;
  }
  std::optional<std::vector<ScanStep>> TryAddressedWrites(
      const std::map<size_t, std::string>& goals,
      const std::map<size_t, std::string>& preserved, bool explicit_writes) {
    const auto& data = state_.DataRegisters();
    const std::vector<BitReference>* address = nullptr;
    std::map<std::string, std::vector<size_t>> batches;
    std::vector<size_t> satisfied;
    for (const auto& [id, goal] : goals) {
      const auto access = data.writes.find(id);
      if (access == data.writes.end() || !access->second.signals) {
        return std::nullopt;
      }
      const auto& signals = *access->second.signals;
      if (!address) address = &signals;
      if (signals.size() != address->size()) return std::nullopt;
      for (size_t bit = 0; bit < signals.size(); ++bit) {
        const auto& a = signals[bit];
        const auto& b = address->at(bit);
        if (a.node == kMissing ||
            netlist_.nodes[a.node].kind != "ScanRegister" || a.node != b.node ||
            a.index != b.index || a.inverted != b.inverted ||
            a.virtual_signal != b.virtual_signal) {
          return std::nullopt;
        }
      }
      auto code = access->second.value.data;
      code.insert(0, signals.size() - code.size(), '0');
      if (state_.Value(id) == goal) {
        satisfied.push_back(id);
      } else {
        batches[code].push_back(id);
      }
    }
    // Already-satisfied data still requires its controllers' UpdateEn, but
    // not a separate address transaction. Include it in the first batch.
    auto& first = batches.empty() ? batches[""] : batches.begin()->second;
    first.insert(first.end(), satisfied.begin(), satisfied.end());
    // One physical address bus cannot issue different address codes on the
    // same update. Avoid searching permutations of these necessary batches.
    const auto started = std::chrono::steady_clock::now();
    Impl trial(netlist_);
    trial.state_.Restore(state_);
    trial.fill_ = fill_;
    trial.global_resets_ = global_resets_;
    trial.tap_ready_ = tap_ready_;
    if (execution_) {
      trial.execution_ = std::make_unique<TapExecution>(*execution_);
    }
    std::vector<ScanStep> steps;
    auto held = preserved;
    for (auto& [code, members] : batches) {
      if (std::chrono::steady_clock::now() - started >
              std::chrono::seconds(15) ||
          steps.size() * (graph_.size() + data.order.size()) > 262144) {
        return std::nullopt;
      }
      std::sort(members.begin(), members.end(), [&](size_t a, size_t b) {
        return netlist_.nodes[a].name < netlist_.nodes[b].name;
      });
      std::vector<Operation> operations;
      for (size_t id : members) {
        const auto& node = netlist_.nodes[id];
        // Goals include the original slice/X fill, before any prerequisite.
        operations.push_back(
            {"iWrite", {node.name, "0b" + goals.at(id)}, node.origin, {}});
      }
      try {
        auto scans = trial.Solve(operations, false, kMaxSteps, {}, false, held,
                                 explicit_writes);
        for (auto& [id, value] : held) {
          value = Masked(trial.state_.Value(id), value);
        }
        steps.insert(steps.end(), std::make_move_iterator(scans.begin()),
                     std::make_move_iterator(scans.end()));
      } catch (const ScheduleLimit&) {
        return std::nullopt;
      }
    }
    // Cross-address reset/control dependencies may invalidate earlier goals.
    // Only publish a complete replayed candidate; otherwise use joint solving.
    for (const auto& [id, goal] : goals) {
      if (trial.state_.Value(id) != goal) return std::nullopt;
    }
    state_.Restore(trial.state_);
    fill_ = std::move(trial.fill_);
    execution_ = std::move(trial.execution_);
    return steps;
  }
  std::optional<std::vector<ScanStep>> TryGreedy(
      const std::vector<Operation>& operations,
      const std::map<size_t, std::string>& goals,
      const std::set<size_t>& required, bool together,
      const std::map<size_t, std::string>& preserved,
      const std::map<size_t, std::string>& desired_preserved,
      bool explicit_writes, bool events) {
    // shortcut: multi-TAP planning stays joint until greedy has a multi-path
    // proof.
    if (netlist_.taps.size() > 1) return std::nullopt;
    const auto start = std::chrono::steady_clock::now();
    NetworkState current(state_);
    if (tap_) current.Drive(tap_->ir_select, "0");
    const auto visible = ActivePath(current);
    if (visible &&
        std::all_of(required.begin(), required.end(), [&](size_t id) {
          const auto& node = netlist_.nodes[id];
          return visible->count(id) &&
                 (!node.reset || current.Read(*node.reset) == '0');
        })) {
      // Already expanded patterns need no prerequisite configuration solve.
      try {
        return Solve(operations, together, 1, {}, false, preserved,
                     explicit_writes, events);
      } catch (const ScheduleLimit&) {
        // A visible target can still need other state/control preparation.
      }
    }
    z3::optimize solver(context_);
    z3::params parameters(context_);
    parameters.set("timeout", 2000U);
    solver.set(parameters);
    Values assignments, choices;
    std::map<size_t, z3::expr> drives;
    std::vector<z3::expr> ties;
    auto count = context_.int_val(0);
    size_t bits = 0;
    for (const auto& [name, id] : netlist_.node_ids) {
      const auto& node = netlist_.nodes[id];
      if (node.kind != "ScanRegister") {
        if (node.model == ModelKind::kRegister || state_.IsDriven(id)) {
          assignments.emplace(
              id, SymbolicValue::Literal(context_, state_.Value(id)));
        }
        continue;
      }
      const auto previous = SymbolicValue::Literal(context_, state_.Value(id));
      const auto mask = control_masks_.find(id);
      if (mask == control_masks_.end()) {
        assignments.emplace(id, previous);
        continue;
      }
      bits += node.Width();
      if (bits > 4096) return std::nullopt;
      const auto drive =
          context_.bool_const(("prerequisite_write_" + name).c_str());
      const auto load = context_.bv_const(
          ("prerequisite_value_" + name).c_str(), node.Width());
      const auto allowed = SymbolicValue::Literal(context_, mask->second);
      const auto fill = SymbolicValue::Literal(context_, fill_.at(id));
      if (preserved.count(id)) {
        const auto held = SymbolicValue::Literal(context_, preserved.at(id));
        const auto desired =
            SymbolicValue::Literal(context_, desired_preserved.at(id));
        solver.add((load & held.known) == held.bits ||
                   (load & desired.known) == desired.bits);
      } else {
        solver.add((load & ~allowed.bits) == (fill.bits & ~allowed.bits));
      }
      SymbolicValue value{load, ~context_.bv_val(0, node.Width())};
      assignments.emplace(id, Choose(drive, value, previous));
      choices.emplace(id, value);
      drives.emplace(id, drive);
      count = count + z3::ite(drive, context_.int_val(1), context_.int_val(0));
      ties.push_back(z3::bv2int(load ^ fill.bits, false));
    }
    if (tap_) {
      assignments.insert_or_assign(tap_->ir_select,
                                   SymbolicValue::Literal(context_, "0"));
    }
    Frame frame;
    SelectClients(frame, 0);
    if (client_controls_) {
      assignments = EventPins(assignments, frame, "capture");
    }
    SettleResets(&assignments, nullptr, nullptr, &solver);
    SymbolicNetwork values(netlist_, context_, assignments, global_resets_);
    SelectScan(solver, values, 0, frame);
    for (size_t id : required) {
      if (!frame.active.count(id)) return std::nullopt;
      solver.add(frame.active.at(id));
      const auto& node = netlist_.nodes[id];
      if (node.reset) solver.add(Known(values.Read(*node.reset), false));
    }
    for (const auto& [id, drive] : drives) {
      const auto& node = netlist_.nodes[id];
      if (node.reset) {
        solver.add(z3::implies(drive, Known(values.Read(*node.reset), false)));
      }
    }
    solver.minimize(count);
    for (const auto& tie : ties) solver.minimize(tie);
    if (solver.check() != z3::sat) return std::nullopt;
    const auto model = solver.get_model();
    std::map<size_t, std::string> configuration;
    for (const auto& [id, drive] : drives) {
      if (model.eval(drive, true).is_true()) {
        configuration.emplace(id, Decode(model, choices.at(id)));
      }
    }

    // Speculative scans never change live hardware or controlling fill state.
    Impl trial(netlist_);
    trial.state_.Restore(state_);
    trial.fill_ = fill_;
    trial.global_resets_ = global_resets_;
    trial.tap_ready_ = tap_ready_;
    if (execution_) {
      trial.execution_ = std::make_unique<TapExecution>(*execution_);
    }
    std::vector<ScanStep> steps;
    auto held = preserved;
    size_t previous_pending = configuration.size() + 1;
    for (size_t batch = 0; batch <= configuration.size(); ++batch) {
      if (std::chrono::steady_clock::now() - start > std::chrono::seconds(15)) {
        return std::nullopt;
      }
      NetworkState preview(trial.state_);
      if (tap_) {
        preview.Drive(tap_->ir_select, "0");
        const auto ir = configuration.find(tap_->instruction_register);
        if (ir != configuration.end()) {
          preview.SetRegister(ir->first, preview.Value(ir->first, true),
                              ir->second);
        }
      }
      const auto active = ActivePath(preview);
      if (!active) return std::nullopt;
      const auto writable = [&](size_t id) {
        const auto& node = netlist_.nodes[id];
        return active->count(id) &&
               (!node.reset || preview.Read(*node.reset) == '0');
      };
      const bool ready =
          std::all_of(required.begin(), required.end(), writable);
      auto fixed = configuration;
      std::vector<Operation> next;
      size_t pending = 0;
      if (ready) {
        next = operations;
        // Explicit final writes may close selectors after the target update.
        for (const auto& [id, goal] : goals) {
          fixed.erase(id);
          // Freeze slice/X fill from before prerequisites, retaining the
          // original operations above for their AccessTogether obligations.
          next.push_back({"iWrite",
                          {netlist_.nodes[id].name, "0b" + goal},
                          operations.front().origin,
                          {}});
        }
      } else {
        for (const auto& [id, desired] : configuration) {
          if (instruction_registers_.count(id)) continue;
          const auto& mask = control_masks_.at(id);
          const auto actual = trial.state_.Value(id);
          auto load = trial.fill_.at(id);
          bool satisfied = true;
          for (size_t bit = 0; bit < mask.size(); ++bit) {
            if (mask[bit] == '1') {
              satisfied = satisfied && actual[bit] == desired[bit];
              load[bit] = desired[bit];
            }
          }
          if (preserved.count(id)) {
            const auto& mask = preserved.at(id);
            for (size_t bit = 0; bit < mask.size(); ++bit) {
              if (mask[bit] != 'x') load[bit] = desired[bit];
            }
          }
          if (satisfied) continue;
          ++pending;
          if (writable(id)) {
            const auto& node = netlist_.nodes[id];
            next.push_back(
                {"iWrite", {node.name, "0b" + load}, node.origin, {}});
          }
        }
        if (next.empty() || pending >= previous_pending) return std::nullopt;
        previous_pending = pending;
      }
      try {
        auto scans = trial.Solve(next, ready && together, tap_ ? 2 : 1, fixed,
                                 false, held, explicit_writes, events);
        for (auto& [id, value] : held) {
          value = Masked(trial.state_.Value(id), value);
        }
        steps.insert(steps.end(), std::make_move_iterator(scans.begin()),
                     std::make_move_iterator(scans.end()));
      } catch (const ScheduleLimit&) {
        return std::nullopt;
      }
      if (ready) {
        state_.Restore(trial.state_);
        fill_ = std::move(trial.fill_);
        execution_ = std::move(trial.execution_);
        return steps;
      }
    }
    return std::nullopt;
  }
  using Events = std::map<std::string, std::map<size_t, z3::expr>>;
  std::map<size_t, std::string> ClientPins(
      const std::string& phase, bool before = false,
      const std::map<Key, char>& selections = {}) const {
    std::map<size_t, std::string> pins;
    for (const auto& [id, role] : client_pins_) {
      bool active = role == "select" || role == phase ||
                    (role == "clock" && phase != "update" && phase != "idle");
      if (before && role == "clock") active = !active;
      auto bits = state_.Value(id);
      for (size_t offset : client_pin_bits_.at(id)) {
        const auto& node = netlist_.nodes.at(id);
        const int64_t index = node.left + (node.left <= node.right ? 1 : -1) *
                                              static_cast<int64_t>(offset);
        const auto selected = selections.find({id, index});
        bits[offset] =
            selected == selections.end()
                ? (active ? '1' : '0')
                : (role == "shift" && phase != "shift" ? '0'
                                                       : selected->second);
      }
      pins.emplace(id, std::move(bits));
    }
    return pins;
  }
  Values EventPins(const Values& state, const Frame& frame,
                   const std::string& phase) {
    auto pins = state;
    if (!tap_) {
      for (const auto& [id, bits] : ClientPins(phase)) {
        auto value = SymbolicValue::Literal(context_, bits);
        if (frame.parallel) {
          const auto old = state.find(id);
          value = Choose(!*frame.parallel, value,
                         old == state.end() ? SymbolicValue::Literal(
                                                  context_, state_.Value(id))
                                            : old->second);
        }
        pins.insert_or_assign(id, std::move(value));
      }
      ClientSelections(&pins, frame, phase);
      return pins;
    }
    const auto ir = Known(*frame.instruction, true);
    for (const auto& [name, tap] : netlist_.taps) {
      for (const auto& [role, id] : tap.controls) {
        pins.insert_or_assign(
            id, BooleanValue(!ir && context_.bool_val(role == phase)));
      }
    }
    for (size_t id : tap_clocks_) {
      pins.insert_or_assign(
          id, SymbolicValue::Literal(context_, phase == "update" ? "0" : "1"));
    }
    return pins;
  }
  Events ControlEvents(z3::optimize& solver, const Values& state,
                       const Frame& frame, bool shadows) {
    Events result;
    const auto scan =
        frame.parallel ? !*frame.parallel : context_.bool_val(true);
    const auto ir =
        tap_ ? Known(*frame.instruction, true) : context_.bool_val(false);
    const auto& bindings =
        tap_ ? execution_->RegisterInterfaces() : register_ports_;
    for (const std::string phase : {"capture", "shift", "update", "idle"}) {
      auto pins = EventPins(state, frame, phase);
      SymbolicNetwork after(netlist_, context_, pins, global_resets_);
      if (tap_) {
        for (size_t id : tap_clocks_) {
          pins.insert_or_assign(
              id,
              SymbolicValue::Literal(context_, phase == "update" ? "1" : "0"));
        }
      } else {
        for (const auto& [id, bits] : ClientPins(phase, true)) {
          pins.insert_or_assign(id, SymbolicValue::Literal(context_, bits));
        }
      }
      if (!tap_) ClientSelections(&pins, frame, phase);
      SymbolicNetwork before(netlist_, context_, pins, global_resets_);
      for (size_t id : registers_) {
        if (instruction_registers_.count(id)) continue;
        const auto active = frame.active.count(id) ? frame.active.at(id)
                                                   : context_.bool_val(false);
        const auto entry = bindings.find(id);
        std::vector<TapExecution::InterfacePorts> defaults;
        if (entry == bindings.end()) {
          defaults.emplace_back();
        }
        const auto& interfaces =
            entry == bindings.end() ? defaults : entry->second;
        for (const auto& [kind, role] :
             std::map<std::string, std::string>{{"CaptureEnPort", "capture"},
                                                {"ShiftEnPort", "shift"},
                                                {"UpdateEnPort", "update"}}) {
          auto effective = SymbolicValue::Literal(context_, "0");
          for (const auto& ports : interfaces) {
            const auto control = [&](const std::string& type,
                                     const z3::expr& fallback) {
              const auto found = ports.find(type);
              if (found == ports.end()) return BooleanValue(fallback);
              auto enabled = SymbolicValue::Literal(context_, "1");
              for (const auto& [port, index] : found->second) {
                const auto value =
                    !netlist_.nodes[port].inputs.count("Source") &&
                            !state_.IsDriven(port) && !client_pins_.count(port)
                        ? BooleanValue(fallback)
                        : after.Read({port, index});
                enabled = And(enabled, value);
              }
              return enabled;
            };
            auto enabled = control("SelectPort", active);
            const auto clocks = ports.find("TCKPort");
            if (clocks != ports.end()) {
              for (const auto& [port, index] : clocks->second) {
                const auto prior = before.Read({port, index});
                const auto next = after.Read({port, index});
                const bool falling =
                    phase == "update" || (!tap_ && phase == "idle");
                const auto edge =
                    Known(prior, falling) && Known(next, !falling);
                const auto reverse =
                    Known(prior, !falling) && Known(next, falling);
                solver.add(z3::implies(scan, !reverse));
                const auto absent =
                    Known(prior, !falling) || Known(next, falling);
                const SymbolicValue clocked{BooleanValue(edge).bits,
                                            BooleanValue(edge || absent).bits};
                enabled = And(enabled, clocked);
              }
            }
            effective =
                Or(effective,
                   And(enabled,
                       control(kind, !ir && context_.bool_val(role == phase))));
          }
          solver.add(
              z3::implies(scan, effective.known == context_.bv_val(1, 1)));
          const auto enabled = scan && Known(effective, true);
          if (!shadows || phase != role) {
            solver.add(z3::implies(
                scan, enabled ==
                          (active && !ir && context_.bool_val(role == phase))));
          }
          if (phase == role) {
            result[role].emplace(id, enabled);
            if (shadows && role != "capture") {
              solver.add(z3::implies(active && !ir, enabled));
            }
          }
        }
      }
    }
    if (shadows && tap_) {
      for (const std::string role : {"capture", "shift", "update"}) {
        for (const auto& [id, tap] : instruction_registers_) {
          result[role].emplace(id, scan && ir);
        }
      }
    }
    return result;
  }
  Events ConstrainScanControls(z3::optimize& solver, const Values& state,
                               const Frame& frame, size_t time, bool events) {
    Events result;
    if (tap_ || client_controls_) {
      result = ControlEvents(solver, state, frame, events);
    }
    if (!client_controls_) return result;
    for (const std::string phase : {"shift", "update"}) {
      const auto pins = EventPins(state, frame, phase);
      SymbolicNetwork during(netlist_, context_, pins, global_resets_);
      Frame selected;
      selected.parallel = frame.parallel;
      selected.clients = frame.clients;
      SelectScan(solver, during, time, selected);
      for (const auto& [id, active] : frame.active) {
        solver.add(active == selected.active.at(id));
      }
    }
    return result;
  }
  void ConstrainEndStateFrames(z3::optimize& solver, const Frame& frame,
                               const Values& state, SymbolicNetwork& before,
                               const z3::expr& event) {
    auto emitted = event;
    for (const auto& comparison : frame.comparisons) {
      emitted = emitted || comparison.selected;
    }
    SymbolicNetwork serialized(netlist_, context_, state, global_resets_);
    for (const auto& [id, defaults] : end_states_) {
      const auto& node = netlist_.nodes[id];
      if (node.kind != "ScanRegister") continue;
      const auto value = serialized.Value(id);
      const auto fallback = SymbolicValue::Literal(context_, defaults);
      const auto overridden =
          ((value.bits ^ fallback.bits) & value.known & fallback.known) !=
          context_.bv_val(0, node.Width());
      auto represented = frame.active.count(id) ? frame.active.at(id)
                                                : context_.bool_val(false);
      represented = represented && Matches(value, frame.loads.at(id));
      if (node.reset) {
        represented = represented && Known(before.Read(*node.reset), false) &&
                      Known(serialized.Read(*node.reset), false);
      }
      // Each emitted apply must explicitly retain a temporary override.
      solver.add(z3::implies(emitted && overridden, represented));
    }
  }
  bool NeedsScanEvents() {
    Values state;
    for (size_t id = 0; id < netlist_.nodes.size(); ++id) {
      const auto& node = netlist_.nodes[id];
      if (node.model == ModelKind::kRegister || PrimaryInput(node)) {
        state.emplace(
            id, SymbolicValue{
                    context_.bv_const(("event_probe_" + node.name).c_str(),
                                      node.Width()),
                    ~context_.bv_val(0, node.Width())});
      } else if (state_.IsDriven(id)) {
        state.emplace(id, SymbolicValue::Literal(context_, state_.Value(id)));
      }
    }
    const auto ir = context_.bv_const("event_probe_ir", 1);
    z3::optimize solver(context_);
    z3::params parameters(context_);
    parameters.set("timeout", 250U);
    solver.set(parameters);
    Frame frame;
    SelectClients(frame, 0);
    if (tap_) {
      for (const auto& [id, tap] : instruction_registers_) {
        state.insert_or_assign(tap->ir_select,
                               SymbolicValue{ir, context_.bv_val(1, 1)});
      }
      frame.instruction = SymbolicValue{ir, context_.bv_val(1, 1)};
    } else {
      state = EventPins(state, frame, "capture");
      SettleResets(&state, nullptr, nullptr, &solver);
    }
    SymbolicNetwork values(netlist_, context_, state, global_resets_);
    SelectScan(solver, values, 0, frame);
    const auto events = ControlEvents(solver, state, frame, true);
    auto differs = context_.bool_val(false);
    for (size_t id : registers_) {
      const auto active = frame.active.count(id) ? frame.active.at(id)
                                                 : context_.bool_val(false);
      for (const std::string role : {"capture", "shift", "update"}) {
        differs = differs || events.at(role).at(id) != active;
      }
    }
    solver.add(differs);
    // An unproved fast-path equivalence must use the concrete event model.
    return solver.check() != z3::unsat;
  }
  size_t MaximumScanLength() const {
    std::vector<size_t> lengths(graph_.size());
    for (size_t id : order_) {
      for (const auto& edge : graph_[id].edges) {
        lengths[id] = std::max(lengths[id], lengths[edge.next]);
      }
      const auto reg = graph_[id].bit.node;
      if (reg != kMissing && netlist_.nodes[reg].kind == "ScanRegister") {
        lengths[id] += netlist_.nodes[reg].Width();
      }
    }
    size_t maximum = 0;
    for (size_t root : roots_) maximum = std::max(maximum, lengths[root]);
    return maximum;
  }
  Values SerialPins(const Values& state, const Frame& frame, size_t time,
                    size_t tick) {
    auto pins = EventPins(state, frame, "shift");
    SymbolicNetwork before(netlist_, context_, pins, global_resets_);
    for (const auto& [key, path] : frame.inputs) {
      const auto& [active, length] = path;
      const auto& node = netlist_.nodes[key.first];
      const auto name = "serial_" + std::to_string(time) + "_" +
                        std::to_string(tick) + "_" + node.name + "_" +
                        std::to_string(key.second);
      const SymbolicValue incoming{context_.bv_const(name.c_str(), 1),
                                   context_.bv_val(1, 1)};
      const auto cycle = context_.int_val(tick);
      const auto value =
          Choose(active && cycle < *frame.clocks,
                 Choose(cycle < *frame.clocks - length,
                        SymbolicValue::Literal(context_, "0"), incoming),
                 before.Read({key.first, key.second}));
      auto found = pins.find(key.first);
      if (found == pins.end()) {
        found = pins.emplace(key.first, before.Value(key.first)).first;
      }
      const auto width = static_cast<unsigned>(node.Width());
      const auto shift =
          context_.bv_val(width - 1 - Offset(node, key.second), width);
      const auto mask = z3::shl(context_.bv_val(1, width), shift);
      found->second = {(found->second.bits & ~mask) |
                           z3::shl(z3::zext(value.bits, width - 1), shift),
                       (found->second.known & ~mask) |
                           z3::shl(z3::zext(value.known, width - 1), shift)};
    }
    return pins;
  }
  Values ScanEvents(z3::optimize& solver, const Values& state,
                    const Frame& frame, Values* shifts, size_t time,
                    size_t maximum, const z3::expr& event,
                    const Events& events) {
    auto pins = EventPins(state, frame, "capture");
    SymbolicNetwork capture(netlist_, context_, pins, global_resets_, shifts);
    for (const auto& [key, path] : frame.inputs) {
      if (!pins.count(key.first)) {
        pins.emplace(key.first, capture.Value(key.first));
      }
    }
    Values captured;
    const auto clocks = *frame.clocks;
    for (size_t id : registers_) {
      const auto& node = netlist_.nodes[id];
      const auto found = node.inputs.find("CaptureSource");
      const auto value =
          instruction_registers_.count(id)
              ? SymbolicValue::Literal(
                    context_, instruction_registers_.at(id)->bsdl.capture)
          : found != node.inputs.end()
              ? capture.Bits(found->second)
              : SymbolicValue::Literal(context_,
                                       std::string(node.Width(), 'x'));
      captured.emplace(
          id, Choose(events.at("capture").at(id), value, shifts->at(id)));
    }
    *shifts = std::move(captured);
    if (feedback_resets_) SettleResets(&pins, nullptr, &event, &solver);
    const auto before_update = pins;
    for (size_t tick = 0; tick < maximum; ++tick) {
      pins = SerialPins(pins, frame, time, tick);
      SymbolicNetwork values(netlist_, context_, pins, global_resets_, shifts);
      Values next;
      for (size_t id : registers_) {
        const auto& node = netlist_.nodes[id];
        const auto incoming =
            values.Read(node.inputs.at("ScanInSource").at(0), true);
        const auto& previous = shifts->at(id);
        const auto shifted =
            node.Width() == 1
                ? incoming
                : SymbolicValue{
                      z3::concat(incoming.bits,
                                 previous.bits.extract(node.Width() - 1, 1)),
                      z3::concat(incoming.known,
                                 previous.known.extract(node.Width() - 1, 1))};
        next.emplace(id,
                     Choose(context_.int_val(static_cast<int>(tick)) < clocks &&
                                events.at("shift").at(id),
                            shifted, previous));
      }
      *shifts = std::move(next);
    }
    Values next = before_update;
    if (!tap_) {
      for (const auto& [key, path] : frame.inputs) {
        next.insert_or_assign(key.first, pins.at(key.first));
      }
    }
    SymbolicNetwork before(netlist_, context_, before_update, global_resets_);
    for (size_t id : registers_) {
      const auto& node = netlist_.nodes[id];
      if (frame.active.count(id)) {
        const auto& load = frame.loads.at(id);
        solver.add(z3::implies(frame.active.at(id),
                               shifts->at(id).known == load.known &&
                                   shifts->at(id).bits == load.bits));
      }
      auto value = Choose(events.at("update").at(id), shifts->at(id),
                          before_update.at(id));
      if (node.reset) {
        value = ResetValue(before.Read(*node.reset),
                           SymbolicValue::Literal(
                               context_, node.constants.at("ResetValue").data),
                           value);
      }
      next.insert_or_assign(id, Choose(event, value, state.at(id)));
      // Shift storage becomes unspecified once UpdateEn has been removed.
      shifts->at(id) = Choose(
          events.at("update").at(id),
          SymbolicValue::Literal(context_, std::string(node.Width(), 'x')),
          shifts->at(id));
    }
    return next;
  }
  size_t EventWorkSize() const {
    size_t bits = netlist_.nodes.size();
    for (const auto& node : netlist_.nodes) {
      if (node.kind == "ScanRegister" ||
          (node.kind == "ScanInPort" && node.scope == 0)) {
        bits += node.Width();
      }
    }
    return bits;
  }
  std::pair<std::vector<size_t>, size_t> MixedPrimaryInputs(
      const std::map<size_t, std::string>& goals,
      const std::map<size_t, std::string>& end_goals,
      std::set<size_t> requested_pins) {
    std::vector<size_t> pins;
    size_t pin_bits = 0;
    for (const auto& [id, goal] : goals) {
      if (PrimaryInput(netlist_.nodes[id])) requested_pins.insert(id);
    }
    for (const auto& [id, bits] : end_goals) {
      if (PrimaryInput(netlist_.nodes[id]) && NeedsEndState({{id, bits}})) {
        requested_pins.insert(id);
      }
    }
    for (const auto& [name, id] : netlist_.node_ids) {
      if (!PrimaryInput(netlist_.nodes[id])) continue;
      if (tap_pins_.count(id)) {
        continue;
      }
      const auto control =
          control_seen_.lower_bound({id, std::numeric_limits<int64_t>::min()});
      if (!requested_pins.count(id) &&
          (control == control_seen_.end() || control->first != id)) {
        continue;
      }
      pins.push_back(id);
      pin_bits += netlist_.nodes[id].Width();
    }
    if (pins.size() > 256 || pin_bits > 4096) {
      throw std::runtime_error(
          "mixed-I/O primary-input resource limit exceeded");
    }
    return {std::move(pins), pin_bits};
  }

  std::map<size_t, std::set<size_t>> PortDrivers(
      const std::set<size_t>& ports) {
    std::map<size_t, std::set<size_t>> result;
    size_t remaining = 4 * 1024 * 1024;
    const auto& data = state_.DataRegisters();
    for (size_t id : ports) {
      const auto& port = netlist_.nodes[id];
      for (size_t offset = 0; offset < port.Width(); ++offset) {
        AddControl({id, port.left + (port.left <= port.right
                                         ? static_cast<int64_t>(offset)
                                         : -static_cast<int64_t>(offset))});
      }
      auto& drivers = result[id];
      std::vector<size_t> pending{id};
      std::set<size_t> visited;
      while (!pending.empty()) {
        const size_t current = pending.back();
        pending.pop_back();
        if (!remaining--) {
          throw std::runtime_error("port driver dependency resource limit");
        }
        if (current == kMissing || !visited.insert(current).second) continue;
        const auto& node = netlist_.nodes[current];
        if (node.model == ModelKind::kRegister || PrimaryInput(node)) {
          drivers.insert(current);
          const auto scans = data.scan_inputs.find(current);
          if (scans != data.scan_inputs.end()) {
            drivers.insert(scans->second.begin(), scans->second.end());
          }
          continue;
        }
        for (const auto& [name, refs] : node.inputs) {
          for (const auto& ref : refs) pending.push_back(ref.node);
        }
        for (const auto& choice : node.choices) {
          for (const auto& ref : choice.source) pending.push_back(ref.node);
        }
        for (const auto& source : node.one_hot_sources) {
          for (const auto& ref : source.bits) pending.push_back(ref.node);
          for (const auto& access : one_hot_.Resolve(source)) {
            AddControl(access.enable);
            pending.push_back(access.enable.node);
            if (access.signals) {
              for (const auto& ref : *access.signals) {
                AddControl(ref);
                pending.push_back(ref.node);
              }
            }
          }
        }
        for (const auto& term : node.logic) {
          for (const auto& ref : term.signals) pending.push_back(ref.node);
        }
      }
    }
    return result;
  }

  void ActivatePorts(const std::map<size_t, std::set<size_t>>& ports,
                     SymbolicNetwork& before, SymbolicNetwork& after,
                     const Values& state, size_t time, Frame* frame,
                     std::map<size_t, z3::expr>* touched) {
    std::map<size_t, std::unique_ptr<SymbolicNetwork>> alternatives;
    std::map<std::tuple<size_t, int64_t, bool>, z3::expr> cache;
    size_t remaining = 262144;
    std::function<z3::expr(BitReference, size_t, bool)> access;
    access = [&](BitReference bit, size_t depth, bool control) -> z3::expr {
      if (!remaining-- || depth > 256) {
        throw std::runtime_error("port activity dependency resource limit");
      }
      if (bit.node == kMissing) {
        return context_.bool_val(!control &&
                                 (bit.constant == '0' || bit.constant == '1'));
      }
      const auto key = std::make_tuple(bit.node, bit.index, control);
      const auto cached = cache.find(key);
      if (cached != cache.end()) return cached->second;
      const auto& node = netlist_.nodes[bit.node];
      const size_t offset = Offset(node, bit.index);
      auto active = context_.bool_val(false);
      if (node.model == ModelKind::kRegister || PrimaryInput(node)) {
        const auto event = frame->active.find(bit.node);
        if (event != frame->active.end()) active = event->second;
        if (node.reset) {
          active = active && Known(before.Read(*node.reset), false) &&
                   Known(after.Read(*node.reset), false);
        }
      } else if (node.inputs.count("Source")) {
        active =
            access(node.inputs.at("Source").at(offset), depth + 1, control);
      } else if (node.kind == "DataMux") {
        auto select = context_.bool_val(false);
        for (const auto& ref : node.inputs.at("SelectedBy")) {
          select = select || access(ref, depth + 1, true);
        }
        for (size_t choice = 0; choice < node.choices.size(); ++choice) {
          active = active ||
                   (DataReadGuardActive(after, {bit.node, choice}) &&
                    (select || access(node.choices[choice].source.at(offset),
                                      depth + 1, control)));
        }
      } else if (node.kind == "OneHotDataGroup") {
        auto select = context_.bool_val(false);
        for (const auto& source : node.one_hot_sources) {
          for (const auto& condition : one_hot_.Resolve(source)) {
            select = select || access(condition.enable, depth + 1, true);
            if (condition.signals) {
              for (const auto& ref : *condition.signals) {
                select = select || access(ref, depth + 1, true);
              }
            }
          }
        }
        for (size_t choice = 0; choice < node.one_hot_sources.size();
             ++choice) {
          active =
              active || (DataReadGuardActive(after, {bit.node, choice}) &&
                         (select || access(node.OneHotSourceBit(choice, offset),
                                           depth + 1, control)));
        }
      } else if (node.model == ModelKind::kLogic) {
        auto& alternative = alternatives[bit.node];
        if (!alternative) {
          const size_t work = state.size() + netlist_.nodes.size();
          if (work > remaining) {
            throw std::runtime_error("port activity dependency resource limit");
          }
          remaining -= work;
          auto varied = state;
          for (const auto& [id, event] : frame->active) {
            const auto& driver = netlist_.nodes[id];
            if (driver.model != ModelKind::kRegister && !PrimaryInput(driver)) {
              continue;
            }
            auto enabled = event;
            if (driver.reset) {
              enabled = enabled && Known(before.Read(*driver.reset), false) &&
                        Known(after.Read(*driver.reset), false);
            }
            const auto name = "port_event_" + std::to_string(time) + "_" +
                              std::to_string(bit.node) + "_" + driver.name;
            const auto bits = context_.bv_const(name.c_str(), driver.Width());
            const auto known =
                context_.bv_const((name + "_known").c_str(), driver.Width());
            varied.at(id) = Choose(enabled, {bits, known}, state.at(id));
          }
          alternative = std::make_unique<SymbolicNetwork>(
              netlist_, context_, std::move(varied), global_resets_);
        }
        // Vary updated drivers' values and knownness: an update can establish
        // a known result even when all known input values give the same result.
        // Shared references and masks use the real signal evaluator.
        const auto actual = after.Value(bit.node);
        const auto possible = alternative->Value(bit.node);
        active = actual.known != possible.known ||
                 ((actual.bits ^ possible.bits) & actual.known &
                  possible.known) != context_.bv_val(0, 1);
        const bool constant =
            std::all_of(node.logic.begin(), node.logic.end(),
                        [](const auto& term) { return term.signals.empty(); });
        active = active || context_.bool_val(constant && !control);
      }
      const auto enable = node.inputs.find("Enable");
      if (enable != node.inputs.end()) {
        active = (active || access(enable->second.at(0), depth + 1, true)) &&
                 Known(after.Read(enable->second.at(0)), true);
      }
      return cache.emplace(key, active).first->second;
    };
    for (const auto& [id, drivers] : ports) {
      const auto& node = netlist_.nodes[id];
      auto all = context_.bool_val(true), any = context_.bool_val(false);
      for (size_t offset = 0; offset < node.Width(); ++offset) {
        const BitReference ref{
            id, node.left + (node.left <= node.right
                                 ? static_cast<int64_t>(offset)
                                 : -static_cast<int64_t>(offset))};
        const auto active = FixedOneHotPadding(ref) ? context_.bool_val(true)
                                                    : access(ref, 0, false);
        all = all && active;
        any = any || active;
      }
      frame->active.emplace(id, all);
      const auto found = touched->find(id);
      if (found != touched->end()) found->second = found->second || any;
    }
  }

  void ConstrainPortControllers(z3::optimize& solver, SymbolicNetwork& values,
                                const std::set<size_t>& ports,
                                const std::set<size_t>& requested,
                                const std::map<size_t, z3::expr>& updated) {
    using Requirements = std::map<size_t, z3::expr>;
    using Key = std::pair<size_t, int64_t>;
    std::map<Key, Requirements> cache;
    std::set<Key> visiting;
    size_t remaining = 262144;
    const auto spend = [&] {
      if (!remaining--) {
        throw std::runtime_error("port controller dependency resource limit");
      }
    };
    std::function<const Requirements&(BitReference, size_t)> trace;
    trace = [&](BitReference bit, size_t depth) -> const Requirements& {
      spend();
      const Key key{bit.node, bit.index};
      const auto cached = cache.find(key);
      if (cached != cache.end()) return cached->second;
      if (depth > 256 || !visiting.insert(key).second) {
        throw std::runtime_error(
            "port controller dependency cycle/depth limit");
      }
      Requirements result;
      const auto merge = [&](BitReference ref, const z3::expr& guard) {
        for (const auto& [id, condition] : trace(ref, depth + 1)) {
          spend();
          const auto selected = (guard && condition).simplify();
          const auto [entry, inserted] = result.emplace(id, selected);
          if (!inserted) entry->second = (entry->second || selected).simplify();
        }
      };
      const auto always = context_.bool_val(true);
      if (bit.node != kMissing) {
        const auto& node = netlist_.nodes[bit.node];
        if (node.kind == "ScanRegister") {
          result.emplace(bit.node, always);
        } else if (node.kind == "DataRegister") {
          const auto& controls = state_.DataRegisters().scan_inputs;
          const auto found = controls.find(bit.node);
          if (found != controls.end()) {
            for (size_t id : found->second) {
              spend();
              result.emplace(id, always);
            }
          }
        } else {
          const auto enable = node.inputs.find("Enable");
          if (enable != node.inputs.end()) merge(enable->second.at(0), always);
          if (node.inputs.count("Source")) {
            merge(node.inputs.at("Source").at(Offset(node, bit.index)), always);
          } else if (node.kind == "DataMux") {
            for (const auto& ref : node.inputs.at("SelectedBy")) {
              merge(ref, always);
            }
            for (size_t choice = 0; choice < node.choices.size(); ++choice) {
              merge(node.choices[choice].source.at(Offset(node, bit.index)),
                    DataReadGuardActive(values, {bit.node, choice}));
            }
          } else if (node.kind == "OneHotDataGroup") {
            for (size_t choice = 0; choice < node.one_hot_sources.size();
                 ++choice) {
              for (const auto& access :
                   one_hot_.Resolve(node.one_hot_sources[choice])) {
                merge(access.enable, always);
                if (access.signals) {
                  for (const auto& ref : *access.signals) merge(ref, always);
                }
              }
              merge(node.OneHotSourceBit(choice, Offset(node, bit.index)),
                    DataReadGuardActive(values, {bit.node, choice}));
            }
          } else if (node.model == ModelKind::kLogic) {
            for (const auto& term : node.logic) {
              for (const auto& ref : term.signals) merge(ref, always);
            }
          }
        }
      }
      visiting.erase(key);
      return cache.emplace(key, std::move(result)).first->second;
    };
    // Cache each bit's guarded controllers, rather than enumerate paths through
    // reconvergent logic. A controller shared by mux branches combines guards.
    for (size_t id : ports) {
      const auto& node = netlist_.nodes[id];
      if (!requested.count(id) || node.kind != "DataInPort") continue;
      for (size_t offset = 0; offset < node.Width(); ++offset) {
        const BitReference bit{
            id, node.left + (node.left <= node.right
                                 ? static_cast<int64_t>(offset)
                                 : -static_cast<int64_t>(offset))};
        for (const auto& [controller, selected] : trace(bit, 0)) {
          solver.add(z3::implies(selected, updated.at(controller)));
        }
      }
    }
  }

  void CheckScanConnectivity(const std::set<size_t>& required) const {
    // Check SI/SO connectivity independently of unknown selector values.
    std::vector<bool> connected(graph_.size());
    std::set<size_t> reachable;
    for (size_t id : order_) {
      connected[id] = graph_[id].input;
      for (const auto& edge : graph_[id].edges) {
        connected[id] = connected[id] || connected[edge.next];
      }
      if (connected[id]) reachable.insert(graph_[id].bit.node);
    }
    for (size_t id : required) {
      if (!reachable.count(id)) {
        throw std::runtime_error(netlist_.nodes[id].origin +
                                 ": disconnected scan input/output for " +
                                 netlist_.nodes[id].name);
      }
    }
  }

  void UpdatePortFill(const NetworkState& replay,
                      const std::map<size_t, std::string>& goals,
                      const std::set<size_t>& ports,
                      std::map<size_t, std::string>* fill) const {
    for (const auto& [id, goal] : goals) {
      if (!ports.count(id)) continue;
      const auto actual = replay.Value(id);
      for (size_t bit = 0; bit < goal.size(); ++bit) {
        if (goal[bit] == 'x') continue;
        if (actual[bit] != goal[bit]) {
          throw std::runtime_error("symbolic/concrete port goal mismatch at " +
                                   netlist_.nodes[id].name);
        }
        fill->at(id)[bit] = goal[bit];
      }
    }
  }

  std::pair<Values, Values> InitialValues(const std::vector<size_t>& storage,
                                          const std::vector<size_t>& pins) {
    Values state, fills;
    for (size_t id : storage) {
      state.emplace(id, SymbolicValue::Literal(context_, state_.Value(id)));
      fills.emplace(id, SymbolicValue::Literal(context_, fill_.at(id)));
    }
    for (size_t id = 0; id < netlist_.nodes.size(); ++id) {
      if (state_.IsDriven(id)) {
        state.emplace(id, SymbolicValue::Literal(context_, state_.Value(id)));
      }
    }
    for (size_t id : pins) {
      state.insert_or_assign(
          id, SymbolicValue::Literal(context_, state_.Value(id)));
      fills.insert_or_assign(id,
                             SymbolicValue::Literal(context_, fill_.at(id)));
    }
    return {std::move(state), std::move(fills)};
  }
  std::vector<ScanStep> Solve(
      const std::vector<Operation>& operations, bool together,
      size_t max_steps = kMaxSteps,
      const std::map<size_t, std::string>& fixed_loads = {},
      bool allow_greedy = true,
      const std::map<size_t, std::string>& preserved = {},
      bool explicit_writes = true, bool events = false) {
    const auto end_goals = EndGoals(operations);
    if (operations.empty() && !NeedsEndState(end_goals)) return {};
    if (roots_.empty() && registers_.empty() && !tap_ &&
        std::none_of(
            operations.begin(), operations.end(), [&](const auto& operation) {
              return !operation.arguments.empty() &&
                     !ResolvePdlTarget(netlist_, operation.arguments[0])
                          .access_together.empty();
            })) {
      return SolvePrimaryWrites(operations, together, preserved,
                                explicit_writes);
    }
    if (tap_ && !tap_ready_) {
      throw std::runtime_error(
          "TAP state is unknown; explicit iReset is required");
    }
    const auto& data_plan = state_.DataRegisters();
    for (size_t id : data_plan.order) {
      const auto& node = netlist_.nodes[id];
      for (const auto& bit : node.inputs.at("WriteDataSource")) AddControl(bit);
      const auto& access = data_plan.writes.at(id);
      AddControl(access.enable);
      if (access.signals) {
        for (const auto& bit : *access.signals) AddControl(bit);
      }
    }
    bool mixed =
        std::any_of(netlist_.nodes.begin(), netlist_.nodes.end(),
                    [&](const auto& node) { return PrimaryInput(node); }) ||
        std::any_of(netlist_.nodes.begin(), netlist_.nodes.end(),
                    [](const auto& node) {
                      return node.scope == 0 && node.kind == "DataOutPort";
                    });
    std::vector<size_t> storage = registers_;
    for (size_t id = 0; id < netlist_.nodes.size(); ++id) {
      if (netlist_.nodes[id].kind == "DataRegister") storage.push_back(id);
    }
    std::map<size_t, std::string> goals, expected;
    std::map<size_t, std::map<int64_t, char>> data_reads;
    std::set<size_t> required, group_pins, port_goals;
    std::unique_ptr<AddressResolver> controls;
    struct AccessGroup {
      bool read = false;
      std::set<size_t> members, requested;
    };
    std::map<std::pair<size_t, bool>, AccessGroup> groups;
    for (const auto& operation : operations) {
      const bool read = operation.command == "iRead";
      if ((!read && operation.command != "iWrite") ||
          operation.arguments.empty() || operation.arguments.size() > 2 ||
          (!read && operation.arguments.size() != 2)) {
        throw std::runtime_error(
            "serial apply requires literal iWrite/iRead target/value "
            "operations");
      }
      const auto target = ResolvePdlTarget(netlist_, operation.arguments[0]);
      for (size_t alias : target.access_together) {
        auto& group = groups[{alias, read}];
        group.read = read;
        if (group.members.empty()) {
          std::set<size_t> pending{alias}, expanded;
          while (!pending.empty()) {
            const size_t current = *pending.begin();
            pending.erase(pending.begin());
            if (!expanded.insert(current).second) continue;
            const auto full =
                ResolvePdlTarget(netlist_, netlist_.nodes[current].name);
            pending.insert(full.access_together.begin(),
                           full.access_together.end());
            for (auto bit : full.bits) {
              if (!read) bit = WriteReference(bit, &controls);
              if (bit.node == kMissing ||
                  (netlist_.nodes.at(bit.node).kind != "ScanRegister" &&
                   !data_plan.writes.count(bit.node) &&
                   !(read ? netlist_.nodes.at(bit.node).kind == "DataOutPort"
                          : DataPort(netlist_.nodes.at(bit.node))))) {
                throw std::runtime_error(
                    "AccessTogether " + netlist_.nodes[alias].name +
                    " requires accessible registers or primary ports");
              }
              group.members.insert(bit.node);
              if (read && (data_plan.writes.count(bit.node) ||
                           netlist_.nodes[bit.node].kind == "DataOutPort")) {
                data_reads[bit.node].emplace(bit.index, 'x');
              } else if (PrimaryInput(netlist_.nodes[bit.node])) {
                group_pins.insert(bit.node);
              } else if (DataPort(netlist_.nodes[bit.node])) {
                port_goals.insert(bit.node);
              } else if (data_plan.writes.count(bit.node)) {
                // DataRegister update qualification is modeled in each frame.
              } else {
                required.insert(bit.node);
              }
            }
          }
        }
        for (const auto& bit : target.bits) {
          if (group.members.count(bit.node)) group.requested.insert(bit.node);
        }
      }
      const auto bits = operation.arguments.size() == 1
                            ? std::string(target.bits.size(), 'x')
                            : PdlDataValue(context_, netlist_, target,
                                           operation.arguments[1]);
      for (size_t i = 0; i < bits.size(); ++i) {
        auto ref = target.bits[i];
        if (!read) ref = WriteReference(ref, &controls);
        if (read && ref.node != kMissing &&
            (netlist_.nodes[ref.node].kind == "DataRegister" ||
             netlist_.nodes[ref.node].kind == "DataOutPort")) {
          const char value = ref.inverted && bits[i] != 'x'
                                 ? (bits[i] == '1' ? '0' : '1')
                                 : bits[i];
          data_reads[ref.node][ref.index] = value;
          continue;
        }
        if (ref.node == kMissing ||
            (netlist_.nodes.at(ref.node).kind != "ScanRegister" &&
             (read || (!data_plan.writes.count(ref.node) &&
                       !DataPort(netlist_.nodes[ref.node]))))) {
          throw std::runtime_error(
              "write target requires a register or data port; reads require "
              "observable storage or outputs");
        }
        if (tap_data_registers_.count(ref.node)) {
          throw std::runtime_error(
              "synthetic TAP data registers are not PDL targets");
        }
        if (netlist_.nodes[ref.node].kind == "ScanRegister") {
          required.insert(ref.node);
        } else if (!read && DataPort(netlist_.nodes[ref.node]) &&
                   !PrimaryInput(netlist_.nodes[ref.node])) {
          port_goals.insert(ref.node);
        }
        auto& values = read ? expected : goals;
        auto entry = values.find(ref.node);
        if (entry == values.end()) {
          entry = values
                      .emplace(ref.node,
                               read ? std::string(
                                          netlist_.nodes[ref.node].Width(), 'x')
                                    : WriteFill(ref.node))
                      .first;
        }
        if (read || bits[i] != 'x') {
          entry->second.at(Offset(netlist_.nodes[ref.node], ref.index)) =
              ref.inverted && bits[i] != 'x' ? (bits[i] == '1' ? '0' : '1')
                                             : bits[i];
        }
      }
    }
    const DataReadRoutes* routes = nullptr;
    std::map<size_t, std::set<size_t>> pending_reads;
    if (!data_reads.empty()) {
      auto& cache = mixed ? mixed_read_routes_ : scan_read_routes_;
      if (!cache) {
        cache = std::make_unique<DataReadRoutes>(
            netlist_, mixed ? DataReadMode::kBoth : DataReadMode::kScan);
      }
      routes = cache.get();
      OneHotReadResolver resolver(netlist_);
      for (const auto& [id, bits] : data_reads) {
        for (const auto& [index, expected] : bits) {
          const auto found = routes->routes.find({id, index});
          if (found == routes->routes.end()) {
            throw std::runtime_error("unobservable data read target " +
                                     netlist_.nodes[id].name);
          }
          for (const auto& route : found->second) {
            const auto stored =
                routes->storage.find({route.output.node, route.output.index});
            if (stored != routes->storage.end()) {
              for (size_t reg : stored->second) pending_reads[reg].insert(id);
            }
            for (const auto& guard : route.guards) {
              const auto& node = netlist_.nodes[guard.node];
              if (node.kind != "OneHotDataGroup" || guard.choice == kMissing) {
                continue;
              }
              for (const auto& access :
                   resolver.Resolve(node.one_hot_sources.at(guard.choice))) {
                AddControl(access.enable);
                if (access.signals) {
                  for (const auto& bit : *access.signals) AddControl(bit);
                }
              }
            }
          }
        }
      }
    }
    const auto port_drivers = PortDrivers(port_goals);
    const auto [pins, pin_bits] =
        MixedPrimaryInputs(goals, end_goals, group_pins);
    mixed = !pins.empty() || !port_drivers.empty();
    if (routes) {
      for (const auto& [id, bits] : data_reads) {
        for (const auto& [index, expected] : bits) {
          const auto& choices = routes->routes.at({id, index});
          mixed = mixed ||
                  std::any_of(choices.begin(), choices.end(),
                              [&](const auto& route) {
                                return netlist_.nodes[route.output.node].kind ==
                                       "DataOutPort";
                              });
        }
      }
    }
    for (const auto& [id, tap] : instruction_registers_) {
      if (!required.count(id)) continue;
      const auto& instruction = goals.count(id) ? goals.at(id) : fill_.at(id);
      if (instruction.find('x') != std::string::npos) {
        throw std::runtime_error(
            "ambiguous instruction after reset; explicitly write its unknown "
            "bits before reading or partially updating the IR");
      }
    }
    if (together && !goals.empty()) {
      auto& group = groups[{kMissing, false}];
      for (const auto& [id, goal] : goals) group.members.insert(id);
    }
    std::set<size_t> explicit_goals;
    for (const auto& [id, goal] : goals) explicit_goals.insert(id);
    for (const auto& [id, bits] : end_goals) {
      if (netlist_.nodes[id].kind != "ScanRegister" &&
          !data_plan.writes.count(id) && !PrimaryInput(netlist_.nodes[id])) {
        throw std::runtime_error(
            "mixed primary/scan iApplyEndState scheduling is not implemented");
      }
      if (NeedsEndState({{id, bits}})) {
        goals.emplace(id, fill_.at(id));
        if (netlist_.nodes[id].kind == "ScanRegister") required.insert(id);
      }
    }
    // IEEE 7.9.14(a): matching stored data does not waive UpdateEn on
    // the scan registers controlling an explicit DataRegister write.
    std::set<size_t> data_controllers;
    for (size_t id : explicit_goals) {
      const auto inputs = data_plan.scan_inputs.find(id);
      if (inputs != data_plan.scan_inputs.end()) {
        data_controllers.insert(inputs->second.begin(), inputs->second.end());
      }
    }
    auto tracked_controllers = data_controllers;
    for (const auto& [id, drivers] : port_drivers) {
      if (!explicit_goals.count(id) ||
          netlist_.nodes[id].kind != "DataInPort") {
        continue;
      }
      for (size_t source : drivers) {
        if (netlist_.nodes[source].kind == "ScanRegister") {
          tracked_controllers.insert(source);
        }
      }
    }
    required.insert(data_controllers.begin(), data_controllers.end());
    std::string access_context;
    std::set<size_t> grouped_reads;
    for (const auto& [key, group] : groups) {
      const auto irs = std::count_if(
          group.members.begin(), group.members.end(),
          [&](size_t id) { return instruction_registers_.count(id); });
      if (irs != 0 && static_cast<size_t>(irs) != group.members.size()) {
        throw std::runtime_error(
            "IR and DR registers cannot share a simultaneous access");
      }
      if (group.read) {
        grouped_reads.insert(group.requested.begin(), group.requested.end());
      }
      if (key.first != kMissing) {
        access_context += " AccessTogether " + netlist_.nodes[key.first].name;
      }
    }
    std::vector<size_t> read_order(grouped_reads.begin(), grouped_reads.end());
    std::sort(read_order.begin(), read_order.end(), [&](size_t a, size_t b) {
      return netlist_.nodes[a].name < netlist_.nodes[b].name;
    });
    CheckScanConnectivity(required);
    const auto desired_preserved =
        DesiredPreserved(preserved, end_goals, explicit_writes, operations);
    if (allow_greedy && !mixed && !events && goals.size() >= 4 && !together &&
        groups.empty() && end_states_.empty() && expected.empty() &&
        data_reads.empty()) {
      auto planned = TryAddressedWrites(goals, preserved, explicit_writes);
      if (planned) return std::move(*planned);
    }
    const size_t maximum = events ? MaximumScanLength() : 0;
    // Keep joint optimization for small networks; wide event histories need
    // replayed prerequisite batches to bound cross-frame expression growth.
    if (allow_greedy && !mixed && data_plan.order.empty() &&
        end_states_.empty() && (maximum > 32 || registers_.size() >= 32) &&
        expected.empty() && data_reads.empty()) {
      auto planned = TryGreedy(operations, goals, required, together, preserved,
                               desired_preserved, explicit_writes, events);
      if (planned) return std::move(*planned);
    }
    const size_t event_bits = events ? EventWorkSize() : 0;
    const auto start = std::chrono::steady_clock::now();
    for (size_t horizon = 1; horizon <= max_steps; ++horizon) {
      if (horizon * reset_work_ > 1048576) {
        throw std::runtime_error(
            "client reset resource limit; fixed point not proved");
      }
      if (events && horizon * (maximum + 1) * event_bits > 1048576) {
        throw std::runtime_error(
            "scan event resource limit; reachability not proved");
      }
      if (std::chrono::steady_clock::now() - start > std::chrono::seconds(15) ||
          horizon * (graph_.size() + storage.size() + pin_bits) > 262144) {
        throw std::runtime_error(
            "scan scheduling resource limit; reachability not proved");
      }
      z3::optimize solver(context_);
      z3::params parameters(context_);
      parameters.set("timeout", 2000U);
      solver.set(parameters);
      auto [state, fills] = InitialValues(storage, pins);
      Values shifts;
      if (events) {
        for (size_t id : registers_) {
          shifts.emplace(
              id, SymbolicValue::Literal(context_, state_.Value(id, true)));
        }
      }
      std::map<size_t, z3::expr> reserved_changed;
      for (const auto& [id, value] : preserved) {
        reserved_changed.emplace(id, context_.bool_val(false));
      }
      std::map<size_t, z3::expr> touched;
      for (const auto& [id, goal] : goals) {
        touched.emplace(id, context_.bool_val(data_plan.writes.count(id) &&
                                              state_.Value(id) == goal));
      }
      std::map<size_t, z3::expr> controller_updated;
      for (size_t id : tracked_controllers) {
        controller_updated.emplace(id, context_.bool_val(false));
      }
      std::map<size_t, z3::expr> read;
      for (const auto& [id, bits] : expected) {
        read.emplace(id, context_.bool_val(false));
      }
      std::map<size_t, z3::expr> data_read_done;
      for (const auto& [id, bits] : data_reads) {
        data_read_done.emplace(id, context_.bool_val(false));
      }
      std::vector<Frame> frames;
      std::vector<z3::expr> simultaneous(groups.size(),
                                         context_.bool_val(false));
      auto cycles = context_.int_val(0), changes = context_.int_val(0);
      for (size_t time = 0; time < horizon; ++time) {
        Frame frame;
        SelectClients(frame, time);
        auto scan = context_.bool_val(true);
        auto any_drive = context_.bool_val(false);
        if (mixed) {
          frame.parallel =
              context_.bool_const(("parallel_" + std::to_string(time)).c_str());
          scan = !*frame.parallel;
          for (size_t id : pins) {
            const auto& node = netlist_.nodes[id];
            const auto suffix = node.name + "_" + std::to_string(time);
            const auto drive = context_.bool_const(("drive_" + suffix).c_str());
            const auto bits =
                context_.bv_const(("pin_" + suffix).c_str(), node.Width());
            solver.add(z3::implies(drive, *frame.parallel));
            frame.drives.emplace(id, drive);
            any_drive = any_drive || drive;
            state.insert_or_assign(
                id, Choose(drive, {bits, ~context_.bv_val(0, node.Width())},
                           state.at(id)));
            fills.at(id) = Choose(drive, state.at(id), fills.at(id));
            changes = changes +
                      z3::ite(drive, context_.int_val(1), context_.int_val(0));
            if (touched.count(id)) touched.at(id) = touched.at(id) || drive;
          }
          cycles = cycles + z3::ite(*frame.parallel, context_.int_val(1),
                                    context_.int_val(0));
        }
        if (tap_) {
          const auto ir =
              context_.bv_const(("ir_" + std::to_string(time)).c_str(), 1);
          const auto instruction = Choose(scan, {ir, context_.bv_val(1, 1)},
                                          state.at(tap_->ir_select));
          for (const auto& [id, tap] : instruction_registers_) {
            state.insert_or_assign(tap->ir_select, instruction);
          }
          frame.instruction = instruction;
          cycles = cycles +
                   z3::ite(scan,
                           z3::ite(ir == context_.bv_val(1, 1),
                                   context_.int_val(6), context_.int_val(5)),
                           context_.int_val(0));
        }
        if (client_controls_) state = EventPins(state, frame, "capture");
        const auto event = scan || any_drive;
        const auto before_capture =
            (tap_ || client_controls_) ? event : any_drive;
        SettleResets(&state, &fills, &before_capture, &solver);
        SymbolicNetwork values(netlist_, context_, state, global_resets_);
        SelectScan(solver, values, time, frame);
        const auto scan_events =
            ConstrainScanControls(solver, state, frame, time, events);
        auto data_event = scan || any_drive;
        if (tap_ || client_controls_) {
          data_event = any_drive;
          const auto updates = scan_events.find("update");
          if (updates != scan_events.end()) {
            for (const auto& [id, enabled] : updates->second) {
              if (!instruction_registers_.count(id)) {
                data_event = data_event || enabled;
              }
            }
          }
        }
        for (const auto& [id, drive] : frame.drives) {
          const auto held = scan && context_.bool_val(goals.count(id) != 0);
          frame.active.emplace(id, drive);
          if (touched.count(id)) touched.at(id) = touched.at(id) || held;
        }
        auto before_update = state;
        if (feedback_resets_) {
          SettleResets(&before_update, &fills, &scan, &solver);
        }
        SymbolicNetwork update_values(netlist_, context_, before_update,
                                      global_resets_);
        Values next = before_update;
        for (size_t id : registers_) {
          const auto& node = netlist_.nodes[id];
          const auto active = frame.active.count(id) ? frame.active.at(id)
                                                     : context_.bool_val(false);
          if (read.count(id)) {
            // Incidental prerequisite captures do not fulfill a grouped read.
            const auto ready =
                grouped_reads.count(id)
                    ? context_.bool_const(
                          ("read_" + std::to_string(time) + "_" + node.name)
                              .c_str())
                    : context_.bool_val(true);
            frame.read.emplace(id, active && ready && !read.at(id));
            read.at(id) = read.at(id) || frame.read.at(id);
          }
          const auto loaded = context_.bv_const(
              ("load_" + std::to_string(time) + "_" + node.name).c_str(),
              node.Width());
          const auto allowed = SymbolicValue::Literal(
              context_, control_masks_.count(id)
                            ? control_masks_.at(id)
                            : std::string(node.Width(), '0'));
          auto requested = goals.count(id)
                               ? SymbolicValue::Literal(context_, goals.at(id))
                               : fills.at(id);
          if (node.reset && goals.count(id)) {
            const auto writable = Known(update_values.Read(*node.reset), false);
            requested = Choose(writable, requested, fills.at(id));
            solver.add(z3::implies(!writable, loaded == fills.at(id).bits));
          }
          solver.add((loaded & ~allowed.bits) ==
                     (requested.bits & ~allowed.bits));
          const auto fixed = fixed_loads.find(id);
          if (fixed != fixed_loads.end()) {
            solver.add((loaded & allowed.bits) ==
                       (SymbolicValue::Literal(context_, fixed->second).bits &
                        allowed.bits));
          }
          const SymbolicValue load{loaded, ~context_.bv_val(0, node.Width())};
          frame.loads.emplace(id, load);
          if (node.reset) {
            frame.needs_write.emplace(id, loaded != fills.at(id).bits);
          }
          auto value = Choose(active, load, before_update.at(id));
          if (node.reset) {
            value =
                ResetValue(update_values.Read(*node.reset),
                           SymbolicValue::Literal(
                               context_, node.constants.at("ResetValue").data),
                           value);
          }
          next.insert_or_assign(id, Choose(event, value, state.at(id)));
          const auto desired = SymbolicValue::Literal(context_, fill_.at(id));
          changes = changes + z3::ite(active && loaded != desired.bits,
                                      context_.int_val(1), context_.int_val(0));
          if (controller_updated.count(id)) {
            controller_updated.at(id) = controller_updated.at(id) || active;
          }
          if (touched.count(id)) {
            touched.at(id) =
                touched.at(id) ||
                (active &&
                 (!node.reset ? context_.bool_val(true)
                              : Known(update_values.Read(*node.reset), false)));
          }
        }
        cycles = cycles + *frame.clocks;
        if (events) {
          next = ScanEvents(solver, state, frame, &shifts, time, maximum, event,
                            scan_events);
        }
        if (client_controls_) next = EventPins(next, frame, "update");
        SymbolicNetwork scan_after(netlist_, context_, next, global_resets_);
        for (size_t id : registers_) {
          const auto active = frame.active.count(id) ? frame.active.at(id)
                                                     : context_.bool_val(false);
          auto fill = Choose(active, frame.loads.at(id), fills.at(id));
          const auto& node = netlist_.nodes[id];
          if (node.reset) {
            auto reset_fill = initial_fill_.at(id);
            const auto& reset_bits = node.constants.at("ResetValue").data;
            for (size_t i = 0; i < reset_fill.size(); ++i) {
              if (reset_bits[i] != 'x') reset_fill[i] = reset_bits[i];
            }
            fill =
                Choose(event && (Known(update_values.Read(*node.reset), true) ||
                                 Known(scan_after.Read(*node.reset), true)),
                       SymbolicValue::Literal(context_, reset_fill), fill);
          }
          fills.at(id) = fill;
        }
        // Native scan updates precede simultaneous reset propagation.
        SettleResets(&next, &fills, &event, &solver);
        // Route comparisons through the value present at Capture, before
        // this scan's new write-data/address/enable bits reach Update.
        std::map<std::pair<Key, char>, z3::expr> physical_reads;
        for (const auto& [id, bits] : expected) {
          const auto& node = netlist_.nodes[id];
          for (size_t bit = 0; bit < bits.size(); ++bit) {
            if (bits[bit] == 'x') continue;
            const int64_t index =
                node.left +
                (node.left <= node.right ? 1 : -1) * static_cast<int64_t>(bit);
            physical_reads.emplace(std::make_pair(Key{id, index}, bits[bit]),
                                   frame.read.at(id));
          }
        }
        for (const auto& [id, bits] : data_reads) {
          const auto observe = context_.bool_const(
              ("capture_" + std::to_string(id) + "_" + std::to_string(time))
                  .c_str());
          solver.add(z3::implies(observe, !data_read_done.at(id)));
          for (const auto& [index, expected] : bits) {
            auto count = context_.int_val(0);
            const auto& choices = routes->routes.at({id, index});
            for (size_t route = 0; route < choices.size(); ++route) {
              const auto& path = choices[route];
              const auto selected = context_.bool_const(
                  ("capture_route_" + std::to_string(id) + "_" +
                   std::to_string(index) + "_" + std::to_string(time) + "_" +
                   std::to_string(route))
                      .c_str());
              const bool primary =
                  netlist_.nodes[path.output.node].kind == "DataOutPort";
              auto valid = primary && mixed
                               ? (*frame.parallel && !any_drive)
                               : (frame.active.count(path.output.node)
                                      ? frame.active.at(path.output.node)
                                      : context_.bool_val(false));
              for (const auto& guard : path.guards) {
                valid = valid && DataReadGuardActive(values, guard);
              }
              const auto& output = netlist_.nodes[path.output.node];
              if (output.reset) {
                valid = valid && Known(values.Read(*output.reset), false);
              }
              solver.add(z3::implies(selected, valid));
              count = count + z3::ite(selected, context_.int_val(1),
                                      context_.int_val(0));
              const char compare = path.output.inverted && expected != 'x'
                                       ? (expected == '1' ? '0' : '1')
                                       : expected;
              frame.comparisons.push_back({path.output, compare, selected});
              if (compare != 'x') {
                const auto key = std::make_pair(
                    Key{path.output.node, path.output.index}, compare);
                const auto found = physical_reads.find(key);
                if (found == physical_reads.end()) {
                  physical_reads.emplace(key, selected);
                } else {
                  found->second = found->second || selected;
                }
              }
            }
            solver.add(count == z3::ite(observe, context_.int_val(1),
                                        context_.int_val(0)));
          }
          frame.read.emplace(id, observe);
          data_read_done.at(id) = data_read_done.at(id) || observe;
        }
        for (const auto& [key, selected] : physical_reads) {
          const auto opposite =
              physical_reads.find({key.first, key.second == '1' ? '0' : '1'});
          if (opposite != physical_reads.end()) {
            solver.add(!(selected && opposite->second));
          }
        }
        // A primary output can expose scan storage directly. Keep its old
        // held value until the selected observation, just as for data storage.
        for (const auto& [id, readers] : pending_reads) {
          if (netlist_.nodes[id].kind != "ScanRegister") continue;
          for (size_t reader : readers) {
            solver.add(
                z3::implies(!data_read_done.at(reader),
                            next.at(id).bits == state.at(id).bits &&
                                next.at(id).known == state.at(id).known));
          }
        }
        // Data storage settles after Update, in dependency order. Capture
        // uses the pre-update state and cannot be satisfied by these writes.
        for (size_t id : data_plan.order) {
          const auto& node = netlist_.nodes[id];
          SymbolicNetwork data(netlist_, context_, next, global_resets_);
          const auto active =
              Choose(data_event, data.Active(data_plan.writes.at(id)),
                     SymbolicValue::Literal(context_, "0"));
          const auto load = data.Bits(node.inputs.at("WriteDataSource"));
          auto enabled = Known(active, true);
          auto definite = enabled || Known(active, false);
          auto updated = ResetValue(active, load, next.at(id));
          auto fill = Choose(enabled, load, fills.at(id));
          if (node.reset) {
            const auto reset = data.Read(*node.reset);
            updated =
                ResetValue(reset,
                           SymbolicValue::Literal(
                               context_, node.constants.at("ResetValue").data),
                           updated);
            enabled = enabled && Known(reset, false);
            definite = definite || Known(reset, true);
            fill =
                Choose(Known(reset, true),
                       SymbolicValue::Literal(context_, initial_fill_.at(id)),
                       Choose(enabled, load, fills.at(id)));
          }
          updated = Choose(event, updated, next.at(id));
          fill = Choose(event, fill, fills.at(id));
          solver.add(definite);
          solver.add(z3::implies(
              enabled, load.known == ~context_.bv_val(0, node.Width())));
          const auto source_scans = data_plan.scan_inputs.find(id);
          const bool explicit_scan =
              source_scans != data_plan.scan_inputs.end() &&
              std::any_of(
                  source_scans->second.begin(), source_scans->second.end(),
                  [&](size_t scan) { return explicit_goals.count(scan); });
          const auto primary_inputs = data_plan.primary_inputs.find(id);
          const bool explicit_pin =
              primary_inputs != data_plan.primary_inputs.end() &&
              std::any_of(
                  primary_inputs->second.begin(), primary_inputs->second.end(),
                  [&](size_t pin) { return explicit_goals.count(pin); });
          if (!goals.count(id) && !explicit_scan && !explicit_pin) {
            const auto care = SymbolicValue::Literal(
                context_, data_control_masks_.count(id)
                              ? data_control_masks_.at(id)
                              : std::string(node.Width(), '0'));
            solver.add(z3::implies(
                enabled,
                (load.bits & ~care.bits) == (fills.at(id).bits & ~care.bits)));
          }

          const auto waiting = pending_reads.find(id);
          if (waiting != pending_reads.end()) {
            for (size_t read : waiting->second) {
              solver.add(z3::implies(!data_read_done.at(read),
                                     updated.bits == state.at(id).bits &&
                                         updated.known == state.at(id).known));
            }
          }
          next.insert_or_assign(id, updated);
          fills.at(id) = fill;
          frame.active.insert_or_assign(id, enabled);
          if (touched.count(id)) touched.at(id) = touched.at(id) || enabled;
        }
        if (data_reset_dependencies_) {
          SettleResets(&next, &fills, &event, &solver);
        }
        for (const auto& [id, previous] : preserved) {
          if (!frame.loads.count(id)) continue;
          const auto old = SymbolicValue::Literal(context_, previous);
          const auto wanted =
              SymbolicValue::Literal(context_, desired_preserved.at(id));
          const auto active = frame.active.count(id) ? frame.active.at(id)
                                                     : context_.bool_val(false);
          const auto& load = frame.loads.at(id);
          solver.add(z3::implies(
              active, (!reserved_changed.at(id) && Matches(load, old)) ||
                          Matches(load, wanted)));
        }
        SymbolicNetwork after(netlist_, context_, next, global_resets_);
        ConstrainResetLoads(solver, frame, update_values, after, events);
        ActivatePorts(port_drivers, values, after, next, time, &frame,
                      &touched);
        ConstrainPreserved(solver, after, preserved, desired_preserved,
                           reserved_changed);
        size_t group_index = 0;
        for (const auto& [key, group] : groups) {
          auto complete = context_.bool_val(true);
          for (size_t id : group.members) {
            const auto active = frame.active.count(id)
                                    ? frame.active.at(id)
                                    : context_.bool_val(false);
            complete = complete &&
                       (group.read && data_reads.count(id) ? frame.read.at(id)
                                                           : active);
            if (group.read) {
              if (group.requested.count(id)) {
                complete = complete && frame.read.at(id);
              }
            } else {
              if (goals.count(id)) {
                const auto requested =
                    SymbolicValue::Literal(context_, goals.at(id));
                complete = complete && Matches(after.Value(id), requested);
                if (netlist_.nodes[id].kind == "ScanRegister") {
                  complete = complete && Matches(frame.loads.at(id), requested);
                }
              }
              const auto& node = netlist_.nodes[id];
              if (node.reset) {
                complete = complete &&
                           Known(update_values.Read(*node.reset), false) &&
                           Known(after.Read(*node.reset), false);
              }
            }
          }
          simultaneous[group_index] = simultaneous[group_index] || complete;
          ++group_index;
        }
        // Each emitted TAP scan finishes in Run-Test/Idle, where IR select is
        // inactive. A following parallel frame must see that idle snapshot.
        for (const auto& [id, tap] : instruction_registers_) {
          next.insert_or_assign(tap->ir_select,
                                SymbolicValue::Literal(context_, "0"));
        }
        if (client_controls_) next = EventPins(next, frame, "idle");
        ConstrainEndStateFrames(solver, frame, next, update_values, event);
        frame.fill = fills;
        frame.state = next;
        frames.push_back(std::move(frame));
        state = std::move(next);
      }
      SymbolicNetwork final(netlist_, context_, state, global_resets_);
      for (const auto& [id, goal] : goals) {
        const auto expected = SymbolicValue::Literal(context_, goal);
        solver.add(touched.at(id) && Matches(final.Value(id), expected));
      }
      for (const auto& [id, bits] : end_goals) {
        const auto wanted = SymbolicValue::Literal(context_, bits);
        solver.add((state.at(id).known & wanted.known) == wanted.known &&
                   (state.at(id).bits & wanted.known) == wanted.bits);
      }
      for (size_t id : data_controllers) solver.add(controller_updated.at(id));
      ConstrainPortControllers(solver, final, port_goals, explicit_goals,
                               controller_updated);
      for (const auto& [id, observed] : read) solver.add(observed);
      for (const auto& [id, observed] : data_read_done) solver.add(observed);
      for (const auto& complete : simultaneous) solver.add(complete);
      solver.minimize(cycles);
      solver.minimize(changes);
      // Deterministic tie-breaking, independent of source/flat node numbering.
      for (const auto& frame : frames) {
        for (const auto& [name, id] : netlist_.node_ids) {
          const auto value = frame.loads.find(id);
          if (value != frame.loads.end()) {
            solver.minimize(z3::bv2int(value->second.bits, false));
          }
          if (frame.drives.count(id)) {
            solver.minimize(z3::bv2int(
                frame.state.at(id).bits ^
                    SymbolicValue::Literal(context_, fill_.at(id)).bits,
                false));
          }
        }
      }
      // Prefer the earliest legal grouped comparisons after scan/load costs.
      for (const auto& frame : frames) {
        for (size_t id : read_order) {
          solver.maximize(z3::ite(frame.read.at(id), context_.int_val(1),
                                  context_.int_val(0)));
        }
      }
      const auto result = solver.check();
      if (result == z3::unknown) {
        throw std::runtime_error(
            "scan solver resource limit; reachability not proved");
      }
      if (result == z3::unsat) continue;
      const auto model = solver.get_model();
      // Replay into a copy: a failed model must not partially change state.
      NetworkState replay(state_);
      auto tap_replay =
          execution_ ? std::make_unique<TapExecution>(*execution_) : nullptr;
      auto fill = fill_;
      std::vector<ScanStep> steps;
      auto replayed_preserved = preserved;
      std::vector<bool> simultaneous_replayed(groups.size());
      for (const auto& frame : frames) {
        replay.TakeControlEvents();
        replay.TakeResetEvents();
        if (tap_replay) {
          tap_replay->TakeControlEvents();
          tap_replay->TakeResetEvents();
        }
        std::set<size_t> reset;
        for (size_t id : registers_) {
          if (netlist_.nodes[id].reset &&
              replay.Read(*netlist_.nodes[id].reset) == '1') {
            reset.insert(id);
          }
        }
        const bool parallel =
            frame.parallel && model.eval(*frame.parallel, true).is_true();
        const bool event =
            !parallel ||
            std::any_of(frame.drives.begin(), frame.drives.end(),
                        [&](const auto& drive) {
                          return model.eval(drive.second, true).is_true();
                        });
        auto step =
            parallel ? ReplayParallel(frame, model, &replay, tap_replay.get(),
                                      replayed_preserved, desired_preserved)
                     : Replay(frame, model, expected, required, events, &replay,
                              tap_replay.get(), replayed_preserved,
                              desired_preserved);
        for (size_t id : replay.TakeResetEvents()) reset.insert(id);
        const auto control_events = replay.TakeControlEvents();
        for (const auto& [id, drive] : frame.drives) {
          if (model.eval(drive, true).is_true()) fill[id] = replay.Value(id);
          if (replay.Value(id) != Decode(model, frame.state.at(id))) {
            throw std::runtime_error(
                "symbolic/concrete mixed-input replay mismatch");
          }
        }
        size_t group_index = 0;
        for (const auto& [key, group] : groups) {
          bool complete = true;
          for (size_t id : group.members) {
            const auto& name = netlist_.nodes[id].name;
            if (group.read) {
              if (data_reads.count(id)) {
                complete =
                    complete && model.eval(frame.read.at(id), true).is_true();
              } else {
                complete =
                    complete && step.loads.count(name) &&
                    (!group.requested.count(id) || step.expected.count(name));
              }
            } else {
              if (data_plan.writes.count(id) || port_drivers.count(id)) {
                complete =
                    complete &&
                    model.eval(frame.active.at(id), true).is_true() &&
                    (!goals.count(id) || replay.Value(id) == goals.at(id));
              } else {
                const auto found = step.writes.find(name);
                complete = complete && found != step.writes.end() &&
                           (!goals.count(id) || found->second == goals.at(id));
              }
            }
          }
          if (complete && !simultaneous_replayed[group_index]) {
            // Preserve the verified update grouping after alias names are
            // inlined.
            step.together = step.together || !group.read;
            simultaneous_replayed[group_index] = true;
          }
          ++group_index;
        }
        for (const auto& [name, bits] : step.loads) {
          fill[netlist_.node_ids.at(name)] = bits;
        }
        UpdateStorageFill(frame, model, replay, reset, control_events, step,
                          event, fill);
        AppendDataEndOverrides(replay, &step);
        FilterTapData(&step, parallel);
        if (!step.writes.empty() || !step.expected.empty()) {
          steps.push_back(std::move(step));
        }
      }
      if (std::find(simultaneous_replayed.begin(), simultaneous_replayed.end(),
                    false) != simultaneous_replayed.end()) {
        throw std::runtime_error(
            "concrete replay has no simultaneous access cycle" +
            access_context);
      }
      UpdatePortFill(replay, goals, port_goals, &fill);
      state_.Restore(replay);
      execution_ = std::move(tap_replay);
      fill_ = std::move(fill);
      return steps;
    }
    throw ScheduleLimit("no schedule within " + std::to_string(max_steps) +
                        " scans; reachability not proved (unknown "
                        "control, reset, or blocked path)" +
                        access_context);
  }
  void ConstrainResetLoads(z3::optimize& solver, const Frame& frame,
                           SymbolicNetwork& before, SymbolicNetwork& after,
                           bool events) {
    for (const auto& [id, needed] : frame.needs_write) {
      const auto active = frame.active.find(id);
      if (active == frame.active.end()) continue;
      const auto reset = *netlist_.nodes[id].reset;
      const auto old_reset = before.Read(reset);
      const auto writable = Known(old_reset, false) &&
                            Known(after.Read(reset), false) &&
                            Matches(after.Value(id), frame.loads.at(id));
      const auto ignored = !events && Known(old_reset, true);
      solver.add(z3::implies(active->second && needed, writable || ignored));
    }
  }
  void UpdateStorageFill(const Frame& frame, const z3::model& model,
                         const NetworkState& replay,
                         const std::set<size_t>& reset,
                         const NetworkState::ControlEvents& control_events,
                         const ScanStep& step, bool event,
                         std::map<size_t, std::string>& fill) const {
    for (size_t id : state_.DataRegisters().order) {
      if (replay.Value(id) != Decode(model, frame.state.at(id))) {
        throw std::runtime_error(
            "symbolic/concrete DataRegister replay mismatch at " +
            netlist_.nodes[id].name);
      }
      const auto history = control_events.find(id);
      if (history != control_events.end()) {
        fill[id] = history->second ? *history->second : initial_fill_.at(id);
      } else if (model.eval(frame.active.at(id), true).is_true()) {
        throw std::runtime_error("missing concrete DataRegister write event");
      }
      if (fill.at(id) != Decode(model, frame.fill.at(id))) {
        throw std::runtime_error("symbolic/concrete data fill mismatch");
      }
    }
    for (size_t id : registers_) {
      const auto& node = netlist_.nodes[id];
      const auto history = control_events.find(id);
      const bool planned = step.loads.count(node.name) != 0;
      if (history != control_events.end() && history->second && planned) {
        fill[id] = *history->second;
      }
      const bool recorded_reset =
          history != control_events.end() &&
          (!history->second || (!planned && reset.count(id)));
      const bool fallback_reset =
          history == control_events.end() &&
          ((reset.count(id) && !step.writes.count(node.name)) ||
           (node.reset && replay.Read(*node.reset) == '1'));
      if (event && node.reset && (recorded_reset || fallback_reset)) {
        fill[id] = initial_fill_.at(id);
        const auto& reset_bits = node.constants.at("ResetValue").data;
        for (size_t i = 0; i < reset_bits.size(); ++i) {
          if (reset_bits[i] != 'x') fill[id][i] = reset_bits[i];
        }
      }
      if (fill.at(id) != Decode(model, frame.fill.at(id))) {
        throw std::runtime_error("symbolic/concrete controlling fill mismatch");
      }
    }
  }
  void FilterTapData(ScanStep* step, bool parallel) const {
    for (size_t id : tap_data_registers_) {
      const auto& name = netlist_.nodes[id].name;
      const auto comparison = step->expected.find(name);
      if (comparison != step->expected.end() &&
          comparison->second.find_first_not_of('x') != std::string::npos) {
        throw ScheduleLimit(
            "synthetic TAP comparison requires raw scan output at " + name);
      }
      step->writes.erase(name);
      step->expected.erase(name);
    }
    if (!parallel && step->writes.empty() && step->expected.empty()) {
      throw ScheduleLimit(
          "TAP scan has no expressible PDL access; raw scan output is "
          "required");
    }
  }
  ScanStep ReplayParallel(
      const Frame& frame, const z3::model& model, NetworkState* replay,
      TapExecution* tap_replay, std::map<size_t, std::string>& preserved,
      const std::map<size_t, std::string>& desired_preserved) {
    ScanStep step;
    std::map<size_t, std::string> driven;
    for (const auto& [id, drive] : frame.drives) {
      if (!model.eval(drive, true).is_true()) continue;
      const auto bits = Decode(model, frame.state.at(id));
      driven.emplace(id, bits);
      step.writes.emplace(netlist_.nodes[id].name, bits);
    }
    if (!driven.empty()) {
      if (tap_replay) {
        tap_replay->ApplyInputs(driven);
        replay->Restore(tap_replay->State());
      } else {
        replay->ApplyInputs(driven);
      }
    }
    CheckPreserved(*replay, preserved, desired_preserved);
    for (const auto& comparison : frame.comparisons) {
      if (!model.eval(comparison.selected, true).is_true()) continue;
      const auto& output = netlist_.nodes[comparison.output.node];
      auto& bits = step.expected.try_emplace(output.name, output.Width(), 'x')
                       .first->second;
      if (comparison.expected != 'x') {
        bits.at(Offset(output, comparison.output.index)) = comparison.expected;
      }
    }
    for (const auto& [id, value] : frame.state) {
      // Protocol pins (TDI/TMS/clock) belong to concrete TAP/scan replay, not
      // the parallel data event. Check every storage element and planned PI.
      if (netlist_.nodes[id].model != ModelKind::kRegister &&
          !frame.drives.count(id)) {
        continue;
      }
      if (replay->Value(id) != Decode(model, value)) {
        throw std::runtime_error(
            "symbolic/concrete parallel replay mismatch at " +
            netlist_.nodes[id].name);
      }
    }
    return step;
  }
  std::map<size_t, ScanControl> ClientControls(
      NetworkState* state, const std::set<size_t>& active,
      const std::string& phase, const std::map<Key, char>& selections) const {
    const bool falling = phase == "update";
    const auto pins = ClientPins(phase, false, selections);
    for (const auto& [id, bits] : ClientPins(phase, true, selections)) {
      state->Drive(id, bits);
    }
    state->Reset();
    std::map<Key, char> clocks;
    for (const auto& pin : client_clocks_) {
      clocks.emplace(pin, state->Read({pin.first, pin.second}));
    }
    for (const auto& [id, bits] : pins) {
      if (client_pins_.at(id) == "clock") state->Drive(id, bits);
    }
    for (auto& [pin, before] : clocks) {
      const char after = state->Read({pin.first, pin.second});
      const char from = falling ? '1' : '0', to = falling ? '0' : '1';
      if (before == to && after == from) {
        throw std::runtime_error("inverted client TCK phase is not supported");
      }
      before = before == to || after == from   ? '0'
               : before == from && after == to ? '1'
                                               : 'x';
    }
    std::map<size_t, ScanControl> controls;
    const std::vector<TapExecution::InterfacePorts> empty;
    for (size_t id : registers_) {
      const auto found = register_ports_.find(id);
      controls.emplace(
          id, EffectiveScanControl(
                  netlist_, *state,
                  found == register_ports_.end() ? empty : found->second,
                  {phase == "capture" ? '1' : '0', phase == "shift" ? '1' : '0',
                   falling ? '1' : '0', active.count(id) ? '1' : '0'},
                  clocks));
    }
    return controls;
  }
  ScanStep Replay(const Frame& frame, const z3::model& model,
                  const std::map<size_t, std::string>& expected,
                  const std::set<size_t>& required, bool events,
                  NetworkState* replay, TapExecution* tap_replay,
                  std::map<size_t, std::string>& preserved,
                  const std::map<size_t, std::string>& desired_preserved) {
    std::function<void(const NetworkState&)> check;
    if (!preserved.empty()) {
      check = [&](const NetworkState& state) {
        CheckPreserved(state, preserved, desired_preserved);
      };
    }
    std::vector<size_t> roots;
    for (size_t i = 0; i < frame.selected.size(); ++i) {
      if (model.eval(frame.selected[i], true).is_true()) {
        roots.push_back(roots_[i]);
      }
    }
    if (roots.empty()) {
      throw std::runtime_error("solver selected no serial output");
    }
    ScanStep step;
    std::map<Key, char> selections;
    for (const auto& [key, clients] : client_selectors_) {
      char active = '0';
      for (size_t id : clients) {
        if (model.eval(frame.clients.at(id), true).is_true()) active = '1';
      }
      selections.emplace(key, active);
    }
    if (client_controls_) {
      for (const auto& [id, bits] : ClientPins("capture", false, selections)) {
        replay->Drive(id, bits);
      }
      if (!clients_.empty()) {
        replay->Reset();
        if (check) check(*replay);
      }
    }
    if (tap_) {
      step.instruction = Decode(model, *frame.instruction) == "1";
      for (const auto& [id, tap] : instruction_registers_) {
        replay->Drive(tap->ir_select, step.instruction ? "1" : "0");
      }
    }
    std::map<size_t, std::string> capture_expected;
    for (const auto& item : frame.comparisons) {
      if (!model.eval(item.selected, true).is_true()) continue;
      const auto& node = netlist_.nodes[item.output.node];
      auto& bits =
          capture_expected.try_emplace(item.output.node, node.Width(), 'x')
              .first->second;
      if (item.expected != 'x') {
        bits.at(Offset(node, item.output.index)) = item.expected;
      }
    }
    std::map<size_t, ScanControl> controls;
    std::vector<std::pair<BitReference, ScanLane>> lanes;
    size_t clocks = 0;
    for (size_t root : roots) {
      ScanLane lane;
      const auto endpoint = [&](BitReference bit) {
        const auto& node = netlist_.nodes.at(bit.node);
        return node.name +
               (node.indexed ? "[" + std::to_string(bit.index) + "]" : "");
      };
      lane.output = endpoint(graph_[root].bit);
      std::vector<std::pair<size_t, bool>> registers;
      bool inverted = false;
      size_t vertex = root;
      while (!graph_[vertex].input) {
        const auto& current = graph_[vertex];
        if (current.bit.node == kMissing) {
          throw std::runtime_error("solver chose disconnected serial input");
        }
        const auto& node = netlist_.nodes.at(current.bit.node);
        if (node.kind == "ScanRegister") {
          registers.emplace_back(current.bit.node, inverted);
        }
        size_t selected = kMissing;
        for (size_t e = 0; e < current.edges.size(); ++e) {
          const auto& edge = current.edges[e];
          const bool active =
              edge.choice == kMissing || Match(*replay, node, edge.choice);
          if (active) {
            if (selected != kMissing) {
              throw std::runtime_error("ambiguous replay scan path");
            }
            selected = e;
          }
        }
        if (selected == kMissing) {
          throw std::runtime_error("unknown or disconnected replay scan path");
        }
        inverted = inverted != current.edges[selected].inverted;
        vertex = current.edges[selected].next;
      }
      const auto input_bit = graph_[vertex].bit;
      lane.input = endpoint(input_bit);
      for (const auto& [id, suffix] : registers) {
        const auto bits = Decode(model, frame.loads.at(id));
        step.loads.emplace(netlist_.nodes[id].name, bits);
        for (auto it = bits.rbegin(); it != bits.rend(); ++it) {
          lane.serial += inverted != suffix ? (*it == '1' ? '0' : '1') : *it;
        }
        std::string compare(bits.size(), 'x');
        if (frame.read.count(id) &&
            model.eval(frame.read.at(id), true).is_true()) {
          compare = expected.at(id);
          step.expected.emplace(netlist_.nodes[id].name, compare);
        } else if (step.instruction && !required.count(id)) {
          compare = instruction_registers_.at(id)->bsdl.capture;
          step.expected.emplace(netlist_.nodes[id].name, compare);
        }
        const auto captured = capture_expected.find(id);
        if (captured != capture_expected.end()) {
          for (size_t bit = 0; bit < compare.size(); ++bit) {
            if (captured->second[bit] != 'x') {
              compare[bit] = captured->second[bit];
            }
          }
          step.expected.insert_or_assign(netlist_.nodes[id].name, compare);
        }
        for (auto it = compare.rbegin(); it != compare.rend(); ++it) {
          lane.serial_expected +=
              suffix && *it != 'x' ? (*it == '1' ? '0' : '1') : *it;
        }
        controls[id] = {'1', '0', '0', '1'};
      }
      clocks = std::max(clocks, lane.serial.size());
      lanes.emplace_back(input_bit, std::move(lane));
    }
    for (auto& [input, lane] : lanes) {
      lane.serial.insert(0, clocks - lane.serial.size(), '0');
      lane.serial_expected.append(clocks - lane.serial_expected.size(), 'x');
      if (lanes.size() == 1) {
        step.input = lane.input;
        step.output = lane.output;
        step.serial = lane.serial;
        step.serial_expected = lane.serial_expected;
      } else {
        step.lanes.push_back(lane);
      }
    }
    std::set<size_t> writable, reset_held;
    if (tap_replay) {
      for (char tms : std::string(step.instruction ? "1100" : "100")) {
        Cycle(tap_replay, tms, {}, check);
      }
      for (size_t i = 0; i < clocks; ++i) {
        std::map<size_t, std::string> inputs;
        for (const auto& [input, lane] : lanes) {
          inputs.emplace(input.node, std::string(1, lane.serial[i]));
        }
        Cycle(tap_replay, i + 1 == clocks ? '1' : '0', inputs, check);
      }
      for (const auto& [id, control] : controls) {
        const auto& node = netlist_.nodes[id];
        if (!node.reset || tap_replay->State().Read(*node.reset) == '0') {
          writable.insert(id);
        } else if (tap_replay->State().Read(*node.reset) == '1') {
          reset_held.insert(id);
        }
      }
      Cycle(tap_replay, '1', {}, check);
      Cycle(tap_replay, '0', {}, check);
      replay->Restore(tap_replay->State());
    } else {
      std::set<size_t> active;
      for (const auto& [id, control] : controls) active.insert(id);
      if (client_controls_) {
        controls = ClientControls(replay, active, "capture", selections);
      }
      replay->Clock(controls, true);
      if (check) check(*replay);
      for (auto& [id, control] : controls) control = {'0', '1', '0', '1'};
      for (size_t tick = 0; tick < clocks; ++tick) {
        std::map<size_t, std::string> inputs;
        for (const auto& [input, lane] : lanes) {
          auto [entry, inserted] = inputs.try_emplace(input.node);
          if (inserted) entry->second = replay->Value(input.node);
          entry->second.at(Offset(netlist_.nodes.at(input.node), input.index)) =
              lane.serial[tick];
        }
        for (const auto& [input, bits] : inputs) replay->Drive(input, bits);
        if (client_controls_) {
          controls = ClientControls(replay, active, "shift", selections);
        }
        replay->Clock(controls, true);
        if (check) check(*replay);
      }
      for (auto& [id, control] : controls) control = {'0', '0', '1', '1'};
      if (client_controls_) {
        controls = ClientControls(replay, active, "update", selections);
      }
      for (const auto& [id, control] : controls) {
        const auto& node = netlist_.nodes[id];
        if (control.select == '1' && control.update == '1' &&
            step.loads.count(node.name) &&
            (!node.reset || replay->Read(*node.reset) == '0')) {
          writable.insert(id);
        }
        if (node.reset && replay->Read(*node.reset) == '1') {
          reset_held.insert(id);
        }
      }
      replay->Clock(controls, false);
      if (check) check(*replay);
      if (client_controls_) {
        for (const auto& [id, bits] : ClientPins("idle", false, selections)) {
          replay->Drive(id, bits);
        }
        replay->Reset();
        if (check) check(*replay);
      }
    }
    for (size_t id : writable) {
      const auto& node = netlist_.nodes[id];
      if ((!node.reset || replay->Read(*node.reset) == '0') &&
          replay->Value(id) == step.loads.at(node.name)) {
        step.writes.emplace(node.name, step.loads.at(node.name));
      }
    }
    // A scanned register held in reset was observed, but received no update.
    for (const auto& [name, bits] : step.loads) {
      if (!step.writes.count(name)) {
        const auto required =
            frame.needs_write.find(netlist_.node_ids.at(name));
        // Hidden storage can still consume a reset-held register's serial bits.
        if ((events || !reset_held.count(netlist_.node_ids.at(name))) &&
            required != frame.needs_write.end() &&
            model.eval(required->second, true).is_true()) {
          throw ScheduleLimit("unrepresented scan load under reset at " + name +
                              "; raw scan output is required");
        }
        step.expected.emplace(name, std::string(bits.size(), 'x'));
      }
    }
    for (size_t id : registers_) {
      if (replay->Value(id) != Decode(model, frame.state.at(id))) {
        throw std::runtime_error(
            netlist_.nodes[id].origin +
            ": symbolic/concrete scan replay mismatch at " +
            netlist_.nodes[id].name);
      }
    }
    return step;
  }
  const Netlist& netlist_;
  NetworkState state_;
  OneHotReadResolver one_hot_;
  const TapRuntimeModel* tap_ = nullptr;
  std::map<size_t, const TapRuntimeModel*> instruction_registers_;
  std::set<size_t> tap_data_registers_, tap_pins_, tap_clocks_;
  std::unique_ptr<TapExecution> execution_;
  bool client_controls_ = false;
  bool parallel_chains_ = false;
  bool data_reset_dependencies_ = false, feedback_resets_ = false;
  size_t reset_passes_ = 0, reset_work_ = 0;
  std::vector<std::vector<Key>> clients_;
  std::vector<size_t> client_route_counts_;
  std::map<Key, std::vector<size_t>> client_selectors_, client_inputs_;
  std::map<Key, std::vector<std::pair<size_t, Key>>> client_routes_;
  std::map<size_t, std::string> client_pins_;
  std::map<size_t, std::set<size_t>> client_pin_bits_;
  std::set<Key> client_clocks_;
  std::set<Key> idle_selects_;
  std::map<size_t, std::vector<TapExecution::InterfacePorts>> register_ports_;
  std::unique_ptr<DataReadRoutes> data_read_routes_, scan_read_routes_,
      mixed_read_routes_;
  bool tap_ready_ = false;
  std::string global_resets_ = "xx";
  z3::context context_;
  std::vector<size_t> registers_, roots_, order_, reset_order_;
  std::vector<std::vector<size_t>> reset_components_;
  std::map<size_t, size_t> reset_component_;
  std::map<size_t, std::string> fill_, initial_fill_, control_masks_,
      end_states_, data_control_masks_;
  std::map<size_t, std::set<size_t>> reset_dependencies_;
  std::set<Key> control_seen_;
  std::map<Key, size_t> ids_;
  std::vector<Vertex> graph_;
  std::map<size_t, std::vector<std::pair<BitReference, size_t>>> source_edges_;
};
ScanScheduler::ScanScheduler(const Netlist& netlist)
    : impl_(std::make_unique<Impl>(netlist)) {}
ScanScheduler::ScanScheduler(const ScanScheduler& other)
    : impl_(std::make_unique<Impl>(*other.impl_)) {}
ScanScheduler::~ScanScheduler() = default;
void ScanScheduler::Reset(bool synchronous) { impl_->Reset(synchronous); }
void ScanScheduler::Idle(uint64_t cycles,
                         const std::map<size_t, std::string>& preserved) {
  impl_->Idle(cycles, preserved);
}
void ScanScheduler::Assume(const std::string& target, const std::string& bits) {
  impl_->Assume(target, bits);
}
std::vector<ScanStep> ScanScheduler::Apply(
    const std::vector<Operation>& operations, bool together,
    const std::map<size_t, std::string>& preserved, bool explicit_writes) {
  return impl_->Apply(operations, together, preserved, explicit_writes);
}
std::vector<std::vector<Operation>> ScanScheduler::ResetPrerequisites(
    const std::vector<Operation>& operations,
    const std::map<size_t, std::string>& preserved,
    const std::map<size_t, std::string>& unwritten) {
  return impl_->ResetPrerequisites(operations, preserved, unwritten);
}
void ScanScheduler::CheckLocalResets(
    const std::vector<Operation>& operations) const {
  impl_->CheckLocalResets(operations);
}
std::string ScanScheduler::Value(const std::string& target) const {
  return impl_->Value(target);
}
}  // namespace pattern
