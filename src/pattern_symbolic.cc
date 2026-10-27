// Copyright 2026 The ICL Toolsets Authors.
// SPDX-License-Identifier: MIT
#include "pattern_symbolic.h"

#include <algorithm>
#include <memory>
#include <stdexcept>
#include <utility>

namespace pattern {
namespace {
constexpr size_t kMissing = std::numeric_limits<size_t>::max();
constexpr size_t kMaxBits = 4 * 1024 * 1024;
z3::expr BoolBit(const z3::expr& value) {
  return z3::ite(value, value.ctx().bv_val(1, 1), value.ctx().bv_val(0, 1));
}
SymbolicValue Invert(const SymbolicValue& value) {
  return {~value.bits & value.known, value.known};
}
SymbolicValue Concat(const SymbolicValue& a, const SymbolicValue& b) {
  return {z3::concat(a.bits, b.bits), z3::concat(a.known, b.known)};
}
SymbolicValue Select(const z3::expr& condition, const SymbolicValue& yes,
                     const SymbolicValue& no) {
  return {z3::ite(condition, yes.bits, no.bits),
          z3::ite(condition, yes.known, no.known)};
}
SymbolicValue Join(const SymbolicValue& a, const SymbolicValue& b) {
  const auto known = a.known & b.known & ~(a.bits ^ b.bits);
  return {a.bits & known, known};
}
}  // namespace
SymbolicValue SymbolicValue::Literal(z3::context& context,
                                     const std::string& value) {
  if (value.empty() || value.size() > kMaxBits ||
      value.find_first_not_of("01x") != std::string::npos) {
    throw std::runtime_error(
        "symbolic literal requires bounded nonempty 0/1/x bits");
  }
  auto bits = std::make_unique<bool[]>(value.size());
  auto known = std::make_unique<bool[]>(value.size());
  for (size_t i = 0; i < value.size(); ++i) {
    bits[i] = value[value.size() - 1 - i] == '1';
    known[i] = value[value.size() - 1 - i] != 'x';
  }
  return {context.bv_val(static_cast<unsigned>(value.size()), bits.get()),
          context.bv_val(static_cast<unsigned>(value.size()), known.get())};
}
SymbolicNetwork::SymbolicNetwork(const Netlist& netlist, z3::context& context,
                                 std::map<size_t, SymbolicValue> assignments,
                                 std::string global_resets)
    : netlist_(netlist),
      context_(context),
      global_resets_(std::move(global_resets)) {
  if (global_resets_.size() != 2 ||
      global_resets_.find_first_not_of("01x") != std::string::npos) {
    throw std::runtime_error("invalid symbolic global reset state");
  }
  size_t remaining = kMaxBits;
  for (const auto& node : netlist.nodes) {
    if (node.Width() > remaining) {
      throw std::runtime_error("symbolic network bit limit exceeded");
    }
    remaining -= node.Width();
  }
  for (auto& [id, value] : assignments) {
    const auto& node = netlist.nodes.at(id);
    if (&value.bits.ctx() != &context || &value.known.ctx() != &context ||
        !value.bits.is_bv() || !value.known.is_bv() ||
        value.bits.get_sort().bv_size() != node.Width() ||
        value.known.get_sort().bv_size() != node.Width() ||
        (node.model != ModelKind::kRegister &&
         (!IsInputPort(node.kind) || node.inputs.count("Source")))) {
      throw std::runtime_error(node.origin +
                               ": invalid symbolic state assignment");
    }
    cache_.emplace(id, SymbolicValue{value.bits & value.known, value.known});
  }
}
SymbolicValue SymbolicNetwork::Read(const BitReference& bit) {
  auto result = SymbolicValue::Literal(context_, "x");
  if (bit.node == kMissing) {
    if (bit.virtual_signal == VirtualSignal::kNone && bit.constant != '?') {
      result = SymbolicValue::Literal(context_, std::string(1, bit.constant));
    } else if (bit.virtual_signal != VirtualSignal::kNone) {
      result = SymbolicValue::Literal(
          context_,
          std::string(
              1,
              global_resets_[bit.virtual_signal == VirtualSignal::kGlobalReset
                                 ? 0
                                 : 1]));
    }
  } else {
    const auto& node = netlist_.nodes.at(bit.node);
    if (bit.index < std::min(node.left, node.right) ||
        bit.index > std::max(node.left, node.right)) {
      throw std::runtime_error("symbolic reference index outside range");
    }
    const unsigned offset = node.left >= node.right ? bit.index - node.right
                                                    : node.right - bit.index;
    const auto source = Value(bit.node);
    result = {source.bits.extract(offset, offset),
              source.known.extract(offset, offset)};
  }
  return bit.inverted ? Invert(result) : result;
}
SymbolicValue SymbolicNetwork::Bits(const std::vector<BitReference>& bits) {
  if (bits.empty()) throw std::runtime_error("empty symbolic signal");
  auto result = Read(bits[0]);
  for (size_t i = 1; i < bits.size(); ++i) {
    result = Concat(result, Read(bits[i]));
  }
  return result;
}
SymbolicValue SymbolicNetwork::Value(size_t node) {
  if (cache_.count(node)) return cache_.at(node);
  std::vector<std::pair<size_t, bool>> stack{{node, false}};
  while (!stack.empty()) {
    const auto [id, expanded] = stack.back();
    stack.pop_back();
    if (cache_.count(id)) continue;
    const auto& object = netlist_.nodes.at(id);
    if (expanded) {
      cache_.emplace(id, Compute(id));
      visiting_.erase(id);
      continue;
    }
    if (!visiting_.insert(id).second) {
      throw std::runtime_error(object.origin +
                               ": symbolic combinational cycle");
    }
    stack.emplace_back(id, true);
    if (object.model == ModelKind::kRegister) continue;
    std::set<size_t> dependencies;
    const auto add = [&](const BitReference& bit) {
      if (bit.node != kMissing) dependencies.insert(bit.node);
    };
    for (const auto& [name, bits] : object.inputs) {
      for (const auto& bit : bits) add(bit);
    }
    for (const auto& choice : object.choices) {
      for (const auto& bit : choice.source) add(bit);
    }
    for (const auto& source : object.one_hot_sources) {
      for (const auto& bit : source.bits) add(bit);
    }
    for (const auto& term : object.logic) {
      for (const auto& bit : term.signals) add(bit);
    }
    for (size_t dependency : dependencies) {
      if (!cache_.count(dependency)) stack.emplace_back(dependency, false);
    }
  }
  return cache_.at(node);
}
SymbolicValue SymbolicNetwork::Logic(const NetlistNode& node) {
  std::vector<SymbolicValue> values;
  for (const auto& term : node.logic) {
    if (term.operation == LogicOperation::kLiteral) {
      values.push_back(SymbolicValue::Literal(context_, term.literal.data));
      continue;
    }
    if (term.operation == LogicOperation::kSignal) {
      values.push_back(Bits(term.signals));
      continue;
    }
    const auto a = values.at(term.left);
    const auto zero = context_.bv_val(0, a.bits.get_sort().bv_size());
    const auto ones = ~zero;
    if (term.operation == LogicOperation::kInvert ||
        term.operation == LogicOperation::kNot) {
      values.push_back(Invert(a));
      continue;
    }
    if (term.operation == LogicOperation::kReduceAnd) {
      const auto all_known = a.known == ones;
      values.push_back({BoolBit(all_known && a.bits == ones),
                        BoolBit(all_known || (a.known & ~a.bits) != zero)});
      continue;
    }
    if (term.operation == LogicOperation::kReduceOr) {
      const auto any = (a.bits & a.known) != zero;
      values.push_back({BoolBit(any), BoolBit(any || a.known == ones)});
      continue;
    }
    if (term.operation == LogicOperation::kReduceXor) {
      auto parity = a.bits.extract(0, 0);
      for (unsigned i = 1; i < a.bits.get_sort().bv_size(); ++i) {
        parity = parity ^ a.bits.extract(i, i);
      }
      const auto known = BoolBit(a.known == ones);
      values.push_back({parity & known, known});
      continue;
    }
    const auto b = values.at(term.right);
    if (term.operation == LogicOperation::kConcat) {
      values.push_back(Concat(a, b));
    } else if (term.operation == LogicOperation::kEqual ||
               term.operation == LogicOperation::kNotEqual) {
      const auto different = ((a.bits ^ b.bits) & a.known & b.known) != zero;
      const auto missing = (b.known & ~a.known) != zero;
      auto equal = SymbolicValue{BoolBit(!different && !missing),
                                 BoolBit(different || !missing)};
      values.push_back(
          term.operation == LogicOperation::kEqual ? equal : Invert(equal));
    } else if (term.operation == LogicOperation::kAnd) {
      const auto known =
          (a.known & b.known) | (a.known & ~a.bits) | (b.known & ~b.bits);
      values.push_back({a.bits & b.bits & known, known});
    } else if (term.operation == LogicOperation::kOr) {
      const auto known =
          (a.known & b.known) | (a.known & a.bits) | (b.known & b.bits);
      values.push_back({(a.bits | b.bits) & known, known});
    } else if (term.operation == LogicOperation::kXor) {
      const auto known = a.known & b.known;
      values.push_back({(a.bits ^ b.bits) & known, known});
    } else {
      throw std::runtime_error("unsupported symbolic logic instruction");
    }
  }
  if (values.empty() || values.back().bits.get_sort().bv_size() != 1) {
    throw std::runtime_error("invalid symbolic LogicSignal");
  }
  return values.back();
}
SymbolicValue SymbolicNetwork::Compute(size_t id) {
  const auto& node = netlist_.nodes.at(id);
  const auto unknown =
      SymbolicValue::Literal(context_, std::string(node.Width(), 'x'));
  if (node.model == ModelKind::kRegister) return unknown;
  if (node.model == ModelKind::kLogic) return Logic(node);
  auto result = unknown;
  if (node.model == ModelKind::kMux) {
    const auto selector = Bits(node.inputs.at("SelectedBy"));
    const auto zero = context_.bv_val(0, selector.bits.get_sort().bv_size());
    const auto defined = selector.known == ~zero;
    auto found = context_.bool_val(false);
    for (const auto& choice : node.choices) {
      auto match = context_.bool_val(false);
      for (const auto& code : choice.codes) {
        const auto pattern = SymbolicValue::Literal(context_, code.data);
        match =
            match || ((selector.bits ^ pattern.bits) & pattern.known) == zero;
      }
      const auto next = Bits(choice.source);
      result = Select(match, Select(found, Join(result, next), next), result);
      found = found || match;
    }
    result = Select(defined, result, unknown);
  } else if (node.model == ModelKind::kGroup) {
    if (node.kind != "OneHotDataGroup") {
      throw std::runtime_error(
          node.origin + ": symbolic one-hot scan execution is not implemented");
    }
    auto found = context_.bool_val(false), uncertain = context_.bool_val(false);
    for (size_t source = 0; source < node.one_hot_sources.size(); ++source) {
      const auto enabled =
          Read(netlist_.OneHotPortEnable(node.one_hot_sources[source]));
      const auto known = enabled.known == context_.bv_val(1, 1);
      const auto active = known && enabled.bits == context_.bv_val(1, 1);
      uncertain = uncertain || !known || (found && active);
      auto next = Read(node.OneHotSourceBit(source, 0));
      for (size_t bit = 1; bit < node.Width(); ++bit) {
        next = Concat(next, Read(node.OneHotSourceBit(source, bit)));
      }
      result = Select(active, next, result);
      found = found || active;
    }
    result = Select(found && !uncertain, result, unknown);
  } else {
    const auto source = node.inputs.find("Source");
    if (source != node.inputs.end()) result = Bits(source->second);
  }
  const auto enable = node.inputs.find("Enable");
  if (enable != node.inputs.end()) {
    const auto active = Read(enable->second.at(0));
    result = Select(active.known == context_.bv_val(1, 1) &&
                        active.bits == context_.bv_val(1, 1),
                    result, unknown);
  }
  return result;
}
}  // namespace pattern
