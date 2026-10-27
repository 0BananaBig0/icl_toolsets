// Copyright 2026 The ICL Toolsets Authors.
// SPDX-License-Identifier: MIT
#ifndef ICL_TOOLSETS_SRC_PATTERN_PATTERN_MERGE_H_
#define ICL_TOOLSETS_SRC_PATTERN_PATTERN_MERGE_H_

#include <vector>

#include "pattern/pattern_netlist.h"
#include "pattern/pattern_runtime.h"

namespace pattern {
// Consumes a finished Runtime stream; preserves origins and call stacks.
// Combined streams still require scheduling/replay and may need serial
// fallback.
std::vector<Operation> MergeOperations(const Netlist& netlist,
                                       const std::vector<Operation>& operations,
                                       bool combine);
}  // namespace pattern
#endif  // ICL_TOOLSETS_SRC_PATTERN_PATTERN_MERGE_H_
