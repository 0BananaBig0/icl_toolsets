// Copyright 2026 The ICL Toolsets Authors.
// SPDX-License-Identifier: MIT
#include "pattern_scheduler.h"

#include <algorithm>
#include <chrono>
#include <functional>
#include <set>
#include <stdexcept>
#include <utility>

#include "pattern_state.h"
#include "pattern_symbolic.h"
#include "pattern_values.h"

namespace pattern {
namespace {
constexpr size_t kMissing = std::numeric_limits<size_t>::max();
constexpr size_t kMaxSteps = 32;
using Values = std::map<size_t, SymbolicValue>;
using Key = std::pair<size_t, int64_t>;
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
    // These require temporal controller/address models, not direct SI/SO scans.
    if (!netlist.taps.empty()) {
      throw std::runtime_error(
          "TAP scheduling requires controller transaction integration");
    }
    for (size_t id = 0; id < netlist.nodes.size(); ++id) {
      const auto& node = netlist.nodes[id];
      if (node.kind == "ScanRegister") {
        registers_.push_back(id);
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
  void Reset() {
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
      if (node.kind != "ScanRegister") {
        throw std::runtime_error("state assumption requires ScanRegister");
      }
      auto entry = changed.emplace(ref.node, state_.Value(ref.node)).first;
      char bit = bits[i];
      if (ref.inverted && bit != 'x') bit = bit == '1' ? '0' : '1';
      entry->second.at(Offset(node, ref.index)) = bit;
    }
    for (const auto& [id, value] : changed) {
      state_.SetRegister(id, state_.Value(id, true), value);
      for (size_t i = 0; i < value.size(); ++i) {
        if (value[i] != 'x') fill_.at(id)[i] = value[i];
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
  std::vector<ScanStep> Apply(const std::vector<Operation>& operations) {
    try {
      return Solve(operations);
    } catch (const std::exception& error) {
      std::string message = error.what();
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

 private:
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
    std::map<size_t, std::set<size_t>> dependencies;
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
    std::set<size_t> done;
    while (done.size() < registers_.size()) {
      bool progress = false;
      for (size_t id : registers_) {
        if (done.count(id)) continue;
        bool ready = true;
        for (size_t parent : dependencies[id]) {
          if (!done.count(parent)) ready = false;
        }
        if (ready) {
          done.insert(id);
          reset_order_.push_back(id);
          progress = true;
        }
      }
      if (!progress) {
        throw std::runtime_error(
            "feedback reset scheduling requires fixed-point constraints");
      }
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
    std::map<size_t, z3::expr> active, read;
    std::vector<z3::expr> selected;
  };
  std::vector<ScanStep> Solve(const std::vector<Operation>& operations) {
    if (operations.empty()) return {};
    std::map<size_t, std::string> goals, expected;
    std::set<size_t> required;
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
      const auto bits =
          operation.arguments.size() == 1
              ? std::string(target.bits.size(), 'x')
              : PdlNumber(context_, operation.arguments[1], target.bits.size());
      for (size_t i = 0; i < bits.size(); ++i) {
        const auto& ref = target.bits[i];
        if (ref.node == kMissing ||
            netlist_.nodes.at(ref.node).kind != "ScanRegister") {
          throw std::runtime_error(
              "read/write target requires a ScanRegister or its Alias");
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
    const auto start = std::chrono::steady_clock::now();
    for (size_t horizon = 1; horizon <= kMaxSteps; ++horizon) {
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
      auto cycles = context_.int_val(0), changes = context_.int_val(0);
      for (size_t time = 0; time < horizon; ++time) {
        Frame frame;
        SymbolicNetwork values(netlist_, context_, state, global_resets_);
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
              ("scan_" + std::to_string(time) + "_" + std::to_string(i))
                  .c_str());
          frame.selected.push_back(selected);
          solver.add(z3::implies(selected, viable[roots_[i]]));
          observed[roots_[i]] = observed[roots_[i]] || selected;
          count = count +
                  z3::ite(selected, context_.int_val(1), context_.int_val(0));
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
            observed[vertex.edges[e].next] = observed[vertex.edges[e].next] ||
                                             (observed[id] && gates[id][e]);
          }
        }
        Values next = state;
        for (size_t id : registers_) {
          const auto& node = netlist_.nodes[id];
          const auto active = frame.active.count(id) ? frame.active.at(id)
                                                     : context_.bool_val(false);
          if (read.count(id)) {
            frame.read.emplace(id, active && !read.at(id));
            read.at(id) = read.at(id) || active;
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
        SymbolicNetwork after(netlist_, context_, next, global_resets_);
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
      const auto result = solver.check();
      if (result == z3::unknown) {
        throw std::runtime_error(
            "scan solver resource limit; reachability not proved");
      }
      if (result == z3::unsat) continue;
      const auto model = solver.get_model();
      // Replay into a copy: a failed model must not partially change state.
      NetworkState replay(state_);
      auto fill = fill_;
      std::vector<ScanStep> steps;
      for (const auto& frame : frames) {
        std::set<size_t> reset;
        for (size_t id : registers_) {
          if (netlist_.nodes[id].reset &&
              replay.Read(*netlist_.nodes[id].reset) == '1') {
            reset.insert(id);
          }
        }
        auto step = Replay(frame, model, expected, &replay);
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
      // NetworkState borrows the immutable netlist and cannot be assigned.
      for (size_t id : registers_) {
        state_.SetRegister(id, replay.Value(id, true), replay.Value(id));
      }
      fill_ = std::move(fill);
      return steps;
    }
    throw std::runtime_error(
        "no schedule within 32 scans; reachability not proved (unknown "
        "control, reset, or blocked path)");
  }
  ScanStep Replay(const Frame& frame, const z3::model& model,
                  const std::map<size_t, std::string>& expected,
                  NetworkState* replay) {
    size_t root = kMissing;
    for (size_t i = 0; i < frame.selected.size(); ++i) {
      if (model.eval(frame.selected[i], true).is_true()) root = roots_[i];
    }
    if (root == kMissing) {
      throw std::runtime_error("solver selected no serial output");
    }
    ScanStep step;
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
      }
      for (auto it = compare.rbegin(); it != compare.rend(); ++it) {
        step.serial_expected +=
            suffix && *it != 'x' ? (*it == '1' ? '0' : '1') : *it;
      }
      controls[id] = {'1', '0', '0', '1'};
    }
    replay->Clock(controls, true);
    for (auto& [id, control] : controls) control = {'0', '1', '0', '1'};
    for (char bit : step.serial) {
      replay->Drive(input, std::string(1, bit));
      replay->Clock(controls, true);
    }
    for (auto& [id, control] : controls) control = {'0', '0', '1', '1'};
    std::set<size_t> writable;
    for (const auto& [id, control] : controls) {
      const auto& node = netlist_.nodes[id];
      if (!node.reset || replay->Read(*node.reset) == '0') writable.insert(id);
    }
    replay->Clock(controls, false);
    for (size_t id : writable) {
      const auto& node = netlist_.nodes[id];
      if (!node.reset || replay->Read(*node.reset) == '0') {
        step.writes.emplace(node.name, step.loads.at(node.name));
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
  std::string global_resets_ = "xx";
  z3::context context_;
  std::vector<size_t> registers_, roots_, order_, reset_order_;
  std::map<size_t, std::string> fill_, initial_fill_, control_masks_;
  std::set<Key> control_seen_;
  std::map<Key, size_t> ids_;
  std::vector<Vertex> graph_;
  std::map<size_t, std::vector<std::pair<BitReference, size_t>>> source_edges_;
};
ScanScheduler::ScanScheduler(const Netlist& netlist)
    : impl_(std::make_unique<Impl>(netlist)) {}
ScanScheduler::~ScanScheduler() = default;
void ScanScheduler::Reset() { impl_->Reset(); }
void ScanScheduler::Assume(const std::string& target, const std::string& bits) {
  impl_->Assume(target, bits);
}
std::vector<ScanStep> ScanScheduler::Apply(
    const std::vector<Operation>& operations) {
  return impl_->Apply(operations);
}
std::string ScanScheduler::Value(const std::string& target) const {
  return impl_->Value(target);
}
}  // namespace pattern
