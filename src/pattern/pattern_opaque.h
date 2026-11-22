// Copyright 2026 The ICL Toolsets Authors.
// SPDX-License-Identifier: MIT
#ifndef ICL_TOOLSETS_SRC_PATTERN_PATTERN_OPAQUE_H_
#define ICL_TOOLSETS_SRC_PATTERN_PATTERN_OPAQUE_H_

#include <z3++.h>

#include <map>
#include <optional>
#include <set>
#include <utility>
#include <vector>

#include "pattern/pattern_netlist.h"
#include "pattern/pattern_runtime.h"

namespace pattern {
// Shared single-chain request validation, padding and canonical binary data.
// Caller proves a single unnamed client; accepts Tessent -chain default.
Operation NormalizeScan(z3::context& context, const std::string& target,
                        const Operation& operation);
// Validate payloads/options without selecting or executing a hardware route.
Operation NormalizeScan(z3::context& context, const Operation& operation);
struct ScanFields {
  bool instruction = false;
  size_t target = 0, length = 0, input = 0, expected = 0;
  std::string chain;
};
ScanFields ReadScanFields(const Operation& normalized);
struct OpaqueScanModel {
  std::string client;
  std::map<std::string, std::string> defaults;
};
OpaqueScanModel DescribeOpaqueScans(const Netlist& netlist);
Operation NormalizeOpaqueScan(z3::context& context,
                              const OpaqueScanModel& model,
                              const Operation& operation);
class OpaqueScanQueue {
 public:
  explicit OpaqueScanQueue(OpaqueScanModel model) : model_(std::move(model)) {}
  void Add(z3::context& context, const Operation& operation);
  void Reset(const Operation& operation);
  std::vector<Operation> Apply(const Operation& operation);
  const std::map<std::string, Operation>& Pending() const { return pending_; }

 private:
  OpaqueScanModel model_;
  size_t pending_bits_ = 0;
  bool defaults_available_ = false;
  std::set<std::string> unknown_lengths_;
  std::map<std::string, Operation> pending_, stable_;
};
// No result without iScan. Otherwise validate a root opaque client and lower
// its queued scans. Unsupported topology/interactions fail before any
// publication.
std::optional<std::vector<Operation>> LowerOpaqueScans(
    const Netlist& netlist, const std::vector<Operation>& operations);
}  // namespace pattern
#endif  // ICL_TOOLSETS_SRC_PATTERN_PATTERN_OPAQUE_H_
