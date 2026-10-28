// Copyright 2026 The ICL Toolsets Authors.
// SPDX-License-Identifier: MIT
#ifndef ICL_TOOLSETS_SRC_PATTERN_PATTERN_DATA_H_
#define ICL_TOOLSETS_SRC_PATTERN_PATTERN_DATA_H_

#include <map>
#include <memory>
#include <set>
#include <vector>

#include "pattern/pattern_address.h"

namespace pattern {
// Immutable event dependencies shared by concrete replay and symbolic planning.
// A null address signal list denotes an ordinary selectable register.
struct DataRegisterPlan {
  explicit DataRegisterPlan(const Netlist& netlist);
  std::map<size_t, AddressRequirement> writes;
  std::vector<size_t> order;
  std::map<size_t, std::set<size_t>> primary_inputs;
};
struct DataReadGuard {
  size_t node;
  // A missing choice denotes the node's explicit output Enable.
  size_t choice = std::numeric_limits<size_t>::max();
};
struct DataReadRoute {
  BitReference output;
  std::vector<DataReadGuard> guards;
};
// Bounded structural paths from primary outputs through forwarding/mux/groups.
class DataReadRoutes {
 public:
  explicit DataReadRoutes(const Netlist& netlist);
  std::map<std::pair<size_t, int64_t>, std::vector<DataReadRoute>> routes;
  std::map<std::pair<size_t, int64_t>, std::set<size_t>> storage;
};
// Resolves selection separately from observed data, including addressed
// Register/Instance contributors and forwarded DataOutPorts.
class OneHotReadResolver {
 public:
  explicit OneHotReadResolver(const Netlist& netlist) : netlist_(netlist) {}
  const std::vector<AddressRequirement>& Resolve(const OneHotSource& source);

 private:
  const Netlist& netlist_;
  std::unique_ptr<AddressResolver> addresses_;
  std::map<const OneHotSource*, std::vector<AddressRequirement>> cache_;
};
}  // namespace pattern
#endif  // ICL_TOOLSETS_SRC_PATTERN_PATTERN_DATA_H_
