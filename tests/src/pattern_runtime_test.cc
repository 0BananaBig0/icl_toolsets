// Copyright 2026 The ICL Toolsets Authors.
// SPDX-License-Identifier: MIT
#include "pattern/pattern_runtime.h"

#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <string>

#include "pattern/pattern_netlist.h"
#include "pattern/pattern_state.h"
#include "pattern/pattern_tap.h"

namespace {
void Require(bool value, const std::string& message) {
  if (!value) throw std::runtime_error(message);
}
template <typename Action>
void Fails(Action action, const std::string& expected) {
  try {
    action();
  } catch (const std::runtime_error& error) {
    Require(std::string(error.what()).find(expected) != std::string::npos,
            "expected " + expected + ", got " + error.what());
    return;
  }
  throw std::runtime_error("expected error: " + expected);
}
const std::string kCallbackModel = R"(
Module Leaf {
 Parameter Tag = "leaf";
 DataRegister value[3:0] {WriteCallBack ::Top::api store <D> <R> "<D>" 8'b10x1;}
}
Module Top {
 Parameter Tag = "root";
 DataInPort p[3:0]; DataOutPort q[3:0] {Source p;}
 Instance first Of Leaf;
 Alias low[1:0] = first.value[1:0];
})";
void TestCallbackQueueAndBinding() {
  pattern::Design design;
  design.AddIcl("callback.icl", kCallbackModel);
  const auto netlist = pattern::Elaborate(design, "Top");
  pattern::Runtime runtime(design, netlist, true);
  runtime.Execute("callback.pdl", R"(
iWrite first.value 3; iReset
iProcsForModule Top -iProcNameSpace api
iProc store {data reg literal number} {
 if {$Tag ne "root" || $literal ne "<D>" || $number ne "0b000010x1"} {
  error {wrong callback arguments or ancestor parameters}
 }
 iNote "$reg=$data"
 iWrite p $data
}
iWrite low 2; iWrite first.value 9; iApply
iWrite p 8; iApply
)");
  runtime.Finish();
  size_t markers = 0, notes = 0, endpoints = 0, states = 0;
  for (const auto& op : runtime.operations()) {
    if (op.callback) {
      ++markers;
      continue;
    }
    if (op.command == "iNote") {
      ++notes;
      Require(op.arguments == std::vector<std::string>{"first.value=0b1001"},
              "callback R/data resolve relative to real ancestor");
    }
    if (op.command == "iWrite") {
      ++endpoints;
      Require(op.arguments[0] == "p", "callback frame did not leak prefix");
    }
    if (op.command == "iState") {
      ++states;
      Require(op.arguments[0] == "first.value" && op.arguments[1] == "0b1001",
              "callback declares successful logical write postcondition");
    }
  }
  Require(
      markers == 3 && notes == 1 && endpoints == 2 && states == 1,
      "latest callback coalesces once, reset cancels, ordinary path retained");
  pattern::Runtime retry(design, netlist);
  retry.Execute("retry.pdl", R"(
iWrite first.value 9
if {![catch {iApply} message] || [string first {undefined iProc} $message] < 0} {
 error {missing callback preflight did not fail}
}
iProcsForModule Top -iProcNameSpace api
iProc store {data reg literal number} {iWrite p $data}
iApply
)");
  retry.Finish();
}
void TestCallbackFailureRecovery() {
  pattern::Design design;
  design.AddIcl("callback-error.icl", kCallbackModel);
  const auto netlist = pattern::Elaborate(design, "Top");
  const std::string source = R"(
iProcsForModule Top -iProcNameSpace api
iProc store {data reg literal number} {
 iWrite p $data; iApply
 iWrite p 5; error broken
}
iWrite first.value 9
if {![catch {iApply} message] || [string first broken $message] < 0} {
 error {callback body error missing}
}
if {![catch {iApply} message] || [string first iReset $message] < 0} {
 error {unfinished callback endpoint queue flushed}
}
)";
  pattern::Runtime runtime(design, netlist);
  runtime.Execute("failure.pdl", source + R"(
iReset
iProc store {data reg literal number} {iWrite p $data}
iWrite first.value 6; iApply
)");
  runtime.Finish();
  size_t states = 0;
  for (const auto& op : runtime.operations()) {
    if (op.command != "iState") continue;
    ++states;
    Require(op.arguments[1] == "0b0110",
            "failed callback cannot publish logical write postcondition");
  }
  Require(states == 1, "only recovered callback declares a postcondition");
  pattern::Runtime blocked(design, netlist);
  blocked.Execute("blocked.pdl", source);
  Fails([&] { blocked.Finish(); }, "iReset");
}
void TestCaughtCallbackReturnsToCaller() {
  pattern::Design design;
  design.AddIcl("callback-wrapper.icl", kCallbackModel);
  const auto netlist = pattern::Elaborate(design, "Top");
  pattern::Runtime runtime(design, netlist);
  runtime.Execute("wrapper.pdl", R"(
iProcsForModule Top -iProcNameSpace api
iProc store {data reg literal number} {iWrite p $data; error broken}
iProc wrapper {} {
 iWrite first.value 9
 if {![catch {iApply}]} {error {callback failure missing}}
}
iCall api::wrapper
iReset
iProc store {data reg literal number} {iWrite p $data}
iWrite first.value 6; iApply
)");
  runtime.Finish();
}
void TestCallbackCycles() {
  auto source = kCallbackModel;
  const auto where = source.find(" Instance first Of Leaf;");
  source.insert(where, " Instance second Of Leaf;\n");
  pattern::Design design;
  design.AddIcl("callback-cycle.icl", source);
  const auto netlist = pattern::Elaborate(design, "Top");
  for (bool indirect : {false, true}) {
    pattern::Runtime runtime(design, netlist);
    std::string procedure = R"(
iProcsForModule Top -iProcNameSpace api
iProc store {data reg literal number} {
 set target first.value
)";
    if (indirect)
      procedure += " if {$reg eq {first.value}} {set target second.value}\n";
    procedure += R"(
 iWrite $target $data; iApply
}
iWrite first.value 9; iApply
)";
    Fails([&] { runtime.Execute("cycle.pdl", procedure); }, "callback cycle");
  }
}
void TestCallbackPartialHistory() {
  pattern::Design design;
  design.AddIcl("callback-partial.icl", R"(
Module Top {
 DataInPort p[3:0]; DataOutPort q[3:0] {Source p;}
 DataRegister observed[3:0] {ReadCallBack Top load <D> <R>; ReadDataSource p;}
})");
  const auto netlist = pattern::Elaborate(design, "Top");
  pattern::Runtime runtime(design, netlist);
  runtime.Execute("partial.pdl", R"(
iProcsForModule Top
iProc load {data reg} {iRead q $data}
iState observed 12 -LastReadValue
iRead observed[1:0] 0; iApply
if {[iGetReadData observed[3:2]] ne {0b11}} {
 error {callback lost unrequested manual history}
}
if {![catch {iGetReadData observed[1:0]}]} {
 error {callback invented observed data from expected}
}
)");
  runtime.Finish();
}
void TestCallbackActionSpan() {
  pattern::Design design;
  design.AddIcl("callback-span.icl", kCallbackModel);
  const auto netlist = pattern::Elaborate(design, "Top");
  pattern::Runtime runtime(design, netlist, true);
  runtime.Execute("span.pdl", R"(
iProcsForModule Top -iProcNameSpace api
iProc store {data reg literal number} {iNote setup; iApply; iWrite p $data}
iProc request {data} {iWrite first.value $data}
iCall api::request 9
iApply
)");
  runtime.Finish();
  for (bool whole : {false, true}) {
    const auto actions = runtime.SourceActions(whole);
    Require(
        actions.size() == 1 && actions[0].begin == 0 &&
            actions[0].end == runtime.operations().size() &&
            actions[0].arguments ==
                std::vector<std::string>{"api::request", "9"},
        "callback source anchor spans every endpoint Apply and outer Apply");
  }
}
void CheckInvalidRawProviderMode(bool mixed) {
  const std::string folder = "tests/cases/ours/pattern_generation/raw_tap_ir/";
  const auto read = [&](const std::string& name) {
    std::ifstream stream(folder + name);
    Require(stream.good(), "raw Runtime TAP fixture opens");
    return std::string((std::istreambuf_iterator<char>(stream)), {});
  };
  pattern::Design design;
  design.AddIcl("raw.icl", read("model.icl"));
  design.AddBsdl("raw.bsdl", read("model.bsdl"), "RawIrProbe");
  const auto network = pattern::ElaborateTapNetwork(design, "RawIrTop");
  pattern::ReadbackUpdates updates;
  updates.scans = {{"jtag", "", "1001", "0000"}};
  updates.failures = 0;
  size_t calls = 0;
  pattern::Runtime runtime(
      design, network.netlist, false,
      [&](const auto&, bool) -> const pattern::ReadbackUpdates* {
        ++calls;
        return &updates;
      });
  runtime.Execute("initial.pdl", R"(
iReset; iScan -ir jtag 4; iApply
if {[iGetReadData jtag] ne "0b1001"} {error {initial raw history}}
)");
  runtime.Execute("queued.pdl", mixed ? "iScan jtag 8; iScan -ir jtag 4"
                                      : "iScan -ir core.client 4");
  runtime.Execute("rejected.pdl", R"(
if {![catch {iApply}]} {error {invalid raw mode accepted}}
iReset
if {[iGetReadData jtag] ne "0b1001"} {error {invalid Apply lost prior history}}
if {![catch {iGetReadData core.client}]} {error {invalid IR published width}}
)");
  Require(calls == 1,
          "invalid raw mode never reaches provider or invalidates history");
}
void TestRawProviderRejectsRegularIr() { CheckInvalidRawProviderMode(false); }
void TestRawProviderRejectsMixedModes() { CheckInvalidRawProviderMode(true); }
void TestRawReadbackProvider() {
  pattern::Design design;
  design.AddIcl("raw-updates.icl", R"(
Module Top {
 ScanInPort si; ScanOutPort so; SelectPort sel;
 DataInPort p; DataOutPort q {Source p;}
 ScanInterface client {Port si; Port so; Port sel; DefaultLoadValue 4'b0;}
})");
  const auto netlist = pattern::Elaborate(design, "Top");
  pattern::ReadbackUpdates updates;
  updates.scans = {{"client", "", "1001", "1001"}};
  updates.failures = 2;
  size_t calls = 0;
  pattern::Runtime runtime(
      design, netlist, false,
      [&](const auto&, bool) -> const pattern::ReadbackUpdates* {
        ++calls;
        return &updates;
      });
  runtime.Execute("raw-updates.pdl", R"(
iScan client -chain default 4 -si 6 -so 0; iApply
if {[iGetReadData client -hex] ne "0x9"} {error {raw hex}}
if {[iGetMiscompares client -dec] ne "9"} {error {raw mask}}
iReset
if {[iGetReadData client -bin] ne "0b1001"} {error {reset lost raw history}}
)");
  Require(calls == 1, "raw getters reuse a validated provider delta");
  runtime.Finish();
  for (int kind = 0; kind < 6; ++kind) {
    pattern::ReadbackUpdates invalid;
    invalid.scans = {{"client", "", "1001", "1001"}};
    invalid.bits = {{netlist.node_ids.at("q"), 0, '1', '1'}};
    if (kind == 0) invalid.scans[0].target = "missing";
    if (kind == 1) invalid.scans[0].chain = "wrong";
    if (kind == 2) invalid.scans[0].read = "100";
    if (kind == 3) invalid.scans[0].miscompare = "100?";
    if (kind == 4) {
      invalid.scans.push_back({"client", "default", "0000", "0000"});
    }
    if (kind == 5) invalid.bits.push_back({netlist.nodes.size(), 0, '1', '1'});
    bool first = true;
    pattern::Runtime rejected(
        design, netlist, false,
        [&](const auto&, bool) -> const pattern::ReadbackUpdates* {
          if (!first) return nullptr;
          first = false;
          return &invalid;
        });
    rejected.Execute("atomic-raw.pdl", R"(
iRead q 0; iScan client 4 -so 0; iApply
if {![catch {iGetReadData client}]} {error {invalid raw delta accepted}}
if {![catch {iGetReadData client}]} {error {partial raw delta published}}
if {![catch {iGetReadData q}]} {error {partial bit delta published}}
if {![catch {iGetStatus}]} {error {partial status published}}
)");
  }
}
void TestReadbackAdapter() {
  pattern::Design design;
  design.AddIcl("readback.icl", R"(
Module Leaf {DataInPort p[3:0]; DataOutPort q[3:0] {Source p;}}
Module Top {
 DataInPort p[3:0]; Instance leaf Of Leaf {InputPort p = p;}
 DataOutPort q[3:0] {Source leaf.q;}
})");
  const auto netlist = pattern::Elaborate(design, "Top");
  pattern::NetworkState actual(netlist);
  actual.Assume(netlist.node_ids.at("p"), "1111");
  const auto q = netlist.node_ids.at("leaf.q");
  pattern::ReadbackUpdates updates;
  uint64_t failures = 4;
  bool first = true, cleared = false;
  pattern::Runtime runtime(
      design, netlist, true,
      [&](const std::vector<pattern::Operation>& operations,
          bool clear) -> const pattern::ReadbackUpdates* {
        Require(operations.size() == 3 && operations.back().command == "iApply",
                "readback consumes the executed normalized stream");
        updates.bits.clear();
        if (first) {
          const auto value = actual.Value(q);
          for (size_t i = 0; i < value.size(); ++i) {
            updates.bits.push_back(
                {q, static_cast<int64_t>(3 - i), value[i], '1'});
          }
          first = false;
        }
        updates.failures = failures;
        if (clear) {
          failures = 0;
          cleared = true;
        }
        return &updates;
      });
  runtime.Execute("readback.pdl", R"(
set calls 0
proc value {} {global calls; incr calls; return 15}
iState p [value]
iPrefix leaf
iRead q 0
iApply
if {[iGetReadData q] ne "0b1111"} {error {expected data used as observation}}
if {[iGetReadData q -hex] ne "0xf"} {error {bad hex observation}}
if {[iGetReadData q -dec] ne "15"} {error {bad decimal observation}}
if {[iGetMiscompares q] ne "0b1111"} {error {bad miscompare plane}}
if {[iGetStatus] != 4} {error {status must count failing bits}}
if {[iGetStatus -clear] != 4} {error {clear must return old global count}}
if {[iGetStatus] != 0} {error {count was not cleared}}
if {$calls != 1} {error {readback reevaluated Tcl}}
)");
  runtime.Finish();
  Require(cleared && runtime.operations().size() == 3,
          "prefixed clear reaches adapter without changing source operations");
}
void TestMalformedReadbackUpdates() {
  pattern::Design design;
  design.AddIcl("updates.icl",
                "Module Top {DataInPort p[3:0];"
                "DataOutPort q[3:0] {Source p;}}");
  const auto netlist = pattern::Elaborate(design, "Top");
  pattern::ReadbackUpdates updates;
  updates.bits = {{netlist.node_ids.at("q"), 0, '1', '1'},
                  {netlist.nodes.size(), 0, '1', '1'}};
  updates.failures = 1;
  size_t calls = 0;
  pattern::Runtime runtime(design, netlist, false,
                           [&](const std::vector<pattern::Operation>&,
                               bool) -> const pattern::ReadbackUpdates* {
                             return ++calls == 1 ? &updates : nullptr;
                           });
  runtime.Execute("updates.pdl", R"(
iRead q 0
iApply
if {![catch {iGetReadData q}]} {error {invalid observation accepted}}
if {![catch {iGetReadData {q[0]}}]} {error {partial observation escaped}}
)");
  runtime.Finish();
  Require(calls == 2,
          "invalid updates must leave selected history unavailable");
}
void TestWriteOptionNormalization() {
  pattern::Design design;
  design.AddIcl("write_options.icl",
                "Module Top {DataInPort value[7:0]; "
                "DataOutPort q[7:0] {Source value;}}");
  const auto netlist = pattern::Elaborate(design, "Top");
  for (const auto& mode : {"auto", "off", "on"}) {
    pattern::Runtime runtime(design, netlist, true);
    runtime.Execute("write.pdl",
                    std::string("iWrite value 0xa "
                                "-ignore_when_value_already_present ") +
                        mode + "; iApply;");
    runtime.Finish();
    Require(runtime.operations().size() == 2 &&
                runtime.operations()[0].arguments ==
                    std::vector<std::string>{"value", "0xa"},
            "validated optioned write must normalize to its target/value");
    const auto actions = runtime.SourceActions();
    Require(
        actions.size() == 1 &&
            actions[0].arguments ==
                std::vector<std::string>{
                    "value", "0xa", "-ignore_when_value_already_present", mode},
        "original evaluated source arguments must retain the option");
  }
  pattern::Runtime invalid(design, netlist);
  Fails(
      [&] {
        invalid.Execute(
            "invalid.pdl",
            "iWrite value 256 "
            "-ignore_when_value_already_present off; iRead q 0; iApply;");
      },
      "truncates significant bits");
}

void TestCallbackAccessBoundary() {
  pattern::Design design;
  design.AddIcl("callbacks.icl", R"(
Module Top {
 DataRegister observed[3:0] {ReadCallBack Top observe <D>;}
 DataRegister written[3:0] {WriteCallBack Top store <D>;}
 Alias reader[3:0] = observed;
 Alias writer[3:0] = written;
 DataOutPort q[3:0] {Source observed;}
}
)");
  const auto netlist = pattern::Elaborate(design, "Top");
  for (const auto& command : {"iRead observed 0", "iRead reader[2:1] 0",
                              "iWrite written 0", "iWrite writer[2:1] 0"}) {
    pattern::Runtime runtime(design, netlist);
    runtime.Execute("queue.pdl", std::string(command));
    Fails([&] { runtime.Execute("apply.pdl", "iApply"); }, "undefined iProc");
  }
  pattern::Runtime canceled(design, netlist);
  canceled.Execute("canceled.pdl",
                   "iWrite written 0; iRead observed 0; iReset; "
                   "iState observed 3 -LastReadValue; "
                   "if {[iGetReadData observed] ne {0b0011}} {error history}; "
                   "iState observed 0; iRead q 0; iApply;");
  canceled.Finish();
}

pattern::Design MakeDesign() {
  pattern::Design design;
  design.AddIcl("top.icl", R"(
Module Top { DataInPort reg[7:0]; Instance a Of vendor::Child; Instance b Of other::Child; }
NameSpace vendor;
Module Child { DataInPort reg[7:0]; }
NameSpace other;
Module Child { DataInPort reg[7:0]; }
)");
  return design;
}
void TestConstructorAndParseFailures() {
  const auto design = MakeDesign();
  Fails([&] { pattern::Runtime missing(design, "missing"); },
        "unknown runtime top module missing");
  pattern::Runtime runtime(design, "Top");
  runtime.Execute("valid.pdl", "iNote previous\n");
  Fails([&] { runtime.Execute("invalid.pdl", "iNote leaked\nif {1} {\n"); },
        "invalid.pdl:3: missing close brace");
  const auto& ops = runtime.operations();
  Require(
      ops.size() == 1 && ops[0].command == "iNote" &&
          ops[0].arguments == std::vector<std::string>{"previous"},
      "malformed later syntax is rejected before publishing any operations");
  Fails([&] { runtime.Execute("late.pdl", "iNote resumed\n"); },
        "cannot resume after an error");
  Fails([&] { runtime.Finish(); }, "cannot finish after an error");
}

void TestNestedPendingEndOfStream() {
  const auto design = MakeDesign();
  pattern::Runtime runtime(design, "Top");
  runtime.Execute("queued-call.pdl",
                  "iProcsForModule Top\n"
                  "iProc queue {} {iWrite reg 1}\niCall queue\n");
  Fails([&] { runtime.Finish(); }, "while iCall queue");
  Fails([&] { runtime.Execute("late.pdl", "iApply\n"); },
        "cannot resume after an error");
}

void TestFallbackInstanceResolution() {
  pattern::Design design;
  design.AddIcl("fallback.icl", R"(
Module Plain { DataInPort d[3:0]; }
Module Typed { Parameter VALUE = 4'b10x1; DataInPort d[3:0]; }
Module Outer { Instance plain Of Plain; Instance typed Of Typed; }
Module Top { Instance a Of Outer; }
)");
  const auto netlist = pattern::Elaborate(design, "Top");
  const std::string definitions =
      "iProcsForModule Plain\niProc emit {} {iNote plain}\n"
      "iProcsForModule Typed\niProc emit {} {iNote $VALUE}\n";
  pattern::Runtime plain(design, "Top");
  plain.Execute("plain.pdl", definitions + "iCall a.plain.emit\n");
  plain.Finish();
  Require(
      plain.operations().size() == 1 &&
          plain.operations()[0].arguments == std::vector<std::string>{"plain"},
      "non-elaborated runtime resolves a nested instance procedure");

  pattern::Runtime unavailable(design, "Top");
  Fails(
      [&] {
        unavailable.Execute("typed.pdl", definitions + "iCall a.typed.emit\n");
      },
      "ICL parameters require an elaborated instance a.typed");
  Require(unavailable.operations().empty(),
          "missing specialization rejects before the procedure body executes");

  pattern::Runtime specialized(design, netlist);
  specialized.Execute("specialized.pdl", definitions + "iCall a.typed.emit\n");
  specialized.Finish();
  Require(specialized.operations().size() == 1 &&
              specialized.operations()[0].arguments ==
                  std::vector<std::string>{"0b10x1"},
          "elaborated unknown parameter bits retain their binary spelling");

  pattern::Runtime unknown_module(design, "Top");
  Fails(
      [&] {
        unknown_module.Execute("module.pdl", "iProcsForModule Missing\n");
      },
      "unknown module Missing");
  pattern::Runtime unknown_plain(design, "Top");
  Fails(
      [&] {
        unknown_plain.Execute("instance.pdl",
                              definitions + "iCall a.absent.emit\n");
      },
      "unknown or ambiguous instance a.absent");
  pattern::Runtime unknown_specialized(design, netlist);
  Fails(
      [&] {
        unknown_specialized.Execute("instance.pdl",
                                    definitions + "iCall a.absent.emit\n");
      },
      "unknown instance a.absent");
}

void TestProcedureNamespaceSelection() {
  const auto design = MakeDesign();
  pattern::Runtime runtime(design, "Top");
  runtime.Execute("namespace.pdl", R"(iProcsForModule Top
iProc emit {} {iNote default}
iProcsForModule Top -iProcNameSpace alternate
iProc emit {} {iNote alternate}
iUseProcNameSpace alternate
iCall emit
iUseProcNameSpace -
iCall emit
)");
  runtime.Finish();
  const auto& ops = runtime.operations();
  Require(
      ops.size() == 2 && ops[0].command == "iNote" &&
          ops[0].arguments == std::vector<std::string>{"alternate"} &&
          ops[1].command == "iNote" &&
          ops[1].arguments == std::vector<std::string>{"default"} &&
          runtime.warnings().empty(),
      "selecting and clearing the call namespace chooses distinct procedures");
}

void TestExplicitObservationGetters() {
  pattern::Design design;
  design.AddIcl("history.icl", R"(
Module History {
  DataRegister value[7:0]; DataRegister wide[79:0];
  Alias inverted[7:0] = ~value;
  Alias repeated[1:0] = value[0], value[0];
  DataOutPort q[7:0] { Source value; }
  Instance child Of HistoryChild;
}
Module HistoryChild { DataRegister value[7:0]; }
)");
  const auto netlist = pattern::Elaborate(design, "History");
  pattern::Runtime runtime(design, netlist);
  runtime.Execute("history.pdl", R"(iPDLLevel 1
iState value 0xa5 -LastReadValue
iState value 0x0c -LastMiscompareValue
iNote [iGetReadData value]
iNote [iGetReadData inverted -hex]
iNote [iGetReadData value -dec]
iNote [iGetMiscompares inverted -bin]
iState inverted 0x33 -LastReadValue
iState inverted 7 -LastMiscompareValue
iNote [iGetReadData value -hex]
iNote [iGetMiscompares value -dec]
iState value 0bxxxxxxxx -LastReadValue
iNote [iGetReadData value -hex]
iState value[7:4] 10 -LastReadValue
iNote [iGetReadData value -hex]
iState value[1:0] 1 -LastReadValue
iNote [iGetReadData value -hex]
iNote [iGetReadData value -dec]
iState value 0bxxx1x001 -LastReadValue
iNote [iGetReadData value[4:0] -hex]
iState value[4:0] 0bxxxxx -LastReadValue
iNote [iGetReadData value[4:0] -hex]
iNote [iGetReadData value[2:0] -hex]
iNote [iGetReadData value[0] -hex]
iState value[2:0] 0b1x1 -LastReadValue
iNote [iGetReadData value[2:0] -hex]
iNote [iGetReadData value[0] -hex]
iState repeated 3 -LastReadValue
iNote [iGetReadData repeated]
iState wide 0xffffffffffffffffffff -LastReadValue
iNote [iGetReadData wide -dec]
iState wide 0 -LastReadValue
iNote [iGetReadData wide -dec]
iProcsForModule HistoryChild
iProc seed {} { iState value 19 -LastReadValue; iNote [iGetReadData value -dec] }
iProcsForModule History
iCall child.seed
)");
  runtime.Execute("history-second.pdl", R"(iPrefix child
iNote [iGetReadData value -dec]
iPrefix -
iNote [iGetReadData child.value -hex]
proc expr {args} {return 77}
iNote [iGetReadData child.value -dec]
)");
  runtime.Finish();
  std::vector<std::string> notes;
  for (const auto& op : runtime.operations()) {
    if (op.command == "iNote") notes.push_back(op.arguments.at(0));
    Require(op.command != "iGetReadData" && op.command != "iGetMiscompares",
            "history getters expand to literal Tcl values");
  }
  Require(notes ==
              std::vector<std::string>{
                  "0b10100101", "0x5a", "165",
                  "0b00001100", "0xcc", "7",
                  "0xXX",       "0xaX", "0xax",
                  "X",          "0x1x", "0xXX",
                  "0xX",        "0xX",  "0xx",
                  "0x1",        "0b11", "1208925819614629174706175",
                  "0",          "19",   "19",
                  "0x13",       "19"},
          "explicit history formats, alias masks and cross-file identities");
}

void TestNeverReadAndInitialStatus() {
  pattern::Design design;
  design.AddIcl("initial.icl", R"(Module Initial {
    DataRegister value[7:0]; Alias inverted[7:0] = ~value;
    DataOutPort q[7:0] {Source value;}
  })");
  const auto netlist = pattern::Elaborate(design, "Initial");
  pattern::Runtime runtime(design, netlist);
  runtime.Execute("initial-first.pdl", R"(iPDLLevel 1
iNote [iGetStatus]
iNote [iGetStatus -clear]
iNote [iGetReadData value]
iNote [iGetReadData q]
iNote [iGetMiscompares value -hex]
iState value[3:0] 10 -LastReadValue
iNote [iGetReadData value]
iNote [iGetReadData inverted]
)");
  runtime.Execute("initial-second.pdl", R"(
iNote [iGetReadData value]
iRead value
iApply
iNote [iGetStatus]
iRead value 0bxxxxxxxx
iApply
iNote [iGetStatus -clear]
)");
  runtime.Finish();
  std::vector<std::string> notes;
  for (const auto& op : runtime.operations()) {
    if (op.command == "iNote") notes.push_back(op.arguments.at(0));
  }
  Require(notes == std::vector<std::string>(
                       {"0", "0", "0bxxxxxxxx", "0bxxxxxxxx", "0xXX",
                        "0bxxxx1010", "0bxxxx0101", "0bxxxx1010", "0", "0"}),
          "never-read bits remain unknown while initial status is known zero");
}
void TestFailedApplyHistory() {
  pattern::Design design;
  design.AddIcl(
      "failed-history.icl",
      "Module History { ScanInPort si; SelectPort sel; CaptureEnPort ce; "
      "ShiftEnPort se; UpdateEnPort ue; TCKPort tck; ResetPort reset; "
      "ScanRegister value[7:0] {ScanInSource si; CaptureSource value;} "
      "ScanOutPort so {Source value[0];} "
      "ScanInterface client {Port si; Port so; Port sel; Port ce; Port se; "
      "Port ue; Port tck; Port reset;} "
      "Alias combo[1:0] = value[1], si; }");
  const auto netlist = pattern::Elaborate(design, "History");
  for (bool unknown_status : {false, true}) {
    for (const std::string first :
         {"iRead value 0bxxxxxxxx; ", "iRead value 0; "}) {
      for (const std::string invalid :
           {"iRead value 0x100; ", "iRead missing 0; ", "iRead si 0; ",
            "iRead combo 0; ", "iWrite missing 0; ", "iWrite value 0x100; ",
            "iScan client 8 -si 0x100; "}) {
        pattern::Runtime runtime(design, netlist);
        runtime.Execute(
            "failed-history.pdl",
            std::string("iPDLLevel 1; iReset; ") +
                (unknown_status ? "iRead value 0; iApply; " : "") +
                "iState value 9 -LastReadValue; "
                "iState value 6 -LastMiscompareValue; " +
                first + invalid +
                "if {![catch {iApply}]} {error {invalid apply was accepted}}; "
                "iReset; iNote [iGetReadData value -dec]; "
                "iNote [iGetMiscompares value -dec]; " +
                (unknown_status ? "if {![catch {iGetStatus}]} {error {unknown "
                                  "status lost}}"
                                : "iNote [iGetStatus]"));
        runtime.Finish();
        std::vector<std::string> notes;
        for (const auto& operation : runtime.operations()) {
          if (operation.command == "iNote") {
            notes.push_back(operation.arguments.at(0));
          }
        }
        Require(
            notes == (unknown_status ? std::vector<std::string>{"9", "6"}
                                     : std::vector<std::string>{"9", "6", "0"}),
            "failed apply preserves supplied histories and prior status");
      }
    }
  }
}
void TestObservationHistoryInvalidation() {
  pattern::Design design;
  design.AddIcl("history.icl", R"(
Module History {
  DataRegister value[7:0];
  Alias reversed[7:0] = ~value[0:7];
  DataOutPort q[7:0] { Source value; }
}
)");
  const auto netlist = pattern::Elaborate(design, "History");
  for (const std::string body :
       {"iRead value; iApply; iGetReadData value",
        "iRead value 0; iApply; iGetReadData value",
        "iRead value 0; iApply; iGetMiscompares value",
        "iRead value 0; iApply; iGetStatus",
        "iRead value 0; iApply; iReset; iGetStatus -clear",
        "iState value 9 -LastReadValue; iRead value; iApply; iGetReadData "
        "value",
        "iState value 9 -LastReadValue; iRead value 0bxxxxxxxx; iApply; "
        "iGetReadData value",
        "iRead value 0; iState value 9 -LastReadValue; iApply; iGetReadData "
        "value",
        "iState value 9 -LastReadValue; iRead reversed[0]; iApply; "
        "iGetReadData value[7]",
        "iState value 9 -LastMiscompareValue; iRead value 9; iApply; "
        "iGetMiscompares value"}) {
    pattern::Runtime runtime(design, netlist);
    Fails([&] { runtime.Execute("unavailable.pdl", "iPDLLevel 1\n" + body); },
          "hardware readback");
  }
  pattern::Runtime invalid(design, netlist);
  Fails([&] { invalid.Execute("invalid.pdl", "iState q 3 -LastReadValue\n"); },
        "state assumption");
  for (const std::string target : {"missing", "value[8]"}) {
    pattern::Runtime invalid_getter(design, netlist);
    Fails(
        [&] { invalid_getter.Execute("target.pdl", "iGetReadData " + target); },
        "PDL target");
  }
  pattern::Runtime runtime(design, netlist);
  runtime.Execute("order.pdl", R"(iPDLLevel 1
iState value 0xa5 -LastReadValue
iRead value[3:0] 5
iNote [iGetReadData value -hex]
iApply
iNote [iGetReadData value[7:4] -hex]
iState value[3:0] 0bx001 -LastReadValue
iNote [iGetReadData value -hex]
iRead value
iState value 0x36 -LastReadValue
iReset
iApply
iNote [iGetReadData value -hex]
iWrite value 0
iApply
iState value 1
iNote [iGetReadData value -hex]
iRead q 0
iApply
iNote [iGetReadData value -hex]
)");
  runtime.Finish();
  std::vector<std::string> notes;
  for (const auto& op : runtime.operations()) {
    if (op.command == "iNote") notes.push_back(op.arguments.at(0));
  }
  Require(notes == std::vector<std::string>{"0xa5", "0xa", "0xax", "0x36",
                                            "0x36", "0x36"},
          "only applied reads invalidate supplied observation bits");
}

void TestSequentialDefinitions() {
  const auto design = MakeDesign();
  pattern::Runtime runtime(design, "Top");
  runtime.Execute("first.pdl", R"(iProcsForModule Top
iProc configure {} {iWrite reg 1; iApply}
iCall configure
)");
  runtime.Execute("second.pdl", R"(iProcsForModule Top
iProc configure {} {iWrite reg 2; iApply}
iCall configure
)");
  const auto& ops = runtime.operations();
  Require(ops.size() == 4 && ops[0].arguments.at(1) == "1" &&
              ops[2].arguments.at(1) == "2",
          "replacement affects subsequent calls only");
  Require(runtime.warnings().size() == 1 &&
              runtime.warnings()[0].find("first.pdl") != std::string::npos &&
              runtime.warnings()[0].find("second.pdl") != std::string::npos,
          "warning must identify both definitions");
}
void TestBindingAndPrefixes() {
  const auto design = MakeDesign();
  pattern::Runtime runtime(design, "Top");
  runtime.Execute("bindings.pdl", R"(iProcsForModule vendor::Child
iProc configure {v} {iWrite reg $v; iApply}
iProcsForModule other::Child
iProc configure {v} {iWrite reg $v; iApply}
iProcsForModule vendor::Child -iProcNameSpace special
iProc configure {v} {iWrite reg $v; iApply}
iCall a.configure 1
iCall b.configure 2
iCall a.special::configure 3
iPrefix a
iCall configure 4
iPrefix -
iWrite reg 5
iApply
)");
  const auto& ops = runtime.operations();
  Require(runtime.warnings().empty(), "distinct bindings must not override");
  Require(ops.size() == 10 && ops[0].arguments[0] == "a.reg" &&
              ops[2].arguments[0] == "b.reg" &&
              ops[4].arguments[0] == "a.reg" &&
              ops[6].arguments[0] == "a.reg" && ops[8].arguments[0] == "reg",
          "instance and procedure namespace resolution");
}
void TestTclAndVectorSyntax() {
  const auto design = MakeDesign();
  pattern::Runtime runtime(design, "Top");
  runtime.Execute("loop.pdl", R"(iProcsForModule Top
iProc run {{count 3}} {
  for {set i 0} {$i < $count} {incr i} {
    iWrite reg[$i] [expr {$i % 2}]
    iApply
  }
}
iCall run
)");
  const auto& ops = runtime.operations();
  Require(ops.size() == 6, "Tcl loop expands into six operations");
  for (size_t i = 0; i < 3; ++i) {
    Require(ops[2 * i].arguments[0] == "reg[" + std::to_string(i) + "]",
            "PDL vector index must not execute as a Tcl command");
    Require(ops[2 * i].arguments[1] == std::to_string(i % 2),
            "ordinary Tcl substitution executes");
    Require(ops[2 * i].origin.find("loop.pdl:4:") != std::string::npos &&
                !ops[2 * i].call_stack.empty(),
            "nested source and call stack");
  }
}
void TestErrorsAndSafety() {
  const auto design = MakeDesign();
  pattern::Runtime unknown(design, "Top");
  Fails([&] { unknown.Execute("unknown.pdl", "iCall absent\n"); }, "absent");
  pattern::Runtime readback(design, "Top");
  Fails([&] { readback.Execute("read.pdl", "set v [iGetReadData reg]\n"); },
        "hardware readback");
  pattern::Runtime unsafe(design, "Top");
  Fails([&] { unsafe.Execute("unsafe.pdl", "exec echo unsafe\n"); }, "exec");
  for (const auto& command : {"after 1", "update", "interp create"}) {
    pattern::Runtime blocking(design, "Top");
    Fails([&] { blocking.Execute("blocking.pdl", command); },
          "invalid command");
  }
  pattern::Runtime event_loop(design, "Top");
  event_loop.Execute("blocking.pdl", "iNote [llength [info commands vwait]]");
  Require(event_loop.operations()[0].arguments[0] == "0",
          "unbounded vwait is unavailable in the generation interpreter");
  pattern::Runtime pending(design, "Top");
  Fails([&] { pending.Execute("pending.pdl", "iWrite reg 1\niRunLoop 2\n"); },
        "unapplied");
  pattern::Runtime stack(design, "Top");
  Fails(
      [&] {
        stack.Execute(
            "stack.pdl",
            "iProcsForModule Top\niProc run {} {iCall absent}\niCall run\n");
      },
      "iCall run");
}

void TestRuntimeValidationAndNamespaceScope() {
  const auto design = MakeDesign();
  pattern::Runtime runtime(design, "Top");
  runtime.Execute("scope.pdl", R"(iProcsForModule Top
iProc write {} {iWrite reg 1; iApply}
iProcsForModule Top -iProcNameSpace special
iProc write {} {iWrite reg 2; iApply}
iProc run {} {iCall write}
iCall special::run
)");
  Require(runtime.operations()[0].arguments[1] == "1",
          "iUseProcNameSpace is local; a selected procedure namespace is not "
          "inherited");
  pattern::Runtime arity(design, "Top");
  Fails([&] { arity.Execute("arity.pdl", "set cmd iWrite; $cmd reg\n"); },
        "arguments");
  pattern::Runtime nul(design, "Top");
  Fails([&] { nul.Execute("nul.pdl", "iWrite reg [format %c 0]\n"); }, "NUL");
  pattern::Runtime empty(design, "Top");
  Fails([&] { empty.Execute("empty.pdl", "set t {}; iWrite $t 1\n"); },
        "empty target");
}

void TestReturnValueAndInputStream() {
  const auto design = MakeDesign();
  pattern::Runtime runtime(design, "Top");
  runtime.Execute("return.pdl", R"(iProcsForModule Top
iProc value {} {return 7}
iWrite reg [iCall value]
iApply
)");
  Require(runtime.operations()[0].arguments[1] == "7",
          "preserve iProc return value");
  runtime.Execute(
      "continued.pdl",
      "iProc value {} {return 8}\niWrite reg [iCall value]\niApply\n");
  Require(runtime.operations()[2].arguments[1] == "8",
          "module binding spans input files");
}

void TestGeneratedBodies() {
  const auto design = MakeDesign();
  pattern::Runtime runtime(design, "Top");
  runtime.Execute("generated.pdl", R"(iProcsForModule Top
set body {iWrite reg[0] 1; iApply}
iProc p {} $body
iCall p
iProc q {} "iWrite reg\[1\] 0; iApply"
iCall q
)");
  Require(runtime.operations().size() == 4 &&
              runtime.operations()[0].arguments[0] == "reg[0]" &&
              runtime.operations()[2].arguments[0] == "reg[1]",
          "evaluated procedure bodies retain PDL vector semantics");
}

void TestDynamicBinding() {
  const auto design = MakeDesign();
  pattern::Runtime runtime(design, "Top");
  runtime.Execute("binding.pdl", R"(if {1} {iProcsForModule Top}
iProc p {} {iWrite reg 1; iApply}
iCall p
)");
  Require(runtime.operations().size() == 2,
          "execute bindings before procedure definitions");
}

void TestDynamicOptions() {
  const auto design = MakeDesign();
  pattern::Runtime runtime(design, "Top");
  Fails(
      [&] {
        runtime.Execute("options.pdl",
                        "set opt -bogus; iWrite reg 1 $opt on; iApply\n");
      },
      "option");
}

void TestDynamicLevel() {
  const auto design = MakeDesign();
  pattern::Runtime levels(design, "Top");
  Require(levels.Level() == 1, "new runtimes start at PDL level1");
  levels.Execute("level0.pdl", "iPDLLevel 0\n");
  Require(levels.Level() == 0, "ordered input files retain actual PDL level");
  levels.Execute("level1.pdl", "iPDLLevel 1\n");
  levels.Finish();
  Require(levels.Level() == 1, "finished runtime retains final PDL level");
  pattern::Runtime runtime(design, "Top");
  Fails(
      [&] {
        runtime.Execute(
            "level.pdl",
            "set level 0; iPDLLevel $level; set x 1; iWrite reg $x; iApply\n");
      },
      "level-0");
}

void TestMergeScope() {
  const auto design = MakeDesign();
  pattern::Runtime runtime(design, "Top");
  Fails(
      [&] {
        runtime.Execute("merge.pdl",
                        "iProcsForModule Top\niProc p {} {iMerge "
                        "-begin}\niCall p\niWrite reg 1; iApply\n");
      },
      "unclosed iMerge");
}

void TestAllTargetArguments() {
  const auto design = MakeDesign();
  pattern::Runtime runtime(design, "Top");
  runtime.Execute("targets.pdl", R"(iPrefix a
iScan -ir iface 8 -si 0xAA
iApply
iClockOverride clk -source src
iOverrideScanInterface iface iface2 -capture on
iRunLoop 2 -sck clk
iScan iface 8 -si 0x55
iApply
)");
  const auto& ops = runtime.operations();
  Require(ops[0].arguments[1] == "a.iface",
          "qualify iScan interface after -ir");
  Require(ops[2].arguments[0] == "a.clk" && ops[2].arguments[2] == "a.src",
          "qualify clock and source");
  Require(ops[3].arguments[0] == "a.iface" && ops[3].arguments[1] == "a.iface2",
          "qualify every overridden interface");
  Require(ops[4].arguments[2] == "a.clk", "qualify iRunLoop clock");
  Require(ops[5].arguments[0] == "a.iface",
          "qualify ordinary iScan target exactly once");
}

void TestEndOfStream() {
  const auto design = MakeDesign();
  pattern::Runtime runtime(design, "Top");
  runtime.Execute("queued.pdl", "iWrite reg 1\n");
  runtime.Execute("applied.pdl", "iApply\n");
  runtime.Finish();
  runtime.Finish();
  Fails([&] { runtime.Execute("late.pdl", "iApply\n"); }, "finished");

  pattern::Runtime reset(design, "Top");
  reset.Execute("queued.pdl", "iWrite reg 1\niRead reg 0\n");
  reset.Execute("reset.pdl", "iReset\n");
  reset.Finish();
  Require(reset.operations().size() == 3 &&
              reset.operations().back().command == "iReset",
          "reset clears pending status without losing source operation order");

  pattern::Runtime pending(design, "Top");
  pending.Execute("pending.pdl", "iWrite reg 1\n");
  Fails([&] { pending.Finish(); }, "pending.pdl:1:1");
  Fails([&] { pending.Execute("late.pdl", "iApply\n"); }, "error");

  pattern::Runtime merging(design, "Top");
  merging.Execute("unclosed.pdl", "iMerge -begin\n");
  Fails([&] { merging.Finish(); }, "unclosed iMerge");

  pattern::Runtime failed(design, "Top");
  Fails([&] { failed.Execute("bad.pdl", "iBogus\n"); }, "iBogus");
  Fails([&] { failed.Finish(); }, "error");
}
void TestInstanceParameters() {
  pattern::Design design;
  design.AddIcl("params.icl", R"(
Module Child { Parameter W = 4; LocalParameter LAST = $W - 1; DataInPort d[$LAST:0]; }
Module Top { Instance a Of Child { Parameter W = 2; } Instance b Of Child; }
)");
  const auto netlist = pattern::Elaborate(design, "Top");
  pattern::Runtime runtime(design, netlist);
  runtime.Execute("params.pdl", R"(iProcsForModule Child
iProc configure {{value 1}} {iWrite d[$LAST:0] $value; iApply; iNote $W}
iCall a.configure
iCall b.configure 2
)");
  runtime.Finish();
  const auto& ops = runtime.operations();
  Require(
      ops[0].arguments[0] == "a.d[1:0]" && ops[3].arguments[0] == "b.d[3:0]",
      "inject specialized ICL parameters into each iProc call");
  Require(ops[2].arguments[0] == "2" && ops[5].arguments[0] == "4",
          "numeric parameters are Tcl integers rather than bit strings");
  pattern::Runtime collision(design, netlist);
  Fails(
      [&] {
        collision.Execute("collision.pdl",
                          "iProcsForModule Child\niProc p {W} {iNote $W}\n");
      },
      "ICL parameter");
}
void TestParameterNamedArgs() {
  pattern::Design design;
  design.AddIcl("args.icl", "Module Top { Parameter args = \"hello world\"; }");
  const auto netlist = pattern::Elaborate(design, "Top");
  pattern::Runtime runtime(design, netlist);
  runtime.Execute("args.pdl",
                  "iProcsForModule Top\niProc p {} {iNote $args}\niCall p\n");
  Require(runtime.operations()[0].arguments[0] == "hello world",
          "parameter args is an ordinary value, not a variadic Tcl formal");
  Fails([&] { runtime.Execute("excess.pdl", "iCall p 1\n"); }, "wrong # args");
}
void TestResetScope() {
  const auto design = MakeDesign();
  pattern::Runtime child(design, "Top");
  Fails(
      [&] {
        child.Execute("child-reset.pdl",
                      "iProcsForModule vendor::Child\niProc reset {} {iReset}\n"
                      "iCall a.reset\n");
      },
      "cannot retarget iReset");
  pattern::Runtime top(design, "Top");
  top.Execute("top-reset.pdl",
              "iProcsForModule Top\niProc reset {} {iPrefix a; iReset}\n"
              "iCall reset\n");
  Require(
      top.operations().size() == 1 && top.operations()[0].command == "iReset",
      "top-module reset is independent of the current target prefix");
}
void TestVectorSourceActions() {
  auto design = MakeDesign();
  const auto netlist = pattern::Elaborate(design, "Top");
  pattern::Runtime runtime(design, netlist, true);
  const std::string source =
      "iPDLLevel 1\nset index 0\niWrite reg[$index] [iReset; expr 1]\niApply\n";
  runtime.Execute("vectors.pdl", source);
  runtime.Finish();
  const auto actions = runtime.SourceActions();
  const auto begin = source.find("[$index]");
  Require(
      actions.size() == 1 && actions[0].begin == 1 &&
          actions[0].arguments == std::vector<std::string>{"reg[0]", "1"} &&
          actions[0].vector_brackets == std::vector<size_t>{begin, begin + 7},
      "source metadata distinguishes vector brackets from argument scripts");
  pattern::Runtime empty(design, netlist, true);
  empty.Execute("empty.pdl", R"(iPDLLevel 1
 iProcsForModule Top
 iProc noop {value} {return $value}
 foreach command {noop missing} {catch {iCall $command retained}}
 iNote done
)");
  empty.Finish();
  const auto observed = empty.SourceActions();
  Require(observed.size() == 2 && observed[0].begin == 0 &&
              observed[0].end == 0 && observed[1].begin == 0 &&
              observed[1].end == 0 &&
              observed[0].arguments ==
                  std::vector<std::string>{"noop", "retained"} &&
              observed[1].arguments ==
                  std::vector<std::string>{"missing", "retained"},
          "source tracing retains no-op and caught-error caller arguments");
}
void TestCompoundSourceActions() {
  auto design = MakeDesign();
  const auto netlist = pattern::Elaborate(design, "Top");
  pattern::Runtime runtime(design, netlist, true);
  const std::string source =
      "iPDLLevel 1\niProcsForModule Top\n"
      "iProc put {v} {iWrite reg $v; iApply}\niPrefix a\n"
      "foreach target {a b} {iPrefix $target; iWrite reg 3}\niApply\n";
  runtime.Execute("loop.pdl", source);
  const auto literal = runtime.SourceActions();
  const auto whole = runtime.SourceActions(true);
  Require(literal.size() == 2 && whole.size() == 1 && whole[0].source == 0 &&
              whole[0].offset == source.find("foreach") &&
              whole[0].begin == 0 && whole[0].end == 2 &&
              whole[0].prefix == "a",
          "whole command captures entry prefix and all repeated writes");
  runtime.Execute("merge-start.pdl",
                  "iPrefix -\nif {1} {iMerge -begin; iCall put 4}\n");
  runtime.Execute("merge-end.pdl", "iCall put 5; iMerge -end\n");
  runtime.Execute("return.pdl",
                  "if {1} {iWrite reg 6; iApply; return}; iWrite absent 0\n");
  runtime.Finish();
  const auto actions = runtime.SourceActions(true);
  Require(actions.size() == 3 && actions[1].source == 1 &&
              actions[1].begin == 3 && actions[1].end == 9 &&
              actions[2].source == 3 && actions[2].begin == 9 &&
              actions[2].end == 11 && runtime.operations().size() == 11,
          "compound spans retain cross-file merges and top-level return");
  pattern::Runtime overlap(design, netlist, true);
  const std::string crossing =
      "iMerge -begin\nforeach v {1 2} {if {$v == 1} {iMerge -end}; "
      "iWrite reg $v}\niApply\n";
  overlap.Execute("crossing.pdl", crossing);
  overlap.Finish();
  const auto combined = overlap.SourceActions(true);
  Require(combined.size() == 1 && combined[0].offset == 0 &&
              combined[0].begin == 0 && combined[0].end == 4,
          "overlapping merge and compound spans use the earliest caller site");
}
}  // namespace
int main() {
  int failures = 0;
  for (const auto test : {TestCallbackQueueAndBinding,
                          TestCallbackFailureRecovery,
                          TestCaughtCallbackReturnsToCaller,
                          TestCallbackCycles,
                          TestCallbackPartialHistory,
                          TestCallbackActionSpan,
                          TestRawProviderRejectsRegularIr,
                          TestRawProviderRejectsMixedModes,
                          TestRawReadbackProvider,
                          TestReadbackAdapter,
                          TestMalformedReadbackUpdates,
                          TestWriteOptionNormalization,
                          TestCallbackAccessBoundary,
                          TestConstructorAndParseFailures,
                          TestNestedPendingEndOfStream,
                          TestFallbackInstanceResolution,
                          TestProcedureNamespaceSelection,
                          TestExplicitObservationGetters,
                          TestNeverReadAndInitialStatus,
                          TestFailedApplyHistory,
                          TestObservationHistoryInvalidation,
                          TestSequentialDefinitions,
                          TestBindingAndPrefixes,
                          TestTclAndVectorSyntax,
                          TestErrorsAndSafety,
                          TestRuntimeValidationAndNamespaceScope,
                          TestReturnValueAndInputStream,
                          TestGeneratedBodies,
                          TestDynamicBinding,
                          TestDynamicOptions,
                          TestDynamicLevel,
                          TestMergeScope,
                          TestAllTargetArguments,
                          TestEndOfStream,
                          TestInstanceParameters,
                          TestParameterNamedArgs,
                          TestResetScope,
                          TestVectorSourceActions,
                          TestCompoundSourceActions}) {
    try {
      test();
    } catch (const std::exception& error) {
      std::cerr << "FAIL: " << error.what() << '\n';
      ++failures;
    }
  }
  std::cout << "PDL runtime failures: " << failures << '\n';
  return failures ? 1 : 0;
}
