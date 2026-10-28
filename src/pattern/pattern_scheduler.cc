// Copyright 2026 The ICL Toolsets Authors.
// SPDX-License-Identifier: MIT
#include "pattern/pattern_scheduler.h"

#include <algorithm>
#include <chrono>
#include <functional>
#include <set>
#include <stdexcept>
#include <utility>

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
SymbolicValue Choose(const z3::expr& choice, const SymbolicValue& a,
                     const SymbolicValue& b) {
  return {z3::ite(choice, a.bits, b.bits), z3::ite(choice, a.known, b.known)};
}
SymbolicValue ResetValue(const SymbolicValue& control,
                         const SymbolicValue& reset,
                         const SymbolicValue& previous) {
  const auto known =
      reset.known & previous.known & ~(reset.bits ^ previous.bits);
  return Choose(
      Known(control, true), reset,
      Choose(Known(control, false), previous, {previous.bits & known, known}));
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
  explicit Impl(const Netlist& netlist) : netlist_(netlist), state_(netlist) {
    if (netlist.taps.size() > 1) {
      throw std::runtime_error("multiple TAP scheduling is not implemented");
    }
    if (!netlist.taps.empty()) {
      tap_ = &netlist.taps.begin()->second;
      execution_ = std::make_unique<TapExecution>(netlist);
      state_.Restore(execution_->State());
    }
    for (size_t id = 0; id < netlist.nodes.size(); ++id) {
      const auto& node = netlist.nodes[id];
      // End-state obligations apply even when PDL names the underlying storage.
      if (node.properties.count("iApplyEndState")) {
        throw std::runtime_error(node.origin + ": iApplyEndState on " +
                                 node.name +
                                 " requires restoration scheduling, which is "
                                 "not implemented");
      }
      if (node.kind == "ScanRegister" || node.kind == "DataInPort" ||
          node.kind == "DataOutPort") {
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
          roots_.push_back(Add({id, index}));
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
    if (roots_.size() > 1) {
      throw std::runtime_error(
          "multiple scan outputs require shared-control transaction "
          "scheduling");
    }
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
      control_masks_[tap_->instruction_register] =
          std::string(netlist.nodes[tap_->instruction_register].Width(), '1');
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
    for (const auto& binding : EffectiveScanInterfaces(netlist)) {
      const auto check = [&](const std::vector<BitReference>& ports) {
        for (const auto& port : ports) {
          const auto& kind = netlist.nodes.at(port.node).kind;
          if (kind != "ScanInPort" && kind != "ScanOutPort" &&
              kind != "ResetPort") {
            throw std::runtime_error(
                binding.origin +
                ": interface controls require event scheduling integration");
          }
        }
      };
      check(binding.interface.ports);
      for (const auto& chain : binding.interface.chains) check(chain.ports);
    }
  }
  void Cycle(TapExecution* run, char tms, char tdi = '0',
             const std::function<void(const NetworkState&)>& check = {}) const {
    run->Drive(netlist_.nodes.at(tap_->pins.at("tap_scan_mode")).name,
               std::string(1, tms));
    run->Drive(netlist_.nodes.at(tap_->pins.at("tap_scan_in")).name,
               std::string(1, tdi));
    const auto& clock = netlist_.nodes.at(tap_->pins.at("tap_scan_clock")).name;
    run->RisingEdge(clock);
    if (check) check(run->State());
    run->FallingEdge(clock);
    if (check) check(run->State());
  }
  void Reset(bool synchronous) {
    if (tap_) {
      auto run = std::make_unique<TapExecution>(*execution_);
      const auto reset = tap_->pins.find("tap_scan_reset");
      if (reset != tap_->pins.end()) {
        const auto& pin = netlist_.nodes.at(reset->second).name;
        if (!synchronous) run->Drive(pin, "0");
        run->Drive(pin, "1");
      }
      if (synchronous || reset == tap_->pins.end()) {
        for (int i = 0; i < 5; ++i) Cycle(run.get(), '1');
      }
      auto fill = initial_fill_;
      // IR reset is driven by the controller, not an ICL ResetValue property.
      fill[tap_->instruction_register] =
          run->State().Value(tap_->instruction_register);
      for (size_t id : registers_) {
        const auto& node = netlist_.nodes[id];
        if (node.reset && run->State().Read(*node.reset) == '1') {
          const auto& bits = node.constants.at("ResetValue").data;
          for (size_t i = 0; i < bits.size(); ++i) {
            if (bits[i] != 'x') fill[id][i] = bits[i];
          }
        }
      }
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
  void Idle(uint64_t cycles, const std::map<size_t, std::string>& preserved) {
    if (cycles == 0) return;
    if (!tap_ || !tap_ready_) {
      throw std::runtime_error(
          "TCK idle replay requires a single TAP and explicit iReset");
    }
    auto run = std::make_unique<TapExecution>(*execution_);
    run->TakeResetEvents();
    auto fill = fill_;
    auto held = preserved;
    const auto check = [&](const NetworkState& state) {
      CheckPreserved(state, held, preserved);
    };
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
      const NetworkState previous(run->State());
      Cycle(run.get(), '0', '0', check);
      for (size_t id : run->TakeResetEvents()) {
        const auto& node = netlist_.nodes[id];
        if (node.kind == "ScanRegister") {
          fill[id] = initial_fill_.at(id);
          const auto& bits = node.constants.at("ResetValue").data;
          for (size_t i = 0; i < bits.size(); ++i) {
            if (bits[i] != 'x') fill[id][i] = bits[i];
          }
        }
      }
      // Every cycle starts/ends in known Run-Test/Idle with the same inputs.
      // An identical complete state is a fixed point for all remaining cycles.
      if (run->State().SameState(previous)) break;
    }
    state_.Restore(run->State());
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
    try {
      return Solve(operations, together, kMaxSteps, {}, true, preserved,
                   explicit_writes);
    } catch (const std::exception& error) {
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
      const std::set<size_t>& unwritten) {
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
      if (controls.count(id) && !unwritten.count(id)) {
        if (node.kind != "ScanRegister" ||
            (tap_ && id == tap_->instruction_register)) {
          throw std::runtime_error(
              "local-reset control is not a writable TDR: " + name);
        }
        total_bits += node.Width();
        if (total_bits > 4096) {
          throw std::runtime_error("local-reset solver bit limit exceeded");
        }
        const auto bits =
            context_.bv_const(("reset_" + name).c_str(), node.Width());
        const auto known =
            SymbolicValue::Literal(context_, std::string(node.Width(), '1'))
                .bits;
        const SymbolicValue value{bits, known};
        assignments.emplace(id, value);
        choices.emplace(id, value);
        const auto held = preserved.find(id);
        if (held != preserved.end()) {
          solver.add(bits ==
                     SymbolicValue::Literal(context_, held->second).bits);
        }
        const auto previous =
            SymbolicValue::Literal(context_, state_.Value(id));
        changes =
            changes + z3::ite(bits == previous.bits && previous.known == known,
                              context_.int_val(0), context_.int_val(1));
        const auto fill = SymbolicValue::Literal(context_, fill_.at(id));
        tie_breaks.push_back(z3::bv2int(bits ^ fill.bits, false));
      } else if (node.kind == "ScanRegister" || state_.IsDriven(id)) {
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
      for (const auto& [id, value] : preserved) {
        if (controls.count(id)) {
          message +=
              "\n  reserved " + netlist_.nodes.at(id).name + " = " + value;
        }
      }
      for (size_t id : unwritten) {
        if (controls.count(id)) {
          message +=
              "\n  taken-but-unwritten resource " + netlist_.nodes.at(id).name;
        }
      }
      throw std::runtime_error(message);
    }
    const auto model = solver.get_model();
    std::map<size_t, size_t> depth;
    std::map<size_t, std::vector<Operation>> layers;
    for (size_t id : reset_order_) {
      if (!choices.count(id)) continue;
      const auto bits = Decode(model, choices.at(id));
      if (bits == state_.Value(id)) continue;
      size_t level = 0;
      for (size_t parent : reset_dependencies_[id]) {
        const auto found = depth.find(parent);
        if (found != depth.end()) level = std::max(level, found->second + 1);
      }
      depth[id] = level;
      const auto& node = netlist_.nodes[id];
      layers[level].push_back(
          {"iWrite", {node.name, "0b" + bits}, node.origin, {}});
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
  void CheckPreserved(const NetworkState& state,
                      std::map<size_t, std::string>& held,
                      const std::map<size_t, std::string>& desired) const {
    for (auto& [id, previous] : held) {
      const auto actual = state.Value(id);
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
    for (const auto& operation : operations) {
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
    return node.scope == 0 && node.kind == "DataInPort" &&
           !node.inputs.count("Source");
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
  std::vector<ScanStep> SolvePrimaryWrites(
      const std::vector<Operation>& operations, bool together) {
    for (const auto& node : netlist_.nodes) {
      if (node.model == ModelKind::kRegister ||
          (node.model == ModelKind::kPort && node.kind == "AddressPort")) {
        throw std::runtime_error(
            "stateful/addressed primary-input scheduling is not implemented");
      }
    }
    std::map<size_t, std::string> goals;
    for (const auto& operation : operations) {
      if (operation.command != "iWrite" || operation.arguments.size() != 2) {
        throw std::runtime_error(
            "primary-input scheduling requires literal iWrite target/value");
      }
      const auto target = ResolvePdlTarget(netlist_, operation.arguments[0]);
      if (!target.access_together.empty()) {
        throw std::runtime_error(
            "AccessTogether primary-port groups are not implemented");
      }
      const auto bits =
          PdlDataValue(context_, netlist_, target, operation.arguments[1]);
      for (size_t i = 0; i < bits.size(); ++i) {
        const auto& ref = target.bits[i];
        if (ref.node == kMissing ||
            (netlist_.nodes.at(ref.node).kind != "DataInPort" &&
             netlist_.nodes.at(ref.node).kind != "DataOutPort")) {
          throw std::runtime_error(
              "primary write requires a data port or its Alias");
        }
        auto found = goals.find(ref.node);
        if (found == goals.end()) {
          auto goal = fill_.at(ref.node);
          const auto& node = netlist_.nodes[ref.node];
          for (size_t bit = 0; bit < goal.size(); ++bit) {
            const int64_t index =
                node.left + (node.left <= node.right
                                 ? static_cast<int64_t>(bit)
                                 : -static_cast<int64_t>(bit));
            // Tessent permits X on structurally padded bits; disabled ports are
            // not padding and retain their distinct 0/1 controlling state.
            if (FixedOneHotPadding({ref.node, index})) goal[bit] = 'x';
          }
          found = goals.emplace(ref.node, std::move(goal)).first;
        }
        if (bits[i] != 'x') {
          found->second.at(Offset(netlist_.nodes[ref.node], ref.index)) =
              ref.inverted ? (bits[i] == '1' ? '0' : '1') : bits[i];
        }
      }
    }
    z3::optimize solver(context_);
    z3::params parameters(context_);
    parameters.set("timeout", 2000U);
    solver.set(parameters);
    Values assignments;
    std::map<size_t, z3::expr> drives;
    std::vector<z3::expr> deviations;
    auto count = context_.int_val(0);
    size_t total_bits = 0;
    for (const auto& [name, id] : netlist_.node_ids) {
      const auto& node = netlist_.nodes[id];
      if (!PrimaryInput(node)) continue;
      total_bits += node.Width();
      if (drives.size() >= 256 || total_bits > 4096) {
        throw std::runtime_error(
            "primary-input scheduler resource limit exceeded");
      }
      const auto drive = context_.bool_const(("drive_" + name).c_str());
      const auto bits =
          context_.bv_const(("pin_" + name).c_str(), node.Width());
      const auto known =
          SymbolicValue::Literal(context_, std::string(node.Width(), '1')).bits;
      assignments.emplace(
          id, Choose(drive, {bits, known},
                     SymbolicValue::Literal(context_, state_.Value(id))));
      drives.emplace(id, drive);
      count = count + z3::ite(drive, context_.int_val(1), context_.int_val(0));
      deviations.push_back(bits ^
                           SymbolicValue::Literal(context_, fill_.at(id)).bits);
      // An explicitly written primary port remains an action even if unchanged.
      if (goals.count(id)) solver.add(drive);
    }
    SymbolicNetwork values(netlist_, context_, assignments, global_resets_);
    for (const auto& [id, goal] : goals) {
      const auto wanted = SymbolicValue::Literal(context_, goal);
      const auto actual = values.Value(id);
      solver.add((actual.known & wanted.known) == wanted.known &&
                 (actual.bits & wanted.known) == wanted.bits);
    }
    solver.minimize(count);
    // Name order and bit-vector objectives make flat/source replay
    // deterministic.
    for (const auto& deviation : deviations) solver.minimize(deviation);
    const auto status = solver.check();
    if (status == z3::unsat) {
      throw std::runtime_error(
          "cannot justify primary-input write requirements");
    }
    if (status != z3::sat) {
      throw std::runtime_error("primary-input solver resource limit reached");
    }
    const auto model = solver.get_model();
    NetworkState replay(state_);
    auto fill = fill_;
    ScanStep step;
    step.together = together;
    for (const auto& [id, drive] : drives) {
      if (!model.eval(drive, true).is_true()) continue;
      const auto bits = Decode(model, assignments.at(id));
      replay.Drive(id, bits);
      fill.at(id) = bits;
      step.writes.emplace(netlist_.nodes[id].name, bits);
    }
    for (const auto& [id, goal] : goals) {
      const auto actual = replay.Value(id);
      for (size_t i = 0; i < goal.size(); ++i) {
        if (goal[i] != 'x' && actual[i] != goal[i]) {
          throw std::runtime_error(
              "symbolic/concrete primary replay mismatch at " +
              netlist_.nodes[id].name);
        }
      }
    }
    for (const auto& [id, goal] : goals) {
      for (size_t i = 0; i < goal.size(); ++i) {
        if (goal[i] != 'x') fill.at(id)[i] = goal[i];
      }
    }
    state_.Restore(replay);
    fill_ = std::move(fill);
    return step.writes.empty() ? std::vector<ScanStep>{}
                               : std::vector<ScanStep>{std::move(step)};
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
      } else if (node.inputs.count("Source")) {
        sources.emplace_back(
            node.inputs.at("Source").at(Offset(node, current.index)), kMissing);
      } else if (node.kind == "ScanInPort" && node.scope == 0 &&
                 node.Width() == 1) {
        graph_[id].input = true;
      } else if (node.model == ModelKind::kGroup) {
        throw std::runtime_error(
            node.origin + ": one-hot serial scheduling is not implemented");
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
      if (bit.node == kMissing ||
          !control_seen_.insert({bit.node, bit.index}).second) {
        continue;
      }
      const auto& node = netlist_.nodes.at(bit.node);
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
      for (const auto& instruction : node.logic) {
        pending.insert(pending.end(), instruction.signals.begin(),
                       instruction.signals.end());
      }
    }
  }
  void BuildResetOrder() {
    auto& dependencies = reset_dependencies_;
    for (size_t id : registers_) {
      const auto& node = netlist_.nodes[id];
      if (!node.reset) continue;
      std::vector<size_t> pending;
      if (node.reset->node != kMissing) pending.push_back(node.reset->node);
      std::set<size_t> seen;
      while (!pending.empty()) {
        const auto next = pending.back();
        pending.pop_back();
        if (!seen.insert(next).second) continue;
        const auto& source = netlist_.nodes.at(next);
        if (source.model == ModelKind::kRegister) {
          dependencies[id].insert(next);
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
    for (size_t id : registers_) {
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
    if (reset_order_.size() != registers_.size()) {
      throw std::runtime_error(
          "feedback reset scheduling requires fixed-point constraints");
    }
  }
  z3::expr Match(SymbolicNetwork& values, const NetlistNode& node,
                 size_t choice) {
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
  struct Frame {
    Values state, loads, fill;
    std::optional<SymbolicValue> instruction;
    std::map<size_t, z3::expr> active, read;
    std::vector<z3::expr> selected;
  };
  void SelectScan(z3::optimize& solver, SymbolicNetwork& values, size_t time,
                  Frame& frame) {
    std::vector<z3::expr> viable, observed;
    for (size_t i = 0; i < graph_.size(); ++i) {
      viable.push_back(context_.bool_val(false));
      observed.push_back(context_.bool_val(false));
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
      }
      if (vertex.bit.node != kMissing) {
        const auto& node = netlist_.nodes.at(vertex.bit.node);
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
      solver.add(z3::implies(selected, viable[roots_[i]]));
      observed[roots_[i]] = observed[roots_[i]] || selected;
      count =
          count + z3::ite(selected, context_.int_val(1), context_.int_val(0));
    }
    solver.add(count == 1);
    for (auto it = order_.rbegin(); it != order_.rend(); ++it) {
      const size_t id = *it;
      const auto& vertex = graph_[id];
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
        observed[vertex.edges[e].next] =
            observed[vertex.edges[e].next] || (observed[id] && gates[id][e]);
      }
    }
  }
  std::optional<std::set<size_t>> ActivePath(
      const NetworkState& snapshot) const {
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
        bool matches = edge.choice == kMissing;
        if (!matches) {
          for (const auto& code : node.choices.at(edge.choice).codes) {
            bool equal = true;
            for (size_t bit = 0; bit < code.data.size(); ++bit) {
              if (code.data[bit] != 'x' &&
                  snapshot.Read(node.inputs.at("SelectedBy").at(bit)) !=
                      code.data[bit]) {
                equal = false;
              }
            }
            matches = matches || equal;
          }
        }
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
  std::optional<std::vector<ScanStep>> TryGreedy(
      const std::vector<Operation>& operations,
      const std::map<size_t, std::string>& goals,
      const std::set<size_t>& required, bool together,
      const std::map<size_t, std::string>& preserved,
      const std::map<size_t, std::string>& desired_preserved,
      bool explicit_writes) {
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
                     explicit_writes);
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
        if (state_.IsDriven(id)) {
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
        solver.add(load == held.bits || load == desired.bits);
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
    SymbolicNetwork values(netlist_, context_, assignments, global_resets_);
    Frame frame;
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
          if (tap_ && id == tap_->instruction_register) continue;
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
          // A protected register may only take the original explicit value,
          // including its non-control bits, during prerequisite configuration.
          if (preserved.count(id)) load = desired;
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
                                 false, held, explicit_writes);
        for (auto& [id, value] : held) value = trial.state_.Value(id);
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
  void ConstrainControls(z3::optimize& solver, const Values& state,
                         const Frame& frame) {
    const auto ir = Known(*frame.instruction, true);
    const auto zero = SymbolicValue::Literal(context_, "0");
    const auto one = SymbolicValue::Literal(context_, "1");
    const auto clock = tap_->pins.at("tap_scan_clock");
    for (const std::string phase : {"capture", "shift", "update", "idle"}) {
      Values pins = state;
      for (const auto& [role, id] : tap_->controls) {
        pins.insert_or_assign(
            id, Choose(!ir && context_.bool_val(role == phase), one, zero));
      }
      pins.insert_or_assign(clock, phase == "update" ? one : zero);
      SymbolicNetwork before(netlist_, context_, pins, global_resets_);
      pins.insert_or_assign(clock, phase == "update" ? zero : one);
      SymbolicNetwork after(netlist_, context_, pins, global_resets_);
      for (size_t id : registers_) {
        if (id == tap_->instruction_register) continue;
        const auto active = frame.active.count(id) ? frame.active.at(id)
                                                   : context_.bool_val(false);
        const auto entry = execution_->RegisterInterfaces().find(id);
        std::vector<TapExecution::InterfacePorts> defaults;
        if (entry == execution_->RegisterInterfaces().end()) {
          defaults.emplace_back();
        }
        const auto& interfaces = entry == execution_->RegisterInterfaces().end()
                                     ? defaults
                                     : entry->second;
        for (const auto& [kind, role] :
             std::map<std::string, std::string>{{"CaptureEnPort", "capture"},
                                                {"ShiftEnPort", "shift"},
                                                {"UpdateEnPort", "update"}}) {
          auto effective = context_.bool_val(false);
          for (const auto& ports : interfaces) {
            const auto control = [&](const std::string& type,
                                     z3::expr fallback) {
              const auto found = ports.find(type);
              if (found == ports.end()) return fallback;
              auto enabled = context_.bool_val(true);
              for (const auto& [port, index] : found->second) {
                if (!netlist_.nodes[port].inputs.count("Source") &&
                    !state_.IsDriven(port)) {
                  enabled = enabled && fallback;
                } else {
                  enabled = enabled && Known(after.Read({port, index}), true);
                }
              }
              return enabled;
            };
            auto enabled = control("SelectPort", active);
            const auto clocks = ports.find("TCKPort");
            if (clocks != ports.end()) {
              for (const auto& [port, index] : clocks->second) {
                enabled =
                    enabled &&
                    Known(before.Read({port, index}), phase == "update") &&
                    Known(after.Read({port, index}), phase != "update");
              }
            }
            effective =
                effective ||
                (enabled &&
                 control(kind, !ir && context_.bool_val(role == phase)));
          }
          // A complete scan may not silently clock hidden or disabled storage.
          solver.add(effective ==
                     (active && !ir && context_.bool_val(role == phase)));
        }
      }
    }
  }
  std::vector<ScanStep> Solve(
      const std::vector<Operation>& operations, bool together,
      size_t max_steps = kMaxSteps,
      const std::map<size_t, std::string>& fixed_loads = {},
      bool allow_greedy = true,
      const std::map<size_t, std::string>& preserved = {},
      bool explicit_writes = true) {
    if (operations.empty()) return {};
    if (roots_.empty() && registers_.empty() && !tap_) {
      return SolvePrimaryWrites(operations, together);
    }
    if (tap_ && !tap_ready_) {
      throw std::runtime_error(
          "TAP state is unknown; explicit iReset is required");
    }
    std::map<size_t, std::string> goals, expected;
    std::set<size_t> required;
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
            for (const auto& bit : full.bits) {
              if (bit.node == kMissing ||
                  netlist_.nodes.at(bit.node).kind != "ScanRegister") {
                throw std::runtime_error(
                    "AccessTogether " + netlist_.nodes[alias].name +
                    " requires ScanRegister members in the serial backend");
              }
              group.members.insert(bit.node);
              required.insert(bit.node);
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
        const auto& ref = target.bits[i];
        if (ref.node == kMissing ||
            netlist_.nodes.at(ref.node).kind != "ScanRegister") {
          throw std::runtime_error(
              "read/write target requires a ScanRegister or its Alias");
        }
        if (tap_ &&
            std::any_of(
                tap_->data_registers.begin(), tap_->data_registers.end(),
                [&](const auto& item) { return item.second == ref.node; })) {
          throw std::runtime_error(
              "synthetic TAP data registers are not PDL targets");
        }
        required.insert(ref.node);
        auto& values = read ? expected : goals;
        auto entry =
            values
                .emplace(
                    ref.node,
                    read ? std::string(netlist_.nodes[ref.node].Width(), 'x')
                         : fill_.at(ref.node))
                .first;
        if (read || bits[i] != 'x') {
          entry->second.at(Offset(netlist_.nodes[ref.node], ref.index)) =
              ref.inverted && bits[i] != 'x' ? (bits[i] == '1' ? '0' : '1')
                                             : bits[i];
        }
      }
    }
    if (tap_ && required.count(tap_->instruction_register)) {
      const auto id = tap_->instruction_register;
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
    std::string access_context;
    std::set<size_t> grouped_reads;
    for (const auto& [key, group] : groups) {
      if (tap_ && group.members.count(tap_->instruction_register) &&
          group.members.size() > 1) {
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
    // Check structural SI/SO connectivity independently of unknown selectors.
    std::vector<bool> connected(graph_.size());
    for (size_t id : order_) {
      connected[id] = graph_[id].input;
      for (const auto& edge : graph_[id].edges) {
        connected[id] = connected[id] || connected[edge.next];
      }
    }
    for (size_t id : required) {
      bool reachable = false;
      for (const auto& [key, vertex] : ids_) {
        if (key.first == id && connected[vertex]) reachable = true;
      }
      if (!reachable) {
        throw std::runtime_error(netlist_.nodes[id].origin +
                                 ": disconnected scan input/output for " +
                                 netlist_.nodes[id].name);
      }
    }
    auto desired_preserved = preserved;
    for (auto& [id, value] : desired_preserved) {
      const auto& node = netlist_.nodes.at(id);
      if (node.kind != "ScanRegister" || value.size() != node.Width() ||
          value.find_first_not_of("01") != std::string::npos) {
        throw std::runtime_error("invalid reserved ScanRegister value");
      }
      if (explicit_writes && goals.count(id)) value = goals.at(id);
    }
    if (allow_greedy && registers_.size() >= 32 && expected.empty()) {
      auto planned = TryGreedy(operations, goals, required, together, preserved,
                               desired_preserved, explicit_writes);
      if (planned) return std::move(*planned);
    }
    const auto start = std::chrono::steady_clock::now();
    for (size_t horizon = 1; horizon <= max_steps; ++horizon) {
      if (std::chrono::steady_clock::now() - start > std::chrono::seconds(15) ||
          horizon * (graph_.size() + registers_.size()) > 262144) {
        throw std::runtime_error(
            "scan scheduling resource limit; reachability not proved");
      }
      z3::optimize solver(context_);
      z3::params parameters(context_);
      parameters.set("timeout", 2000U);
      solver.set(parameters);
      Values state, fills;
      std::map<size_t, z3::expr> reserved_changed;
      for (const auto& [id, value] : preserved) {
        reserved_changed.emplace(id, context_.bool_val(false));
      }
      for (size_t id : registers_) {
        state.emplace(id, SymbolicValue::Literal(context_, state_.Value(id)));
        fills.emplace(id, SymbolicValue::Literal(context_, fill_.at(id)));
      }
      for (size_t id = 0; id < netlist_.nodes.size(); ++id) {
        if (state_.IsDriven(id)) {
          state.emplace(id, SymbolicValue::Literal(context_, state_.Value(id)));
        }
      }
      std::map<size_t, z3::expr> touched;
      for (const auto& [id, goal] : goals) {
        touched.emplace(id, context_.bool_val(false));
      }
      std::map<size_t, z3::expr> read;
      for (const auto& [id, bits] : expected) {
        read.emplace(id, context_.bool_val(false));
      }
      std::vector<Frame> frames;
      std::vector<z3::expr> simultaneous(groups.size(),
                                         context_.bool_val(false));
      auto cycles = context_.int_val(0), changes = context_.int_val(0);
      for (size_t time = 0; time < horizon; ++time) {
        Frame frame;
        if (tap_) {
          const auto ir =
              context_.bv_const(("ir_" + std::to_string(time)).c_str(), 1);
          state.insert_or_assign(tap_->ir_select,
                                 SymbolicValue{ir, context_.bv_val(1, 1)});
          frame.instruction = SymbolicValue{ir, context_.bv_val(1, 1)};
          cycles = cycles + z3::ite(ir == context_.bv_val(1, 1),
                                    context_.int_val(6), context_.int_val(5));
        }
        SymbolicNetwork values(netlist_, context_, state, global_resets_);
        SelectScan(solver, values, time, frame);
        if (tap_) ConstrainControls(solver, state, frame);
        Values next = state;
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
          const auto requested =
              goals.count(id) ? SymbolicValue::Literal(context_, goals.at(id))
                              : fills.at(id);
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
          auto value = Choose(active, load, state.at(id));
          if (node.reset) {
            value =
                ResetValue(values.Read(*node.reset),
                           SymbolicValue::Literal(
                               context_, node.constants.at("ResetValue").data),
                           value);
          }
          next.insert_or_assign(id, value);
          cycles =
              cycles + z3::ite(active,
                               context_.int_val(static_cast<int>(node.Width())),
                               context_.int_val(0));
          const auto desired = SymbolicValue::Literal(context_, fill_.at(id));
          changes = changes + z3::ite(active && loaded != desired.bits,
                                      context_.int_val(1), context_.int_val(0));
          if (touched.count(id)) {
            touched.at(id) =
                touched.at(id) ||
                (active &&
                 (!node.reset ? context_.bool_val(true)
                              : Known(values.Read(*node.reset), false)));
          }
        }
        // Update qualification uses the old state. New reset assertions then
        // propagate in dependency order, without retroactively enabling writes.
        for (size_t id : reset_order_) {
          const auto& node = netlist_.nodes[id];
          if (!node.reset) continue;
          SymbolicNetwork after(netlist_, context_, next, global_resets_);
          next.at(id) =
              ResetValue(after.Read(*node.reset),
                         SymbolicValue::Literal(
                             context_, node.constants.at("ResetValue").data),
                         next.at(id));
        }
        for (const auto& [id, previous] : preserved) {
          const auto old = SymbolicValue::Literal(context_, previous);
          const auto wanted =
              SymbolicValue::Literal(context_, desired_preserved.at(id));
          const auto changed = reserved_changed.at(id);
          const auto legal = [&](const SymbolicValue& value) {
            return value.known == wanted.known &&
                   ((!changed && value.bits == old.bits) ||
                    value.bits == wanted.bits);
          };
          const auto active = frame.active.count(id) ? frame.active.at(id)
                                                     : context_.bool_val(false);
          solver.add(z3::implies(active, legal(frame.loads.at(id))));
          solver.add(legal(next.at(id)));
          reserved_changed.at(id) =
              reserved_changed.at(id) || next.at(id).bits == wanted.bits;
        }
        SymbolicNetwork after(netlist_, context_, next, global_resets_);
        size_t group_index = 0;
        for (const auto& [key, group] : groups) {
          auto complete = context_.bool_val(true);
          for (size_t id : group.members) {
            const auto active = frame.active.count(id)
                                    ? frame.active.at(id)
                                    : context_.bool_val(false);
            complete = complete && active;
            if (group.read) {
              if (group.requested.count(id)) {
                complete = complete && frame.read.at(id);
              }
            } else {
              if (goals.count(id)) {
                const auto requested =
                    SymbolicValue::Literal(context_, goals.at(id));
                complete = complete &&
                           frame.loads.at(id).bits == requested.bits &&
                           next.at(id).known == requested.known &&
                           next.at(id).bits == requested.bits;
              }
              const auto& node = netlist_.nodes[id];
              if (node.reset) {
                complete = complete && Known(values.Read(*node.reset), false) &&
                           Known(after.Read(*node.reset), false);
              }
            }
          }
          simultaneous[group_index] = simultaneous[group_index] || complete;
          ++group_index;
        }
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
            fill = Choose(Known(values.Read(*node.reset), true) ||
                              Known(after.Read(*node.reset), true),
                          SymbolicValue::Literal(context_, reset_fill), fill);
          }
          fills.at(id) = fill;
        }
        frame.fill = fills;
        frame.state = next;
        frames.push_back(std::move(frame));
        state = std::move(next);
      }
      for (const auto& [id, goal] : goals) {
        const auto expected = SymbolicValue::Literal(context_, goal);
        solver.add(touched.at(id) && state.at(id).known == expected.known &&
                   state.at(id).bits == expected.bits);
      }
      for (const auto& [id, observed] : read) solver.add(observed);
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
        std::set<size_t> reset;
        for (size_t id : registers_) {
          if (netlist_.nodes[id].reset &&
              replay.Read(*netlist_.nodes[id].reset) == '1') {
            reset.insert(id);
          }
        }
        auto step =
            Replay(frame, model, expected,
                   tap_ && !required.count(tap_->instruction_register), &replay,
                   tap_replay.get(), replayed_preserved, desired_preserved);
        size_t group_index = 0;
        for (const auto& [key, group] : groups) {
          bool complete = true;
          for (size_t id : group.members) {
            const auto& name = netlist_.nodes[id].name;
            if (group.read) {
              complete =
                  complete && step.loads.count(name) &&
                  (!group.requested.count(id) || step.expected.count(name));
            } else {
              const auto found = step.writes.find(name);
              complete = complete && found != step.writes.end() &&
                         (!goals.count(id) || found->second == goals.at(id));
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
        for (size_t id : registers_) {
          const auto& node = netlist_.nodes[id];
          if (node.reset &&
              (reset.count(id) || replay.Read(*node.reset) == '1')) {
            fill[id] = initial_fill_.at(id);
            const auto& reset_bits = node.constants.at("ResetValue").data;
            for (size_t i = 0; i < reset_bits.size(); ++i) {
              if (reset_bits[i] != 'x') fill[id][i] = reset_bits[i];
            }
          }
          if (fill.at(id) != Decode(model, frame.fill.at(id))) {
            throw std::runtime_error(
                "symbolic/concrete controlling fill mismatch");
          }
        }
        steps.push_back(std::move(step));
      }
      if (std::find(simultaneous_replayed.begin(), simultaneous_replayed.end(),
                    false) != simultaneous_replayed.end()) {
        throw std::runtime_error(
            "concrete replay has no simultaneous access cycle" +
            access_context);
      }
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
  ScanStep Replay(const Frame& frame, const z3::model& model,
                  const std::map<size_t, std::string>& expected,
                  bool implicit_instruction, NetworkState* replay,
                  TapExecution* tap_replay,
                  std::map<size_t, std::string>& preserved,
                  const std::map<size_t, std::string>& desired_preserved) {
    std::function<void(const NetworkState&)> check;
    if (!preserved.empty()) {
      check = [&](const NetworkState& state) {
        CheckPreserved(state, preserved, desired_preserved);
      };
    }
    size_t root = kMissing;
    for (size_t i = 0; i < frame.selected.size(); ++i) {
      if (model.eval(frame.selected[i], true).is_true()) root = roots_[i];
    }
    if (root == kMissing) {
      throw std::runtime_error("solver selected no serial output");
    }
    ScanStep step;
    if (tap_) {
      step.instruction = Decode(model, *frame.instruction) == "1";
      replay->Drive(tap_->ir_select, step.instruction ? "1" : "0");
    }
    step.output = netlist_.nodes.at(graph_[root].bit.node).name;
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
        bool active = edge.choice == kMissing;
        if (!active) {
          for (const auto& code : node.choices.at(edge.choice).codes) {
            bool matches = true;
            for (size_t i = 0; i < code.data.size(); ++i) {
              if (code.data[i] != 'x' &&
                  replay->Read(node.inputs.at("SelectedBy").at(i)) !=
                      code.data[i]) {
                matches = false;
              }
            }
            active = active || matches;
          }
        }
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
    const size_t input = graph_[vertex].bit.node;
    step.input = netlist_.nodes.at(input).name;
    std::map<size_t, ScanControl> controls;
    for (const auto& [id, suffix] : registers) {
      const auto bits = Decode(model, frame.loads.at(id));
      step.loads.emplace(netlist_.nodes[id].name, bits);
      for (auto it = bits.rbegin(); it != bits.rend(); ++it) {
        step.serial += inverted != suffix ? (*it == '1' ? '0' : '1') : *it;
      }
      std::string compare(bits.size(), 'x');
      if (frame.read.count(id) &&
          model.eval(frame.read.at(id), true).is_true()) {
        compare = expected.at(id);
        step.expected.emplace(netlist_.nodes[id].name, compare);
      } else if (step.instruction && implicit_instruction) {
        compare = tap_->bsdl.capture;
        step.expected.emplace(netlist_.nodes[id].name, compare);
      }
      for (auto it = compare.rbegin(); it != compare.rend(); ++it) {
        step.serial_expected +=
            suffix && *it != 'x' ? (*it == '1' ? '0' : '1') : *it;
      }
      controls[id] = {'1', '0', '0', '1'};
    }
    std::set<size_t> writable;
    if (tap_replay) {
      for (char tms : std::string(step.instruction ? "1100" : "100")) {
        Cycle(tap_replay, tms, '0', check);
      }
      for (size_t i = 0; i < step.serial.size(); ++i) {
        Cycle(tap_replay, i + 1 == step.serial.size() ? '1' : '0',
              step.serial[i], check);
      }
      for (const auto& [id, control] : controls) {
        const auto& node = netlist_.nodes[id];
        if (!node.reset || tap_replay->State().Read(*node.reset) == '0') {
          writable.insert(id);
        }
      }
      Cycle(tap_replay, '1', '0', check);
      Cycle(tap_replay, '0', '0', check);
      replay->Restore(tap_replay->State());
    } else {
      replay->Clock(controls, true);
      if (check) check(*replay);
      for (auto& [id, control] : controls) control = {'0', '1', '0', '1'};
      for (char bit : step.serial) {
        replay->Drive(input, std::string(1, bit));
        replay->Clock(controls, true);
        if (check) check(*replay);
      }
      for (auto& [id, control] : controls) control = {'0', '0', '1', '1'};
      for (const auto& [id, control] : controls) {
        const auto& node = netlist_.nodes[id];
        if (!node.reset || replay->Read(*node.reset) == '0') {
          writable.insert(id);
        }
      }
      replay->Clock(controls, false);
      if (check) check(*replay);
    }
    for (size_t id : writable) {
      const auto& node = netlist_.nodes[id];
      if (!node.reset || replay->Read(*node.reset) == '0') {
        step.writes.emplace(node.name, step.loads.at(node.name));
      }
    }
    // A scanned register held in reset was observed, but received no update.
    for (const auto& [name, bits] : step.loads) {
      if (!step.writes.count(name)) {
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
  const TapRuntimeModel* tap_ = nullptr;
  std::unique_ptr<TapExecution> execution_;
  bool tap_ready_ = false;
  std::string global_resets_ = "xx";
  z3::context context_;
  std::vector<size_t> registers_, roots_, order_, reset_order_;
  std::map<size_t, std::string> fill_, initial_fill_, control_masks_;
  std::map<size_t, std::set<size_t>> reset_dependencies_;
  std::set<Key> control_seen_;
  std::map<Key, size_t> ids_;
  std::vector<Vertex> graph_;
  std::map<size_t, std::vector<std::pair<BitReference, size_t>>> source_edges_;
};
ScanScheduler::ScanScheduler(const Netlist& netlist)
    : impl_(std::make_unique<Impl>(netlist)) {}
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
    const std::set<size_t>& unwritten) {
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
