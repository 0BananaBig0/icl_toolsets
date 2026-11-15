// Copyright 2026 The ICL Toolsets Authors.
// SPDX-License-Identifier: MIT
#ifndef ICL_TOOLSETS_SRC_PATTERN_PATTERN_SCHEDULER_H_
#define ICL_TOOLSETS_SRC_PATTERN_PATTERN_SCHEDULER_H_

#include <cstdint>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

#include "pattern/pattern_netlist.h"
#include "pattern/pattern_runtime.h"

namespace pattern {
struct ScanLane {
  std::string input, output, serial, serial_expected;
};
struct ScanStep {
  bool instruction = false, together = false;
  std::string input, output, serial, serial_expected;
  // Simultaneous chains use lanes; scalar fields remain empty for those steps.
  std::vector<ScanLane> lanes;
  std::map<std::string, std::string> loads;
  std::map<std::string, std::string> writes, expected;
};
// Plans serial transactions on SI/SO boundaries or common-phase TAPs, and
// writes through primary DataInPort networks or scan-driven DataRegisters.
// State assumptions are explicit PDL declarations, never inferred from
// ResetValue metadata.
// The borrowed netlist must outlive the scheduler.
class ScanScheduler {
 public:
  explicit ScanScheduler(const Netlist& netlist);
  ScanScheduler(const ScanScheduler& other);
  ~ScanScheduler();
  void Reset(bool synchronous = false);
  // Preserved values use 'x' for unprotected bits, '0'/'1' for held bits.
  void Idle(uint64_t cycles,
            const std::map<size_t, std::string>& preserved = {});
  void Assume(const std::string& target, const std::string& bits);
  std::vector<ScanStep> Apply(
      const std::vector<Operation>& operations, bool together = false,
      const std::map<size_t, std::string>& preserved = {},
      bool explicit_writes = true);
  // Raw scans target a known root route or disjoint accessible child
  // intervals.
  // An ordinary queue may prepare the root route before its length is checked.
  Operation NormalizeScan(const Operation& operation,
                          bool defer_root_length = false);
  // Assigned reports explicitly addressed pre-CSU registers only on success.
  Operation ApplyScan(const Operation& operation,
                      const std::map<size_t, std::string>& preserved = {},
                      std::set<size_t>* assigned = nullptr);
  Operation ApplyScans(const std::vector<Operation>& operations,
                       const std::map<size_t, std::string>& preserved = {},
                       std::set<size_t>* assigned = nullptr);
  std::string Value(const std::string& target) const;
  // Reset-only control writes, grouped by dependency depth; no SIB setup.
  // Unwritten masks use '1' for taken bits that must not be assigned.
  std::vector<std::vector<Operation>> ResetPrerequisites(
      const std::vector<Operation>& operations,
      const std::map<size_t, std::string>& preserved = {},
      const std::map<size_t, std::string>& unwritten = {});
  void CheckLocalResets(const std::vector<Operation>& operations) const;

 private:
  class Impl;
  std::unique_ptr<Impl> impl_;
};
}  // namespace pattern
#endif  // ICL_TOOLSETS_SRC_PATTERN_PATTERN_SCHEDULER_H_
