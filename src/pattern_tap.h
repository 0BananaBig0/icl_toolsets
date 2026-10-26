// Copyright 2026 The ICL Toolsets Authors.
// SPDX-License-Identifier: MIT
#ifndef ICL_TOOLSETS_SRC_PATTERN_TAP_H_
#define ICL_TOOLSETS_SRC_PATTERN_TAP_H_

#include <map>
#include <string>

#include "pattern_access_link.h"
#include "pattern_netlist.h"

namespace pattern {
struct TapDatapathModel {
  Netlist netlist;
  size_t instruction_register = 0;
  size_t scan_input = 0;
  size_t scan_output = 0;
  size_t ir_select = 0;
  std::map<std::string, size_t> instruction_selects;
  std::map<std::string, size_t> data_registers;
  std::map<std::string, size_t> scan_returns;
};

// Builds an owning IR/ActiveSignal/DR datapath through the shared ICL frontend.
// ir_select and scan_returns are boundary inputs, not an implemented TAP FSM or
// connected child chains. No reset, clock timing, or initialization is implied.
// Non-AccessLink instructions currently need explicit BSDL REGISTER_ACCESS.
TapDatapathModel BuildTapDatapathModel(const AccessLink& link);
}  // namespace pattern
#endif  // ICL_TOOLSETS_SRC_PATTERN_TAP_H_
