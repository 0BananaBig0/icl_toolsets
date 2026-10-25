// Copyright 2026 The ICL Toolsets Authors.
// SPDX-License-Identifier: MIT
#ifndef ICL_TOOLSETS_SRC_PATTERN_EXPRESSION_H_
#define ICL_TOOLSETS_SRC_PATTERN_EXPRESSION_H_

#include <cstdint>
#include <functional>
#include <string>
#include <vector>

#include "icl_ast.h"

namespace pattern {
struct Constant {
  bool is_string = false;
  bool sized = false;
  // Decoded string, or most-significant-first numeric bits (0, 1, x).
  std::string data;
  int64_t AsInteger() const;
  Constant Sized(size_t width) const;
};

Constant EvaluateConstant(
    const std::vector<icl::SourceToken>& tokens, size_t begin, size_t end,
    const std::function<Constant(const std::string&)>& parameter,
    size_t context_width = 0, size_t max_string_bytes = 1024 * 1024);

// Evaluates already parsed ICL integer tokens, with checked 64-bit
// intermediates. Parameter lookup belongs to the elaborated instance, not the
// editor workspace.
int64_t EvaluateInteger(
    const std::vector<icl::SourceToken>& tokens, size_t begin, size_t end,
    const std::function<int64_t(const std::string&)>& parameter);
}  // namespace pattern
#endif  // ICL_TOOLSETS_SRC_PATTERN_EXPRESSION_H_
