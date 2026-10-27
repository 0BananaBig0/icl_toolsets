// Copyright 2026 The ICL Toolsets Authors.
// SPDX-License-Identifier: MIT
#ifndef ICL_TOOLSETS_SRC_PATTERN_PATTERN_LOGIC_H_
#define ICL_TOOLSETS_SRC_PATTERN_PATTERN_LOGIC_H_

#include <functional>

#include "pattern/pattern_netlist.h"

namespace pattern {
void CompileLogic(Netlist* netlist, size_t node, size_t* remaining_bits);
char EvaluateLogic(const NetlistNode& node,
                   const std::function<char(const BitReference&)>& value);
}  // namespace pattern
#endif  // ICL_TOOLSETS_SRC_PATTERN_PATTERN_LOGIC_H_
