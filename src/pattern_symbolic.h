// Copyright 2026 The ICL Toolsets Authors.
// SPDX-License-Identifier: MIT
#ifndef ICL_TOOLSETS_SRC_PATTERN_SYMBOLIC_H_
#define ICL_TOOLSETS_SRC_PATTERN_SYMBOLIC_H_

#include <z3++.h>

#include <map>
#include <set>
#include <string>

#include "pattern_netlist.h"

namespace pattern {
struct SymbolicValue {
  z3::expr bits;
  z3::expr known;
  static SymbolicValue Literal(z3::context& context, const std::string& value);
};
// Combinational data-state view for scheduling constraints. Register values are
// update-stage values; scan traversal and temporal events remain separate.
// Unassigned terminals are unknown, never existentially free hardware state.
class SymbolicNetwork {
 public:
  SymbolicNetwork(const Netlist& netlist, z3::context& context,
                  std::map<size_t, SymbolicValue> assignments,
                  std::string global_resets = "xx");
  SymbolicValue Value(size_t node);
  SymbolicValue Read(const BitReference& bit);

 private:
  SymbolicValue Bits(const std::vector<BitReference>& bits);
  SymbolicValue Compute(size_t node);
  SymbolicValue Logic(const NetlistNode& node);
  const Netlist& netlist_;
  z3::context& context_;
  std::map<size_t, SymbolicValue> cache_;
  std::set<size_t> visiting_;
  std::string global_resets_;
};
}  // namespace pattern
#endif  // ICL_TOOLSETS_SRC_PATTERN_SYMBOLIC_H_
