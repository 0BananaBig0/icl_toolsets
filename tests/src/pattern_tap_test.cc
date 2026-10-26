// Copyright 2026 The ICL Toolsets Authors.
// SPDX-License-Identifier: MIT
#include "pattern_tap.h"

#include <chrono>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>

#include "pattern_flatten.h"
#include "pattern_logic.h"
#include "text_input.h"

namespace {
void Require(bool value, const std::string& message) {
  if (!value) throw std::runtime_error(message);
}
std::string Read(const std::string& path) {
  std::ifstream input(path);
  std::string text;
  Require(input.good() && toolsets::ReadTextStream(input, &text),
          "read " + path);
  return text;
}
const std::string kDirectory =
    "tests/cases/ours/pattern_generation/access_link/";
std::string Replace(std::string text, const std::string& from,
                    const std::string& to) {
  const auto at = text.find(from);
  Require(at != std::string::npos, "replacement exists: " + from);
  text.replace(at, from.size(), to);
  return text;
}
pattern::TapInstructionModel Build(const std::string& icl,
                                   const std::string& bsdl) {
  pattern::Design design;
  design.AddIcl("probe.icl", icl);
  design.AddBsdl("probe.bsdl", bsdl, "AccessProbe");
  const auto links =
      pattern::BindAccessLinks(design, design.SelectTop("AccessTop"));
  return pattern::BuildTapInstructionModel(links[0]);
}
char ReadBit(const pattern::TapInstructionModel& model,
             const pattern::BitReference& bit, const std::string& instruction) {
  if (bit.node == std::numeric_limits<size_t>::max()) return bit.constant;
  if (bit.node == model.instruction_register) {
    return instruction.at(instruction.size() - 1 - bit.index);
  }
  const auto& node = model.netlist.nodes.at(bit.node);
  if (node.model == pattern::ModelKind::kLogic) {
    return pattern::EvaluateLogic(node,
                                  [&](const pattern::BitReference& child) {
                                    return ReadBit(model, child, instruction);
                                  });
  }
  return ReadBit(model,
                 node.inputs.at("Source").at(node.left > node.right
                                                 ? node.left - bit.index
                                                 : bit.index - node.left),
                 instruction);
}
void TestSharedSignalsAndMasks() {
  auto icl =
      Replace(Read(kDirectory + "simple.icl"), "ActiveSignals { long_setup; }",
              "ActiveSignals { long_setup; common; }");
  icl = Replace(icl, "IJTAG_OFF {", "IJTAG_OFF { ActiveSignals { common; }");
  const auto bsdl = Replace(Read(kDirectory + "simple.bsdl"), "IJTAG_ON (0011)",
                            "IJTAG_ON (0011, 10XX, 0110)");
  const auto model = Build(icl, bsdl);
  for (int value = 0; value < 16; ++value) {
    std::string bits(4, '0');
    for (size_t i = 0; i < bits.size(); ++i) bits[3 - i] += (value >> i) & 1;
    const bool on = value == 3 || value == 6 || (value >= 8 && value <= 11);
    const auto common = model.netlist.node_ids.at("common");
    const auto setup = model.netlist.node_ids.at("long_setup");
    Require(
        ReadBit(model, {setup, 0}, bits) == (on ? '1' : '0') &&
            ReadBit(model, {common, 0}, bits) == (on || value == 4 ? '1' : '0'),
        "mask alternatives and shared ActiveSignals follow truth table");
  }
  Require(ReadBit(model, {model.netlist.node_ids.at("long_setup"), 0},
                  "10xx") == '1',
          "opcode masks do not depend on unknown masked bits");
}
void TestNamesAndMissingSignals() {
  auto icl =
      Replace(Read(kDirectory + "simple.icl"), "ActiveSignals { long_setup; }",
              "ActiveSignals { long_setup; INSTRUCTION; tdi; "
              "instruction_select; active_signal; }");
  const auto model = Build(icl, Read(kDirectory + "simple.bsdl"));
  Require(model.netlist.nodes.at(model.instruction_register).name !=
                  "INSTRUCTION" &&
              model.netlist.nodes.at(model.scan_input).name != "tdi",
          "internal names cannot collide with named ActiveSignals");
  for (const auto* name : {"long_setup", "INSTRUCTION", "tdi",
                           "instruction_select", "active_signal"}) {
    Require(
        ReadBit(model, {model.netlist.node_ids.at(name), 0}, "0011") == '1' &&
            ReadBit(model, {model.netlist.node_ids.at(name), 0}, "0100") == '0',
        "colliding names preserve user-visible ActiveSignal identity");
  }
  icl = Replace(Read(kDirectory + "simple.icl"),
                "ActiveSignals { long_setup; }", "");
  icl = Replace(icl, "InputPort mode = tap.long_setup;",
                "InputPort mode = 1'b0;");
  const auto no_signals = Build(icl, Read(kDirectory + "simple.bsdl"));
  Require(no_signals.instruction_selects.size() == 2 &&
              !no_signals.netlist.node_ids.count("long_setup"),
          "instructions without ActiveSignals still have decoders");
}
void TestManyAlternativesAndCaptureMask() {
  std::string codes = "0011";
  for (int i = 0; i < 2048; ++i) codes += ",0011";
  auto bsdl = Replace(Read(kDirectory + "simple.bsdl"), "IJTAG_ON (0011)",
                      "IJTAG_ON (" + codes + ")");
  bsdl = Replace(bsdl, "is \"0001\";", "is \"XX01\";");
  const auto start = std::chrono::steady_clock::now();
  const auto model = Build(Read(kDirectory + "simple.icl"), bsdl);
  const auto& capture = model.netlist.nodes.at(model.instruction_register)
                            .inputs.at("CaptureSource");
  std::string captured;
  for (const auto& bit : capture) captured += bit.constant;
  Require(captured == "xx01", "BSDL capture masks remain unknown, not zero");
  const auto select = model.instruction_selects.at("IJTAG_ON");
  Require(ReadBit(model, {select, 0}, "0011") == '1' &&
              ReadBit(model, {select, 0}, "0001") == '0',
          "balanced alternatives avoid expression-depth failures");
  const std::chrono::duration<double> elapsed =
      std::chrono::steady_clock::now() - start;
  std::cout << "TAP model: 2049 opcode alternatives, seconds="
            << elapsed.count() << '\n';
}
void TestMissingBinding() {
  pattern::AccessLink missing;
  missing.name = "tap";
  missing.origin = "missing.icl:1:1";
  try {
    pattern::BuildTapInstructionModel(missing);
  } catch (const std::runtime_error& error) {
    Require(
        std::string(error.what()).find("missing.icl:1:1: AccessLink tap:") == 0,
        "missing binding has source context");
    return;
  }
  throw std::runtime_error("missing BSDL binding accepted");
}
void TestIndexedSignals() {
  for (const auto* file :
       {"range_down.icl", "range_up.icl", "single_bit.icl"}) {
    pattern::Design design;
    design.AddIcl("base.icl", Read(kDirectory + "simple.icl"));
    design.AddIcl("probe.icl", Read(kDirectory + file));
    design.AddBsdl("probe.bsdl", Read(kDirectory + "simple.bsdl"),
                   "AccessProbe");
    const auto links =
        pattern::BindAccessLinks(design, design.SelectTop("AccessTop"));
    const auto model = pattern::BuildTapInstructionModel(links[0]);
    const auto id = model.netlist.node_ids.at("long_setup");
    const auto& node = model.netlist.nodes.at(id);
    const bool single = std::string(file) == "single_bit.icl";
    Require(node.indexed && node.Width() == (single ? 1 : 3),
            "indexed ActiveSignals retain their declared width");
    Require(node.left == (std::string(file) == "range_up.icl" ? 1 : 3) &&
                node.right == (single                                ? 3
                               : std::string(file) == "range_up.icl" ? 3
                                                                     : 1),
            "ActiveSignal range direction and nonzero index survive");
    for (int bit = single ? 3 : 1; bit <= 3; ++bit) {
      Require(ReadBit(model, {id, bit}, "0011") == '1' &&
                  ReadBit(model, {id, bit}, "0100") == '0' &&
                  ReadBit(model, {id, bit}, "xxxx") == 'x',
              "every selected bit follows instruction state");
    }
    const auto flat = pattern::SerializeIcl(model.netlist);
    pattern::Design replay;
    replay.AddIcl("flat.icl", flat);
    Require(pattern::SerializeIcl(
                pattern::Elaborate(replay, model.netlist.top_module)) == flat,
            "indexed instruction component round trip");
  }
}
void TestOverlappingParameterizedSignals() {
  auto icl = Replace(Read(kDirectory + "simple.icl"), "Module AccessTop {",
                     "Module AccessTop { Parameter N = 3; "
                     "LocalParameter M = $N - 1;");
  icl = Replace(icl, "ActiveSignals { long_setup; }",
                "ActiveSignals { long_setup; bus[$N:1]; bus[2]; "
                "sparse[5]; INSTRUCTION[$M:0]; }");
  icl = Replace(icl, "IJTAG_OFF {",
                "IJTAG_OFF { ActiveSignals { bus[2:4]; sparse[7]; }");
  const auto model = Build(icl, Read(kDirectory + "simple.bsdl"));
  const auto bus = model.netlist.node_ids.at("bus");
  const auto sparse = model.netlist.node_ids.at("sparse");
  Require(model.netlist.nodes.at(bus).left == 4 &&
              model.netlist.nodes.at(bus).right == 1,
          "overlapping selections extend the first range orientation");
  for (int bit = 1; bit <= 4; ++bit) {
    Require(ReadBit(model, {bus, bit}, "0011") == (bit <= 3 ? '1' : '0') &&
                ReadBit(model, {bus, bit}, "0100") == (bit >= 2 ? '1' : '0') &&
                ReadBit(model, {bus, bit}, "0000") == '0',
            "overlapping ranges OR selection per bit, not per whole port");
  }
  Require(ReadBit(model, {sparse, 5}, "0011") == '1' &&
              ReadBit(model, {sparse, 7}, "0011") == '0' &&
              ReadBit(model, {sparse, 6}, "xxxx") == '0',
          "unlisted sparse bits are inactive even with unknown IR");
  Require(model.netlist.NodeAt("INSTRUCTION").Width() == 3 &&
              model.netlist.nodes.at(model.instruction_register).name !=
                  "INSTRUCTION",
          "parameterized ActiveSignal reserves its base name");
}
void TestExtremeSignalIndex() {
  auto icl =
      Replace(Read(kDirectory + "simple.icl"), "ActiveSignals { long_setup; }",
              "ActiveSignals { long_setup; high[9223372036854775807]; }");
  const auto model = Build(icl, Read(kDirectory + "simple.bsdl"));
  Require(ReadBit(model,
                  {model.netlist.node_ids.at("high"),
                   std::numeric_limits<int64_t>::max()},
                  "0011") == '1',
          "single bit at maximum index does not overflow traversal");
}
void TestInvalidSignalRanges() {
  for (const auto& probe : std::vector<std::pair<std::string, std::string>>{
           {"bus; bus[0];", "scalar/indexed ActiveSignal conflict"},
           {"bus[0]; bus;", "scalar/indexed ActiveSignal conflict"},
           {"bus[0]; bus[9223372036854775807];", "width limit"},
           {"bus[$missing];", "unknown parameter"},
           {"bus[2-3];", "negative ICL integer"},
           {"bus[1048576:0];", "width limit"}}) {
    const auto icl = Replace(
        Read(kDirectory + "simple.icl"), "ActiveSignals { long_setup; }",
        "ActiveSignals { long_setup; " + probe.first + " }");
    bool rejected = false;
    try {
      Build(icl, Read(kDirectory + "simple.bsdl"));
    } catch (const std::runtime_error& error) {
      const std::string message = error.what();
      rejected = message.find("probe.icl:") != std::string::npos &&
                 message.find(probe.second) != std::string::npos;
      Require(rejected, "unexpected range diagnostic: " + message);
    }
    Require(rejected, "invalid or excessive ActiveSignal range accepted");
  }
}
void TestSignalDiagnosticOrigin() {
  pattern::Design design;
  design.AddIcl(
      "range_error.icl",
      Replace(
          Replace(Read(kDirectory + "simple.icl"), "Module AccessTop {",
                  "Module AccessTop { Parameter IclPatternSignalShapes = 3;"),
          "ActiveSignals { long_setup; }",
          "ActiveSignals { long_setup;\n bus[$missing]; }"));
  design.AddBsdl("simple.bsdl", Read(kDirectory + "simple.bsdl"),
                 "AccessProbe");
  const auto links =
      pattern::BindAccessLinks(design, design.SelectTop("AccessTop"));
  const auto& bad = links[0].instructions[0].active_signals.back();
  try {
    pattern::BuildTapInstructionModel(links[0]);
  } catch (const std::runtime_error& error) {
    Require(
        std::string(error.what()).find(bad.origin + ":") != std::string::npos,
        "range errors identify original ActiveSignal location: " +
            std::string(error.what()));
    return;
  }
  throw std::runtime_error("undefined ActiveSignal parameter accepted");
}
void TestOverlappingRangeBudget() {
  std::string signals;
  for (int i = 0; i < 18; ++i) {
    signals += "bus[65535:" + std::to_string(i) + "];";
  }
  const auto icl =
      Replace(Read(kDirectory + "simple.icl"), "ActiveSignals { long_setup; }",
              "ActiveSignals { long_setup; " + signals + " }");
  try {
    Build(icl, Read(kDirectory + "simple.bsdl"));
  } catch (const std::runtime_error& error) {
    Require(
        std::string(error.what()).find("selection limit") != std::string::npos,
        "bounded expansion of distinct overlapping ranges");
    return;
  }
  throw std::runtime_error("overlapping ranges evade work budget");
}
void TestDuplicateRangeScale() {
  std::string signals;
  for (int i = 0; i < 10000; ++i) signals += "bus[65535:0];";
  const auto icl =
      Replace(Read(kDirectory + "simple.icl"), "ActiveSignals { long_setup; }",
              "ActiveSignals { long_setup; " + signals + " }");
  const auto start = std::chrono::steady_clock::now();
  const auto model = Build(icl, Read(kDirectory + "simple.bsdl"));
  const auto bus = model.netlist.node_ids.at("bus");
  Require(ReadBit(model, {bus, 0}, "0011") == '1' &&
              ReadBit(model, {bus, 65535}, "0100") == '0',
          "duplicate ranges retain bit semantics without duplicate work");
  std::cout << "TAP model: 10000 repeated 65536-bit ranges, seconds="
            << std::chrono::duration<double>(std::chrono::steady_clock::now() -
                                             start)
                   .count()
            << '\n';
}
void TestCopiedTopLifetime() {
  pattern::Design design;
  design.AddIcl("copy.icl",
                Replace(Read(kDirectory + "simple.icl"), "Module AccessTop {",
                        "Module AccessTop { Parameter N = 2;"));
  design.AddBsdl("simple.bsdl", Read(kDirectory + "simple.bsdl"),
                 "AccessProbe");
  std::vector<pattern::AccessLink> links;
  {
    const auto top = design.SelectTop("AccessTop");
    links = pattern::BindAccessLinks(design, top);
  }
  const auto model = pattern::BuildTapInstructionModel(links[0]);
  Require(ReadBit(model, {model.netlist.node_ids.at("long_setup"), 0},
                  "0011") == '1',
          "binding lifetime depends on Design, not a temporary top copy");
}
void TestInstructionModel() {
  pattern::Design design;
  design.AddIcl("simple.icl", Read(kDirectory + "simple.icl"));
  design.AddBsdl("simple.bsdl", Read(kDirectory + "simple.bsdl"),
                 "AccessProbe");
  const auto links =
      pattern::BindAccessLinks(design, design.SelectTop("AccessTop"));
  const auto model = pattern::BuildTapInstructionModel(links[0]);
  const auto& netlist = model.netlist;
  const auto& reg = netlist.nodes.at(model.instruction_register);
  Require(reg.kind == "ScanRegister" && reg.Width() == 4 &&
              reg.inputs.at("ScanInSource")[0].node == model.scan_input &&
              reg.inputs.at("CaptureSource").size() == 4 &&
              reg.inputs.at("CaptureSource")[3].constant == '1' &&
              reg.inputs.at("CaptureSource")[0].constant == '0',
          "instruction width/capture/scan input reuse elaborated ICL nodes");
  Require(!reg.reset && !reg.constants.count("ResetValue"),
          "component construction does not imply reset or initialized state");
  const auto source = netlist.NodeAt("long_setup").inputs.at("Source")[0];
  for (int bits = 0; bits < 16; ++bits) {
    const auto value = [&](const pattern::BitReference& bit) {
      Require(bit.node == model.instruction_register,
              "decoder reads instruction register");
      return ((bits >> bit.index) & 1) ? '1' : '0';
    };
    const auto& on = netlist.nodes.at(model.instruction_selects.at("IJTAG_ON"));
    const auto& off =
        netlist.nodes.at(model.instruction_selects.at("IJTAG_OFF"));
    Require(pattern::EvaluateLogic(on, value) == (bits == 3 ? '1' : '0') &&
                pattern::EvaluateLogic(off, value) == (bits == 4 ? '1' : '0'),
            "all instruction states agree with oracle opcodes");
    Require(source.node == model.instruction_selects.at("IJTAG_ON"),
            "ActiveSignal uses shared instruction decoder");
  }
  Require(pattern::EvaluateLogic(
              netlist.nodes.at(source.node),
              [](const pattern::BitReference&) { return 'x'; }) == 'x',
          "uninitialized instruction is unknown");
  const auto flat = pattern::SerializeIcl(netlist);
  pattern::Design replay;
  replay.AddIcl("flat.icl", flat);
  const auto again = pattern::Elaborate(replay, netlist.top_module);
  Require(pattern::SerializeIcl(again) == flat,
          "instruction component flat replay");
}
}  // namespace
int main() {
  try {
    TestCopiedTopLifetime();
    TestSignalDiagnosticOrigin();
    TestOverlappingRangeBudget();
    TestDuplicateRangeScale();
    TestExtremeSignalIndex();
    TestInvalidSignalRanges();
    TestInstructionModel();
    TestSharedSignalsAndMasks();
    TestNamesAndMissingSignals();
    TestIndexedSignals();
    TestOverlappingParameterizedSignals();
    TestManyAlternativesAndCaptureMask();
    TestMissingBinding();
    std::cout << "pattern TAP instruction tests passed\n";
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
