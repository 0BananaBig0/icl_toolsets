// Copyright 2026 The ICL Toolsets Authors.
// SPDX-License-Identifier: MIT
#include "pattern_symbolic.h"

#include <iostream>
#include <stdexcept>

#include "pattern_design.h"
#include "pattern_state.h"
#include "pattern_values.h"

namespace {
void Require(bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}
void TestTernaryAgreement() {
  pattern::Design design;
  design.AddIcl("symbolic.icl", R"(
Module Signals {
  DataInPort a[1:0]; DataInPort b[1:0];
  LogicSignal logical { (a[1] & ~a[0]) | b[0]; }
  LogicSignal parity { ^a; }
  LogicSignal any { |a; }
  LogicSignal all { &a; }
  LogicSignal masked { a == 2'b1x; }
  LogicSignal negated { a != 2'b0x; }
  DataMux route SelectedBy a[1] { 1'b0: ~b[0]; 1'b1: b[1]; }
  DataOutPort out { Source route; }
})");
  const auto netlist = pattern::Elaborate(design, "Signals");
  z3::context context;
  for (char a0 : std::string("01x"))
    for (char a1 : std::string("01x")) {
      for (char b0 : std::string("01x"))
        for (char b1 : std::string("01x")) {
          const std::string a{a1, a0}, b{b1, b0};
          pattern::NetworkState native(netlist);
          native.Drive(netlist.node_ids.at("a"), a);
          native.Drive(netlist.node_ids.at("b"), b);
          pattern::SymbolicNetwork symbolic(
              netlist, context,
              {{netlist.node_ids.at("a"),
                pattern::SymbolicValue::Literal(context, a)},
               {netlist.node_ids.at("b"),
                pattern::SymbolicValue::Literal(context, b)}});
          for (const auto* name : {"logical", "parity", "any", "all", "masked",
                                   "negated", "out"}) {
            const auto id = netlist.node_ids.at(name);
            const auto result = symbolic.Value(id);
            const auto expected =
                pattern::SymbolicValue::Literal(context, native.Value(id));
            z3::solver solver(context);
            solver.add(result.known != expected.known ||
                       result.bits != expected.bits);
            Require(solver.check() == z3::unsat,
                    "symbolic and concrete three-valued evaluation disagree");
          }
        }
    }
}
void TestJustification() {
  pattern::Design design;
  design.AddIcl("justify.icl", R"(
Module Justify {
  ScanInPort si;
  ScanRegister select[1:0] { ScanInSource si; }
  LogicSignal enabled { select == 2'b10; }
  DataMux result SelectedBy enabled { 1'b1: 1'b1; 1'b0: 1'b0; }
})");
  const auto netlist = pattern::Elaborate(design, "Justify");
  z3::context context;
  const auto choice = context.bv_const("select", 2);
  pattern::SymbolicNetwork symbolic(
      netlist, context,
      {{netlist.node_ids.at("select"), {choice, context.bv_val(3, 2)}}});
  const auto result = symbolic.Value(netlist.node_ids.at("result"));
  z3::solver solver(context);
  solver.add(result.known == context.bv_val(1, 1) &&
             result.bits == context.bv_val(1, 1));
  Require(
      solver.check() == z3::sat &&
          solver.get_model().eval(choice).get_numeral_uint() == 2,
      "solver justifies register controls through existing compiled netlist");
  pattern::SymbolicNetwork unknown(netlist, context, {});
  const auto uninitialized = unknown.Value(netlist.node_ids.at("result"));
  solver.reset();
  solver.add(uninitialized.known == context.bv_val(1, 1));
  Require(solver.check() == z3::unsat,
          "unknown hardware state is not a free solver choice");
}

void TestPdlOperands() {
  z3::context context;
  Require(pattern::PdlNumber(context, "0x96", 16) == "0000000010010110",
          "PDL numeric zero extension");
  Require(pattern::PdlNumber(context, "0bx01", 8) == "xxxxxx01",
          "PDL numeric X extension");
  Require(pattern::PdlNumber(context, "0bxx001", 2) == "01",
          "extra leading zero/X bits may be truncated");
  Require(pattern::PdlNumber(context, "18446744073709551616", 65) ==
              "1" + std::string(64, '0'),
          "decimal PDL values are not limited to machine integers");
  for (const auto* value :
       {"0b101", "-1", "1'b1", "$value", "0x", "0o1", "1_0"}) {
    bool rejected = false;
    try {
      pattern::PdlNumber(context, value, 2);
    } catch (const std::exception&) {
      rejected = true;
    }
    Require(rejected, "invalid PDL values and significant truncation rejected");
  }
  pattern::Design design;
  design.AddIcl("operands.icl", R"(
Module Targets {
 ScanInPort si;
 ScanRegister r[0:7] { ScanInSource si; }
 Alias a[3:0] = ~r[4:7];
})");
  const auto netlist = pattern::Elaborate(design, "Targets");
  const auto target = pattern::ResolvePdlTarget(netlist, "a[2:1]");
  Require(target.bits.size() == 2 && target.bits[0].index == 5 &&
              target.bits[1].index == 6 && target.bits[0].inverted,
          "PDL alias ranges preserve declaration order and polarity");
  const auto reverse = pattern::ResolvePdlTarget(netlist, "r(7:4)");
  Require(reverse.bits.size() == 4 && reverse.bits[0].index == 7 &&
              reverse.bits[3].index == 4,
          "legacy PDL parentheses preserve explicitly reversed bit order");
}
}  // namespace
int main() {
  try {
    TestTernaryAgreement();
    TestJustification();
    TestPdlOperands();
    std::cout << "pattern symbolic tests passed\n";
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
