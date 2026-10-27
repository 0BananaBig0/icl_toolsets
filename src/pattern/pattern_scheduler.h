// Copyright 2026 The ICL Toolsets Authors.
// SPDX-License-Identifier: MIT
#ifndef ICL_TOOLSETS_SRC_PATTERN_PATTERN_SCHEDULER_H_
#define ICL_TOOLSETS_SRC_PATTERN_PATTERN_SCHEDULER_H_

#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "pattern/pattern_netlist.h"
#include "pattern/pattern_runtime.h"

namespace pattern {
struct ScanStep {
  bool instruction = false, together = false;
  std::string input, output, serial, serial_expected;
  std::map<std::string, std::string> loads;
  std::map<std::string, std::string> writes, expected;
};
// Plans serial transactions on SI/SO boundaries or a single TAP, and writes
// through purely combinational primary DataInPort networks.
// State assumptions are explicit PDL declarations, never inferred from
// ResetValue metadata.
// The borrowed netlist must outlive the scheduler.
class ScanScheduler {
 public:
  explicit ScanScheduler(const Netlist& netlist);
  ~ScanScheduler();
  void Reset(bool synchronous = false);
  void Idle(uint64_t cycles,
            const std::map<size_t, std::string>& preserved = {});
  void Assume(const std::string& target, const std::string& bits);
  std::vector<ScanStep> Apply(
      const std::vector<Operation>& operations, bool together = false,
      const std::map<size_t, std::string>& preserved = {},
      bool explicit_writes = true);
  std::string Value(const std::string& target) const;
  // Reset-only control writes, grouped by dependency depth; no SIB setup.
  std::vector<std::vector<Operation>> ResetPrerequisites(
      const std::vector<Operation>& operations);
  void CheckLocalResets(const std::vector<Operation>& operations) const;

 private:
  class Impl;
  std::unique_ptr<Impl> impl_;
};
}  // namespace pattern
#endif  // ICL_TOOLSETS_SRC_PATTERN_PATTERN_SCHEDULER_H_
