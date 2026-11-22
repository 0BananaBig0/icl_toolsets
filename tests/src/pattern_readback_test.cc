// Copyright 2026 The ICL Toolsets Authors.
// SPDX-License-Identifier: MIT
#include "pattern/pattern_readback.h"

#include <iostream>
#include <map>
#include <stdexcept>

#include "pattern/pattern_access_queue.h"
#include "pattern/pattern_design.h"
#include "pattern/pattern_scheduler.h"
#include "pattern/pattern_state.h"
#include "pattern/pattern_values.h"

namespace {
void Require(bool value, const char* message) {
  if (!value) throw std::runtime_error(message);
}
void TestReadModel() {
  pattern::Design design;
  design.AddIcl("queue.icl", R"(
Module Top {
 DataInPort p[3:0]; DataOutPort q[3:0] {Source p;}
 Alias inv[1:0] = ~q[1:0];
})");
  const auto netlist = pattern::Elaborate(design, "Top");
  z3::context context;
  pattern::QueuedAccesses queue;
  const auto add = [&](const std::string& command, const std::string& name,
                       const std::string& value) {
    const auto target = pattern::ResolvePdlTarget(netlist, name);
    queue.Add({command, {name, value}, "queue", {}}, target,
              pattern::PdlDataValue(context, netlist, target, value));
  };
  add("iWrite", "p", "0");
  add("iRead", "q", "15");
  add("iRead", "inv", "2");
  queue.Assume(pattern::ResolvePdlTarget(netlist, "p[3:2]"), "10");
  Require(queue.Operations().front().arguments[1] == "0b1000",
          "queued assumption changes the final write value");
  const auto check = [&](bool unknown) {
    const auto [bits, expected] = queue.ReadModel();
    Require(bits.size() == 4 && expected.size() == bits.size(),
            "read model has unique physical bits");
    std::map<int64_t, char> values;
    for (size_t i = 0; i < bits.size(); ++i) {
      Require(bits[i].node == netlist.node_ids.at("q") && !bits[i].inverted,
              "read model uses physical output coordinates");
      values.emplace(bits[i].index, expected[i]);
    }
    Require(values ==
                std::map<int64_t, char>{
                    {0, '1'}, {1, '0'}, {2, unknown ? 'x' : '1'}, {3, '1'}},
            "latest inverted reads and X clearing define expected model");
  };
  check(false);
  add("iRead", "q[2]", "0bx");
  check(true);
  pattern::NetworkState actual(netlist);
  actual.Assume(netlist.node_ids.at("p"), "1010");
  Require(actual.ReadBits(queue.ReadModel().first) == "0101",
          "batched observations use actual values, not expected data");
  queue.Clear();
  Require(queue.ReadModel().first.empty(), "clear discards read model");
}
void TestModeledReadback() {
  pattern::Design design;
  design.AddIcl("model.icl", R"(
Module Top {
 DataInPort p[3:0]; DataOutPort q[3:0] {Source p;}
})");
  const auto netlist = pattern::Elaborate(design, "Top");
  const auto readback = pattern::MakeModeledReadback(netlist);
  std::vector<pattern::Operation> operations{
      {"iState", {"p", "15"}, "test", {}},
      {"iRead", {"q", "0"}, "test", {}},
      {"iApply", {}, "test", {}}};
  const auto* update = readback(operations, false);
  Require(
      update && update->bits.size() == 4 && update->failures == 4,
      "actual observation counts four failures, independent of expected zero");
  for (const auto& bit : update->bits) {
    Require(bit.read == '1' && bit.miscompare == '1',
            "actual one differs from expected zero");
  }
  update = readback(operations, true);
  Require(update && update->bits.empty() && update->failures == 4,
          "repeated query delivers no stale delta and clears old count");
  update = readback(operations, false);
  Require(update && update->bits.empty() && update->failures == 0,
          "clear takes effect on the next query");
  for (const std::string command : {"iScan", "iMerge", "iClock"}) {
    const auto unsupported = pattern::MakeModeledReadback(netlist);
    bool rejected = false;
    try {
      unsupported({{command, {}, "unsupported", {}}}, false);
    } catch (const std::runtime_error& error) {
      rejected = std::string(error.what()).find(command + " histories") !=
                 std::string::npos;
    }
    Require(rejected, "unsupported histories cannot produce observations");
  }
}
void TestScheduledObservations() {
  pattern::Design design;
  design.AddIcl("frames.icl", R"(
Module Top {
 DataInPort p; DataInPort sel;
 DataOutPort a {Source p; Enable sel;}
 DataOutPort b {Source p; Enable ~sel;}
 Alias group = a {AccessTogether;}
})");
  const auto netlist = pattern::Elaborate(design, "Top");
  pattern::ScanScheduler scheduler(netlist);
  scheduler.Assume("p", "1");
  std::map<std::pair<size_t, int64_t>, char> observed;
  size_t frames = 0;
  const auto steps = scheduler.Apply(
      {{"iRead", {"group", "0"}, "test", {}},
       {"iRead", {"b", "0"}, "test", {}}},
      false, {}, true, false, [&](const auto& bits, const auto& values) {
        ++frames;
        for (size_t i = 0; i < bits.size(); ++i) {
          observed.emplace(std::make_pair(bits[i].node, bits[i].index),
                           values[i]);
        }
      });
  pattern::NetworkState oracle(netlist);
  oracle.Assume(netlist.node_ids.at("p"), "1");
  std::map<std::pair<size_t, int64_t>, char> replayed;
  for (const auto& step : steps) {
    std::map<size_t, std::string> writes;
    for (const auto& [name, bits] : step.writes) {
      writes.emplace(netlist.node_ids.at(name), bits);
    }
    oracle.ApplyInputs(writes);
    for (const auto& [name, expected] : step.expected) {
      const auto target = pattern::ResolvePdlTarget(netlist, name);
      const auto actual = oracle.ReadBits(target.bits);
      for (size_t i = 0; i < target.bits.size(); ++i) {
        replayed.emplace(
            std::make_pair(target.bits[i].node, target.bits[i].index),
            actual[i]);
      }
    }
  }
  Require(frames == 2 && observed.size() == 2 && observed == replayed,
          "observations match independent replay of actual emitted events");
  for (const auto& [bit, value] : observed) {
    Require(value == '1', "both enabled observations are one");
  }
  Require(scheduler.Value("a") + scheduler.Value("b") != "11",
          "final state cannot replace earlier enabled observations");
}
}  // namespace
int main() {
  try {
    TestReadModel();
    TestModeledReadback();
    TestScheduledObservations();
    std::cout << "readback tests passed\n";
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
