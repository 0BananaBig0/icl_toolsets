// Copyright 2026 The ICL Toolsets Authors.
// SPDX-License-Identifier: MIT
#ifndef ICL_TOOLSETS_SRC_PATTERN_PATTERN_RESOURCES_H_
#define ICL_TOOLSETS_SRC_PATTERN_PATTERN_RESOURCES_H_
#include <map>
#include <set>
#include <string>
#include <vector>

#include "pattern/pattern_scheduler.h"
namespace pattern {
// Persistent takes belong to main; protection starts at applied writes/iState.
class ResourceReservations {
 public:
  explicit ResourceReservations(const Netlist& netlist);
  void Enter(const std::vector<std::string>& call_stack);
  void Change(const Operation& operation);
  void CheckAutomaticWrites(const std::vector<Operation>& operations) const;
  std::set<size_t> UnwrittenRegisters() const;
  void Assigned(const std::vector<Operation>& operations,
                const ScanScheduler& scheduler);
  const std::map<size_t, std::string>& Values() const { return values_; }

 private:
  std::set<size_t> Targets(const std::string& name) const;
  const Netlist& netlist_;
  std::map<std::string, size_t> scopes_;
  std::vector<std::vector<size_t>> registers_;
  std::vector<std::string> calls_;
  std::vector<std::set<size_t>> owners_{1};
  std::map<size_t, size_t> counts_;
  std::map<size_t, std::string> values_;
};
}  // namespace pattern
#endif  // ICL_TOOLSETS_SRC_PATTERN_PATTERN_RESOURCES_H_
