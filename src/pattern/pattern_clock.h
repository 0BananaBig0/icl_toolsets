// Copyright 2026 The ICL Toolsets Authors.
// SPDX-License-Identifier: MIT
#ifndef ICL_TOOLSETS_SRC_PATTERN_PATTERN_CLOCK_H_
#define ICL_TOOLSETS_SRC_PATTERN_PATTERN_CLOCK_H_

#include <cstdint>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "pattern/pattern_scheduler.h"

namespace pattern {
// Relative clock duration only: accepted roots cannot drive modeled
// data/events. The immutable netlist must outlive this copyable checked-clock
// state.
class SystemClocks {
 public:
  explicit SystemClocks(const Netlist& netlist) : netlist_(netlist) {}
  std::string Check(const std::string& target, const ScanScheduler& scheduler);
  std::pair<std::string, uint64_t> Wait(
      const std::string& target, uint64_t cycles,
      const ScanScheduler& scheduler,
      const std::map<size_t, std::string>& preserved = {});
  // Call after changing applied hardware state, not after merely queueing
  // writes.
  void Invalidate();

 private:
  using Bit = std::pair<size_t, int64_t>;
  struct Route {
    Bit root;
    uint64_t multiplier = 1, divider = 1;
  };
  Bit Target(const std::string& name) const;
  std::string Name(Bit bit) const;
  Route Resolve(Bit bit, const ScanScheduler& scheduler);
  BitReference Source(Bit bit, const ScanScheduler& scheduler);
  void BuildSafety();
  const Netlist& netlist_;
  bool safety_ready_ = false;
  std::set<Bit> unsafe_;
  std::map<Bit, std::vector<Bit>> fanout_;
  std::map<Bit, Route> checked_, routes_;
  std::map<size_t, std::string> values_;
};
}  // namespace pattern
#endif  // ICL_TOOLSETS_SRC_PATTERN_PATTERN_CLOCK_H_
