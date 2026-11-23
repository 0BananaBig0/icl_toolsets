// Copyright 2026 The ICL Toolsets Authors.
// SPDX-License-Identifier: MIT
#include "pattern/pattern_opaque.h"

#include <functional>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
void Require(bool value, const char* message) {
  if (!value) throw std::runtime_error(message);
}
const pattern::Operation kApply{"iApply", {}, "test", {}};
const pattern::Operation kReset{"iReset", {}, "test", {}};
void Rejects(const std::function<void()>& action,
             const std::string& diagnostic) {
  try {
    action();
  } catch (const std::runtime_error& error) {
    Require(std::string(error.what()).find(diagnostic) != std::string::npos,
            "rejection preserves the relevant diagnostic");
    return;
  }
  throw std::runtime_error("expected rejection");
}
void CheckScan(const std::vector<pattern::Operation>& operations,
               const std::string& input, const std::string& expected,
               bool stable, size_t position = 0) {
  Require(
      operations.size() > position + 1 && operations.back().command == "iApply",
      "required completion emits scan and one Apply");
  const auto& scan = operations[position];
  const auto fields = pattern::ReadScanFields(scan);
  Require(scan.command == "iScan" &&
              scan.arguments[fields.input] == "0b" + input &&
              scan.arguments[fields.expected] == "0b" + expected &&
              (scan.arguments.back() == "-stable") == stable,
          "completion preserves controlling payload, compare and stable state");
}
void TestScalarCompletionDefaultStableUnknown() {
  z3::context context;
  pattern::OpaqueScanQueue queue({"client", {{"", "00000000"}}});
  Require(queue.Apply(kApply).empty(), "empty root Apply remains event-free");
  Rejects([&] { queue.Complete(kApply); }, "length");
  queue.Reset(kReset);
  Require(queue.Apply(kApply).empty(), "reset does not manufacture root CSU");
  CheckScan(queue.Complete(kApply), "00000000", "xxxxxxxx", true);
  queue.Add(context, {"iScan",
                      {"client", "8", "-si", "165", "-so", "60", "-stable"},
                      "explicit",
                      {}});
  CheckScan(queue.Apply(kApply), "10100101", "00111100", true);
  Require(queue.Apply(kApply).empty(),
          "stable data does not make empty Apply run");
  CheckScan(queue.Complete(kApply), "10100101", "xxxxxxxx", true);
  queue.Add(context,
            {"iScan", {"client", "4", "-si", "9", "-stable"}, "short", {}});
  CheckScan(queue.Complete(kApply), "1001", "xxxx", true);
  CheckScan(queue.Complete(kApply), "1001", "xxxx", true);
  queue.Add(context, {"iScan", {"client", "8", "-si", "90"}, "nonstable", {}});
  CheckScan(queue.Apply(kApply), "01011010", "xxxxxxxx", false);
  Rejects([&] { queue.Complete(kApply); }, "length");
  Require(queue.Apply(kApply).empty(),
          "unknown length still permits empty Apply");
  queue.Reset(kReset);
  CheckScan(queue.Complete(kApply), "00000000", "xxxxxxxx", true);
}
void TestCompletionFailureAndCopies() {
  z3::context context;
  pattern::OpaqueScanQueue queue({"client", {{"", "10xx1100"}}});
  queue.Reset(kReset);
  CheckScan(queue.Complete(kApply), "10xx1100", "xxxxxxxx", true);
  queue.Add(context,
            {"iScan", {"client", "8", "-si", "165", "-stable"}, "valid", {}});
  Rejects(
      [&] {
        queue.Add(context,
                  {"iScan", {"client", "8", "-si", "256"}, "invalid", {}});
      },
      "scan");
  Require(queue.Pending().size() == 1,
          "invalid replacement retains valid queue");
  Rejects([&] { queue.Complete({"iApply", {"-invalid"}, "bad apply", {}}); },
          "iApply");
  Require(queue.Pending().size() == 1,
          "invalid Apply does not consume requests");
  auto copy = queue;
  CheckScan(copy.Complete(kApply), "10100101", "xxxxxxxx", true);
  queue.Reset(kReset);
  CheckScan(queue.Complete(kApply), "10xx1100", "xxxxxxxx", true);
  CheckScan(copy.Complete(kApply), "10100101", "xxxxxxxx", true);
  auto failed = copy;
  failed.Add(context, {"iScan", {"client", "8", "-si", "90"}, "nonstable", {}});
  failed.Complete(kApply);
  Rejects([&] { failed.Complete(kApply); }, "length");
  CheckScan(copy.Complete(kApply), "10100101", "xxxxxxxx", true);
}
void TestNamedRequiredCompletionAndBudget() {
  z3::context context;
  pattern::OpaqueScanQueue queue(
      {"client", {{"first", "0000"}, {"second", "00"}}});
  Rejects([&] { queue.Complete(kApply); }, "length");
  queue.Reset(kReset);
  const auto initial = queue.Complete(kApply);
  Require(initial.size() == 3, "all required named defaults complete together");
  CheckScan(initial, "0000", "xxxx", true);
  CheckScan(initial, "00", "xx", true, 1);
  queue.Add(context, {"iScan",
                      {"client", "-chain", "first", "4", "-si", "9", "-so", "3",
                       "-stable"},
                      "named",
                      {}});
  const auto explicit_scan = queue.Apply(kApply);
  CheckScan(explicit_scan, "1001", "0011", true);
  CheckScan(explicit_scan, "00", "xx", true, 1);
  const auto implicit = queue.Complete(kApply);
  CheckScan(implicit, "1001", "xxxx", true);
  CheckScan(implicit, "00", "xx", true, 1);
  pattern::OpaqueScanQueue large(
      {"client", {{"first", std::string(1048576, '0')}, {"second", "0"}}});
  large.Reset(kReset);
  Rejects([&] { large.Complete(kApply); }, "resource");
  Require(large.Pending().empty(),
          "failed implicit completion adds no requests");
  large.Add(context, {"iScan",
                      {"client", "-chain", "first", "1", "-si", "1", "-stable"},
                      "smaller",
                      {}});
  const auto recovered = large.Complete(kApply);
  CheckScan(recovered, "1", "x", true);
  CheckScan(recovered, "0", "x", true, 1);
}
}  // namespace

int main() {
  size_t failures = 0;
  for (const auto test :
       {TestScalarCompletionDefaultStableUnknown,
        TestCompletionFailureAndCopies, TestNamedRequiredCompletionAndBudget}) {
    try {
      test();
    } catch (const std::exception& error) {
      ++failures;
      std::cerr << "FAIL: " << error.what() << '\n';
    }
  }
  std::cout << "opaque scan failures: " << failures << '\n';
  return failures ? 1 : 0;
}
