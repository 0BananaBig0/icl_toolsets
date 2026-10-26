// Copyright 2026 The ICL Toolsets Authors.
// SPDX-License-Identifier: MIT
#ifndef ICL_TOOLSETS_SRC_PATTERN_STATE_H_
#define ICL_TOOLSETS_SRC_PATTERN_STATE_H_

#include <map>
#include <string>
#include <vector>

#include "pattern_netlist.h"

namespace pattern {
struct ScanControl {
  char capture = '0', shift = '0', update = '0', select = '0';
};
class SignalEvaluator;
// Netlist must outlive this state and remain immutable. Values are MSB/leftmost
// first, independent of numeric index direction. No initial reset is assumed.
class NetworkState {
 public:
  explicit NetworkState(const Netlist& netlist);
  void Drive(size_t node, const std::string& value);
  std::string Value(size_t node, bool scan = false) const;
  char Read(const BitReference& bit, bool scan = false) const;
  void Clock(const std::map<size_t, ScanControl>& controls, bool rising);
  void Reset();
  void Restore(const NetworkState& state);
  bool IsDriven(size_t node) const;
  void SetGlobalReset(char synchronous, char asynchronous);
  // Controller-owned state, not a user write or an implicit initialization.
  void SetRegister(size_t node, const std::string& shift,
                   const std::string& update);

 private:
  friend class SignalEvaluator;
  const Netlist& netlist_;
  std::vector<std::string> update_, shift_, driven_;
  std::set<size_t> updated_;
  char global_reset_ = 'x', async_reset_ = 'x';
};
}  // namespace pattern
#endif  // ICL_TOOLSETS_SRC_PATTERN_STATE_H_
