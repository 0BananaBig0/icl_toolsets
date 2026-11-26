// Copyright 2026 The ICL Toolsets Authors.
// SPDX-License-Identifier: MIT
#ifndef ICL_TOOLSETS_SRC_PATTERN_PATTERN_SCHEDULER_H_
#define ICL_TOOLSETS_SRC_PATTERN_PATTERN_SCHEDULER_H_

#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <string>
#include <vector>

#include "pattern/pattern_netlist.h"
#include "pattern/pattern_runtime.h"

namespace pattern {
struct ScanLane {
  std::string input, output, serial, serial_expected;
};
struct PrimaryRead {
  BitReference bit;
  size_t route = 0;
};
struct ScanStep {
  bool instruction = false, together = false;
  std::string input, output, serial, serial_expected;
  // Simultaneous chains use lanes; scalar fields remain empty for those steps.
  std::vector<ScanLane> lanes;
  std::map<std::string, std::string> loads;
  std::map<std::string, std::string> writes, expected;
  // Selected routes when observing; not serialized as hardware or assumptions.
  std::vector<PrimaryRead> primary_reads;
};
struct RawHistoryProjection {
  std::string target, expected;
  size_t offset = 0;
  bool inverted = false;
};
// Plans serial transactions on SI/SO boundaries or common-phase TAPs, and
// writes through primary DataInPort networks or scan-driven DataRegisters.
// State assumptions are explicit PDL declarations, never inferred from
// ResetValue metadata.
// The borrowed netlist must outlive the scheduler.
class ScanScheduler {
 public:
  // Logical bits at their selected PIO frame or serial return.
  // State is committed before delivery; subscriber errors do not undo it.
  // Publish observations only after Apply returns successfully.
  using Observation =
      std::function<void(const std::vector<BitReference>&, const std::string&)>;
  using PioObservation = Observation;
  using RawObservation =
      std::function<void(const Operation&, const std::string&)>;
  explicit ScanScheduler(const Netlist& netlist);
  ScanScheduler(const ScanScheduler& other);
  ~ScanScheduler();
  bool HasOpaqueChild() const;
  // The most recent scalar raw CSU captured a dynamic route.
  bool DynamicRawHistory() const;
  // root_scan is the canonical result of the current successful raw CSU.
  // Saved dynamic results from another execution are rejected.
  std::optional<RawHistoryProjection> ModeledRawHistory(
      const Operation& note, const Operation& root_scan);
  void Reset(bool synchronous = false, bool physical = false);
  // Preserved values use 'x' for unprotected bits, '0'/'1' for held bits.
  void Idle(uint64_t cycles,
            const std::map<size_t, std::string>& preserved = {});
  void Assume(const std::string& target, const std::string& bits);
  void Assume(const std::string& target, const std::string& bits,
              const std::map<size_t, std::string>& preserved);
  // Physical mode replays one exact binary TAP CSU; writes are scan loads.
  std::vector<ScanStep> Apply(
      const std::vector<Operation>& operations, bool together = false,
      const std::map<size_t, std::string>& preserved = {},
      bool explicit_writes = true, bool physical = false,
      const PioObservation& observe = {},
      const std::map<size_t, std::string>& caller_goals = {});
  // Execute planned primary frames on actual state, validating actual routes.
  // DataRegister goal markers do not drive or assume storage.
  void ReplayPrimary(const std::vector<Operation>& operations,
                     const std::vector<ScanStep>& steps,
                     const std::map<size_t, std::string>& preserved = {},
                     const PioObservation& observe = {},
                     const std::map<size_t, std::string>& caller_goals = {});
  // Raw scans target a known root route or disjoint accessible child
  // intervals, or a known-binary IR/full-path DR on one bound root
  // client-TAP. An ordinary queue may prepare a modeled client before its
  // route/length check.
  Operation NormalizeScan(const Operation& operation,
                          bool defer_length = false);
  // Assigned reports explicitly addressed pre-CSU registers only on success.
  // A preparations output opts into child setup and receives every prefix CSU.
  Operation ApplyScan(const Operation& operation,
                      const std::map<size_t, std::string>& preserved = {},
                      std::set<size_t>* assigned = nullptr,
                      std::vector<ScanStep>* preparations = nullptr);
  Operation ApplyScans(const std::vector<Operation>& operations,
                       const std::map<size_t, std::string>& preserved = {},
                       std::set<size_t>* assigned = nullptr,
                       std::vector<ScanStep>* preparations = nullptr);
  Operation ApplyScan(const Operation& operation,
                      const std::map<size_t, std::string>& preserved,
                      std::set<size_t>* assigned,
                      std::vector<ScanStep>* preparations,
                      const RawObservation& observe);
  Operation ApplyScans(const std::vector<Operation>& operations,
                       const std::map<size_t, std::string>& preserved,
                       std::set<size_t>* assigned,
                       std::vector<ScanStep>* preparations,
                       const RawObservation& observe);
  // Preserve stable knowledge and explicit opaque-child request identity.
  std::vector<Operation> OpaqueScanNotes() const;
  void RestoreOpaqueScanNote(const Operation& note);
  // Validated unknown observation; no hardware transaction or failure recount.
  std::optional<ScanReadbackUpdate> OpaqueRawHistory(
      const Operation& note) const;
  // Canonical fields of one raw CSU; the caller emits one following Apply.
  // Parallel fields retain distinct chain names and share concrete hardware
  // state.
  std::vector<Operation> ApplyRawScans(
      const std::vector<Operation>& operations,
      const std::map<size_t, std::string>& preserved = {},
      std::set<size_t>* assigned = nullptr,
      std::vector<ScanStep>* preparations = nullptr,
      const RawObservation& observe = {});
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
