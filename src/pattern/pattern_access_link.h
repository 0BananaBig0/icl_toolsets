// Copyright 2026 The ICL Toolsets Authors.
// SPDX-License-Identifier: MIT
#ifndef ICL_TOOLSETS_SRC_PATTERN_PATTERN_ACCESS_LINK_H_
#define ICL_TOOLSETS_SRC_PATTERN_PATTERN_ACCESS_LINK_H_

#include <string>
#include <vector>

#include "pattern/pattern_design.h"

namespace pattern {
struct AccessLinkInterface {
  std::string instance, name, origin;
  const ModuleDefinition* module = nullptr;
  const icl::AstBlock* ast = nullptr;
};
struct AccessLinkSignal {
  std::string text, origin;
};
struct AccessLinkInstruction {
  std::string name, origin;
  std::vector<AccessLinkInterface> interfaces;
  std::vector<AccessLinkSignal> active_signals;
};
struct AccessLink {
  std::string name, standard, origin;
  const BsdlTap* bsdl = nullptr;
  std::vector<AccessLinkInstruction> instructions;
  const ModuleDefinition* module = nullptr;
};

// Source-level binding for subsequent TAP construction, not a connectivity or
// retargeting verdict. The selected hierarchy must be valid. Design must
// outlive the result and remain unchanged; interfaces borrow the existing
// shared AST.
std::vector<AccessLink> BindAccessLinks(const Design& design,
                                        const ModuleDefinition& top);
}  // namespace pattern
#endif  // ICL_TOOLSETS_SRC_PATTERN_PATTERN_ACCESS_LINK_H_
