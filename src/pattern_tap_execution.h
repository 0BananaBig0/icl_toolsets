// Copyright 2026 The ICL Toolsets Authors.
// SPDX-License-Identifier: MIT
#ifndef ICL_TOOLSETS_SRC_PATTERN_TAP_EXECUTION_H_
#define ICL_TOOLSETS_SRC_PATTERN_TAP_EXECUTION_H_

#include "pattern_state.h"
#include "pattern_tap.h"

namespace pattern {
// Observed pin events execute all TAPs sharing that clock. The immutable
// netlist must outlive the executor. This is hardware replay, not pattern
// scheduling.
class TapExecution {
 public:
  using InterfacePorts =
      std::map<std::string, std::set<std::pair<size_t, int64_t>>>;
  const NetworkState& State() const { return state_; }
  std::set<size_t> TakeResetEvents() { return state_.TakeResetEvents(); }
  const std::map<size_t, std::vector<InterfacePorts>>& RegisterInterfaces()
      const {
    return register_ports_;
  }
  void Assume(size_t node, const std::string& value);
  explicit TapExecution(const Netlist& netlist);
  void Drive(const std::string& port, const std::string& value);
  void RisingEdge(const std::string& clock);
  void FallingEdge(const std::string& clock);
  std::string Value(const std::string& path, bool scan = false) const;

 private:
  void Edge(const std::string& clock, bool rising);
  void GlobalReset();
  void Synchronize(const std::string& name);
  void Trace(BitReference output, char selected, std::map<size_t, char>* active,
             bool all = false,
             const std::set<std::pair<size_t, int64_t>>& stops = {}) const;
  std::map<size_t, char> Active(const TapRuntimeModel& tap) const;
  std::map<size_t, ScanControl> Controls(
      const TapRuntimeModel& tap, const TapActions& actions,
      const std::map<std::pair<size_t, int64_t>, char>& clocks) const;
  const Netlist& netlist_;
  NetworkState state_;
  std::map<std::string, TapInstructionRegister> controllers_;
  std::map<size_t, std::set<size_t>> owned_registers_;
  std::map<size_t, std::vector<InterfacePorts>> register_ports_;
};
}  // namespace pattern
#endif  // ICL_TOOLSETS_SRC_PATTERN_TAP_EXECUTION_H_
