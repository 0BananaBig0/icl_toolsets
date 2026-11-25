// Copyright 2026 The ICL Toolsets Authors.
// SPDX-License-Identifier: MIT
#include "pattern/pattern_readback.h"

#include <fstream>
#include <iostream>
#include <iterator>
#include <limits>
#include <map>
#include <stdexcept>

#include "pattern/pattern_access_queue.h"
#include "pattern/pattern_design.h"
#include "pattern/pattern_opaque.h"
#include "pattern/pattern_scheduler.h"
#include "pattern/pattern_state.h"
#include "pattern/pattern_tap_execution.h"
#include "pattern/pattern_values.h"

namespace {
void Require(bool value, const char* message) {
  if (!value) throw std::runtime_error(message);
}
void CheckCommittedObserverFailure(pattern::ScanScheduler* scheduler,
                                   const pattern::Netlist& netlist,
                                   const std::string& target,
                                   const std::string& committed,
                                   const std::string& next_read, bool physical,
                                   bool write) {
  std::vector<pattern::Operation> operations{
      {"iRead", {target, "0"}, "test", {}}};
  if (write) operations.push_back({"iWrite", {target, "6"}, "test", {}});
  size_t calls = 0;
  bool propagated = false;
  try {
    scheduler->Apply(operations, false, {}, true, physical,
                     [&](const auto&, const auto&) {
                       ++calls;
                       throw std::runtime_error("listener failure");
                     });
  } catch (const std::runtime_error& error) {
    propagated =
        std::string(error.what()).find("listener failure") != std::string::npos;
  }
  Require(propagated && calls == 1, "subscriber failure propagates once");
  Require(scheduler->Value(target) == committed,
          "subscriber failure retains completed transaction state");
  std::map<std::pair<size_t, int64_t>, char> observed;
  scheduler->Apply(
      {{"iRead", {target, "0b" + next_read}, "test", {}}}, false, {}, true,
      physical, [&](const auto& bits, const auto& values) {
        for (size_t i = 0; i < bits.size(); ++i) {
          observed.emplace(std::make_pair(bits[i].node, bits[i].index),
                           values[i]);
        }
      });
  std::string actual;
  for (const auto& bit : pattern::ResolvePdlTarget(netlist, target).bits) {
    actual += observed.at({bit.node, bit.index});
  }
  Require(
      actual == next_read && scheduler->Value(target) == committed,
      "following read observes committed data without restoring stale fill");
}
void TestDynamicChildCommittedGeometry() {
  const std::string path =
      "tests/cases/ours/pattern_generation/active_child_raw/model.icl";
  std::ifstream source(path);
  Require(source.good(), "dynamic child fixture is available");
  pattern::Design design;
  design.AddIcl(path, std::string(std::istreambuf_iterator<char>(source),
                                  std::istreambuf_iterator<char>()));
  const auto netlist = pattern::Elaborate(design, "Top");
  pattern::ScanScheduler scheduler(netlist);
  scheduler.Reset();
  scheduler.Assume("open", "1");
  scheduler.Assume("child.value", "10100101");
  pattern::Operation root{
      "iScan",
      {"client", "9", "-si", "0b001111000", "-so", "0b00000000x"},
      "test",
      {}};
  const auto committed = scheduler.ApplyScan(root);
  const pattern::Operation note{
      "iNote",
      {"-comment",
       "modeled dynamic child raw history client child.local 0b00000000"},
      "test",
      {}};
  const auto projection = scheduler.ModeledRawHistory(note, committed);
  Require(projection && projection->target == "child.local" &&
              projection->offset == 0 && projection->expected == "00000000" &&
              !projection->inverted && scheduler.Value("open") == "0",
          "dynamic history retains the completed nine-bit route after closure");
  pattern::ScanScheduler copy(scheduler);
  bool failed = false;
  try {
    scheduler.ApplyScan(root);
  } catch (const std::runtime_error&) {
    failed = true;
  }
  Require(failed && scheduler.ModeledRawHistory(note, committed).has_value(),
          "failed raw execution preserves the last committed geometry");
  scheduler.ApplyScan(
      {"iScan", {"client", "1", "-si", "0b1", "-so", "0bx"}, "test", {}});
  Require(
      !scheduler.ModeledRawHistory(note, committed) &&
          copy.ModeledRawHistory(note, committed).has_value(),
      "successful replacement is local while copies retain immutable history");
  pattern::Runtime::HistoryReadback validate;
  const auto readback =
      pattern::MakeModeledReadback(netlist, false, nullptr, &validate);
  std::vector<pattern::Operation> operations{
      {"iReset", {}, "test", {}},
      {"iState", {"open", "1"}, "test", {}},
      {"iState", {"child.value", "165"}, "test", {}},
      root,
      {"iApply", {}, "test", {}}};
  operations[3].raw_history_omitted = true;
  Require(validate(operations, note),
          "preview projects actual completed dynamic geometry");
  auto accepted = note;
  accepted.modeled_raw_history = true;
  operations.push_back(accepted);
  const auto* update = readback(operations, false);
  Require(
      update->failures == 4 && update->scans.size() == 1 &&
          update->scans[0].target == "child.local" &&
          update->scans[0].read == "10100101" &&
          update->scans[0].miscompare == "10100101",
      "dynamic preview retains actual child deltas without root or recount");
  update = readback(operations, false);
  Require(update->scans.empty() && update->failures == 4,
          "dynamic child deltas publish exactly once");
}
void TestModeledChildHistoryProjection() {
  pattern::Design design;
  design.AddIcl("child_history.icl", R"(
Module Leaf {
 ScanInPort si; SelectPort sel; CaptureEnPort ce; ShiftEnPort se;
 UpdateEnPort ue; TCKPort tck; ResetPort reset;
 ScanRegister value[3:0] {ScanInSource si; CaptureSource value; ResetValue 0;}
 ScanOutPort so {Source value[0];}
 ScanInterface local {Port si; Port so; Port sel; Port ce; Port se; Port ue; Port tck; Port reset;}
}
Module Top {
 ScanInPort si; SelectPort sel; CaptureEnPort ce; ShiftEnPort se;
 UpdateEnPort ue; TCKPort tck; ResetPort reset;
 Instance child Of Leaf {InputPort si = si; InputPort sel = sel; InputPort ce = ce;
  InputPort se = se; InputPort ue = ue; InputPort tck = tck; InputPort reset = reset;}
 ScanOutPort so {Source child.so;}
 ScanInterface client {Port si; Port so; Port sel; Port ce; Port se; Port ue; Port tck; Port reset;}
})");
  const auto netlist = pattern::Elaborate(design, "Top");
  pattern::Runtime::HistoryReadback validate;
  const auto readback =
      pattern::MakeModeledReadback(netlist, false, nullptr, &validate);
  std::vector<pattern::Operation> operations{
      {"iReset", {}, "test", {}},
      {"iState", {"child.value", "9"}, "test", {}},
      {"iScan", {"client", "4", "-si", "0b0000", "-so", "0b0000"}, "test", {}},
      {"iApply", {}, "test", {}}};
  operations[2].raw_history_omitted = true;
  pattern::Operation note{
      "iNote",
      {"-comment", "modeled child raw history client child.local 0b0000"},
      "test",
      {}};
  auto rejected = note;
  rejected.arguments[1].replace(rejected.arguments[1].size() - 4, 4, "1111");
  Require(!validate(operations, rejected) && validate(operations, note),
          "metadata preview validates the actual committed root comparison");
  note.modeled_raw_history = true;
  operations.push_back(note);
  const auto* update = readback(operations, false);
  Require(
      update->failures == 2 && update->scans.size() == 1,
      "child projection preserves once-only failures and absent root history");
  Require(
      update->scans[0].target == "child.local" &&
          update->scans[0].read == "1001" &&
          update->scans[0].miscompare == "1001",
      "child history comes from actual root return, not input or expectation");
  Require(readback(operations, false)->scans.empty(),
          "child history deltas publish exactly once");
}
void TestStatusPreviewRetainsUnpublishedHistory() {
  pattern::Design design;
  design.AddIcl("preview.icl", R"(
Module Top {
 ScanInPort si;
 ScanRegister r[3:0] {ScanInSource si; CaptureSource r; ResetValue 0;}
 ScanOutPort so {Source r[0];}
}
)");
  const auto netlist = pattern::Elaborate(design, "Top");
  pattern::Runtime::StatusReadback preview;
  const auto readback = pattern::MakeModeledReadback(netlist, false, &preview);
  std::vector<pattern::Operation> operations{
      {"iState", {"r", "15"}, "test", {}},
      {"iRead", {"r", "0"}, "test", {}},
      {"iApply", {}, "test", {}}};
  Require(preview(operations) == 4 && preview(operations) == 4,
          "status previews do not recount failures");
  const auto* update = readback(operations, false);
  Require(update->bits.size() == 4 && update->failures == 4,
          "ordinary publication retains all previewed observations");
  for (const auto& bit : update->bits) {
    Require(bit.read == '1' && bit.miscompare == '1',
            "preview does not consume actual read or miscompare values");
  }
  operations.push_back({"iState", {"r", "15"}, "test", {}});
  operations.push_back({"iRead", {"r", "14"}, "test", {}});
  operations.push_back({"iApply", {}, "test", {}});
  Require(preview(operations) == 5, "preview advances only new comparisons");
  operations.push_back({"iState", {"r", "0", "-LastReadValue"}, "test", {}});
  update = readback(operations, false);
  Require(update->bits.size() == 4 && update->failures == 5,
          "unpublished deltas survive a later status preview");
  for (const auto& bit : update->bits) {
    Require(bit.read == '?' && bit.miscompare == (bit.index == 0 ? '1' : '0'),
            "later history assignments suppress only their unpublished plane");
  }
  Require(readback(operations, false)->bits.empty(),
          "published deltas are consumed exactly once");
}
void TestOrderedStatusClearEvents() {
  const auto note = [](const std::string& count) {
    return pattern::Operation{
        "iNote", {"-comment", pattern::StatusClearNoteText(count)}, "test", {}};
  };
  Require(pattern::ReadStatusClearNote(note("0")) == 0 &&
              pattern::ReadStatusClearNote(note("18446744073709551615")) ==
                  std::numeric_limits<uint64_t>::max(),
          "status-clear parser accepts canonical uint64 boundaries");
  for (const auto& text : {"", "-1", "04", "4 extra", "18446744073709551616"}) {
    Require(!pattern::ReadStatusClearNote(note(text)),
            "malformed or overflowing status counts stay informational");
  }
  pattern::Design design;
  design.AddIcl("clear.icl", R"(
Module Top {
 DataInPort p[3:0]; DataOutPort q[3:0] {Source p;}
})");
  const auto netlist = pattern::Elaborate(design, "Top");
  const auto readback = pattern::MakeModeledReadback(netlist);
  std::vector<pattern::Operation> operations{
      {"iWrite", {"p", "15"}, "test", {}},
      {"iApply", {}, "test", {}},
      {"iRead", {"q", "0"}, "test", {}},
      {"iApply", {}, "test", {}}};
  Require(readback(operations, false)->failures == 4,
          "four actual failures precede the ordered clear");
  operations.push_back({"iGetStatus", {"-clear", "4"}, "test", {}});
  operations.push_back({"iRead", {"q", "14"}, "test", {}});
  operations.push_back({"iApply", {}, "test", {}});
  Require(readback(operations, false)->failures == 1,
          "clear retires earlier failures before later comparisons");
  Require(readback(operations, false)->failures == 1,
          "an already consumed clear never clears a later failure");
  const auto unknown = pattern::MakeModeledReadback(netlist);
  operations = {{"iRead", {"q", "0"}, "test", {}},
                {"iApply", {}, "test", {}},
                {"iGetStatus", {"-clear", "0"}, "test", {}}};
  Require(!unknown(operations, false)->failures,
          "internal clear cannot invent a known count from unknown data");
}
void TestMergeMarkersAreEventFree() {
  pattern::Design design;
  design.AddIcl("merge_events.icl", R"(
Module Top {
 DataInPort p;
 DataRegister d {WriteDataSource p; WriteEnSource 1'b1; ResetValue 1;}
 DataOutPort q {Source d;}
})");
  const auto netlist = pattern::Elaborate(design, "Top");
  const auto readback = pattern::MakeModeledReadback(netlist);
  std::vector<pattern::Operation> operations{
      {"iState", {"p", "1"}, "test", {}},
      {"iState", {"d", "0"}, "test", {}},
      {"iMerge", {"-begin"}, "test", {}},
      {"iRead", {"q", "0"}, "test", {"reader"}},
      {"iApply", {}, "test", {"reader"}}};
  const auto* update = readback(operations, false);
  Require(update && update->bits.size() == 1 &&
              update->bits.front().read == '0' && update->failures == 0,
          "open merge prefix preserves event-free assumptions");
  operations.push_back({"iMerge", {"-end"}, "test", {}});
  operations.push_back({"iRead", {"q", "0"}, "test", {}});
  operations.push_back({"iApply", {}, "test", {}});
  update = readback(operations, false);
  Require(update && update->bits.size() == 1 &&
              update->bits.front().read == '0' && update->failures == 0,
          "merge end creates no input, reset or data-load event");
  update = readback(operations, false);
  Require(update && update->bits.empty() && update->failures == 0,
          "repeated merged query produces no stale delta");
}

void TestMergedRawReadbackPlanes() {
  pattern::Design design;
  design.AddIcl("merge_raw.icl", R"(
Module Top {
 ScanInPort si; ScanOutPort so {Source value[0];}
 ScanRegister value[3:0] {ScanInSource si; CaptureSource value; ResetValue 0;}
 SelectPort sel; CaptureEnPort ce; ShiftEnPort se; UpdateEnPort ue;
 TCKPort tck; ResetPort reset;
 ScanInterface client {Port si; Port so; Port sel; Port ce; Port se; Port ue;
  Port tck; Port reset; DefaultLoadValue 4'b0;}
})");
  const auto netlist = pattern::Elaborate(design, "Top");
  const auto readback = pattern::MakeModeledReadback(netlist);
  std::vector<pattern::Operation> operations{
      {"iReset", {}, "test", {}},
      {"iWrite", {"value", "10"}, "test", {}},
      {"iApply", {}, "test", {}},
      {"iRead", {"value", "0"}, "test", {}},
      {"iApply", {}, "test", {}},
      {"iMerge", {"-begin"}, "test", {}},
      {"iScan", {"client", "4", "-si", "3", "-so", "0"}, "test", {"raw"}},
      {"iApply", {}, "test", {"raw"}}};
  const auto* update = readback(operations, false);
  Require(update && update->bits.size() == 4 && update->scans.size() == 1 &&
              update->scans.front().read == "1010" && update->failures == 4,
          "merged raw and logical planes use actual asymmetric captured words");
  std::string logical(4, '?');
  for (const auto& bit : update->bits) logical.at(3 - bit.index) = bit.read;
  Require(logical == "1010",
          "logical history remains independent of raw writes");
  update = readback(operations, true);
  Require(update && update->bits.empty() && update->scans.empty() &&
              update->failures == 4,
          "merged clear returns old count without repeating observations");
  operations.push_back({"iMerge", {"-end"}, "test", {}});
  operations.push_back({"iRead", {"value", "0"}, "test", {}});
  operations.push_back({"iApply", {}, "test", {}});
  update = readback(operations, false);
  Require(update && update->bits.size() == 4 && update->scans.empty() &&
              update->failures == 2,
          "post-merge logical read counts only new cared failures");
  logical.assign(4, '?');
  for (const auto& bit : update->bits) logical.at(3 - bit.index) = bit.read;
  Require(logical == "0011", "logical plane observes the new raw update value");
}

void TestMergedReadbackGuardPreflight() {
  for (bool physical : {false, true}) {
    pattern::Design design;
    design.AddIcl("guard.icl", physical ? R"(
Module Top {DataInPort p; DataOutPort q {Source p;}}
)"
                                        : R"(
Module Top {
 ScanInPort si; ScanOutPort so; SelectPort sel; CaptureEnPort ce;
 ShiftEnPort se; UpdateEnPort ue; TCKPort tck; ResetPort reset;
 ScanInterface client {Port si; Port so; Port sel; Port ce; Port se; Port ue;
  Port tck; Port reset; DefaultLoadValue 4'b0;}
})");
    const auto netlist = pattern::Elaborate(design, "Top");
    const auto readback = pattern::MakeModeledReadback(netlist, physical);
    bool rejected = false;
    try {
      readback({{"iMerge", {"-begin"}, "test", {}}}, false);
    } catch (const std::runtime_error& error) {
      rejected = std::string(error.what()).find("iMerge histories") !=
                 std::string::npos;
    }
    Require(rejected, "physical and opaque merge histories remain guarded");
  }
}

void TestWriteCallbackEndpointReadback() {
  pattern::Design design;
  design.AddIcl("write_callback_readback.icl", R"(
Module Top {
 DataInPort p[3:0]; DataOutPort q[3:0] {Source p;}
 DataRegister value[3:0] {WriteCallBack Top store <D> <R>;}
})");
  const auto netlist = pattern::Elaborate(design, "Top");
  const auto readback = pattern::MakeModeledReadback(netlist);
  const std::vector<pattern::Operation> ops{
      {"iReset", {}, "test", {}},
      {"iWrite", {"value", "9"}, "test", {}, true},
      {"iWrite", {"p", "9"}, "callback", {}, false, true},
      {"iApply", {}, "callback", {}, false, true},
      {"iState",
       {"value", "9", "-LastWrittenValue"},
       "callback",
       {},
       false,
       true},
      {"iApply", {}, "test", {}, false, true},
      {"iRead", {"q", "0"}, "test", {}},
      {"iApply", {}, "test", {}}};
  const auto* updates = readback(ops, true);
  std::string actual, mismatches;
  for (const auto& update : updates->bits) {
    actual += update.read;
    mismatches += update.miscompare;
  }
  Require(actual == "1001" && mismatches == "1001" && updates->failures == 2,
          "write callback compares actual endpoint data and counts once");
  Require(readback(ops, false)->failures == 0,
          "callback endpoint status clears");
}

void TestCallbackAssumptionsCannotBecomeActualData() {
  pattern::Design design;
  design.AddIcl("callback_shadow.icl", R"(
Module Top {
 DataInPort p[3:0]; DataOutPort q[3:0] {Source p;}
 DataRegister value[3:0] {WriteCallBack Top store <D>; ResetValue 0;}
 DataOutPort shadow[3:0] {Source value;}
 Alias mixed[7:0] = value, p;
})");
  const auto netlist = pattern::Elaborate(design, "Top");
  const auto readback = pattern::MakeModeledReadback(netlist);
  std::vector<pattern::Operation> ops{
      {"iReset", {}, "test", {}},
      {"iState", {"mixed", "0b10011010", "-LastWrittenValue"}, "test", {}},
      {"iRead", {"q", "0"}, "test", {}},
      {"iRead", {"shadow", "0"}, "test", {}},
      {"iApply", {}, "test", {}}};
  const auto check = [&](const pattern::ReadbackUpdates* updates,
                         const std::string& expected) {
    std::map<std::pair<size_t, int64_t>, char> values;
    for (const auto& bit : updates->bits) {
      values.emplace(std::make_pair(bit.node, bit.index), bit.read);
    }
    const auto actual = [&](const std::string& name) {
      std::string result;
      for (const auto& bit : pattern::ResolvePdlTarget(netlist, name).bits) {
        result += values.at({bit.node, bit.index});
      }
      return result;
    };
    Require(actual("q") == expected && actual("shadow") == "xxxx" &&
                !updates->failures,
            "mixed aliases preserve ordinary state but abstract storage is "
            "unknown");
  };
  check(readback(ops, false), "1010");
  ops.push_back({"iReset", {}, "test", {}});
  ops.push_back({"iState", {"p", "3"}, "test", {}});
  ops.push_back({"iRead", {"q", "0"}, "test", {}});
  ops.push_back({"iRead", {"shadow", "0"}, "test", {}});
  ops.push_back({"iApply", {}, "test", {}});
  check(readback(ops, false), "0011");
}

void TestUnexpandedLogicalReadbackRejects() {
  pattern::Design design;
  design.AddIcl("unexpanded_callback.icl", R"(
Module Top {
 DataInPort p[3:0];
 DataRegister observed[3:0] {ReadCallBack Top load <D>; ReadDataSource p;}
})");
  const auto netlist = pattern::Elaborate(design, "Top");
  const auto readback = pattern::MakeModeledReadback(netlist);
  bool rejected = false;
  try {
    readback({{"iState", {"p", "9"}, "test", {}},
              {"iRead", {"observed", "0"}, "test", {}},
              {"iApply", {}, "test", {}}},
             false);
  } catch (const std::runtime_error& error) {
    rejected = std::string(error.what()).find("callback") != std::string::npos;
  }
  Require(rejected,
          "unexpanded logical callback access cannot impersonate an endpoint");
}

void TestRawObservationCommit() {
  pattern::Design design;
  design.AddIcl("raw.icl", R"(
Module Top {
 ScanInPort si; ScanOutPort so {Source r[0];}
 SelectPort sel; CaptureEnPort ce; ShiftEnPort se; UpdateEnPort ue; TCKPort ck;
 ScanRegister r[3:0] {ScanInSource si; CaptureSource r; ResetValue 0;}
 ScanInterface client {Port si; Port so; Port sel; Port ce; Port se; Port ue; Port ck;}
})");
  const auto netlist = pattern::Elaborate(design, "Top");
  pattern::ScanScheduler scheduler(netlist);
  scheduler.Reset();
  scheduler.Assume("r", "1001");
  std::set<size_t> assigned;
  std::vector<pattern::ScanStep> preparations;
  bool thrown = false;
  try {
    scheduler.ApplyScan(
        {"iScan", {"client", "4", "-si", "6", "-so", "0"}, "raw", {}}, {},
        &assigned, &preparations, [&](const auto& operation, const auto& bits) {
          Require(bits == "1001" && operation.arguments[3] == "0b0110",
                  "raw callback carries actual return and canonical request");
          throw std::runtime_error("raw listener failure");
        });
  } catch (const std::runtime_error& error) {
    thrown = std::string(error.what()).find("raw listener failure") !=
             std::string::npos;
  }
  Require(thrown && scheduler.Value("r") == "0110" &&
              assigned.count(netlist.node_ids.at("r")) && preparations.empty(),
          "raw subscriber failure retains committed state and outputs");
  std::string actual;
  scheduler.ApplyScan({"iScan", {"client", "4", "-si", "0"}, "next", {}}, {},
                      nullptr, nullptr,
                      [&](const auto&, const auto& bits) { actual = bits; });
  Require(actual == "0110" && scheduler.Value("r") == "0000",
          "next raw call uses committed state after callback error");
  const auto readback = pattern::MakeModeledReadback(netlist);
  const auto* updates =
      readback({{"iReset", {}, "raw", {}},
                {"iWrite", {"r", "9"}, "raw", {}},
                {"iApply", {}, "raw", {}},
                {"iScan", {"client", "4", "-si", "6", "-so", "0"}, "raw", {}},
                {"iApply", {}, "raw", {}}},
               false);
  Require(updates && updates->bits.empty() && updates->scans.size() == 1 &&
              updates->scans[0].read == "1001" &&
              updates->scans[0].miscompare == "1001" && updates->failures == 2,
          "raw adapter observes independently without register history");
}
void TestRawTapObservationCommit() {
  const std::string folder =
      "tests/cases/ours/pattern_generation/raw_tap_builtin_dr/";
  const auto read = [&](const std::string& name) {
    std::ifstream stream(folder + name);
    Require(stream.good(), "raw TAP observer fixture opens");
    return std::string((std::istreambuf_iterator<char>(stream)), {});
  };
  pattern::Design design;
  design.AddIcl("raw-tap.icl", read("model.icl"));
  design.AddBsdl("raw-tap.bsdl", read("model.bsdl"), "BuiltinDrProbe");
  const auto network = pattern::ElaborateTapNetwork(design, "BuiltinDrTop");
  for (bool literal : {false, true}) {
    pattern::ScanScheduler scheduler(network.netlist);
    scheduler.Reset();
    scheduler.ApplyScan(
        {"iScan", {"-ir", "jtag", "4", "-si", "3"}, "setup", {}});
    scheduler.Assume("core.r", "00001001");
    std::set<size_t> assigned;
    std::vector<pattern::ScanStep> preparations;
    bool thrown = false;
    const std::string committed = literal ? "0x1001x0" : "00000110";
    try {
      scheduler.ApplyScan(
          {"iScan",
           {"jtag", "8", "-si", "0b" + committed, "-so", "0"},
           "raw-tap",
           {}},
          {}, &assigned, &preparations, [&](const auto&, const auto& bits) {
            Require(bits == "00001001", "raw TAP observes actual old data");
            throw std::runtime_error("raw TAP listener failure");
          });
    } catch (const std::runtime_error& error) {
      thrown = std::string(error.what()).find("raw TAP listener failure") !=
               std::string::npos;
    }
    Require(thrown && scheduler.Value("core.r") == committed &&
                assigned.count(network.netlist.node_ids.at("core.r")) &&
                preparations.empty(),
            "raw TAP subscriber failure preserves committed transaction");
    std::string actual;
    scheduler.ApplyScan({"iScan", {"jtag", "8", "-si", "0"}, "next", {}}, {},
                        nullptr, nullptr,
                        [&](const auto&, const auto& bits) { actual = bits; });
    Require(actual == committed && scheduler.Value("core.r") == "00000000",
            "raw TAP follow-up observes committed binary/literal state");
  }
}
void TestCallbackResetRetainsActualModelReadback() {
  pattern::Design design;
  design.AddIcl("callback-model-reset.icl", R"(
Module Top {
 DataInPort p[3:0]; DataOutPort q[3:0] {Source p;}
 DataRegister observed[3:0] {ReadCallBack Top load <D> <R>; ReadDataSource p;}
})");
  const auto netlist = pattern::Elaborate(design, "Top");
  auto readback = pattern::MakeModeledReadback(netlist);
  std::vector<pattern::Operation> operations{
      {"iRead", {"observed", "9"}, "test", {}, true},
      {"iReset", {}, "callback", {}, false, true, true},
      {"iWrite", {"p", "0"}, "callback", {}, false, true, true},
      {"iApply", {}, "callback", {}, false, true, true},
      {"iRead", {"q", "9"}, "callback", {}, false, true, true},
      {"iApply", {}, "callback", {}, false, true, true}};
  const auto* delta = readback(operations, false);
  Require(delta && delta->bits.size() == 4 && delta->failures == 2,
          "read callback reset retains actual endpoint mismatches");
  const auto output = netlist.node_ids.at("q");
  for (const auto& bit : delta->bits) {
    Require(
        bit.node == output && bit.read == '0' &&
            bit.miscompare == (bit.index == 3 || bit.index == 0 ? '1' : '0'),
        "callback expected9 never becomes an observed value");
  }
  const auto* cleared = readback(operations, true);
  Require(cleared->bits.empty() && cleared->failures == 2,
          "callback endpoint history is incremental and clears status");
  Require(readback(operations, false)->failures == 0,
          "callback endpoint status remains cleared");
}
void TestCallbackHistoryBoundary() {
  pattern::Design design;
  design.AddIcl("callback-history.icl", R"(
Module Top {
 DataInPort p;
 DataRegister value {WriteCallBack Top store <D>;}
})");
  const auto netlist = pattern::Elaborate(design, "Top");
  const auto canceled = pattern::MakeModeledReadback(netlist);
  const auto* delta =
      canceled({{"iWrite", {"value", "1"}, "callback", {}, true},
                {"iReset", {}, "cancel", {}}},
               false);
  Require(delta && delta->bits.empty() && delta->failures == 0,
          "canceled callback marker produces no observation or failure");
  const auto readback = pattern::MakeModeledReadback(netlist);
  bool rejected = false;
  try {
    readback({{"iWrite", {"value", "1"}, "callback", {}, true},
              {"iApply", {}, "callback apply", {}}},
             false);
  } catch (const std::runtime_error& error) {
    rejected = std::string(error.what()).find("callback") != std::string::npos;
  }
  Require(rejected, "logical callback data cannot become modeled observations");
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
  for (const std::string command : {"iMerge", "iClock"}) {
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
void TestSerialEventOracle() {
  pattern::Design design;
  design.AddIcl("serial.icl", R"(
Module Top {
 ScanInPort si;
 ScanRegister first[3:0] {ScanInSource si; CaptureSource first; ResetValue 0;}
 ScanRegister last[1:0] {ScanInSource ~first[0]; CaptureSource last; ResetValue 0;}
 ScanOutPort so {Source ~last[0];}
})");
  const auto netlist = pattern::Elaborate(design, "Top");
  pattern::ScanScheduler scheduler(netlist);
  scheduler.Reset();
  scheduler.Assume("first", "1001");
  scheduler.Assume("last", "10");
  std::map<std::pair<size_t, int64_t>, char> observed;
  const auto steps = scheduler.Apply(
      {{"iRead", {"first", "0"}, "test", {}},
       {"iRead", {"last", "0"}, "test", {}},
       {"iWrite", {"first", "6"}, "test", {}}},
      false, {}, true, false, [&](const auto& bits, const auto& values) {
        for (size_t i = 0; i < bits.size(); ++i) {
          observed.emplace(std::make_pair(bits[i].node, bits[i].index),
                           values[i]);
        }
      });
  pattern::NetworkState oracle(netlist);
  oracle.SetGlobalReset('0', '0');
  oracle.Assume(netlist.node_ids.at("first"), "1001");
  oracle.Assume(netlist.node_ids.at("last"), "10");
  std::string returned;
  for (const auto& step : steps) {
    std::map<size_t, pattern::ScanControl> controls;
    for (const auto& [name, bits] : step.loads) {
      controls[netlist.node_ids.at(name)] = {'1', '0', '0', '1'};
    }
    oracle.Clock(controls, true);
    for (auto& [id, control] : controls) control = {'0', '1', '0', '1'};
    for (char input : step.serial) {
      oracle.Drive(netlist.node_ids.at("si"), std::string(1, input));
      returned += oracle.Read({netlist.node_ids.at("so"), 0}, true);
      oracle.Clock(controls, true);
    }
    for (auto& [id, control] : controls) control = {'0', '0', '1', '1'};
    oracle.Clock(controls, false);
  }
  Require(returned == "101001",
          "independent pre-edge wire oracle has literal return bits");
  Require(observed.size() == 6,
          "listener delivers all six requested logical bits");
  const auto first = netlist.node_ids.at("first"),
             last = netlist.node_ids.at("last");
  Require(
      observed.at({first, 3}) == '1' && observed.at({first, 2}) == '0' &&
          observed.at({first, 1}) == '0' && observed.at({first, 0}) == '1' &&
          observed.at({last, 1}) == '1' && observed.at({last, 0}) == '0',
      "logical observations decode actual wire order and both path inversions");
  Require(scheduler.Value("first") == "0110",
          "observation precedes the new update");
}
void TestFastObserverExceptions() {
  for (bool native : {true, false}) {
    const std::string path = native ? "tests/cases/ours/pattern_generation/"
                                      "native_ordinary_scans/model.icl"
                                    : "tests/cases/ours/pattern_generation/"
                                      "control_mux_operations/capture.icl";
    std::ifstream stream(path);
    Require(stream.good(), "observer fixture opens");
    const std::string source((std::istreambuf_iterator<char>(stream)), {});
    pattern::Design design;
    design.AddIcl(path, source);
    const auto netlist =
        pattern::Elaborate(design, native ? "NativeOrdinaryTop" : "ClientTop");
    pattern::ScanScheduler scheduler(netlist);
    scheduler.Reset();
    scheduler.Assume(native ? "open" : "cap_enable", "1");
    scheduler.Assume("child.value", native ? "00001001" : "1001");
    CheckCommittedObserverFailure(&scheduler, netlist, "child.value",
                                  native ? "00000110" : "1001",
                                  native ? "00000110" : "1010", false, native);
  }
}
void TestObserverFailureInvalidatesRoute() {
  const std::string path =
      "tests/cases/ours/pattern_generation/native_ordinary_scans/model.icl";
  std::ifstream stream(path);
  Require(stream.good(), "route-change fixture opens");
  pattern::Design design;
  design.AddIcl(path,
                std::string((std::istreambuf_iterator<char>(stream)), {}));
  const auto netlist = pattern::Elaborate(design, "NativeOrdinaryTop");
  pattern::ScanScheduler scheduler(netlist);
  scheduler.Reset();
  bool thrown = false;
  try {
    scheduler.Apply({{"iWrite", {"open", "1"}, "test", {}},
                     {"iRead", {"child.value", "0"}, "test", {}},
                     {"iWrite", {"child.value", "6"}, "test", {}}},
                    false, {}, true, false, [](const auto&, const auto&) {
                      throw std::runtime_error("route listener failure");
                    });
  } catch (const std::runtime_error& error) {
    thrown = std::string(error.what()).find("route listener failure") !=
             std::string::npos;
  }
  Require(
      thrown && scheduler.Value("open") == "1" &&
          scheduler.Value("child.value") == "00000110",
      "route-changing transaction remains committed after subscriber failure");
  const auto normalized = scheduler.NormalizeScan(
      {"iScan", {"client", "9", "-si", "0"}, "after observer failure", {}});
  Require(normalized.arguments[1] == "9" &&
              normalized.arguments[3] == "0b000000000",
          "raw normalization uses the newly committed open route");
}
void TestGeneralObserverException() {
  pattern::Design design;
  design.AddIcl("general.icl", R"(
Module Top {
 ScanInPort si;
 ScanRegister r[3:0] {ScanInSource si; CaptureSource r; ResetValue 0;}
 ScanOutPort so {Source r[0];}
})");
  const auto netlist = pattern::Elaborate(design, "Top");
  pattern::ScanScheduler scheduler(netlist);
  scheduler.Reset();
  scheduler.Assume("r", "1001");
  CheckCommittedObserverFailure(&scheduler, netlist, "r", "0110", "0110", false,
                                true);
}
void TestTapReturnOracle() {
  const std::string directory =
      "tests/cases/ours/pattern_generation/access_link/";
  const auto read = [](const std::string& path) {
    std::ifstream stream(path);
    Require(stream.good(), "TAP oracle fixture opens");
    return std::string((std::istreambuf_iterator<char>(stream)), {});
  };
  auto source = read(directory + "simple.icl");
  const std::string declaration = "ScanRegister r[7:0] { ScanInSource si; }";
  source.replace(
      source.find(declaration), declaration.size(),
      "ScanRegister r[7:0] {ScanInSource si; CaptureSource r; ResetValue 0;}");
  pattern::Design design;
  design.AddIcl("tap.icl", source);
  design.AddBsdl("tap.bsdl", read(directory + "simple.bsdl"), "AccessProbe");
  const auto network = pattern::ElaborateTapNetwork(design, "AccessTop");
  const auto& netlist = network.netlist;
  const auto id = netlist.node_ids.at("core.r");
  pattern::ScanScheduler scheduler(netlist);
  scheduler.Reset();
  scheduler.Assume("core.r", "00001001");
  std::map<int64_t, char> observed;
  const auto steps = scheduler.Apply(
      {{"iRead", {"core.r", "0"}, "test", {}},
       {"iWrite", {"core.r", "6"}, "test", {}}},
      false, {}, true, false, [&](const auto& bits, const auto& values) {
        for (size_t i = 0; i < bits.size(); ++i) {
          if (bits[i].node == id) observed.emplace(bits[i].index, values[i]);
        }
      });
  pattern::TapExecution oracle(netlist);
  const auto cycle = [&](char tms, char input = '0',
                         std::string* returned = nullptr) {
    oracle.Drive("TMS", std::string(1, tms));
    oracle.Drive("TDI", std::string(1, input));
    if (returned) *returned += oracle.ReadTdo("tap");
    oracle.RisingEdge("TCK");
    oracle.FallingEdge("TCK");
  };
  oracle.Drive("TRST", "0");
  oracle.Drive("TRST", "1");
  cycle('0');
  oracle.Assume(id, "00001001");
  std::string data_return;
  for (const auto& step : steps) {
    for (char tms : std::string(step.instruction ? "1100" : "100")) cycle(tms);
    std::string returned;
    for (size_t tick = 0; tick < step.serial.size(); ++tick) {
      cycle(tick + 1 == step.serial.size() ? '1' : '0', step.serial[tick],
            &returned);
    }
    cycle('1');
    cycle('0');
    if (!step.instruction) data_return = returned;
  }
  Require(
      data_return == "10010000" && oracle.Value("core.r") == "00000110",
      "independent TAP edge replay returns old9 LSB first before updating6");
  Require(observed.size() == 8,
          "TAP listener delivers all eight requested data bits");
  for (int64_t bit = 0; bit < 8; ++bit) {
    Require(observed.at(bit) == (bit == 0 || bit == 3 ? '1' : '0'),
            "logical observations agree with independently sampled TDO");
  }
  for (bool physical : {false, true}) {
    pattern::ScanScheduler throwing(netlist);
    throwing.Reset(false, physical);
    throwing.Apply({{"iWrite", {"tap.INSTRUCTION", "3"}, "test", {}}}, false,
                   {}, true, physical);
    throwing.Assume("core.r", "00001001");
    CheckCommittedObserverFailure(&throwing, netlist, "core.r", "00000110",
                                  "00000110", physical, true);
  }
}
void TestDataReadbackPlanes() {
  pattern::Design design;
  design.AddIcl("data.icl", R"(
Module Top {
 DataInPort data[3:0]; DataInPort enable;
 DataRegister value[3:0] {WriteDataSource data; WriteEnSource enable; ResetValue 0;}
 DataOutPort q[3:0] {Source value;}
})");
  const auto netlist = pattern::Elaborate(design, "Top");
  const auto readback = pattern::MakeModeledReadback(netlist);
  const std::vector<pattern::Operation> operations{
      {"iState", {"enable", "0"}, "test", {}},
      {"iState", {"value", "9"}, "test", {}},
      {"iRead", {"value", "0"}, "test", {}},
      {"iApply", {}, "test", {}},
      {"iState", {"value", "15", "-LastReadValue"}, "test", {}}};
  const auto* updates = readback(operations, false);
  Require(updates && updates->failures == 2 && updates->bits.size() == 4,
          "stored1001 versus expected0000 counts two actual failures");
  for (const auto& bit : updates->bits) {
    Require(
        bit.node == netlist.node_ids.at("value") && bit.read == '?' &&
            bit.miscompare == (bit.index == 0 || bit.index == 3 ? '1' : '0'),
        "later manual read history suppresses only the older read plane");
  }
}
void TestDataFrameEventOracle() {
  pattern::Design design;
  design.AddIcl("events.icl", R"(
Module Top {
 DataInPort data[3:0]; DataInPort enable;
 DataRegister value[3:0] {WriteDataSource data; WriteEnSource enable; ResetValue 0;}
 DataOutPort q[3:0] {Source value;}
})");
  const auto netlist = pattern::Elaborate(design, "Top");
  pattern::ScanScheduler scheduler(netlist);
  scheduler.Reset();
  scheduler.Assume("enable", "0");
  scheduler.Assume("data", "0000");
  scheduler.Assume("value", "1010");
  std::map<int64_t, char> observed;
  const auto steps = scheduler.Apply(
      {{"iRead", {"value", "0"}, "test", {}},
       {"iWrite", {"value", "3"}, "test", {}}},
      false, {}, true, false, [&](const auto& bits, const auto& values) {
        for (size_t i = 0; i < bits.size(); ++i) {
          Require(bits[i].node == netlist.node_ids.at("value"),
                  "listener identifies the logical stored target");
          observed.emplace(bits[i].index, values[i]);
        }
      });
  pattern::NetworkState oracle(netlist);
  oracle.Reset();
  oracle.SetGlobalReset('0', '0');
  oracle.Assume(netlist.node_ids.at("enable"), "0");
  oracle.Assume(netlist.node_ids.at("data"), "0000");
  oracle.Assume(netlist.node_ids.at("value"), "1010");
  size_t reads = 0;
  for (const auto& step : steps) {
    std::map<size_t, std::string> pins;
    for (const auto& [name, bits] : step.writes) {
      const auto id = netlist.node_ids.at(name);
      if (netlist.nodes[id].model == pattern::ModelKind::kPort) {
        pins.emplace(id, bits);
      }
    }
    if (!pins.empty()) oracle.ApplyInputs(pins);
    for (const auto& [name, bits] : step.writes) {
      const auto id = netlist.node_ids.at(name);
      if (netlist.nodes[id].model == pattern::ModelKind::kRegister) {
        Require(oracle.Value(id) == bits,
                "goal markers describe independently reached storage");
      }
    }
    if (step.expected.count("q")) {
      ++reads;
      Require(oracle.Value(netlist.node_ids.at("q")) == "1010",
              "actual physical output is old data at the selected read frame");
    }
  }
  Require(reads == 1 && observed.size() == 4,
          "one complete stored observation is delivered");
  for (const auto& [index, value] : observed) {
    Require(value == ((10 >> index) & 1 ? '1' : '0'),
            "callback matches independently observed physical output");
  }
  Require(oracle.Value(netlist.node_ids.at("value")) == "0011" &&
              scheduler.Value("value") == "0011",
          "actual pin events reach final3 after observing old10");
}
}  // namespace
int main() {
  try {
    TestDynamicChildCommittedGeometry();
    TestModeledChildHistoryProjection();
    TestStatusPreviewRetainsUnpublishedHistory();
    TestOrderedStatusClearEvents();
    TestMergeMarkersAreEventFree();
    TestMergedRawReadbackPlanes();
    TestMergedReadbackGuardPreflight();
    TestWriteCallbackEndpointReadback();
    TestCallbackAssumptionsCannotBecomeActualData();
    TestUnexpandedLogicalReadbackRejects();
    TestRawObservationCommit();
    TestRawTapObservationCommit();
    TestCallbackResetRetainsActualModelReadback();
    TestCallbackHistoryBoundary();
    TestReadModel();
    TestModeledReadback();
    TestScheduledObservations();
    TestSerialEventOracle();
    TestFastObserverExceptions();
    TestGeneralObserverException();
    TestObserverFailureInvalidatesRoute();
    TestTapReturnOracle();
    TestDataReadbackPlanes();
    TestDataFrameEventOracle();
    std::cout << "readback tests passed\n";
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
