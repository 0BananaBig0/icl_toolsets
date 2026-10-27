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

#include "pattern/pattern_design.h"
#include "pattern/pattern_flatten.h"
#include "pattern/pattern_tap.h"

namespace {
void Require(bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
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
  bool rejected = false;
  try {
    scheduler.Apply({{"iWrite", {"paired[1]", "0"}, "control:5", {}}});
  } catch (const std::exception& error) {
    rejected =
        std::string(error.what()).find("AccessTogether") != std::string::npos;
  }
  Require(rejected && scheduler.Value("out") == "1",
          "unimplemented whole-alias port access must not be ignored");
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
  bool rejected = false;
  try {
    scheduler.Apply({{"iWrite", {"invalid[1]", "1"}, "unsupported:1", {}}});
  } catch (const std::exception& error) {
    rejected =
        std::string(error.what()).find("requires ScanRegister members") !=
        std::string::npos;
  }
  Require(rejected && scheduler.Value("gate") == "0",
          "unsupported full group members cannot be hidden behind a slice");

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
void TestUnsupportedEndState() {
  std::ifstream file("tests/cases/ours/pattern_generation/apply_end_state.icl");
  const std::string source(std::istreambuf_iterator<char>(file), {});
  pattern::Design design;
  design.AddIcl("end-state.icl", source);
  const auto netlist = pattern::Elaborate(design, "EndState");
  pattern::Design flat_design;
  flat_design.AddIcl("flat.icl", pattern::SerializeIcl(netlist));
  const auto flat = pattern::Elaborate(flat_design, "EndState");
  for (const auto* candidate : {&netlist, &flat}) {
    bool rejected = false;
    try {
      pattern::ScanScheduler scheduler(*candidate);
    } catch (const std::exception& error) {
      const std::string message = error.what();
      rejected = message.find("iApplyEndState") != std::string::npos &&
                 message.find("idle_trigger") != std::string::npos &&
                 message.find(candidate == &netlist
                                  ? "end-state.icl:"
                                  : "flat.icl:") != std::string::npos;
    }
    Require(rejected,
            "source and flat models must not silently ignore end states");
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
    TestResetDependencyScale();
    TestGreedyRollback();
    TestReservationRollback();
    TestPrimaryWrites();
    TestPrimaryLimits();
    TestPrimaryControllingState();
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
    TestUnsupportedEndState();
    TestPrerequisites();
    TestUnknownAndDisconnected();
    TestImplicitControl();
    TestResetFill();
    TestReadCapture();
    TestMinimumScans();
    std::cout << "pattern scheduler tests passed\n";
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
