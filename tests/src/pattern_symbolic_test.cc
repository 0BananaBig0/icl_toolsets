// Copyright 2026 The ICL Toolsets Authors.
// SPDX-License-Identifier: MIT
#include "pattern/pattern_symbolic.h"

#include <iostream>
#include <stdexcept>

#include "pattern/pattern_design.h"
#include "pattern/pattern_state.h"
#include "pattern/pattern_values.h"

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
void TestOneHotPortValues() {
  pattern::Design design;
  design.AddIcl("one_hot.icl", R"(
Module Ports {
 DataInPort a[1:0]; DataInPort b[1:0];
 DataInPort ea; DataInPort eb;
 DataOutPort pa[1:0] { Source a; Enable ea; }
 DataOutPort forwarded[1:0] { Source pa; }
 DataOutPort pb[1:0] { Source b; Enable eb; }
 OneHotDataGroup group[0:2] { Port forwarded; Port pb; }
 DataOutPort out[2:0] { Source group; }
})");
  const auto netlist = pattern::Elaborate(design, "Ports");
  z3::context context;
  for (char ea : std::string("01x")) {
    for (char eb : std::string("01x")) {
      pattern::NetworkState native(netlist);
      std::map<size_t, pattern::SymbolicValue> assignments;
      for (const auto& [name, value] : std::map<std::string, std::string>{
               {"a", "10"}, {"b", "01"}, {"ea", {ea}}, {"eb", {eb}}}) {
        const auto id = netlist.node_ids.at(name);
        native.Drive(id, value);
        assignments.emplace(id,
                            pattern::SymbolicValue::Literal(context, value));
      }
      const std::string expected = ea == '1' && eb == '0'   ? "x10"
                                   : ea == '0' && eb == '1' ? "x01"
                                                            : "xxx";
      pattern::SymbolicNetwork symbolic(netlist, context, assignments);
      for (const auto* name : {"group", "out"}) {
        const auto id = netlist.node_ids.at(name);
        Require(native.Value(id) == expected,
                "one-hot unique selection and unknown padding truth table");
        const auto value = symbolic.Value(id);
        const auto want = pattern::SymbolicValue::Literal(context, expected);
        z3::solver solver(context);
        solver.add(value.bits != want.bits || value.known != want.known);
        Require(solver.check() == z3::unsat,
                "symbolic one-hot value matches independent truth table");
      }
    }
  }
}
void TestOneHotScanValues() {
  pattern::Design design;
  design.AddIcl("scan-values.icl", R"(
Module ScanValues {
  ScanInPort si;
  DataInPort ea; DataInPort eb; DataInPort ec;
  ScanRegister a { ScanInSource si; CaptureSource 1'b0; }
  ScanRegister b { ScanInSource si; CaptureSource 1'b1; }
  ScanOutPort pa { Source a; Enable ea; }
  ScanOutPort pb { Source b; Enable eb; }
  ScanOutPort pc { Source si; Enable ec; }
  OneHotScanGroup inner { Port pa; Port pb; }
  ScanOutPort forwarded { Source inner; }
  OneHotScanGroup outer { Port ~forwarded; Port pc; }
  ScanOutPort so { Source outer; }
})");
  const auto netlist = pattern::Elaborate(design, "ScanValues");
  z3::context context;
  for (char ea : std::string("01x")) {
    for (char eb : std::string("01x")) {
      for (char ec : std::string("01x")) {
        pattern::NetworkState native(netlist);
        std::map<size_t, pattern::SymbolicValue> assignments;
        for (const auto& [name, value] : std::map<std::string, std::string>{
                 {"si", "0"}, {"ea", {ea}}, {"eb", {eb}}, {"ec", {ec}}}) {
          const auto id = netlist.node_ids.at(name);
          native.Drive(id, value);
          assignments.emplace(id,
                              pattern::SymbolicValue::Literal(context, value));
        }
        const auto a = netlist.node_ids.at("a"), b = netlist.node_ids.at("b");
        native.Assume(a, "1");
        native.Assume(b, "0");
        native.Clock({{a, {'1', '0', '0', '1'}}, {b, {'1', '0', '0', '1'}}},
                     true);
        assignments.emplace(a, pattern::SymbolicValue::Literal(context, "0"));
        assignments.emplace(b, pattern::SymbolicValue::Literal(context, "1"));
        const std::string expected = ea == '1' && eb == '0' && ec == '0'   ? "1"
                                     : ea == '0' && eb == '1' && ec == '0' ? "0"
                                     : ea == '0' && eb == '0' && ec == '1'
                                         ? "0"
                                         : "x";
        const auto id = netlist.node_ids.at("so");
        Require(native.Value(id) == expected,
                "nested one-hot scan uses shift state and exclusive enables");
        pattern::SymbolicNetwork symbolic(netlist, context, assignments);
        const auto value = symbolic.Value(id);
        const auto want = pattern::SymbolicValue::Literal(context, expected);
        z3::solver solver(context);
        solver.add(value.bits != want.bits || value.known != want.known);
        Require(solver.check() == z3::unsat,
                "symbolic nested one-hot scan truth table");
        std::map<size_t, pattern::SymbolicValue> shifts{{a, assignments.at(a)},
                                                        {b, assignments.at(b)}};
        assignments.at(a) = pattern::SymbolicValue::Literal(context, "1");
        assignments.at(b) = pattern::SymbolicValue::Literal(context, "0");
        pattern::SymbolicNetwork separate(netlist, context, assignments, "xx",
                                          &shifts);
        const auto serial = separate.Value(id);
        z3::solver serial_solver(context);
        serial_solver.add(serial.bits != want.bits ||
                          serial.known != want.known);
        Require(serial_solver.check() == z3::unsat,
                "one-hot scan uses shift storage while guards use held values");
      }
    }
  }
}
void TestSeparateShiftState() {
  pattern::Design design;
  design.AddIcl("shift-state.icl", R"(
Module ShiftState {
  ScanInPort si;
  ScanRegister a[1:0] { ScanInSource si; CaptureSource 2'b10; }
  ScanRegister b[1:0] { ScanInSource a[0]; CaptureSource 2'b01; }
  Alias inverted = ~a[0];
  ScanMux mux SelectedBy a[1] { 1'b0: inverted; 1'b1: b[0]; }
  ScanOutPort so { Source mux; }
  DataOutPort data { Source inverted; }
})");
  const auto netlist = pattern::Elaborate(design, "ShiftState");
  z3::context context;
  const auto a = netlist.node_ids.at("a"), b = netlist.node_ids.at("b");
  pattern::NetworkState native(netlist);
  native.SetRegister(a, "10", "01");
  native.SetRegister(b, "01", "10");
  std::map<size_t, pattern::SymbolicValue> shifts{
      {a, pattern::SymbolicValue::Literal(context, "10")},
      {b, pattern::SymbolicValue::Literal(context, "01")}};
  pattern::SymbolicNetwork symbolic(
      netlist, context,
      {{a, pattern::SymbolicValue::Literal(context, "01")},
       {b, pattern::SymbolicValue::Literal(context, "10")}},
      "xx", &shifts);
  for (bool scan : {false, true, false}) {
    for (const auto* name : {"a", "b", "inverted", "mux", "so", "data"}) {
      const size_t id = netlist.node_ids.at(name);
      const auto actual = symbolic.Value(id, scan);
      const auto expected =
          pattern::SymbolicValue::Literal(context, native.Value(id, scan));
      z3::solver solver(context);
      solver.add(actual.bits != expected.bits ||
                 actual.known != expected.known);
      Require(solver.check() == z3::unsat,
              "symbolic held/shift views must agree with concrete serial "
              "semantics");
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
       {"0b101", "-1", "1'b1", "$value", "0x", "0o1", "1_0", "Active"}) {
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
 Enum register_values { Active = 8'h96; Low = 8'h03; Mask = 8'bxxxxxx01; }
 Enum alias_values { Active = 4'b0101; }
 ScanInPort si;
 ScanRegister r[0:7] { ScanInSource si; RefEnum register_values; }
 Alias a[3:0] = ~r[4:7] { RefEnum alias_values; }
 Alias plain[3:0] = r[4:7];
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
  const auto value = [&](const std::string& name, const std::string& literal) {
    return pattern::PdlDataValue(
        context, netlist, pattern::ResolvePdlTarget(netlist, name), literal);
  };
  Require(value("r", "Active") == "10010110" &&
              value("a", "Active") == "0101" && value("a", "0xa") == "1010",
          "register and alias enums resolve against their own RefEnum");
  Require(value("r(7:4)", "Low") == "0011" && value("r[6:7]", "Mask") == "01",
          "enum values follow numeric sizing without reordering the literal");
  for (const auto& [name, literal] :
       {std::pair{"r", "Missing"}, {"plain", "Active"}, {"r[6:7]", "Active"}}) {
    bool rejected = false;
    try {
      value(name, literal);
    } catch (const std::exception&) {
      rejected = true;
    }
    Require(rejected,
            "unknown/unattached enums and significant truncation fail");
  }
}
}  // namespace
int main() {
  try {
    TestTernaryAgreement();
    TestOneHotPortValues();
    TestOneHotScanValues();
    TestSeparateShiftState();
    TestJustification();
    TestPdlOperands();
    std::cout << "pattern symbolic tests passed\n";
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
