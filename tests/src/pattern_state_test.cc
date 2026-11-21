// Copyright 2026 The ICL Toolsets Authors.
// SPDX-License-Identifier: MIT
#include "pattern/pattern_state.h"

#include <chrono>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <stdexcept>

#include "pattern/pattern_design.h"
#include "pattern/pattern_flatten.h"
#include "pattern/pattern_tap_execution.h"

namespace {
void Require(bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

void TestCompleteShift() {
  pattern::Design design;
  design.AddIcl("complete-shift.icl", R"(Module Top {
    ScanInPort si; ResetPort reset;
    ScanRegister a[0:1] { ScanInSource si; ResetValue 2'b01; }
    ScanRegister b[2:0] { ScanInSource a[1]; ResetValue 3'b010; }
    ScanOutPort so { Source b[0]; }
  })");
  const auto netlist = pattern::Elaborate(design, "Top");
  const auto a = netlist.node_ids.at("a"), b = netlist.node_ids.at("b");
  const auto si = netlist.node_ids.at("si"),
             reset = netlist.node_ids.at("reset");
  pattern::NetworkState serial(netlist), bulk(netlist);
  const std::map<size_t, pattern::ScanControl> shift{{a, {'0', '1', '0', '1'}},
                                                     {b, {'0', '1', '0', '1'}}};
  const std::map<size_t, pattern::ScanControl> update{
      {a, {'0', '0', '1', '1'}}, {b, {'0', '0', '1', '1'}}};
  for (auto* state : {&serial, &bulk}) {
    state->SetGlobalReset('0', '0');
    state->Drive(reset, "1");
    state->Reset();
    state->Drive(reset, "0");
    state->Reset();
  }
  for (const std::string payload : {"10110", "0x1x0", "11111", "00000"}) {
    for (auto it = payload.rbegin(); it != payload.rend(); ++it) {
      serial.Drive(si, std::string(1, *it));
      serial.Clock(shift, true);
    }
    bulk.Drive(si, payload.substr(0, 1));
    bulk.ShiftComplete({{a, payload.substr(0, 2)}, {b, payload.substr(2)}},
                       shift);
    Require(serial.SameState(bulk),
            "bulk full-chain shift matches every native snapshot field");
    Require(serial.TakeControlEvents() == bulk.TakeControlEvents() &&
                serial.TakeResetEvents() == bulk.TakeResetEvents(),
            "bulk shift retains native event histories");
    serial.Clock(update, false);
    bulk.Clock(update, false);
    Require(serial.SameState(bulk),
            "bulk shift updates and repeated shadow bookkeeping match");
  }
  const pattern::NetworkState before(bulk);
  for (const std::map<size_t, std::string>& loads :
       {std::map<size_t, std::string>{{a, "01"}, {b, "10"}},
        std::map<size_t, std::string>{{a, "01"}},
        std::map<size_t, std::string>{{a, "0z"}, {b, "101"}}}) {
    bool rejected = false;
    try {
      bulk.ShiftComplete(loads, shift);
    } catch (const std::runtime_error&) {
      rejected = true;
    }
    Require(rejected && before.SameState(bulk),
            "all bulk loads validate before any shift mutation");
  }
  auto inactive = shift;
  inactive.at(b).select = '0';
  bool invalid_control = false;
  try {
    bulk.ShiftComplete({{a, "01"}, {b, "101"}}, inactive);
  } catch (const std::runtime_error&) {
    invalid_control = true;
  }
  Require(invalid_control && before.SameState(bulk),
          "bulk control validation precedes native or vector mutation");
  for (const std::string reset_value : {"1", "x"}) {
    bulk.Drive(reset, reset_value);
    const pattern::NetworkState snapshot(bulk);
    bool rejected = false;
    try {
      bulk.ShiftComplete({{a, "01"}, {b, "101"}}, shift);
    } catch (const std::runtime_error&) {
      rejected = true;
    }
    Require(rejected && snapshot.SameState(bulk),
            "inactive reset is a precondition of complete-shift batching");
  }
}

void TestImpliedToTckValues() {
  pattern::Design design;
  design.AddIcl("implied_clock_values.icl",
                "Module Child { TCKPort ck[3]; ToTCKPort out[0:1]; } "
                "Module M { TCKPort ck[7]; ToTCKPort out[3:0]; "
                "Instance child Of Child { InputPort ck[3] = ck[7]; } }");
  const auto original = pattern::Elaborate(design, "M");
  pattern::Design flat;
  flat.AddIcl("flat.icl", pattern::SerializeIcl(original));
  const auto replay = pattern::Elaborate(flat, "M");
  auto legacy = original;
  legacy.nodes.at(legacy.node_ids.at("out")).inputs.erase("Source");
  legacy.nodes.at(legacy.node_ids.at("child.out")).inputs.erase("Source");
  pattern::Design old_flat;
  old_flat.AddIcl("legacy_clock.icl", pattern::SerializeIcl(legacy));
  const auto restored_legacy = pattern::Elaborate(old_flat, "M");
  for (const auto* netlist : {&original, &replay, &restored_legacy}) {
    const auto& child_source = netlist->NodeAt("child.out").inputs.at("Source");
    Require(
        child_source.size() == 2 &&
            netlist->nodes.at(child_source[0].node).name == "child.ck",
        "restored implicit clocks reconnect within their original child scope");
    pattern::NetworkState state(*netlist);
    for (char clock : std::string("01x10")) {
      state.Drive(netlist->node_ids.at("ck"), std::string(1, clock));
      Require(
          state.Value(netlist->node_ids.at("out")) == std::string(4, clock) &&
              state.Value(netlist->node_ids.at("child.out")) ==
                  std::string(2, clock),
          "implied ToTCK connections carry concrete parent clock transitions");
    }
  }
}

void TestRegisterEvents() {
  pattern::Design design;
  design.AddIcl("events.icl", R"(
Module Events {
  ScanInPort si;
  DataInPort data[3:0];
  ResetPort reset;
  ScanRegister a[3:0] { ScanInSource si; CaptureSource data; ResetValue 4'b0011; }
  ScanRegister b[0:3] { ScanInSource a; ResetValue 4'b0101; }
  ScanOutPort so { Source b[3]; }
})");
  const auto netlist = pattern::Elaborate(design, "Events");
  pattern::NetworkState state(netlist);
  const auto a = netlist.node_ids.at("a"), b = netlist.node_ids.at("b");
  Require(state.Value(a) == "xxxx",
          "reset metadata does not initialize hardware");
  state.Drive(netlist.node_ids.at("reset"), "0");
  state.Drive(netlist.node_ids.at("data"), "1010");
  state.Clock({{a, {'1', '0', '0', '1'}}, {b, {'1', '0', '0', '1'}}}, true);
  Require(state.Value(a, true) == "1010" && state.Value(b, true) == "xxxx" &&
              state.Value(a) == "xxxx",
          "capture changes only shift state, missing source is X");
  state.Drive(netlist.node_ids.at("si"), "1");
  state.Clock({{a, {'0', '1', '0', '1'}}, {b, {'0', '1', '0', '1'}}}, true);
  Require(state.Value(a, true) == "1101" && state.Value(b, true) == "0xxx",
          "serial shifts sample old rightmost bit simultaneously");
  state.Clock({{a, {'0', '0', '1', '1'}}, {b, {'0', '0', '1', '0'}}}, false);
  Require(state.Value(a) == "1101" && state.Value(b) == "xxxx",
          "falling-edge update is select qualified");
  state.Drive(netlist.node_ids.at("reset"), "1");
  state.Reset();
  Require(state.TakeResetEvents() == std::set<size_t>{a, b} &&
              state.TakeResetEvents().empty(),
          "reset events are retained until consumed");
  Require(state.Value(a) == "0011" && state.Value(b) == "0101" &&
              state.Value(a, true) == "1101",
          "reset affects update state regardless of select, not shift state");
  state.Clock({{a, {'0', '1', '0', '1'}}}, true);
  Require(state.TakeResetEvents() == std::set<size_t>{a, b},
          "held reset is observed even when values already match reset");
  Require(state.Value(a, true) == "1110" && state.Value(a) == "0011",
          "held reset does not block shifting");
  state.Clock({{a, {'0', '0', '1', '1'}}}, false);
  state.Clock({}, true);
  Require(state.Value(a, true) == "xxxx",
          "shift data is not assumed retained after UpdateEn deasserts outside "
          "capture/shift");
  const auto flat = pattern::SerializeIcl(netlist);
  pattern::Design replay;
  replay.AddIcl("flat.icl", flat);
  const auto restored = pattern::Elaborate(replay, "Events");
  pattern::NetworkState copy(restored);
  copy.Drive(restored.node_ids.at("reset"), "1");
  copy.Reset();
  Require(copy.Value(restored.node_ids.at("a")) == "0011",
          "flat reset semantics replay");
}

void TestFeedbackResetSnapshots() {
  pattern::Design design;
  design.AddIcl("feedback.icl", R"(
Module Cell {
 Parameter W=1; ResetPort rst; DataRegister value[$W-1:0] { ResetValue 0; }
 DataOutPort q[$W-1:0] { Source value; }
}
Module Feedback {
 Instance a Of Cell { InputPort rst=1'b0; }
 Instance b Of Cell { InputPort rst=1'b0; }
}
Module Transient {
 DataInPort reset;
 Instance gate Of Cell { InputPort rst=1'b0; }
 LogicSignal asserted { gate.q & b.q; }
 Instance a Of Cell { InputPort rst=1'b0; }
 Instance b Of Cell { InputPort rst=1'b0; }
 Instance child Of Cell { Parameter W=4; InputPort rst=1'b0; }
}
)");
  auto netlist = pattern::Elaborate(design, "Feedback");
  // Explicit lowered circuits retain arbitrary reset feedback and unknown
  // state.
  for (const auto& [target, source] :
       std::map<std::string, std::string>{{"a.rst", "b.q"}, {"b.rst", "a.q"}}) {
    netlist.nodes.at(netlist.node_ids.at(target)).inputs["Source"] = {
        {netlist.node_ids.at(source), 0}};
  }
  pattern::CheckCombinationalConnections(netlist);
  const auto a = netlist.node_ids.at("a.value");
  const auto b = netlist.node_ids.at("b.value");
  for (const auto& initial :
       {std::string("11"), std::string("1x"), std::string("01")}) {
    pattern::NetworkState state(netlist);
    state.Assume(a, initial.substr(0, 1));
    state.Assume(b, initial.substr(1, 1));
    state.Reset();
    Require(state.Value(a) == (initial == "1x" ? "x" : "0") &&
                state.Value(b) == (initial == "01" ? "1" : "0"),
            "feedback reset retains snapshot and unknown-state semantics");
    const auto events = state.TakeControlEvents();
    Require(events.count(a) == (initial != "1x") &&
                events.count(b) == (initial != "01"),
            "reset events include unchanged storage, never unknown assertions");
  }
  auto transient = pattern::Elaborate(design, "Transient");
  for (const auto& [target, source] :
       std::map<std::string, std::string>{{"gate.rst", "reset"},
                                          {"a.rst", "asserted"},
                                          {"b.rst", "a.q"},
                                          {"child.rst", "a.q"}}) {
    transient.nodes.at(transient.node_ids.at(target)).inputs["Source"] = {
        {transient.node_ids.at(source), 0}};
  }
  pattern::CheckCombinationalConnections(transient);
  pattern::NetworkState state(transient);
  for (const std::string name : {"gate.value", "a.value", "b.value"}) {
    state.Assume(transient.node_ids.at(name), "1");
  }
  state.Assume(transient.node_ids.at("child.value"), "1001");
  state.Drive(transient.node_ids.at("reset"), "1");
  state.Reset();
  for (const std::string name : {"gate.value", "a.value", "b.value"}) {
    Require(state.Value(transient.node_ids.at(name)) == "0",
            "feedback closure retains a transient ancestor reset");
  }
  Require(state.Value(transient.node_ids.at("child.value")) == "0000",
          "transient feedback reset reaches downstream storage");
}

void TestControllingEventHistory() {
  pattern::Design design;
  design.AddIcl("control-history.icl", R"(
Module History {
 ScanInPort si; DataInPort data[3:0]; DataInPort enable; ResetPort rst;
 ScanRegister scan[3:0] { ScanInSource si; ResetValue 0; }
 DataRegister value[3:0] { WriteDataSource data; WriteEnSource enable; ResetValue 0; }
})");
  const auto netlist = pattern::Elaborate(design, "History");
  pattern::NetworkState state(netlist);
  const auto value = netlist.node_ids.at("value");
  const auto scan = netlist.node_ids.at("scan");
  const auto reset = netlist.node_ids.at("rst");
  const auto data = netlist.node_ids.at("data");
  state.Drive(reset, "0");
  state.Drive(data, "0110");
  state.Drive(netlist.node_ids.at("enable"), "1");
  state.ApplyInputs({});
  state.Drive(reset, "x");
  state.Reset();
  Require(
      state.Value(value) == "0xx0" && state.TakeControlEvents().at(value) ==
                                          std::optional<std::string>("0110"),
      "unknown reset changes hardware without erasing last known write");
  Require(state.TakeControlEvents().empty(), "history drains once");
  state.Drive(reset, "1");
  state.Reset();
  Require(!state.TakeControlEvents().at(value),
          "known reset records even unchanged reset bits");
  state.Reset();
  state.Drive(reset, "0");
  state.ApplyInputs({});
  auto events = state.TakeControlEvents();
  Require(events.at(value) == std::optional<std::string>("0110") &&
              !events.at(scan),
          "data write supersedes its earlier reset, retaining unrelated reset");
  state.ApplyInputs({});
  state.Drive(reset, "1");
  state.Reset();
  Require(!state.TakeControlEvents().at(value),
          "later reset supersedes a known write");
  state.Reset();
  state.Drive(reset, "0");
  state.Drive(data, "xxxx");
  state.ApplyInputs({});
  Require(state.Value(value) == "xxxx" && !state.TakeControlEvents().at(value),
          "unknown hardware writes do not erase controlling reset history");
  state.Assume(value, "1001");
  state.SetRegister(scan, "1010", "0101");
  Require(state.TakeControlEvents().empty(),
          "assumptions and controller synchronization emit no hardware event");
  state.Clock({{scan, {'0', '0', '1', '1'}}}, false);
  Require(
      state.TakeControlEvents().at(scan) == std::optional<std::string>("1010"),
      "known scan update payload is available to planned-write consumers");
  state.Drive(reset, "1");
  state.Reset();
  const pattern::NetworkState snapshot(state);
  state.TakeControlEvents();
  Require(!state.SameState(snapshot),
          "pending history participates in snapshot equality");
  state.Restore(snapshot);
  Require(state.SameState(snapshot) && !state.TakeControlEvents().at(value),
          "rollback restores pending controlling events");
}
void TestDataRegisterStateSnapshot() {
  pattern::Design design;
  design.AddIcl("data-state.icl", R"(
Module Snapshot {
 ScanInPort si; DataInPort data[3:0]; DataInPort enable; ResetPort rst;
 ScanRegister scan[3:0] { ScanInSource si; ResetValue 0; }
 DataRegister first[3:0] { WriteDataSource data; WriteEnSource enable; ResetValue 0; }
 DataRegister second[3:0] { WriteDataSource first; WriteEnSource enable; ResetValue 0; }
})");
  const auto netlist = pattern::Elaborate(design, "Snapshot");
  pattern::NetworkState state(netlist);
  const auto first = netlist.node_ids.at("first");
  const auto second = netlist.node_ids.at("second");
  const auto scan = netlist.node_ids.at("scan");
  state.Drive(netlist.node_ids.at("rst"), "1");
  state.Drive(netlist.node_ids.at("data"), "0110");
  state.Drive(netlist.node_ids.at("enable"), "1");
  state.SetRegister(scan, "1010", "0101");
  state.Assume(first, "1001");
  Require(state.Value(first) == "1001" && state.Value(second) == "xxxx" &&
              state.Value(scan) == "0101" &&
              state.Value(scan, true) == "1010" &&
              state.TakeResetEvents().empty(),
          "DataRegister assumption triggers no reset, cascade or scan event");
  bool rejected = false;
  try {
    state.Assume(first, "11");
  } catch (const std::exception&) {
    rejected = true;
  }
  Require(rejected && state.Value(first) == "1001",
          "invalid DataRegister snapshot preserves old storage");
  state.Assume(first, "x001");
  Require(state.Value(first) == "x001", "snapshot retains unknown data bits");
  state.Reset();
  Require(state.Value(first) == "0000" && state.Value(second) == "0000" &&
              state.Value(scan, true) == "1010",
          "an explicit later reset updates storage without clearing scan shift "
          "state");
  state.Drive(netlist.node_ids.at("rst"), "0");
  state.ApplyInputs({});
  Require(state.Value(first) == "0110" && state.Value(second) == "0110",
          "a later data event resumes normal cascade behavior");
}
void TestDataRegisterEvents() {
  pattern::Design design;
  design.AddIcl("data-events.icl", R"(
Module DataEvents {
  ScanInPort si;
  ResetPort rst;
  ScanRegister bus[4:0] { ScanInSource si; ResetValue 5'b0; }
  DataRegister first[3:0] {
    WriteDataSource bus[3:0]; WriteEnSource bus[4]; ResetValue 4'b0;
  }
  DataRegister second[3:0] { WriteDataSource first; ResetValue 4'b0; }
  DataRegister constant { WriteDataSource 1'b1; ResetValue 1'b0; }
  ScanOutPort so { Source bus[0]; }
})");
  const auto netlist = pattern::Elaborate(design, "DataEvents");
  pattern::NetworkState state(netlist);
  const auto bus = netlist.node_ids.at("bus");
  const auto first = netlist.node_ids.at("first");
  const auto second = netlist.node_ids.at("second");
  state.Drive(netlist.node_ids.at("rst"), "1");
  state.Reset();
  state.Drive(netlist.node_ids.at("rst"), "0");
  state.Clock({{bus, {'0', '0', 'x', '1'}}}, false);
  Require(state.Value(netlist.node_ids.at("constant")) == "x",
          "uncertain update joins event and no-event storage");
  state.Clock({{bus, {'0', '0', '1', '1'}}}, false);
  Require(state.Value(netlist.node_ids.at("constant")) == "1",
          "known update loads constant data");
  state.Clock({{bus, {'0', '0', 'x', '1'}}}, false);
  Require(state.Value(netlist.node_ids.at("constant")) == "1",
          "uncertain update retains equal event and no-event values");
  state.SetRegister(bus, "11010", "00000");
  state.Clock({{bus, {'0', '0', '1', '1'}}}, false);
  Require(state.Value(first) == "1010" && state.Value(second) == "1010",
          "selectable data-register cascades evaluate after scan update");
  state.SetRegister(bus, "10101", "11010");
  state.Clock({{bus, {'0', '0', '1', '1'}}}, false);
  Require(state.Value(first) == "0101" && state.Value(second) == "0101",
          "active write enable loads the next update request");
  state.SetRegister(bus, "01111", "10101");
  state.Clock({{bus, {'0', '0', '1', '1'}}}, false);
  Require(state.Value(first) == "0101" && state.Value(second) == "0101",
          "disabled selectable data register retains storage");
  state.SetRegister(bus, "x1111", "01111");
  state.Clock({{bus, {'0', '0', '1', '1'}}}, false);
  Require(state.Value(first) == "x1x1" && state.Value(second) == "x1x1",
          "unknown enable joins retained and potentially loaded values");
  state.Drive(netlist.node_ids.at("rst"), "1");
  state.SetRegister(bus, "11111", "00000");
  state.Clock({{bus, {'0', '0', '1', '1'}}}, false);
  Require(state.Value(first) == "0000" && state.Value(second) == "0000",
          "reset dominates data updates");
}

void TestAddressedDataEvents() {
  std::ifstream file(
      "tests/cases/tessent/pattern_generation/address_defaults.icl");
  const std::string source(std::istreambuf_iterator<char>(file), {});
  pattern::Design design;
  design.AddIcl("address-events.icl", source);
  const auto netlist = pattern::Elaborate(design, "AddressDefaultsTop");
  pattern::NetworkState state(netlist);
  const auto address = netlist.node_ids.at("address");
  const auto data = netlist.node_ids.at("data");
  const auto we = netlist.node_ids.at("we");
  const auto target = netlist.node_ids.at("child.r");
  state.ApplyInputs(
      {{address, "00000101"}, {data, "1001011000000000"}, {we, "1"}});
  Require(
      state.Value(target) == "10010110",
      "cumulative address and Tessent data-bus default update the register");
  state.ApplyInputs({{netlist.node_ids.at("re"), "1"}, {we, "0"}});
  Require(state.Value(netlist.node_ids.at("out")) == "10010110",
          "addressed instance and register contributors expose selected data");
  state.ApplyInputs(
      {{address, "00000100"}, {data, "0000000000000000"}, {we, "1"}});
  Require(state.Value(netlist.node_ids.at("out")) == "xxxxxxxx",
          "unmapped address does not drive a one-hot output");
  Require(state.Value(target) == "10010110",
          "a nonmatching address does not update storage");
  state.ApplyInputs({{address, "0000010x"}});
  Require(state.Value(target) == "x00x0xx0",
          "uncertain address joins possible update and retained state");
  pattern::NetworkState before(state);
  bool rejected = false;
  try {
    state.ApplyInputs({{we, "0"}, {data, "1"}});
  } catch (const std::exception&) {
    rejected = true;
  }
  Require(rejected && state.SameState(before),
          "invalid parallel updates are atomic");
}

void TestAmbiguousDataAddress() {
  pattern::Design design;
  design.AddIcl("ambiguous.icl", R"(
Module Ambiguous {
  AddressPort address; DataInPort data; WriteEnPort we; ReadEnPort re;
  OneHotDataGroup group {
    DataRegister a { AddressValue 0; }
    DataRegister b { AddressValue 0; }
  }
  DataOutPort out { Source group; }
})");
  const auto netlist = pattern::Elaborate(design, "Ambiguous");
  pattern::NetworkState state(netlist), before(state);
  bool rejected = false;
  try {
    state.ApplyInputs({{netlist.node_ids.at("address"), "0"},
                       {netlist.node_ids.at("data"), "1"},
                       {netlist.node_ids.at("we"), "1"}});
  } catch (const std::exception& error) {
    rejected =
        std::string(error.what()).find("ambiguous data-register address") !=
        std::string::npos;
  }
  Require(rejected && state.SameState(before),
          "colliding addressed storage rejects before mutation");
}

void TestDataCascadeScaleAndFeedback() {
  const size_t count = 1024;
  std::string source = "Module Cascade { DataInPort data;\n";
  for (size_t i = 0; i < count; ++i) {
    source += "DataRegister r" + std::to_string(i) + " { WriteDataSource " +
              (i + 1 == count ? "data" : "r" + std::to_string(i + 1)) + "; }\n";
  }
  source += "}";
  pattern::Design design;
  design.AddIcl("cascade.icl", source);
  const auto netlist = pattern::Elaborate(design, "Cascade");
  pattern::NetworkState state(netlist);
  const auto start = std::chrono::steady_clock::now();
  state.ApplyInputs({{netlist.node_ids.at("data"), "1"}});
  Require(state.Value(netlist.node_ids.at("r0")) == "1",
          "reverse-declared data cascade propagates in dependency order");
  state.ApplyInputs({{netlist.node_ids.at("data"), "0"}});
  Require(state.Value(netlist.node_ids.at("r0")) == "0",
          "cached cascade order supports subsequent transactions");
  std::cout << "data cascade: registers=" << count << " seconds="
            << std::chrono::duration<double>(std::chrono::steady_clock::now() -
                                             start)
                   .count()
            << "\n";
  const auto position = source.rfind("WriteDataSource data");
  source.replace(position, std::string("WriteDataSource data").size(),
                 "WriteDataSource r0");
  pattern::Design cyclic;
  cyclic.AddIcl("cyclic.icl", source);
  const auto loop = pattern::Elaborate(cyclic, "Cascade");
  pattern::NetworkState failed(loop);
  failed.ApplyInputs({{loop.node_ids.at("data"), "1"}});
  Require(
      failed.Value(loop.node_ids.at("r0")) == "x",
      "unknown feedback retains X instead of choosing a binary fixed point");
  for (size_t i = 0; i < count; ++i) {
    failed.Assume(loop.node_ids.at("r" + std::to_string(i)), i % 2 ? "1" : "0");
  }
  const pattern::NetworkState before(failed);
  bool rejected = false;
  try {
    failed.ApplyInputs({{loop.node_ids.at("data"), "0"}});
  } catch (const std::exception& error) {
    rejected = std::string(error.what()).find("data-register feedback") !=
               std::string::npos;
  }
  Require(rejected && failed.SameState(before),
          "unproved feedback convergence rejects atomically");
  for (size_t i = 0; i < count; ++i) {
    failed.Assume(loop.node_ids.at("r" + std::to_string(i)), "1");
  }
  failed.ApplyInputs({{loop.node_ids.at("data"), "0"}});
  Require(
      failed.Value(loop.node_ids.at("r0")) == "1",
      "a failed feedback event can retry from an explicitly stable snapshot");
}

void TestDataRegisterFeedback() {
  for (const bool reverse : {false, true}) {
    pattern::Design design;
    const std::string a =
        "DataRegister a[3:0] {WriteDataSource b; WriteEnSource enable; "
        "ResetValue 0;}";
    const std::string b =
        "DataRegister b[3:0] {WriteDataSource 1'b0,a[3:1]; WriteEnSource "
        "enable; ResetValue 0;}";
    design.AddIcl("data_feedback.icl",
                  "Module Top {DataInPort enable; ResetPort rst;" +
                      (reverse ? b + a : a + b) +
                      "DataRegister hold[3:0] {WriteDataSource hold; "
                      "WriteEnSource enable; ResetValue 0;}"
                      "DataRegister shift[3:0] {WriteDataSource "
                      "1'b0,shift[3:1]; WriteEnSource enable; ResetValue 0;}"
                      "}");
    const auto original = pattern::Elaborate(design, "Top");
    pattern::Design replay;
    replay.AddIcl("flat.icl", pattern::SerializeIcl(original));
    const auto flat = pattern::Elaborate(replay, "Top");
    for (const auto* netlist : {&original, &flat}) {
      const auto enable = netlist->node_ids.at("enable"),
                 rst = netlist->node_ids.at("rst");
      const auto aid = netlist->node_ids.at("a"),
                 bid = netlist->node_ids.at("b");
      const auto hold = netlist->node_ids.at("hold"),
                 shift = netlist->node_ids.at("shift");
      pattern::NetworkState state(*netlist);
      state.Drive(rst, "0");
      state.Assume(hold, "1010");
      for (const auto& control : {"0", "1", "x"}) {
        state.ApplyInputs({{enable, control}});
        Require(state.Value(hold) == "1010",
                "self hold keeps its actual old value for all enables");
        const auto events = state.TakeControlEvents();
        Require(events.count(hold) == (std::string(control) == "1"),
                "only definite known self writes enter controlling history");
      }
      state.Assume(hold, "10xx");
      state.Assume(aid, "1111");
      state.Assume(bid, "1111");
      state.Assume(shift, "1111");
      state.ApplyInputs({{enable, "1"}});
      Require(state.Value(hold) == "10xx" && state.Value(shift) == "0000" &&
                  state.Value(aid) == "0000" && state.Value(bid) == "0000",
              "simultaneous feedback deltas converge independently of "
              "declaration order");
      state.Assume(aid, "xxxx");
      state.Assume(bid, "xxxx");
      state.Assume(shift, "xxxx");
      state.ApplyInputs({{enable, "1"}});
      Require(state.Value(aid) == "0000" && state.Value(bid) == "0000" &&
                  state.Value(shift) == "0000",
              "feedback convergence can establish known zero bits without "
              "inventing hold bits");
      state.Drive(rst, "1");
      state.ApplyInputs({{enable, "1"}});
      Require(state.Value(hold) == "0000" && state.Value(aid) == "0000" &&
                  state.Value(bid) == "0000" && state.Value(shift) == "0000",
              "reset priority is retained in feedback groups");
    }
  }
}

void TestDataFeedbackClockAndRollback() {
  pattern::Design design;
  design.AddIcl("feedback_clock.icl", R"(Module Top {
 ScanInPort si;
 ScanRegister enable {ScanInSource si;}
 DataRegister shift[3:0] {WriteDataSource 1'b0,shift[3:1]; WriteEnSource enable;}
 ScanRegister observed[3:0] {ScanInSource enable; CaptureSource shift;}
 ScanOutPort so {Source observed[0];}
})");
  const auto netlist = pattern::Elaborate(design, "Top");
  const auto enable = netlist.node_ids.at("enable"),
             shift = netlist.node_ids.at("shift"),
             observed = netlist.node_ids.at("observed");
  pattern::NetworkState state(netlist);
  state.SetRegister(enable, "1", "0");
  state.Assume(shift, "1111");
  state.Clock(
      {{enable, {'0', '0', '0', '1'}}, {observed, {'1', '0', '0', '1'}}}, true);
  Require(state.Value(observed, true) == "1111" && state.Value(shift) == "1111",
          "capture uses the old data snapshot before any update settling");
  state.Clock({{enable, {'0', '0', '1', '1'}}}, false);
  Require(state.Value(shift) == "0000" && state.Value(observed, true) == "1111",
          "feedback settles only after the falling update event");

  pattern::Design oscillating;
  oscillating.AddIcl("oscillating.icl", R"(Module Top {
 DataInPort enable;
 DataRegister value {WriteDataSource ~value; WriteEnSource enable;}
})");
  const auto loop = pattern::Elaborate(oscillating, "Top");
  pattern::NetworkState failed(loop);
  const auto value = loop.node_ids.at("value"),
             gate = loop.node_ids.at("enable");
  failed.Assume(value, "0");
  failed.Drive(gate, "0");
  const pattern::NetworkState before(failed);
  bool rejected = false;
  try {
    failed.ApplyInputs({{gate, "1"}});
  } catch (const std::runtime_error& error) {
    rejected = std::string(error.what()).find("data-register feedback") !=
               std::string::npos;
  }
  Require(rejected && failed.SameState(before),
          "oscillating feedback rolls back pins, storage and events");
  failed.ApplyInputs({{gate, "0"}});
  Require(failed.Value(value) == "0",
          "disabled feedback retries without changing the saved value");
}

void TestOneHotSelectionFeedback() {
  pattern::Design design;
  design.AddIcl("one_hot_feedback.icl", R"(Module Driver {
 DataInPort gate; DataInPort data;
 DataOutPort q {Source data; Enable gate;}
}
Module Bank {
 DataInPort selector;
 Instance a Of Driver {InputPort gate = ~selector; InputPort data = 1'b1;}
 Instance b Of Driver {InputPort gate = selector; InputPort data = 1'b0;}
 OneHotDataGroup group {Port a.q; Port b.q;}
 DataOutPort q {Source group;}
}
Module Top {
 DataInPort enable;
 DataRegister value {WriteDataSource bank.q; WriteEnSource enable;}
 Instance bank Of Bank {InputPort selector = value;}
})");
  const auto netlist = pattern::Elaborate(design, "Top");
  pattern::NetworkState state(netlist);
  const auto value = netlist.node_ids.at("value"),
             enable = netlist.node_ids.at("enable");
  state.Assume(value, "0");
  state.Assume(enable, "0");
  const pattern::NetworkState before(state);
  bool rejected = false;
  try {
    state.ApplyInputs({{enable, "1"}});
  } catch (const std::runtime_error& error) {
    rejected = std::string(error.what()).find("data-register feedback") !=
               std::string::npos;
  }
  Require(rejected && state.SameState(before),
          "implicit one-hot selection feedback oscillates and rolls back");
  state.ApplyInputs({{enable, "0"}});
  Require(state.Value(value) == "0", "disabled selection feedback can retry");
}

void TestPinSnapshot() {
  pattern::Design design;
  design.AddIcl("snapshot.icl", R"(
Module Leaf {
  ResetPort rst; ScanInPort si;
  ScanRegister r { ScanInSource si; ResetValue 1'b0; }
}
Module Top {
  DataInPort reset_value; SelectPort selected;
  Instance leaf Of Leaf { InputPort rst = 1'b0; }
})");
  auto netlist = pattern::Elaborate(design, "Top");
  netlist.nodes.at(netlist.node_ids.at("leaf.rst")).inputs["Source"] = {
      {netlist.node_ids.at("reset_value"), 0}};
  pattern::CheckCombinationalConnections(netlist);
  pattern::NetworkState state(netlist);
  const auto reg = netlist.node_ids.at("leaf.r");
  const auto pin = netlist.node_ids.at("reset_value");
  state.Assume(reg, "1");
  state.Assume(pin, "1");
  state.Assume(netlist.node_ids.at("selected"), "x");
  Require(state.Value(reg) == "1" && state.TakeResetEvents().empty(),
          "iState pin assumptions do not propagate resets or execute events");
  bool rejected = false;
  try {
    state.Assume(pin, "10");
  } catch (const std::exception&) {
    rejected = true;
  }
  Require(rejected && state.Value(pin) == "1",
          "invalid pin width preserves assumed state");
  state.Reset();
  Require(state.Value(reg) == "0", "later reset replay uses assumed pins");
}

void TestLogicEvaluation() {
  pattern::Design design;
  design.AddIcl("logic-state.icl", R"(
Module LogicState {
  DataInPort a[1:0];
  LogicSignal inverted { a[1], ~a[0] == 2'b00; }
  DataMux select SelectedBy a[1] { 1'b0: ~a[0]; 1'b1: a[0]; }
  DataOutPort out { Source select; }
})");
  const auto netlist = pattern::Elaborate(design, "LogicState");
  pattern::NetworkState state(netlist);
  for (int value = 0; value < 4; ++value) {
    state.Drive(netlist.node_ids.at("a"),
                std::string{value & 2 ? '1' : '0', value & 1 ? '1' : '0'});
    Require(state.Value(netlist.node_ids.at("inverted")) ==
                (value == 1 ? "1" : "0"),
            "logic references are inverted once");
    Require(state.Value(netlist.node_ids.at("out")) ==
                (value == 0 || value == 3 ? "1" : "0"),
            "mux references are inverted once");
  }
  auto modeled = netlist;
  for (auto& term : modeled.nodes.at(modeled.node_ids.at("inverted")).logic) {
    if (!term.signals.empty()) {
      term.signals.at(0).inverted = !term.signals.at(0).inverted;
      break;
    }
  }
  pattern::NetworkState typed(modeled);
  typed.Drive(modeled.node_ids.at("a"), "01");
  pattern::Design replay;
  replay.AddIcl("logic-flat.icl", pattern::SerializeIcl(modeled));
  const auto flattened = pattern::Elaborate(replay, "LogicState");
  pattern::NetworkState copy(flattened);
  copy.Drive(flattened.node_ids.at("a"), "01");
  Require(typed.Value(modeled.node_ids.at("inverted")) ==
              copy.Value(flattened.node_ids.at("inverted")),
          "typed inverted references preserve logic through flat replay");
}
void TestResetReleaseOrdering() {
  pattern::Design design;
  design.AddIcl("reset-order.icl", R"(
Module Leaf {
  ResetPort rst; ScanInPort si;
  ScanRegister r { ScanInSource si; CaptureSource 1'b1; ResetValue 1'b0; }
}
Module Top {
  DataInPort value; ScanInPort si;
  ScanRegister reset_control { ScanInSource si; CaptureSource value; }
  Instance leaf Of Leaf { InputPort rst = 1'b0; }
})");
  auto netlist = pattern::Elaborate(design, "Top");
  netlist.nodes.at(netlist.node_ids.at("leaf.rst")).inputs["Source"] = {
      {netlist.node_ids.at("reset_control"), 0}};
  pattern::CheckCombinationalConnections(netlist);
  pattern::NetworkState state(netlist);
  const auto control = netlist.node_ids.at("reset_control"),
             target = netlist.node_ids.at("leaf.r");
  state.Drive(netlist.node_ids.at("value"), "1");
  state.Clock({{control, {'1', '0', '0', '1'}}}, true);
  state.Clock({{control, {'0', '0', '1', '1'}}}, false);
  Require(state.Value(target) == "0",
          "asserted local reset initializes target");
  state.TakeResetEvents();
  state.Drive(netlist.node_ids.at("value"), "0");
  state.Clock({{control, {'1', '0', '0', '1'}}, {target, {'1', '0', '0', '1'}}},
              true);
  state.TakeResetEvents();
  state.Clock({{control, {'0', '0', '1', '1'}}, {target, {'0', '0', '1', '1'}}},
              false);
  Require(
      state.Value(control) == "0" && state.Value(target) == "0",
      "same-edge reset release cannot enable protected update retroactively");
  Require(state.TakeResetEvents() == std::set<size_t>{target},
          "reset on the update edge survives deassertion during that edge");
  state.Clock({{target, {'0', '0', '1', '1'}}}, false);
  Require(state.Value(target) == "1",
          "released target updates on subsequent event");
}
std::string ReadFile(const std::string& path) {
  std::ifstream input(path);
  if (!input) throw std::runtime_error("missing fixture " + path);
  std::ostringstream text;
  text << input.rdbuf();
  return text.str();
}
void TestTapExecution(bool indexed, bool clock_off = false, bool shared = false,
                      bool implicit = false) {
  const std::string directory =
      "tests/cases/ours/pattern_generation/access_link/";
  pattern::Design design;
  auto source = ReadFile(directory + "simple.icl");
  if (indexed) {
    for (const auto& [from, to] :
         std::vector<std::pair<std::string, std::string>>{
             {"SelectPort select;", "SelectPort select[4:3];"},
             {"Port select;", "Port select[3];"},
             {"InputPort mode",
              "InputPort select[4] = 1'b0; InputPort mode"}}) {
      source.replace(source.find(from), from.size(), to);
    }
  }
  if (clock_off)
    source.insert(source.find("InputPort mode"), "InputPort clock = 1'b0; ");
  if (shared)
    source = R"(
Module Shared {
  ScanInPort si1; ScanInPort si2; SelectPort sel1; SelectPort sel2;
  CaptureEnPort ce; ShiftEnPort se; UpdateEnPort ue; TCKPort clk;
  ScanMux input SelectedBy sel1 { 1'b1:si1; 1'b0:si2; }
  ScanRegister r[7:0] { ScanInSource input; }
  ScanOutPort so1 { Source r[0]; } ScanOutPort so2 { Source r[0]; }
  ScanInterface a { Port si1; Port so1; Port sel1; Port ce; Port se; Port ue; Port clk; }
  ScanInterface b { Port si2; Port so2; Port sel2; Port ce; Port se; Port ue; Port clk; }
}
Module AccessTop {
  Instance core Of Shared;
  AccessLink tap Of STD_1149_1_2001 {
    BSDLEntity AccessProbe;
    IJTAG_ON { ScanInterface {core.a;} }
    IJTAG_OFF { ScanInterface {core.b;} }
  }
})";
  design.AddIcl("network.icl", source);
  design.AddBsdl("simple.bsdl", ReadFile(directory + "simple.bsdl"),
                 "AccessProbe");
  const auto network = pattern::ElaborateTapNetwork(design, "AccessTop");
  const auto flat = pattern::SerializeIcl(network.netlist);
  pattern::Design replay;
  replay.AddIcl("flat.icl", flat);
  auto restored = pattern::ElaborateTapNetwork(replay, "AccessTop");
  if (implicit) {
    for (auto& node : restored.netlist.nodes)
      if (node.scan_interface &&
          restored.netlist.scopes[node.scope].path == "core")
        node.scan_interface.reset();
  }
  pattern::TapExecution run(network.netlist), copy(restored.netlist);
  Require(run.Value("core.r") == "xxxxxxxx",
          "TAP construction does not reset child hardware");
  run.Drive("TRST", "0");
  copy.Drive("TRST", "0");
  run.Drive("TRST", "1");
  copy.Drive("TRST", "1");
  const auto cycle = [&](char tms, char tdi = '0') {
    for (auto* engine : {&run, &copy}) {
      engine->Drive("TMS", std::string(1, tms));
      engine->Drive("TDI", std::string(1, tdi));
      engine->RisingEdge("TCK");
      engine->FallingEdge("TCK");
    }
    Require(run.Value("core.r") == copy.Value("core.r") &&
                run.Value("core.r", true) == copy.Value("core.r", true) &&
                run.Value("TDO") == copy.Value("TDO"),
            "flat topology and runtime produce identical events");
  };
  cycle('0');
  for (char tms : std::string("1100")) cycle(tms);
  const std::string instruction = "1100";  // IJTAG_ON = 0011, LSB first.
  for (size_t i = 0; i < instruction.size(); ++i)
    cycle(i + 1 == instruction.size() ? '1' : '0', instruction[i]);
  cycle('1');
  cycle('0');
  for (char tms : std::string("100")) cycle(tms);
  const std::string data = "10011010";  // 01011001, LSB first.
  for (size_t i = 0; i < data.size(); ++i)
    cycle(i + 1 == data.size() ? '1' : '0', data[i]);
  Require(run.Value("core.r") == "xxxxxxxx",
          "DR shift has not updated instrument data");
  cycle('1');
  Require(run.Value("core.r") == (clock_off ? "xxxxxxxx" : "01011001"),
          "TAP falling UpdateDR updates selected child register");
  const auto events = run.TakeControlEvents();
  copy.TakeControlEvents();
  if (!clock_off) {
    Require(events.at(network.netlist.node_ids.at("core.r")) ==
                std::optional<std::string>("01011001"),
            "TAP exposes its authoritative known-write event");
  }
  cycle('0');
  Require(run.TakeControlEvents().empty() && copy.TakeControlEvents().empty(),
          "the following idle frame does not resurrect drained TAP events");
  for (char tms : std::string("1100")) cycle(tms);
  const std::string bypass_instruction = "0010";  // IJTAG_OFF = 0100.
  for (size_t i = 0; i < bypass_instruction.size(); ++i) {
    cycle(i + 1 == bypass_instruction.size() ? '1' : '0',
          bypass_instruction[i]);
  }
  cycle('1');
  cycle('0');
  for (char tms : std::string("100")) cycle(tms);
  for (size_t i = 0; i < 8; ++i) cycle(i == 7 ? '1' : '0', '0');
  cycle('1');
  Require(run.Value("core.r") == (clock_off ? "xxxxxxxx" : "00000000"),
          "explicitly enabled registers execute even when a serial mux "
          "bypasses them");
}

void TestSharedTapEdge(bool producer_first, const std::string& control,
                       bool separate_clocks = false) {
  const std::string producer = producer_first ? "a_source" : "z_source";
  pattern::Design design;
  design.AddIcl("shared_edge.icl", R"(
Module EdgeCell {
  ScanInPort si; ScanOutPort so { Source r; }
  SelectPort select; CaptureEnPort capture; ShiftEnPort shift;
  UpdateEnPort update; TCKPort clock;
  ToCaptureEnPort capture_out { Source capture; }
  ToShiftEnPort shift_out { Source shift; }
  ToUpdateEnPort update_out { Source update; }
  ScanRegister r { ScanInSource si; CaptureSource 1'b1; }
  ScanInterface client {
    Port si; Port so; Port select; Port capture; Port shift; Port update;
    Port clock;
  }
}
Module SharedEdge {
  Instance producer Of EdgeCell;
  Instance sink Of EdgeCell { InputPort )" +
                                       control + " = producer." + control +
                                       R"(_out; }
  AccessLink )" + producer + R"( Of STD_1149_1_2001 {
    BSDLEntity ProbeA;
    IJTAG_ON { ScanInterface { producer.client; } }
    IJTAG_OFF { ScanInterface { producer.client; } }
  }
  AccessLink m_sink Of STD_1149_1_2001 {
    BSDLEntity ProbeB;
    IJTAG_ON { ScanInterface { sink.client; } }
    IJTAG_OFF { ScanInterface { sink.client; } }
  }
})");
  for (const std::string suffix : {"A", "B"}) {
    auto bsdl =
        ReadFile("tests/cases/ours/pattern_generation/access_link/simple.bsdl");
    std::vector<std::string> tokens{"AccessProbe", "TDI", "TDO", "TMS", "TRST"};
    if (separate_clocks) tokens.push_back("TCK");
    for (const auto& token : tokens) {
      const auto replacement =
          token == "AccessProbe" ? "Probe" + suffix
          : token == "TCK"
              ? ((suffix == "A") == producer_first ? "TCK_A" : "TCK_Z")
              : token + suffix;
      size_t at = 0;
      while ((at = bsdl.find(token, at)) != std::string::npos) {
        bsdl.replace(at, token.size(), replacement);
        at += replacement.size();
      }
    }
    design.AddBsdl("probe" + suffix + ".bsdl", bsdl, "Probe" + suffix);
  }
  const auto network = pattern::ElaborateTapNetwork(design, "SharedEdge");
  pattern::Design flat;
  flat.AddIcl("flat.icl", pattern::SerializeIcl(network.netlist));
  const auto restored = pattern::ElaborateTapNetwork(flat, "SharedEdge");
  pattern::TapExecution run(network.netlist), copy(restored.netlist);
  for (auto* engine : {&run, &copy}) {
    for (const auto* reset : {"TRSTA", "TRSTB"}) {
      engine->Drive(reset, "0");
      engine->Drive(reset, "1");
    }
  }
  const auto cycle = [&](char tms, char tdi = '0') {
    for (auto* engine : {&run, &copy}) {
      engine->Drive("TMSA", std::string(1, tms));
      engine->Drive("TMSB", std::string(1, tms));
      engine->Drive("TDIA", std::string(1, tdi));
      engine->Drive("TDIB", std::string(1, tdi));
      const std::vector<std::string> clocks =
          separate_clocks ? std::vector<std::string>{"TCK_A", "TCK_Z"}
                          : std::vector<std::string>{"TCK"};
      if (separate_clocks) {
        const auto& netlist =
            engine == &run ? network.netlist : restored.netlist;
        std::set<size_t> ids;
        for (const auto& clock : clocks) ids.insert(netlist.node_ids.at(clock));
        engine->RisingEdge(ids);
        engine->FallingEdge(ids);
      } else {
        engine->RisingEdge("TCK");
        engine->FallingEdge("TCK");
      }
    }
    for (const auto* cell : {"producer.r", "sink.r"}) {
      Require(run.Value(cell) == copy.Value(cell) &&
                  run.Value(cell, true) == copy.Value(cell, true),
              "shared-edge original and flat state agree");
    }
  };
  cycle('0');
  for (char tms : std::string("1100")) cycle(tms);
  const std::string instruction = "1100";
  for (size_t i = 0; i < instruction.size(); ++i) {
    cycle(i + 1 == instruction.size() ? '1' : '0', instruction[i]);
  }
  cycle('1');
  cycle('0');
  for (char tms : std::string("100")) cycle(tms);
  Require(
      run.Value("producer.r", true) == "1" && run.Value("sink.r", true) == "1",
      "shared TCK capture uses all current controller outputs");
  cycle('1', '0');
  Require(run.Value("producer.r", true) == "0" &&
              run.Value("sink.r", true) == "0" && run.Value("sink.r") == "x",
          "shared TCK shift uses current controls without updating held data");
  cycle('1');
  Require(run.Value("producer.r") == "0" && run.Value("sink.r") == "0",
          "shared TCK update uses all current controller outputs");
  cycle('0');
  if (producer_first && control == "capture") {
    const auto started = std::chrono::steady_clock::now();
    for (size_t i = 0; i < 1000; ++i) cycle('0');
    Require(run.Value("producer.r") == "0" && run.Value("sink.r") == "0",
            "shared idle edges preserve both held registers");
    std::cout << "shared TAP replay: taps=2 roots=" << (separate_clocks ? 2 : 1)
              << " cycles=1000 forms=2 seconds="
              << std::chrono::duration<double>(
                     std::chrono::steady_clock::now() - started)
                     .count()
              << '\n';
  }
}

void TestEquivalentClockBatch() {
  const std::string fixture =
      "tests/cases/ours/pattern_generation/multiple_taps/";
  auto source = ReadFile(fixture + "multiple_explicit.icl");
  auto models = ReadFile(fixture + "models.bsdl");
  size_t at = models.find("entity ProbeB");
  Require(at != std::string::npos, "second TAP model exists");
  while ((at = models.find("TCK", at)) != std::string::npos) {
    models.replace(at, 3, "TCKB");
    at += 4;
  }
  const auto replace = [&](const std::string& from, const std::string& to) {
    const auto position = source.find(from);
    Require(position != std::string::npos, "clock fixture substitution exists");
    source.replace(position, from.size(), to);
  };
  replace("Module TapCell {",
          "Module TapCell { DataInPort observed[3:0]; DataOutPort q[3:0] { "
          "Source value; }");
  replace("ScanInSource si; ResetValue",
          "ScanInSource si; CaptureSource observed; ResetValue");
  replace("Module MultiTapTop {", "Module MultiTapTop { TCKPort extra;");
  replace("InputPort reset = TRSTA;",
          "InputPort reset = TRSTA; InputPort clock = extra; InputPort "
          "observed = b.q;");
  replace("InputPort reset = TRSTB;",
          "InputPort reset = TRSTB; InputPort observed = a.q;");
  pattern::Design design;
  design.AddIcl("clock-batch.icl", source);
  for (const std::string entity : {"ProbeA", "ProbeB"}) {
    design.AddBsdl("models.bsdl", models, entity);
  }
  const auto netlist =
      pattern::ElaborateTapNetwork(design, "MultiTapTop").netlist;
  pattern::Design flat_design;
  flat_design.AddIcl("flat.icl", pattern::SerializeIcl(netlist));
  const auto flat =
      pattern::ElaborateTapNetwork(flat_design, "MultiTapTop").netlist;
  for (const auto* model : {&netlist, &flat}) {
    const auto id = [&](const std::string& name) {
      return model->node_ids.at(name);
    };
    pattern::TapExecution run(*model);
    run.Drive({{id("TRSTA"), "0"}, {id("TRSTB"), "0"}});
    run.Drive({{id("TRSTA"), "1"}, {id("TRSTB"), "1"}});
    const std::vector<std::set<size_t>> invalid{
        {},
        {id("extra")},
        {id("TCK"), id("TDIA")},
        {id("TCK"), id("a.clock")},
        {id("TCK"), model->nodes.size()}};
    for (const auto& clocks : invalid) {
      for (bool rising : {false, true}) {
        const pattern::NetworkState before(run.State());
        bool rejected = false;
        try {
          if (rising)
            run.RisingEdge(clocks);
          else
            run.FallingEdge(clocks);
        } catch (const std::exception&) {
          rejected = true;
        }
        Require(rejected && run.State().SameState(before),
                "invalid clock batch leaves the entire state unchanged");
      }
    }
    const auto single_cycle = [&](char tms, char tdi = '0') {
      run.Drive({{id("TMSA"), std::string(1, tms)},
                 {id("TDIA"), std::string(1, tdi)}});
      run.RisingEdge("TCK");
      run.FallingEdge("TCK");
    };
    single_cycle('0');
    for (char tms : std::string("1100")) single_cycle(tms);
    for (size_t tick = 0; tick < 4; ++tick) {
      single_cycle(tick == 3 ? '1' : '0', std::string("1100")[tick]);
    }
    single_cycle('1');
    single_cycle('0');
    Require(run.Value("tapA.INSTRUCTION") == "0011" &&
                run.Value("tapB.INSTRUCTION") == "0010" &&
                run.Value("extra") == "x",
            "singleton edges advance only the observed TAP and clock");
    const std::set<size_t> clocks{id("TCK"), id("TCKB"), id("extra")};
    const auto cycle = [&](char tms, char a = '0', char b = '0') {
      run.Drive({{id("TMSA"), std::string(1, tms)},
                 {id("TMSB"), std::string(1, tms)},
                 {id("TDIA"), std::string(1, a)},
                 {id("TDIB"), std::string(1, b)}});
      run.RisingEdge(clocks);
      run.FallingEdge(clocks);
    };
    for (size_t tick = 0; tick < 5; ++tick) cycle('1');
    cycle('0');
    Require(run.Value("tapA.INSTRUCTION") == "0010" &&
                run.Value("tapB.INSTRUCTION") == "0010" &&
                run.Value("extra") == "0",
            "batch reset clocks include extra roots without their own TAP");
    for (char tms : std::string("1100")) cycle(tms);
    for (size_t tick = 0; tick < 4; ++tick) {
      const char bit = std::string("1100")[tick];
      cycle(tick == 3 ? '1' : '0', bit, bit);
    }
    cycle('1');
    cycle('0');
    for (char tms : std::string("100")) cycle(tms);
    for (size_t tick = 0; tick < 4; ++tick) {
      cycle(tick == 3 ? '1' : '0', std::string("1010")[tick],
            std::string("0101")[tick]);
    }
    Require(run.Value("a.value") == "0000" && run.Value("b.value") == "0000" &&
                run.Value("a.value", true) == "0101" &&
                run.Value("b.value", true) == "1010",
            "equivalent roots shift separate lanes before the common update");
    cycle('1');
    Require(run.Value("a.value") == "0101" && run.Value("b.value") == "1010",
            "one batch updates both held values including the extra-root "
            "instrument");
    cycle('0');
    for (char tms : std::string("100")) cycle(tms);
    Require(run.Value("a.value", true) == "1010" &&
                run.Value("b.value", true) == "0101",
            "cross-root captures sample the old shared held state");
  }
}

void TestInstructionReset() {
  const std::string directory =
      "tests/cases/ours/pattern_generation/access_link/";
  auto source = ReadFile(directory + "simple.icl");
  source.insert(source.find("ScanRegister r"), "ResetPort rst; ");
  source.insert(
      source.find("ScanInSource si;") + std::string("ScanInSource si;").size(),
      " ResetValue 8'b0;");
  source.insert(source.find("InputPort mode"), "InputPort rst = 1'b0; ");
  pattern::Design design;
  design.AddIcl("instruction-reset.icl", source);
  design.AddBsdl("simple.bsdl", ReadFile(directory + "simple.bsdl"),
                 "AccessProbe");
  auto network = pattern::ElaborateTapNetwork(design, "AccessTop");
  // A lowered decode-driven reset isolates IR-event propagation from port
  // rules.
  network.netlist.nodes.at(network.netlist.node_ids.at("core.rst"))
      .inputs["Source"] = {{network.netlist.node_ids.at("tap.long_setup"), 0}};
  pattern::CheckCombinationalConnections(network.netlist);
  pattern::TapExecution run(network.netlist);
  run.Drive("TRST", "0");
  run.Drive("TRST", "1");
  const auto cycle = [&](char tms, char tdi = '0') {
    run.Drive("TMS", std::string(1, tms));
    run.Drive("TDI", std::string(1, tdi));
    run.RisingEdge("TCK");
    run.FallingEdge("TCK");
  };
  cycle('0');
  for (char tms : std::string("1100")) cycle(tms);
  const std::string instruction = "1100";
  for (size_t i = 0; i < instruction.size(); ++i)
    cycle(i == 3 ? '1' : '0', instruction[i]);
  Require(run.Value("core.r") == "xxxxxxxx",
          "instruction shift does not assert decoded reset");
  cycle('1');
  Require(run.Value("core.r") == "00000000",
          "instruction latch immediately propagates decoded reset");
}
void TestLongChain() {
  const size_t count = 1000;
  std::string source = R"(Module Cell {
    ScanInPort si; ScanOutPort so { Source r[0]; }
    SelectPort sel; ShiftEnPort se; CaptureEnPort ce; UpdateEnPort ue;
    ScanRegister r[7:0] { ScanInSource si; }
    ScanInterface client { Port si; Port so; Port sel; Port se; Port ce; Port ue; }
  } Module LongChain { )";
  std::string interfaces;
  for (size_t i = 0; i < count; ++i) {
    const auto name = "u" + std::to_string(i);
    source += "Instance " + name + " Of Cell;";
    interfaces += name + ".client;";
  }
  source +=
      "AccessLink tap Of STD_1149_1_2001 { BSDLEntity AccessProbe; IJTAG_ON { "
      "ScanInterface {" +
      interfaces + "} } IJTAG_OFF { ScanInterface {" + interfaces + "} } } }";
  pattern::Design design;
  design.AddIcl("long.icl", source);
  design.AddBsdl(
      "simple.bsdl",
      ReadFile("tests/cases/ours/pattern_generation/access_link/simple.bsdl"),
      "AccessProbe");
  const auto network = pattern::ElaborateTapNetwork(design, "LongChain");
  const auto started = std::chrono::steady_clock::now();
  pattern::TapExecution run(network.netlist);
  Require(run.Value("u999.r") == "xxxxxxxx",
          "large serial network retains unknown initial state");
  std::cout << "TAP runtime: interfaces=" << count << " construction seconds="
            << std::chrono::duration<double>(std::chrono::steady_clock::now() -
                                             started)
                   .count()
            << '\n';
}
}  // namespace
int main() {
  try {
    TestCompleteShift();
    TestImpliedToTckValues();
    TestEquivalentClockBatch();
    for (const auto* control : {"capture", "shift", "update"}) {
      TestSharedTapEdge(true, control);
      TestSharedTapEdge(false, control);
      TestSharedTapEdge(true, control, true);
      TestSharedTapEdge(false, control, true);
    }
    TestFeedbackResetSnapshots();
    TestControllingEventHistory();
    TestLogicEvaluation();
    TestPinSnapshot();
    TestResetReleaseOrdering();
    TestRegisterEvents();
    TestDataRegisterStateSnapshot();
    TestDataRegisterEvents();
    TestDataRegisterFeedback();
    TestDataFeedbackClockAndRollback();
    TestOneHotSelectionFeedback();
    TestAddressedDataEvents();
    TestAmbiguousDataAddress();
    TestDataCascadeScaleAndFeedback();
    TestTapExecution(false);
    TestTapExecution(true);
    TestTapExecution(false, true);
    TestTapExecution(false, false, true);
    TestTapExecution(false, true, false, true);
    TestInstructionReset();
    TestLongChain();
    std::cout << "pattern state tests passed\n";
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
