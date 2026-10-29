// Copyright 2026 The ICL Toolsets Authors.
// SPDX-License-Identifier: MIT
#ifndef ICL_TOOLSETS_SRC_PATTERN_PATTERN_SYMBOLIC_H_
#define ICL_TOOLSETS_SRC_PATTERN_PATTERN_SYMBOLIC_H_

#include <z3++.h>

#include <map>
#include <set>
#include <string>

#include "pattern/pattern_address.h"
#include "pattern/pattern_data.h"
#include "pattern/pattern_netlist.h"

namespace pattern {
struct SymbolicValue {
  z3::expr bits;
  z3::expr known;
  static SymbolicValue Literal(z3::context& context, const std::string& value);
};
// Combinational data-state view for scheduling constraints. Register values are
// held values; an optional separate shift-state view supports serial events.
// Unassigned terminals are unknown, never existentially free hardware state.
class SymbolicNetwork {
 public:
  SymbolicNetwork(const Netlist& netlist, z3::context& context,
                  std::map<size_t, SymbolicValue> assignments,
                  std::string global_resets = "xx",
                  const std::map<size_t, SymbolicValue>* shifts = nullptr);
  SymbolicValue Value(size_t node, bool scan = false);
  SymbolicValue Read(const BitReference& bit, bool scan = false);

  SymbolicValue Bits(const std::vector<BitReference>& bits, bool scan = false);
  SymbolicValue Active(const AddressRequirement& requirement);
  SymbolicValue OneHotActive(const OneHotSource& source);

 private:
  SymbolicValue Compute(size_t node, bool scan);
  SymbolicValue Logic(const NetlistNode& node);
  const Netlist& netlist_;
  z3::context& context_;
  using Key = std::pair<size_t, bool>;
  std::map<Key, SymbolicValue> cache_;
  std::set<Key> visiting_;
  bool has_shift_state_ = false;
  std::string global_resets_;
  OneHotReadResolver one_hot_;
};
}  // namespace pattern
#endif  // ICL_TOOLSETS_SRC_PATTERN_PATTERN_SYMBOLIC_H_
