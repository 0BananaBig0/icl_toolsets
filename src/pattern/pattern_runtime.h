// Copyright 2026 The ICL Toolsets Authors.
// SPDX-License-Identifier: MIT
#ifndef ICL_TOOLSETS_SRC_PATTERN_PATTERN_RUNTIME_H_
#define ICL_TOOLSETS_SRC_PATTERN_PATTERN_RUNTIME_H_

#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "pattern/pattern_design.h"

namespace pattern {
struct Netlist;
// One executed main command, including its nested operations.
struct SourceAction {
  size_t source = 0, offset = 0, begin = 0, end = 0;
  std::string prefix;
  // Source positions requiring vector-literal protection in Tcl wrappers.
  std::vector<size_t> vector_brackets;
  // Evaluated caller arguments, excluding the command word.
  std::vector<std::string> arguments{};
};
struct Operation {
  std::string command;
  std::vector<std::string> arguments;
  std::string origin;
  std::vector<std::string> call_stack;
  // Logical callback request; endpoint operations are appended at Apply.
  bool callback = false;
};

struct ReadbackUpdate {
  size_t node = 0;
  int64_t index = 0;
  char read = '?', miscompare = '?';
};
struct ScanReadbackUpdate {
  std::string target, chain, read, miscompare;
};
struct ReadbackUpdates {
  std::vector<ReadbackUpdate> bits;
  std::optional<uint64_t> failures;
  std::vector<ScanReadbackUpdate> scans{};
};

// Only the generator evaluates PDL. Editor tools continue using Document alone.
class Runtime {
 public:
  // Updates are deltas for this append-only stream, valid until the next call.
  using Readback = std::function<const ReadbackUpdates*(
      const std::vector<Operation>&, bool clear_status)>;
  Runtime(const Design& design, const std::string& top);
  Runtime(const Design& design, const Netlist& netlist,
          bool trace_source = false, Readback readback = {});
  ~Runtime();
  Runtime(const Runtime&) = delete;
  Runtime& operator=(const Runtime&) = delete;
  void Execute(const std::string& path, const std::string& text);
  // Validate the complete ordered stream, allowing queues across input files.
  void Finish();
  const std::vector<Operation>& operations() const;
  const std::vector<std::string>& warnings() const;
  int Level() const;
  bool CallbacksExecuted() const;
  std::vector<SourceAction> SourceActions(bool whole_commands = false) const;

 private:
  class Impl;
  std::unique_ptr<Impl> impl_;
};
}  // namespace pattern
#endif  // ICL_TOOLSETS_SRC_PATTERN_PATTERN_RUNTIME_H_
