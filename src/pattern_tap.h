// Copyright 2026 The ICL Toolsets Authors.
// SPDX-License-Identifier: MIT
#ifndef ICL_TOOLSETS_SRC_PATTERN_TAP_H_
#define ICL_TOOLSETS_SRC_PATTERN_TAP_H_

#include <map>
#include <string>

#include "pattern_access_link.h"
#include "pattern_netlist.h"

namespace pattern {
struct TapInstructionModel {
  Netlist netlist;
  size_t instruction_register = 0;
  size_t scan_input = 0;
  std::map<std::string, size_t> instruction_selects;
};

// Builds an owning IR/ActiveSignal component using the shared ICL frontend.
// No TAP FSM, reset, data-register routing, or child-interface wiring is
// implied.
TapInstructionModel BuildTapInstructionModel(const AccessLink& link);
}  // namespace pattern
#endif  // ICL_TOOLSETS_SRC_PATTERN_TAP_H_
