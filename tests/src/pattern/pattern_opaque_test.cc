// Copyright 2026 The ICL Toolsets Authors.
// SPDX-License-Identifier: MIT
#include "pattern/pattern_opaque.h"

#include <functional>
#include <iostream>
#include <map>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>

#include "pattern/pattern_design.h"
#include "pattern/pattern_flatten.h"
#include "pattern/pattern_scheduler.h"
#include "pattern/pattern_values.h"

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
    if (std::string(error.what()).find(diagnostic) == std::string::npos) {
      throw std::runtime_error("unexpected diagnostic: " +
                               std::string(error.what()));
    }
    return;
  }
  throw std::runtime_error("expected rejection: " + diagnostic);
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

const std::string kOpaqueChild = R"(
Module Leaf {
 ScanInPort si; ScanOutPort so;
 SelectPort sel; CaptureEnPort ce; ShiftEnPort se; UpdateEnPort ue;
 TCKPort tck; ResetPort reset;
 ScanInterface local {Port si; Port so; Port sel; Port ce; Port se; Port ue;
                      Port tck; Port reset; DefaultLoadValue 8'b0;}
}
Module Top {
 ScanInPort si; SelectPort sel; CaptureEnPort ce; ShiftEnPort se;
 UpdateEnPort ue; TCKPort tck; ResetPort reset;
 ScanRegister head[2:0] {ScanInSource si; CaptureSource head; ResetValue 0;}
 Instance child Of Leaf {InputPort si = head[0]; InputPort sel = sel;
  InputPort ce = ce; InputPort se = se; InputPort ue = ue; InputPort tck = tck;
  InputPort reset = reset;}
 ScanRegister tail[4:0] {ScanInSource child.so; CaptureSource tail; ResetValue 0;}
 ScanOutPort so {Source tail[0];}
 ScanInterface client {Port si; Port so; Port sel; Port ce; Port se; Port ue;
                       Port tck; Port reset; DefaultLoadValue 16'b0;}
}
)";
std::vector<pattern::Operation> ParentWrite(const std::string& head,
                                            const std::string& tail = "19") {
  return {{"iWrite", {"head", head}, "parent", {}},
          {"iWrite", {"tail", tail}, "parent", {}}};
}
pattern::Operation ChildScan(const std::string& input = "165",
                             bool stable = true) {
  pattern::Operation op{
      "iScan", {"child.local", "8", "-si", input, "-so", "60"}, "child", {}};
  if (stable) op.arguments.push_back("-stable");
  return op;
}
void CheckPublicSteps(const pattern::Netlist& original,
                      const std::vector<pattern::ScanStep>& steps) {
  for (const auto& step : steps) {
    for (const auto* fields : {&step.loads, &step.writes, &step.expected}) {
      for (const auto& [name, value] : *fields) {
        Require(original.node_ids.count(name),
                "public steps contain original names only");
      }
    }
  }
}
std::string MultipleOpaqueSource() {
  auto source = kOpaqueChild;
  const auto tail = source.find(" ScanRegister tail");
  source.insert(tail, R"(
 Instance second Of Leaf {InputPort si = child.so; InputPort sel = sel;
  InputPort ce = ce; InputPort se = se; InputPort ue = ue; InputPort tck = tck;
  InputPort reset = reset;}
)");
  const auto input = source.find("ScanInSource child.so");
  source.replace(input, std::string("ScanInSource child.so").size(),
                 "ScanInSource second.so");
  const auto width = source.find("DefaultLoadValue 16'b0");
  source.replace(width, std::string("DefaultLoadValue 16'b0").size(),
                 "DefaultLoadValue 24'b0");
  return source;
}
void TestMultipleOpaqueChildDefaults() {
  pattern::Design design;
  design.AddIcl("multiple.icl", MultipleOpaqueSource());
  const auto netlist = pattern::Elaborate(design, "Top");
  Require(pattern::ExpandOpaqueChild(netlist) != nullptr,
          "multiple repeated opaque leaves have one native route");
  pattern::ScanScheduler scheduler(netlist);
  scheduler.Reset();
  CheckPublicSteps(netlist, scheduler.Apply(ParentWrite("5")));
  auto second = ChildScan("60");
  second.arguments[0] = "second.local";
  std::vector<pattern::ScanStep> prefix;
  const auto result =
      scheduler.ApplyScans({ChildScan(), second}, {}, nullptr, &prefix);
  Require(result.arguments[1] == "24" &&
              result.arguments[3] == "0b101101001010011110010011" &&
              scheduler.Value("head") == "101" &&
              scheduler.Value("tail") == "10011" && prefix.empty(),
          "disjoint opaque requests preserve exact parent and child payloads");
}
void TestMultipleOpaqueAggregateLimit() {
  auto source = MultipleOpaqueSource();
  const auto width = source.find("DefaultLoadValue 8'b0");
  source.replace(width, std::string("DefaultLoadValue 8'b0").size(),
                 "DefaultLoadValue 524288'b0");
  pattern::Design design;
  design.AddIcl("aggregate.icl", source);
  const auto netlist = pattern::Elaborate(design, "Top");
  Require(!pattern::ExpandOpaqueChild(netlist),
          "aggregate opaque and parent width rejects before private expansion");
}
void TestMultipleOpaqueCopiesRollbackAndSubscriber() {
  pattern::Design design;
  design.AddIcl("multiple.icl", MultipleOpaqueSource());
  const auto netlist = pattern::Elaborate(design, "Top");
  pattern::ScanScheduler scheduler(netlist);
  scheduler.Reset();
  scheduler.Apply(ParentWrite("5"));
  auto second = ChildScan("60");
  second.arguments[0] = "second.local";
  scheduler.ApplyScans({ChildScan("165", false), second});
  auto unknown = scheduler;
  scheduler.ApplyScan(ChildScan("240"));
  Rejects([&] { unknown.Apply(ParentWrite("3")); }, "length");
  auto invalid = second;
  invalid.arguments[1] = "9";
  Rejects([&] { scheduler.ApplyScans({ChildScan("1"), invalid}); }, "length");
  auto steps = scheduler.Apply(ParentWrite("3"));
  Require(std::string(steps.back().serial.rbegin(),
                      steps.back().serial.rend()) == "011111100000011110010011",
          "failed later child and copied unknown length preserve every queue");
  second.arguments[3] = "170";
  bool subscriber = false;
  try {
    scheduler.ApplyScans({ChildScan("85"), second}, {}, nullptr, nullptr,
                         [&](const auto&, const auto&) {
                           throw std::runtime_error("subscriber");
                         });
  } catch (const std::runtime_error& error) {
    subscriber = std::string(error.what()) == "subscriber";
  }
  Require(subscriber, "multiple-child observer throws after commit");
  steps = scheduler.Apply(ParentWrite("2"));
  Require(std::string(steps.back().serial.rbegin(),
                      steps.back().serial.rend()) == "010010101011010101010011",
          "subscriber error retains both newly committed stable payloads");
  CheckPublicSteps(netlist, steps);
  const auto expanded = pattern::ExpandOpaqueChild(netlist);
  Require(expanded->intervals.size() == 2,
          "repeated module instances have separate private descriptors");
  for (const auto& interval : expanded->intervals) {
    const auto& name = expanded->netlist.nodes[interval.storage].name;
    Rejects([&] { scheduler.Value(name); }, "unknown");
    Rejects([&] { scheduler.Assume(name, "00000000"); }, "unknown");
  }
}
void TestOpaqueHistorySnapshotCopiesResetAndSubscriber() {
  pattern::Design design;
  design.AddIcl("history.icl", kOpaqueChild);
  const auto netlist = pattern::Elaborate(design, "Top");
  pattern::ScanScheduler scheduler(netlist);
  const pattern::Operation note{
      "iNote",
      {"-comment",
       "modeled opaque child raw history client child.local 0b10xx0011"},
      "history",
      {}};
  Require(!scheduler.OpaqueRawHistory(note), "unanchored history is ignored");
  scheduler.Reset();
  scheduler.Apply(ParentWrite("5"));
  auto unanchored = note;
  unanchored.arguments[1] =
      "modeled opaque child raw history client child.local 0bxxxxxxxx";
  Require(!scheduler.OpaqueRawHistory(unanchored),
          "ordinary frame does not authorize a raw history annotation");
  auto request = ChildScan("165");
  request.arguments[5] = "0b10xx0011";
  const auto physical = scheduler.ApplyScan(request);
  Require(physical.raw_history_omitted,
          "child lowering keeps physical root outside logical history scope");
  pattern::Operation explicit_root{"iScan", physical.arguments, "root", {}};
  auto root_copy = scheduler;
  Require(!root_copy.ApplyScan(explicit_root).raw_history_omitted,
          "explicit root request retains its logical history");
  const auto actual = scheduler.OpaqueRawHistory(note);
  Require(actual && actual->target == "child.local" && actual->chain.empty() &&
              actual->read == "xxxxxxxx" && actual->miscompare == "xx00xxxx",
          "history uses opaque capture and exact native comparison mask");
  auto copied = scheduler;
  scheduler.Reset();
  Require(!scheduler.OpaqueRawHistory(note) && copied.OpaqueRawHistory(note),
          "copy retains committed snapshot while reset invalidates original");
  auto invalid = request;
  invalid.arguments[1] = "9";
  Rejects([&] { copied.ApplyScan(invalid); }, "length");
  Require(copied.OpaqueRawHistory(note).has_value(),
          "failed scan retains prior comparison snapshot");
  request.arguments[5] = "0bxxxxxxxx";
  Rejects(
      [&] {
        copied.ApplyScan(request, {}, nullptr, nullptr,
                         [](const auto&, const auto&) {
                           throw std::runtime_error("subscriber");
                         });
      },
      "subscriber");
  auto latest = note;
  latest.arguments[1] =
      "modeled opaque child raw history client child.local 0bxxxxxxxx";
  const auto committed = copied.OpaqueRawHistory(latest);
  Require(
      !copied.OpaqueRawHistory(note) && committed &&
          committed->miscompare == "00000000",
      "subscriber failure retains newly committed snapshot without recounting");
}
void TestRawHistoryScopeUsesWinningRequest() {
  pattern::Design design;
  design.AddIcl("scope.icl", kOpaqueChild);
  const auto netlist = pattern::Elaborate(design, "Top");
  for (bool omitted : {false, true}) {
    pattern::ScanScheduler scheduler(netlist);
    scheduler.Reset();
    pattern::Operation first{
        "iScan", {"client", "16", "-si", "0", "-so", "0"}, "scope", {}};
    first.raw_history_omitted = omitted;
    auto last = first;
    last.raw_history_omitted = !omitted;
    size_t observed = 0;
    const auto physical = scheduler.ApplyScans(
        {first, last}, {}, nullptr, nullptr,
        [&](const auto& operation, const auto&) {
          ++observed;
          Require(operation.raw_history_omitted == last.raw_history_omitted,
                  "subscriber receives winning root scope");
        });
    Require(observed == 1 &&
                physical.raw_history_omitted == last.raw_history_omitted,
            "returned physical root scope matches the winning request");
  }
}
void TestUnusedOpaqueChildDoesNotChangePrimaryPaths() {
  const auto leaf = kOpaqueChild.substr(0, kOpaqueChild.find("Module Top"));
  pattern::Design design;
  design.AddIcl("unused.icl", leaf + R"(
Module Top {DataInPort p; DataOutPort q {Source p;} Instance child Of Leaf;}
)");
  const auto netlist = pattern::Elaborate(design, "Top");
  Require(!pattern::ExpandOpaqueChild(netlist),
          "unrelated primary network does not activate opaque scan expansion");
  pattern::ScanScheduler scheduler(netlist);
  Require(!scheduler.HasOpaqueChild(),
          "existing primary backend remains selected");
  scheduler.Apply({{"iWrite", {"p", "1"}, "primary", {}}});
  std::string actual;
  scheduler.Apply({{"iRead", {"q", "0"}, "primary", {}}}, false, {}, true,
                  false, [&](const auto&, const auto& bits) { actual = bits; });
  Require(actual == "1", "unrelated PIO behavior remains unchanged");
}
void TestOpaqueExpansionRequiresActiveStaticRoute() {
  auto bypass = kOpaqueChild;
  const auto position = bypass.find("ScanInSource child.so");
  bypass.replace(position, std::string("ScanInSource child.so").size(),
                 "ScanInSource head[0]");
  pattern::Design design;
  design.AddIcl("bypass.icl", bypass);
  const auto netlist = pattern::Elaborate(design, "Top");
  Require(!pattern::ExpandOpaqueChild(netlist),
          "unused opaque branch does not replace the existing native backend");
  pattern::ScanScheduler scheduler(netlist);
  scheduler.Reset();
  scheduler.Apply(ParentWrite("5"));
  Require(
      scheduler.Value("head") == "101" && scheduler.Value("tail") == "10011",
      "existing modeled route bypassing an unused opaque branch remains "
      "usable");

  auto gated = kOpaqueChild;
  const auto select = gated.find("InputPort sel = sel");
  gated.replace(select, std::string("InputPort sel = sel").size(),
                "InputPort sel = head[0]");
  pattern::Design conditional;
  conditional.AddIcl("conditional.icl", gated);
  const auto unsupported = pattern::Elaborate(conditional, "Top");
  Require(
      !pattern::ExpandOpaqueChild(unsupported),
      "conditionally selected opaque route is outside the static expansion");
}
void TestDynamicOpaqueQueueCopies() {
  auto source = kOpaqueChild;
  const auto select = source.find("InputPort sel = sel");
  source.replace(select, std::string("InputPort sel = sel").size(),
                 "InputPort sel = enable");
  const auto tail = source.find(" ScanRegister tail");
  source.insert(tail, R"(
 ScanMux route SelectedBy enable {1'b0: head[0]; 1'b1: child.so;}
 ScanRegister enable {ScanInSource route; CaptureSource enable; ResetValue 0;}
)");
  const auto input = source.find("ScanInSource child.so");
  source.replace(input, std::string("ScanInSource child.so").size(),
                 "ScanInSource enable");
  source.replace(source.find("DefaultLoadValue 16'b0"),
                 std::string("DefaultLoadValue 16'b0").size(),
                 "DefaultLoadValue 9'b0");
  pattern::Design design;
  design.AddIcl("dynamic_opaque.icl", source);
  const auto netlist = pattern::Elaborate(design, "Top");
  pattern::ScanScheduler scheduler(netlist);
  Require(scheduler.HasOpaqueChild(),
          "binary bypass network uses the private opaque execution model");
  for (const auto& role : {"CaptureSource", "SelectedBy"}) {
    auto unsafe = netlist;
    const bool capture = std::string(role) == "CaptureSource";
    auto& target =
        unsafe.nodes.at(unsafe.node_ids.at(capture ? "enable" : "route"));
    const auto input = unsafe.node_ids.at(capture ? "route" : "child.so");
    target.inputs[role] = {{input, unsafe.nodes[input].left}};
    Require(!pattern::ExpandOpaqueChild(unsafe),
            "opaque mux payload cannot become capture or selection control");
  }
  pattern::ScanScheduler automatic(netlist);
  automatic.Reset();
  std::set<size_t> assigned{netlist.node_ids.at("head")};
  std::vector<pattern::ScanStep> preparation(1);
  preparation.front().input = "unchanged";
  size_t observations = 0;
  for (size_t id : {netlist.nodes.size(), netlist.nodes.size() + 1}) {
    Rejects(
        [&] {
          automatic.ApplyScan(
              ChildScan(), {{id, "00000000"}}, &assigned, &preparation,
              [&](const auto&, const auto&) { ++observations; });
        },
        "unknown opaque child reserved resource");
  }
  Require(
      automatic.Value("enable") == "0" && observations == 0 &&
          assigned == std::set<size_t>{netlist.node_ids.at("head")} &&
          preparation.size() == 1 && preparation.front().input == "unchanged",
      "invalid public reservation leaves state and caller outputs untouched");
  Rejects(
      [&] {
        auto invalid = ChildScan();
        invalid.arguments[1] = "9";
        automatic.ApplyScan(invalid, {}, &assigned, &preparation,
                            [&](const auto&, const auto&) { ++observations; });
      },
      "length");
  Require(automatic.Value("enable") == "0" && observations == 0 &&
              assigned == std::set<size_t>{netlist.node_ids.at("head")} &&
              preparation.size() == 1 &&
              preparation.front().input == "unchanged",
          "failed raw length rolls back preparation and publishes no results");
  pattern::ScanScheduler closed_copy(automatic);
  std::string prepared_actual;
  const auto prepared = automatic.ApplyScan(
      ChildScan(), {}, &assigned, &preparation,
      [&](const auto&, const auto& bits) { prepared_actual = bits; });
  Require(
      preparation.size() == 1 && prepared.arguments[1] == "17" &&
          prepared_actual == "xxxxxxxx" && assigned.empty() &&
          automatic.Value("enable") == "1" &&
          closed_copy.Value("enable") == "0",
      "automatic opaque request commits one parent-only opening and raw CSU");
  CheckPublicSteps(netlist, preparation);
  Require(preparation.front().writes.count("enable") &&
              preparation.front().writes.at("enable") == "1",
          "preparation serializes the real modeled opening control");
  const auto prepared_completion = automatic.Apply(ParentWrite("3"));
  Require(
      prepared_completion.size() == 1 &&
          std::string(prepared_completion.front().serial.rbegin(),
                      prepared_completion.front().serial.rend())
                  .substr(3, 8) == "10100101",
      "preparation retains the requested stable payload for later completion");
  scheduler.Reset();
  scheduler.ApplyScan({"iScan",
                       {"client", "9", "-si", "0b101110011", "-so", "0bx"},
                       "open",
                       {}});
  std::string actual;
  const auto physical = scheduler.ApplyScan(
      ChildScan(), {}, nullptr, nullptr,
      [&](const auto&, const auto& bits) { actual = bits; });
  Require(actual == "xxxxxxxx" && physical.arguments[1] == "17" &&
              physical.arguments[3] == "0b10110100101110011",
          "active opaque request uses actual unknown return and intended "
          "physical input");
  pattern::ScanScheduler copy(scheduler);
  Rejects(
      [&] {
        scheduler.ApplyScan(
            {"iScan", {"client", "18", "-si", "0", "-so", "0bx"}, "bad", {}});
      },
      "length");
  scheduler.ApplyScan(
      {"iScan",
       {"client", "17", "-si", "0b10100000000010011", "-so", "0bx"},
       "close",
       {}});
  scheduler.ApplyScan({"iScan",
                       {"client", "9", "-si", "0b101010011", "-so", "0bx"},
                       "closed",
                       {}});
  actual.clear();
  scheduler.ApplyScan({"iScan",
                       {"client", "9", "-si", "0b101110011", "-so", "0bx"},
                       "reopen",
                       {}},
                      {}, nullptr, nullptr,
                      [&](const auto&, const auto& bits) { actual = bits; });
  Require(actual == "101010011",
          "inactive opaque queues do not block known parent-only reopening");
  scheduler.ApplyScan(ChildScan("60"));
  const auto current_steps = scheduler.Apply(ParentWrite("3"));
  const auto copied_steps = copy.Apply(ParentWrite("3"));
  Require(current_steps.size() == 1 && copied_steps.size() == 1,
          "active parent completion needs one real CSU in each scheduler copy");
  if (!current_steps.empty() && !copied_steps.empty()) {
    const std::string current(current_steps[0].serial.rbegin(),
                              current_steps[0].serial.rend());
    const std::string copied(copied_steps[0].serial.rbegin(),
                             copied_steps[0].serial.rend());
    Require(current.size() == 17 && copied.size() == 17 &&
                current.substr(3, 8) == "00111100" &&
                copied.substr(3, 8) == "10100101",
            "rollback and inactive scans preserve independent opaque "
            "completion intent");
  }
  CheckPublicSteps(netlist, current_steps);
  CheckPublicSteps(netlist, copied_steps);
}
void TestCallbackCallerFrameProof() {
  pattern::Design design;
  design.AddIcl("caller-proof.icl", R"(
Module Child {WriteEnPort we;}
Module Top {WriteEnPort we; DataInPort p; DataOutPort q {Source p;}
 Instance child Of Child {InputPort we = we;}}
)");
  const auto netlist = pattern::Elaborate(design, "Top");
  const auto id = netlist.node_ids.at("we");
  pattern::ScanScheduler scheduler(netlist);
  scheduler.Apply({{"iWrite", {"we", "0"}, "setup", {}},
                   {"iWrite", {"p", "0"}, "setup", {}}});
  size_t deliveries = 0;
  const pattern::ScanScheduler::PioObservation observe =
      [&](const auto&, const auto&) { ++deliveries; };
  bool rejected = false;
  try {
    scheduler.Apply({{"iWrite", {"we", "0"}, "caller", {}},
                     {"iWrite", {"child.we", "1"}, "callback", {}},
                     {"iWrite", {"p", "1"}, "callback", {}},
                     {"iRead", {"q", "0"}, "observer", {}}},
                    false, {}, true, false, observe, {{id, "0"}});
  } catch (const std::exception&) {
    rejected = true;
  }
  Require(
      rejected && scheduler.Value("we") == "0" && scheduler.Value("p") == "0" &&
          deliveries == 0,
      "failed callback frame rolls back state and publishes no observations");
  const auto steps =
      scheduler.Apply({{"iWrite", {"we", "1"}, "caller", {}},
                       {"iWrite", {"child.we", "1"}, "callback", {}}},
                      false, {}, true, false, {}, {{id, "1"}});
  Require(!steps.empty() && scheduler.Value("we") == "1",
          "compatible mapped caller goal commits after a failed frame");
}
void TestOpaqueChildNativeStateAndPrivacy() {
  pattern::Design design;
  design.AddIcl("opaque.icl", kOpaqueChild);
  const auto netlist = pattern::Elaborate(design, "Top");
  const auto expanded = pattern::ExpandOpaqueChild(netlist);
  Require(expanded && expanded->source_nodes == netlist.nodes.size(),
          "private expansion preserves original node IDs");
  const auto private_name =
      expanded->netlist.nodes.at(expanded->intervals.front().storage).name;
  Require(!netlist.node_ids.count(private_name) &&
              netlist.nodes.at(netlist.node_ids.at("child.so")).inputs.empty(),
          "expansion leaves the source network untouched");
  pattern::ScanScheduler scheduler(netlist);
  Require(scheduler.HasOpaqueChild(), "native scheduler owns an opaque child");
  scheduler.Reset();
  auto steps = scheduler.Apply(ParentWrite("5"));
  Require(!steps.empty() && scheduler.Value("head") == "101" &&
              scheduler.Value("tail") == "10011",
          "parent defaults traverse opaque child");
  CheckPublicSteps(netlist, steps);
  Require(scheduler.Apply({}).empty(), "empty Apply adds no opaque CSU");
  std::set<size_t> assigned{netlist.node_ids.at("head")};
  std::vector<pattern::ScanStep> preparation;
  std::string actual;
  const auto physical = scheduler.ApplyScan(
      ChildScan(), {}, &assigned, &preparation,
      [&](const auto&, const auto& bits) { actual = bits; });
  Require(
      actual == "xxxxxxxx" && preparation.empty() && assigned.empty(),
      "opaque observations/assignment footprints never invent private data");
  Require(
      physical.arguments ==
          std::vector<std::string>({"client", "16", "-si", "0b1011010010110011",
                                    "-so", "0bxxx00111100xxxxx", "-stable"}),
      "child interval preserves modeled parents and maps expected data");
  Require(
      scheduler.Value("child.so") == "x" && scheduler.Value("head") == "101" &&
          scheduler.Value("tail") == "10011",
      "private state stays unknown after shifting while parent state commits");
  const std::vector<pattern::Operation> private_reset{
      {"iWrite", {private_name, "1"}, "private reset target", {}}};
  Rejects([&] { scheduler.ResetPrerequisites(private_reset); }, "unknown");
  Rejects([&] { scheduler.CheckLocalResets(private_reset); }, "unknown");
  Rejects(
      [&] {
        scheduler.ResetPrerequisites(
            {}, {{expanded->intervals.front().storage, "0"}});
      },
      "reserved resource");
  Rejects(
      [&] {
        scheduler.ResetPrerequisites(
            {}, {}, {{expanded->intervals.front().storage, "0"}});
      },
      "reserved resource");
  const auto head = netlist.node_ids.at("head");
  Rejects([&] { scheduler.Assume("head", "000", {{head, "101"}}); },
          "reserved resource");
  Require(scheduler.Value("head") == "101",
          "guarded state assumption failure preserves native state");
  Rejects(
      [&] {
        scheduler.Assume("head", "101",
                         {{expanded->intervals.front().storage, "00000000"}});
      },
      "reserved resource");
  Rejects([&] { scheduler.Value(private_name); }, "unknown");
  Rejects([&] { scheduler.Assume(private_name, "00000000"); }, "unknown");
  Rejects(
      [&] {
        scheduler.Apply({{"iWrite", {private_name, "0"}, "private", {}}});
      },
      "unknown");
  std::map<std::pair<size_t, int64_t>, char> observed;
  steps = scheduler.Apply(
      {{"iRead", {"head", "0"}, "read", {}},
       {"iRead", {"tail", "0"}, "read", {}}},
      false, {}, true, false, [&](const auto& bits, const auto& values) {
        for (size_t i = 0; i < bits.size(); ++i) {
          Require(bits[i].node < netlist.nodes.size(),
                  "public observations exclude private IDs");
          observed[{bits[i].node, bits[i].index}] = values[i];
        }
      });
  CheckPublicSteps(netlist, steps);
  for (const auto& name : {"head", "tail"}) {
    std::string value;
    for (const auto& bit : pattern::ResolvePdlTarget(netlist, name).bits) {
      value += observed.at({bit.node, bit.index});
    }
    Require(value == scheduler.Value(name),
            "parent observations use actual native capture");
  }
  Rejects([&] { scheduler.Idle(1); }, "opaque");
  Rejects([&] { scheduler.Apply(ParentWrite("3"), false, {}, true, true); },
          "opaque");
  Rejects([&] { scheduler.Apply(ParentWrite("3"), false, {}, false); },
          "opaque");
}
void TestOpaqueChildLengthCopiesAndSubscriberCommit() {
  pattern::Design design;
  design.AddIcl("opaque.icl", kOpaqueChild);
  const auto netlist = pattern::Elaborate(design, "Top");
  pattern::ScanScheduler scheduler(netlist);
  scheduler.Reset();
  scheduler.Apply(ParentWrite("5"));
  scheduler.ApplyScan(ChildScan("165", false));
  Require(scheduler.Apply({}).empty(),
          "unknown opaque length permits empty Apply");
  auto unknown = scheduler;
  Rejects([&] { scheduler.Apply(ParentWrite("3")); }, "length");
  Require(scheduler.Value("head") == "101",
          "unknown-length preparation preserves state");
  scheduler.ApplyScan({"iScan",
                       {"client", "16", "-si", "0b1011001011010011", "-stable"},
                       "root stable",
                       {}});
  const auto projected = scheduler.Apply(ParentWrite("5"));
  Require(std::string(projected.back().serial.rbegin(),
                      projected.back().serial.rend()) == "1011001011010011",
          "full-root stable request projects the opaque controlling interval");
  Rejects([&] { unknown.Apply(ParentWrite("3")); }, "length");
  unknown.Reset();
  unknown.Apply(ParentWrite("3"));
  Require(unknown.Value("head") == "011" && scheduler.Value("head") == "101",
          "copies/reset have independent state and opaque knowledge");
  bool subscriber = false;
  try {
    scheduler.ApplyScan(ChildScan("60"), {}, nullptr, nullptr,
                        [&](const auto&, const auto&) {
                          throw std::runtime_error("subscriber");
                        });
  } catch (const std::runtime_error& error) {
    subscriber = std::string(error.what()) == "subscriber";
  }
  Require(subscriber, "test exercises post-commit subscriber failure");
  const auto steps = scheduler.Apply(ParentWrite("2"));
  Require(!steps.empty(), "committed stable opaque payload remains usable");
  const auto& serial = steps.back().serial;
  Require(
      std::string(serial.rbegin(), serial.rend()) == "0100011110010011",
      "subscriber failure commits the new opaque stable controlling payload");
  auto bad = ChildScan("60");
  bad.arguments[1] = "9";
  Rejects([&] { scheduler.ApplyScan(bad); }, "length");
  Require(scheduler.Value("head") == "010",
          "invalid length preserves parent state");
  const auto clock = netlist.node_ids.at("tck");
  Rejects([&] { scheduler.ApplyScan(ChildScan("240"), {{clock, "0"}}); },
          "reserved resource tck");
  Require(scheduler.Value("head") == "010",
          "ownership failure preserves original state");
  const auto recovered = scheduler.Apply(ParentWrite("4"));
  CheckPublicSteps(netlist, recovered);
  Require(std::string(recovered.back().serial.rbegin(),
                      recovered.back().serial.rend()) == "1000011110010011",
          "failed raw transaction retains prior opaque stable payload");
}
void TestOpaqueChildVisibleReferenceAndFlatOrigins() {
  for (unsigned inversion = 0; inversion < 4; ++inversion) {
    std::string opaque = kOpaqueChild;
    if (inversion & 1) {
      const auto at = opaque.find("InputPort si = head[0]");
      opaque.replace(at, std::string("InputPort si = head[0]").size(),
                     "InputPort si = ~head[0]");
    }
    if (inversion & 2) {
      const auto at = opaque.find("ScanInSource child.so");
      opaque.replace(at, std::string("ScanInSource child.so").size(),
                     "ScanInSource ~child.so");
    }
    auto visible = opaque;
    const auto output = visible.find("ScanOutPort so;");
    visible.replace(
        output, std::string("ScanOutPort so;").size(),
        "ScanRegister value[7:0] {ScanInSource si; CaptureSource 8'bxxxxxxxx;"
        "ResetValue 8'bxxxxxxxx; DefaultLoadValue 8'b0;}"
        "ScanOutPort so {Source value[0];}");
    pattern::Design visible_design;
    visible_design.AddIcl("visible.icl", visible);
    const auto control = pattern::Elaborate(visible_design, "Top");
    pattern::ScanScheduler reference(control);
    Require(!reference.HasOpaqueChild(),
            "visible reference is ordinary native state");
    reference.Reset();
    reference.Apply(ParentWrite("5"));
    std::string expected;
    const auto golden = reference.ApplyScan(
        ChildScan(), {}, nullptr, nullptr,
        [&](const auto&, const auto& bits) { expected = bits; });
    for (bool flat : {false, true}) {
      pattern::Design design;
      design.AddIcl("opaque.icl", opaque);
      auto netlist = pattern::Elaborate(design, "Top");
      pattern::Design flattened;
      if (flat) {
        flattened.AddIcl("flat.icl", pattern::SerializeIcl(netlist));
        netlist = pattern::Elaborate(flattened, "Top");
      }
      pattern::ScanScheduler scheduler(netlist);
      scheduler.Reset();
      scheduler.Apply(ParentWrite("5"));
      std::string actual;
      const auto result = scheduler.ApplyScan(
          ChildScan(), {}, nullptr, nullptr,
          [&](const auto&, const auto& bits) { actual = bits; });
      Require(result.arguments == golden.arguments && actual == expected &&
                  actual == "xxxxxxxx" && scheduler.Value("head") == "101" &&
                  scheduler.Value("tail") == "10011",
              "opaque/native reference agree across inversion and restored "
              "origins");
      scheduler.ApplyScan(golden);
      auto reference_copy = reference;
      reference_copy.ApplyScan(golden);
      const auto projected = scheduler.Apply(ParentWrite("3"));
      const auto expected_steps = reference_copy.Apply(ParentWrite("3"));
      Require(
          !projected.empty() && !expected_steps.empty() &&
              projected.back().serial == expected_steps.back().serial,
          "root-stable projection retains input/output inversion on later CSU");
      Require(scheduler.Value("child.so") == "x",
              "opaque Value remains unknown");
    }
  }
}

void TestOpaqueChildWrappersWithoutVisibleStorage() {
  const auto leaf = kOpaqueChild.substr(0, kOpaqueChild.find("Module Top"));
  const std::string wrapper = R"(
Module Wrapper {
 ScanInPort si; SelectPort sel; CaptureEnPort ce; ShiftEnPort se;
 UpdateEnPort ue; TCKPort tck; ResetPort reset;
 Instance leaf Of Leaf {InputPort si = si; InputPort sel = sel;
  InputPort ce = ce; InputPort se = se; InputPort ue = ue; InputPort tck = tck;
  InputPort reset = reset;}
 ScanOutPort so {Source leaf.so;}
 ScanInterface link {Port si; Port so; Port sel; Port ce; Port se; Port ue;
                     Port tck; Port reset; DefaultLoadValue 8'b0;}
}
Module Top {
 ScanInPort si; SelectPort sel; CaptureEnPort ce; ShiftEnPort se;
 UpdateEnPort ue; TCKPort tck; ResetPort reset;
 Instance child Of Wrapper {InputPort si = si; InputPort sel = sel;
  InputPort ce = ce; InputPort se = se; InputPort ue = ue; InputPort tck = tck;
  InputPort reset = reset;}
 ScanOutPort so {Source child.so;}
 ScanInterface client {Port si; Port so; Port sel; Port ce; Port se; Port ue;
                       Port tck; Port reset; DefaultLoadValue 8'b0;}
}
)";
  pattern::Design design;
  design.AddIcl("wrapper.icl", leaf + wrapper);
  const auto netlist = pattern::Elaborate(design, "Top");
  for (const auto& target : {"child.leaf.local", "child.link", "client"}) {
    pattern::ScanScheduler scheduler(netlist);
    Require(scheduler.HasOpaqueChild(),
            "wrapper-only hierarchy expands one opaque segment");
    scheduler.Reset();
    std::vector<pattern::ScanStep> prefix;
    std::string actual;
    const auto result = scheduler.ApplyScan(
        {"iScan", {target, "8", "-si", "165", "-stable"}, "wrapper", {}}, {},
        nullptr, &prefix,
        [&](const auto&, const auto& data) { actual = data; });
    Require(result.arguments[0] == "client" &&
                result.arguments[3] == "0b10100101" && actual == "xxxxxxxx" &&
                prefix.empty() && scheduler.Value("child.leaf.so") == "x",
            "every wrapper client maps to the same opaque physical interval");
  }
}
}  // namespace

int main() {
  size_t failures = 0;
  for (const auto test :
       {TestScalarCompletionDefaultStableUnknown,
        TestCompletionFailureAndCopies, TestNamedRequiredCompletionAndBudget,
        TestMultipleOpaqueChildDefaults, TestMultipleOpaqueAggregateLimit,
        TestMultipleOpaqueCopiesRollbackAndSubscriber,
        TestOpaqueHistorySnapshotCopiesResetAndSubscriber,
        TestRawHistoryScopeUsesWinningRequest,
        TestUnusedOpaqueChildDoesNotChangePrimaryPaths,
        TestOpaqueExpansionRequiresActiveStaticRoute,
        TestDynamicOpaqueQueueCopies, TestCallbackCallerFrameProof,
        TestOpaqueChildNativeStateAndPrivacy,
        TestOpaqueChildLengthCopiesAndSubscriberCommit,
        TestOpaqueChildVisibleReferenceAndFlatOrigins,
        TestOpaqueChildWrappersWithoutVisibleStorage}) {
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
