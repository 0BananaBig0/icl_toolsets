// Copyright 2026 The ICL Toolsets Authors.
// SPDX-License-Identifier: MIT
#include "pattern_state.h"

#include <chrono>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>

#include "pattern_design.h"
#include "pattern_flatten.h"
#include "pattern_tap_execution.h"

namespace {
void Require(bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
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
  Require(state.Value(a) == "0011" && state.Value(b) == "0101" &&
              state.Value(a, true) == "1101",
          "reset affects update state regardless of select, not shift state");
  state.Clock({{a, {'0', '1', '0', '1'}}}, true);
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
  Instance leaf Of Leaf { InputPort rst = reset_control; }
})");
  const auto netlist = pattern::Elaborate(design, "Top");
  pattern::NetworkState state(netlist);
  const auto control = netlist.node_ids.at("reset_control"),
             target = netlist.node_ids.at("leaf.r");
  state.Drive(netlist.node_ids.at("value"), "1");
  state.Clock({{control, {'1', '0', '0', '1'}}}, true);
  state.Clock({{control, {'0', '0', '1', '1'}}}, false);
  Require(state.Value(target) == "0",
          "asserted local reset initializes target");
  state.Drive(netlist.node_ids.at("value"), "0");
  state.Clock({{control, {'1', '0', '0', '1'}}, {target, {'1', '0', '0', '1'}}},
              true);
  state.Clock({{control, {'0', '0', '1', '1'}}, {target, {'0', '0', '1', '1'}}},
              false);
  Require(
      state.Value(control) == "0" && state.Value(target) == "0",
      "same-edge reset release cannot enable protected update retroactively");
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
  cycle('0');
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

void TestInstructionReset() {
  const std::string directory =
      "tests/cases/ours/pattern_generation/access_link/";
  auto source = ReadFile(directory + "simple.icl");
  source.insert(source.find("ScanRegister r"), "ResetPort rst; ");
  source.insert(
      source.find("ScanInSource si;") + std::string("ScanInSource si;").size(),
      " ResetValue 8'b0;");
  source.insert(source.find("InputPort mode"),
                "InputPort rst = tap.long_setup; ");
  pattern::Design design;
  design.AddIcl("instruction-reset.icl", source);
  design.AddBsdl("simple.bsdl", ReadFile(directory + "simple.bsdl"),
                 "AccessProbe");
  const auto network = pattern::ElaborateTapNetwork(design, "AccessTop");
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
    TestLogicEvaluation();
    TestResetReleaseOrdering();
    TestRegisterEvents();
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
