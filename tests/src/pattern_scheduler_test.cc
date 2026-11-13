// Copyright 2026 The ICL Toolsets Authors.
// SPDX-License-Identifier: MIT
#include "pattern/pattern_scheduler.h"

#include <chrono>
#include <fstream>
#include <iostream>
#include <limits>
#include <map>
#include <stdexcept>
#include <tuple>

#include "pattern/pattern_data.h"
#include "pattern/pattern_design.h"
#include "pattern/pattern_flatten.h"
#include "pattern/pattern_merge.h"
#include "pattern/pattern_tap.h"

namespace {
void Require(bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}
void TestOneHotSharedAnalysisBudget() {
  pattern::Design design;
  design.AddIcl("one-hot-budget.icl", R"(
Module Driver {
  DataInPort data[4095:0]; DataInPort enable;
  DataOutPort terminal[4095:0] { Source data; Enable enable; }
  DataOutPort forwarded[4095:0] { Source terminal; }
}
Module OneHotBudget {
  DataInPort data[4095:0]; DataInPort enable;
  Instance driver Of Driver { InputPort data = data; InputPort enable = enable; }
  OneHotDataGroup group { Port driver.forwarded[0]; }
}
)");
  const auto netlist = pattern::Elaborate(design, "OneHotBudget");
  const auto& source = netlist.NodeAt("group").one_hot_sources.at(0);
  size_t budget = 8;
  pattern::OneHotReadResolver bounded(netlist, &budget);
  bool limited = false;
  try {
    bounded.Resolve(source);
  } catch (const std::runtime_error& error) {
    limited =
        std::string(error.what()).find("resource limit") != std::string::npos;
  }
  Require(limited && budget == 0,
          "implicit one-hot traversal consumes the shared budget");
  budget = 8192;
  pattern::OneHotReadResolver cached(netlist, &budget);
  Require(cached.Resolve(source).size() == 1 && budget < 8192,
          "successful selection charges dependency traversal");
  budget = 0;
  Require(cached.Resolve(source).size() == 1,
          "cached selection needs no new traversal budget");
  pattern::OneHotReadResolver ordinary(netlist);
  Require(ordinary.Resolve(source).size() == 1,
          "ordinary callers retain the per-source budget");
}
void TestMergeAnalysisBudget() {
  pattern::Design design;
  design.AddIcl("merge-budget.icl", R"(
Module MergeBudget {
  DataOutPort wide[4095:0] { Source 4096'b0; }
  DataInPort a; DataInPort b;
}
)");
  const auto netlist = pattern::Elaborate(design, "MergeBudget");
  std::vector<pattern::Operation> operations{
      {"iMerge", {"-begin"}, "test:1", {}}};
  for (size_t i = 0; i < 1024; ++i) {
    const std::vector<std::string> call{"wide" + std::to_string(i)};
    operations.push_back({"iWrite", {"wide", "0"}, "test:1", call});
    operations.push_back({"iApply", {}, "test:1", call});
  }
  for (const std::string name : {"a", "b"}) {
    operations.push_back({"iWrite", {name, "1"}, "test:1", {name}});
    operations.push_back({"iApply", {}, "test:1", {name}});
  }
  operations.push_back({"iMerge", {"-end"}, "test:1", {}});
  const auto start = std::chrono::steady_clock::now();
  const auto result = pattern::MergeOperations(netlist, operations, true);
  Require(result.size() + 2 == operations.size(),
          "exhausted merge analysis preserves the serial operations");
  for (size_t i = 0; i < result.size(); ++i) {
    Require(result[i].command == operations[i + 1].command &&
                result[i].arguments == operations[i + 1].arguments &&
                result[i].call_stack == operations[i + 1].call_stack,
            "analysis fallback retains operands and call ownership");
  }
  std::cout << "merge budget: 1024 calls over a 4096-bit port, seconds="
            << std::chrono::duration<double>(std::chrono::steady_clock::now() -
                                             start)
                   .count()
            << '\n';
}
void TestSharedScanOutputs() {
  pattern::Design design;
  design.AddIcl("shared-outputs.icl", R"(
Module SharedTop {
  ScanInPort si;
  ScanRegister data[3:0] { ScanInSource si; ResetValue 4'b0; }
  ScanOutPort a { Source ~data[0]; }
  ScanOutPort b { Source data[0]; }
})");
  const auto netlist = pattern::Elaborate(design, "SharedTop");
  pattern::ScanScheduler scheduler(netlist);
  scheduler.Reset();
  const auto first = scheduler.Apply({{"iWrite", {"data", "9"}, "write", {}}});
  Require(first.size() == 1 && first[0].output == "a" &&
              first[0].serial == "1001" && first[0].loads.size() == 1 &&
              scheduler.Value("data") == "1001" &&
              scheduler.Value("a") == "0" && scheduler.Value("b") == "1",
          "shared outputs clock their common storage once");
  const auto second = scheduler.Apply({{"iRead", {"data", "9"}, "read", {}},
                                       {"iWrite", {"data", "6"}, "write", {}}});
  Require(second.size() == 1 && second[0].serial_expected == "0110" &&
              scheduler.Value("data") == "0110" &&
              scheduler.Value("a") == "1" && scheduler.Value("b") == "0",
          "inverted shared observation preserves physical readback and writes");
  bool rejected = false;
  try {
    scheduler.Apply({{"iWrite", {"data", "9"}, "reserved", {}}}, false,
                    {{netlist.node_ids.at("data"), "0110"}}, false);
  } catch (const std::exception&) {
    rejected = true;
  }
  Require(rejected && scheduler.Value("data") == "0110" &&
              scheduler.Value("a") == "1" && scheduler.Value("b") == "0",
          "shared-output reservation failure rolls back all observations");
}
void TestEnabledScanOutputs() {
  pattern::Design design;
  design.AddIcl("enabled-outputs.icl", R"(
Module SharedTop {
  ScanInPort si;
  DataInPort ena; DataInPort enb;
  ScanRegister data[3:0] { ScanInSource si; ResetValue 4'b0; }
  ScanOutPort a { Source data[0]; Enable ena; }
  ScanOutPort b { Source ~data[0]; Enable enb; }
})");
  const auto netlist = pattern::Elaborate(design, "SharedTop");
  pattern::ScanScheduler scheduler(netlist);
  scheduler.Reset();
  scheduler.Assume("ena", "0");
  scheduler.Assume("enb", "1");
  const auto first = scheduler.Apply({{"iWrite", {"data", "9"}, "write", {}}});
  Require(first.size() == 1 && first[0].output == "b" &&
              first[0].serial == "1001" && scheduler.Value("data") == "1001",
          "enabled observation chooses the physical path without extra shifts");
  const auto read = scheduler.Apply({{"iRead", {"data", "9"}, "read", {}}});
  Require(read.size() == 1 && read[0].output == "b" &&
              read[0].serial_expected == "0110",
          "gated inverted output retains serial comparison polarity");
  scheduler.Assume("ena", "1");
  scheduler.Assume("enb", "0");
  const auto second = scheduler.Apply({{"iWrite", {"data", "6"}, "write", {}}});
  Require(second.size() == 1 && second[0].output == "a" &&
              scheduler.Value("data") == "0110",
          "changing observation enable preserves common storage");
  scheduler.Assume("ena", "0");
  bool rejected = false;
  try {
    scheduler.Apply(
        {{"iWrite", {"data", "9"}, "blocked", {}}}, false,
        {{netlist.node_ids.at("ena"), "0"}, {netlist.node_ids.at("enb"), "0"}});
  } catch (const std::exception&) {
    rejected = true;
  }
  Require(rejected && scheduler.Value("data") == "0110",
          "no enabled output fails without changing storage");
  scheduler.Assume("enb", "1");
  rejected = false;
  try {
    scheduler.Apply({{"iWrite", {"data", "9"}, "reserved", {}}}, false,
                    {{netlist.node_ids.at("data"), "0110"}}, false);
  } catch (const std::exception&) {
    rejected = true;
  }
  Require(rejected && scheduler.Value("data") == "0110",
          "gated observation preserves reservation rollback");
}
void TestClientScanControls() {
  pattern::Design design;
  design.AddIcl("client-controls.icl", R"(
Module ClientTop {
  ScanInPort si; ScanOutPort so { Source value[0]; }
  SelectPort sel; CaptureEnPort ce; ShiftEnPort se; UpdateEnPort ue; TCKPort ck;
  DataInPort data;
  DataRegister d { WriteDataSource data; WriteEnSource ue; ResetValue 1'b0; }
  ScanRegister value[3:0] { ScanInSource si; ResetValue 4'b0; CaptureSource value; }
  ScanInterface client { Port si; Port so; Port sel; Port ce; Port se; Port ue; Port ck; }
})");
  const auto netlist = pattern::Elaborate(design, "ClientTop");
  pattern::ScanScheduler scheduler(netlist);
  scheduler.Reset();
  scheduler.Assume("data", "0");
  const auto steps = scheduler.Apply({{"iWrite", {"value", "9"}, "write", {}}});
  Require(steps.size() == 1 && steps[0].serial == "1001" &&
              scheduler.Value("value") == "1001" &&
              scheduler.Value("ce") == "0" && scheduler.Value("se") == "0" &&
              scheduler.Value("ue") == "0" && scheduler.Value("ck") == "0",
          "client control phases write storage and return clock/enables idle");
  scheduler.Apply({{"iWrite", {"data", "1"}, "parallel", {}}});
  Require(scheduler.Value("data") == "1" && scheduler.Value("d") == "0" &&
              scheduler.Value("ue") == "0" && scheduler.Value("ck") == "0",
          "parallel frame leaves client update disabled and storage unchanged");
  const auto read = scheduler.Apply({{"iRead", {"value", "9"}, "read", {}},
                                     {"iWrite", {"value", "6"}, "write", {}}});
  Require(read.size() == 2 && read[0].loads.empty() &&
              read[0].writes.at("data") == "0" &&
              read.back().serial_expected == "1001" &&
              scheduler.Value("value") == "0110" && scheduler.Value("d") == "0",
          "client scan restores parallel data before capture and update");
  bool rejected = false;
  try {
    scheduler.Apply({{"iWrite", {"value", "9"}, "reserved", {}}}, false,
                    {{netlist.node_ids.at("value"), "0110"}}, false);
  } catch (const std::exception&) {
    rejected = true;
  }
  Require(rejected && scheduler.Value("value") == "0110" &&
              scheduler.Value("ck") == "0" && scheduler.Value("ue") == "0",
          "failed client access rolls back storage and interface pins");
  pattern::Design vector_design;
  vector_design.AddIcl("vector-client.icl", R"(
Module ClientTop {
  ScanInPort si; ScanOutPort so { Source value[0]; }
  SelectPort sel[5:2]; CaptureEnPort ce[5:2]; ShiftEnPort se[5:2];
  UpdateEnPort ue[5:2]; TCKPort ck[5:2];
  ScanRegister value[3:0] { ScanInSource si; ResetValue 4'b0; }
  ScanInterface client {
    Port si; Port so; Port sel[3]; Port ce[3]; Port se[3]; Port ue[3]; Port ck[3];
  }
})");
  const auto vector_netlist = pattern::Elaborate(vector_design, "ClientTop");
  pattern::ScanScheduler vector_scheduler(vector_netlist);
  vector_scheduler.Reset();
  vector_scheduler.Apply({{"iWrite", {"value", "9"}, "write", {}}});
  Require(vector_scheduler.Value("value") == "1001" &&
              vector_scheduler.Value("sel") == "xx1x" &&
              vector_scheduler.Value("ce") == "xx0x" &&
              vector_scheduler.Value("se") == "xx0x" &&
              vector_scheduler.Value("ue") == "xx0x" &&
              vector_scheduler.Value("ck") == "xx0x",
          "client phases drive only the interface's indexed control lanes");
}
void TestMultipleClientSelection() {
  const std::string path =
      "tests/cases/ours/pattern_generation/multiple_clients.icl";
  std::ifstream input(path);
  Require(input.good(), "read shared-client fixture");
  const std::string source((std::istreambuf_iterator<char>(input)), {});
  pattern::Design design;
  design.AddIcl(path, source);
  const auto netlist = pattern::Elaborate(design, "Chains");
  pattern::ScanScheduler scheduler(netlist);
  scheduler.Reset();
  scheduler.Apply({{"iWrite", {"route", "0"}, "route", {}}});
  const auto first =
      scheduler.Apply({{"iWrite", {"c.value", "9"}, "first", {}}});
  Require(first.size() == 1 && first[0].input == "si[0]" &&
              first[0].output == "so0" && first[0].serial == "1001" &&
              scheduler.Value("c.value") == "1001" &&
              scheduler.Value("sel") == "01" && scheduler.Value("si") == "x1",
          "first client selects one path and leaves unused input undriven");
  scheduler.Apply({{"iWrite", {"route", "1"}, "route", {}}});
  Require(
      scheduler.Value("sel") == "01" && scheduler.Value("c.value") == "1001",
      "parallel frame preserves previous selection and shared storage");
  const auto second =
      scheduler.Apply({{"iRead", {"c.value", "9"}, "read", {}},
                       {"iWrite", {"c.value", "6"}, "second", {}}});
  Require(second.size() == 1 && second[0].input == "si[1]" &&
              second[0].output == "so1" && second[0].serial == "0110" &&
              second[0].serial_expected == "1001" &&
              scheduler.Value("c.value") == "0110" &&
              scheduler.Value("sel") == "10" && scheduler.Value("ck") == "0",
          "second client captures common prior state before updating it");
  bool rejected = false;
  try {
    scheduler.Apply({{"iWrite", {"c.value", "3"}, "reserved", {}}}, false,
                    {{netlist.node_ids.at("c.value"), "0110"}}, false);
  } catch (const std::exception&) {
    rejected = true;
  }
  Require(rejected && scheduler.Value("c.value") == "0110" &&
              scheduler.Value("sel") == "10",
          "failed shared-state request restores storage and interface pins");
}
void TestSimultaneousScanChains() {
  pattern::Design design;
  design.AddIcl("simultaneous.icl", R"(
Module Chains {
  ScanInPort si[3:0];
  ScanRegister a[3:0] { ScanInSource si[3]; CaptureSource a; ResetValue 4'b0; }
  ScanRegister b[1:0] { ScanInSource si[1]; CaptureSource b; ResetValue 2'b0; }
  ScanOutPort so[5:4] { Source a[0], b[0]; }
  SelectPort sel; CaptureEnPort ce; ShiftEnPort se; UpdateEnPort ue; TCKPort ck;
  ScanInterface client {
    Chain first { Port si[3]; Port so[5]; }
    Chain second { Port si[1]; Port so[4]; }
    Port sel; Port ce; Port se; Port ue; Port ck;
  }
})");
  const auto netlist = pattern::Elaborate(design, "Chains");
  pattern::ScanScheduler scheduler(netlist);
  scheduler.Reset();
  const auto first = scheduler.Apply({{"iWrite", {"a", "9"}, "write", {}},
                                      {"iWrite", {"b", "2"}, "write", {}}},
                                     true);
  Require(first.size() == 1 && first[0].lanes.size() == 2 &&
              first[0].serial.empty() && first[0].lanes[0].input == "si[1]" &&
              first[0].lanes[0].serial == "0001" &&
              first[0].lanes[1].input == "si[3]" &&
              first[0].lanes[1].serial == "1001" &&
              scheduler.Value("a") == "1001" && scheduler.Value("b") == "10" &&
              scheduler.Value("si") == "1x1x",
          "unequal chains shift together with leading padding and intact "
          "unused pins");
  const auto second = scheduler.Apply({{"iRead", {"a", "9"}, "read", {}},
                                       {"iRead", {"b", "2"}, "read", {}},
                                       {"iWrite", {"a", "6"}, "write", {}},
                                       {"iWrite", {"b", "1"}, "write", {}}},
                                      true);
  Require(second.size() == 1 && second[0].lanes[0].serial_expected == "01xx" &&
              second[0].lanes[1].serial_expected == "1001" &&
              scheduler.Value("a") == "0110" && scheduler.Value("b") == "01" &&
              scheduler.Value("ck") == "0" && scheduler.Value("ue") == "0",
          "parallel chain readback precedes the shared update and pads "
          "comparisons");
  bool rejected = false;
  try {
    scheduler.Apply({{"iWrite", {"a", "9"}, "reserved", {}}}, false,
                    {{netlist.node_ids.at("a"), "0110"}}, false);
  } catch (const std::exception&) {
    rejected = true;
  }
  Require(rejected && scheduler.Value("a") == "0110" &&
              scheduler.Value("b") == "01" && scheduler.Value("ck") == "0",
          "failed simultaneous access rolls back both chains");
}
void TestHiddenClientEvents() {
  for (bool parallel : {false, true}) {
    std::string source = R"(
Module Hidden {
  ScanInPort si[5:2];
  SelectPort selected; CaptureEnPort capture; ShiftEnPort shift;
  UpdateEnPort update; TCKPort clock;
  ScanRegister gate[1:0] { ScanInSource si[3]; ResetValue 2'b0; }
  ScanRegister target[3:0] {
    ScanInSource gate[0]; CaptureSource 4'b1010; ResetValue 4'b0;
  }
  ScanMux route SelectedBy gate[0] { 1'b0: gate[0]; 1'b1: target[0]; }
  ScanOutPort so { Source route; }
  ScanInterface client {
    Port si[3]; Port so; Port selected; Port capture; Port shift; Port update; Port clock;
  }
})";
    if (parallel) {
      const std::string ports = "Port si[3]; Port so;";
      source.replace(source.find(ports), ports.size(), R"(
        Chain first { Port si[3]; Port so; }
        Chain second { Port si[2]; Port other_so; }
      )");
      source.insert(source.find("  ScanInterface"), R"(
        ScanRegister other[3:0] { ScanInSource si[2]; ResetValue 4'b0; }
        ScanOutPort other_so { Source other[0]; }
      )");
    }
    pattern::Design design;
    design.AddIcl("hidden-client.icl", source);
    const auto netlist = pattern::Elaborate(design, "Hidden");
    pattern::ScanScheduler scheduler(netlist);
    scheduler.Reset();
    bool rejected = false;
    try {
      scheduler.Apply({{"iWrite", {"gate", "2"}, "blocked", {}}}, false,
                      {{netlist.node_ids.at("target"), "0000"}}, false);
    } catch (const std::exception&) {
      rejected = true;
    }
    Require(rejected && scheduler.Value("gate") == "00" &&
                scheduler.Value("target") == "0000",
            "hidden client reservation failure rolls back visible and hidden "
            "storage");
    const auto first =
        scheduler.Apply({{"iWrite", {"gate", "2"}, "write", {}}});
    Require(
        first.size() == 1 && first[0].loads.count("target") == 0 &&
            first[0].writes.count("target") == 0 &&
            scheduler.Value("target") == (parallel ? "00xx" : "xx10") &&
            scheduler.Value("si") == (parallel ? "xx10" : "xx1x"),
        "hidden capture and shifts use actual serial bits including padding");
    scheduler.Apply({{"iWrite", {"target", "5"}, "write", {}}});
    Require(
        scheduler.Value("target") == "0101" &&
            scheduler.Value("clock") == "0" && scheduler.Value("update") == "0",
        "opening the hidden path permits a visible write and idle controls");
  }
}
void TestVectorScanInputs() {
  std::ifstream file(
      "tests/cases/ours/pattern_generation/vector_scan_inputs/network.icl");
  const std::string original(std::istreambuf_iterator<char>(file), {});
  for (bool ascending : {false, true}) {
    auto source = original;
    if (ascending) source.replace(source.find("5:2"), 3, "2:5");
    pattern::Design design;
    design.AddIcl("vector-input.icl", source);
    const auto netlist = pattern::Elaborate(design, "VectorScanTop");
    pattern::ScanScheduler scheduler(netlist);
    scheduler.Reset();
    const auto first =
        scheduler.Apply({{"iWrite", {"data", "9"}, "write", {}},
                         {"iWrite", {"select", "1"}, "write", {}}});
    Require(first.size() == 1 && first[0].input == "si[2]" &&
                first[0].output == "so[7]" && first[0].serial == "11001" &&
                scheduler.Value("data") == "1001" &&
                scheduler.Value("si[4]") == "x" &&
                scheduler.Value("si[2]") == "1",
            "vector scan input drives only the selected bit");
    const auto before = scheduler.Value("si");
    bool rejected = false;
    try {
      scheduler.Apply({{"iWrite", {"data", "6"}, "reserved", {}}}, false,
                      {{netlist.node_ids.at("data"), "1001"}}, false);
    } catch (const std::exception&) {
      rejected = true;
    }
    Require(rejected && scheduler.Value("si") == before &&
                scheduler.Value("data") == "1001",
            "failed vector-input scan preserves input and register state");
    const auto second =
        scheduler.Apply({{"iRead", {"data", "9"}, "read", {}},
                         {"iWrite", {"data", "6"}, "write", {}},
                         {"iWrite", {"select", "0"}, "write", {}}});
    Require(second.size() == 1 && second[0].input == "si[4]" &&
                second[0].serial == "11001" &&
                scheduler.Value("data") == "0110" &&
                scheduler.Value("si[4]") == "1" &&
                scheduler.Value("si[2]") == "1" &&
                scheduler.Value("si[5:3]") == "x1x",
            "mux-selected inverted scan input retains other bus bits");
  }
}
void TestMixedReadRoutes() {
  pattern::Design design;
  design.AddIcl("mixed-routes.icl", R"(
Module MixedRoutes {
  ScanInPort si;
  DataInPort data[3:0]; DataInPort enable;
  DataRegister value[3:0] { WriteDataSource data; WriteEnSource enable; }
  ScanRegister capture[3:0] { ScanInSource si; CaptureSource ~value; }
  ScanOutPort so { Source capture[0]; }
  DataOutPort observed[3:0] { Source value; Enable enable; }
}
)");
  const auto netlist = pattern::Elaborate(design, "MixedRoutes");
  const pattern::DataReadRoutes routes(netlist, pattern::DataReadMode::kBoth);
  const auto value = netlist.node_ids.at("value");
  for (int64_t bit = 0; bit < 4; ++bit) {
    const auto& choices = routes.routes.at({value, bit});
    Require(choices.size() == 2, "mixed reads retain both observation routes");
    for (const auto& route : choices) {
      const bool capture =
          netlist.nodes[route.output.node].kind == "ScanRegister";
      Require(route.output.inverted == capture &&
                  route.guards.size() == (capture ? 0U : 1U) &&
                  routes.storage.at({route.output.node, route.output.index})
                      .count(value),
              "mixed routes retain inversion, guards and observed storage");
    }
  }
}
void TestMixedIoPhases() {
  for (bool tap : {false, true}) {
    for (bool primary_data : {false, true}) {
      std::ifstream file(
          std::string("tests/cases/ours/pattern_generation/mixed_io/") +
          (primary_data ? "primary_data.icl" : "scan_data.icl"));
      pattern::Design design;
      const std::string instrument =
          primary_data ? "MixedPrimaryData" : "MixedScanData";
      std::string source(std::istreambuf_iterator<char>(file), {});
      const std::string pin = primary_data ? "data" : "enable";
      if (tap) {
        source.insert(source.find("  ScanInPort si;"), R"(
  SelectPort select; CaptureEnPort capture; ShiftEnPort shift;
  UpdateEnPort update; TCKPort clock;
  ScanInterface client {
    Port si; Port so; Port select; Port capture; Port shift; Port update; Port clock;
  }
)");
        source += "Module Probe { DataInPort " + pin +
                  (primary_data ? "[3:0];" : ";") + " Instance core Of " +
                  instrument + " { InputPort " + pin + " = " + pin + "; }" + R"(
  DataOutPort observed[3:0] { Source core.observed; }
  AccessLink tap Of STD_1149_1_2001 {
    BSDLEntity AccessProbe;
    IJTAG_ON { ScanInterface { core.client; } }
    IJTAG_OFF { ScanInterface { core.client; } }
  }
})";
        std::ifstream bsdl(
            "tests/cases/ours/pattern_generation/access_link/simple.bsdl");
        design.AddBsdl("simple.bsdl",
                       std::string(std::istreambuf_iterator<char>(bsdl), {}),
                       "AccessProbe");
      }
      design.AddIcl("mixed.icl", source);
      const auto netlist =
          tap ? pattern::ElaborateTapNetwork(design, "Probe").netlist
              : pattern::Elaborate(design, instrument);
      const std::string prefix = tap ? "core." : "";
      const auto target = prefix + "value";
      pattern::ScanScheduler scheduler(netlist), replay(netlist);
      scheduler.Reset();
      replay.Reset();
      const auto first =
          scheduler.Apply({{"iWrite", {target, "5"}, "mixed:1", {}}});
      const auto second =
          scheduler.Apply({{"iRead", {target, "5"}, "mixed:2", {}},
                           {"iWrite", {target, "9"}, "mixed:3", {}}});
      bool observed = false, controller_updated = false;
      for (const auto* group : {&first, &second}) {
        for (const auto& step : *group) {
          if (step.expected.count("observed")) {
            Require(step.writes.empty() && replay.Value("observed") == "0101",
                    "parallel observation precedes the stored-value change");
            observed = true;
          }
          if (group == &second &&
              step.writes.count(prefix + (primary_data ? "enable" : "data"))) {
            controller_updated = true;
          }
          std::vector<pattern::Operation> operations;
          for (const auto& [name, value] : step.writes) {
            operations.push_back(
                {"iWrite", {name, "0b" + value}, "replay", {}});
          }
          for (const auto& [name, value] : step.expected) {
            operations.push_back({"iRead", {name, "0b" + value}, "replay", {}});
          }
          replay.Apply(operations);
        }
      }
      Require(observed && controller_updated &&
                  scheduler.Value(target) == "1001" &&
                  replay.Value(target) == "1001",
              "mixed frames retain observations, controller updates and final "
              "state");
      const auto before = scheduler.Value(pin);
      const auto instruction = tap ? scheduler.Value("tap.INSTRUCTION") : "";
      bool rejected = false;
      try {
        scheduler.Apply({{"iWrite", {target, "3"}, "mixed:4", {}}}, false,
                        {{netlist.node_ids.at(target), "1001"}}, false);
      } catch (const std::exception& error) {
        rejected =
            std::string(error.what()).find("reserved") != std::string::npos;
      }
      Require(
          rejected && scheduler.Value(target) == "1001" &&
              scheduler.Value(pin) == before &&
              (!tap || scheduler.Value("tap.INSTRUCTION") == instruction),
          "failed mixed plans preserve storage, drivers and TAP instruction");
      scheduler.Apply({{"iWrite", {target, "3"}, "mixed:5", {}}});
      Require(scheduler.Value(target) == "0011",
              "mixed scheduler remains usable after rollback");
    }
  }
}
void TestPrimaryObservationOfScanStorage() {
  std::ifstream file(
      "tests/cases/ours/pattern_generation/mixed_io/scan_observation.icl");
  pattern::Design design;
  design.AddIcl("scan-observation.icl",
                std::string(std::istreambuf_iterator<char>(file), {}));
  const auto netlist = pattern::Elaborate(design, "MixedScanObservation");
  pattern::ScanScheduler scheduler(netlist), replay(netlist);
  scheduler.Reset();
  replay.Reset();
  const auto steps = scheduler.Apply({{"iRead", {"observed", "0"}, "read", {}},
                                      {"iWrite", {"data", "9"}, "write", {}}});
  bool observed = false;
  for (const auto& step : steps) {
    if (step.expected.count("observed")) {
      Require(replay.Value("observed") == "0000",
              "primary comparison observes old scan storage");
      observed = true;
    }
    std::vector<pattern::Operation> writes;
    for (const auto& [name, value] : step.writes) {
      writes.push_back({"iWrite", {name, "0b" + value}, "replay", {}});
    }
    if (!writes.empty()) replay.Apply(writes);
  }
  Require(observed && replay.Value("data") == "1001",
          "scan update follows observation");
}
void TestPrimaryAccessTogether() {
  for (bool serial : {false, true}) {
    pattern::Design design;
    const std::string scan = serial ? R"(
  ScanInPort si;
  ScanRegister sr { ScanInSource si; ResetValue 1'b0; }
  ScanOutPort so { Source sr; }
)"
                                    : "";
    design.AddIcl("primary-groups.icl", "Module PrimaryGroups {" + scan + R"(
  DataInPort data[3:0]; DataInPort enable;
  DataRegister value[3:0] {
    WriteDataSource data; WriteEnSource enable; ResetValue 4'b0;
  }
  DataOutPort observed[3:0] { Source value; }
  DataOutPort inverted[3:0] { Source ~value; }
  Alias pair[4:0] = enable, value { AccessTogether; }
  Alias reads[7:0] = observed, inverted { AccessTogether; }
})");
    const auto netlist = pattern::Elaborate(design, "PrimaryGroups");
    pattern::ScanScheduler scheduler(netlist);
    scheduler.Reset();
    const auto first =
        scheduler.Apply({{"iWrite", {"pair[3:0]", "9"}, "group:1", {}}});
    Require(first.size() == 1 && first[0].together &&
                first[0].writes.at("data") == "1001" &&
                first[0].writes.at("enable") == "1" &&
                scheduler.Value("value") == "1001",
            "grouped primary drive updates actual data storage in one event");
    bool rejected = false;
    try {
      scheduler.Apply({{"iWrite", {"pair", "0x13"}, "group:2", {}}}, false,
                      {{netlist.node_ids.at("data"), "1001"}});
    } catch (const std::exception& error) {
      rejected =
          std::string(error.what()).find("reserved data") != std::string::npos;
    }
    Require(rejected && scheduler.Value("data") == "1001" &&
                scheduler.Value("value") == "1001" &&
                scheduler.Value("enable") == "1",
            "failed primary groups preserve driver and storage state");
    scheduler.Apply({{"iWrite", {"pair", "0x13"}, "group:3", {}}});
    const auto read =
        scheduler.Apply({{"iRead", {"reads", "0x3c"}, "group:4", {}}});
    Require(read.size() == 1 && read[0].writes.empty() &&
                read[0].expected.at("observed") == "0011" &&
                read[0].expected.at("inverted") == "1100" &&
                scheduler.Value("observed") == "0011" &&
                scheduler.Value("inverted") == "1100",
            "grouped output comparisons observe actual storage together");
  }
}
void TestMixedPortGoals() {
  pattern::Design design;
  design.AddIcl("port-goals.icl", R"(
Module Sink { DataInPort value[3:0]; }
Module PortGoals {
  ScanInPort si;
  ScanRegister sr[3:0] { ScanInSource si; ResetValue 4'b0; }
  ScanOutPort so { Source sr[0]; }
  DataInPort pin[3:0];
  DataOutPort out[7:0] { Source sr, pin; }
  Instance sink Of Sink { InputPort value = sr; }
})");
  const auto netlist = pattern::Elaborate(design, "PortGoals");
  pattern::ScanScheduler scheduler(netlist);
  scheduler.Reset();
  const auto unchanged =
      scheduler.Apply({{"iWrite", {"sink.value", "0"}, "port:1", {}}});
  Require(unchanged.size() == 1 && unchanged[0].writes.at("sr") == "0000",
          "unchanged scan-driven DataInPort still requires controller update");
  scheduler.Apply({{"iWrite", {"out", "0x59"}, "port:2", {}}});
  Require(scheduler.Value("sr") == "0101" && scheduler.Value("pin") == "1001" &&
              scheduler.Value("out") == "01011001",
          "logical port goals update actual serial and parallel sources");
  scheduler.Apply({{"iWrite", {"sr", "3"}, "port:3", {}}});
  scheduler.Apply({{"iWrite", {"out[3:0]", "6"}, "port:4", {}}});
  Require(scheduler.Value("sr") == "0101" && scheduler.Value("pin") == "0110" &&
              scheduler.Value("out") == "01010110",
          "logical controlling history survives physical source writes");
  bool rejected = false;
  try {
    scheduler.Apply({{"iWrite", {"out", "0x99"}, "port:5", {}}}, false,
                    {{netlist.node_ids.at("sr"), "0101"}});
  } catch (const std::exception& error) {
    rejected =
        std::string(error.what()).find("reserved sr") != std::string::npos;
  }
  Require(rejected && scheduler.Value("out") == "01010110",
          "failed port writes preserve physical state");
  scheduler.Apply({{"iWrite", {"out", "0bx"}, "port:6", {}}});
  Require(scheduler.Value("out") == "01010110",
          "failed port writes preserve logical controlling history");
}
void TestParallelControlReservations() {
  for (bool serial : {false, true}) {
    pattern::Design design;
    design.AddIcl("control-resources.icl", std::string(R"(
Module Child { ReadEnPort re; }
Module ControlResources {
  ReadEnPort re;
  Instance child Of Child { InputPort re = re; }
  Alias inverted = ~child.re;
)") +
                                               (serial ? R"(
  ScanInPort si;
  ScanRegister sr { ScanInSource si; ResetValue 1'b0; }
  ScanOutPort so { Source sr; }
)"
                                                       : "") +
                                               "}");
    const auto netlist = pattern::Elaborate(design, "ControlResources");
    pattern::ScanScheduler scheduler(netlist);
    scheduler.Reset();
    const size_t child = netlist.node_ids.at("child.re");
    scheduler.Apply({{"iWrite", {"child.re", "0"}, "control:1", {}}});
    scheduler.Apply({{"iWrite", {"inverted", "0"}, "control:2", {}}}, false,
                    {{child, "0"}});
    Require(scheduler.Value("child.re") == "1" && scheduler.Value("re") == "1",
            "explicit alias write replaces child ownership on actual source");
    bool rejected = false;
    try {
      scheduler.Apply({{"iWrite", {"re", "0"}, "control:3", {}}}, false,
                      {{child, "1"}});
    } catch (const std::exception&) {
      rejected = true;
    }
    Require(rejected && scheduler.Value("re") == "1",
            "failed implicit child change preserves physical state");
    rejected = false;
    try {
      scheduler.Apply({{"iWrite", {"child.re", "0"}, "control:4", {}}}, false,
                      {{child, "1"}}, false);
    } catch (const std::exception&) {
      rejected = true;
    }
    Require(rejected && scheduler.Value("child.re") == "1",
            "automatic writes cannot replace child ownership");
    scheduler.Apply({{"iWrite", {"child.re", "0"}, "control:5", {}}}, false,
                    {{child, "1"}});
    Require(scheduler.Value("re") == "0",
            "explicit replacement succeeds after failed planning");
  }
}
void TestResetDependencyScale() {
  // Reverse declaration order is the worst case for repeatedly scanning all
  // registers to discover the next reset dependency.
  pattern::Netlist netlist;
  constexpr size_t kRegisters = 4096;
  for (size_t id = 0; id < kRegisters; ++id) {
    pattern::NetlistNode node;
    node.name = "r" + std::to_string(id);
    node.kind = "ScanRegister";
    node.model = pattern::ModelKind::kRegister;
    if (id + 1 < kRegisters) node.reset = pattern::BitReference{id + 1, 0};
    netlist.node_ids.emplace(node.name, id);
    netlist.nodes.push_back(std::move(node));
  }
  const auto start = std::chrono::steady_clock::now();
  pattern::ScanScheduler scheduler(netlist);
  std::cout << "reset dependencies: registers=" << kRegisters << " seconds="
            << std::chrono::duration<double>(std::chrono::steady_clock::now() -
                                             start)
                   .count()
            << '\n';
  netlist.nodes.back().reset = pattern::BitReference{0, 0};
  bool rejected = false;
  try {
    pattern::ScanScheduler cyclic(netlist);
  } catch (const std::exception& error) {
    rejected =
        std::string(error.what()).find("feedback reset") != std::string::npos;
  }
  Require(rejected, "reset dependency cycle must not be silently ordered");
}
void TestGreedyRollback() {
  std::string source = R"(
Module GreedyLeaf {
 ScanInPort si; ResetPort rst;
 ScanRegister target[3:0] { ScanInSource si; ResetValue 4'b0; }
 ScanOutPort so { Source target[0]; }
}
Module GreedyRollback {
 ScanInPort si;
 ScanRegister release { ScanInSource si; ResetValue 1'b0; }
 Instance child Of GreedyLeaf { InputPort si = release; InputPort rst = ~release; }
 ScanOutPort so { Source child.so; }
)";
  for (size_t i = 0; i < 32; ++i) {
    source += "ScanRegister pad" + std::to_string(i) +
              " { ScanInSource si; ResetValue 1'b0; }\n";
  }
  pattern::Design design;
  design.AddIcl("greedy-rollback.icl", source + "}\n");
  const auto netlist = pattern::Elaborate(design, "GreedyRollback");
  pattern::ScanScheduler scheduler(netlist);
  scheduler.Reset();
  const std::map<size_t, std::string> held{{netlist.node_ids.at("pad0"), "0"}};
  bool rejected = false;
  try {
    scheduler.Apply({{"iWrite", {"child.target", "5"}, "rollback:1", {}},
                     {"iWrite", {"release", "0"}, "rollback:2", {}}},
                    false, held);
  } catch (const std::exception& error) {
    rejected =
        std::string(error.what()).find("no schedule") != std::string::npos;
  }
  Require(rejected && scheduler.Value("release") == "0" &&
              scheduler.Value("child.target") == "0000",
          "failed greedy final update must not publish prerequisite state");
  const auto steps = scheduler.Apply(
      {{"iWrite", {"child.target", "5"}, "rollback:3", {}}}, false, held);
  Require(
      steps.size() == 2 && scheduler.Value("release") == "1" &&
          scheduler.Value("child.target") == "0101",
      "greedy reset prerequisite precedes target update and preserves fill");
}
void TestAddressedRollback() {
  std::ifstream file(
      "tests/cases/ours/pattern_generation/scan_data_registers/"
      "addressed_eight.icl");
  const std::string source((std::istreambuf_iterator<char>(file)), {});
  pattern::Design design;
  design.AddIcl("addressed-rollback.icl", source);
  auto netlist = pattern::Elaborate(design, "ScanData");
  // The last address invalidates the first goal: per-batch success alone
  // cannot publish this speculative sequence.
  netlist.nodes[netlist.node_ids.at("bank.r0")].reset =
      pattern::BitReference{netlist.node_ids.at("bank.r3"), 0};
  pattern::ScanScheduler scheduler(netlist);
  scheduler.Reset();
  const auto initial = scheduler.Value("bank.r0");
  bool rejected = false;
  try {
    scheduler.Apply({{"iWrite", {"bank.r0", "1"}, "rollback:1", {}},
                     {"iWrite", {"bank.r1", "2"}, "rollback:2", {}},
                     {"iWrite", {"bank.r2", "3"}, "rollback:3", {}},
                     {"iWrite", {"bank.r3", "1"}, "rollback:4", {}}});
  } catch (const std::exception&) {
    rejected = true;
  }
  Require(rejected && scheduler.Value("bus") == "00000000" &&
              scheduler.Value("bank.r0") == initial &&
              scheduler.Value("bank.r1") == "0000" &&
              scheduler.Value("bank.r2") == "0000" &&
              scheduler.Value("bank.r3") == "0000",
          "failed address ordering must not publish intermediate state");
  const auto steps =
      scheduler.Apply({{"iWrite", {"bank.r0", "1"}, "rollback:5", {}}});
  Require(steps.size() == 1 && scheduler.Value("bank.r0") == "0001",
          "failed addressed speculation must preserve controlling fill");
}
void TestPrimaryWrites() {
  pattern::Design design;
  design.AddIcl("primary.icl", R"(
Module Primary {
 DataInPort data[3:0]; DataInPort unused;
 DataOutPort out[3:0] { Source data; }
 DataOutPort inverted[3:0] { Source ~data; }
})");
  const auto netlist = pattern::Elaborate(design, "Primary");
  pattern::ScanScheduler scheduler(netlist);
  const auto first =
      scheduler.Apply({{"iWrite", {"data[0]", "1"}, "primary:1", {}}});
  Require(first.size() == 1 && first[0].writes.at("data") == "0001",
          "partial primary write fills unspecified initial bits");
  Require(scheduler.Value("unused") == "x",
          "unrelated primary input remains undriven");
  scheduler.Assume("data", "1010");
  scheduler.Apply({{"iWrite", {"data", "0bxx01"}, "primary:2", {}},
                   {"iWrite", {"data[1]", "1"}, "primary:3", {}}},
                  true);
  Require(scheduler.Value("out") == "1011",
          "masked primary values and last write wins");
  scheduler.Apply({{"iWrite", {"out", "0b1011"}, "primary:3a", {}}});
  const auto selected =
      scheduler.Apply({{"iWrite", {"out[2]", "1"}, "primary:4", {}}});
  Require(selected.size() == 1 && scheduler.Value("out") == "1111",
          "forwarded port writes preserve other known bits");
  bool rejected = false;
  try {
    scheduler.Apply({{"iWrite", {"out", "0"}, "primary:5", {}},
                     {"iWrite", {"inverted", "0"}, "primary:6", {}}});
  } catch (const std::exception& error) {
    rejected =
        std::string(error.what()).find("cannot justify") != std::string::npos;
  }
  Require(rejected && scheduler.Value("out") == "1111",
          "conflicting parallel targets fail without changing state");
}
void TestReservationRollback() {
  std::ifstream file(
      "tests/cases/ours/pattern_generation/resources/network.icl");
  const std::string source(std::istreambuf_iterator<char>(file), {});
  for (size_t padding : {0U, 32U}) {
    auto expanded = source;
    for (size_t i = 0; i < padding; ++i) {
      expanded.insert(expanded.rfind('}'),
                      "ScanRegister pad" + std::to_string(i) +
                          " { ScanInSource si; ResetValue 1'b0; }\n");
    }
    pattern::Design design;
    design.AddIcl("resource.icl", expanded);
    const auto netlist = pattern::Elaborate(design, "ResourceNetwork");
    pattern::ScanScheduler scheduler(netlist);
    scheduler.Reset();
    const std::map<size_t, std::string> held{
        {netlist.node_ids.at("gate"), "00"}};
    for (const auto& operations : std::vector<std::vector<pattern::Operation>>{
             {{"iWrite", {"target", "5"}, "resource:1", {}}},
             {{"iWrite", {"gate", "1"}, "resource:2", {}}}}) {
      bool rejected = false;
      try {
        scheduler.Apply(operations, false, held, false);
      } catch (const std::exception& error) {
        rejected = std::string(error.what()).find("reserved gate") !=
                   std::string::npos;
      }
      Require(
          rejected && scheduler.Value("gate") == "00" &&
              scheduler.Value("target") == "0000",
          "reserved-control failures must leave hardware and fill unchanged");
    }
    scheduler.Apply({{"iWrite", {"gate", "1"}, "resource:3", {}},
                     {"iWrite", {"target", "5"}, "resource:4", {}}},
                    false, held);
    Require(
        scheduler.Value("gate") == "01" && scheduler.Value("target") == "0101",
        "explicit source update replaces a reservation after failed trials");
  }
}
void TestMaskedReservationRollback() {
  std::ifstream file(
      "tests/cases/ours/pattern_generation/resources/network.icl");
  const std::string source(std::istreambuf_iterator<char>(file), {});
  for (size_t padding : {0U, 32U}) {
    auto expanded = source;
    for (size_t i = 0; i < padding; ++i) {
      expanded.insert(expanded.rfind('}'),
                      "ScanRegister pad" + std::to_string(i) +
                          " { ScanInSource si; ResetValue 1'b0; }\n");
    }
    pattern::Design design;
    design.AddIcl("masked-resource.icl", expanded);
    const auto netlist = pattern::Elaborate(design, "ResourceNetwork");
    pattern::ScanScheduler scheduler(netlist);
    scheduler.Reset();
    const size_t gate = netlist.node_ids.at("gate");
    bool rejected = false;
    try {
      scheduler.Apply({{"iWrite", {"target", "5"}, "masked:1", {}}}, false,
                      {{gate, "x0"}});
    } catch (const std::exception& error) {
      rejected =
          std::string(error.what()).find("reserved gate") != std::string::npos;
    }
    Require(rejected && scheduler.Value("gate") == "00" &&
                scheduler.Value("target") == "0000",
            "masked reservation failure must preserve hardware state");
    scheduler.Apply({{"iWrite", {"target", "5"}, "masked:2", {}}}, false,
                    {{gate, "0x"}});
    Require(scheduler.Value("gate").at(0) == '0' &&
                scheduler.Value("target") == "0101",
            "unprotected selector can change after failed masked trial");
    scheduler.Apply({{"iWrite", {"gate[1]", "1"}, "masked:3", {}}}, false,
                    {{gate, "0x"}});
    Require(scheduler.Value("gate").at(0) == '1',
            "explicit protected bit update must remain permitted");
  }
}
void TestPrimaryControllingState() {
  pattern::Design design;
  design.AddIcl("primary-control.icl", R"(
Module Control {
 DataInPort data;
 DataInPort enable;
 DataOutPort out { Source data; Enable enable; }
 Alias paired[1:0] = data, enable { AccessTogether; }
}
)");
  const auto netlist = pattern::Elaborate(design, "Control");
  pattern::ScanScheduler scheduler(netlist);
  const auto first =
      scheduler.Apply({{"iWrite", {"out", "0bX"}, "control:1", {}}});
  Require(first.size() == 1 && scheduler.Value("out") == "0" &&
              first[0].writes.at("data") == "0" &&
              first[0].writes.at("enable") == "1",
          "unknown hardware is not the initial output controlling state");
  scheduler.Apply({{"iWrite", {"out", "1"}, "control:2", {}}});
  scheduler.Apply({{"iWrite", {"data", "0"}, "control:3", {}}});
  scheduler.Apply({{"iWrite", {"out", "0bX"}, "control:4", {}}});
  Require(scheduler.Value("out") == "1",
          "output controlling state survives writes to upstream pins");
  const auto grouped =
      scheduler.Apply({{"iWrite", {"paired[1]", "0"}, "control:5", {}}});
  Require(grouped.size() == 1 && grouped[0].together &&
              grouped[0].writes.at("data") == "0" &&
              grouped[0].writes.at("enable") == "1" &&
              scheduler.Value("out") == "0",
          "partial alias writes access the whole primary group in one event");
}
void TestDataReservationRollback() {
  for (bool serial : {false, true}) {
    const std::string family =
        serial ? "scan_data_registers" : "data_registers";
    std::ifstream file("tests/cases/ours/pattern_generation/" + family +
                       "/selectable.icl");
    pattern::Design design;
    design.AddIcl("data-resource.icl",
                  std::string(std::istreambuf_iterator<char>(file), {}));
    const auto netlist =
        pattern::Elaborate(design, serial ? "ScanData" : "SelectableData");
    pattern::ScanScheduler scheduler(netlist);
    scheduler.Reset();
    scheduler.Apply({{"iWrite", {"value", "5"}, "data-resource:1", {}}});
    const auto driver = serial ? "bus" : "data";
    const auto before = scheduler.Value(driver);
    const std::map<size_t, std::string> held{
        {netlist.node_ids.at("value"), "0101"}};
    std::vector<pattern::Operation> indirect{
        {"iWrite", {driver, serial ? "25" : "9"}, "data-resource:2", {}}};
    if (!serial)
      indirect.push_back({"iWrite", {"enable", "1"}, "data-resource:3", {}});
    const std::vector<pattern::Operation> replacement{
        {"iWrite", {"value", "9"}, "data-resource:4", {}}};
    for (bool automatic : {false, true}) {
      bool rejected = false;
      try {
        scheduler.Apply(automatic ? replacement : indirect, false, held,
                        !automatic);
      } catch (const std::exception& error) {
        rejected = std::string(error.what()).find("reserved value") !=
                   std::string::npos;
      }
      Require(rejected && scheduler.Value("value") == "0101" &&
                  scheduler.Value(driver) == before,
              "data resource failure must preserve hardware and driver state");
    }
    scheduler.Apply({{"iWrite", {"value", "9"}, "data-resource:5", {}}}, false,
                    held);
    Require(scheduler.Value("value") == "1001",
            "explicit data replacement works after failed preservation trials");
  }
}
void TestPrimaryLimits() {
  pattern::Design design;
  design.AddIcl("wide-primary.icl", R"(
Module Wide {
 DataInPort data[1023:0];
 DataOutPort out[1023:0] { Source data; }
}
Module TooWide { DataInPort data[4096:0]; }
)");
  const auto netlist = pattern::Elaborate(design, "Wide");
  pattern::ScanScheduler scheduler(netlist);
  const auto start = std::chrono::steady_clock::now();
  const std::string bits(1024, '1');
  const auto steps =
      scheduler.Apply({{"iWrite", {"out", "0b" + bits}, "wide:1", {}}});
  Require(steps.size() == 1 && steps[0].writes.at("data") == bits,
          "wide primary-input replay");
  std::cout << "primary write: bits=1024 seconds="
            << std::chrono::duration<double>(std::chrono::steady_clock::now() -
                                             start)
                   .count()
            << '\n';
  const auto large = pattern::Elaborate(design, "TooWide");
  pattern::ScanScheduler limited(large);
  bool rejected = false;
  try {
    limited.Apply({{"iWrite", {"data", "1"}, "wide:2", {}}});
  } catch (const std::exception& error) {
    rejected =
        std::string(error.what()).find("resource limit") != std::string::npos;
  }
  Require(rejected && limited.Value("data") == std::string(4097, 'x'),
          "oversized primary input fails atomically");
}
void TestHiddenScanState() {
  const auto read = [](const std::string& name) {
    std::ifstream file("tests/cases/ours/pattern_generation/" + name);
    return std::string(std::istreambuf_iterator<char>(file), {});
  };
  for (size_t width : {4U, 32U}) {
    for (bool capture : {false, true}) {
      auto source = read("resources/slices.icl");
      if (capture) {
        for (const auto& [reset, property] : std::map<std::string, std::string>{
                 {"ResetValue 2'b00;", "CaptureSource 2'b11; "},
                 {"ResetValue 4'b0;", "CaptureSource 4'b1010; "}}) {
          source.insert(source.find(reset), property);
        }
      }
      source.replace(source.find("target[3:0]"),
                     std::string("target[3:0]").size(),
                     "target[" + std::to_string(width - 1) + ":0]");
      if (width != 4) {
        size_t offset = 0;
        while ((offset = source.find("4'b", offset)) != std::string::npos) {
          source.replace(offset, 3, std::to_string(width) + "'b");
          offset += std::to_string(width).size() + 2;
        }
      }
      pattern::Design design;
      design.AddIcl("hidden.icl", source);
      design.AddBsdl("simple.bsdl", read("access_link/simple.bsdl"),
                     "AccessProbe");
      auto network = pattern::ElaborateTapNetwork(design, "SliceTop");
      pattern::ScanScheduler scheduler(network.netlist);
      scheduler.Reset();
      const auto steps =
          scheduler.Apply({{"iWrite", {"core.gate", "2"}, "hidden:1", {}}});
      Require(steps.size() == 2 && scheduler.Value("core.gate") == "10",
              "hidden event scheduling selects IR then updates visible gate");
      Require(scheduler.Value("core.target") ==
                  (capture ? "11" + std::string(width - 4, '0') + "10"
                           : std::string(width, 'x')),
              "two bypass clocks shift captured gate bits into hidden target");
      scheduler.Assume("core.target", std::string(width, '1'));
      bool rejected = false;
      try {
        scheduler.Apply({{"iWrite", {"core.gate", "0"}, "hidden:2", {}}}, false,
                        {{network.netlist.node_ids.at("core.target"),
                          std::string(width, '1')}});
      } catch (const std::exception&) {
        rejected = true;
      }
      Require(rejected &&
                  scheduler.Value("core.target") == std::string(width, '1') &&
                  scheduler.Value("core.gate") == "10",
              "hidden protected writes fail without changing live state");
      scheduler.Reset();
      Require(
          scheduler.Value("core.target") == std::string(width, '0') &&
              scheduler.Value("core.gate") == "00",
          "TAP reset reaches bypassed shadow storage after failed planning");
    }
  }
}
void TestTapScheduling(bool synchronous, bool no_trst, bool implicit) {
  const std::string directory =
      "tests/cases/ours/pattern_generation/access_link/";
  const auto read = [&](const std::string& name) {
    std::ifstream file(directory + name);
    return std::string(std::istreambuf_iterator<char>(file), {});
  };
  pattern::Design design;
  design.AddIcl("simple.icl", read("simple.icl"));
  auto bsdl = read("simple.bsdl");
  if (no_trst) {
    const auto start = bsdl.find("  attribute TAP_SCAN_RESET");
    bsdl.erase(start, bsdl.find('\n', start) + 1 - start);
  }
  design.AddBsdl("simple.bsdl", bsdl, "AccessProbe");
  auto network = pattern::ElaborateTapNetwork(design, "AccessTop");
  if (implicit) {
    for (auto& node : network.netlist.nodes) {
      if (node.scan_interface &&
          network.netlist.scopes[node.scope].path == "core") {
        node.scan_interface.reset();
      }
    }
  }
  pattern::ScanScheduler scheduler(network.netlist);
  Require(scheduler.Value("core.r") == "xxxxxxxx", "no implicit reset");
  bool rejected = false;
  try {
    scheduler.Apply({{"iWrite", {"core.r", "0x59"}, "tap-test:0", {}}});
  } catch (const std::exception& error) {
    rejected =
        std::string(error.what()).find("explicit iReset") != std::string::npos;
  }
  Require(rejected, "unknown controller state must require explicit reset");
  scheduler.Reset(synchronous);
  const auto ir = network.netlist.taps.begin()->second.instruction_register;
  rejected = false;
  try {
    scheduler.Assume(network.netlist.nodes[ir].name,
                     std::string(network.netlist.nodes[ir].Width(), '1'));
  } catch (const std::exception& error) {
    rejected =
        std::string(error.what()).find("controller-owned") != std::string::npos;
  }
  Require(rejected, "iState cannot replace controller-owned instruction state");
  const auto steps =
      scheduler.Apply({{"iWrite", {"core.r", "0x59"}, "tap-test:1", {}}});
  Require(steps.size() == 2, "IR selection must precede DR write");
  Require(steps[0].serial == "1100" && steps[1].serial == "10011010",
          "hand-derived IJTAG_ON opcode and DR serial data");
  Require(scheduler.Value("core.r") == "01011001",
          "native TAP replay writes target");
  scheduler.Idle(std::numeric_limits<uint64_t>::max());
  Require(scheduler.Value("core.r") == "01011001",
          "idle keeps stable instrument state");
  Require(scheduler.Apply({{"iWrite", {"core.r", "0xa6"}, "tap-test:2", {}}})
                  .size() == 1,
          "reuse active instruction without redundant IR scan");
  Require(scheduler.Value("core.r") == "10100110", "second DR transaction");
  scheduler.Assume("core.r", "00000011");
  const auto read_steps =
      scheduler.Apply({{"iRead", {"core.r", "3"}, "tap-test:3", {}}});
  Require(read_steps.size() == 1 && read_steps[0].serial == "11000000" &&
              read_steps[0].serial_expected == "11000000",
          "assumed fill and expected serial data use left/right bit order");
  const auto previous = scheduler.Value("core.r");
  rejected = false;
  try {
    scheduler.Apply({{"iWrite", {"absent", "1"}, "tap-test:4", {}}});
  } catch (const std::exception&) {
    rejected = true;
  }
  Require(rejected && scheduler.Value("core.r") == previous,
          "failed scheduling preserves committed hardware state");
}
void TestChangingIdleState() {
  const std::string directory =
      "tests/cases/ours/pattern_generation/access_link/";
  const auto read = [&](const std::string& name) {
    std::ifstream file(directory + name);
    return std::string(std::istreambuf_iterator<char>(file), {});
  };
  for (bool periodic : {false, true}) {
    auto source = read("simple.icl");
    source.replace(source.find("r[7:0]"), 6, "r[2:0]");
    source.insert(
        source.find("ScanInSource si;") +
            std::string("ScanInSource si;").size(),
        periodic ? " CaptureSource r[1:0], r[2];" : " CaptureSource 3'b000;");
    source.insert(source.find("InputPort mode"),
                  "InputPort select = 1'b1; InputPort capture = 1'b1; "
                  "InputPort shift = 1'b0; InputPort update = 1'b1; ");
    pattern::Design design;
    design.AddIcl("idle.icl", source);
    design.AddBsdl("simple.bsdl", read("simple.bsdl"), "AccessProbe");
    const auto network = pattern::ElaborateTapNetwork(design, "AccessTop");
    pattern::ScanScheduler scheduler(network.netlist);
    scheduler.Idle(0);
    Require(scheduler.Value("core.r") == "xxx",
            "zero cycles must not initialize hardware");
    bool rejected = false;
    try {
      scheduler.Idle(1);
    } catch (const std::exception& error) {
      rejected = std::string(error.what()).find("explicit iReset") !=
                 std::string::npos;
    }
    Require(rejected,
            "nonzero idle cycles require established controller state");
    scheduler.Reset();
    scheduler.Assume("core.r", "100");
    rejected = false;
    try {
      scheduler.Idle(1, {{network.netlist.node_ids.at("core.r"), "100"}});
    } catch (const std::exception& error) {
      rejected = std::string(error.what()).find("reserved resource core.r") !=
                 std::string::npos;
    }
    Require(rejected && scheduler.Value("core.r") == "100",
            "reserved idle failure must not publish transient hardware state");
    if (periodic) {
      scheduler.Idle(1);
      Require(scheduler.Value("core.r") == "001",
              "idle capture/update follows actual pin controls");
      scheduler.Idle(2);
      Require(scheduler.Value("core.r") == "100",
              "changing idle state is not skipped as stable");
      rejected = false;
      try {
        scheduler.Idle(std::numeric_limits<uint64_t>::max());
      } catch (const std::exception& error) {
        rejected = std::string(error.what()).find("resource limit") !=
                   std::string::npos;
      }
      Require(rejected && scheduler.Value("core.r") == "100",
              "bounded nonconverging replay must fail without partial state "
              "changes");
      scheduler.Idle(1);
      Require(scheduler.Value("core.r") == "001",
              "failed idle replay also preserves executor state");
    } else {
      scheduler.Idle(std::numeric_limits<uint64_t>::max());
      Require(scheduler.Value("core.r") == "000",
              "idle must replay changes before skipping a fixed point");
    }
  }
}
void TestTopPortAssumptions() {
  pattern::Design design;
  design.AddIcl("port-state.icl", R"(
Module Top {
  ScanInPort si;
  DataInPort mode[1:0];
  DataInPort wide[65535:0];
  ScanRegister r[3:0] { ScanInSource si; }
  ScanMux route SelectedBy mode { 2'b01: r[0]; 2'b10: r[0]; 2'b00: si; }
  ScanOutPort so { Source route; }
  DataOutPort observed { Source r[0]; }
  Alias mixed[2:0] = ~mode, r[3];
  Alias invalid[2:0] = mode, observed;
})");
  const auto netlist = pattern::Elaborate(design, "Top");
  pattern::ScanScheduler scheduler(netlist);
  scheduler.Assume("mode", "01");
  scheduler.Assume("r", "0000");
  scheduler.Assume("mixed", "001");
  Require(scheduler.Value("mode") == "11" && scheduler.Value("r") == "1000",
          "mixed pin/register alias assumptions preserve inversion");
  scheduler.Assume("mode[0]", "0");
  Require(scheduler.Value("mode") == "10", "pin slices preserve other bits");
  bool rejected = false;
  try {
    scheduler.Assume("invalid", "000");
  } catch (const std::exception&) {
    rejected = true;
  }
  Require(rejected && scheduler.Value("mode") == "10",
          "invalid state-point alias must not partially change valid pins");
  scheduler.Assume("mode", "x1");
  Require(scheduler.Value("mode") == "x1", "pin assumptions retain unknowns");
  scheduler.Assume("mode", "01");
  const std::vector<pattern::Operation> operations{
      {"iWrite", {"r", "3"}, "port-state.pdl", {}}};
  const auto steps = scheduler.Apply(operations);
  Require(steps.size() == 1 && scheduler.Value("r") == "0011",
          "serial scheduling uses the assumed pin state");
  const auto start = std::chrono::steady_clock::now();
  scheduler.Assume("wide", std::string(65536, '1'));
  const auto seconds =
      std::chrono::duration<double>(std::chrono::steady_clock::now() - start)
          .count();
  Require(scheduler.Value("wide[65535]") == "1" &&
              scheduler.Value("wide[0]") == "1",
          "wide pin assumptions retain both ends of the vector");
  std::cout << "pin assumption: bits=65536 seconds=" << seconds << '\n';
}
void TestTapExternalReset() {
  const std::string directory =
      "tests/cases/ours/pattern_generation/access_link/";
  const auto read = [&](const std::string& name) {
    std::ifstream file(directory + name);
    return std::string(std::istreambuf_iterator<char>(file), {});
  };
  auto source = read("simple.icl");
  source.insert(source.find("Instance core"), "ResetPort rst; ");
  source.insert(
      source.find("ScanInSource si;") + std::string("ScanInSource si;").size(),
      " ResetValue 8'b00000000;");
  pattern::Design design;
  design.AddIcl("external-reset.icl", source);
  design.AddBsdl("simple.bsdl", read("simple.bsdl"), "AccessProbe");
  const auto network = pattern::ElaborateTapNetwork(design, "AccessTop");
  for (bool synchronous : {false, true}) {
    pattern::ScanScheduler scheduler(network.netlist);
    scheduler.Reset(synchronous);
    Require(scheduler.Value("core.r") == "00000000",
            "iReset must toggle top-level ResetPort as well as the TAP");
    scheduler.Apply({{"iWrite", {"core.r[0]", "1"}, "reset:1", {}}});
    Require(scheduler.Value("core.r") == "00000001",
            "post-reset fill must use reset data");
  }
}
void TestTapControlRejection() {
  const std::string directory =
      "tests/cases/ours/pattern_generation/access_link/";
  const auto read = [&](const std::string& name) {
    std::ifstream file(directory + name);
    return std::string(std::istreambuf_iterator<char>(file), {});
  };
  for (const auto* port : {"clock", "shift", "update", "select"}) {
    auto source = read("simple.icl");
    source.insert(source.find("InputPort mode"),
                  "InputPort " + std::string(port) + " = 1'b0; ");
    pattern::Design design;
    design.AddIcl("disabled.icl", source);
    design.AddBsdl("simple.bsdl", read("simple.bsdl"), "AccessProbe");
    const auto network = pattern::ElaborateTapNetwork(design, "AccessTop");
    pattern::ScanScheduler scheduler(network.netlist);
    scheduler.Reset();
    bool rejected = false;
    try {
      scheduler.Apply(
          {{"iWrite", {"core.r", "0x59"}, "disabled:1", {"iCall write"}}});
    } catch (const std::exception& error) {
      const std::string message = error.what();
      rejected = message.find("disabled:1") != std::string::npos &&
                 message.find("iCall write") != std::string::npos;
    }
    Require(
        rejected && scheduler.Value("core.r") == "xxxxxxxx",
        "disabled interface cannot produce a successful scan or change state");
  }
}
void TestApplyTogether() {
  pattern::Design design;
  design.AddIcl("exclusive.icl", R"(
Module Network {
 ScanInPort si;
 ScanRegister gate { ScanInSource si; }
 ScanRegister a[3:0] { ScanInSource gate; }
 ScanRegister b[3:0] { ScanInSource gate; }
 ScanMux route SelectedBy gate { 1'b0: a[0]; 1'b1: b[0]; }
 ScanOutPort so { Source route; }
})");
  const auto netlist = pattern::Elaborate(design, "Network");
  const std::vector<pattern::Operation> writes{
      {"iWrite", {"a", "3"}, "exclusive:1", {}},
      {"iWrite", {"b", "5"}, "exclusive:2", {}}};
  pattern::ScanScheduler serial(netlist);
  serial.Assume("gate", "0");
  Require(serial.Apply(writes).size() == 2,
          "mutually exclusive paths remain legal without -together");
  pattern::ScanScheduler together(netlist);
  together.Assume("gate", "0");
  together.Assume("a", "0000");
  together.Assume("b", "0000");
  bool rejected = false;
  try {
    together.Apply(writes, true);
  } catch (const std::exception& error) {
    const std::string message = error.what();
    rejected = message.find("iApply -together") != std::string::npos &&
               message.find("exclusive:2") != std::string::npos;
  }
  Require(
      rejected && together.Value("a") == "0000" &&
          together.Value("b") == "0000" && together.Value("gate") == "0",
      "different update cycles cannot satisfy -together; failure is atomic");
  const auto reads = together.Apply({{"iRead", {"a", "0"}, "read:1", {}},
                                     {"iRead", {"b", "0"}, "read:2", {}}},
                                    true);
  Require(reads.size() == 2,
          "IEEE 7.9.14(c) constrains writes, not simultaneous reads");

  pattern::Design reset_design;
  reset_design.AddIcl("reset.icl", R"(
Module Child {
 ScanInPort si; ResetPort rst;
 ScanRegister r { ScanInSource si; ResetValue 1'b0; }
 ScanOutPort so { Source r; }
}
Module Network {
 ScanInPort si;
 ScanRegister release { ScanInSource si; }
 Instance child Of Child { InputPort si = release; InputPort rst = ~release; }
 ScanOutPort so { Source child.so; }
})");
  const auto reset_netlist = pattern::Elaborate(reset_design, "Network");
  pattern::ScanScheduler reset(reset_netlist);
  reset.Assume("release", "0");
  const auto scans = reset.Apply({{"iWrite", {"release", "1"}, "reset:1", {}},
                                  {"iWrite", {"child.r", "0"}, "reset:2", {}}},
                                 true);
  Require(scans.size() == 2 && !scans[0].together && scans[1].together,
          "matching reset data is not an update; release must precede shared "
          "update");
  Require(scans[1].writes.at("release") == "1" &&
              scans[1].writes.at("child.r") == "0",
          "both requested writes appear on the verified common update edge");
}
void TestAccessTogether() {
  pattern::Design design;
  design.AddIcl("access-together.icl", R"(
Module Network {
 ScanInPort si;
 ScanRegister gate { ScanInSource si; }
 ScanRegister a { ScanInSource gate; }
 ScanRegister b { ScanInSource gate; }
 ScanMux route SelectedBy gate { 1'b0: a; 1'b1: b; }
 ScanOutPort so { Source route; }
 Alias pair[1:0] = a, b { AccessTogether; }
 Alias nested = pair[1];
 Alias outer[1:0] = gate, pair[1] { AccessTogether; }
})");
  const auto netlist = pattern::Elaborate(design, "Network");
  for (const auto& command : {"iWrite", "iRead"}) {
    for (const auto& target : {"pair", "pair[1]", "nested", "outer[1]"}) {
      pattern::ScanScheduler scheduler(netlist);
      scheduler.Assume("gate", "0");
      scheduler.Assume("a", "0");
      scheduler.Assume("b", "0");
      bool rejected = false;
      try {
        scheduler.Apply({{command, {target, "1"}, "group.pdl:7", {}}});
      } catch (const std::exception& error) {
        const std::string message = error.what();
        rejected = message.find("AccessTogether") != std::string::npos &&
                   message.find(std::string(target) == "outer[1]"
                                    ? "outer"
                                    : "pair") != std::string::npos &&
                   message.find("group.pdl:7") != std::string::npos;
      }
      Require(rejected && scheduler.Value("gate") == "0" &&
                  scheduler.Value("a") == "0" && scheduler.Value("b") == "0",
              "AccessTogether cannot span exclusive paths, even via a slice; "
              "failure must preserve state and identify the alias");
    }
  }
}
void TestAccessTogetherPrerequisites() {
  pattern::Design design;
  design.AddIcl("access-prerequisites.icl", R"(
Module Network {
 ScanInPort si;
 ScanRegister gate { ScanInSource si; }
 ScanRegister a { ScanInSource gate; }
 ScanRegister b { ScanInSource a; }
 ScanMux route SelectedBy gate { 1'b0: a; 1'b1: b; }
 ScanOutPort so { Source route; }
 DataOutPort output { Source a; }
 Alias pair[1:0] = a, ~b { AccessTogether; }
 Alias nested = ~pair[1];
 Alias invalid[1:0] = a, output { AccessTogether; }
})");
  const auto netlist = pattern::Elaborate(design, "Network");
  for (const auto& command : {"iWrite", "iRead"}) {
    for (const auto& target : {"pair", "pair[1]", "nested"}) {
      pattern::ScanScheduler scheduler(netlist);
      scheduler.Assume("gate", "0");
      const auto scans =
          scheduler.Apply({{command, {target, "1"}, "prerequisite:1", {}}});
      Require(
          scans.size() == 2 && scans.back().loads.count("a") &&
              scans.back().loads.count("b"),
          "full alias must participate even when only a slice is requested");
      if (std::string(command) == "iRead") {
        Require(
            scans.front().expected.empty() && scans.back().expected.count("a"),
            "first requested reads must wait for the entire access group");
      } else {
        Require(
            scans.back().together &&
                scans.back().writes.at("a") ==
                    (std::string(target) == "pair" ||
                             std::string(target) == "nested"
                         ? "0"
                         : "1"),
            "alias inversion and partial writes survive simultaneous update");
      }
    }
  }
  pattern::ScanScheduler scheduler(netlist);
  scheduler.Assume("gate", "0");
  const auto forwarded =
      scheduler.Apply({{"iWrite", {"invalid[1]", "1"}, "forwarded:1", {}}});
  Require(forwarded.size() == 1 && forwarded[0].together &&
              scheduler.Value("a") == "1" && scheduler.Value("output") == "1",
          "partial groups include forwarded outputs in their update");

  pattern::Design reset_design;
  reset_design.AddIcl("reset-group.icl", R"(
Module Child {
 ScanInPort si; ResetPort rst;
 ScanRegister r { ScanInSource si; ResetValue 1'b0; }
 ScanOutPort so { Source r; }
}
Module Network {
 ScanInPort si;
 ScanRegister release { ScanInSource si; }
 Instance child Of Child { InputPort si = release; InputPort rst = ~release; }
 ScanOutPort so { Source child.so; }
 Alias pair[1:0] = release, child.r { AccessTogether; }
})");
  const auto reset_netlist = pattern::Elaborate(reset_design, "Network");
  pattern::ScanScheduler reset(reset_netlist);
  reset.Assume("release", "0");
  const auto scans =
      reset.Apply({{"iWrite", {"pair[1]", "1"}, "reset-group:1", {}}});
  Require(scans.size() == 2 && !scans.front().together &&
              scans.back().together && scans.back().writes.at("child.r") == "0",
          "even an unrequested alias member must update after reset release");
}
void TestEndState() {
  std::ifstream file("tests/cases/ours/pattern_generation/apply_end_state.icl");
  const std::string source(std::istreambuf_iterator<char>(file), {});
  pattern::Design design;
  design.AddIcl("end-state.icl", source);
  const auto netlist = pattern::Elaborate(design, "EndState");
  pattern::Design flat_design;
  flat_design.AddIcl("flat.icl", pattern::SerializeIcl(netlist));
  const auto flat = pattern::Elaborate(flat_design, "EndState");
  for (const auto* candidate : {&netlist, &flat}) {
    pattern::ScanScheduler scheduler(*candidate);
    scheduler.Reset();
    auto steps =
        scheduler.Apply({{"iWrite", {"idle_trigger", "1"}, "test", {}}});
    Require(steps.size() == 1 && scheduler.Value("trigger") == "1",
            "an explicit end-state override lasts through its apply");
    bool rejected = false;
    try {
      scheduler.Apply({}, false, {{candidate->node_ids.at("trigger"), "1"}});
    } catch (const std::exception&) {
      rejected = true;
    }
    Require(rejected && scheduler.Value("trigger") == "1",
            "failed automatic restoration must preserve reserved state");
    steps = scheduler.Apply({});
    Require(steps.size() == 1 && steps[0].writes.at("trigger") == "0" &&
                scheduler.Value("trigger") == "0" &&
                scheduler.Apply({}).empty(),
            "source and flat end states restore once after the override");
  }
}
void TestPrerequisites() {
  pattern::Design design;
  design.AddIcl("scheduler.icl", R"(
Module Leaf {
  ScanInPort si;
  ResetPort rst;
  ScanRegister target[3:0] { ScanInSource si; ResetValue 4'b0000; }
  ScanOutPort so { Source target[0]; }
}
Module Network {
  ScanInPort si;
  ScanRegister gate { ScanInSource si; }
  ScanRegister reset { ScanInSource gate; }
  Instance child Of Leaf { InputPort si = reset; InputPort rst = ~reset; }
  ScanMux route SelectedBy gate { 1'b0: reset; 1'b1: child.so; }
  ScanOutPort so { Source route; }
})");
  const auto netlist = pattern::Elaborate(design, "Network");
  pattern::ScanScheduler scheduler(netlist);
  scheduler.Assume("gate", "0");
  scheduler.Assume("reset", "0");
  const auto steps =
      scheduler.Apply({{"iWrite", {"child.target", "0xa"}, "test:1", {}}});
  Require(steps.size() == 2, "gate opening and reset release share first scan");
  Require(steps[0].loads.at("gate") == "1" && steps[0].loads.at("reset") == "1",
          "prerequisites must precede target update");
  Require(steps[1].loads.at("child.target") == "1010" &&
              scheduler.Value("child.target") == "1010",
          "child.target receives requested value");
  // A repeated write is still an observable apply event.
  Require(!scheduler.Apply({{"iWrite", {"child.target", "10"}, "test:2", {}}})
               .empty(),
          "repeat write must not be optimized away");
  const auto partial =
      scheduler.Apply({{"iWrite", {"child.target[2:1]", "0b01"}, "test:3", {}},
                       {"iWrite", {"child.target[1]", "0b0"}, "test:4", {}}});
  Require(partial.back().loads.at("child.target") == "1000",
          "last write wins and untouched bits retain controlling state");
  pattern::Design flat_design;
  flat_design.AddIcl("flat.icl", pattern::SerializeIcl(netlist));
  const auto flat = pattern::Elaborate(flat_design, "Network");
  pattern::ScanScheduler replay(flat);
  replay.Assume("gate", "0");
  replay.Assume("reset", "0");
  const auto repeated =
      replay.Apply({{"iWrite", {"child.target", "0xa"}, "test:1", {}}});
  Require(repeated.size() == steps.size() &&
              repeated[0].loads == steps[0].loads &&
              repeated[1].loads == steps[1].loads,
          "flat netlist schedules the same prerequisite sequence");
}
void TestUnknownAndDisconnected() {
  pattern::Design design;
  design.AddIcl("disconnected.icl", R"(
Module Network {
  ScanInPort si;
  ScanRegister target { ScanInSource 1'b0; }
  ScanOutPort so { Source target; }
})");
  const auto netlist = pattern::Elaborate(design, "Network");
  pattern::ScanScheduler scheduler(netlist);
  bool rejected = false;
  try {
    scheduler.Apply(
        {{"iWrite", {"target", "1"}, "source.pdl:5", {"iCall main"}}});
  } catch (const std::exception& error) {
    const std::string message = error.what();
    rejected = message.find("source.pdl:5") != std::string::npos &&
               message.find("iCall main") != std::string::npos &&
               message.find("disconnected") != std::string::npos;
  }
  Require(rejected, "SO reachability without controllable SI cannot pass");
}
void TestCloseSelectOnTargetUpdate() {
  const auto read = [](const std::string& path) {
    std::ifstream input("tests/cases/ours/pattern_generation/" + path);
    Require(input.good(), "missing select-close fixture");
    return std::string(std::istreambuf_iterator<char>(input), {});
  };
  pattern::Design design;
  design.AddIcl("vendor.icl", read("resources/vendor.icl"));
  design.AddBsdl("simple.bsdl", read("access_link/simple.bsdl"), "AccessProbe");
  const auto network =
      pattern::ElaborateTapNetwork(design, "ResourceAccessTop");
  pattern::ScanScheduler scheduler(network.netlist);
  scheduler.Reset();
  // Establish selection, then close it on the target's update edge.
  scheduler.Apply({{"iWrite", {"core.gate", "1"}, "close.pdl:1", {}}});
  const auto steps =
      scheduler.Apply({{"iWrite", {"core.leaf.value", "5"}, "close.pdl:2", {}},
                       {"iWrite", {"core.gate", "0"}, "close.pdl:3", {}}});
  Require(
      steps.size() == 1 && steps[0].writes.at("core.gate") == "00" &&
          steps[0].writes.at("core.leaf.value") == "00000101" &&
          scheduler.Value("core.gate") == "00" &&
          scheduler.Value("core.leaf.value") == "00000101",
      "pre-update selection permits the target write and SIB close together");
  const auto reopen = scheduler.Apply(
      {{"iWrite", {"core.leaf.value", "6"}, "close.pdl:4", {}}});
  Require(reopen.size() == 2 && reopen[0].writes.at("core.gate") == "01" &&
              scheduler.Value("core.leaf.value") == "00000110",
          "a subsequent target write must reopen the closed select first");
}
void TestImplicitControl() {
  pattern::Design design;
  design.AddIcl("implicit.icl", R"(
Module Child {
 ScanInPort si; ScanOutPort so { Source r; }
 SelectPort sel; ShiftEnPort shift;
 ScanRegister r { ScanInSource si; }
}
Module Network {
 ScanInPort si;
 Instance child Of Child { InputPort si = si; InputPort sel = 1'b0; }
 ScanOutPort so { Source child.so; }
})");
  const auto netlist = pattern::Elaborate(design, "Network");
  bool rejected = false;
  try {
    pattern::ScanScheduler scheduler(netlist);
    scheduler.Apply({{"iWrite", {"child.r", "1"}, "test:1", {}}});
  } catch (const std::exception&) {
    rejected = true;
  }
  Require(rejected, "implicit disabled client controls must not be ignored");
}
void TestResetFill() {
  pattern::Design design;
  design.AddIcl("reset-fill.icl", R"(
Module Child {
 ScanInPort si; ResetPort rst;
 ScanRegister r { ScanInSource si; ResetValue 1'b0; }
 ScanOutPort so { Source r; }
}
Module Network {
 ScanInPort si; ScanRegister reset { ScanInSource si; }
 Instance child Of Child { InputPort si = reset; InputPort rst = reset; }
 ScanOutPort so { Source child.so; }
})");
  const auto netlist = pattern::Elaborate(design, "Network");
  pattern::ScanScheduler scheduler(netlist);
  scheduler.Assume("reset", "0");
  scheduler.Assume("child.r", "1");
  scheduler.Apply({{"iWrite", {"reset", "1"}, "test:1", {}}});
  scheduler.Apply({{"iWrite", {"reset", "0"}, "test:2", {}}});
  scheduler.Apply({{"iWrite", {"child.r", "0bx"}, "test:3", {}}});
  Require(scheduler.Value("child.r") == "0",
          "X write after reset must not resurrect pre-reset fill data");
}
void TestReadCapture() {
  pattern::Design design;
  design.AddIcl("read.icl", R"(
Module Network {
 ScanInPort si;
 ScanRegister r[1:0] { ScanInSource si; CaptureSource 2'b10; }
 ScanOutPort so { Source r[0]; }
})");
  const auto netlist = pattern::Elaborate(design, "Network");
  pattern::ScanScheduler scheduler(netlist);
  const auto steps = scheduler.Apply({{"iRead", {"r", "0b10"}, "test:1", {}},
                                      {"iWrite", {"r", "0"}, "test:2", {}}});
  Require(steps.size() == 1 && steps[0].expected.at("r") == "10" &&
              steps[0].serial_expected == "01" && scheduler.Value("r") == "00",
          "expected capture data is separate from update data and serialized "
          "LSB first");
  const auto read_only = scheduler.Apply({{"iRead", {"r"}, "test:3", {}}});
  Require(read_only.size() == 1 && read_only[0].expected.at("r") == "xx",
          "all-X reads still require an observation cycle");
}
void TestMinimumScans() {
  pattern::Design design;
  design.AddIcl("nested.icl", R"(
Module Network {
 ScanInPort si;
 ScanRegister outer { ScanInSource si; }
 ScanRegister inner { ScanInSource outer; }
 ScanRegister target[3:0] { ScanInSource inner; }
 ScanMux inner_path SelectedBy inner { 1'b0: inner; 1'b1: target[0]; }
 ScanMux outer_path SelectedBy outer { 1'b0: outer; 1'b1: inner_path; }
 ScanOutPort so { Source outer_path; }
})");
  const auto netlist = pattern::Elaborate(design, "Network");
  using State = std::tuple<int, int, bool>;
  for (int outer = 0; outer < 2; ++outer) {
    for (int inner = 0; inner < 2; ++inner) {
      // Independent finite-state breadth-first oracle; no production model or
      // Z3.
      std::map<State, size_t> frontier{{{outer, inner, false}, 0}};
      size_t scans = 0, cycles = 0;
      while (true) {
        ++scans;
        std::map<State, size_t> next;
        for (const auto& [state, cost] : frontier) {
          const auto [a, b, read] = state;
          for (int next_a = 0; next_a < 2; ++next_a) {
            for (int next_b = 0; next_b < 2; ++next_b) {
              if (!a && next_b != b) continue;
              const State destination{next_a, next_b, read || (a && b)};
              const size_t total = cost + (a ? (b ? 6 : 2) : 1);
              const auto old = next.find(destination);
              if (old == next.end() || total < old->second)
                next[destination] = total;
            }
          }
        }
        cycles = static_cast<size_t>(-1);
        for (const auto& [state, cost] : next)
          if (std::get<2>(state)) cycles = std::min(cycles, cost);
        if (cycles != static_cast<size_t>(-1)) break;
        frontier = std::move(next);
        Require(scans < 4, "finite reference search must converge");
      }
      pattern::ScanScheduler scheduler(netlist);
      scheduler.Assume("outer", std::to_string(outer));
      scheduler.Assume("inner", std::to_string(inner));
      const auto steps =
          scheduler.Apply({{"iWrite", {"target", "5"}, "test:1", {}}});
      size_t actual_cycles = 0;
      for (const auto& step : steps) actual_cycles += step.serial.size();
      Require(steps.size() == scans && actual_cycles == cycles,
              "solver agrees with independent shortest scan/cycle search");
    }
  }
}
}  // namespace
int main() {
  try {
    TestOneHotSharedAnalysisBudget();
    TestMergeAnalysisBudget();
    TestSharedScanOutputs();
    TestEnabledScanOutputs();
    TestClientScanControls();
    TestMultipleClientSelection();
    TestSimultaneousScanChains();
    TestHiddenClientEvents();
    TestVectorScanInputs();
    TestMixedReadRoutes();
    TestMixedIoPhases();
    TestPrimaryObservationOfScanStorage();
    TestPrimaryAccessTogether();
    TestMixedPortGoals();
    TestParallelControlReservations();
    TestResetDependencyScale();
    TestGreedyRollback();
    TestAddressedRollback();
    TestReservationRollback();
    TestMaskedReservationRollback();
    TestPrimaryWrites();
    TestPrimaryLimits();
    TestPrimaryControllingState();
    TestDataReservationRollback();
    TestHiddenScanState();
    TestTapScheduling(false, false, false);
    TestTapScheduling(true, false, false);
    TestTapScheduling(false, true, true);
    TestChangingIdleState();
    TestTopPortAssumptions();
    TestTapExternalReset();
    TestTapControlRejection();
    TestApplyTogether();
    TestAccessTogether();
    TestAccessTogetherPrerequisites();
    TestEndState();
    TestPrerequisites();
    TestUnknownAndDisconnected();
    TestImplicitControl();
    TestCloseSelectOnTargetUpdate();
    TestResetFill();
    TestReadCapture();
    TestMinimumScans();
    std::cout << "pattern scheduler tests passed\n";
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
