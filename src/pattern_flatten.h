// Copyright 2026 The ICL Toolsets Authors.
// SPDX-License-Identifier: MIT
#ifndef ICL_TOOLSETS_SRC_PATTERN_FLATTEN_H_
#define ICL_TOOLSETS_SRC_PATTERN_FLATTEN_H_

#include <string>

#include "pattern_netlist.h"

namespace pattern {
inline constexpr char kFlatOriginAttribute[] = "icl_pattern_origins_v1";
std::string SerializeIcl(const Netlist& netlist);
// Returns whether flat metadata supplied the hierarchy-dependent associations.
bool RestoreFlatOrigins(Netlist* netlist);
}  // namespace pattern
#endif  // ICL_TOOLSETS_SRC_PATTERN_FLATTEN_H_
