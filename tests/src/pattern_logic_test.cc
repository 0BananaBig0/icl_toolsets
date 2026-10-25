// Copyright 2026 The ICL Toolsets Authors.
// SPDX-License-Identifier: MIT
#include "pattern_logic.h"

#include <iostream>
#include <stdexcept>
#include <string>

namespace {
void Require(bool condition, const std::string& message) {
  if (!condition) throw std::runtime_error(message);
}

void TestStandardExamples() {
  pattern::Design design;
  design.AddIcl("logic.icl", R"(
Module M {
  DataInPort e1[1:0];
  DataInPort e2[1:0];
  LogicSignal ctl { e1 == 2'b10 || e2 == 2'b11; }
  LogicSignal bitwise { (e1[1] & ~e1[0]) | (e2[1] & e2[0]); }
  LogicSignal parity { ^e1; }
  LogicSignal all { &e1; }
  LogicSignal any { |e1; }
  LogicSignal masked { e1 == 2'b1x; }
}
)");
  const auto netlist = pattern::Elaborate(design, "M");
  for (int a = 0; a < 4; ++a) {
    for (int b = 0; b < 4; ++b) {
      const auto value = [&](const pattern::BitReference& bit) {
        const int bits = netlist.nodes[bit.node].name == "e1" ? a : b;
        return ((bits >> bit.index) & 1) ? '1' : '0';
      };
      const char expected = a == 2 || b == 3 ? '1' : '0';
      Require(
          pattern::EvaluateLogic(netlist.NodeAt("ctl"), value) == expected &&
              pattern::EvaluateLogic(netlist.NodeAt("bitwise"), value) ==
                  expected,
          "IEEE 6.4.10 alternative control expressions agree with truth table");
      Require(pattern::EvaluateLogic(netlist.NodeAt("parity"), value) ==
                  (((a & 1) ^ (a >> 1)) ? '1' : '0'),
              "XOR reduction");
      Require(pattern::EvaluateLogic(netlist.NodeAt("all"), value) ==
                  (a == 3 ? '1' : '0'),
              "AND reduction");
      Require(pattern::EvaluateLogic(netlist.NodeAt("any"), value) ==
                  (a ? '1' : '0'),
              "OR reduction");
      Require(pattern::EvaluateLogic(netlist.NodeAt("masked"), value) ==
                  (a >= 2 ? '1' : '0'),
              "comparison masks");
    }
  }
  Require(!netlist.NodeAt("ctl").logic.empty(),
          "compiled logic reuses shared AST expressions");
}

void TestEnumsAndParameters() {
  pattern::Design design;
  design.AddIcl("logic_enum.icl", R"(
Module M {
  Parameter N = 2;
  Enum E { blue = 2'b01; }
  DataInPort a[$N-1:0] { RefEnum E; }
  LogicSignal same { a == blue; }
  LogicSignal concat { a[1], a[0] != 2'b01; }
  LogicSignal unknown { !a[0]; }
}
)");
  const auto netlist = pattern::Elaborate(design, "M");
  const auto bits = [](const pattern::BitReference& bit) {
    return bit.index ? '0' : '1';
  };
  Require(pattern::EvaluateLogic(netlist.NodeAt("same"), bits) == '1' &&
              pattern::EvaluateLogic(netlist.NodeAt("concat"), bits) == '0',
          "enum and concatenation comparison");
  Require(pattern::EvaluateLogic(
              netlist.NodeAt("unknown"),
              [](const pattern::BitReference&) { return 'x'; }) == 'x',
          "unknown network state is not invented as a known value");
}
void TestKeywordOperands() {
  pattern::Design design;
  design.AddIcl("logic_keywords.icl",
                "Module M { DataInPort Source; LogicSignal c { Source; } }");
  const auto netlist = pattern::Elaborate(design, "M");
  Require(pattern::EvaluateLogic(
              netlist.NodeAt("c"),
              [](const pattern::BitReference&) { return '1'; }) == '1',
          "keyword operands are not interpreted as properties");
}

void TestLongExpressionLifetime() {
  pattern::Design design;
  std::string source = "Module M { DataInPort a; LogicSignal c { a";
  for (int i = 0; i < 20000; ++i) source += " | a";
  source += "; } }";
  design.AddIcl("long_logic.icl", source);
  // Parsing and destruction must remain safe even beyond elaboration limits.
  try {
    pattern::Elaborate(design, "M");
    throw std::runtime_error("missing logic depth limit");
  } catch (const std::exception& error) {
    Require(std::string(error.what()).find("depth limit") != std::string::npos,
            "deep logic has a bounded elaboration error");
  }
}

void TestWidthsAndMasks() {
  pattern::Design design;
  design.AddIcl("widths.icl", R"(
Module M {
  DataInPort a[3:0];
  LogicSignal concat { a == 1'b0, 2, 1'b1; }
  LogicSignal extended { a == 'b10; }
  LogicSignal masked { a == 4'b1xxx; }
  LogicSignal inverted { a == ~4'b1000; }
}
)");
  const auto netlist = pattern::Elaborate(design, "M");
  for (int a = 0; a < 16; ++a) {
    const auto value = [a](const pattern::BitReference& bit) {
      return ((a >> bit.index) & 1) ? '1' : '0';
    };
    const std::pair<const char*, bool> expectations[] = {{"concat", a == 5},
                                                         {"extended", a == 2},
                                                         {"masked", a >= 8},
                                                         {"inverted", a == 7}};
    for (const auto& expected : expectations) {
      Require(pattern::EvaluateLogic(netlist.NodeAt(expected.first), value) ==
                  (expected.second ? '1' : '0'),
              "inferred widths and masked comparisons");
    }
  }
}

void TestInvalidExpressions() {
  const std::pair<std::string, std::string> cases[] = {
      {"DataInPort a[3:0]; LogicSignal c { a == ~1; }", "unsized"},
      {"Enum E { value = 1'b1; } Alias a = E; LogicSignal c { a; }", "signal"},
      {"Enum E { value = 1'b1; } LogicSignal c { E; }", "data signal"},
      {"ScanInPort si; ScanInterface I { Port si; } LogicSignal c { I; }",
       "data signal"},
      {"Parameter P = 1'bx; LogicSignal c { $P; }", "unknown"},
      {"DataInPort a[1:0]; LogicSignal c { a && 1'b1; }", "width"},
      {"DataInPort a[1:0]; LogicSignal c { a == 1'b1; }", "width"},
      {"LogicSignal c { ^1; }", "width"},
      {"DataInPort a[3:0]; LogicSignal c { a == 1, 2; }", "unsized"},
      {"LogicSignal c { b; } LogicSignal b { c; }", "cycle"}};
  for (const auto& test : cases) {
    bool rejected = false;
    try {
      pattern::Design design;
      design.AddIcl("invalid_logic.icl", "Module M { " + test.first + " }");
      pattern::Elaborate(design, "M");
    } catch (const std::runtime_error& error) {
      Require(std::string(error.what()).find(test.second) != std::string::npos,
              "unexpected diagnosis: " + std::string(error.what()));
      rejected = true;
    }
    Require(rejected, "accepted invalid logic: " + test.first);
  }
}
void TestExpansionBudget() {
  pattern::Design design;
  std::string source = "Module M { DataInPort a[32767:0]; LogicSignal c { ";
  for (int i = 0; i < 64; ++i) {
    if (i) source += " || ";
    source += "a == 0";
  }
  source += "; } }";
  design.AddIcl("wide_logic.icl", source);
  bool rejected = false;
  try {
    pattern::Elaborate(design, "M");
  } catch (const std::runtime_error& error) {
    Require(std::string(error.what()).find("operand bit limit") !=
                std::string::npos,
            "bounded logic allocation has a specific resource diagnostic");
    rejected = true;
  }
  Require(rejected, "compact input exceeds aggregate expanded logic budget");
}
}  // namespace

int main() {
  try {
    TestStandardExamples();
    TestEnumsAndParameters();
    TestKeywordOperands();
    TestLongExpressionLifetime();
    TestWidthsAndMasks();
    TestInvalidExpressions();
    TestExpansionBudget();
    std::cout << "PASS: shared AST LogicSignal compilation and truth tables\n";
  } catch (const std::exception& error) {
    std::cerr << "FAIL: " << error.what() << '\n';
    return 1;
  }
}
