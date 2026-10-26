// Copyright 2026 The ICL Toolsets Authors.
// SPDX-License-Identifier: MIT
#include "pattern_access_link.h"

#include <chrono>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>

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
void Reject(const std::string& text, const std::string& expected,
            bool bsdl = true, const std::string& top = "AccessTop") {
  pattern::Design design;
  design.AddIcl("invalid.icl", text);
  if (bsdl) {
    design.AddBsdl("simple.bsdl", Read(kDirectory + "simple.bsdl"),
                   "AccessProbe");
  }
  try {
    pattern::BindAccessLinks(design, design.SelectTop(top));
  } catch (const std::exception& error) {
    const std::string message = error.what();
    Require(message.find("invalid.icl:") != std::string::npos &&
                message.find(expected) != std::string::npos,
            "expected source-located " + expected + ", got " + message);
    return;
  }
  throw std::runtime_error("expected failure: " + expected);
}
void TestInvalidBindings() {
  const auto text = Read(kDirectory + "simple.icl");
  Reject(Replace(text, "IJTAG_ON {", "UNKNOWN {"), "UNKNOWN");
  Reject(text, "BSDL model", false);
  Reject(Replace(text, "core.client;", "unknown.client;"), "unknown instance");
  Reject(Replace(text, "core.client;", "core.unknown;"),
         "unknown ScanInterface");
  Reject(Replace(text, "ScanInterface client", "ScanInterface another"),
         "unknown ScanInterface");
  auto ambiguous = Replace(
      text, "Module AccessInstrument {",
      "Module AccessInstrument { ScanInterface other { Port si; Port so; }");
  Reject(Replace(ambiguous, "core.client;", "core;"),
         "ambiguous ScanInterface");
  auto missing = text;
  const auto start = missing.find("ScanInterface client");
  missing.erase(start, missing.find('}', start) - start + 1);
  Reject(Replace(missing, "core.client;", "core;"), "unknown ScanInterface");
  Reject(text + "Module Wrapper { Instance inner Of AccessTop; }", "top-level",
         true, "Wrapper");
  Reject("Module AccessTop { AccessLink tap Of Generic { opaque; } }",
         "generic");
  Reject(Replace(text, "Port si; Port so;",
                 "Chain a { Port si; Port so; } "
                 "Chain b { Port si; Port so; }"),
         "one scan chain");
}
void TestOrderedAndShorthandBindings() {
  auto text = Read(kDirectory + "simple.icl");
  text = Replace(
      text, "Instance core Of AccessInstrument",
      "Instance first Of AccessInstrument; Instance core Of AccessInstrument");
  text = Replace(text, "core.client;", "first; /* order */ core.client;");
  text = Replace(text, "ActiveSignals { long_setup;",
                 "ActiveSignals { long_setup;\n bus[3:1];");
  text = Replace(text, "IJTAG_ON {", "IJTAG_ON /* gap */ {\n // selection\n");
  text = Replace(text, "STD_1149_1_2001", "STD_1149_1_2013");
  pattern::Design design;
  design.AddIcl("ordered.icl", text);
  design.AddBsdl("simple.bsdl", Read(kDirectory + "simple.bsdl"),
                 "AccessProbe");
  auto links = pattern::BindAccessLinks(design, design.SelectTop("AccessTop"));
  const auto& instruction = links[0].instructions[0];
  Require(links[0].standard == "STD_1149_1_2013" &&
              instruction.interfaces.size() == 2 &&
              instruction.interfaces[0].instance == "first" &&
              instruction.interfaces[1].instance == "core" &&
              instruction.interfaces[0].ast == instruction.interfaces[1].ast,
          "ordered chains borrow one AST for the shared module type");
  Require(instruction.active_signals[1].text == "bus[3:1]",
          "ActiveSignal selection retained for subsequent model construction");
  text = Replace(text, "Port si; Port so;", "Chain only { Port si; Port so; }");
  pattern::Design named;
  named.AddIcl("named.icl", text);
  named.AddBsdl("simple.bsdl", Read(kDirectory + "simple.bsdl"), "AccessProbe");
  Require(
      pattern::BindAccessLinks(named, named.SelectTop("AccessTop")).size() == 1,
      "one explicit Chain may be selected");
}
void TestNoLinksAndNamespaces() {
  pattern::Design empty;
  empty.AddIcl(
      "plain.icl",
      "Module Leaf {} Module Top { Instance a Of Leaf; Instance b Of Leaf; }");
  Require(pattern::BindAccessLinks(empty, empty.SelectTop("Top")).empty(),
          "ordinary hierarchy does not need BSDL");
  auto text = Read(kDirectory + "simple.icl");
  text = Replace(text, "Module AccessInstrument",
                 "NameSpace lib; Module AccessInstrument");
  text = Replace(text, "Module AccessTop",
                 "NameSpace; UseNameSpace lib; Module AccessTop");
  pattern::Design design;
  design.AddIcl("namespaces.icl", text);
  design.AddBsdl("simple.bsdl", Read(kDirectory + "simple.bsdl"),
                 "AccessProbe");
  const auto links =
      pattern::BindAccessLinks(design, design.SelectTop("AccessTop"));
  Require(links[0].instructions[0].interfaces[0].module->name() ==
              "lib::AccessInstrument",
          "reuse Design instance namespace resolution");
}
void TestDuplicateNames() {
  const auto text = Read(kDirectory + "simple.icl");
  Reject(Replace(text, "Module AccessTop {",
                 "Module AccessTop { Instance core Of AccessInstrument;"),
         "duplicate instance");
  Reject(Replace(text, "Module AccessInstrument {",
                 "Module AccessInstrument { ScanInterface client { Port si; "
                 "Port so; }"),
         "duplicate ScanInterface");
}
void TestKeywordInstructionName() {
  pattern::Design design;
  design.AddIcl("keyword.icl", Replace(Read(kDirectory + "simple.icl"),
                                       "IJTAG_ON {", "BSDLEntity {"));
  design.AddBsdl(
      "keyword.bsdl",
      Replace(Read(kDirectory + "simple.bsdl"), "IJTAG_ON (", "BSDLEntity ("),
      "AccessProbe");
  const auto links =
      pattern::BindAccessLinks(design, design.SelectTop("AccessTop"));
  Require(links[0].instructions[0].name == "BSDLEntity" &&
              links[0].instructions[0].interfaces[0].instance == "core",
          "instruction keyword names are identified by their AST block");
}
void TestSharedModuleScale() {
  auto text = Read(kDirectory + "simple.icl");
  std::string ports, instances, references;
  constexpr size_t kCount = 3000;
  for (size_t i = 0; i < kCount; ++i) {
    const auto index = std::to_string(i);
    ports += "DataInPort unused" + index + ";";
    instances += "Instance child" + index + " Of AccessInstrument;";
    references += "child" + index + ".client;";
  }
  text = Replace(text, "Module AccessInstrument {",
                 "Module AccessInstrument {" + ports);
  text = Replace(text, "Module AccessTop {", "Module AccessTop {" + instances);
  text = Replace(text, "core.client;", references);
  pattern::Design design;
  design.AddIcl("scale.icl", text);
  design.AddBsdl("simple.bsdl", Read(kDirectory + "simple.bsdl"),
                 "AccessProbe");
  const auto& top = design.SelectTop("AccessTop");
  const auto start = std::chrono::steady_clock::now();
  const auto links = pattern::BindAccessLinks(design, top);
  const auto& bound = links[0].instructions[0].interfaces;
  Require(bound.size() == kCount && bound.front().instance == "child0" &&
              bound.back().instance == "child2999" &&
              bound.front().ast == bound.back().ast,
          "large ordered fanout retains shared interface AST");
  const std::chrono::duration<double> elapsed =
      std::chrono::steady_clock::now() - start;
  std::cout << "AccessLink binding: " << kCount << " references, " << kCount
            << " unrelated ports, seconds=" << elapsed.count() << '\n';
}
void TestOracleBinding() {
  const std::string directory =
      "tests/cases/ours/pattern_generation/access_link/";
  pattern::Design design;
  design.AddIcl("simple.icl", Read(directory + "simple.icl"));
  design.AddBsdl("simple.bsdl", Read(directory + "simple.bsdl"), "AccessProbe");
  const auto links =
      pattern::BindAccessLinks(design, design.SelectTop("AccessTop"));
  Require(links.size() == 1 && links[0].name == "tap" &&
              links[0].standard == "STD_1149_1_2001" &&
              links[0].bsdl == &design.ResolveBsdl("AccessProbe"),
          "link reuses loaded BSDL model");
  Require(links[0].instructions.size() == 2 &&
              links[0].instructions[0].name == "IJTAG_ON" &&
              links[0].instructions[1].name == "IJTAG_OFF",
          "instruction source order retained");
  const auto& instruction = links[0].instructions[0];
  Require(instruction.interfaces.size() == 1 &&
              instruction.interfaces[0].instance == "core" &&
              instruction.interfaces[0].name == "client" &&
              instruction.interfaces[0].module ==
                  &design.modules().at("AccessInstrument") &&
              instruction.interfaces[0].ast != nullptr,
          "interface binds to original module AST");
  Require(instruction.active_signals.size() == 1 &&
              instruction.active_signals[0].text == "long_setup" &&
              instruction.active_signals[0].origin.find("simple.icl:") == 0,
          "active signals retain source provenance");
}
}  // namespace
int main() {
  try {
    TestOracleBinding();
    TestInvalidBindings();
    TestOrderedAndShorthandBindings();
    TestNoLinksAndNamespaces();
    TestDuplicateNames();
    TestKeywordInstructionName();
    TestSharedModuleScale();
    std::cout << "pattern AccessLink binding tests passed\n";
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
