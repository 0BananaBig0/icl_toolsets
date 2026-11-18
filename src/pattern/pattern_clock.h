// Copyright 2026 The ICL Toolsets Authors.
// SPDX-License-Identifier: MIT
#ifndef ICL_TOOLSETS_SRC_PATTERN_PATTERN_CLOCK_H_
#define ICL_TOOLSETS_SRC_PATTERN_PATTERN_CLOCK_H_

#include <cstdint>
#include <map>
#include <optional>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "pattern/pattern_scheduler.h"
#include "pattern/pattern_values.h"

namespace pattern {
// Relative clock duration only: accepted roots cannot drive modeled
// data/events. The immutable netlist must outlive this copyable checked-clock
// state.
class SystemClocks {
 public:
  explicit SystemClocks(const Netlist& netlist) : netlist_(netlist) {}
  void Override(const Operation& operation, z3::context& context);
  std::string Check(const std::string& target, const ScanScheduler& scheduler);
  std::pair<std::string, uint64_t> Wait(
      const std::string& target, uint64_t cycles,
      const ScanScheduler& scheduler,
      const std::map<size_t, std::string>& preserved = {});
  void CheckTimeWait(const std::map<size_t, std::string>& preserved = {});
  // Call after changing applied hardware state, not after merely queueing
  // writes.
  void Invalidate();

 private:
  using Bit = std::pair<size_t, int64_t>;
  struct Route {
    Bit root, phase_origin;
    bool inverted = false;
    uint64_t multiplier = 1, divider = 1;
  };
  struct Checked {
    Route route;
    size_t version = 0;
  };
  struct Definition {
    size_t version = 0;
    BitReference source;
    uint64_t multiplier = 1, divider = 1;
  };
  const Definition* DefinitionAt(Bit bit, size_t version) const;
  void CheckCycle(Bit target, BitReference source) const;
  Bit Target(const std::string& name) const;
  std::string ClockTargetName(const std::string& name) const;
  std::string Name(Bit bit) const;
  Route Resolve(Bit bit, const ScanScheduler& scheduler, size_t version);
  BitReference Source(Bit bit, const ScanScheduler& scheduler, size_t version);
  void BuildSafety();
  const Netlist& netlist_;
  mutable std::optional<bool> flat_origins_;
  bool safety_ready_ = false;
  bool time_ready_ = false;
  std::set<Bit> unsafe_;
  std::map<Bit, std::set<Bit>> fanout_;
  size_t version_ = 0, cache_version_ = 0;
  std::map<Bit, std::vector<Definition>> overrides_;
  std::map<Bit, Checked> checked_;
  std::map<Bit, Route> routes_;
  std::map<size_t, std::string> values_;
};
}  // namespace pattern
#endif  // ICL_TOOLSETS_SRC_PATTERN_PATTERN_CLOCK_H_
