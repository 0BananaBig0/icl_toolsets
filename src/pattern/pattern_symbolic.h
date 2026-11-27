// Copyright 2026 The ICL Toolsets Authors.
// SPDX-License-Identifier: MIT
#ifndef ICL_TOOLSETS_SRC_PATTERN_PATTERN_SYMBOLIC_H_
#define ICL_TOOLSETS_SRC_PATTERN_PATTERN_SYMBOLIC_H_

#include <z3++.h>

#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

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
  // An observation view can retain capture-time port availability separately
  // from its original storage. The borrowed access context must stay valid.
  SymbolicNetwork(const Netlist& netlist, z3::context& context,
                  std::map<size_t, SymbolicValue> assignments,
                  std::string global_resets = "xx",
                  const std::map<size_t, SymbolicValue>* shifts = nullptr,
                  SymbolicNetwork* access_context = nullptr);
  SymbolicValue Value(size_t node, bool scan = false);
  SymbolicValue Read(const BitReference& bit, bool scan = false);

  SymbolicValue Bits(const std::vector<BitReference>& bits, bool scan = false);
  SymbolicValue Active(const AddressRequirement& requirement);
  SymbolicValue OneHotActive(const OneHotSource& source);
  // Update only storage/input assignments; shift snapshots remain independent.
  // Validate the complete batch before invalidating derived values.
  void Assign(const std::map<size_t, SymbolicValue>& assignments);

 private:
  void CheckAssignment(size_t node, const SymbolicValue& value) const;
  SymbolicValue Compute(size_t node, bool scan);
  SymbolicValue Logic(const NetlistNode& node);
  const Netlist& netlist_;
  z3::context& context_;
  using Key = std::pair<size_t, bool>;
  std::map<Key, SymbolicValue> cache_;
  std::set<Key> visiting_;
  bool track_derived_ = false;
  std::vector<Key> derived_;
  bool has_shift_state_ = false;
  std::string global_resets_;
  SymbolicNetwork* access_context_;
  std::map<size_t, SymbolicValue> observation_assignments_;
  std::unique_ptr<SymbolicNetwork> observation_payloads_;
  OneHotReadResolver one_hot_;
};
}  // namespace pattern
#endif  // ICL_TOOLSETS_SRC_PATTERN_PATTERN_SYMBOLIC_H_
