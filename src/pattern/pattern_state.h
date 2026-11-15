// Copyright 2026 The ICL Toolsets Authors.
// SPDX-License-Identifier: MIT
#ifndef ICL_TOOLSETS_SRC_PATTERN_PATTERN_STATE_H_
#define ICL_TOOLSETS_SRC_PATTERN_PATTERN_STATE_H_

#include <map>
#include <memory>
#include <optional>
#include <set>
#include <string>
#include <vector>

#include "pattern/pattern_netlist.h"

namespace pattern {
struct ScanControl {
  char capture = '0', shift = '0', update = '0', select = '0';
};
class SignalEvaluator;
struct DataRegisterPlan;
// State points accepted by iState, including observation history.
void ValidateStateTarget(const Netlist& netlist, size_t node);
// Hardware assumptions exclude controller-owned instruction storage.
void ValidateStateAssumption(const Netlist& netlist, size_t node);
// Netlist must outlive this state and remain immutable. Values are MSB/leftmost
// first, independent of numeric index direction. No initial reset is assumed.
class NetworkState {
 public:
  using ControlEvents = std::map<size_t, std::optional<std::string>>;
  explicit NetworkState(const Netlist& netlist);
  void Drive(size_t node, const std::string& value);
  // Update the assumed snapshot without clock edges or reset propagation.
  void Assume(size_t node, const std::string& value);
  std::string Value(size_t node, bool scan = false) const;
  char Read(const BitReference& bit, bool scan = false) const;
  void Clock(const std::map<size_t, ScanControl>& controls, bool rising);
  // Fast-forward a caller-proven complete chain overwrite. Controls/resets must
  // remain invariant with no omitted side effects. All local preconditions are
  // checked before mutation; update storage changes only at its later edge.
  void ShiftComplete(const std::map<size_t, std::string>& loads,
                     const std::map<size_t, ScanControl>& controls);
  // A parallel apply changes all primary pins simultaneously, then settles
  // data-register updates and resets. Failure leaves the snapshot unchanged.
  void ApplyInputs(const std::map<size_t, std::string>& values);
  const DataRegisterPlan& DataRegisters() const;
  void Reset();
  void Restore(const NetworkState& state);
  bool SameState(const NetworkState& state) const;
  // Drain known reset assertions, including pulses during reset propagation.
  std::set<size_t> TakeResetEvents();
  // Last definite event per storage: known write payload or nullopt for reset.
  // Unknown writes/resets do not replace two-state controlling history.
  ControlEvents TakeControlEvents();
  bool IsDriven(size_t node) const;
  void SetGlobalReset(char synchronous, char asynchronous);
  // Controller-owned state, not a user write or an implicit initialization.
  void SetRegister(size_t node, const std::string& shift,
                   const std::string& update);

 private:
  friend class SignalEvaluator;
  void ClockRegisters(const std::map<size_t, ScanControl>& controls,
                      bool rising);
  void UpdateDataRegisters(char event = '1');
  mutable std::shared_ptr<const DataRegisterPlan> data_registers_;
  bool has_data_registers_ = false;
  const Netlist& netlist_;
  std::vector<std::string> update_, shift_, driven_;
  std::set<size_t> updated_;
  std::set<size_t> reset_events_;
  ControlEvents control_events_;
  char global_reset_ = 'x', async_reset_ = 'x';
};
}  // namespace pattern
#endif  // ICL_TOOLSETS_SRC_PATTERN_PATTERN_STATE_H_
