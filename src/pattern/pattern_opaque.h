// Copyright 2026 The ICL Toolsets Authors.
// SPDX-License-Identifier: MIT
#ifndef ICL_TOOLSETS_SRC_PATTERN_PATTERN_OPAQUE_H_
#define ICL_TOOLSETS_SRC_PATTERN_PATTERN_OPAQUE_H_

#include <z3++.h>

#include <optional>
#include <vector>

#include "pattern/pattern_netlist.h"
#include "pattern/pattern_runtime.h"

namespace pattern {
// Shared single-chain request validation, padding and canonical binary data.
Operation NormalizeScan(z3::context& context, const std::string& target,
                        const Operation& operation);
// No result without iScan. Otherwise validate a root opaque client and lower
// its queued scans. Unsupported topology/interactions fail before any
// publication.
std::optional<std::vector<Operation>> LowerOpaqueScans(
    const Netlist& netlist, const std::vector<Operation>& operations);
}  // namespace pattern
#endif  // ICL_TOOLSETS_SRC_PATTERN_PATTERN_OPAQUE_H_
