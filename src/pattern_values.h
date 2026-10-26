// Copyright 2026 The ICL Toolsets Authors.
// SPDX-License-Identifier: MIT
#ifndef ICL_TOOLSETS_SRC_PATTERN_VALUES_H_
#define ICL_TOOLSETS_SRC_PATTERN_VALUES_H_

#include <z3++.h>

#include "pattern_netlist.h"

namespace pattern {
struct PdlTarget {
  size_t declaration = 0;
  std::vector<BitReference> bits;
};
std::string PdlNumber(z3::context& context, const std::string& text,
                      size_t width);
PdlTarget ResolvePdlTarget(const Netlist& netlist, const std::string& text);
}  // namespace pattern
#endif  // ICL_TOOLSETS_SRC_PATTERN_VALUES_H_
