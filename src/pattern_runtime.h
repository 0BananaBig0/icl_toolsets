// Copyright 2026 The ICL Toolsets Authors.
// SPDX-License-Identifier: MIT
#ifndef ICL_TOOLSETS_SRC_PATTERN_RUNTIME_H_
#define ICL_TOOLSETS_SRC_PATTERN_RUNTIME_H_

#include <memory>
#include <string>
#include <vector>

#include "pattern_design.h"

namespace pattern {
struct Netlist;
struct Operation {
  std::string command;
  std::vector<std::string> arguments;
  std::string origin;
  std::vector<std::string> call_stack;
};

// Only the generator evaluates PDL. Editor tools continue using Document alone.
class Runtime {
 public:
  Runtime(const Design& design, const std::string& top);
  Runtime(const Design& design, const Netlist& netlist);
  ~Runtime();
  Runtime(const Runtime&) = delete;
  Runtime& operator=(const Runtime&) = delete;
  void Execute(const std::string& path, const std::string& text);
  // Validate the complete ordered stream, allowing queues across input files.
  void Finish();
  const std::vector<Operation>& operations() const;
  const std::vector<std::string>& warnings() const;

 private:
  class Impl;
  std::unique_ptr<Impl> impl_;
};
}  // namespace pattern
#endif  // ICL_TOOLSETS_SRC_PATTERN_RUNTIME_H_
