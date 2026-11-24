// Copyright 2026 The ICL Toolsets Authors.
// SPDX-License-Identifier: MIT
#ifndef ICL_TOOLSETS_SRC_PATTERN_PATTERN_READBACK_H_
#define ICL_TOOLSETS_SRC_PATTERN_PATTERN_READBACK_H_

#include "pattern/pattern_runtime.h"

namespace pattern {
inline constexpr char kModeledReadbackNote[] =
    "modeled register/PIO readback; no live device";
// The netlist must outlive this append-only, single-runtime session.
// Replay follows supplied operation order; observed merges must retain it.
Runtime::Readback MakeModeledReadback(const Netlist& netlist);
Runtime::Readback MakeModeledReadback(const Netlist& netlist, bool physical);
}  // namespace pattern
#endif  // ICL_TOOLSETS_SRC_PATTERN_PATTERN_READBACK_H_
