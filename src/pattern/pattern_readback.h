// Copyright 2026 The ICL Toolsets Authors.
// SPDX-License-Identifier: MIT
#ifndef ICL_TOOLSETS_SRC_PATTERN_PATTERN_READBACK_H_
#define ICL_TOOLSETS_SRC_PATTERN_PATTERN_READBACK_H_

#include "pattern/pattern_runtime.h"

namespace pattern {
inline constexpr char kModeledReadbackNote[] =
    "modeled combinational PIO readback; no live device";
// The netlist must outlive this append-only, single-runtime session.
Runtime::Readback MakeModeledReadback(const Netlist& netlist);
}  // namespace pattern
#endif  // ICL_TOOLSETS_SRC_PATTERN_PATTERN_READBACK_H_
