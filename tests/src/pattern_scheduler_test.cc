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

#include "pattern/pattern_clock.h"
#include "pattern/pattern_data.h"
#include "pattern/pattern_design.h"
#include "pattern/pattern_flatten.h"
#include "pattern/pattern_merge.h"
#include "pattern/pattern_resources.h"
#include "pattern/pattern_state.h"
#include "pattern/pattern_tap.h"
#include "pattern/pattern_tap_execution.h"

namespace {
void Require(bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}
std::string RawSibModel() {
  return R"ICL(Module RawSibLeaf {
 ScanInPort si; SelectPort sel; CaptureEnPort ce; ShiftEnPort se;
 UpdateEnPort ue; TCKPort tck; ResetPort reset;
 ScanRegister value[7:0] { ScanInSource si; CaptureSource value; ResetValue 8'b0; }
 ScanOutPort so { Source value[0]; }
 ScanInterface local { Port si; Port so; Port sel; Port ce; Port se; Port ue; Port tck; Port reset; }
}
Module Top {
 ScanInPort si; SelectPort sel; CaptureEnPort ce; ShiftEnPort se;
 UpdateEnPort ue; TCKPort tck; ResetPort reset;
 Instance child Of RawSibLeaf {
  InputPort si = si; InputPort sel = open; InputPort ce = ce;
  InputPort se = se; InputPort ue = ue; InputPort tck = tck; InputPort reset = reset;
 }
 ScanMux route SelectedBy open { 1'b0: si; 1'b1: child.so; }
 ScanRegister open { ScanInSource route; CaptureSource open; ResetValue 1'b0; }
 ScanOutPort so { Source open; }
 ScanInterface client {
  Port si; Port so; Port sel; Port ce; Port se; Port ue; Port tck; Port reset;
  DefaultLoadValue 1'b0;
 }
}
)ICL";
}
void TestAutomaticChildRawPreparation() {
  std::string source = RawSibModel();
  for (bool flat : {false, true}) {
    pattern::Design design;
    design.AddIcl("automatic-child.icl", source);
    const auto netlist = pattern::Elaborate(design, "Top");
    const auto id = [&](const std::string& name) {
      return netlist.node_ids.at(name);
    };
    const auto open = id("open"), value = id("child.value");
    pattern::ScanScheduler scheduler(netlist), manual(netlist);
    scheduler.Reset();
    manual.Reset();
    const pattern::Operation raw{
        "iScan",
        {"child.local", "8", "-si", "0x96", "-so", "0", "-stable"},
        "automatic:raw",
        {}};
    bool strict = false;
    try {
      scheduler.ApplyScan(raw);
    } catch (const std::runtime_error&) {
      strict = true;
    }
    Require(strict, "default native child apply remains accessible-only");
    std::map<std::string, std::string> before;
    for (const auto& node : netlist.nodes) {
      if (node.model == pattern::ModelKind::kPort ||
          node.model == pattern::ModelKind::kRegister) {
        before.emplace(node.name, scheduler.Value(node.name));
      }
    }
    pattern::ScanStep sentinel;
    sentinel.writes.emplace("sentinel", "1");
    std::vector<pattern::ScanStep> preparations{sentinel};
    std::set<size_t> assigned{open};
    auto invalid = raw;
    invalid.arguments = {"child.local", "7", "-si", "0"};
    for (bool protected_parent : {false, true}) {
      bool rejected = false;
      try {
        scheduler.ApplyScan(protected_parent ? raw : invalid,
                            protected_parent
                                ? std::map<size_t, std::string>{{open, "0"}}
                                : std::map<size_t, std::string>{},
                            &assigned, &preparations);
      } catch (const std::runtime_error&) {
        rejected = true;
      }
      Require(rejected && assigned == std::set<size_t>{open} &&
                  preparations.size() == 1 &&
                  preparations[0].writes == sentinel.writes,
              "failed automatic setup leaves both caller outputs untouched");
      for (const auto& [name, bits] : before) {
        Require(scheduler.Value(name) == bits,
                "failed prefix/raw validation adopts no hardware state");
      }
    }
    const auto physical =
        scheduler.ApplyScan(raw, {}, &assigned, &preparations);
    Require(
        preparations.size() == 1 &&
            preparations[0].writes ==
                std::map<std::string, std::string>{{"open", "1"}} &&
            preparations[0].expected.empty() && preparations[0].serial == "1" &&
            physical.arguments[0] == "client" && physical.arguments[1] == "9" &&
            physical.arguments[3] == "0b100101101" &&
            physical.arguments[5] == "0b00000000x" &&
            assigned == std::set<size_t>{value},
        "opt-in returns parent setup and exact child-only raw footprint");
    manual.Apply({{"iWrite", {"open", "1"}, "manual:setup", {}}});
    Require(manual.ApplyScan(raw).arguments == physical.arguments,
            "automatic child packet matches explicit manual preparation");
    pattern::NetworkState literal(netlist);
    const auto pin = [&](const std::string& name, char bit) {
      literal.Drive(id(name), std::string(1, bit));
    };
    pin("reset", '1');
    literal.SetGlobalReset('1', '1');
    literal.Reset();
    pin("reset", '0');
    literal.SetGlobalReset('0', '0');
    literal.Reset();
    const auto phase = [&](char capture, char shift, char update, char clock) {
      pin("sel", '1');
      pin("ce", capture);
      pin("se", shift);
      pin("ue", update);
      pin("tck", clock);
      literal.Reset();
    };
    const auto controls = [&](char capture, char shift, char update) {
      return std::map<size_t, pattern::ScanControl>{
          {open, {capture, shift, update, '1'}},
          {value, {capture, shift, update, literal.Value(open)[0]}}};
    };
    const auto scan = [&](const std::string& payload,
                          const std::string& expected) {
      phase('1', '0', '0', '0');
      pin("tck", '1');
      literal.Clock(controls('1', '0', '0'), true);
      for (size_t i = 0; i < payload.size(); ++i) {
        phase('0', '1', '0', '0');
        pin("si", payload[payload.size() - 1 - i]);
        const char mask = expected[expected.size() - 1 - i];
        Require(mask == 'x' || literal.Read({id("so"), 0}, true) == mask,
                "literal child capture masks use actual physical SO order");
        pin("tck", '1');
        literal.Clock(controls('0', '1', '0'), true);
      }
      phase('0', '0', '1', '0');
      literal.Clock(controls('0', '0', '1'), false);
      phase('0', '0', '0', '0');
    };
    scan("1", "x");
    scan("100101101", "00000000x");
    Require(literal.Value(value) == "10010110" && literal.Value(open) == "1",
            "literal prefix/raw execution retains parent and loads child96");
    for (const auto& [name, bits] : before) {
      Require(scheduler.Value(name) == manual.Value(name) &&
                  scheduler.Value(name) == literal.Value(id(name)),
              "automatic/manual/literal agree on every public endpoint");
    }
    scheduler.Apply({{"iWrite", {"child.value[0]", "1"}, "auto:partial", {}}});
    Require(scheduler.Value("child.value") == "10010111",
            "raw controlling fill supports following partial child write");
    if (!flat) source = pattern::SerializeIcl(netlist);
  }
  std::ifstream file(
      "tests/cases/ours/pattern_generation/dynamic_child_raw/model.icl");
  const std::string nested((std::istreambuf_iterator<char>(file)), {});
  Require(!nested.empty(), "automatic nested stable fixture exists");
  pattern::Design design;
  design.AddIcl("automatic-stable.icl", nested);
  const auto netlist = pattern::Elaborate(design, "Top");
  pattern::ScanScheduler scheduler(netlist);
  scheduler.Reset();
  std::map<std::string, std::string> before;
  for (const auto& node : netlist.nodes) {
    if (node.model == pattern::ModelKind::kPort ||
        node.model == pattern::ModelKind::kRegister) {
      before.emplace(node.name, scheduler.Value(node.name));
    }
  }
  pattern::ScanStep sentinel;
  sentinel.writes.emplace("sentinel", "1");
  std::vector<pattern::ScanStep> preparations{sentinel};
  const auto open = netlist.node_ids.at("open");
  std::set<size_t> assigned{open};
  bool rejected = false;
  try {
    scheduler.ApplyScan({"iScan",
                         {"child.local", "1", "-si", "1", "-stable"},
                         "automatic:stable-failure",
                         {}},
                        {}, &assigned, &preparations);
  } catch (const std::runtime_error& error) {
    rejected = std::string(error.what()).find("stable") != std::string::npos;
  }
  Require(rejected && preparations.size() == 1 &&
              preparations[0].writes == sentinel.writes &&
              assigned == std::set<size_t>{open},
          "post-setup stable failure publishes no prefix or assigned output");
  for (const auto& [name, bits] : before) {
    Require(scheduler.Value(name) == bits,
            "post-setup stable failure restores every caller-visible value");
  }
  scheduler.ApplyScan({"iScan",
                       {"child.leaf.local", "8", "-si", "0x96", "-stable"},
                       "automatic:retry",
                       {}},
                      {}, &assigned, &preparations);
  Require(preparations.size() == 2 &&
              assigned ==
                  std::set<size_t>{netlist.node_ids.at("child.leaf.value")} &&
              scheduler.Value("open") == "1" &&
              scheduler.Value("child.enable") == "1" &&
              scheduler.Value("child.leaf.value") == "10010110",
          "valid retry returns both parent CSUs and exact leaf footprint");
}
void TestRawTapMultipleDrScans() {
  const std::string folder =
      "tests/cases/ours/pattern_generation/raw_tap_multiple_dr/";
  const auto read = [&](const std::string& name) {
    std::ifstream f(folder + name);
    return std::string((std::istreambuf_iterator<char>(f)), {});
  };
  const auto replace = [](std::string& text, const std::string& from,
                          const std::string& to) {
    const auto pos = text.find(from);
    Require(pos != std::string::npos,
            "multi-register fixture replacement exists");
    text.replace(pos, from.size(), to);
  };
  for (unsigned flags = 0; flags < 8; ++flags) {
    for (bool ascending : {false, true}) {
      const bool si = flags & 1, between = flags & 2, so = flags & 4;
      auto model = read("model.icl");
      replace(model, "ScanInSource si; CaptureSource a;",
              std::string("ScanInSource ") + (si ? "~si" : "si") +
                  "; CaptureSource a;");
      replace(model, "  ScanRegister b",
              "  Alias hop[2:0] = a;\n  Alias forwarding[2:0] = hop;\n  "
              "ScanRegister b");
      replace(model, "ScanInSource a[0];",
              std::string("ScanInSource ") + (between ? "~" : "") +
                  "forwarding[0];");
      replace(model, "Source mux;",
              std::string("Source ") + (so ? "~" : "") + "mux;");
      if (ascending) {
        replace(model, "a[2:0]", "a[0:2]");
        replace(model, "b[4:0]", "b[0:4]");
        replace(model, "b[0];", "b[4];");
      }
      const int avalue = 4 ^ (si ? 7 : 0),
                bvalue = 22 ^ ((si != between) ? 31 : 0);
      const unsigned observed = ((between != so) ? 0xe0 : 0) | (so ? 0x1f : 0);
      std::string physical(8, '0');
      for (size_t bit = 0; bit < 8; ++bit)
        physical[7 - bit] = (observed >> bit) & 1 ? '1' : '0';
      for (size_t bit : {0, 1, 6, 7}) physical[bit] = 'x';
      pattern::Design design;
      design.AddIcl("multiple.icl", model);
      design.AddBsdl("multiple.bsdl", read("model.bsdl"), "MultipleDrProbe");
      const auto network =
          pattern::ElaborateTapNetwork(design, "MultipleDrTop");
      const auto& netlist = network.netlist;
      const auto& tap = netlist.taps.begin()->second;
      const auto a = netlist.node_ids.at("core.a"),
                 b = netlist.node_ids.at("core.b");
      pattern::ScanScheduler reference(netlist);
      reference.Reset();
      reference.Apply(
          {{"iWrite", {"tap.INSTRUCTION", "3"}, "multi:prepare", {}}});
      pattern::ScanScheduler scheduler(reference);
      const auto steps = reference.Apply(
          {{"iWrite", {"core.a", std::to_string(avalue)}, "multi:a", {}},
           {"iWrite", {"core.b", std::to_string(bvalue)}, "multi:b", {}},
           {"iRead", {"core.a", "0bxx0"}, "multi:read-a", {}},
           {"iRead", {"core.b", "0b000xx"}, "multi:read-b", {}}});
      Require(steps.size() == 1 && !steps[0].instruction &&
                  steps[0].serial == "01101001" &&
                  steps[0].serial_expected ==
                      std::string(physical.rbegin(), physical.rend()) &&
                  steps[0].loads.size() == 2,
              "ordinary backend proves one ordered unequal-register data CSU");
      const pattern::Operation request{
          "iScan",
          {"jtag", "8", "-si", "0x96", "-so", "0b" + physical, "-stable"},
          "multi:raw",
          {}};
      const auto normalized = scheduler.NormalizeScan(request);
      std::set<size_t> assigned{netlist.nodes.size() + 1};
      scheduler.ApplyScan(normalized, {{a, "000"}, {b, "00000"}}, &assigned);
      Require(assigned == std::set<size_t>{a, b} &&
                  std::stoi(scheduler.Value("core.a"), nullptr, 2) == avalue &&
                  std::stoi(scheduler.Value("core.b"), nullptr, 2) == bvalue,
              "raw packet stores upstream prefix/downstream suffix and assigns "
              "pre-path registers");
      pattern::TapExecution literal(netlist);
      literal.Drive({{tap.pins.at("tap_scan_reset"), "0"}});
      literal.Drive({{tap.pins.at("tap_scan_reset"), "1"}});
      size_t cycles = 0;
      const auto cycle = [&](char tms, char tdi = '0') {
        literal.Drive({{tap.pins.at("tap_scan_mode"), std::string(1, tms)},
                       {tap.pins.at("tap_scan_in"), std::string(1, tdi)}});
        literal.RisingEdge(std::set<size_t>{tap.pins.at("tap_scan_clock")});
        literal.FallingEdge(std::set<size_t>{tap.pins.at("tap_scan_clock")});
        ++cycles;
      };
      cycle('0');
      for (char tms : std::string("1100")) cycle(tms);
      for (size_t i = 0; i < 4; ++i)
        cycle(i == 3 ? '1' : '0', i < 2 ? '1' : '0');
      cycle('1');
      cycle('0');
      cycles = 0;
      for (char tms : std::string("100")) cycle(tms);
      const std::string serial = "01101001",
                        expected(physical.rbegin(), physical.rend());
      for (size_t i = 0; i < 8; ++i) {
        Require(expected[i] == 'x' ||
                    literal.Value("TDO", true) == std::string(1, expected[i]),
                "literal unequal-chain capture mask");
        cycle(i == 7 ? '1' : '0', serial[i]);
      }
      cycle('1');
      cycle('0');
      Require(cycles == 13, "unequal eight-bit chain uses N+5 clocks");
      for (const auto& node : netlist.nodes) {
        if (node.model == pattern::ModelKind::kPort ||
            node.model == pattern::ModelKind::kRegister) {
          Require(scheduler.Value(node.name) == reference.Value(node.name) &&
                      scheduler.Value(node.name) == literal.Value(node.name),
                  "multi-register raw equals ordinary and literal endpoints");
        }
      }
      scheduler.Idle(1);
      cycle('0');
      Require(std::stoi(scheduler.Value("core.a"), nullptr, 2) == avalue &&
                  std::stoi(scheduler.Value("core.b"), nullptr, 2) == bvalue,
              "multi-register raw state survives idle");
    }
  }
  auto dynamic = read("model.icl");
  replace(dynamic, "ScanInSource a[0];", "ScanInSource routing;");
  replace(dynamic, "  ScanRegister b",
          "  ScanRegister extra {ScanInSource a[0]; CaptureSource extra; "
          "ResetValue 0;}\n  ScanMux routing SelectedBy b[0] {1'b0: a[0]; "
          "1'b1: extra;}\n  ScanRegister b");
  pattern::Design dynamic_design;
  dynamic_design.AddIcl("dynamic.icl", dynamic);
  dynamic_design.AddBsdl("multiple.bsdl", read("model.bsdl"),
                         "MultipleDrProbe");
  const auto dynamic_network =
      pattern::ElaborateTapNetwork(dynamic_design, "MultipleDrTop");
  pattern::ScanScheduler changing(dynamic_network.netlist);
  changing.Reset();
  changing.Apply({{"iWrite", {"tap.INSTRUCTION", "3"}, "dynamic:prepare", {}}});
  const auto aid = dynamic_network.netlist.node_ids.at("core.a"),
             bid = dynamic_network.netlist.node_ids.at("core.b"),
             eid = dynamic_network.netlist.node_ids.at("core.extra");
  pattern::Operation open{"iScan",
                          {"jtag", "8", "-si", "0x97", "-so", "0", "-stable"},
                          "dynamic:raw",
                          {}};
  std::set<size_t> assigned{dynamic_network.netlist.nodes.size() + 5};
  const auto sentinel = assigned;
  std::map<std::string, std::string> before;
  for (const auto& node : dynamic_network.netlist.nodes)
    if (node.model == pattern::ModelKind::kRegister ||
        node.model == pattern::ModelKind::kPort)
      before.emplace(node.name, changing.Value(node.name));
  auto protected_open = open;
  protected_open.arguments.pop_back();
  bool protected_rejected = false;
  try {
    changing.ApplyScan(protected_open, {{eid, "0"}}, &assigned);
  } catch (const std::runtime_error& error) {
    const std::string message = error.what();
    protected_rejected =
        message.find("reserved core.extra") != std::string::npos ||
        message.find("one exact requested CSU") != std::string::npos;
  }
  Require(protected_rejected && assigned == sentinel,
          "protected off-path register rejects its raw physical side effect");
  for (const auto& [name, value] : before)
    Require(changing.Value(name) == value,
            "failed off-path preservation leaves all endpoint state unchanged");
  bool rejected = false;
  try {
    changing.ApplyScan(open, {}, &assigned);
  } catch (const std::runtime_error& error) {
    rejected = std::string(error.what()).find("stable") != std::string::npos;
  }
  Require(
      rejected && assigned == sentinel,
      "multi-register unstable post-route rejects before assigned publication");
  for (const auto& [name, value] : before)
    Require(
        changing.Value(name) == value,
        "failed multi-register stable transaction preserves every endpoint");
  open.arguments.pop_back();
  changing.ApplyScan(open, {}, &assigned);
  Require(assigned == std::set<size_t>{aid, bid} &&
              changing.Value("core.extra") == "1",
          "raw topology opening assigns pre-CSU path only and retains "
          "actual unaddressed side effects");
  const pattern::Operation full{
      "iScan",
      {"jtag", "9", "-si", "389", "-so", "311", "-stable"},
      "dynamic:full",
      {}};
  changing.ApplyScan(full, {{eid, "1"}}, &assigned);
  Require(assigned == std::set<size_t>{aid, bid, eid} &&
              changing.Value("core.a") == "110" &&
              changing.Value("core.extra") == "0" &&
              changing.Value("core.b") == "00101",
          "later full nine-bit route authoritatively owns its new pre-CSU "
          "register");
}
void TestNoIdBuiltinBypassAliases() {
  const std::string folder =
      "tests/cases/ours/pattern_generation/raw_tap_builtin_dr/";
  const auto read = [&](const std::string& name) {
    std::ifstream file(folder + name);
    return std::string((std::istreambuf_iterator<char>(file)), {});
  };
  auto source = read("model.icl");
  auto bsdl = read("model.bsdl");
  const auto replace = [&](const std::string& from, const std::string& to) {
    const auto at = bsdl.find(from);
    Require(at != std::string::npos, "no-ID fixture replacement exists");
    bsdl.replace(at, from.size(), to);
  };
  replace("BYPASS (1111)", "BYPASS (1000)");
  replace("IDCODE (0010), ", "");
  replace(", DEVICE_ID (IDCODE)", "");
  const auto begin = bsdl.find("  attribute IDCODE_REGISTER");
  const auto end = bsdl.find(';', begin);
  Require(begin != std::string::npos && end != std::string::npos,
          "no-ID fixture attribute exists");
  bsdl.erase(begin, end - begin + 1);
  for (bool flat : {false, true}) {
    pattern::Design design;
    design.AddIcl("no-id.icl", source);
    design.AddBsdl("no-id.bsdl", bsdl, "BuiltinDrProbe");
    const auto netlist =
        flat ? pattern::Elaborate(design, "BuiltinDrTop")
             : pattern::ElaborateTapNetwork(design, "BuiltinDrTop").netlist;
    pattern::ScanScheduler scheduler(netlist);
    scheduler.Reset();
    Require(scheduler.Value("tap.INSTRUCTION") == "1xxx",
            "reset does not fabricate a unique BYPASS encoding");
    scheduler.ApplyScan(
        {"iScan", {"jtag", "1", "-si", "1", "-so", "0"}, "no-id:scan", {}});
    Require(scheduler.Value("tap.INSTRUCTION") == "1xxx",
            "known BYPASS data scan retains uncertain held IR bits");
    for (const auto& code : {"8", "15"}) {
      scheduler.ApplyScan(
          {"iScan", {"-ir", "jtag", "4", "-si", code}, "no-id:ir", {}});
      scheduler.ApplyScan(
          {"iScan", {"jtag", "1", "-si", "0", "-so", "0"}, "no-id:data", {}});
    }
    if (!flat) source = pattern::SerializeIcl(netlist);
  }
}
void TestRawTapBuiltinDrScans() {
  const std::string folder =
      "tests/cases/ours/pattern_generation/raw_tap_builtin_dr/";
  const auto read = [&](const std::string& name) {
    std::ifstream file(folder + name);
    return std::string((std::istreambuf_iterator<char>(file)), {});
  };
  pattern::Design design;
  design.AddIcl("builtin.icl", read("model.icl"));
  design.AddBsdl("builtin.bsdl", read("model.bsdl"), "BuiltinDrProbe");
  const auto network = pattern::ElaborateTapNetwork(design, "BuiltinDrTop");
  const auto& netlist = network.netlist;
  const auto& tap = netlist.taps.begin()->second;
  pattern::ScanScheduler scheduler(netlist);
  scheduler.Reset();
  const auto public_rejected = [&](pattern::ScanScheduler& owner) {
    for (const auto& name :
         std::vector<std::string>{"tap.DEVICE_ID", "tap.BYPASS"}) {
      for (const auto& command : std::vector<std::string>{"iWrite", "iRead"}) {
        bool rejected = false;
        try {
          owner.Apply({{command, {name, "0"}, "public:builtin", {}}});
        } catch (const std::runtime_error& error) {
          rejected = std::string(error.what()).find("synthetic TAP") !=
                     std::string::npos;
        }
        Require(rejected, "public builtin operands stay forbidden");
      }
    }
  };
  public_rejected(scheduler);
  const pattern::Operation idcode{
      "iScan",
      {"jtag", "32", "-si", "0x89abcdef", "-so", "0x10000003", "-stable"},
      "builtin:raw",
      {}};
  std::set<size_t> assigned{netlist.nodes.size() + 1};
  const auto normalized = scheduler.NormalizeScan(idcode);
  scheduler.ApplyScan(normalized, {{tap.instruction_register, "0010"}},
                      &assigned);
  Require(
      assigned == std::set<size_t>{tap.data_registers.at("DEVICE_ID")} &&
          scheduler.Value("tap.INSTRUCTION") == "0010",
      "raw IDCODE publishes only its builtin footprint and preserves owned IR");
  public_rejected(scheduler);
  pattern::ScanScheduler copy(scheduler);
  public_rejected(copy);
  pattern::TapExecution literal(netlist);
  literal.Drive({{tap.pins.at("tap_scan_reset"), "0"}});
  literal.Drive({{tap.pins.at("tap_scan_reset"), "1"}});
  size_t clocks = 0;
  const auto cycle = [&](char tms, char tdi = '0') {
    literal.Drive({{tap.pins.at("tap_scan_mode"), std::string(1, tms)},
                   {tap.pins.at("tap_scan_in"), std::string(1, tdi)}});
    literal.RisingEdge(std::set<size_t>{tap.pins.at("tap_scan_clock")});
    literal.FallingEdge(std::set<size_t>{tap.pins.at("tap_scan_clock")});
    ++clocks;
  };
  const auto compare = [&] {
    for (const auto& node : netlist.nodes) {
      if (node.model == pattern::ModelKind::kPort ||
          node.model == pattern::ModelKind::kRegister) {
        Require(scheduler.Value(node.name) == literal.Value(node.name),
                "builtin raw matches independent literal endpoint state");
      }
    }
  };
  cycle('0');
  clocks = 0;
  for (char tms : std::string("100")) cycle(tms);
  const std::string numeric = "10001001101010111100110111101111";
  const std::string capture = "00010000000000000000000000000011";
  for (size_t i = 0; i < 32; ++i) {
    Require(literal.Value("TDO", true) == std::string(1, capture[31 - i]),
            "literal IDCODE capture shifts LSB first");
    cycle(i == 31 ? '1' : '0', numeric[31 - i]);
  }
  cycle('1');
  cycle('0');
  Require(clocks == 37, "IDCODE32 from idle uses exactly N+5 clocks");
  compare();
  scheduler.Apply(
      {{"iWrite", {"tap.INSTRUCTION", "15"}, "builtin:select", {}}});
  for (char tms : std::string("1100")) cycle(tms);
  for (size_t i = 0; i < 4; ++i) cycle(i == 3 ? '1' : '0', '1');
  cycle('1');
  cycle('0');
  clocks = 0;
  const pattern::Operation bypass{
      "iScan",
      {"jtag", "1", "-si", "1", "-so", "0", "-stable"},
      "builtin:bypass",
      {}};
  scheduler.ApplyScan(bypass, {{tap.instruction_register, "1111"}}, &assigned);
  for (char tms : std::string("100")) cycle(tms);
  Require(literal.Value("TDO", true) == "0",
          "BYPASS capture is zero independent of prior shift");
  cycle('1', '1');
  cycle('1');
  cycle('0');
  Require(clocks == 6 &&
              assigned == std::set<size_t>{tap.data_registers.at("BYPASS")},
          "BYPASS1 uses N+5 clocks and exact assigned register");
  compare();
  scheduler.Idle(1, {{tap.instruction_register, "1111"}});
  cycle('0');
  compare();
  public_rejected(scheduler);
  assigned = {netlist.nodes.size() + 99};
  const auto failed_assigned = assigned;
  bool private_rejected = false;
  try {
    scheduler.ApplyScan(bypass, {{tap.instruction_register, "0010"}},
                        &assigned);
  } catch (const std::runtime_error& error) {
    // This context is appended by private Apply, after raw normalization.
    private_rejected =
        std::string(error.what()).find("reserved tap.INSTRUCTION") !=
        std::string::npos;
  }
  Require(
      private_rejected && assigned == failed_assigned,
      "valid-width builtin private solve failure preserves assigned output");
  compare();
  public_rejected(scheduler);
  pattern::ScanScheduler failed_copy(scheduler);
  public_rejected(failed_copy);
  scheduler.ApplyScan(bypass, {{tap.instruction_register, "1111"}}, &assigned);
  for (char tms : std::string("100")) cycle(tms);
  cycle('1', '1');
  cycle('1');
  cycle('0');
  Require(assigned == std::set<size_t>{tap.data_registers.at("BYPASS")},
          "successful raw retry follows a failed private solve");
  compare();
  auto invalid = bypass;
  invalid.arguments[1] = "2";
  const auto before = assigned;
  bool rejected = false;
  try {
    scheduler.ApplyScan(invalid, {}, &assigned);
  } catch (const std::runtime_error& error) {
    rejected = std::string(error.what()).find("length") != std::string::npos;
  }
  Require(rejected && assigned == before,
          "builtin width failure preserves assigned publication");
  compare();
  public_rejected(scheduler);
}
void TestRawTapDrScans() {
  const std::string folder = "tests/cases/ours/pattern_generation/raw_tap_dr/";
  const auto read = [&](const std::string& name) {
    std::ifstream file(folder + name);
    const std::string text((std::istreambuf_iterator<char>(file)), {});
    Require(!text.empty(), "raw DR fixture exists");
    return text;
  };
  pattern::Design design;
  design.AddIcl("raw-dr.icl", read("model.icl"));
  design.AddBsdl("raw-dr.bsdl", read("model.bsdl"), "RawDrProbe");
  const auto network = pattern::ElaborateTapNetwork(design, "RawDrTop");
  const auto& netlist = network.netlist;
  const auto& tap = netlist.taps.begin()->second;
  const auto reg = netlist.node_ids.at("core.r");
  pattern::ScanScheduler reference(netlist);
  reference.Reset();
  reference.Apply({{"iWrite", {"tap.INSTRUCTION", "3"}, "dr:prepare", {}}});
  pattern::ScanScheduler scheduler(reference);
  const auto steps =
      reference.Apply({{"iWrite", {"core.r", "0x96"}, "dr:reference", {}},
                       {"iRead", {"core.r", "0bxx000000"}, "dr:compare", {}}});
  Require(steps.size() == 1 && !steps[0].instruction &&
              steps[0].lanes.empty() && steps[0].serial == "01101001" &&
              steps[0].serial_expected == "000000xx" &&
              steps[0].loads.size() == 1 && steps[0].writes.size() == 1 &&
              steps[0].expected.size() == 1,
          "existing backend proves one exact modeled DR transaction");
  const pattern::Operation request{
      "iScan",
      {"jtag", "8", "-si", "0x96", "-so", "0bxx000000", "-stable"},
      "dr:raw",
      {}};
  const auto normalized = scheduler.NormalizeScan(request);
  Require(normalized.arguments[3] == "0b10010110" &&
              scheduler.Value("core.r") == "00000000",
          "DR normalization retains state and numeric bit order");
  auto invalid = normalized;
  invalid.arguments[1] = "9";
  std::set<size_t> assigned{netlist.nodes.size() + 1};
  const auto sentinel = assigned;
  bool rejected = false;
  try {
    scheduler.ApplyScan(invalid, {}, &assigned);
  } catch (const std::runtime_error& error) {
    rejected = std::string(error.what()).find("length") != std::string::npos;
  }
  Require(rejected && assigned == sentinel &&
              scheduler.Value("core.r") == "00000000",
          "bad raw DR geometry leaves state and assigned output unchanged");
  const auto physical =
      scheduler.ApplyScan(normalized, {{reg, "00000000"}}, &assigned);
  Require(physical.arguments == normalized.arguments &&
              assigned == std::set<size_t>{reg} &&
              scheduler.Value("core.r") == "10010110" &&
              scheduler.Value("tap.INSTRUCTION") == "0011",
          "raw DR assigns only pre-CSU data register and retains IR");
  pattern::TapExecution literal(netlist);
  literal.Drive({{tap.pins.at("tap_scan_reset"), "0"}});
  literal.Drive({{tap.pins.at("tap_scan_reset"), "1"}});
  size_t cycles = 0;
  const auto cycle = [&](char tms, char tdi = '0') {
    literal.Drive({{tap.pins.at("tap_scan_mode"), std::string(1, tms)},
                   {tap.pins.at("tap_scan_in"), std::string(1, tdi)}});
    literal.RisingEdge(std::set<size_t>{tap.pins.at("tap_scan_clock")});
    literal.FallingEdge(std::set<size_t>{tap.pins.at("tap_scan_clock")});
    ++cycles;
  };
  cycle('0');
  for (char tms : std::string("1100")) cycle(tms);
  const std::string instruction = "1100";
  for (size_t i = 0; i < instruction.size(); ++i)
    cycle(i + 1 == instruction.size() ? '1' : '0', instruction[i]);
  cycle('1');
  cycle('0');
  cycles = 0;
  for (char tms : std::string("100")) cycle(tms);
  const std::string serial = "01101001", expected = "000000xx";
  for (size_t i = 0; i < serial.size(); ++i) {
    const auto output =
        literal.Value(netlist.nodes[tap.pins.at("tap_scan_out")].name, true);
    Require(expected[i] == 'x' || output == std::string(1, expected[i]),
            "literal DR capture mask is observed LSB first");
    cycle(i + 1 == serial.size() ? '1' : '0', serial[i]);
  }
  cycle('1');
  cycle('0');
  Require(cycles == 13,
          "eight-bit DR from idle uses N+5 clocks including final exit shift");
  for (const auto& node : netlist.nodes) {
    if (node.model == pattern::ModelKind::kPort ||
        node.model == pattern::ModelKind::kRegister) {
      Require(scheduler.Value(node.name) == reference.Value(node.name) &&
                  scheduler.Value(node.name) == literal.Value(node.name),
              "raw DR equals ordinary proof and independent literal endpoints");
    }
  }
  scheduler.Idle(1);
  cycle('0');
  Require(scheduler.Value("core.r") == "10010110" &&
              literal.Value("core.r") == "10010110",
          "DR update survives following idle");
  auto disconnected = read("model.icl");
  const std::string old_input = "ScanInSource si; CaptureSource r;";
  disconnected.replace(disconnected.find(old_input), old_input.size(),
                       "ScanInSource feedback; CaptureSource r;");
  disconnected.insert(
      disconnected.find("  ScanMux mux"),
      "  ScanMux feedback SelectedBy r[0] {1'b0: si; 1'b1: 1'b0;}\n");
  pattern::Design bad_design;
  bad_design.AddIcl("post-route.icl", disconnected);
  bad_design.AddBsdl("raw-dr.bsdl", read("model.bsdl"), "RawDrProbe");
  const auto bad_network = pattern::ElaborateTapNetwork(bad_design, "RawDrTop");
  pattern::ScanScheduler bad(bad_network.netlist);
  bad.Reset();
  bad.Apply({{"iWrite", {"tap.INSTRUCTION", "3"}, "post:prepare", {}}});
  auto stable = request;
  stable.arguments[3] = "0x97";
  assigned = sentinel;
  std::map<std::string, std::string> before;
  for (const auto& node : bad_network.netlist.nodes) {
    if (node.model == pattern::ModelKind::kPort ||
        node.model == pattern::ModelKind::kRegister) {
      before.emplace(node.name, bad.Value(node.name));
    }
  }
  rejected = false;
  try {
    bad.ApplyScan(stable, {}, &assigned);
  } catch (const std::runtime_error& error) {
    rejected = std::string(error.what()).find("stable") != std::string::npos;
  }
  Require(rejected && assigned == sentinel,
          "disconnected stable post-route rejects before assigned publication");
  for (const auto& [name, bits] : before) {
    Require(bad.Value(name) == bits,
            "failed stable post-route preserves every endpoint");
  }
  stable.arguments.pop_back();
  bad.ApplyScan(stable, {}, &assigned);
  Require(bad.Value("core.r") == "10010111" &&
              assigned ==
                  std::set<size_t>{bad_network.netlist.node_ids.at("core.r")},
          "nonstable retry adopts exactly the requested current data CSU");
}
void TestRawTapIrScans() {
  const std::string folder = "tests/cases/ours/pattern_generation/raw_tap_ir/";
  const auto read = [&](const std::string& name) {
    std::ifstream file(folder + name);
    const std::string text((std::istreambuf_iterator<char>(file)), {});
    Require(!text.empty(), "raw IR fixture exists");
    return text;
  };
  pattern::Design design;
  design.AddIcl("raw-ir.icl", read("model.icl"));
  design.AddBsdl("raw-ir.bsdl", read("model.bsdl"), "RawIrProbe");
  const auto network = pattern::ElaborateTapNetwork(design, "RawIrTop");
  const auto& netlist = network.netlist;
  const auto& tap = netlist.taps.begin()->second;
  const auto ir = tap.instruction_register;
  pattern::ScanScheduler reference(netlist);
  reference.Reset();
  const auto proof = reference.Apply(
      {{"iWrite", {"tap.INSTRUCTION", "3"}, "ir:reference", {}},
       {"iRead", {"tap.INSTRUCTION", "0bxx01"}, "ir:compare", {}}});
  Require(proof.size() == 1 && proof[0].instruction && proof[0].lanes.empty() &&
              proof[0].input == "TDI" && proof[0].output == "TDO" &&
              proof[0].serial == "1100" && proof[0].serial_expected == "10xx" &&
              proof[0].loads.size() == 1 && proof[0].writes.size() == 1 &&
              proof[0].expected.size() == 1,
          "existing ordinary IR backend proves one exact four-bit transaction");
  pattern::ScanScheduler scheduler(netlist);
  scheduler.Reset();
  const pattern::Operation request{
      "iScan",
      {"-ir", "jtag", "4", "-si", "3", "-so", "0bxx01", "-stable"},
      "ir:raw",
      {}};
  const auto normalized = scheduler.NormalizeScan(request, true);
  Require(normalized.arguments[0] == "-ir" &&
              normalized.arguments[1] == "jtag" &&
              normalized.arguments[4] == "0b0011" &&
              normalized.arguments[6] == "0bxx01" &&
              scheduler.Value("tap.INSTRUCTION") == "0010",
          "raw IR normalization preserves leading flag, masks and reset state");
  auto invalid = normalized;
  invalid.arguments[2] = "5";
  const auto sentinel = netlist.nodes.size() + 100;
  std::set<size_t> assigned{sentinel};
  bool rejected = false;
  try {
    scheduler.ApplyScan(invalid, {}, &assigned);
  } catch (const std::runtime_error& error) {
    rejected = std::string(error.what()).find("length") != std::string::npos;
  }
  Require(rejected && assigned == std::set<size_t>{sentinel} &&
              scheduler.Value("tap.INSTRUCTION") == "0010",
          "invalid IR geometry leaves controller and assigned output intact");
  const auto physical =
      scheduler.ApplyScan(normalized, {{ir, "0010"}}, &assigned);
  Require(
      physical.arguments == normalized.arguments &&
          assigned == std::set<size_t>{ir} &&
          scheduler.Value("tap.INSTRUCTION") == "0011",
      "known raw IR authoritatively updates only its instruction footprint");
  pattern::TapExecution literal(netlist);
  literal.Drive({{tap.pins.at("tap_scan_reset"), "0"}});
  literal.Drive({{tap.pins.at("tap_scan_reset"), "1"}});
  size_t cycles = 0;
  const auto cycle = [&](char tms, char tdi = '0') {
    literal.Drive({{tap.pins.at("tap_scan_mode"), std::string(1, tms)},
                   {tap.pins.at("tap_scan_in"), std::string(1, tdi)}});
    literal.RisingEdge(std::set<size_t>{tap.pins.at("tap_scan_clock")});
    literal.FallingEdge(std::set<size_t>{tap.pins.at("tap_scan_clock")});
    ++cycles;
  };
  cycle('0');
  cycles = 0;
  for (char tms : std::string("1100")) cycle(tms);
  const std::string serial = "1100", expected = "10xx";
  for (size_t i = 0; i < serial.size(); ++i) {
    const auto output =
        literal.Value(netlist.nodes[tap.pins.at("tap_scan_out")].name, true);
    Require(expected[i] == 'x' || output == std::string(1, expected[i]),
            "literal TDO capture mask and LSB-first IR stream agree");
    cycle(i + 1 == serial.size() ? '1' : '0', serial[i]);
  }
  cycle('1');
  cycle('0');
  Require(cycles == 10, "four-bit IR scan from idle uses exactly N+6 clocks");
  for (const auto& node : netlist.nodes) {
    if (node.model == pattern::ModelKind::kPort ||
        node.model == pattern::ModelKind::kRegister) {
      Require(
          scheduler.Value(node.name) == reference.Value(node.name) &&
              scheduler.Value(node.name) == literal.Value(node.name),
          "raw IR matches ordinary proof and independent literal endpoints");
    }
  }
  scheduler.Idle(1);
  cycle('0');
  Require(scheduler.Value("tap.INSTRUCTION") == "0011" &&
              literal.Value("tap.INSTRUCTION") == "0011",
          "IR scan returns to idle and retains instruction on following wait");
  const auto again = [&](const std::vector<pattern::Operation>& operations,
                         const std::string& bits) {
    const auto packets = scheduler.Apply(operations);
    Require(
        packets.size() == 1 && packets[0].instruction &&
            packets[0].serial == std::string(bits.rbegin(), bits.rend()) &&
            packets[0].loads.at("tap.INSTRUCTION") == bits &&
            packets[0].writes == packets[0].loads,
        "ordinary IR still performs one full CSU for same/partial/read goals");
    cycles = 0;
    for (char tms : std::string("1100")) cycle(tms);
    for (size_t i = 0; i < bits.size(); ++i) {
      cycle(i + 1 == bits.size() ? '1' : '0', bits[bits.size() - i - 1]);
    }
    cycle('1');
    cycle('0');
    Require(cycles == bits.size() + 6,
            "ordinary IR uses exact literal N+6 clocks");
    for (const auto& node : netlist.nodes) {
      if (node.model == pattern::ModelKind::kPort ||
          node.model == pattern::ModelKind::kRegister) {
        Require(scheduler.Value(node.name) == literal.Value(node.name),
                "ordinary same/read/partial IR matches every literal endpoint");
      }
    }
  };
  again({{"iRead", {"tap.INSTRUCTION", "0bxx01"}, "ir:read-only", {}}}, "0011");
  again({{"iWrite", {"tap.INSTRUCTION", "3"}, "ir:same", {}},
         {"iRead", {"tap.INSTRUCTION", "0bxx01"}, "ir:same-read", {}}},
        "0011");
  again({{"iWrite", {"tap.INSTRUCTION[0]", "0"}, "ir:partial", {}},
         {"iRead", {"tap.INSTRUCTION", "0bxx01"}, "ir:partial-read", {}}},
        "0010");
  std::map<std::string, std::string> before;
  for (const auto& node : netlist.nodes) {
    if (node.model == pattern::ModelKind::kPort ||
        node.model == pattern::ModelKind::kRegister) {
      before[node.name] = scheduler.Value(node.name);
    }
  }
  rejected = false;
  try {
    scheduler.Apply({{"iWrite", {"tap.INSTRUCTION", "3"}, "ir:automatic", {}}},
                    false, {{ir, "0010"}}, false);
  } catch (const std::runtime_error&) {
    rejected = true;
  }
  Require(rejected,
          "automatic IR setup cannot refresh an established reservation");
  for (const auto& [name, bits] : before) {
    Require(scheduler.Value(name) == bits,
            "failed IR setup preserves every endpoint");
  }
  again({{"iWrite", {"tap.INSTRUCTION", "3"}, "ir:retry", {}},
         {"iRead", {"tap.INSTRUCTION", "0bxx01"}, "ir:retry-read", {}}},
        "0011");
}
void TestRawTapUnknownInput() {
  for (bool instruction : {false, true}) {
    const std::string folder =
        std::string("tests/cases/ours/pattern_generation/raw_tap_") +
        (instruction ? "ir/" : "dr/");
    const auto read = [&](const std::string& name) {
      std::ifstream file(folder + name);
      return std::string((std::istreambuf_iterator<char>(file)), {});
    };
    const std::string top = instruction ? "RawIrTop" : "RawDrTop";
    pattern::Design design;
    design.AddIcl("unknown.icl", read("model.icl"));
    design.AddBsdl("unknown.bsdl", read("model.bsdl"),
                   instruction ? "RawIrProbe" : "RawDrProbe");
    const auto original = pattern::ElaborateTapNetwork(design, top).netlist;
    pattern::Design flat;
    flat.AddIcl("unknown-flat.icl", pattern::SerializeIcl(original));
    const auto serialized = pattern::Elaborate(flat, top);
    for (const auto* model : {&original, &serialized}) {
      const auto& netlist = *model;
      const auto& tap = netlist.taps.begin()->second;
      const auto id = instruction ? tap.instruction_register
                                  : netlist.node_ids.at("core.r");
      pattern::ScanScheduler scheduler(netlist);
      scheduler.Reset();
      pattern::TapExecution literal(netlist);
      literal.Drive({{tap.pins.at("tap_scan_reset"), "0"}});
      literal.Drive({{tap.pins.at("tap_scan_reset"), "1"}});
      size_t clocks = 0;
      const auto cycle = [&](char tms, char tdi = '0') {
        literal.Drive({{tap.pins.at("tap_scan_mode"), std::string(1, tms)},
                       {tap.pins.at("tap_scan_in"), std::string(1, tdi)}});
        literal.RisingEdge(std::set<size_t>{tap.pins.at("tap_scan_clock")});
        literal.FallingEdge(std::set<size_t>{tap.pins.at("tap_scan_clock")});
        ++clocks;
      };
      const auto scan = [&](bool ir, const std::string& bits) {
        clocks = 0;
        for (char tms : std::string(ir ? "1100" : "100")) cycle(tms);
        const std::string serial(bits.rbegin(), bits.rend());
        for (size_t i = 0; i < serial.size(); ++i) {
          cycle(i + 1 == serial.size() ? '1' : '0', serial[i]);
        }
        cycle('1');
        cycle('0');
        Require(clocks == bits.size() + (ir ? 6 : 5),
                "literal unknown input includes final exit-bit shift");
      };
      cycle('0');
      if (!instruction) {
        scheduler.Apply({{"iWrite", {"tap.INSTRUCTION", "3"}, "x:select", {}}});
        scan(true, "0011");
      }
      const std::string bits = instruction ? "x011" : "10x101x0";
      pattern::Operation request{
          "iScan",
          {"jtag", instruction ? "4" : "8", "-si", "0b" + bits},
          "x:raw",
          {}};
      if (instruction)
        request.arguments.insert(request.arguments.begin(), "-ir");
      std::set<size_t> assigned{netlist.nodes.size() + 100};
      std::vector<pattern::ScanStep> preparations(1);
      preparations[0].serial = "sentinel";
      const auto physical =
          scheduler.ApplyScan(request, {}, &assigned, &preparations);
      scan(instruction, bits);
      Require(assigned == std::set<size_t>{id} && preparations.empty() &&
                  scheduler.Value(netlist.nodes[id].name) == bits &&
                  physical.arguments[instruction ? 4 : 3] == "0b" + bits,
              "raw TAP input remains unknown in hardware and emitted packet");
      for (const auto& node : netlist.nodes) {
        if (node.model == pattern::ModelKind::kPort ||
            node.model == pattern::ModelKind::kRegister) {
          Require(scheduler.Value(node.name) == literal.Value(node.name),
                  "unknown raw TAP matches literal original/flat endpoints");
        }
      }
      if (!instruction) {
        scheduler.Apply({{"iWrite", {"core.r[1]", "1"}, "x:partial", {}},
                         {"iRead", {"core.r", "0b10x101x0"}, "x:compare", {}}});
        Require(scheduler.Value("core.r") == "10010110",
                "later partial write retains known-only controlling fill");
      }
      scheduler.Reset();
      if (!instruction) {
        scheduler.Apply({{"iWrite", {"tap.INSTRUCTION", "3"}, "x:select", {}}});
      }
      std::map<std::string, std::string> before;
      for (const auto& node : netlist.nodes) {
        if (node.model == pattern::ModelKind::kPort ||
            node.model == pattern::ModelKind::kRegister) {
          before[node.name] = scheduler.Value(node.name);
        }
      }
      assigned = {netlist.nodes.size() + 100};
      preparations.resize(1);
      preparations[0].serial = "sentinel";
      bool rejected = false;
      try {
        scheduler.ApplyScan(request,
                            {{id, scheduler.Value(netlist.nodes[id].name)}},
                            &assigned, &preparations);
      } catch (const std::runtime_error&) {
        rejected = true;
      }
      Require(rejected &&
                  assigned == std::set<size_t>{netlist.nodes.size() + 100} &&
                  preparations.size() == 1 &&
                  preparations[0].serial == "sentinel",
              "protected unknown assignment preserves both caller outputs");
      for (const auto& [name, value] : before) {
        Require(scheduler.Value(name) == value,
                "failed literal unknown transaction preserves every endpoint");
      }
      request.arguments[instruction ? 4 : 3] = instruction ? "3" : "0x96";
      scheduler.ApplyScan(request, {}, &assigned, &preparations);
      Require(assigned == std::set<size_t>{id} && preparations.empty(),
              "known transaction retries after unknown reservation failure");
    }
  }
}
void TestRawTapUnknownStableRollback() {
  const std::string folder = "tests/cases/ours/pattern_generation/raw_tap_dr/";
  const auto read = [&](const std::string& name) {
    std::ifstream file(folder + name);
    return std::string((std::istreambuf_iterator<char>(file)), {});
  };
  auto model = read("model.icl");
  const std::string from = "  ScanRegister r[7:0] { ScanInSource si;";
  const auto pos = model.find(from);
  Require(pos != std::string::npos,
          "unknown stable fixture replacement exists");
  model.replace(pos, from.size(),
                "  ScanRegister extra {ScanInSource si; CaptureSource extra; "
                "ResetValue 0;}\n"
                "  ScanMux feedback SelectedBy r[0] {1'b0: si; 1'b1: extra;}\n"
                "  ScanRegister r[7:0] { ScanInSource feedback;");
  pattern::Design design;
  design.AddIcl("unknown-stable.icl", model);
  design.AddBsdl("unknown-stable.bsdl", read("model.bsdl"), "RawDrProbe");
  const auto original =
      pattern::ElaborateTapNetwork(design, "RawDrTop").netlist;
  pattern::Design flat;
  flat.AddIcl("unknown-stable-flat.icl", pattern::SerializeIcl(original));
  const auto serialized = pattern::Elaborate(flat, "RawDrTop");
  for (const auto* netlist : {&original, &serialized}) {
    pattern::ScanScheduler scheduler(*netlist);
    scheduler.Reset();
    scheduler.Apply({{"iWrite", {"tap.INSTRUCTION", "3"}, "x:select", {}}});
    std::map<std::string, std::string> before;
    for (const auto& node : netlist->nodes) {
      if (node.model == pattern::ModelKind::kPort ||
          node.model == pattern::ModelKind::kRegister) {
        before[node.name] = scheduler.Value(node.name);
      }
    }
    const auto sentinel = netlist->nodes.size() + 100;
    std::set<size_t> assigned{sentinel};
    std::vector<pattern::ScanStep> preparations(1);
    preparations[0].serial = "sentinel";
    pattern::Operation request{
        "iScan", {"jtag", "8", "-si", "0b0000000x", "-stable"}, "x:stable", {}};
    bool rejected = false;
    try {
      scheduler.ApplyScan(request, {}, &assigned, &preparations);
    } catch (const std::runtime_error& error) {
      rejected = std::string(error.what()).find("stable") != std::string::npos;
    }
    Require(rejected && assigned == std::set<size_t>{sentinel} &&
                preparations.size() == 1 &&
                preparations[0].serial == "sentinel",
            "post-replay unknown stable failure retains caller outputs");
    for (const auto& [name, bits] : before) {
      Require(scheduler.Value(name) == bits,
              "post-replay unknown stable failure rolls back all endpoints");
    }
    request.arguments[3] = "0b10x101x0";
    scheduler.ApplyScan(request, {}, &assigned, &preparations);
    Require(
        scheduler.Value("core.r") == "10x101x0" && preparations.empty() &&
            assigned == std::set<size_t>{netlist->node_ids.at("core.r")},
        "stable retry with a known route bit preserves literal unknown data");
  }
}
void TestMixedChildFallbackPreparation() {
  std::ifstream file(
      "tests/cases/ours/pattern_generation/dynamic_child_raw/model.icl");
  const std::string source((std::istreambuf_iterator<char>(file)), {});
  Require(!source.empty(), "nested preparation fixture exists");
  pattern::Design design;
  design.AddIcl("mixed-child-fallback.icl", source);
  const auto netlist = pattern::Elaborate(design, "Top");
  pattern::ScanScheduler scheduler(netlist);
  scheduler.Reset();
  scheduler.Apply({{"iWrite", {"open", "1"}, "fallback:parent", {}},
                   {"iWrite", {"child.enable", "1"}, "fallback:child", {}}});
  Require(
      scheduler.Value("open") == "1" && scheduler.Value("child.enable") == "1",
      "symbolic preparation establishes both nested SIB goals");
  pattern::ScanScheduler cold(scheduler);
  const pattern::Operation root{"iScan", {"client", "10"}, "fallback:root", {}};
  const auto expected = cold.NormalizeScan(root);
  Require(
      scheduler.NormalizeScan(root).arguments == expected.arguments,
      "successful fallback invalidates the native attempt's pre-route cache");
  scheduler.ApplyScan({"iScan",
                       {"child.local", "9", "-si", "0b101001011", "-stable"},
                       "fallback:child-raw",
                       {}});
  Require(scheduler.Value("open") == "1" &&
              scheduler.Value("child.enable") == "1" &&
              scheduler.Value("child.leaf.value") == "10100101",
          "nested prepared child raw sees the current selected route");
}
void TestMixedChildRawPreparation() {
  pattern::Design design;
  design.AddIcl("mixed-child.icl", RawSibModel());
  const auto netlist = pattern::Elaborate(design, "Top");
  const auto open = netlist.node_ids.at("open");
  const auto value = netlist.node_ids.at("child.value");
  pattern::ScanScheduler scheduler(netlist);
  scheduler.Reset();
  const pattern::Operation raw{"iScan",
                               {"child.local", "8", "-si", "0xa5", "-so", "0"},
                               "mixed-child:raw",
                               {}};
  bool strict_rejected = false;
  try {
    scheduler.NormalizeScan(raw);
  } catch (const std::runtime_error&) {
    strict_rejected = true;
  }
  Require(strict_rejected, "strict child normalization requires active route");
  std::map<std::string, std::string> before;
  for (const auto& node : netlist.nodes) {
    if (node.model == pattern::ModelKind::kPort ||
        node.model == pattern::ModelKind::kRegister) {
      before.emplace(node.name, scheduler.Value(node.name));
    }
  }
  const auto deferred = scheduler.NormalizeScan(raw, true);
  pattern::ScanScheduler cold(scheduler);
  Require(cold.NormalizeScan(raw, true).arguments == deferred.arguments,
          "cold copy admits identical immutable child metadata");
  for (const auto& [name, bits] : before) {
    Require(scheduler.Value(name) == bits && cold.Value(name) == bits,
            "deferred child normalization changes no storage or pin");
  }
  const std::vector<pattern::Operation> preparation{
      {"iWrite", {"open", "1"}, "mixed-child:prepare", {}},
      {"iRead", {"open", "0"}, "mixed-child:observe", {}}};
  scheduler.Apply(preparation);
  auto invalid = raw;
  invalid.arguments = {"child.local", "7"};
  std::set<size_t> assigned{open};
  bool length_rejected = false;
  try {
    scheduler.ApplyScan(invalid, {{open, "1"}}, &assigned);
  } catch (const std::runtime_error& error) {
    length_rejected =
        std::string(error.what()).find("length") != std::string::npos;
  }
  Require(length_rejected && scheduler.Value("open") == "1" &&
              scheduler.Value("child.value") == "00000000" &&
              assigned == std::set<size_t>{open},
          "failed child geometry keeps preparation and assignment output");
  const auto physical = scheduler.ApplyScan(deferred, {{open, "1"}}, &assigned);
  Require(
      physical.arguments[0] == "client" && physical.arguments[1] == "9" &&
          physical.arguments[3] == "0b101001011" &&
          assigned == std::set<size_t>{value},
      "prepared child uses full root CSU with child-only authored footprint");
  cold.Apply(preparation);
  cold.ApplyScan(raw, {{open, "1"}});
  for (const auto& [name, bits] : before) {
    Require(
        scheduler.Value(name) == cold.Value(name),
        "deferred child matches explicit strict boundary on every endpoint");
  }
  Require(scheduler.Value("open") == "1" &&
              scheduler.Value("child.value") == "10100101",
          "child replay updates target and retains held parent SIB");
}
void TestNativeOrdinaryAliases() {
  std::ifstream file(
      "tests/cases/ours/pattern_generation/static_child_scans/model.icl");
  const std::string base((std::istreambuf_iterator<char>(file)), {});
  Require(!base.empty(), "ordinary alias fixture exists");
  for (bool input_inverse : {false, true}) {
    for (bool child_output_inverse : {false, true}) {
      for (bool output_inverse : {false, true}) {
        std::string source = base;
        const auto replace = [&](const std::string& old,
                                 const std::string& next) {
          source.replace(source.find(old), old.size(), next);
        };
        if (input_inverse)
          replace("InputPort si = head[0]", "InputPort si = ~head[0]");
        if (child_output_inverse)
          replace("Source value[0]", "Source ~value[0]");
        if (output_inverse) replace("Source tail[0]", "Source ~tail[0]");
        source.insert(source.find(" ScanInterface client"),
                      " Alias reversed[0:7] = ~child.value;\n");
        pattern::Design design;
        design.AddIcl("ordinary-alias.icl", source);
        const auto netlist = pattern::Elaborate(design, "Top");
        const auto id = [&](const std::string& name) {
          return netlist.node_ids.at(name);
        };
        pattern::ScanScheduler scheduler(netlist);
        scheduler.Reset();
        scheduler.Assume("head", "101");
        scheduler.Assume("child.value", "01010011");
        scheduler.Assume("tail", "10011");
        const std::vector<pattern::Operation> operations{
            {"iWrite", {"reversed[0]", "0"}, "ordinary:alias", {}},
            {"iRead", {"reversed", "0xac"}, "ordinary:compare", {}}};
        if (!input_inverse && !child_output_inverse && !output_inverse) {
          pattern::ScanScheduler blocked(scheduler);
          bool rejected = false;
          try {
            blocked.Apply(operations, false, {{id("child.value"), "0xxxxxxx"}},
                          false);
          } catch (const std::runtime_error& error) {
            rejected =
                std::string(error.what()).find("reserv") != std::string::npos;
          }
          Require(rejected && blocked.Value("child.value") == "01010011",
                  "automatic partial write cannot override protection or adopt "
                  "failed native update");
          blocked.Apply(operations, false, {{id("child.value"), "0xxxxxxx"}});
          Require(blocked.Value("child.value") == "11010011",
                  "explicit known retry after rejected native proof");
        }
        const auto steps = scheduler.Apply(operations, false,
                                           {{id("head"), "101"},
                                            {id("tail"), "10011"},
                                            {id("child.value"), "xxxx0011"}});
        Require(steps.size() == 1 && steps[0].serial.size() == 16 &&
                    steps[0].serial_expected.substr(0, 5) == "xxxxx" &&
                    steps[0].serial_expected.substr(13) == "xxx" &&
                    steps[0].expected.at("child.value") == "01010011",
                "unequal widths and ascending inverted alias preserve exact "
                "ordinary observation scope");
        pattern::NetworkState literal(netlist);
        literal.SetGlobalReset('0', '0');
        literal.Drive(id("reset"), "0");
        for (const auto& [name, bits] :
             std::map<std::string, std::string>{{"head", "101"},
                                                {"child.value", "01010011"},
                                                {"tail", "10011"}}) {
          literal.SetRegister(id(name), bits, bits);
        }
        const auto pin = [&](const std::string& name, char bit) {
          literal.Drive(id(name), std::string(1, bit));
        };
        const auto phase = [&](char capture, char shift, char update,
                               char clock) {
          pin("sel", '1');
          pin("ce", capture);
          pin("se", shift);
          pin("ue", update);
          pin("tck", clock);
          literal.Reset();
        };
        const auto controls = [&](char capture, char shift, char update) {
          std::map<size_t, pattern::ScanControl> result;
          for (const auto& name : {"head", "child.value", "tail"})
            result.emplace(id(name),
                           pattern::ScanControl{capture, shift, update, '1'});
          return result;
        };
        phase('1', '0', '0', '0');
        pin("tck", '1');
        literal.Clock(controls('1', '0', '0'), true);
        for (size_t i = 0; i < steps[0].serial.size(); ++i) {
          const char compare = steps[0].serial_expected[i];
          Require(compare == 'x' || literal.Read({id("so"), 0}) == compare,
                  "expected serial polarity/order matches independently "
                  "captured shifted output");
          phase('0', '1', '0', '0');
          pin("si", steps[0].serial[i]);
          pin("tck", '1');
          literal.Clock(controls('0', '1', '0'), true);
        }
        phase('0', '0', '1', '1');
        pin("tck", '0');
        literal.Clock(controls('0', '0', '1'), false);
        phase('0', '0', '0', '0');
        Require(literal.Value(id("head")) == "101" &&
                    literal.Value(id("tail")) == "10011" &&
                    literal.Value(id("child.value")) == "11010011",
                "native encoded loads meet independent partial-write hardware "
                "goals");
        for (const auto& node : netlist.nodes) {
          if (node.model == pattern::ModelKind::kPort ||
              node.model == pattern::ModelKind::kRegister) {
            Require(scheduler.Value(node.name) == literal.Value(id(node.name)),
                    "ordinary aliases match every literal storage and pin");
          }
        }
      }
    }
  }
}
void TestNativeOrdinaryScans() {
  std::string source = RawSibModel();
  source.replace(source.find("[7:0]"), 5, "[1048574:0]");
  source.replace(source.find("ResetValue 8'b0"), 15, "ResetValue 1048575'b0");
  pattern::Design design;
  design.AddIcl("native-ordinary.icl", source);
  const auto netlist = pattern::Elaborate(design, "Top");
  pattern::ScanScheduler scheduler(netlist);
  scheduler.Reset();
  const auto steps =
      scheduler.Apply({{"iWrite", {"open", "1"}, "native:write", {}},
                       {"iRead", {"open", "0"}, "native:read", {}}});
  Require(steps.size() == 1 && steps[0].serial == "1" &&
              steps[0].serial_expected == "0" &&
              steps[0].writes.at("open") == "1" &&
              steps[0].expected.at("open") == "0",
          "one-bit visible preparation precedes million-bit event probing");
  pattern::NetworkState literal(netlist);
  const auto id = [&](const std::string& name) {
    return netlist.node_ids.at(name);
  };
  const auto pin = [&](const std::string& name, char bit) {
    literal.Drive(id(name), std::string(1, bit));
  };
  pin("reset", '1');
  literal.SetGlobalReset('1', '1');
  literal.Reset();
  pin("reset", '0');
  literal.SetGlobalReset('0', '0');
  literal.Reset();
  const auto phase = [&](char capture, char shift, char update, char clock) {
    pin("sel", '1');
    pin("ce", capture);
    pin("se", shift);
    pin("ue", update);
    pin("tck", clock);
    literal.Reset();
  };
  const auto controls = [&](char capture, char shift, char update) {
    return std::map<size_t, pattern::ScanControl>{
        {id("open"), {capture, shift, update, '1'}},
        {id("child.value"),
         {capture, shift, update, literal.Value(id("open"))[0]}}};
  };
  phase('1', '0', '0', '0');
  pin("tck", '1');
  literal.Clock(controls('1', '0', '0'), true);
  phase('0', '1', '0', '0');
  pin("si", '1');
  pin("tck", '1');
  literal.Clock(controls('0', '1', '0'), true);
  phase('0', '0', '1', '1');
  pin("tck", '0');
  literal.Clock(controls('0', '0', '1'), false);
  phase('0', '0', '0', '0');
  for (const auto& node : netlist.nodes) {
    if (node.model == pattern::ModelKind::kPort ||
        node.model == pattern::ModelKind::kRegister) {
      Require(scheduler.Value(node.name) == literal.Value(id(node.name)),
              "ordinary native preparation matches independent literal clocks "
              "and retained wide storage");
    }
  }
}
void TestMixedRootRawPreparation() {
  pattern::Design design;
  design.AddIcl("mixed-raw.icl", RawSibModel());
  const auto netlist = pattern::Elaborate(design, "Top");
  pattern::ScanScheduler scheduler(netlist);
  scheduler.Reset();
  const pattern::Operation raw{
      "iScan", {"client", "9", "-si", "0b101001011"}, "mixed:raw", {}};
  auto setup = scheduler.NormalizeScan(raw, true);
  Require(setup.arguments[1] == "9" && scheduler.Value("open") == "0" &&
              scheduler.Value("child.value") == "00000000",
          "deferred root setup validates operands without preparing hardware");
  bool rejected = false;
  try {
    scheduler.NormalizeScan(raw);
  } catch (const std::runtime_error& error) {
    rejected = std::string(error.what()).find("length") != std::string::npos;
  }
  Require(rejected, "strict raw setup keeps its current-length validation");
  pattern::ScanScheduler copy(scheduler);
  const auto steps =
      scheduler.Apply({{"iWrite", {"open", "1"}, "mixed:prepare", {}}});
  Require(!steps.empty() && scheduler.Value("open") == "1" &&
              copy.Value("open") == "0",
          "ordinary preparation changes only its own scheduler snapshot");
  scheduler.NormalizeScan(raw);
  rejected = false;
  try {
    copy.NormalizeScan(raw);
  } catch (const std::runtime_error&) {
    rejected = true;
  }
  Require(rejected, "cold copy cannot acquire prepared root geometry");
  rejected = false;
  try {
    scheduler.NormalizeScan(
        {"iScan", {"child.value", "8"}, "mixed:invalid", {}}, true);
  } catch (const std::runtime_error& error) {
    rejected = std::string(error.what()).find("client") != std::string::npos;
  }
  Require(rejected, "deferred preparation requires immutable client metadata");
  rejected = false;
  try {
    scheduler.ApplyScan({"iScan",
                         {"client", "9", "-si", "0b101001010", "-stable"},
                         "mixed:bad-stable",
                         {}});
  } catch (const std::runtime_error& error) {
    rejected = std::string(error.what()).find("stable") != std::string::npos;
  }
  Require(rejected && scheduler.Value("open") == "1" &&
              scheduler.Value("child.value") == "00000000",
          "native raw failure retains separately successful preparation");
  scheduler.ApplyScan(setup);
  Require(scheduler.Value("child.value") == "10100101" &&
              scheduler.Value("open") == "1",
          "raw replay uses the prepared route after a failed trial");
}
void TestRawScanReservations() {
  std::ifstream file(
      "tests/cases/ours/pattern_generation/static_child_scans/model.icl");
  std::string source((std::istreambuf_iterator<char>(file)), {});
  Require(!source.empty(), "raw reservation fixture exists");
  const auto child_output = source.find(" ScanOutPort so { Source value[0]; }");
  Require(child_output != std::string::npos, "raw child output exists");
  source.insert(child_output, " DataOutPort observed { Source value[0]; }\n");
  const auto output = source.find(" ScanOutPort so { Source tail[0]; }");
  Require(output != std::string::npos, "raw reservation output exists");
  source.insert(output,
                " DataOutPort hold { Source 1'b1; }\n"
                " DataOutPort held { Source head[0]; }\n"
                " DataOutPort observed { Source child.observed; }\n");
  for (bool flat : {false, true}) {
    pattern::Design design;
    design.AddIcl("reserved-raw.icl", source);
    const auto netlist = pattern::Elaborate(design, "Top");
    const auto head = netlist.node_ids.at("head");
    const auto child = netlist.node_ids.at("child.value");
    const auto tail = netlist.node_ids.at("tail");
    pattern::ScanScheduler scheduler(netlist);
    scheduler.Reset();
    scheduler.Assume("head", "101");
    scheduler.Assume("child.value", "00001000");
    scheduler.Assume("tail", "10101");
    const std::map<size_t, std::string> preserved{
        {head, "101"}, {child, "xxxx1xxx"}, {tail, "10101"}};
    std::set<size_t> assigned{head, tail};
    const pattern::Operation scan{
        "iScan", {"child.local", "8", "-si", "0xa5"}, "reserve:scan", {}};
    scheduler.ApplyScan(scan, preserved, &assigned);
    Require(assigned == std::set<size_t>{child} &&
                scheduler.Value("head") == "101" &&
                scheduler.Value("tail") == "10101" &&
                scheduler.Value("child.value") == "10100101",
            "explicit child footprint and preserved surrounding loads");
    pattern::ResourceReservations resources(netlist);
    resources.Change({"iTake", {"child.value[3]"}, "reserve:take", {}});
    resources.Assigned(
        {{"iState", {"child.value[3]", "0"}, "reserve:state", {}}}, scheduler);
    scheduler.ApplyScan(
        {"iScan", {"child.local", "8", "-si", "0x3c"}, "reserve:replace", {}},
        resources.Values(), &assigned);
    resources.AssignedScans(assigned, scheduler);
    Require(resources.Values().at(child) == "xxxx1xxx",
            "raw assignment refreshes only established taken bits");
    for (const auto& invalid : std::vector<std::map<size_t, std::string>>{
             {{head, "01"}},
             {{head, "q01"}},
             {{netlist.node_ids.at("sel"), "0"}},
             {{netlist.node_ids.at("tck"), "0"}},
             {{netlist.node_ids.at("observed"), "0"}},
             {{head, "000"}}}) {
      assigned = {tail};
      bool rejected = false;
      try {
        scheduler.ApplyScan(scan, invalid, &assigned);
      } catch (const std::runtime_error& error) {
        if (invalid.count(netlist.node_ids.at("tck"))) {
          Require(std::string(error.what()).find("reserved resource tck") !=
                      std::string::npos,
                  "a child clock lock rejects the actual rising edge");
        }
        if (invalid.count(netlist.node_ids.at("observed"))) {
          Require(
              std::string(error.what()).find("reserved resource observed") !=
                  std::string::npos,
              "a child output lock rejects the actual Update");
        }
        rejected = true;
      }
      Require(rejected && assigned == std::set<size_t>{tail} &&
                  scheduler.Value("head") == "101" &&
                  scheduler.Value("child.value") == "00111100",
              "invalid masks, port ownership and wrong initial protection "
              "reject atomically");
    }
    std::map<std::string, std::string> before;
    for (const auto& node : netlist.nodes) {
      if (node.model == pattern::ModelKind::kPort ||
          node.model == pattern::ModelKind::kRegister) {
        before.emplace(node.name, scheduler.Value(node.name));
      }
    }
    assigned = {head, tail};
    bool rejected = false;
    try {
      scheduler.ApplyScan({"iScan",
                           {"child.local", "8", "-si", "0bxxxxxx01"},
                           "reserve:unknown",
                           {}},
                          resources.Values(), &assigned);
    } catch (const std::runtime_error& error) {
      rejected =
          std::string(error.what()).find("reserved") != std::string::npos;
    }
    Require(rejected && assigned == std::set<size_t>{head, tail},
            "protected X leaves caller footprint unchanged");
    for (const auto& [name, value] : before) {
      Require(scheduler.Value(name) == value,
              "failed raw preservation is atomic");
    }
    scheduler.ApplyScan(scan, resources.Values(), &assigned);
    resources.AssignedScans(assigned, scheduler);
    Require(resources.Values().at(child) == "xxxx0xxx",
            "known retry refreshes history after failed unknown assignment");
    scheduler.ApplyScan(
        {"iScan", {"client", "16", "-si", "0"}, "reserve:root", {}}, {},
        &assigned);
    Require(assigned == std::set<size_t>{head, child, tail},
            "root scan explicitly addresses all pre-route storage");
    pattern::ResourceReservations ports(netlist);
    scheduler.Assume("sel", "0");
    for (const auto& name : {"sel", "held", "hold"}) {
      ports.Change({"iTake", {name}, "reserve:port-take", {}});
    }
    ports.Assigned({{"iState", {"sel", "0"}, "reserve:port-state", {}},
                    {"iWrite", {"held", "0"}, "reserve:port-state", {}},
                    {"iWrite", {"hold", "1"}, "reserve:port-state", {}}},
                   scheduler);
    const auto locks = ports.Values();
    const pattern::Operation root{
        "iScan", {"client", "16", "-si", "0x2000"}, "reserve:root-ports", {}};
    scheduler.ApplyScan(root, locks, &assigned);
    ports.AssignedScans(assigned, scheduler);
    Require(assigned == std::set<size_t>{head, child, tail} &&
                scheduler.Value("sel") == "1" &&
                scheduler.Value("held") == "1" && ports.Values() == locks,
            "explicit root port effects retain original locks and storage-only "
            "assignment footprint");
    std::map<std::string, std::string> port_before;
    for (const auto& node : netlist.nodes) {
      if (node.model == pattern::ModelKind::kPort ||
          node.model == pattern::ModelKind::kRegister) {
        port_before.emplace(node.name, scheduler.Value(node.name));
      }
    }
    for (const auto& mask : {"00", "q"}) {
      assigned = {tail};
      bool invalid = false;
      try {
        scheduler.ApplyScan(root, {{netlist.node_ids.at("sel"), mask}},
                            &assigned);
      } catch (const std::runtime_error& error) {
        invalid = std::string(error.what()).find("invalid reserved") !=
                  std::string::npos;
      }
      Require(invalid && assigned == std::set<size_t>{tail},
              "root exemption validates port masks before filtering");
    }
    bool ordinary_rejected = false;
    try {
      scheduler.Apply({{"iWrite", {"head", "2"}, "reserve:later", {}}}, false,
                      ports.Values());
    } catch (const std::runtime_error&) {
      ordinary_rejected = true;
    }
    Require(ordinary_rejected,
            "old port locks still constrain later retargeting");
    for (const auto& [name, value] : port_before) {
      Require(
          scheduler.Value(name) == value,
          "invalid root mask and later ownership failure leave state intact");
    }
    for (const auto& name : {"sel", "held", "hold"}) {
      ports.Change({"iRelease", {name}, "reserve:port-release", {}});
    }
    scheduler.Apply({{"iWrite", {"head", "2"}, "reserve:recovered", {}}}, false,
                    ports.Values());
    Require(scheduler.Value("head") == "010",
            "explicit port release permits later ordinary retargeting");
    for (bool output_wave : {false, true}) {
      for (const std::string& name :
           output_wave ? std::vector<std::string>{"so", "child.so"}
                       : std::vector<std::string>{"si", "child.si"}) {
        pattern::ScanScheduler trial(netlist);
        trial.Reset();
        trial.ApplyScan({"iScan",
                         {"client", "16", "-si", output_wave ? "0x0b40" : "0"},
                         "reserve:preload",
                         {}});
        const auto old_head = trial.Value("head");
        const auto old_child = trial.Value("child.value");
        const auto old_tail = trial.Value("tail");
        Require(trial.Value(name) == "0", "serial lock starts at zero");
        const pattern::Operation waveform{
            "iScan",
            {"child.local", "8", "-si", output_wave ? "0" : "0x5a", "-so",
             output_wave ? "0x5a" : "0"},
            "reserve:mid-shift",
            {}};
        assigned = {tail};
        bool conflict = false;
        try {
          trial.ApplyScan(waveform, {{netlist.node_ids.at(name), "0"}},
                          &assigned);
        } catch (const std::runtime_error& error) {
          conflict =
              std::string(error.what()).find("reserved resource " + name) !=
              std::string::npos;
        }
        Require(conflict && assigned == std::set<size_t>{tail} &&
                    trial.Value("head") == old_head &&
                    trial.Value("child.value") == old_child &&
                    trial.Value("tail") == old_tail && trial.Value(name) == "0",
                "intermediate serial port changes reject atomically");
      }
    }
    for (bool output_lock : {false, true}) {
      for (const std::string& name :
           output_lock ? std::vector<std::string>{"so", "child.so"}
                       : std::vector<std::string>{"si", "child.si"}) {
        pattern::ScanScheduler trial(netlist);
        trial.Reset();
        trial.ApplyScan({"iScan",
                         {"client", "16", "-si", output_lock ? "0" : "0x0b40"},
                         "",
                         {}});
        trial.ApplyScan(
            {"iScan",
             {"child.local", "8", "-si", output_lock ? "0x5a" : "0"},
             "",
             {}},
            {{netlist.node_ids.at(name), "0"}});
        Require(
            trial.Value(name) == "0",
            "a serial port whose full waveform is constant stays compatible");
      }
    }
    if (!flat) source = pattern::SerializeIcl(netlist);
  }
  pattern::Design design;
  design.AddIcl("reserved-sib.icl", RawSibModel());
  const auto netlist = pattern::Elaborate(design, "Top");
  pattern::ScanScheduler scheduler(netlist);
  scheduler.Reset();
  std::set<size_t> assigned;
  scheduler.ApplyScan(
      {"iScan", {"client", "1", "-si", "1"}, "reserve:opening", {}}, {},
      &assigned);
  Require(
      assigned == std::set<size_t>{netlist.node_ids.at("open")},
      "newly enabled post-route storage is absent from assignment footprint");
}
void TestRawPortReservationScale() {
  std::ifstream file(
      "tests/cases/ours/pattern_generation/static_child_scans/model.icl");
  std::string source((std::istreambuf_iterator<char>(file)), {});
  const size_t width = 65536;
  const auto replace = [&](const std::string& from, const std::string& to) {
    const auto position = source.find(from);
    Require(position != std::string::npos, "wide port reservation fixture");
    source.replace(position, from.size(), to);
  };
  replace("value[7:0]", "value[" + std::to_string(width - 1) + ":0]");
  replace("ResetValue 8'b0", "ResetValue " + std::to_string(width) + "'b0");
  replace("DefaultLoadValue 16'b0",
          "DefaultLoadValue " + std::to_string(width + 8) + "'b0");
  pattern::Design design;
  design.AddIcl("wide-port-reservation.icl", source);
  const auto netlist = pattern::Elaborate(design, "Top");
  pattern::ScanScheduler initial(netlist);
  initial.Reset();
  initial.ApplyScan({"iScan",
                     {"client", std::to_string(width + 8), "-si", "0"},
                     "reserve:wide-preload",
                     {}});
  const auto start = std::chrono::steady_clock::now();
  for (const std::string& name :
       std::vector<std::string>{"sel", "si", "so", "child.si", "child.so"}) {
    pattern::ScanScheduler trial(initial);
    trial.ApplyScan({"iScan",
                     {"child.local", std::to_string(width), "-si", "0"},
                     "reserve:wide-port",
                     {}},
                    {{netlist.node_ids.at(name), name == "sel" ? "1" : "0"}});
    Require(trial.Value("child.value") == std::string(width, '0'),
            "constant wide port proof preserves complete scan loads");
  }
  std::cout << "raw protected ports: bits=" << width << " ports=5 seconds="
            << std::chrono::duration<double>(std::chrono::steady_clock::now() -
                                             start)
                   .count()
            << '\n';
}
void TestGatedSibRawScans() {
  std::string source = RawSibModel();
  for (bool flat : {false, true}) {
    pattern::Design design;
    design.AddIcl("gated-sib-raw.icl", source);
    const auto netlist = pattern::Elaborate(design, "Top");
    const auto id = [&](const std::string& name) {
      return netlist.node_ids.at(name);
    };
    const auto sib = id("open"), child = id("child.value");
    pattern::ScanScheduler scheduler(netlist);
    scheduler.Reset();
    std::map<std::string, std::string> before;
    for (const auto& node : netlist.nodes) {
      if (node.model == pattern::ModelKind::kPort ||
          node.model == pattern::ModelKind::kRegister) {
        before.emplace(node.name, scheduler.Value(node.name));
      }
    }
    bool rejected = false;
    try {
      scheduler.ApplyScan({"iScan",
                           {"client", "1", "-si", "1", "-stable"},
                           "sib:bad-stable",
                           {}});
    } catch (const std::runtime_error& error) {
      rejected = std::string(error.what()).find("stable") != std::string::npos;
    }
    Require(rejected, "opening a longer route cannot claim stable");
    for (const auto& [name, value] : before) {
      Require(scheduler.Value(name) == value,
              "failed stable scan preserves held, shift-observable and driven "
              "pin state");
    }
    pattern::NetworkState literal(netlist);
    const auto pin = [&](const std::string& name, char bit) {
      literal.Drive(id(name), std::string(1, bit));
    };
    pin("reset", '1');
    literal.SetGlobalReset('1', '1');
    literal.Reset();
    pin("reset", '0');
    literal.SetGlobalReset('0', '0');
    literal.Reset();
    // The optional reader must preserve native interface control algebra.
    pattern::NetworkState probe(literal);
    probe.Assume(sib, "1");
    probe.Drive(id("ce"), "1");
    probe.Drive(id("se"), "0");
    probe.Drive(id("ue"), "0");
    const std::vector<pattern::TapExecution::InterfacePorts> interfaces{
        {{"SelectPort", {{id("child.sel"), 0}}},
         {"CaptureEnPort", {{id("child.ce"), 0}}},
         {"ShiftEnPort", {{id("child.se"), 0}}},
         {"UpdateEnPort", {{id("child.ue"), 0}}}}};
    size_t reads = 0;
    const auto observed = pattern::EffectiveScanControl(
        netlist, probe, interfaces, {}, {}, [&](const auto& bit) {
          ++reads;
          return probe.Read(bit);
        });
    const auto native =
        pattern::EffectiveScanControl(netlist, probe, interfaces, {}, {});
    Require(reads == 4 && observed.capture == '1' &&
                observed.capture == native.capture &&
                observed.shift == native.shift &&
                observed.update == native.update &&
                observed.select == native.select,
            "scoped signal reader preserves native scan controls");
    const auto phase = [&](char capture, char shift, char update, char clock) {
      pin("sel", '1');
      pin("ce", capture);
      pin("se", shift);
      pin("ue", update);
      pin("tck", clock);
      literal.Reset();
    };
    const auto controls = [&](char capture, char shift, char update) {
      return std::map<size_t, pattern::ScanControl>{
          {sib, {capture, shift, update, '1'}},
          {child, {capture, shift, update, literal.Value(sib)[0]}}};
    };
    for (const auto& payload : {std::string("1"), std::string("101001011"),
                                std::string("001111000"), std::string("1")}) {
      const auto old_child = literal.Value(child);
      const auto old_sib = literal.Value(sib);
      phase('1', '0', '0', '0');
      pin("tck", '1');
      literal.Clock(controls('1', '0', '0'), true);
      for (auto bit = payload.rbegin(); bit != payload.rend(); ++bit) {
        phase('0', '1', '0', '0');
        pin("si", *bit);
        pin("tck", '1');
        literal.Clock(controls('0', '1', '0'), true);
      }
      phase('0', '0', '1', '1');
      pin("tck", '0');
      literal.Clock(controls('0', '0', '1'), false);
      phase('0', '0', '0', '0');
      scheduler.ApplyScan(
          {"iScan",
           {"client", std::to_string(payload.size()), "-si", "0b" + payload},
           "sib:scan",
           {}});
      for (const auto& node : netlist.nodes) {
        if (node.model == pattern::ModelKind::kPort ||
            node.model == pattern::ModelKind::kRegister) {
          Require(scheduler.Value(node.name) == literal.Value(id(node.name)),
                  "gated SIB bulk matches every literal register and "
                  "scan/control pin");
        }
      }
      if (old_sib == "0") {
        Require(scheduler.Value("child.value") == old_child,
                "opening uses pre-update inactive child eligibility");
      }
    }
    Require(
        scheduler.Value("child.value") == "00111100",
        "closing updates the previously active child and reopening retains it");
    scheduler.ApplyScan({"iScan",
                         {"client", "9", "-si", "0b101001011", "-stable"},
                         "sib:known",
                         {}});
    rejected = false;
    try {
      scheduler.ApplyScan({"iScan",
                           {"client", "9", "-si", "0", "-stable"},
                           "sib:bad-close",
                           {}});
    } catch (const std::runtime_error& error) {
      rejected = std::string(error.what()).find("stable") != std::string::npos;
    }
    Require(rejected && scheduler.Value("open") == "1" &&
                scheduler.Value("child.value") == "10100101",
            "failed closing-stable scan rolls back target and selector");
    scheduler.ApplyScan({"iScan",
                         {"client", "9", "-si", "0b10x01x011", "-stable"},
                         "sib:partial",
                         {}});
    scheduler.Apply({{"iWrite", {"child.value[1]", "1"}, "sib:write", {}}});
    Require(scheduler.Value("child.value") == "10101111",
            "failed stable trial cannot pollute partial-X controlling fill");
    auto bad_reset = netlist;
    bad_reset.nodes.at(child).reset->inverted =
        !bad_reset.nodes.at(child).reset->inverted;
    pattern::ScanScheduler inactive_reset(bad_reset);
    inactive_reset.Reset();
    rejected = false;
    try {
      inactive_reset.ApplyScan(
          {"iScan", {"client", "1", "-si", "0"}, "sib:inactive-reset", {}});
    } catch (const std::runtime_error& error) {
      rejected = std::string(error.what()).find("reset") != std::string::npos;
    }
    Require(rejected,
            "inactive child reset must be inactive before raw replay");
    if (!flat) source = pattern::SerializeIcl(netlist);
  }
}
std::string DynamicSibModel() {
  return R"ICL(Module DynamicRawLeaf {
 ScanInPort si; SelectPort sel; CaptureEnPort ce; ShiftEnPort se;
 UpdateEnPort ue; TCKPort tck; ResetPort reset;
 ScanRegister value[7:0] { ScanInSource si; CaptureSource value; ResetValue 8'b0; }
 ScanOutPort so { Source value[0]; }
 ScanInterface local { Port si; Port so; Port sel; Port ce; Port se; Port ue; Port tck; Port reset; }
}
Module DynamicRawBranch {
 ScanInPort si; SelectPort sel; CaptureEnPort ce; ShiftEnPort se;
 UpdateEnPort ue; TCKPort tck; ResetPort reset;
 Instance leaf Of DynamicRawLeaf {
  InputPort si = si; InputPort sel = enable; InputPort ce = ce;
  InputPort se = se; InputPort ue = ue; InputPort tck = tck; InputPort reset = reset;
 }
 ScanMux inner_route SelectedBy enable { 1'b0: si; 1'b1: leaf.so; }
 ScanRegister enable { ScanInSource inner_route; CaptureSource enable; ResetValue 1'b0; }
 ScanOutPort so { Source enable; }
 ScanInterface local {
  Port si; Port so; Port sel; Port ce; Port se; Port ue; Port tck; Port reset;
  DefaultLoadValue 1'b0;
 }
}
Module Top {
 ScanInPort si; SelectPort sel; CaptureEnPort ce; ShiftEnPort se;
 UpdateEnPort ue; TCKPort tck; ResetPort reset;
 Instance child Of DynamicRawBranch {
  InputPort si = si; InputPort sel = open; InputPort ce = ce;
  InputPort se = se; InputPort ue = ue; InputPort tck = tck; InputPort reset = reset;
 }
 ScanMux route SelectedBy open { 1'b0: si; 1'b1: child.so; }
 ScanRegister open { ScanInSource route; CaptureSource open; ResetValue 1'b0; }
 ScanOutPort so { Source open; }
 ScanInterface client {
  Port si; Port so; Port sel; Port ce; Port se; Port ue; Port tck; Port reset;
  DefaultLoadValue 1'b0;
 }
}
)ICL";
}
void TestDynamicChildRawScans() {
  std::string source = DynamicSibModel();
  for (bool flat : {false, true}) {
    pattern::Design design;
    design.AddIcl("dynamic-child-raw.icl", source);
    const auto netlist = pattern::Elaborate(design, "Top");
    const auto id = [&](const std::string& name) {
      return netlist.node_ids.at(name);
    };
    const auto outer = id("open"), inner = id("child.enable"),
               leaf = id("child.leaf.value");
    pattern::ScanScheduler scheduler(netlist);
    scheduler.Reset();
    scheduler.ApplyScan(
        {"iScan", {"client", "1", "-si", "1"}, "dynamic:outer", {}});
    std::map<std::string, std::string> before;
    for (const auto& node : netlist.nodes) {
      if (node.model == pattern::ModelKind::kPort ||
          node.model == pattern::ModelKind::kRegister) {
        before.emplace(node.name, scheduler.Value(node.name));
      }
    }
    bool rejected = false;
    try {
      scheduler.ApplyScan({"iScan",
                           {"child.local", "1", "-si", "1", "-stable"},
                           "dynamic:bad-open",
                           {}});
    } catch (const std::runtime_error& error) {
      rejected = std::string(error.what()).find("stable") != std::string::npos;
    }
    Require(rejected, "changing local binary child length cannot claim stable");
    for (const auto& [name, value] : before) {
      Require(scheduler.Value(name) == value,
              "failed local stable leaves held/shift/pin state unchanged");
    }
    pattern::NetworkState literal(netlist);
    const auto pin = [&](const std::string& name, char bit) {
      literal.Drive(id(name), std::string(1, bit));
    };
    pin("reset", '1');
    literal.SetGlobalReset('1', '1');
    literal.Reset();
    pin("reset", '0');
    literal.SetGlobalReset('0', '0');
    literal.Reset();
    const auto phase = [&](char capture, char shift, char update, char clock) {
      pin("sel", '1');
      pin("ce", capture);
      pin("se", shift);
      pin("ue", update);
      pin("tck", clock);
      literal.Reset();
    };
    const auto controls = [&](char capture, char shift, char update) {
      return std::map<size_t, pattern::ScanControl>{
          {outer, {capture, shift, update, '1'}},
          {inner, {capture, shift, update, literal.Value(outer)[0]}},
          {leaf, {capture, shift, update, literal.Value(inner)[0]}}};
    };
    const auto scan = [&](const std::string& payload) {
      phase('1', '0', '0', '0');
      pin("tck", '1');
      literal.Clock(controls('1', '0', '0'), true);
      for (auto bit = payload.rbegin(); bit != payload.rend(); ++bit) {
        phase('0', '1', '0', '0');
        pin("si", *bit);
        pin("tck", '1');
        literal.Clock(controls('0', '1', '0'), true);
      }
      phase('0', '0', '1', '1');
      pin("tck", '0');
      literal.Clock(controls('0', '0', '1'), false);
      phase('0', '0', '0', '0');
    };
    scan("1");
    const std::vector<pattern::Operation> operations{
        {"iScan", {"child.local", "1", "-si", "1"}, "dynamic:open", {}},
        {"iScan",
         {"child.local", "9", "-si", "0b101001011", "-stable"},
         "dynamic:load",
         {}},
        {"iScan",
         {"child.local", "9", "-si", "0b001111000"},
         "dynamic:close",
         {}},
        {"iScan", {"child.local", "1", "-si", "1"}, "dynamic:reopen", {}}};
    size_t index = 0;
    for (const auto& payload : {std::string("11"), std::string("1010010111"),
                                std::string("0011110001"), std::string("11")}) {
      scan(payload);
      const auto result = scheduler.ApplyScan(operations[index++]);
      Require(result.arguments[1] == std::to_string(payload.size()) &&
                  result.arguments[3] == "0b" + payload,
              "dynamic child emits the literal root CSU geometry and SI");
      for (const auto& node : netlist.nodes) {
        if (node.model == pattern::ModelKind::kPort ||
            node.model == pattern::ModelKind::kRegister) {
          Require(scheduler.Value(node.name) == literal.Value(id(node.name)),
                  "dynamic child bulk matches literal held/shift/control pins");
        }
      }
    }
    Require(scheduler.Value("child.leaf.value") == "00111100",
            "inner closing updates leaf and reopening preserves it");
    scheduler.ApplyScan({"iScan",
                         {"child.local", "9", "-si", "0b101001011", "-stable"},
                         "dynamic:known",
                         {}});
    rejected = false;
    try {
      scheduler.ApplyScan({"iScan",
                           {"child.local", "9", "-si", "0", "-stable"},
                           "dynamic:bad-close",
                           {}});
    } catch (const std::runtime_error& error) {
      rejected = std::string(error.what()).find("stable") != std::string::npos;
    }
    Require(rejected && scheduler.Value("child.enable") == "1" &&
                scheduler.Value("child.leaf.value") == "10100101",
            "failed local stable closing preserves selectors and leaf data");
    scheduler.ApplyScan({"iScan",
                         {"child.local", "9", "-si", "0b10x01x011", "-stable"},
                         "dynamic:partial",
                         {}});
    scheduler.Apply(
        {{"iWrite", {"child.leaf.value[1]", "1"}, "dynamic:fill", {}}});
    Require(scheduler.Value("child.leaf.value") == "10101111",
            "failed local trial cannot poison partial-X controlling fill");
    if (!flat) source = pattern::SerializeIcl(netlist);
  }
}
void TestActiveChildRawScans() {
  std::string source = RawSibModel();
  for (bool flat : {false, true}) {
    pattern::Design design;
    design.AddIcl("active-child-raw.icl", source);
    const auto netlist = pattern::Elaborate(design, "Top");
    pattern::ScanScheduler child(netlist), physical(netlist);
    child.Reset();
    physical.Reset();
    const pattern::Operation open{
        "iScan", {"client", "1", "-si", "1"}, "child:open", {}};
    child.ApplyScan(open);
    physical.ApplyScan(open);
    const pattern::Operation known{
        "iScan",
        {"client", "9", "-si", "0b101001011", "-stable"},
        "child:known",
        {}};
    child.ApplyScan(known);
    physical.ApplyScan(known);
    const auto actual = child.ApplyScan(
        {"iScan",
         {"child.local", "8", "-si", "0b10x01x01", "-so", "0x3c", "-stable"},
         "child:partial",
         {}});
    const auto expected = physical.ApplyScan(
        {"iScan",
         {"client", "9", "-si", "0b10x01x011", "-so", "0b00111100x", "-stable"},
         "child:physical",
         {}});
    Require(actual.arguments == expected.arguments,
            "child raw SI/SO retarget to the same physical root CSU");
    child.Apply({{"iWrite", {"child.value[1]", "1"}, "child:fill", {}}});
    physical.Apply({{"iWrite", {"child.value[1]", "1"}, "child:fill", {}}});
    Require(child.Value("child.value") == "10101111",
            "child partial-X preserves surrounding controlling fill");
    const auto same = [&](const pattern::ScanScheduler& left,
                          const pattern::ScanScheduler& right) {
      for (const auto& node : netlist.nodes) {
        if (node.model == pattern::ModelKind::kPort ||
            node.model == pattern::ModelKind::kRegister) {
          Require(left.Value(node.name) == right.Value(node.name),
                  "child retarget matches physical held/shift/control state");
        }
      }
    };
    same(child, physical);
    const pattern::Operation setup{
        "iScan",
        {"child.local", "8", "-si", "0x3c", "-stable"},
        "child:setup",
        {}};
    child.NormalizeScan(setup);
    child.Assume("open", "0");
    bool rejected = false;
    try {
      child.NormalizeScan(setup);
    } catch (const std::runtime_error&) {
      rejected = true;
    }
    Require(rejected, "Assume invalidates accessible-child route cache");
    child.Assume("open", "1");
    child.NormalizeScan(setup);
    pattern::ScanScheduler copied(child);
    child.Assume("open", "0");
    Require(copied.NormalizeScan(setup).arguments[1] == "8",
            "warm-cache copy retains its own accessible route after original "
            "changes");
    child.Assume("open", "1");
    child.Idle(1);
    physical.Idle(1);
    copied.Idle(1);
    copied.ApplyScan(setup);
    child.ApplyScan(setup);
    physical.ApplyScan({"iScan",
                        {"client", "9", "-si", "0b001111001", "-stable"},
                        "child:physical",
                        {}});
    same(child, physical);
    same(copied, child);
    child.NormalizeScan(setup);
    child.Apply({{"iWrite", {"open", "0"}, "child:close", {}}});
    rejected = false;
    try {
      child.NormalizeScan(setup);
    } catch (const std::runtime_error&) {
      rejected = true;
    }
    Require(rejected, "ordinary Apply invalidates a warm child interval cache");
    child.Apply({{"iWrite", {"open", "1"}, "child:reopen", {}}});
    child.NormalizeScan(setup);
    child.Reset();
    rejected = false;
    try {
      child.NormalizeScan(setup);
    } catch (const std::runtime_error&) {
      rejected = true;
    }
    Require(rejected,
            "Reset invalidates child slices and dynamic ordered root profile");
    Require(child.NormalizeScan(open).arguments[1] == "1",
            "reset returns raw root width to closed SIB length");
    if (!flat) source = pattern::SerializeIcl(netlist);
  }
  // Equal widths cannot establish that a cached child remains accessible.
  source = RawSibModel();
  const auto start = source.find(" Instance child Of");
  const auto end = source.find(" ScanMux route");
  auto other = source.substr(start, end - start);
  other.replace(other.find("Instance child"),
                std::string("Instance child").size(), "Instance other");
  other.replace(other.find("InputPort sel = open;"),
                std::string("InputPort sel = open;").size(),
                "InputPort sel = ~open;");
  source.insert(end, other);
  source.replace(source.find("1'b0: si;"), std::string("1'b0: si;").size(),
                 "1'b0: other.so;");
  pattern::Design design;
  design.AddIcl("same-width-child.icl", source);
  const auto netlist = pattern::Elaborate(design, "Top");
  pattern::ScanScheduler switched(netlist);
  switched.Reset();
  const pattern::Operation old_child{
      "iScan", {"other.local", "8"}, "child:old", {}};
  switched.NormalizeScan(old_child);
  switched.Assume("open", "1");
  bool rejected = false;
  try {
    switched.NormalizeScan(old_child);
  } catch (const std::runtime_error&) {
    rejected = true;
  }
  Require(rejected,
          "same-width route switch invalidates the old child interval");
  Require(
      switched.NormalizeScan({"iScan", {"child.local", "8"}, "child:new", {}})
              .arguments[1] == "8",
      "same-width route switch exposes the new child interval");
}
void TestStaticChildScans() {
  const std::string original = R"ICL(Module ChildLeaf {
 ScanInPort si; SelectPort sel; CaptureEnPort ce; ShiftEnPort se;
 UpdateEnPort ue; TCKPort tck; ResetPort reset;
 ScanRegister value[7:0] { ScanInSource si; CaptureSource value; ResetValue 8'b0; }
 ScanOutPort so { Source value[0]; }
 ScanInterface local { Port si; Port so; Port sel; Port ce; Port se; Port ue; Port tck; Port reset; }
}
Module Top {
 ScanInPort si; SelectPort sel; CaptureEnPort ce; ShiftEnPort se;
 UpdateEnPort ue; TCKPort tck; ResetPort reset;
 ScanRegister head[2:0] { ScanInSource si; CaptureSource head; ResetValue 3'b0; }
 Instance child Of ChildLeaf {
  InputPort si = head[0]; InputPort sel = sel; InputPort ce = ce;
  InputPort se = se; InputPort ue = ue; InputPort tck = tck; InputPort reset = reset;
 }
 ScanRegister tail[4:0] { ScanInSource child.so; CaptureSource tail; ResetValue 5'b0; }
 ScanOutPort so { Source tail[0]; }
 ScanInterface client {
  Port si; Port so; Port sel; Port ce; Port se; Port ue; Port tck; Port reset;
  DefaultLoadValue 16'b0;
 }
}
)ICL";
  const auto invert = [](std::string bits, bool inverse) {
    if (inverse) {
      for (char& bit : bits) {
        if (bit != 'x') bit = bit == '1' ? '0' : '1';
      }
    }
    return bits;
  };
  for (bool input_inverse : {false, true}) {
    for (bool child_output_inverse : {false, true}) {
      for (bool root_output_inverse : {false, true}) {
        std::string source = original;
        const auto replace = [&](const std::string& from,
                                 const std::string& to) {
          source.replace(source.find(from), from.size(), to);
        };
        if (input_inverse) replace("= head[0]", "= ~head[0]");
        if (child_output_inverse) {
          replace("Source value[0]", "Source ~value[0]");
        }
        if (root_output_inverse) replace("Source tail[0]", "Source ~tail[0]");
        for (bool flat : {false, true}) {
          pattern::Design design;
          design.AddIcl("static-child-scans.icl", source);
          const auto netlist = pattern::Elaborate(design, "Top");
          const auto id = [&](const std::string& name) {
            return netlist.node_ids.at(name);
          };
          const auto head = id("head"), child = id("child.value"),
                     tail = id("tail");
          const auto tail_input =
              invert("10011", input_inverse != child_output_inverse);
          const auto input =
              "101" + invert("10100101", input_inverse) + tail_input;
          pattern::ScanScheduler scheduler(netlist);
          scheduler.Reset();
          scheduler.ApplyScan(
              {"iScan",
               {"client", "16", "-si",
                "0b101" + invert("00000000", input_inverse) + tail_input},
               "child:initial",
               {}});
          pattern::NetworkState literal(netlist);
          literal.SetRegister(head, "101", "101");
          literal.SetRegister(child, "00000000", "00000000");
          literal.SetRegister(tail, "10011", "10011");
          literal.SetGlobalReset('0', '0');
          const auto pin = [&](const std::string& name, char bit) {
            literal.Drive(id(name), std::string(1, bit));
          };
          pin("reset", '0');
          const auto phase = [&](char capture, char shift, char update,
                                 char clock) {
            pin("sel", '1');
            pin("ce", capture);
            pin("se", shift);
            pin("ue", update);
            pin("tck", clock);
            literal.Reset();
          };
          const auto controls = [&](char capture, char shift, char update) {
            std::map<size_t, pattern::ScanControl> result;
            for (size_t reg : {head, child, tail}) {
              result.emplace(reg,
                             pattern::ScanControl{capture, shift, update, '1'});
            }
            return result;
          };
          phase('1', '0', '0', '0');
          pin("tck", '1');
          literal.Clock(controls('1', '0', '0'), true);
          for (auto bit = input.rbegin(); bit != input.rend(); ++bit) {
            phase('0', '1', '0', '0');
            pin("si", *bit);
            pin("tck", '1');
            literal.Clock(controls('0', '1', '0'), true);
          }
          phase('0', '0', '1', '1');
          pin("tck", '0');
          literal.Clock(controls('0', '0', '1'), false);
          phase('0', '0', '0', '0');
          const auto result = scheduler.ApplyScan(
              {"iScan",
               {"child.local", "8", "-si", "0xa5", "-so", "0x3c"},
               "child:scan",
               {}});
          Require(result.arguments[0] == "client" &&
                      result.arguments[1] == "16" &&
                      result.arguments[3] == "0b" + input &&
                      result.arguments[5] ==
                          "0bxxx" + invert("00111100", root_output_inverse) +
                              "xxxxx",
                  "child-local load and expected comparison use separate "
                  "physical root parities and offsets");
          for (const auto& node : netlist.nodes) {
            if (node.model == pattern::ModelKind::kPort ||
                node.model == pattern::ModelKind::kRegister) {
              Require(
                  scheduler.Value(node.name) == literal.Value(id(node.name)),
                  "child bulk replay matches literal full-root shifts and "
                  "bound pins");
            }
          }
          scheduler.Idle(7);
          for (size_t cycle = 0; cycle < 7; ++cycle) {
            pin("tck", '1');
            literal.Clock(controls('0', '0', '0'), true);
            pin("tck", '0');
            literal.Clock(controls('0', '0', '0'), false);
          }
          for (const auto& node : netlist.nodes) {
            if (node.model == pattern::ModelKind::kPort ||
                node.model == pattern::ModelKind::kRegister) {
              Require(
                  scheduler.Value(node.name) == literal.Value(id(node.name)),
                  "raw child scan followed by native idle matches literal "
                  "edges and pin state");
            }
          }
          bool rejected = false;
          try {
            scheduler.ApplyScan(
                {"iScan", {"child.local", "7"}, "child:bad", {}});
          } catch (const std::runtime_error&) {
            rejected = true;
          }
          Require(rejected && scheduler.Value("head") == "101" &&
                      scheduler.Value("child.value") == "10100101" &&
                      scheduler.Value("tail") == "10011",
                  "failed child scan preserves actual surrounding and target "
                  "state");
          if (!flat) source = pattern::SerializeIcl(netlist);
        }
      }
    }
  }
}
void TestHierarchicalRootScans() {
  std::string source = R"ICL(Module Leaf {
 ScanInPort si; SelectPort sel; CaptureEnPort ce; ShiftEnPort se;
 UpdateEnPort ue; TCKPort tck; ResetPort reset;
 ScanRegister value[7:0] { ScanInSource si; CaptureSource value; ResetValue 8'b0; }
 ScanOutPort so { Source value[0]; }
 ScanInterface local { Port si; Port so; Port sel; Port ce; Port se; Port ue; Port tck; Port reset; }
}
Module Top {
 ScanInPort si; SelectPort sel; CaptureEnPort ce; ShiftEnPort se;
 UpdateEnPort ue; TCKPort tck; ResetPort reset;
 Instance child Of Leaf {
  InputPort si = si; InputPort sel = sel; InputPort ce = ce;
  InputPort se = se; InputPort ue = ue; InputPort tck = tck; InputPort reset = reset;
 }
 ScanOutPort so { Source child.so; }
 ScanInterface client {
  Port si; Port so; Port sel; Port ce; Port se; Port ue; Port tck; Port reset;
  DefaultLoadValue 8'b0;
 }
}
)ICL";
  for (bool flat : {false, true}) {
    pattern::Design design;
    design.AddIcl("hierarchical-root-scans.icl", source);
    const auto netlist = pattern::Elaborate(design, "Top");
    const auto id = [&](const std::string& name) {
      return netlist.node_ids.at(name);
    };
    const auto value = id("child.value");
    pattern::ScanScheduler scheduler(netlist);
    scheduler.Reset();
    pattern::NetworkState literal(netlist);
    const auto pin = [&](const std::string& name, char bit) {
      literal.Drive(id(name), std::string(1, bit));
    };
    pin("reset", '1');
    literal.SetGlobalReset('1', '1');
    literal.Reset();
    pin("reset", '0');
    literal.SetGlobalReset('0', '0');
    literal.Reset();
    const auto phase = [&](char capture, char shift, char update, char clock) {
      pin("sel", '1');
      pin("ce", capture);
      pin("se", shift);
      pin("ue", update);
      pin("tck", clock);
      literal.Reset();
    };
    for (const std::string payload : {"10100101", "10x01x01"}) {
      phase('1', '0', '0', '0');
      pin("tck", '1');
      literal.Clock({{value, {'1', '0', '0', '1'}}}, true);
      for (auto it = payload.rbegin(); it != payload.rend(); ++it) {
        phase('0', '1', '0', '0');
        pin("si", *it);
        pin("tck", '1');
        literal.Clock({{value, {'0', '1', '0', '1'}}}, true);
      }
      phase('0', '0', '1', '1');
      pin("tck", '0');
      literal.Clock({{value, {'0', '0', '1', '1'}}}, false);
      phase('0', '0', '0', '0');
      scheduler.ApplyScan(
          {"iScan", {"client", "8", "-si", "0b" + payload}, "hier:1", {}});
      for (const auto& node : netlist.nodes) {
        if (node.model == pattern::ModelKind::kPort ||
            node.model == pattern::ModelKind::kRegister) {
          Require(scheduler.Value(node.name) == literal.Value(id(node.name)),
                  "hierarchical bulk replay matches literal register and bound "
                  "pin values");
        }
      }
    }
    scheduler.Apply({{"iWrite", {"child.value[1]", "1"}, "hier:write", {}}});
    Require(
        scheduler.Value("child.value") == "10101111",
        "hierarchical partial-X raw fill retains earlier known assignments");
    auto unbound = netlist;
    unbound.nodes.at(id("child.tck")).inputs.erase("Source");
    pattern::ScanScheduler bad_clock(unbound);
    bool rejected = false;
    try {
      bad_clock.NormalizeScan({"iScan", {"client", "8"}, "hier:clock", {}});
    } catch (const std::runtime_error& error) {
      rejected =
          std::string(error.what()).find("raw scan") != std::string::npos;
    }
    Require(rejected,
            "native auto-driven child TCK cannot qualify an unbound physical "
            "root path");
    auto dependent = netlist;
    dependent.nodes.at(value).reset = pattern::BitReference{value, 0};
    pattern::ScanScheduler bad_reset(dependent);
    rejected = false;
    try {
      bad_reset.NormalizeScan({"iScan", {"client", "8"}, "hier:reset", {}});
    } catch (const std::runtime_error& error) {
      rejected =
          std::string(error.what()).find("raw scan") != std::string::npos;
    }
    Require(rejected,
            "nested storage-dependent reset remains outside the static proof");
    if (!flat) source = pattern::SerializeIcl(netlist);
  }
}
void TestRawSerialAliases() {
  for (bool input_inverse : {false, true}) {
    for (bool bridge_inverse : {false, true}) {
      for (bool output_inverse : {false, true}) {
        for (bool aliases : {false, true}) {
          const std::string input =
              (input_inverse ? "~" : "") + std::string("si");
          const std::string bridge =
              (bridge_inverse ? "~" : "") + std::string("a[1]");
          const std::string output =
              (output_inverse ? "~" : "") + std::string("b");
          std::string source = R"(Module Top {
            ScanInPort si; SelectPort sel; CaptureEnPort ce; ShiftEnPort se;
            UpdateEnPort ue; TCKPort tck; ResetPort reset;
          )";
          if (aliases) source += "Alias input = " + input + ";";
          source += "ScanRegister a[0:1] { ScanInSource " +
                    (aliases ? "input" : input) +
                    "; CaptureSource a; ResetValue 2'b0; }";
          if (aliases) source += "Alias bridge = " + bridge + ";";
          source += "ScanRegister b[2:0] { ScanInSource " +
                    (aliases ? "bridge" : bridge) +
                    "; CaptureSource b; ResetValue 3'b0; }";
          if (aliases) source += "Alias output[0:2] = " + output + ";";
          source += "ScanOutPort so { Source " +
                    (aliases ? std::string("output[2]")
                             : (output_inverse ? "~b[0]" : "b[0]")) +
                    R"(; } ScanInterface client {
                      Port si; Port so; Port sel; Port ce; Port se; Port ue;
                      Port tck; Port reset; DefaultLoadValue 5'b0;
                    } })";
          for (bool flat : {false, true}) {
            pattern::Design design;
            design.AddIcl("raw-serial-aliases.icl", source);
            const auto netlist = pattern::Elaborate(design, "Top");
            pattern::ScanScheduler scheduler(netlist);
            scheduler.Reset();
            pattern::NetworkState literal(netlist);
            const auto id = [&](const std::string& name) {
              return netlist.node_ids.at(name);
            };
            const auto pin = [&](const std::string& name, char value) {
              literal.Drive(id(name), std::string(1, value));
            };
            pin("reset", '1');
            literal.SetGlobalReset('1', '1');
            literal.Reset();
            pin("reset", '0');
            literal.SetGlobalReset('0', '0');
            literal.Reset();
            const auto controls = [&](char capture, char shift, char update) {
              return std::map<size_t, pattern::ScanControl>{
                  {id("a"), {capture, shift, update, '1'}},
                  {id("b"), {capture, shift, update, '1'}}};
            };
            const auto phase = [&](char capture, char shift, char update,
                                   char clock) {
              pin("sel", '1');
              pin("ce", capture);
              pin("se", shift);
              pin("ue", update);
              pin("tck", clock);
              literal.Reset();
            };
            std::string fill_a = "00", fill_b = "000";
            for (const std::string payload : {"10110", "0x1x0"}) {
              phase('1', '0', '0', '0');
              pin("tck", '1');
              literal.Clock(controls('1', '0', '0'), true);
              std::string expected;
              for (auto it = payload.rbegin(); it != payload.rend(); ++it) {
                expected += literal.Read({id("so"), 0}, true);
                phase('0', '1', '0', '0');
                pin("si", *it);
                pin("tck", '1');
                literal.Clock(controls('0', '1', '0'), true);
              }
              phase('0', '0', '1', '1');
              pin("tck", '0');
              literal.Clock(controls('0', '0', '1'), false);
              phase('0', '0', '0', '0');
              const std::string expected_bits(expected.rbegin(),
                                              expected.rend());
              const auto lowered =
                  scheduler.ApplyScan({"iScan",
                                       {"client", "5", "-si", "0b" + payload,
                                        "-so", "0b" + expected_bits},
                                       "alias:1",
                                       {}});
              Require(lowered.arguments[3] == "0b" + payload &&
                          lowered.arguments[5] == "0b" + expected_bits,
                      "raw scan keeps physical SI and SO payloads unchanged");
              for (const std::string name : {"a", "b", "si", "so", "sel", "ce",
                                             "se", "ue", "tck", "reset"}) {
                Require(scheduler.Value(name) == literal.Value(id(name)),
                        "bulk alias/polarity replay matches literal native "
                        "hardware");
              }
              const auto a = literal.Value(id("a")), b = literal.Value(id("b"));
              for (size_t i = 0; i < a.size(); ++i) {
                if (a[i] != 'x') fill_a[i] = a[i];
              }
              for (size_t i = 0; i < b.size(); ++i) {
                if (b[i] != 'x') fill_b[i] = b[i];
              }
            }
            fill_b[2] = '1';
            scheduler.Apply({{"iWrite", {"b[0]", "1"}, "alias:write", {}}});
            Require(scheduler.Value("a") == fill_a &&
                        scheduler.Value("b") == fill_b,
                    "known physical raw loads retain controlling fill through "
                    "aliases and X bits");
            if (!flat) source = pattern::SerializeIcl(netlist);
          }
        }
      }
    }
  }
}
void TestModeledRootScans() {
  std::string source = R"(Module Top {
    ScanInPort si; SelectPort sel; CaptureEnPort ce; ShiftEnPort se;
    UpdateEnPort ue; TCKPort tck; ResetPort reset;
    ScanRegister value[7:0] {
      ScanInSource si; CaptureSource value; ResetValue 8'b0;
    }
    ScanOutPort so { Source value[0]; }
    ScanInterface client {
      Port si; Port so; Port sel; Port ce; Port se; Port ue; Port tck;
      Port reset; DefaultLoadValue 8'b0;
    }
  })";
  for (bool flat : {false, true}) {
    pattern::Design design;
    design.AddIcl("modeled-root-scans.icl", source);
    const auto netlist = pattern::Elaborate(design, "Top");
    pattern::ScanScheduler scheduler(netlist);
    scheduler.Reset();
    const auto normalized = scheduler.NormalizeScan(
        {"iScan", {"client", "8", "-si", "0xa5", "-so", "0x3c"}, "raw:1", {}});
    Require(scheduler.Value("value") == "00000000",
            "normalizing a queued raw scan does not execute it");
    scheduler.ApplyScan(normalized);
    Require(
        scheduler.Value("value") == "10100101" && scheduler.Value("si") == "1",
        "raw SI updates native state and retains the final SI pin");
    Require(scheduler.Value("tck") == "0" && scheduler.Value("sel") == "1" &&
                scheduler.Value("ce") == "0" && scheduler.Value("se") == "0" &&
                scheduler.Value("ue") == "0" && scheduler.Value("reset") == "0",
            "modeled raw replay retains ordinary client idle pin levels");
    Require(scheduler.Value("value") != "00111100",
            "expected SO comparison does not assign hardware readback");
    auto control = scheduler;
    bool rejected = false;
    try {
      scheduler.ApplyScan({"iScan", {"client", "7"}, "raw:bad", {}});
    } catch (const std::runtime_error&) {
      rejected = true;
    }
    Require(rejected && scheduler.Value("value") == control.Value("value") &&
                scheduler.Value("si") == control.Value("si"),
            "failed raw scan does not adopt partial state");
    const std::vector<pattern::Operation> write{
        {"iWrite", {"value[1]", "1"}, "ordinary:1", {}}};
    scheduler.Apply(write);
    control.Apply(write);
    Require(scheduler.Value("value") == "10100111" &&
                scheduler.Value("value") == control.Value("value"),
            "ordinary masked writes use raw scan controlling fill");
    scheduler.Reset();
    scheduler.ApplyScan(
        {"iScan", {"client", "8", "-si", "0b10x01x01"}, "raw:x", {}});
    Require(scheduler.Value("value") == "10x01x01",
            "unknown raw data stays unknown in native hardware state");
    scheduler.Apply(write);
    Require(scheduler.Value("value") == "10001011",
            "partial-X raw updates retain newly assigned controlling bits");
    auto unsafe = netlist;
    unsafe.nodes.at(unsafe.node_ids.at("value")).reset =
        pattern::BitReference{unsafe.node_ids.at("value"), 0};
    pattern::ScanScheduler unsafe_scheduler(unsafe);
    unsafe_scheduler.Reset();
    rejected = false;
    try {
      unsafe_scheduler.NormalizeScan(
          {"iScan", {"client", "8"}, "raw:reset", {}});
    } catch (const std::runtime_error& error) {
      rejected =
          std::string(error.what()).find("raw scan") != std::string::npos;
    }
    Require(
        rejected,
        "released global reset does not qualify a register reset dependency");
    if (!flat) source = pattern::SerializeIcl(netlist);
  }
}
void TestInvertedClockDurations() {
  std::string source = R"(Module Top {
    ClockPort root;
    Alias inverse = ~root;
    ToClockPort out { Source inverse; FreqDivider 3; }
    ScanInPort si; ScanRegister value { ScanInSource si; }
    ScanOutPort so { Source value; }
  })";
  for (bool flat : {false, true}) {
    pattern::Design design;
    design.AddIcl("inverted-durations.icl", source);
    const auto netlist = pattern::Elaborate(design, "Top");
    pattern::ScanScheduler scheduler(netlist);
    scheduler.Assume("value", "1");
    pattern::SystemClocks clocks(netlist);
    z3::context context;
    Require(clocks.Check("out", scheduler) == "root" &&
                clocks.Wait("out", 2, scheduler).second == 6,
            "clock inversion preserves source frequency and duration ratios");
    for (const std::string held : {"root", "out"}) {
      const std::map<size_t, std::string> preserved{
          {netlist.node_ids.at(held), "0"}};
      bool rejected = false;
      try {
        clocks.Wait("out", 1, scheduler, preserved);
      } catch (const std::runtime_error& error) {
        rejected =
            std::string(error.what()).find("held clock") != std::string::npos;
      }
      Require(
          rejected && clocks.Wait("out", 0, scheduler, preserved).second == 0,
          "inverted inputs and outputs remain protected when held");
    }
    clocks.Override(
        {"iClockOverride", {"out", "-freqDivider", "5"}, "inverted:1", {}},
        context);
    Require(clocks.Wait("out", 2, scheduler).second == 6 &&
                clocks.Check("out", scheduler) == "root" &&
                clocks.Wait("out", 2, scheduler).second == 10,
            "an inverted ICL default survives old and new override snapshots");
    auto unsafe = netlist;
    unsafe.nodes[unsafe.node_ids.at("value")].inputs["CaptureSource"] = {
        {unsafe.node_ids.at("out"), 0}};
    pattern::ScanScheduler unsafe_scheduler(unsafe);
    pattern::SystemClocks unsafe_clocks(unsafe);
    bool rejected = false;
    try {
      unsafe_clocks.Check("root", unsafe_scheduler);
    } catch (const std::runtime_error& error) {
      rejected = std::string(error.what()).find("unsupported fanout") !=
                 std::string::npos;
    }
    Require(rejected,
            "non-clock consumers remain unsafe through inverted aliases");
    Require(scheduler.Value("value") == "1" && scheduler.Value("root") == "x",
            "inverted duration tracing does not alter hardware state");
    if (!flat) source = pattern::SerializeIcl(netlist);
  }
}
void TestDifferentialClockDurations() {
  std::string source = R"(Module Top {
    ClockPort positive;
    ClockPort negative { DifferentialInvOf positive; }
    ToClockPort divided { Source positive; FreqDivider 3; }
    ToClockPort divided_n { DifferentialInvOf divided; }
    ScanInPort si; ScanRegister value { ScanInSource si; }
    ScanOutPort so { Source value; }
  })";
  for (bool flat : {false, true}) {
    pattern::Design design;
    design.AddIcl("differential-durations.icl", source);
    const auto netlist = pattern::Elaborate(design, "Top");
    pattern::ScanScheduler scheduler(netlist);
    scheduler.Assume("value", "1");
    pattern::SystemClocks clocks(netlist);
    z3::context context;
    for (const std::string target : {"positive", "negative"}) {
      Require(clocks.Check(target, scheduler) == "positive",
              "differential halves share a duration root");
      for (const std::string held : {"positive", "negative"}) {
        const std::map<size_t, std::string> preserved{
            {netlist.node_ids.at(held), "0"}};
        bool rejected = false;
        try {
          clocks.Wait(target, 1, scheduler, preserved);
        } catch (const std::runtime_error& error) {
          rejected =
              std::string(error.what()).find("held clock") != std::string::npos;
        }
        Require(rejected &&
                    clocks.Wait(target, 0, scheduler, preserved).second == 0,
                "held positive or negative half forbids only nonzero waits");
      }
    }
    clocks.Check("divided_n", scheduler);
    bool rejected = false;
    try {
      clocks.Override({"iClockOverride",
                       {"divided", "-source", "divided_n"},
                       "differential:1",
                       {}},
                      context);
    } catch (const std::runtime_error& error) {
      rejected =
          std::string(error.what()).find("cyclic system-clock override") !=
          std::string::npos;
    }
    Require(rejected && clocks.Wait("divided_n", 2, scheduler).second == 6 &&
                clocks.Check("divided_n", scheduler) == "positive",
            "a differential override cycle cannot publish a new version");
    auto unsafe = netlist;
    unsafe.nodes[unsafe.node_ids.at("value")].inputs["CaptureSource"] = {
        {unsafe.node_ids.at("negative"), 0}};
    pattern::ScanScheduler unsafe_scheduler(unsafe);
    pattern::SystemClocks unsafe_clocks(unsafe);
    rejected = false;
    try {
      unsafe_clocks.Check("positive", unsafe_scheduler);
    } catch (const std::runtime_error& error) {
      rejected = std::string(error.what()).find("unsupported fanout") !=
                 std::string::npos;
    }
    Require(rejected,
            "native non-clock use propagates through differential association");
    Require(
        scheduler.Value("value") == "1" && scheduler.Value("positive") == "x",
        "clock duration declarations and checks do not change hardware state");
    if (!flat) source = pattern::SerializeIcl(netlist);
  }
}
void TestFlatClockTargetNames() {
  std::string source = R"(Module Stage {
    ClockPort ck[1:0]; ToClockPort out[1:0] { Source ck; FreqDivider 2; }
  }
  Module Top {
    ClockPort clk[5:4]; ClockPort other[5:4]; DataInPort d;
    Instance child Of Stage { InputPort ck = clk; }
  })";
  for (bool flat : {false, true}) {
    pattern::Design design;
    design.AddIcl("physical-clock-names.icl", source);
    const auto netlist = pattern::Elaborate(design, "Top");
    pattern::ScanScheduler scheduler(netlist);
    pattern::SystemClocks clocks(netlist);
    const std::string physical = "n6368696c642e6f7574(1)";
    if (!flat) {
      bool rejected = false;
      try {
        clocks.Check(physical, scheduler);
      } catch (const std::runtime_error& error) {
        rejected = std::string(error.what()).find("unknown PDL target") !=
                   std::string::npos;
      }
      Require(rejected && clocks.Check("child.out(1)", scheduler) == "clk(5)",
              "physical names require restored flat metadata");
      source = pattern::SerializeIcl(netlist);
      continue;
    }
    Require(clocks.Check(physical, scheduler) == "clk(5)" &&
                clocks.Wait("child.out(1)", 3, scheduler).second == 6 &&
                clocks.Wait(physical, 2, scheduler).second == 4,
            "physical and original spellings share one checked clock bit");
    auto preview = clocks;
    preview.Invalidate();
    Require(preview.Wait(physical, 1, scheduler).second == 2,
            "physical-name state copies and invalidates with clock previews");
    z3::context context;
    clocks.Override(
        {"iClockOverride",
         {physical, "-source", "n6f74686572(5)", "-freqDivider", "3"},
         "physical-clock:override",
         {}},
        context);
    Require(clocks.Wait(physical, 2, scheduler).second == 4 &&
                clocks.Check("child.out(1)", scheduler) == "other(5)" &&
                clocks.Wait(physical, 2, scheduler).second == 6,
            "physical names preserve saved and current override versions");
    for (const std::string bad :
         {"n636C6b(5)", "n64", "n636c6b(5:4)", "n636c6b(6)", "n636c6b("}) {
      bool rejected = false;
      try {
        clocks.Check(bad, scheduler);
      } catch (const std::runtime_error&) {
        rejected = true;
      }
      Require(rejected && clocks.Wait(physical, 1, scheduler).second == 3,
              "bad physical spellings do not replace checked clocks");
    }
    clocks.Override({"iClockOverride",
                     {physical, "-freqDivider", "4"},
                     "physical-clock:override-defaults",
                     {}},
                    context);
    Require(
        clocks.Wait(physical, 1, scheduler).second == 3 &&
            clocks.Check(physical, scheduler) == "clk(5)" &&
            clocks.Wait(physical, 1, scheduler).second == 4,
        "physical overrides preserve saved checks and restore ICL defaults");
  }
}

void TestInvariantTimeWaits() {
  std::string source = R"(Module Top {
    ClockPort root[1:0]; ClockPort other[1:0]; DataInPort mode; TCKPort tck;
    ToClockPort out { Source root[0]; FreqDivider 2; }
    ClockMux selected SelectedBy mode { 1'b0: root[0]; 1'b1: other[0]; }
  })";
  for (bool flat : {false, true}) {
    pattern::Design design;
    design.AddIcl("invariant-time.icl", source);
    const auto netlist = pattern::Elaborate(design, "Top");
    pattern::ScanScheduler scheduler(netlist);
    pattern::SystemClocks clocks(netlist);
    clocks.CheckTimeWait();
    clocks.CheckTimeWait({{netlist.node_ids.at("mode"), "0"}});
    auto preview = clocks;
    preview.Invalidate();
    preview.CheckTimeWait({{netlist.node_ids.at("root"), "xx"}});
    Require(clocks.Check("out", scheduler) == "root(0)",
            "time waits retain ordinary checked clock identity");
    z3::context context;
    clocks.Override({"iClockOverride",
                     {"out", "-source", "other(0)", "-freqDivider", "3"},
                     "time:override",
                     {}},
                    context);
    clocks.CheckTimeWait();
    clocks.Invalidate();
    Require(clocks.Wait("out", 2, scheduler).second == 4 &&
                clocks.Check("out", scheduler) == "other(0)",
            "time waits retain saved and current override versions");
    for (const auto& held :
         std::map<size_t, std::string>{{netlist.node_ids.at("root"), "x0"},
                                       {netlist.node_ids.at("out"), "0"},
                                       {netlist.node_ids.at("tck"), "0"}}) {
      bool rejected = false;
      try {
        clocks.CheckTimeWait({held});
      } catch (const std::runtime_error& error) {
        rejected =
            std::string(error.what()).find("held clock") != std::string::npos;
      }
      Require(
          rejected && clocks.Wait("out", 2, scheduler).second == 6,
          "cached time eligibility rechecks held clocks without state changes");
    }
    if (!flat) source = pattern::SerializeIcl(netlist);
  }
  for (const std::string source :
       {"Module Top { ScanInPort si; ScanRegister r { ScanInSource si; } "
        "ScanOutPort so { Source r; } }",
        "Module Top { ScanInPort si; SelectPort sel; CaptureEnPort ce; "
        "ShiftEnPort se; UpdateEnPort ue; TCKPort tck; ScanOutPort so { Source "
        "si; } "
        "ScanInterface client { Port si; Port so; Port sel; Port ce; Port se; "
        "Port ue; Port tck; } }"}) {
    pattern::Design design;
    design.AddIcl("stateful-time.icl", source);
    const auto netlist = pattern::Elaborate(design, "Top");
    pattern::SystemClocks clocks(netlist);
    bool rejected = false;
    try {
      clocks.CheckTimeWait();
    } catch (const std::runtime_error& error) {
      rejected = std::string(error.what()).find("absolute period models") !=
                 std::string::npos;
    }
    Require(rejected, "time waits reject storage and opaque interfaces");
  }
  pattern::Design design;
  design.AddIcl("time-clock-fanout.icl",
                "Module Stage { ClockPort ck; ToClockPort out { Source ck; } } "
                "Module Top { ClockPort root; Instance child Of Stage { "
                "InputPort ck = root; } "
                "ToTCKPort observed { Source child.out; } }");
  const auto netlist = pattern::Elaborate(design, "Top");
  pattern::SystemClocks clocks(netlist);
  bool rejected = false;
  try {
    clocks.CheckTimeWait();
  } catch (const std::runtime_error& error) {
    rejected =
        std::string(error.what()).find("clock fanout") != std::string::npos;
  }
  Require(rejected,
          "time waits reject unsupported free-running clock consumers");

  std::string forwarded_source =
      "Module Endpoint { TCKPort tck; } "
      "Module Top { DataInPort d; DataOutPort out { Source d; } "
      "Instance child Of Endpoint { InputPort tck = d; } }";
  for (bool flat : {false, true}) {
    pattern::Design forwarded_design;
    forwarded_design.AddIcl("forwarded-time-clock.icl", forwarded_source);
    const auto forwarded = pattern::Elaborate(forwarded_design, "Top");
    const auto data = forwarded.node_ids.at("d");
    const auto& clock = forwarded.nodes.at(forwarded.node_ids.at("child.tck"));
    Require(clock.inputs.at("Source").at(0).node == data,
            "data input is the physical source of the child test clock");
    pattern::SystemClocks forwarded_clocks(forwarded);
    bool rejected = false;
    try {
      forwarded_clocks.CheckTimeWait({{data, "1"}});
    } catch (const std::runtime_error& error) {
      rejected = std::string(error.what()).find("test-clock forwarding") !=
                 std::string::npos;
    }
    Require(rejected,
            "time waits cannot bypass clock holds through data bindings");
    if (!flat) forwarded_source = pattern::SerializeIcl(forwarded);
  }
}

void TestBoundDifferentialClocks() {
  std::string source = R"(Module Pair {
    ClockPort positive;
    ClockPort negative { DifferentialInvOf positive; }
  }
  Module Divider {
    ClockPort input; ToClockPort output { Source input; FreqDivider 3; }
  }
  Module Buffer { ClockPort input; ToClockPort path { Source input; } }
  Module Top {
    ClockPort root; ClockPort other; DataInPort mode;
    Instance divided Of Divider { InputPort input = root; }
    Instance buffer Of Buffer { InputPort input = divided.output; }
    ClockMux mux SelectedBy mode { 1'b0: ~buffer.path; 1'b1: ~other; }
    Instance pair Of Pair {
      InputPort positive = divided.output; InputPort negative = mux;
    }
    ScanInPort si; ScanRegister value { ScanInSource si; ResetValue 0; }
    ScanOutPort so { Source value; }
  })";
  for (bool flat : {false, true}) {
    pattern::Design design;
    design.AddIcl("bound-differential.icl", source);
    const auto netlist = pattern::Elaborate(design, "Top");
    pattern::ScanScheduler scheduler(netlist);
    scheduler.Assume("mode", "0");
    scheduler.Assume("value", "1");
    pattern::SystemClocks clocks(netlist);
    z3::context context;
    Require(clocks.Check("pair.negative", scheduler) == "root" &&
                clocks.Wait("pair.negative", 4, scheduler).second == 12,
            "bound complementary inputs share a converted clock origin");
    clocks.Override(
        {"iClockOverride", {"buffer.path", "-source", "other"}, "bound:1", {}},
        context);
    bool rejected = false;
    try {
      clocks.Check("pair.negative", scheduler);
    } catch (const std::runtime_error& error) {
      rejected = std::string(error.what()).find("bound differential") !=
                 std::string::npos;
    }
    Require(rejected && clocks.Wait("pair.negative", 4, scheduler).second == 12,
            "failed new differential check retains the old override snapshot");
    scheduler.Assume("mode", "1");
    clocks.Invalidate();
    rejected = false;
    try {
      clocks.Wait("pair.negative", 0, scheduler);
    } catch (const std::runtime_error& error) {
      rejected = std::string(error.what()).find("bound differential") !=
                 std::string::npos;
    }
    Require(rejected, "even zero waits revalidate applied binding selectors");
    scheduler.Assume("mode", "0");
    clocks.Invalidate();
    Require(clocks.Wait("pair.negative", 1, scheduler).second == 3,
            "failed differential route can recover after selector restoration");
    clocks.Override({"iClockOverride",
                     {"buffer.path", "-source", "divided.output"},
                     "bound:2",
                     {}},
                    context);
    Require(clocks.Check("pair.negative", scheduler) == "root",
            "a later consistent override can publish a fresh checked route");
    const std::map<size_t, std::string> held{
        {netlist.node_ids.at("pair.negative"), "0"}};
    rejected = false;
    try {
      clocks.Wait("pair.negative", 1, scheduler, held);
    } catch (const std::runtime_error& error) {
      rejected =
          std::string(error.what()).find("held clock") != std::string::npos;
    }
    Require(rejected &&
                clocks.Wait("pair.negative", 0, scheduler, held).second == 0,
            "bound differential negative inputs remain protected by ownership");
    Require(scheduler.Value("value") == "1" && scheduler.Value("root") == "x" &&
                scheduler.Value("pair.negative") == "x",
            "differential consistency checks do not change hardware storage or "
            "pins");
    auto cyclic = netlist;
    const auto negative = cyclic.node_ids.at("pair.negative");
    cyclic.nodes[negative].inputs["Source"] = {{negative, 0}};
    pattern::ScanScheduler cyclic_scheduler(cyclic);
    pattern::SystemClocks cyclic_clocks(cyclic);
    cyclic_scheduler.Assume("mode", "0");
    rejected = false;
    try {
      cyclic_clocks.Check("pair.negative", cyclic_scheduler);
    } catch (const std::runtime_error& error) {
      rejected = std::string(error.what()).find("cyclic system-clock") !=
                 std::string::npos;
    }
    Require(rejected,
            "cycles in the explicit binding cannot hide behind the declared "
            "partner");
    if (!flat) source = pattern::SerializeIcl(netlist);
  }
}
void TestClockApiContracts() {
  pattern::Design design;
  design.AddIcl("clock-api.icl", R"(Module Stage {
    ClockPort first; ClockPort second; DataInPort mode;
    ClockMux selected SelectedBy mode { 1'b0: first; 1'b1: second; }
    ToClockPort clock { Source selected; }
  }
  Module Driver {
    ClockPort input; ToClockPort out { Source input; FreqDivider 2; }
  }
  Module Top {
    ClockPort first; ClockPort second; DataInPort mode;
    Instance driver Of Driver { InputPort input = first; }
    Instance stage Of Stage {
      InputPort first = first; InputPort second = driver.out; InputPort mode = mode;
    }
  })");
  const auto netlist = pattern::Elaborate(design, "Top");
  pattern::ScanScheduler scheduler(netlist);
  scheduler.Assume("mode", "0");
  pattern::SystemClocks clocks(netlist);
  z3::context context;
  Require(clocks.Check("driver.out", scheduler) == "first",
          "native override fixture starts on the divided first clock");
  const std::vector<std::pair<std::vector<std::string>, std::string>> invalid{
      {{}, "invalid iClockOverride arguments"},
      {{"driver.out", "-source"}, "invalid iClockOverride arguments"},
      {{"driver.out", "-freqDivider", "7", "-freqDivider", "3"},
       "duplicate iClockOverride option"},
      {{"driver.out", "-source", "second", "-freqDivider", "7", "-source",
        "first"},
       "duplicate iClockOverride option"},
      {{"driver.out", "-freqDivider", "7", "-unknown", "1"},
       "unknown iClockOverride option"},
      {{"driver.out", "-source", "stage.clock"},
       "cyclic system-clock override"}};
  for (const auto& [arguments, diagnostic] : invalid) {
    bool rejected = false;
    try {
      clocks.Override({"iClockOverride", arguments, "clock-api:invalid", {}},
                      context);
    } catch (const std::runtime_error& error) {
      rejected =
          std::string(error.what()).find(diagnostic) != std::string::npos;
    }
    Require(rejected, "native invalid override reports the intended failure");
    const auto saved = clocks.Wait("driver.out", 3, scheduler);
    Require(saved.first == "first" && saved.second == 6,
            "failed override preserves the saved duration");
    clocks.Invalidate();
    Require(clocks.Check("driver.out", scheduler) == "first" &&
                clocks.Wait("driver.out", 3, scheduler).second == 6,
            "failed override preserves the current clock definition");
  }
  clocks.Override({"iClockOverride",
                   {"driver.out", "-source", "second", "-freqDivider", "3"},
                   "clock-api:valid",
                   {}},
                  context);
  const auto saved = clocks.Wait("driver.out", 3, scheduler);
  Require(saved.first == "first" && saved.second == 6 &&
              clocks.Check("driver.out", scheduler) == "second" &&
              clocks.Wait("driver.out", 3, scheduler).second == 9,
          "valid override after rejected calls retains versioned durations");
  Require(scheduler.Value("mode") == "0" && scheduler.Value("first") == "x" &&
              scheduler.Value("second") == "x",
          "native override validation does not drive hardware");
}

void TestClockUnrelatedConsumers() {
  for (bool client : {false, true}) {
    std::string source = R"(Module Top {
      ClockPort root; DataInPort a; DataInPort b;
      LogicSignal ready { a & b; }
      DataOutPort observed { Source ready; }
    )";
    if (client) {
      source += R"(
        ScanInPort si; ScanOutPort so { Source si; }
        SelectPort sel; CaptureEnPort ce; ShiftEnPort se;
        UpdateEnPort ue; TCKPort ck;
        ScanInterface client {
          Chain serial { Port si; Port so; }
          Port sel; Port ce; Port se; Port ue; Port ck;
        }
      )";
    }
    source += "}";
    for (bool flat : {false, true}) {
      pattern::Design design;
      design.AddIcl("unrelated-clock-consumers.icl", source);
      const auto netlist = pattern::Elaborate(design, "Top");
      pattern::ScanScheduler scheduler(netlist);
      scheduler.Assume("a", "1");
      scheduler.Assume("b", "0");
      pattern::SystemClocks clocks(netlist);
      Require(clocks.Check("root", scheduler) == "root",
              "unrelated logic and client pins do not taint a clock root");
      const auto wait = clocks.Wait("root", 7, scheduler);
      Require(wait.first == "root" && wait.second == 7,
              "unrelated consumers preserve root-relative duration");
      if (!client) {
        clocks.CheckTimeWait();
      } else {
        bool rejected = false;
        try {
          clocks.CheckTimeWait();
        } catch (const std::runtime_error& error) {
          rejected = std::string(error.what()).find("opaque hardware") !=
                     std::string::npos;
        }
        Require(rejected, "unrelated opaque clients still exclude time waits");
        for (const auto* pin : {"si", "sel", "ce", "se", "ue", "ck"}) {
          Require(scheduler.Value(pin) == "x",
                  "clock checks and waits preserve unrelated client pins");
        }
      }
      Require(scheduler.Value("a") == "1" && scheduler.Value("b") == "0" &&
                  scheduler.Value("observed") == "0" &&
                  scheduler.Value("root") == "x",
              "clock checks and waits preserve unrelated combinational state");
      if (!flat) source = pattern::SerializeIcl(netlist);
    }
  }
}

void TestClockOverrideSnapshots() {
  pattern::Design design;
  design.AddIcl("overrides.icl", R"(Module Top {
    ClockPort first; ClockPort second;
    ToClockPort out { Source first; FreqDivider 2; }
    ToClockPort other { Source second; }
    ScanInPort si; ScanRegister value { ScanInSource si; }
    ScanOutPort so { Source value; }
  })");
  const auto netlist = pattern::Elaborate(design, "Top");
  pattern::ScanScheduler scheduler(netlist);
  scheduler.Assume("value", "1");
  pattern::SystemClocks clocks(netlist);
  z3::context context;
  const auto out = netlist.node_ids.at("out");
  const auto other = netlist.node_ids.at("other");
  clocks.Check("out", scheduler);
  clocks.Override({"iClockOverride",
                   {"out", "-source", "second", "-freqMultiplier", "3",
                    "-freqDivider", "1"},
                   "overrides:1",
                   {}},
                  context);
  const auto old = clocks.Wait("out", 3, scheduler, {{out, "0"}});
  Require(old.first == "first" && old.second == 6,
          "old duration does not protect a clock moved to another root");
  clocks.Check("second", scheduler);
  bool rejected = false;
  try {
    clocks.Wait("second", 1, scheduler, {{out, "0"}});
  } catch (const std::runtime_error& error) {
    rejected =
        std::string(error.what()).find("held clock") != std::string::npos;
  }
  Require(rejected, "new override connection protects held clock");
  clocks.Check("out", scheduler);
  clocks.Override({"iClockOverride",
                   {"out", "-source", "first", "-freqDivider", "5"},
                   "overrides:2",
                   {}},
                  context);
  Require(clocks.Wait("out", 3, scheduler, {{out, "0"}}).second == 1,
          "current routing differs from checked duration");
  rejected = false;
  try {
    clocks.Wait("out", 3, scheduler, {{other, "1"}});
  } catch (const std::runtime_error& error) {
    rejected =
        std::string(error.what()).find("held clock") != std::string::npos;
  }
  Require(rejected, "old duration still protects another current consumer");
  clocks.Override(
      {"iClockOverride", {"other", "-source", "out"}, "overrides:3", {}},
      context);
  rejected = false;
  try {
    clocks.Override(
        {"iClockOverride", {"out", "-source", "other"}, "overrides:4", {}},
        context);
  } catch (const std::runtime_error& error) {
    rejected = std::string(error.what()).find("cyclic") != std::string::npos;
  }
  Require(rejected, "introduced current cycle rejected before use");
  Require(clocks.Check("out", scheduler) == "first" &&
              clocks.Wait("out", 2, scheduler).second == 10,
          "failed override preserves the previous definition");
  Require(scheduler.Value("value") == "1" && scheduler.Value("first") == "x" &&
              scheduler.Value("second") == "x",
          "override declarations and waits preserve hardware state");
}
void TestClockOverrideScale() {
  std::string source =
      "Module Stage { ClockPort ck; ToClockPort out { Source ck; } } "
      "Module Top { ClockPort first; ClockPort second; ";
  for (size_t i = 0; i < 4096; ++i) {
    const auto previous = i ? "c" + std::to_string(i - 1) + ".out" : "first";
    source += "Instance c" + std::to_string(i) +
              " Of Stage { InputPort ck = " + previous + "; }\n";
  }
  source += "}";
  pattern::Design design;
  design.AddIcl("override-scale.icl", source);
  const auto netlist = pattern::Elaborate(design, "Top");
  pattern::ScanScheduler scheduler(netlist);
  pattern::SystemClocks clocks(netlist);
  z3::context context;
  clocks.Check("c4095.out", scheduler);
  clocks.Override(
      {"iClockOverride", {"c0.out", "-source", "second"}, "scale:1", {}},
      context);
  const std::map<size_t, std::string> held{
      {netlist.node_ids.at("c4095.out"), "0"}};
  const auto start = std::chrono::steady_clock::now();
  for (size_t i = 0; i < 1024; ++i) {
    const auto wait = clocks.Wait("c4095.out", 1, scheduler, held);
    Require(wait.first == "first" && wait.second == 1,
            "alternating duration/reservation versions preserve old wait");
  }
  std::cout << "clock override versions: stages=4096 waits=1024 bytes="
            << source.size() << " seconds="
            << std::chrono::duration<double>(std::chrono::steady_clock::now() -
                                             start)
                   .count()
            << std::endl;
}
void TestOscillatorSourceOverrides() {
  std::string source = R"(Module Osc {
    ToClockPort clock[0:1] { Period 10ps; }
  }
  Module Top {
    ClockPort root; Instance a Of Osc; Instance b Of Osc;
    ToClockPort out { Source a.clock[0]; }
    ScanInPort si; ScanRegister value { ScanInSource si; }
    ScanOutPort so { Source value; }
  })";
  for (bool flat : {false, true}) {
    pattern::Design design;
    design.AddIcl("oscillator-overrides.icl", source);
    const auto netlist = pattern::Elaborate(design, "Top");
    pattern::ScanScheduler scheduler(netlist);
    scheduler.Assume("value", "1");
    pattern::SystemClocks clocks(netlist);
    z3::context context;
    const auto root = netlist.node_ids.at("root");
    const auto oscillator = netlist.node_ids.at("a.clock");
    const auto out = netlist.node_ids.at("out");
    clocks.Check("a.clock(0)", scheduler);
    clocks.Override({"iClockOverride",
                     {"a.clock(0)", "-source", "root", "-freqDivider", "3"},
                     "oscillator:1",
                     {}},
                    context);
    const auto old = clocks.Wait("a.clock(0)", 2, scheduler,
                                 {{oscillator, "0x"}, {out, "1"}});
    Require(old.first == "a.clock(0)" && old.second == 2,
            "old intrinsic duration is independent of current held route");
    clocks.Check("root", scheduler);
    for (const auto& held : std::vector<std::map<size_t, std::string>>{
             {{oscillator, "0x"}}, {{out, "1"}}}) {
      bool rejected = false;
      try {
        clocks.Wait("root", 1, scheduler, held);
      } catch (const std::runtime_error& error) {
        rejected =
            std::string(error.what()).find("held clock") != std::string::npos;
      }
      Require(rejected, "replacement source protects held oscillator output");
    }
    Require(
        clocks.Check("a.clock(1)", scheduler) == "a.clock(1)" &&
            clocks.Wait("root", 2, scheduler, {{oscillator, "x0"}}).second == 2,
        "untouched vector bit remains an independent oscillator");
    clocks.Check("a.clock(0)", scheduler);
    clocks.Override(
        {"iClockOverride",
         {"a.clock(0)", "-source", "b.clock(1)", "-freqMultiplier", "2"},
         "oscillator:2",
         {}},
        context);
    const auto previous =
        clocks.Wait("a.clock(0)", 2, scheduler, {{oscillator, "0x"}});
    Require(previous.first == "root" && previous.second == 6,
            "oscillator replacement retains previous checked version");
    bool rejected = false;
    try {
      clocks.Override({"iClockOverride",
                       {"a.clock(0)", "-source", "out"},
                       "oscillator:3",
                       {}},
                      context);
    } catch (const std::runtime_error& error) {
      rejected = std::string(error.what()).find("cyclic") != std::string::npos;
    }
    Require(
        rejected && clocks.Check("a.clock(0)", scheduler) == "b.clock(1)" &&
            clocks.Wait("a.clock(0)", 2, scheduler, {{root, "0"}}).second == 1,
        "failed oscillator cycle preserves replacement and default divider");
    const auto cached = clocks.Wait("a.clock(0)", 6, scheduler);
    clocks.Override({"iClockOverride",
                     {"a.clock(0)", "-freqMultiplier", "3"},
                     "oscillator:factor",
                     {}},
                    context);
    Require(cached.first == "b.clock(1)" && cached.second == 3 &&
                clocks.Wait("a.clock(0)", 6, scheduler) == cached,
            "factor-only oscillator update retains the previous checked ratio");
    rejected = false;
    try {
      clocks.Check("a.clock(0)", scheduler);
    } catch (const std::runtime_error& error) {
      rejected = std::string(error.what()).find("inactive system-clock path") !=
                 std::string::npos;
    }
    Require(rejected && clocks.Wait("a.clock(0)", 6, scheduler) == cached,
            "failed fresh check preserves the older checked oscillator route");
    rejected = false;
    try {
      clocks.Override(
          {"iClockOverride",
           {"a.clock(0)", "-source", "root", "-freqMultiplier", "0"},
           "oscillator:invalid",
           {}},
          context);
    } catch (const std::runtime_error& error) {
      rejected =
          std::string(error.what()).find("positive") != std::string::npos;
    }
    bool inactive = false;
    try {
      clocks.Check("a.clock(0)", scheduler);
    } catch (const std::runtime_error& error) {
      inactive = std::string(error.what()).find("inactive system-clock path") !=
                 std::string::npos;
    }
    Require(rejected && inactive &&
                clocks.Wait("a.clock(0)", 6, scheduler) == cached,
            "invalid factor cannot publish a partial oscillator source change");
    clocks.Override({"iClockOverride",
                     {"a.clock(0)", "-source", "root", "-freqDivider", "2"},
                     "oscillator:recover",
                     {}},
                    context);
    Require(clocks.Check("a.clock(0)", scheduler) == "root" &&
                clocks.Wait("a.clock(0)", 6, scheduler).second == 12 &&
                clocks.Wait("a.clock(1)", 4, scheduler).second == 4,
            "explicit source recovers while untouched oscillator bits stay "
            "independent");
    Require(netlist.nodes[oscillator].clock_period->magnitude == 10 &&
                scheduler.Value("value") == "1" &&
                scheduler.Value("root") == "x",
            "oscillator overrides preserve original metadata and hardware");
    if (!flat) source = pattern::SerializeIcl(netlist);
  }
}
void TestOscillatorWaits() {
  std::string source = R"(Module Osc {
    ToClockPort clock[3:2] { Period 10ps; }
  }
  Module Top {
    Instance a Of Osc; Instance b Of Osc;
    ToClockPort out[0:1] { Source a.clock; FreqDivider 3; }
    ScanInPort si; ScanRegister value[3:0] { ScanInSource si; }
    ScanOutPort so { Source value[0]; }
  })";
  for (bool flat : {false, true}) {
    pattern::Design design;
    design.AddIcl("oscillators.icl", source);
    const auto netlist = pattern::Elaborate(design, "Top");
    pattern::ScanScheduler scheduler(netlist);
    scheduler.Assume("value", "1010");
    const auto root = netlist.node_ids.at("a.clock");
    const auto other = netlist.node_ids.at("b.clock");
    const auto out = netlist.node_ids.at("out");
    const auto& period = netlist.nodes[root].clock_period;
    Require(period && period->magnitude == 10 && period->unit == "ps",
            "typed oscillator period survives flat elaboration");
    pattern::SystemClocks clocks(netlist);
    Require(clocks.Check("out(0)", scheduler) == "a.clock(3)",
            "oscillator root retains hierarchy and bit identity");
    const auto before = scheduler.Value("a.clock");
    for (const auto& held : std::vector<std::map<size_t, std::string>>{
             {{root, "0x"}}, {{root, "1x"}}, {{out, "0x"}}, {{out, "1x"}}}) {
      Require(clocks.Wait("out(0)", 0, scheduler, held).second == 0,
              "zero oscillator wait permits held levels");
      bool rejected = false;
      try {
        clocks.Wait("out(0)", 1, scheduler, held);
      } catch (const std::runtime_error& error) {
        rejected =
            std::string(error.what()).find("held clock") != std::string::npos;
      }
      Require(rejected, "oscillator waits protect held root and descendant");
    }
    for (const auto& held : std::vector<std::map<size_t, std::string>>{
             {}, {{root, "x0"}}, {{out, "x1"}}, {{other, "00"}}}) {
      const auto wait = clocks.Wait("out(0)", 17, scheduler, held);
      Require(wait.first == "a.clock(3)" && wait.second == 51,
              "equal periods and adjacent bits remain independent roots");
    }
    Require(scheduler.Value("value") == "1010" &&
                scheduler.Value("a.clock") == before,
            "oscillator waits preserve storage and levels");
    if (!flat) source = pattern::SerializeIcl(netlist);
  }
}
void TestRelativeSystemClocks() {
  const std::string source = R"(Module Top {
    ClockPort root[3:2]; ClockPort other;
    ToClockPort stopped { Source 1'b0; }
    DataInPort unknown;
    ClockMux unrelated SelectedBy unknown { 1'b0: other; 1'b1: 1'b0; }
    ToClockPort unknown_other { Source unrelated; }
    ToClockPort out[0:1] { Source root; FreqDivider 3; }
    ScanInPort si;
    ScanRegister value[3:0] { ScanInSource si; }
    ScanOutPort so { Source value[0]; }
  })";
  pattern::Design design;
  design.AddIcl("clocks.icl", source);
  const auto netlist = pattern::Elaborate(design, "Top");
  pattern::ScanScheduler scheduler(netlist);
  scheduler.Assume("value", "1010");
  const auto root = netlist.node_ids.at("root");
  const auto out = netlist.node_ids.at("out");
  const auto other = netlist.node_ids.at("other");
  const auto stopped = netlist.node_ids.at("stopped");
  const auto unknown_other = netlist.node_ids.at("unknown_other");
  pattern::SystemClocks clocks(netlist);
  Require(clocks.Check("out(0)", scheduler) == "root(3)", "clock root bit");
  const auto before_root = scheduler.Value("root");
  const auto before_out = scheduler.Value("out");
  for (const auto& held : std::vector<std::map<size_t, std::string>>{
           {{root, "0x"}}, {{root, "1x"}}, {{out, "0x"}}, {{out, "1x"}}}) {
    Require(clocks.Wait("out(0)", 0, scheduler, held).second == 0,
            "zero clock wait permits held levels");
    bool rejected = false;
    try {
      clocks.Wait("out(0)", 1, scheduler, held);
    } catch (const std::runtime_error& error) {
      rejected =
          std::string(error.what()).find("held clock") != std::string::npos;
    }
    Require(rejected, "nonzero clock wait protects root and forwarded holds");
  }
  for (const auto& held :
       std::vector<std::map<size_t, std::string>>{{},
                                                  {{root, "x0"}},
                                                  {{out, "x1"}},
                                                  {{root, "xx"}},
                                                  {{other, "0"}},
                                                  {{stopped, "0"}},
                                                  {{unknown_other, "1"}}}) {
    const auto wait = clocks.Wait("out(0)", 17, scheduler, held);
    Require(wait.first == "root(3)" && wait.second == 51,
            "relative clock duration");
  }
  Require(
      scheduler.Value("value") == "1010" &&
          scheduler.Value("root") == before_root &&
          scheduler.Value("out") == before_out,
      "accepted and rejected relative waits preserve storage and clock pins");
}
void TestLoweredClockDataFanout() {
  for (bool derived : {false, true}) {
    pattern::Design design;
    design.AddIcl("lowered_clock_fanout.icl", R"(Module Top {
      ClockPort root; ClockPort other; DataInPort data;
      ClockMux branch SelectedBy 1'b0 { 1'b0: other; 1'b1: root; }
      DataMux observed SelectedBy 1'b0 { 1'b0: data; }
    })");
    auto netlist = pattern::Elaborate(design, "Top");
    // Preserve engine guard coverage after frontend source-kind validation.
    netlist.nodes.at(netlist.node_ids.at("observed"))
        .choices.at(0)
        .source.at(0) = {netlist.node_ids.at(derived ? "branch" : "root"), 0};
    pattern::ScanScheduler scheduler(netlist);
    pattern::SystemClocks clocks(netlist);
    bool rejected = false;
    try {
      clocks.Check("root", scheduler);
    } catch (const std::runtime_error& error) {
      rejected = std::string(error.what()).find("unsupported fanout") !=
                 std::string::npos;
    }
    Require(rejected,
            "lowered data fanout remains unsafe through clock branches");
  }
}
void TestMultipleTapResetSnapshot() {
  const std::string fixture =
      "tests/cases/ours/pattern_generation/multiple_taps/";
  std::ifstream icl(fixture + "multiple_explicit.icl");
  std::ifstream bsdl(fixture + "models.bsdl");
  const std::string original(std::istreambuf_iterator<char>(icl), {});
  const std::string models(std::istreambuf_iterator<char>(bsdl), {});
  for (bool reverse : {false, true}) {
    std::string source = original;
    const auto replace = [&](const std::string& from, const std::string& to) {
      const auto at = source.find(from);
      Require(at != std::string::npos, "reset fixture substitution exists");
      source.replace(at, from.size(), to);
    };
    replace("Module TapCell {",
            "Module TapCell { DataOutPort q[3:0] { Source value; }");
    replace("InputPort reset = TRSTB;", "InputPort reset = reset_b;");
    replace("TRSTPort TRSTA; TRSTPort TRSTB;",
            "TRSTPort TRSTA; TRSTPort TRSTB; DataMux reset_b SelectedBy a.q[0] "
            "{ 1'b0: TRSTB; 1'b1: 1'b1; }");
    if (reverse) replace("AccessLink tapA", "AccessLink z_tapA");
    pattern::Design design;
    design.AddIcl("reset-snapshot.icl", source);
    for (const std::string entity : {"ProbeA", "ProbeB"}) {
      design.AddBsdl("models.bsdl", models, entity);
    }
    const auto netlist =
        pattern::ElaborateTapNetwork(design, "MultiTapTop").netlist;
    pattern::ScanScheduler scheduler(netlist);
    scheduler.Reset();
    scheduler.Apply({{"iWrite", {"a.value", "1"}, "reset:1", {}},
                     {"iWrite", {"b.value", "1"}, "reset:2", {}}},
                    true);
    Require(scheduler.Value("a.value") == "0001" &&
                scheduler.Value("b.value") == "0001",
            "prepare gated reset with both instruments nonzero");
    scheduler.Reset();
    Require(scheduler.Value("a.value") == "0000" &&
                scheduler.Value("b.value") == "0000",
            "simultaneous reset assertions settle shared gating independent of "
            "TAP order");
  }
}
void TestMultipleTapScheduling() {
  const std::string fixture =
      "tests/cases/ours/pattern_generation/multiple_taps/";
  std::ifstream icl(fixture + "multiple_explicit.icl");
  std::ifstream bsdl(fixture + "models.bsdl");
  const std::string source(std::istreambuf_iterator<char>(icl), {});
  const std::string models(std::istreambuf_iterator<char>(bsdl), {});
  pattern::Design design;
  design.AddIcl("multiple.icl", source);
  for (const std::string entity : {"ProbeA", "ProbeB"}) {
    design.AddBsdl("models.bsdl", models, entity);
  }
  const auto original =
      pattern::ElaborateTapNetwork(design, "MultiTapTop").netlist;
  pattern::Design flat_design;
  flat_design.AddIcl("flat.icl", pattern::SerializeIcl(original));
  const auto flat = pattern::Elaborate(flat_design, "MultiTapTop");
  for (const auto* netlist : {&original, &flat}) {
    for (bool synchronous : {false, true}) {
      pattern::ScanScheduler scheduler(*netlist), regenerated(*netlist);
      scheduler.Reset(synchronous);
      regenerated.Reset(synchronous);
      Require(scheduler.Value("tapA.INSTRUCTION") == "0010" &&
                  scheduler.Value("tapB.INSTRUCTION") == "0010",
              "all TAP instruction registers receive BSDL reset values");
      if (!synchronous) {
        Require(scheduler.Value("a.value") == "0000" &&
                    scheduler.Value("b.value") == "0000",
                "explicit TRST paths reset both instruments");
      } else {
        Require(scheduler.Value("a.value") == "xxxx" &&
                    scheduler.Value("b.value") == "xxxx",
                "synchronous TAP reset does not invent TRST assertions");
      }
      const auto steps =
          scheduler.Apply({{"iWrite", {"a.value", "5"}, "test:1", {}},
                           {"iWrite", {"b.value", "10"}, "test:2", {}}},
                          true);
      Require(steps.size() == 2 && steps[0].instruction &&
                  !steps[1].instruction && steps[1].together,
              "common IR scan precedes one simultaneous DR update");
      for (const auto& step : steps) {
        Require(step.lanes.size() == 2 && step.serial.empty(),
                "both TAP lanes participate in each common scan");
        Require(step.lanes[0].serial.size() == step.lanes[1].serial.size(),
                "serial lanes use the same number of TCK cycles");
        std::vector<pattern::Operation> operations;
        for (const auto& [name, bits] : step.writes) {
          operations.push_back({"iWrite", {name, "0b" + bits}, "replay", {}});
        }
        for (const auto& [name, bits] : step.expected) {
          operations.push_back({"iRead", {name, "0b" + bits}, "replay", {}});
        }
        regenerated.Apply(operations, step.together);
      }
      for (const auto* path :
           {"a.value", "b.value", "tapA.INSTRUCTION", "tapB.INSTRUCTION"}) {
        Require(scheduler.Value(path) == regenerated.Value(path),
                "source and generated common TAP scans preserve held state");
      }
      Require(scheduler.Value("a.value") == "0101" &&
                  scheduler.Value("b.value") == "1010",
              "separate TAP lanes update both requested values");
      const std::map<size_t, std::string> preserved{
          {netlist->node_ids.at("b.value"), "1010"}};
      scheduler.Idle(3, preserved);
      scheduler.Apply({{"iWrite", {"a.value", "9"}, "test:3", {}}}, false,
                      preserved);
      Require(scheduler.Value("a.value") == "1001" &&
                  scheduler.Value("b.value") == "1010",
              "unrequested lane retains reserved controlling data");
      bool rejected = false;
      try {
        scheduler.Apply({{"iWrite", {"a.value", "2"}, "test:4", {}},
                         {"iWrite", {"b.value", "3"}, "test:5", {}}},
                        true, preserved, false);
      } catch (const std::runtime_error&) {
        rejected = true;
      }
      Require(rejected && scheduler.Value("a.value") == "1001" &&
                  scheduler.Value("b.value") == "1010" &&
                  scheduler.Value("tapA.INSTRUCTION") == "0011" &&
                  scheduler.Value("tapB.INSTRUCTION") == "0011",
              "failed joint transaction preserves both lanes and controllers");
    }
  }
}
void TestOneHotSharedAnalysisBudget() {
  pattern::Design design;
  design.AddIcl("one-hot-budget.icl", R"(
Module Terminal {
  DataInPort data[4095:0]; DataInPort enable;
  DataOutPort terminal[4095:0] { Source data; Enable enable; }
}
Module Driver {
  DataInPort data[4095:0]; DataInPort enable;
  Instance source Of Terminal { InputPort data = data; InputPort enable = enable; }
  DataOutPort forwarded[4095:0] { Source source.terminal; }
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
  DataMux write_enable SelectedBy 1'b0 { 1'b0: ue; 1'b1: 1'b0; }
  ToUpdateEnPort forwarded_update { Source write_enable; }
  DataRegister d { WriteDataSource data; WriteEnSource write_enable; ResetValue 1'b0; }
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
void TestMultipleClientChainGroups() {
  pattern::Design design;
  design.AddIcl("client-groups.icl", R"(
Module GroupCell {
 Parameter W = 4;
 ScanInPort si; SelectPort sel;
 ScanRegister value[$W-1:0] { ScanInSource si; CaptureSource value; ResetValue 0; }
 ScanOutPort so { Source value[0]; }
}
Module GroupedClients {
 ScanInPort si[3:0]; SelectPort sel[1:0]; DataInPort route;
 CaptureEnPort ce; ShiftEnPort se; UpdateEnPort ue; TCKPort ck;
 ScanMux first SelectedBy route { 1'b0: si[0]; 1'b1: si[1]; }
 ScanMux second SelectedBy route { 1'b0: si[1]; 1'b1: si[0]; }
 Instance a Of GroupCell { InputPort si = first; InputPort sel = sel[0]; }
 Instance b Of GroupCell { Parameter W = 2; InputPort si = second; InputPort sel = sel[0]; }
 Instance c Of GroupCell { InputPort si = si[2]; InputPort sel = sel[1]; }
 Instance d Of GroupCell { Parameter W = 2; InputPort si = si[3]; InputPort sel = sel[1]; }
 ScanOutPort so[3:0] { Source d.so, c.so, b.so, a.so; }
 ScanInterface left {
  Chain first { Port si[0]; Port so[0]; }
  Chain second { Port si[1]; Port so[1]; }
  Port sel[0]; Port ce; Port se; Port ue; Port ck;
 }
 ScanInterface right {
  Chain first { Port si[2]; Port so[2]; }
  Chain second { Port si[3]; Port so[3]; }
  Port sel[1]; Port ce; Port se; Port ue; Port ck;
 }
})");
  const auto netlist = pattern::Elaborate(design, "GroupedClients");
  pattern::ScanScheduler scheduler(netlist);
  scheduler.Reset();
  scheduler.Apply({{"iWrite", {"route", "0"}, "route", {}}});
  const auto first =
      scheduler.Apply({{"iWrite", {"a.value", "9"}, "first", {}},
                       {"iWrite", {"b.value", "2"}, "first", {}}},
                      true);
  Require(first.size() == 1 && first[0].lanes.size() == 2 &&
              first[0].lanes[0].input == "si[0]" &&
              first[0].lanes[0].serial == "1001" &&
              first[0].lanes[1].input == "si[1]" &&
              first[0].lanes[1].serial == "0001" &&
              scheduler.Value("si") == "xx11" && scheduler.Value("sel") == "01",
          "selected group's unequal chains share four shifts and preserve "
          "unused lanes");
  const auto second =
      scheduler.Apply({{"iWrite", {"c.value", "6"}, "second", {}},
                       {"iWrite", {"d.value", "1"}, "second", {}}},
                      true);
  Require(second.size() == 1 && second[0].lanes.size() == 2 &&
              second[0].lanes[0].input == "si[2]" &&
              second[0].lanes[0].serial == "0110" &&
              second[0].lanes[1].input == "si[3]" &&
              second[0].lanes[1].serial == "0010" &&
              scheduler.Value("a.value") == "1001" &&
              scheduler.Value("b.value") == "10" &&
              scheduler.Value("si") == "0011" && scheduler.Value("sel") == "10",
          "other client clocks its own group and retains previous storage");
  const auto third =
      scheduler.Apply({{"iRead", {"a.value", "9"}, "read", {}},
                       {"iRead", {"b.value", "2"}, "read", {}},
                       {"iWrite", {"a.value", "3"}, "write", {}},
                       {"iWrite", {"b.value", "1"}, "write", {}}},
                      true);
  Require(
      third.size() == 1 && third[0].lanes.size() == 2 &&
          third[0].lanes[0].serial_expected == "1001" &&
          third[0].lanes[1].serial_expected == "01xx" &&
          scheduler.Value("c.value") == "0110" &&
          scheduler.Value("d.value") == "01",
      "group captures old values with trailing don't-care comparison padding");
  scheduler.Apply({{"iWrite", {"route", "1"}, "swap inputs", {}}});
  bool rejected = false;
  try {
    scheduler.Apply({{"iWrite", {"a.value", "4"}, "wrong pair", {}}}, false,
                    {{netlist.node_ids.at("route"), "1"}});
  } catch (const std::exception&) {
    rejected = true;
  }
  Require(rejected && scheduler.Value("a.value") == "0011" &&
              scheduler.Value("b.value") == "01" &&
              scheduler.Value("sel") == "01" &&
              scheduler.Value("route") == "1" &&
              scheduler.Value("si") == "0000",
          "disjoint but wrongly paired routes fail without changing pins or "
          "storage");
  scheduler.Apply({{"iWrite", {"a.value", "4"}, "free route", {}}});
  Require(scheduler.Value("route") == "0" &&
              scheduler.Value("a.value") == "0100" &&
              scheduler.Value("b.value") == "01",
          "an unreserved route control can restore declared chain pairing");
}

void TestAllPortReservations() {
  const std::string path =
      "tests/cases/ours/pattern_generation/shift_enable_clients.icl";
  std::ifstream input(path);
  Require(input.good(), "read port reservation fixture");
  pattern::Design design;
  design.AddIcl(path, std::string(std::istreambuf_iterator<char>(input), {}));
  const auto netlist = pattern::Elaborate(design, "Chains");
  pattern::ResourceReservations resources(netlist);
  resources.Enter({"parent"});
  resources.Change({"iTake", {"c0"}, "take", {}});
  auto masks = resources.UnwrittenBits();
  Require(masks.size() == 4 && masks.at(netlist.node_ids.at("c0.si")) == "1" &&
              masks.at(netlist.node_ids.at("c0.se")) == "1" &&
              masks.at(netlist.node_ids.at("c0.so")) == "1" &&
              masks.at(netlist.node_ids.at("c0.value")) == "1111",
          "instance ownership includes serial ports and register bits");
  resources.Enter({"parent", "child"});
  resources.Change({"iTake", {"shift[1]"}, "take", {}});
  resources.Change({"iTake", {"ck", "-persistent"}, "take", {}});
  masks = resources.UnwrittenBits();
  Require(masks.at(netlist.node_ids.at("shift")) == "10" &&
              masks.at(netlist.node_ids.at("ck")) == "1",
          "vector lanes and persistent clock ownership retain exact masks");
  resources.Enter({"parent"});
  masks = resources.UnwrittenBits();
  Require(!masks.count(netlist.node_ids.at("shift")) &&
              masks.count(netlist.node_ids.at("ck")) &&
              masks.count(netlist.node_ids.at("c0.si")),
          "return releases child ports while retaining inherited and "
          "persistent ownership");
  resources.Change({"iRelease", {"c0.si"}, "release", {}});
  Require(!resources.UnwrittenBits().count(netlist.node_ids.at("c0.si")),
          "explicit port release removes the inherited mask");
  resources.Enter({});
  Require(resources.UnwrittenBits().size() == 1 && resources.Values().empty(),
          "unwritten takes do not freeze physical pin values");
  resources.Change({"iRelease", {"ck"}, "release", {}});
  Require(resources.UnwrittenBits().empty(),
          "persistent clock can be released");
  resources.Change({"iTake", {""}, "take all", {}});
  Require(
      resources.UnwrittenBits().size() == 15,
      "root ownership includes every register and port, excluding interfaces");
  resources.Reset();
  Require(resources.UnwrittenBits().empty(), "reset clears all resource types");
}
void TestShiftEnableClients() {
  const std::string path =
      "tests/cases/ours/pattern_generation/shift_enable_clients.icl";
  std::ifstream input(path);
  Require(input.good(), "read ShiftEn client fixture");
  const std::string source((std::istreambuf_iterator<char>(input)), {});
  pattern::Design design;
  design.AddIcl(path, source);
  const auto netlist = pattern::Elaborate(design, "Chains");
  pattern::ScanScheduler scheduler(netlist);
  scheduler.Reset();
  const auto first =
      scheduler.Apply({{"iWrite", {"c0.value", "9"}, "first", {}}});
  Require(first.size() == 1 && first[0].input == "si[0]" &&
              first[0].serial == "1001" &&
              scheduler.Value("c0.value") == "1001" &&
              scheduler.Value("c1.value") == "0000" &&
              scheduler.Value("shift") == "00" && scheduler.Value("si") == "x1",
          "ShiftEn client writes one path and returns both enables to idle");
  const auto second =
      scheduler.Apply({{"iWrite", {"c1.value", "6"}, "second", {}}});
  Require(second.size() == 1 && second[0].input == "si[1]" &&
              second[0].serial == "0110" &&
              scheduler.Value("c0.value") == "1001" &&
              scheduler.Value("c1.value") == "0110" &&
              scheduler.Value("shift") == "00" && scheduler.Value("ck") == "0",
          "other ShiftEn client preserves inactive hardware state");
  const auto read = scheduler.Apply({{"iRead", {"c0.value", "9"}, "read", {}}});
  Require(read.size() == 1 && read[0].serial_expected == "1001" &&
              scheduler.Value("c0.value") == "1001" &&
              scheduler.Value("c1.value") == "0110",
          "ShiftEn-only capture returns held data before common update");
  bool rejected = false;
  try {
    scheduler.Apply({{"iWrite", {"c0.value", "3"}, "reserved", {}}}, false,
                    {{netlist.node_ids.at("c0.value"), "1001"}}, false);
  } catch (const std::exception&) {
    rejected = true;
  }
  Require(rejected && scheduler.Value("c0.value") == "1001" &&
              scheduler.Value("c1.value") == "0110" &&
              scheduler.Value("shift") == "00",
          "failed ShiftEn transaction restores storage and idle phase pins");
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
  for (bool inferred : {false, true}) {
    pattern::Design design;
    std::string source = R"(
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
})";
    if (inferred) {
      const auto controls = source.find("  SelectPort sel;");
      source.erase(controls, source.rfind('}') - controls);
    }
    design.AddIcl("simultaneous.icl", source);
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
                scheduler.Value("a") == "1001" &&
                scheduler.Value("b") == "10" && scheduler.Value("si") == "1x1x",
            "unequal chains shift together with leading padding and intact "
            "unused pins");
    pattern::ScanScheduler copy(scheduler);
    const auto second = scheduler.Apply({{"iRead", {"a", "9"}, "read", {}},
                                         {"iRead", {"b", "2"}, "read", {}},
                                         {"iWrite", {"a", "6"}, "write", {}},
                                         {"iWrite", {"b", "1"}, "write", {}}},
                                        true);
    Require(
        second.size() == 1 && second[0].lanes[0].serial_expected == "01xx" &&
            second[0].lanes[1].serial_expected == "1001" &&
            scheduler.Value("a") == "0110" && scheduler.Value("b") == "01" &&
            (inferred ||
             (scheduler.Value("ck") == "0" && scheduler.Value("ue") == "0")),
        "parallel chain readback precedes the shared update and pads "
        "comparisons");
    Require(copy.Value("a") == "1001" && copy.Value("b") == "10",
            "copied inferred lanes retain independent state");
    const auto copied = copy.Apply({{"iWrite", {"a", "6"}, "copy", {}},
                                    {"iWrite", {"b", "1"}, "copy", {}}},
                                   true);
    Require(copied.size() == 1 && copied[0].lanes.size() == 2 &&
                copy.Value("a") == scheduler.Value("a") &&
                copy.Value("b") == scheduler.Value("b"),
            "copied inferred lanes schedule the same updates");
    bool rejected = false;
    try {
      scheduler.Apply({{"iWrite", {"a", "9"}, "reserved", {}}}, false,
                      {{netlist.node_ids.at("a"), "0110"}}, false);
    } catch (const std::exception&) {
      rejected = true;
    }
    Require(rejected && scheduler.Value("a") == "0110" &&
                scheduler.Value("b") == "01" &&
                (inferred || scheduler.Value("ck") == "0"),
            "failed simultaneous access rolls back both chains");
  }
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
void TestFeedbackReleaseGroups() {
  pattern::Design design;
  design.AddIcl("feedback-groups.icl", R"(
Module Cell {
 Parameter W=1; ScanInPort si; ResetPort rst { ActivePolarity 0; }
 ScanRegister value[$W-1:0] { ScanInSource si; CaptureSource value; ResetValue 0; }
 ScanOutPort so { Source value[0]; } DataOutPort q[$W-1:0] { Source value; }
}
Module Feedback {
 ScanInPort si;
 Instance a Of Cell { InputPort si=si; InputPort rst=1'b0; }
 Instance b Of Cell { InputPort si=a.so; InputPort rst=1'b0; }
 LogicSignal ready { a.q & b.q; }
 Instance c Of Cell { Parameter W=4; InputPort si=b.so; InputPort rst=1'b0; }
 ScanOutPort so { Source c.so; }
}
)");
  auto netlist = pattern::Elaborate(design, "Feedback");
  for (const auto& [target, source] : std::map<std::string, std::string>{
           {"a.rst", "b.q"}, {"b.rst", "a.q"}, {"c.rst", "ready"}}) {
    netlist.nodes.at(netlist.node_ids.at(target)).inputs["Source"] = {
        {netlist.node_ids.at(source), 0}};
  }
  pattern::CheckCombinationalConnections(netlist);
  pattern::ScanScheduler scheduler(netlist);
  scheduler.Assume("a.value", "0");
  scheduler.Assume("b.value", "0");
  scheduler.Assume("c.value", "0000");
  const auto groups =
      scheduler.ResetPrerequisites({{"iWrite", {"c.value", "9"}, "test", {}}});
  Require(groups.size() == 1 && groups[0].size() == 2 &&
              groups[0][0].arguments ==
                  std::vector<std::string>({"a.value", "0b1"}) &&
              groups[0][1].arguments ==
                  std::vector<std::string>({"b.value", "0b1"}),
          "mutually dependent release choices occupy one component layer");
  bool rejected = false;
  try {
    scheduler.Apply(groups[0]);
  } catch (const std::exception&) {
    rejected = true;
  }
  Require(rejected && scheduler.Value("a.value") == "0" &&
              scheduler.Value("b.value") == "0" &&
              scheduler.Value("c.value") == "0000",
          "grouping held controls does not invent a reachable release");
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
  const auto cycle_start = std::chrono::steady_clock::now();
  pattern::ScanScheduler cyclic(netlist);
  std::cout << "feedback reset dependencies: registers=" << kRegisters
            << " seconds="
            << std::chrono::duration<double>(std::chrono::steady_clock::now() -
                                             cycle_start)
                   .count()
            << '\n';
}
void TestGreedyRollback() {
  std::string source = R"(
Module GreedyLeaf {
 ScanInPort si; ResetPort rst;
 ScanRegister target[3:0] { ScanInSource si; ResetValue 4'b0; }
 ScanOutPort so { Source target[0]; }
}
Module GreedyRollback {
 ScanInPort si; ResetPort global;
 ScanRegister release { ScanInSource si; ResetValue 1'b0; }
 DataMux reset_gate SelectedBy release { 1'b0: 1'b1; 1'b1: global; }
 Instance child Of GreedyLeaf { InputPort si = release; InputPort rst = reset_gate; }
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
void TestDataResetCallbacks() {
  std::ifstream file(
      "tests/cases/ours/pattern_generation/data_reset_callbacks.icl");
  pattern::Design design;
  design.AddIcl("data-reset-callbacks.icl",
                std::string(std::istreambuf_iterator<char>(file), {}));
  auto netlist = pattern::Elaborate(design, "DataResetCallbacks");
  // These arbitrary transient circuits exercise the lowered backend, not ICL
  // reset-port conformance; retain their original transition functions.
  for (const auto& [target, source] : std::map<std::string, std::string>{
           {"a.rst", "z"}, {"d.rst", "a.q"}, {"b.rst", "rb"}}) {
    netlist.nodes.at(netlist.node_ids.at(target)).inputs["Source"] = {
        {netlist.node_ids.at(source), 0}};
  }
  pattern::CheckCombinationalConnections(netlist);
  for (bool initialized : {false, true}) {
    pattern::ScanScheduler scheduler(netlist);
    if (initialized) scheduler.Reset();
    scheduler.Assume("a.value", "0");
    scheduler.Assume("b.value", "1001");
    scheduler.Assume("d.value", "0");
    scheduler.Assume("z", "0");
    if (!initialized) {
      bool rejected = false;
      for (const auto& operation : std::vector<pattern::Operation>{
               {"iRead", {"b.value", "9"}, "test", {}},
               {"iWrite", {"b.value", "0bx"}, "test", {}}}) {
        std::map<std::string, std::string> before;
        for (const std::string name : {"a.value", "b.value", "d.value", "z"}) {
          before.emplace(name, scheduler.Value(name));
        }
        try {
          scheduler.Apply({operation});
        } catch (const std::runtime_error& error) {
          const std::string message = error.what();
          rejected =
              message.find("unrepresented scan load") != std::string::npos ||
              message.find("reachability not proved") != std::string::npos;
          for (const auto& [name, value] : before) {
            Require(scheduler.Value(name) == value,
                    "failed unknown-global apply preserves preceding state");
          }
          break;
        }
      }
      Require(rejected,
              "unknown global reset cannot invent controlling history");
      continue;
    }
    const auto steps =
        scheduler.Apply({{"iRead", {"b.value", "9"}, "test", {}}});
    Require(scheduler.Value("a.value") == "1" &&
                scheduler.Value("b.value") == "0000" &&
                scheduler.Value("d.value") == "0" &&
                scheduler.Value("z") == "1",
            "post-update mixed reset passes preserve transient assertions");
    Require(steps.size() == 1 && !steps.front().writes.count("b.value") &&
                steps.front().expected.at("b.value") == "1001",
            "capture precedes callback reset and overwritten writes are not "
            "emitted");
    scheduler.Apply({{"iWrite", {"b.value", "0bx"}, "test", {}}});
    Require(scheduler.Value("b.value") == "0000",
            "post-write reset controls the next X-filled scan");
  }
}
void TestUnreleasedDataFeedback() {
  for (bool mixed : {false, true}) {
    pattern::Design design;
    design.AddIcl("unreleased-backend.icl",
                  R"(
Module Scan {
 Parameter W=1; ScanInPort si; ResetPort rst { ActivePolarity 0; }
 ScanRegister value[$W-1:0] { ScanInSource si; )" +
                      std::string(mixed ? "" : "CaptureSource value; ") +
                      R"(ResetValue 0; }
 ScanOutPort so { Source value[0]; } DataOutPort q[$W-1:0] { Source value; }
}
Module Data {
 ResetPort rst { ActivePolarity 0; }
 DataRegister value { WriteDataSource 1'b0; WriteEnSource 1'b0; ResetValue 0; }
 DataOutPort q { Source value; }
}
Module Feedback {
 Parameter W=)" + std::string(mixed ? "1" : "4") +
                      R"(; ScanInPort si;
 Instance c Of Scan { Parameter W=$W; InputPort si=si; InputPort rst=1'b0; }
 Instance d Of Data { InputPort rst=1'b0; }
 ScanOutPort so { Source c.so; }
}
)");
    auto netlist = pattern::Elaborate(design, "Feedback");
    const auto id = [&](const std::string& name) {
      return netlist.node_ids.at(name);
    };
    netlist.nodes.at(id("c.rst")).inputs["Source"] = {
        {id(mixed ? "d.q" : "c.q"), 0}};
    if (mixed) {
      netlist.nodes.at(id("d.rst")).inputs["Source"] = {{id("c.q"), 0}};
    }
    pattern::CheckCombinationalConnections(netlist);
    pattern::ScanScheduler scheduler(netlist);
    scheduler.Reset();
    const auto scan_before = scheduler.Value("c.value");
    const auto data_before = scheduler.Value("d.value");
    bool rejected = false;
    try {
      scheduler.Apply(
          {{"iWrite", {"c.value", mixed ? "1" : "9"}, "feedback:1", {}}});
    } catch (const std::runtime_error& error) {
      rejected = std::string(error.what()).find("reachability not proved") !=
                 std::string::npos;
    }
    Require(rejected && scheduler.Value("c.value") == scan_before &&
                scheduler.Value("d.value") == data_before,
            "unreleased self/mixed feedback fails without releasing the loop");
  }
}
void TestDataResetDependencies() {
  std::ifstream file(
      "tests/cases/ours/pattern_generation/data_reset_dependencies.icl");
  pattern::Design design;
  design.AddIcl("data-reset.icl",
                std::string(std::istreambuf_iterator<char>(file), {}));
  const auto netlist = pattern::Elaborate(design, "DataResetNetwork");
  pattern::ScanScheduler scheduler(netlist);
  scheduler.Reset();
  scheduler.Apply({{"iWrite", {"data", "1"}, "test", {}},
                   {"iWrite", {"enable", "1"}, "test", {}}});
  scheduler.Apply({{"iWrite", {"enable", "0"}, "test", {}}});
  scheduler.Apply({{"iWrite", {"c.value", "9"}, "test", {}}});
  Require(
      scheduler.Value("release") == "1" && scheduler.Value("c.value") == "1001",
      "primary DataRegister release permits scan updates");
  scheduler.Apply({{"iWrite", {"data", "0"}, "test", {}},
                   {"iWrite", {"enable", "1"}, "test", {}}});
  Require(
      scheduler.Value("release") == "0" && scheduler.Value("c.value") == "0000",
      "DataRegister update propagates reset into scan storage");
  scheduler.Apply({{"iWrite", {"data", "1"}, "test", {}}});
  scheduler.Apply({{"iWrite", {"enable", "0"}, "test", {}}});
  scheduler.Apply({{"iWrite", {"c.value", "0bx"}, "test", {}}});
  Require(scheduler.Value("c.value") == "0000",
          "reset controlling fill does not resurrect the old scan value");
  const std::vector<pattern::Operation> write{
      {"iWrite", {"c.value", "9"}, "test", {}}};
  Require(scheduler.ResetPrerequisites(write).empty(),
          "already released DataRegister controls need no repair write");
  scheduler.Assume("release", "0");
  const auto releases = scheduler.ResetPrerequisites(write);
  Require(releases.size() == 1 && releases.front().size() == 1 &&
              releases.front().front().arguments ==
                  std::vector<std::string>{"release", "0b1"} &&
              scheduler.Value("release") == "0",
          "data release planning returns a write without changing hardware");
  scheduler.Apply(releases.front());
  Require(
      scheduler.Value("release") == "1",
      "planned data reset release executes through its declared write path");
  scheduler.Apply(write);
  Require(scheduler.Value("c.value") == "1001",
          "released scan storage receives the requested value");
}
void TestDataResetReleaseReplay() {
  for (bool scan_driven : {false, true}) {
    const std::string file_name = "data_reset_release_replay.icl";
    std::ifstream file("tests/cases/ours/pattern_generation/" + file_name);
    pattern::Design design;
    design.AddIcl(file_name,
                  std::string(std::istreambuf_iterator<char>(file), {}));
    const auto netlist = pattern::Elaborate(
        design, scan_driven ? "DataResetReleaseScan" : "DataResetReleaseMixed");
    const std::vector<pattern::Operation> write{
        {"iWrite", {"c.value", "9"}, "test", {}}};
    for (bool rewritten : {false, true}) {
      pattern::ScanScheduler scheduler(netlist);
      scheduler.Reset();
      if (rewritten) {
        for (const auto& group : scheduler.ResetPrerequisites(write)) {
          scheduler.Apply(group);
        }
      }
      scheduler.Apply(write);
      Require(scheduler.Value("c.value") == "1001" &&
                  scheduler.Value(scan_driven ? "release" : "d.release") == "1",
              "original and reset-rewritten schedules release and load target");
      if (scan_driven) {
        const std::string expected = rewritten ? "1" : "0";
        Require(
            scheduler.Value("data") == expected &&
                scheduler.Value("enable") == expected,
            "explicit data release establishes scan-source controlling fill");
      } else {
        Require(scheduler.Value("gate") == "1",
                "mixed ancestor release retains the earlier scan control");
      }
      scheduler.Apply({{"iWrite", {"c.value", "0bx"}, "test", {}}});
      Require(scheduler.Value("c.value") == "1001",
              "target controlling fill survives either release schedule");
    }
  }
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
    if (!serial) {
      indirect.push_back({"iWrite", {"enable", "1"}, "data-resource:3", {}});
    }
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
  for (const size_t width : {1024U, 4096U}) {
    pattern::Design direct;
    direct.AddIcl("direct-primary.icl",
                  "Module Direct { DataInPort data[" +
                      std::to_string(width - 1) + ":0]; DataOutPort out[" +
                      std::to_string(width - 1) + ":0] { Source data; } }");
    const auto netlist = pattern::Elaborate(direct, "Direct");
    pattern::ScanScheduler scheduler(netlist);
    const auto start = std::chrono::steady_clock::now();
    const std::string bits(width, '1');
    const auto steps =
        scheduler.Apply({{"iWrite", {"out", "0b" + bits}, "direct:1", {}}});
    Require(steps.size() == 1 && steps[0].writes.at("data") == bits,
            "legal large primary input replays its full assignment");
    std::cout << "direct primary: bits=" << width << " seconds="
              << std::chrono::duration<double>(
                     std::chrono::steady_clock::now() - start)
                     .count()
              << '\n';
  }
  {
    pattern::Design choices;
    choices.AddIcl("primary-fill-choice.icl", R"(
Module Choices {
  DataInPort data[261:0];
  LogicSignal merged { |data; }
  DataOutPort out { Source merged; }
}
)");
    const auto netlist = pattern::Elaborate(choices, "Choices");
    pattern::ScanScheduler scheduler(netlist);
    const auto steps =
        scheduler.Apply({{"iWrite", {"out", "1"}, "fill-choice:1", {}}});
    Require(steps.size() == 1 &&
                steps[0].writes.at("data") == std::string(261, '0') + "1",
            "unsigned fill preference crosses nonaligned objective chunks");
  }
  pattern::Design design;
  design.AddIcl("wide-primary.icl", R"(
Module TooWide { DataInPort data[4096:0]; }
)");
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
  pattern::ScanScheduler copied(scheduler);
  const auto copied_steps =
      copied.Apply({{"iWrite", {"core.r", "0bx"}, "copy-fill", {}}});
  Require(copied_steps.size() == 1 && copied.Value("core.r") == "01011001",
          "scheduler copy retains TAP instruction, reset state and controlling "
          "fill");
  copied.Apply({{"iWrite", {"core.r", "0xa6"}, "copy-write", {}}});
  Require(copied.Value("core.r") == "10100110" &&
              scheduler.Value("core.r") == "01011001",
          "copied scheduler writes do not change original storage");
  copied.Reset();
  Require(scheduler.Value("core.r") == "01011001",
          "copied scheduler reset leaves original TAP and storage unchanged");
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
std::string StandaloneIdleSource(bool implicit_select) {
  std::string source = R"(
Module Cell {
  ScanInPort si; CaptureEnPort ce; ShiftEnPort se; UpdateEnPort ue; TCKPort ck;
  ScanRegister r[2:0] {
    ScanInSource si; CaptureSource r[1:0], r[2]; ResetValue 0;
  }
  ScanOutPort so { Source r[0]; }
  ScanInterface client { Port si; Port so; Port ce; Port se; Port ue; Port ck; }
}
Module Top {
  ScanInPort si; SelectPort sel; CaptureEnPort ce; ShiftEnPort se;
  UpdateEnPort ue; TCKPort ck;
  Instance core Of Cell {
    InputPort si = si; InputPort ce = 1'b1; InputPort se = 1'b0;
    InputPort ue = 1'b1; InputPort ck = ck;
  }
  ScanOutPort so { Source core.so; }
  ScanInterface client { Port si; Port so; Port sel; Port ce; Port se; Port ue; Port ck; }
}
)";
  if (!implicit_select) {
    source.insert(source.find("CaptureEnPort ce"), "SelectPort sel; ");
    source.insert(source.find("Port ce;", source.find("ScanInterface")),
                  "Port sel; ");
    source.insert(source.find("InputPort ce"), "InputPort sel = sel; ");
  }
  return source;
}

void TestStandaloneIdle() {
  for (bool controls : {false, true}) {
    pattern::Design design;
    std::string source =
        "Module Top { ScanInPort si; ScanRegister r[3:0] { ScanInSource si; } "
        "ScanOutPort so { Source r[0]; } DataInPort data; DataInPort enable; "
        "DataRegister stored { WriteDataSource data; WriteEnSource enable; } ";
    if (controls) {
      source +=
          "SelectPort sel; CaptureEnPort ce; ShiftEnPort se; "
          "UpdateEnPort ue; TCKPort ck[2:0]; "
          "ToTCKPort clock_seen { Source ck[1]; } "
          "ScanInterface client { Port si; Port so; Port sel; Port ce; "
          "Port se; Port ue; Port ck[1]; }";
    }
    design.AddIcl("standalone-idle.icl", source + "}");
    const auto netlist = pattern::Elaborate(design, "Top");
    pattern::ScanScheduler scheduler(netlist);
    scheduler.Assume("data", "1");
    scheduler.Assume("enable", "1");
    scheduler.Assume("stored", "0");
    scheduler.Idle(0);
    scheduler.Idle(std::numeric_limits<uint64_t>::max());
    Require(scheduler.Value("r") == "xxxx",
            "standalone waits neither require nor invent reset");
    Require(scheduler.Value("stored") == "0",
            "idle normalization does not invent a primary DataRegister write");
    scheduler.Assume("r", "1001");
    if (controls) scheduler.Assume("sel", "0");
    scheduler.Idle(3);
    Require(
        scheduler.Value("r") == "1001",
        "idle protocol enables do not capture/shift/update ordinary storage");
    if (controls) {
      Require(scheduler.Value("sel") == "0", "idle retains explicit selection");
      Require(scheduler.Value("ck") == "x0x",
              "idle preserves unowned vector clock bits");
      bool rejected = false;
      try {
        scheduler.Idle(1, {{netlist.node_ids.at("clock_seen"), "0"}});
      } catch (const std::exception& error) {
        rejected = std::string(error.what()).find("reserved resource") !=
                   std::string::npos;
      }
      Require(rejected && scheduler.Value("clock_seen") == "0",
              "transient reservation violations roll back even if a full cycle "
              "returns low");
    }
  }

  pattern::Design design;
  design.AddIcl("periodic-idle.icl", StandaloneIdleSource(false));
  const auto netlist = pattern::Elaborate(design, "Top");
  pattern::ScanScheduler scheduler(netlist);
  scheduler.Reset();
  scheduler.Assume("sel", "1");
  scheduler.Assume("core.r", "100");
  const size_t id = netlist.node_ids.at("core.r");
  bool rejected = false;
  try {
    scheduler.Idle(1, {{id, "100"}});
  } catch (const std::exception& error) {
    rejected = std::string(error.what()).find("reserved resource core.r") !=
               std::string::npos;
  }
  Require(rejected && scheduler.Value("core.r") == "100",
          "failed standalone idle preserves committed state");
  scheduler.Idle(1);
  Require(scheduler.Value("core.r") == "001",
          "constant child controls capture on rising and update on falling");
  scheduler.Idle(2);
  Require(scheduler.Value("core.r") == "100",
          "changing idle state is not skipped");
  rejected = false;
  try {
    scheduler.Idle(std::numeric_limits<uint64_t>::max());
  } catch (const std::exception& error) {
    rejected =
        std::string(error.what()).find("resource limit") != std::string::npos;
  }
  Require(rejected && scheduler.Value("core.r") == "100",
          "nonconverging standalone idle rolls back");
  scheduler.Assume("sel", "0");
  scheduler.Idle(2);
  Require(scheduler.Value("core.r") == "100",
          "explicitly disabled child ignores constant capture/update");
  scheduler.Assume("sel", "x");
  scheduler.Idle(1);
  Require(scheduler.Value("core.r") == "xxx",
          "explicit unknown selection is not replaced by an inferred one");
}

void TestStandaloneIdleImplicitSelection() {
  for (const std::string select : {"1", "0", "x"}) {
    pattern::Design design;
    design.AddIcl("implicit-idle.icl", StandaloneIdleSource(true));
    const auto netlist = pattern::Elaborate(design, "Top");
    pattern::ScanScheduler scheduler(netlist);
    scheduler.Reset();
    scheduler.Assume("sel", select);
    scheduler.Assume("core.r", "100");
    bool rejected = false;
    try {
      scheduler.Idle(1);
    } catch (const std::exception& error) {
      rejected = std::string(error.what()).find("implicit selection") !=
                 std::string::npos;
    }
    Require(
        select == "1" ? !rejected && scheduler.Value("core.r") == "001"
                      : rejected && scheduler.Value("core.r") == "100",
        "implicit idle selection needs a proved route and active ancestors");
  }
  auto source = StandaloneIdleSource(true);
  const std::string output = "ScanOutPort so { Source core.so; }";
  source.replace(source.find(output), output.size(), R"(
  DataInPort choose;
  ScanMux route SelectedBy choose { 1'b0: si; 1'b1: core.so; }
  ScanOutPort so { Source route; }
)");
  pattern::Design design;
  design.AddIcl("bypassed-idle.icl", source);
  const auto netlist = pattern::Elaborate(design, "Top");
  pattern::ScanScheduler scheduler(netlist);
  scheduler.Reset();
  scheduler.Assume("sel", "1");
  scheduler.Assume("choose", "0");
  scheduler.Assume("core.r", "100");
  scheduler.Idle(1);
  Require(scheduler.Value("core.r") == "100",
          "omitted selection does not activate a mux-disconnected register");
  scheduler.Assume("choose", "1");
  scheduler.Idle(1);
  Require(scheduler.Value("core.r") == "001",
          "omitted selection follows the current connected route");
}

void TestStandaloneIdleResetEdges() {
  for (bool falling : {false, true}) {
    auto source = StandaloneIdleSource(false);
    source.insert(source.find("ScanInPort si;"), "ResetPort rst; ");
    source.insert(source.find("ScanOutPort so"),
                  "DataOutPort q[2:0] { Source r; } ");
    source.insert(source.find("InputPort si"), "InputPort rst = 1'b0; ");
    const std::string output = "ScanOutPort so { Source core.so; }";
    source.replace(source.find(output), output.size(), R"(
  Instance observer Of Observer {
    InputPort si = core.so; InputPort data = core.q;
    InputPort sel = 1'b1; InputPort ce = 1'b1; InputPort se = 1'b0;
    InputPort ue = 1'b1; InputPort ck = ck;
  }
  ScanOutPort so { Source observer.so; }
)");
    source += R"(
Module Observer {
  ScanInPort si; DataInPort data[2:0]; SelectPort sel;
  CaptureEnPort ce; ShiftEnPort se; UpdateEnPort ue; TCKPort ck;
  ScanRegister r[2:0] { ScanInSource si; CaptureSource data; }
  ScanOutPort so { Source r[0]; }
  ScanInterface client { Port si; Port so; Port sel; Port ce; Port se; Port ue; Port ck; }
}
)";
    pattern::Design design;
    design.AddIcl("idle-reset-edge.icl", source);
    auto netlist = pattern::Elaborate(design, "Top");
    // A lowered event circuit tests TCK-derived reset without accepting invalid
    // ICL.
    netlist.nodes.at(netlist.node_ids.at("core.rst")).inputs.at("Source") = {
        {netlist.node_ids.at("ck"), 0, 'x', falling}};
    pattern::ScanScheduler scheduler(netlist);
    scheduler.Reset();
    scheduler.Assume("sel", "1");
    scheduler.Assume("core.r", "100");
    scheduler.Assume("observer.r", "000");
    scheduler.Idle(1);
    // A rising reset clears held storage, not the bits captured before reset.
    Require(scheduler.Value("core.r") == (falling ? "000" : "001") &&
                scheduler.Value("observer.r") == (falling ? "000" : "100"),
            ("idle reset edges retain capture snapshot and reset priority: "
             "falling=" +
             std::to_string(falling) + " core=" + scheduler.Value("core.r") +
             " observer=" + scheduler.Value("observer.r"))
                .c_str());
  }
}

void TestStandaloneIdleClockGates() {
  for (const std::string enabled : {"0", "1", "x"}) {
    auto source = StandaloneIdleSource(false);
    source.insert(
        source.find("Module Top {") + std::string("Module Top {").size(),
        " DataInPort enabled; ClockMux gated SelectedBy enabled "
        "{ 1'b0: 1'b0; 1'b1: ck; } Instance clock_forward Of ClockForward "
        "{ InputPort incoming = gated; } ");
    source +=
        "Module ClockForward { ClockPort incoming; "
        "ToTCKPort out { Source incoming; } }";
    // Explicit ToTCKPort Source is the existing lowered-model gate extension.
    const std::string binding = "InputPort ck = ck;";
    source.replace(source.find(binding), binding.size(),
                   "InputPort ck = clock_forward.out;");
    pattern::Design design;
    design.AddIcl("idle-clock-gate.icl", source);
    const auto netlist = pattern::Elaborate(design, "Top");
    pattern::ScanScheduler scheduler(netlist);
    scheduler.Reset();
    scheduler.Assume("sel", "1");
    scheduler.Assume("enabled", enabled);
    scheduler.Assume("core.r", "100");
    scheduler.Idle(1);
    Require(scheduler.Value("core.r") == (enabled == "0"   ? "100"
                                          : enabled == "1" ? "001"
                                                           : "xxx"),
            "idle evaluates actual known/unknown child clock transitions");
  }
}

void TestStandaloneIdleDataFill() {
  auto source = StandaloneIdleSource(false);
  source.insert(
      source.find("Module Top {") + std::string("Module Top {").size(),
      R"(
  DataInPort data; DataInPort enabled;
  DataRegister stored {
    WriteDataSource data; WriteEnSource enabled; ResetValue 0;
  }
)");
  pattern::Design design;
  design.AddIcl("idle-data-fill.icl", source);
  const auto netlist = pattern::Elaborate(design, "Top");
  pattern::ScanScheduler scheduler(netlist);
  scheduler.Reset();
  scheduler.Assume("sel", "1");
  scheduler.Assume("core.r", "100");
  scheduler.Assume("data", "1");
  scheduler.Assume("enabled", "1");
  scheduler.Idle(1);
  Require(
      scheduler.Value("stored") == "1" && scheduler.Value("core.r") == "001",
      "standalone falling update settles scan and enabled data storage");
  scheduler.Assume("data", "0");
  scheduler.Assume("enabled", "0");
  scheduler.Apply({{"iWrite", {"stored", "0bx"}, "idle-data-fill.pdl", {}}});
  Require(
      scheduler.Value("stored") == "1",
      "standalone data update becomes retained controlling fill for X writes");
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
 ResetPort global;
 DataMux reset_gate SelectedBy release { 1'b0: 1'b1; 1'b1: global; }
 Instance child Of Child { InputPort si = release; InputPort rst = reset_gate; }
 ScanOutPort so { Source child.so; }
})");
  const auto reset_netlist = pattern::Elaborate(reset_design, "Network");
  pattern::ScanScheduler reset(reset_netlist);
  reset.Reset();
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
 ResetPort global;
 DataMux reset_gate SelectedBy release { 1'b0: 1'b1; 1'b1: global; }
 Instance child Of Child { InputPort si = release; InputPort rst = reset_gate; }
 ScanOutPort so { Source child.so; }
 Alias pair[1:0] = release, child.r { AccessTogether; }
})");
  const auto reset_netlist = pattern::Elaborate(reset_design, "Network");
  pattern::ScanScheduler reset(reset_netlist);
  reset.Reset();
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
  ResetPort global;
  DataMux reset_gate SelectedBy reset { 1'b0: 1'b1; 1'b1: global; }
  Instance child Of Leaf { InputPort si = reset; InputPort rst = reset_gate; }
  ScanMux route SelectedBy gate { 1'b0: reset; 1'b1: child.so; }
  ScanOutPort so { Source route; }
})");
  const auto netlist = pattern::Elaborate(design, "Network");
  pattern::ScanScheduler scheduler(netlist);
  scheduler.Reset();
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
  replay.Reset();
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
void TestPhysicalTapResetLoads() {
  pattern::Design design;
  design.AddIcl("physical.icl", R"(Module Cell {
 ScanInPort si; ResetPort rst;
 ScanRegister value[3:0] { ScanInSource si; CaptureSource value; ResetValue 0; }
 ScanOutPort so { Source value[0]; }
}
Module ResetAccessNetwork {
 ScanInPort si; ResetPort rst;
 SelectPort sel; CaptureEnPort capture; ShiftEnPort shift;
 UpdateEnPort update; TCKPort tck;
 ScanRegister control { ScanInSource si; ResetValue 0; }
 DataMux reset_a SelectedBy control { 1'b0: rst; 1'b1: 1'b1; }
 DataMux reset_b SelectedBy control { 1'b0: 1'b1; 1'b1: rst; }
 Instance a Of Cell { InputPort si = control; InputPort rst = reset_a; }
 Instance b Of Cell { InputPort si = a.so; InputPort rst = reset_b; }
 ScanOutPort so { Source b.so; }
 ScanInterface client { Port si; Port so; Port sel; Port capture; Port shift; Port update; Port tck; }
}

Module ResetAccessTop {
  TRSTPort TRST;
  Instance core Of ResetAccessNetwork { InputPort rst = ~TRST; }
  AccessLink tap Of STD_1149_1_2001 {
    BSDLEntity AccessProbe;
    IJTAG_ON { ScanInterface { core.client; } }
    IJTAG_OFF { ScanInterface { core.client; } }
  }
}
)");
  std::ifstream bsdl(
      "tests/cases/ours/pattern_generation/access_link/simple.bsdl");
  design.AddBsdl("simple.bsdl",
                 std::string(std::istreambuf_iterator<char>(bsdl), {}),
                 "AccessProbe");
  const auto netlist =
      pattern::ElaborateTapNetwork(design, "ResetAccessTop").netlist;
  pattern::ScanScheduler scheduler(netlist);
  scheduler.Reset();
  const auto apply = [&](std::vector<pattern::Operation> operations) {
    return scheduler.Apply(operations, false, {}, true, true);
  };
  const auto ir = apply({{"iWrite", {"tap.INSTRUCTION", "4"}, "test:1", {}},
                         {"iRead", {"tap.INSTRUCTION", "1"}, "test:2", {}}});
  Require(ir.size() == 1 && ir[0].serial == "0010" &&
              ir[0].serial_expected == "1000",
          "physical IR compares capture and shifts literal bits LSB first");
  apply({{"iWrite", {"core.control", "1"}, "test:3", {}},
         {"iWrite", {"core.a.value", "0"}, "test:4", {}}});
  apply({{"iWrite", {"core.b.value", "3"}, "test:5", {}}});
  Require(scheduler.Value("core.b.value") == "0011",
          "released register updates");
  const std::vector<pattern::Operation> masked = {
      {"iWrite", {"core.control", "0"}, "test:6", {}},
      {"iWrite", {"core.b.value", "3"}, "test:7", {}},
      {"iRead", {"core.b.value", "3"}, "test:8", {}}};
  pattern::ScanScheduler before(scheduler);
  bool failed = false;
  try {
    scheduler.Apply(masked, false,
                    {{netlist.node_ids.at("core.b.value"), "0011"}}, true,
                    true);
  } catch (const std::runtime_error& error) {
    Require(std::string(error.what())
                    .find("changed reserved resource core.b.value") !=
                std::string::npos,
            "physical replay retains the reservation failure cause");
    failed = true;
  }
  Require(failed && scheduler.Value("core.control") == "1" &&
              scheduler.Value("core.b.value") == "0011",
          "physical reset preserves resource checks and rolls back failure");
  const auto step = apply(masked);
  const auto replay = before.Apply(masked, false, {}, true, true);
  Require(step.size() == 1 && step[0].loads == step[0].writes &&
              step[0].loads.at("core.b.value") == "0011" &&
              step[0].serial == "110000000" &&
              step[0].serial == replay[0].serial &&
              scheduler.Value("core.b.value") == "0000",
          "physical load identity survives a reset-masked update");
  pattern::ScanScheduler saved(scheduler);
  failed = false;
  try {
    apply({{"iRead", {"core.b.value", "3"}, "test:9", {}}});
  } catch (const std::runtime_error&) {
    failed = true;
  }
  Require(
      failed && scheduler.Value("core.b.value") == saved.Value("core.b.value"),
      "failed physical comparison does not commit state");
  const std::vector<pattern::Operation> read = {
      {"iRead", {"core.b.value", "0"}, "test:10", {}}};
  const auto after = apply(read);
  const auto fresh = saved.Apply(read, false, {}, true, true);
  Require(after.size() == 1 && after[0].loads.at("core.b.value") == "0000" &&
              after[0].serial == fresh[0].serial,
          "reset events replace retained fill and failed compares preserve it");
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
 ResetPort global;
 DataMux reset_gate SelectedBy reset { 1'b0: global; 1'b1: 1'b1; }
 Instance child Of Child { InputPort si = reset; InputPort rst = reset_gate; }
 ScanOutPort so { Source child.so; }
})");
  const auto netlist = pattern::Elaborate(design, "Network");
  pattern::ScanScheduler scheduler(netlist);
  scheduler.Reset();
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
              if (old == next.end() || total < old->second) {
                next[destination] = total;
              }
            }
          }
        }
        cycles = static_cast<size_t>(-1);
        for (const auto& [state, cost] : next) {
          if (std::get<2>(state)) cycles = std::min(cycles, cost);
        }
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
int main(int argc, char* argv[]) {
  try {
    if (argc != 1) {
      if (argc == 2 && std::string(argv[1]) == "--raw-reservations") {
        TestRawScanReservations();
        TestRawPortReservationScale();
        TestNoIdBuiltinBypassAliases();
        return 0;
      }
      if (argc == 2 && std::string(argv[1]) == "--clock-snapshots") {
        TestFlatClockTargetNames();
        TestInvariantTimeWaits();
        TestClockApiContracts();
        TestClockUnrelatedConsumers();
        TestBoundDifferentialClocks();
        TestClockOverrideSnapshots();
        TestOscillatorSourceOverrides();
        return 0;
      }
      if (argc == 2 && std::string(argv[1]) == "--reset-backend") {
        TestDataResetCallbacks();
        TestUnreleasedDataFeedback();
        TestGreedyRollback();
        TestApplyTogether();
        TestAccessTogetherPrerequisites();
        TestPrerequisites();
        TestPhysicalTapResetLoads();
        TestResetFill();
        return 0;
      }
      Require(argc == 2 && std::string(argv[1]) == "--primary-limits",
              "expected --primary-limits, --reset-backend, --clock-snapshots, "
              "--raw-reservations or no test arguments");
      TestPrimaryLimits();
      return 0;
    }
    TestPhysicalTapResetLoads();
    TestNoIdBuiltinBypassAliases();
    TestRawTapUnknownInput();
    TestRawTapUnknownStableRollback();
    TestAutomaticChildRawPreparation();
    TestRawTapMultipleDrScans();
    TestRawTapBuiltinDrScans();
    TestRawTapDrScans();
    TestRawTapIrScans();
    TestMixedChildFallbackPreparation();
    TestMixedChildRawPreparation();
    TestNativeOrdinaryAliases();
    TestNativeOrdinaryScans();
    TestMixedRootRawPreparation();
    TestRawScanReservations();
    TestRawPortReservationScale();
    TestDynamicChildRawScans();
    TestActiveChildRawScans();
    TestGatedSibRawScans();
    TestStaticChildScans();
    TestHierarchicalRootScans();
    TestRawSerialAliases();
    TestModeledRootScans();
    TestInvertedClockDurations();
    TestDifferentialClockDurations();
    TestFlatClockTargetNames();
    TestInvariantTimeWaits();
    TestClockApiContracts();
    TestClockUnrelatedConsumers();
    TestBoundDifferentialClocks();
    TestClockOverrideSnapshots();
    TestClockOverrideScale();
    TestRelativeSystemClocks();
    TestOscillatorWaits();
    TestOscillatorSourceOverrides();
    TestMultipleTapResetSnapshot();
    TestLoweredClockDataFanout();
    TestStandaloneIdle();
    TestStandaloneIdleImplicitSelection();
    TestStandaloneIdleResetEdges();
    TestStandaloneIdleClockGates();
    TestStandaloneIdleDataFill();
    TestDataResetCallbacks();
    TestUnreleasedDataFeedback();
    TestDataResetDependencies();
    TestDataResetReleaseReplay();
    TestMultipleTapScheduling();
    TestOneHotSharedAnalysisBudget();
    TestMergeAnalysisBudget();
    TestSharedScanOutputs();
    TestEnabledScanOutputs();
    TestClientScanControls();
    TestMultipleClientChainGroups();
    TestAllPortReservations();
    TestShiftEnableClients();
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
    TestFeedbackReleaseGroups();
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
