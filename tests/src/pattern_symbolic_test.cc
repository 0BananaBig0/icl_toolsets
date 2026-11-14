// Copyright 2026 The ICL Toolsets Authors.
// SPDX-License-Identifier: MIT
#include "pattern/pattern_symbolic.h"

#include <chrono>
#include <iostream>
#include <limits>
#include <stdexcept>

#include "pattern/pattern_design.h"
#include "pattern/pattern_state.h"
#include "pattern/pattern_values.h"

namespace {
void Require(bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}
void TestBooleanMuxIdentity() {
  pattern::Design design;
  design.AddIcl("mux-identity.icl", R"(Module Identity {
    ResetPort global; DataInPort select;
    DataMux gate SelectedBy select { 1'b0: global; 1'b1: 1'b1; }
    ToResetPort out { Source gate; }
    DataInPort a[1:0]; DataInPort b[0:1];
    DataMux vector[1:0] SelectedBy select { 1'b0: a; 1'b1: ~b; }
    DataMux reversed[1:0] SelectedBy select { 1'b1: ~b; 1'b0: a; }
  })");
  const auto netlist = pattern::Elaborate(design, "Identity");
  z3::context context;
  for (bool fully_known : {false, true}) {
    const auto bits = context.bv_const("select_bits", 1);
    const auto known = fully_known ? context.bv_val(1, 1)
                                   : context.bv_const("select_known", 1);
    pattern::SymbolicNetwork values(
        netlist, context,
        {{netlist.node_ids.at("global"),
          pattern::SymbolicValue::Literal(context, "0")},
         {netlist.node_ids.at("select"), {bits, known}}});
    const auto expected = values.Value(netlist.node_ids.at("select"));
    const auto result = values.Value(netlist.node_ids.at("gate"));
    z3::solver solver(context);
    solver.add(result.bits != expected.bits || result.known != expected.known);
    Require(solver.check() == z3::unsat,
            "inactive global reset plus assert-one mux preserves select and X");
  }
  for (char select : std::string("01x")) {
    for (char a0 : std::string("01x")) {
      for (char a1 : std::string("01x")) {
        for (char b0 : std::string("01x")) {
          for (char b1 : std::string("01x")) {
            const std::map<std::string, std::string> inputs{
                {"select", {select}}, {"a", {a0, a1}}, {"b", {b0, b1}}};
            pattern::NetworkState concrete(netlist);
            std::map<size_t, pattern::SymbolicValue> assignments;
            for (const auto& [name, input] : inputs) {
              const auto id = netlist.node_ids.at(name);
              concrete.Drive(id, input);
              assignments.emplace(
                  id, pattern::SymbolicValue::Literal(context, input));
            }
            pattern::SymbolicNetwork symbolic(netlist, context,
                                              std::move(assignments));
            for (const std::string name : {"vector", "reversed"}) {
              const auto id = netlist.node_ids.at(name);
              const auto expected =
                  pattern::SymbolicValue::Literal(context, concrete.Value(id));
              const auto result = symbolic.Value(id);
              Require(
                  z3::eq(result.bits.simplify(), expected.bits) &&
                      z3::eq(result.known.simplify(), expected.known),
                  "binary vector mux preserves inverted and unknown inputs");
            }
          }
        }
      }
    }
  }
}
void TestMalformedMuxWidth() {
  pattern::Design design;
  design.AddIcl("mux-width.icl", R"(Module Width {
    DataInPort select;
    DataMux gate SelectedBy select { 1'b0: 1'b0; 1'b1: 1'b1; }
  })");
  auto netlist = pattern::Elaborate(design, "Width");
  const auto gate = netlist.node_ids.at("gate");
  netlist.nodes.at(gate).left = 1;
  netlist.nodes.at(gate).indexed = true;
  z3::context context;
  bool rejected = false;
  try {
    pattern::SymbolicNetwork symbolic(
        netlist, context,
        {{netlist.node_ids.at("select"),
          pattern::SymbolicValue::Literal(context, "0")}});
    symbolic.Value(gate);
  } catch (const z3::exception&) {
    rejected = true;
  }
  Require(rejected, "malformed mux branch widths cannot change declared width");
}
void TestMuxChainScale() {
  for (bool selector_chain : {false, true}) {
    for (unsigned stages : {512U, 4096U}) {
      std::string source = "Module Scale { DataInPort value; ";
      if (!selector_chain) {
        source += "DataInPort mode[" + std::to_string(stages - 1) + ":0]; ";
      }
      std::string previous = "value";
      for (unsigned i = 0; i < stages; ++i) {
        const auto name = "m" + std::to_string(i);
        source +=
            "DataMux " + name + " SelectedBy " +
            (selector_chain ? previous : "mode[" + std::to_string(i) + "]") +
            " { 1'b0: " + (selector_chain ? "1'b0" : previous) +
            "; 1'b1: " + (selector_chain ? "1'b1" : "1'b0") + "; }\n";
        previous = name;
      }
      source += "DataOutPort out { Source " + previous + "; } }";
      pattern::Design design;
      design.AddIcl("mux-chain.icl", source);
      const auto netlist = pattern::Elaborate(design, "Scale");
      z3::context context;
      const auto value = context.bv_const("value", 1);
      const auto mode = context.bv_const("mode", stages);
      std::map<size_t, pattern::SymbolicValue> assignments{
          {netlist.node_ids.at("value"), {value, context.bv_val(1, 1)}}};
      if (!selector_chain) {
        assignments.emplace(
            netlist.node_ids.at("mode"),
            pattern::SymbolicValue{mode, ~context.bv_val(0, stages)});
      }
      const auto start = std::chrono::steady_clock::now();
      pattern::SymbolicNetwork symbolic(netlist, context,
                                        std::move(assignments));
      const auto actual = symbolic.Value(netlist.node_ids.at("out"));
      const auto built = std::chrono::steady_clock::now();
      const auto expected = selector_chain
                                ? value
                                : z3::ite(mode == context.bv_val(0, stages),
                                          value, context.bv_val(0, 1));
      z3::solver solver(context);
      z3::params parameters(context);
      parameters.set("timeout", 2000U);
      solver.set(parameters);
      solver.add(actual.bits != expected ||
                 actual.known != context.bv_val(1, 1));
      Require(solver.check() == z3::unsat,
              "deep binary mux chain agrees with independent closed form");
      std::cout << "binary mux chain: selector=" << selector_chain
                << " stages=" << stages << " bytes=" << source.size()
                << " construct_seconds="
                << std::chrono::duration<double>(built - start).count()
                << " check_seconds="
                << std::chrono::duration<double>(
                       std::chrono::steady_clock::now() - built)
                       .count()
                << std::endl;
    }
  }
}
void TestMuxConsensus() {
  pattern::Design design;
  design.AddIcl("mux-consensus.icl", R"(
Module Consensus {
 DataInPort sel[2:0];
 DataMux complete[1:0] SelectedBy sel { 3'b0xx: 2'b10; 3'b1xx: 2'b10; }
 DataMux partial[1:0] SelectedBy sel { 3'b00x: 2'b10; 3'b01x: 2'b10; 3'b10x: 2'b11; }
 DataMux overlap[1:0] SelectedBy sel {
  3'b0xx: 2'b10; 3'bx0x: 2'b11; 3'b1xx: 2'b1x; 3'bx1x: 2'b10;
 }
}
)");
  const auto netlist = pattern::Elaborate(design, "Consensus");
  const auto selector = netlist.node_ids.at("sel");
  z3::context context;
  const auto bits = context.bv_const("selector_bits", 3);
  const auto known = context.bv_const("selector_known", 3);
  pattern::SymbolicNetwork symbolic(netlist, context,
                                    {{selector, {bits, known}}});
  for (char first : std::string("01x"))
    for (char second : std::string("01x"))
      for (char third : std::string("01x")) {
        const std::string input{first, second, third};
        pattern::NetworkState uncertain(netlist);
        uncertain.Drive(selector, input);
        for (const std::string name : {"complete", "partial", "overlap"}) {
          const auto id = netlist.node_ids.at(name);
          std::string expected;
          for (size_t value = 0; value < 8; ++value) {
            std::string candidate(3, '0');
            bool compatible = true;
            for (size_t bit = 0; bit < 3; ++bit) {
              candidate[bit] = (value >> (2 - bit)) & 1 ? '1' : '0';
              compatible = compatible &&
                           (input[bit] == 'x' || input[bit] == candidate[bit]);
            }
            if (!compatible) continue;
            pattern::NetworkState concrete(netlist);
            concrete.Drive(selector, candidate);
            const auto result = concrete.Value(id);
            if (expected.empty()) expected = result;
            for (size_t bit = 0; bit < expected.size(); ++bit) {
              if (expected[bit] != result[bit]) expected[bit] = 'x';
            }
          }
          Require(uncertain.Value(id) == expected,
                  "mux unknown selector must join every binary completion");
          const auto input_value =
              pattern::SymbolicValue::Literal(context, input);
          const auto wanted =
              pattern::SymbolicValue::Literal(context, expected);
          const auto result = symbolic.Value(id);
          z3::solver solver(context);
          solver.add(bits == input_value.bits && known == input_value.known);
          solver.add(result.bits != wanted.bits ||
                     result.known != wanted.known);
          Require(solver.check() == z3::unsat,
                  "symbolic mux consensus must match the completion oracle");
        }
      }
}
void TestMuxCoverageBudget() {
  constexpr size_t kWidth = 4096;
  pattern::Design design;
  design.AddIcl("wide-mux.icl",
                "Module Wide { DataInPort sel[4095:0]; "
                "DataMux out SelectedBy sel { 4096'b" +
                    std::string(kWidth, '0') + ": 1'b1; } }");
  const auto netlist = pattern::Elaborate(design, "Wide");
  const auto selector = netlist.node_ids.at("sel");
  const auto output = netlist.node_ids.at("out");
  pattern::NetworkState state(netlist);
  state.Drive(selector, std::string(kWidth, '0'));
  Require(state.Value(output) == "1", "known wide selector retains fast path");
  z3::context context;
  pattern::SymbolicNetwork known(
      netlist, context,
      {{selector,
        pattern::SymbolicValue::Literal(context, std::string(kWidth, '0'))}});
  Require(known.Value(output).known.simplify().get_numeral_uint() == 1,
          "known symbolic selector needs no coverage expansion");
  state.Drive(selector, std::string(kWidth, 'x'));
  bool native_limited = false, symbolic_limited = false;
  try {
    state.Value(output);
  } catch (const std::runtime_error& error) {
    native_limited =
        std::string(error.what()).find("mux coverage resource limit") !=
        std::string::npos;
  }
  try {
    pattern::SymbolicNetwork unknown(
        netlist, context,
        {{selector,
          pattern::SymbolicValue::Literal(context, std::string(kWidth, 'x'))}});
    unknown.Value(output);
  } catch (const std::runtime_error& error) {
    symbolic_limited =
        std::string(error.what()).find("mux coverage resource limit") !=
        std::string::npos;
  }
  Require(native_limited && symbolic_limited,
          "wide sparse mux coverage fails within the character-work budget");
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
void TestSignalRunAgreement() {
  pattern::Design design;
  design.AddIcl("signal_runs.icl", R"(
Module Runs {
  DataInPort down[8:3]; DataInPort up[2:7];
  DataOutPort whole_down[5:0] { Source down; }
  DataOutPort whole_up[0:5] { Source up; }
  DataOutPort slice_down[3:0] { Source down[7:4]; }
  DataOutPort slice_up[0:3] { Source up[3:6]; }
  DataOutPort reverse_down[5:0] { Source down[3:8]; }
  DataOutPort reverse_up[5:0] { Source up[7:2]; }
  DataOutPort mixed[8:0] { Source down[8:6], ~up[3:5], 1'bx, down[3], up[7]; }
  DataOutPort duplicate[2:0] { Source down[6], down[6], down[6]; }
}
)");
  const auto netlist = pattern::Elaborate(design, "Runs");
  z3::context context;
  for (const std::string down : {"000000", "111111", "10x011", "xxxxxx"}) {
    for (const std::string up : {"000000", "111111", "x1010x", "xxxxxx"}) {
      pattern::NetworkState native(netlist);
      native.Drive(netlist.node_ids.at("down"), down);
      native.Drive(netlist.node_ids.at("up"), up);
      pattern::SymbolicNetwork symbolic(
          netlist, context,
          {{netlist.node_ids.at("down"),
            pattern::SymbolicValue::Literal(context, down)},
           {netlist.node_ids.at("up"),
            pattern::SymbolicValue::Literal(context, up)}},
          "10");
      for (const auto* name :
           {"whole_down", "whole_up", "slice_down", "slice_up", "reverse_down",
            "reverse_up", "mixed", "duplicate"}) {
        const auto result = symbolic.Value(netlist.node_ids.at(name));
        const auto expected = pattern::SymbolicValue::Literal(
            context, native.Value(netlist.node_ids.at(name)));
        z3::solver solver(context);
        solver.add(result.bits != expected.bits ||
                   result.known != expected.known);
        Require(
            solver.check() == z3::unsat,
            "symbolic signal runs preserve concrete bit order and unknowns");
      }
      const size_t missing = std::numeric_limits<size_t>::max();
      const auto result = symbolic.Bits(
          {{netlist.node_ids.at("down"), 8},
           {missing, 0, 'x', false, pattern::VirtualSignal::kGlobalReset},
           {netlist.node_ids.at("down"), 7},
           {missing, 0, 'x', false, pattern::VirtualSignal::kAsyncGlobalReset},
           {missing, 0, '?'}});
      const auto expected = pattern::SymbolicValue::Literal(
          context, std::string{down[0], '1', down[1], '0', 'x'});
      z3::solver solver(context);
      solver.add(result.bits != expected.bits ||
                 result.known != expected.known);
      Require(solver.check() == z3::unsat,
              "virtual and unspecified references interrupt physical runs");
      for (const int64_t index : {2, 9}) {
        bool rejected = false;
        try {
          symbolic.Bits({{netlist.node_ids.at("down"), 8},
                         {netlist.node_ids.at("down"), index}});
        } catch (const std::runtime_error& error) {
          rejected = std::string(error.what()).find("index outside range") !=
                     std::string::npos;
        }
        Require(rejected, "run grouping retains reference range validation");
      }
    }
  }
}

void TestOneHotPortValues() {
  pattern::Design design;
  design.AddIcl("one_hot.icl", R"(
Module Producer {
 DataInPort value[1:0]; DataInPort enable;
 DataOutPort out[1:0] { Source value; Enable enable; }
}
Module Forwarder {
 DataInPort value[1:0]; DataInPort enable;
 Instance producer Of Producer { InputPort value = value; InputPort enable = enable; }
 DataOutPort forwarded[1:0] { Source producer.out; }
}
Module Ports {
 DataInPort a[1:0]; DataInPort b[1:0];
 DataInPort ea; DataInPort eb;
 Instance first Of Forwarder { InputPort value = a; InputPort enable = ea; }
 Instance second Of Producer { InputPort value = b; InputPort enable = eb; }
 OneHotDataGroup group[0:2] { Port first.forwarded; Port second.out; }
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
Module EnabledScanOutput {
  ScanInPort incoming;
  DataInPort enable;
  ScanOutPort out { Source incoming; Enable enable; }
}
Module ScanForward {
  ScanInPort incoming;
  ScanOutPort out { Source incoming; }
}
Module ScanValues {
  ScanInPort si;
  DataInPort ea; DataInPort eb; DataInPort ec;
  ScanRegister a { ScanInSource si; CaptureSource 1'b0; }
  ScanRegister b { ScanInSource si; CaptureSource 1'b1; }
  Instance pa Of EnabledScanOutput { InputPort incoming = a; InputPort enable = ea; }
  Instance pb Of EnabledScanOutput { InputPort incoming = b; InputPort enable = eb; }
  Instance pc Of EnabledScanOutput { InputPort incoming = si; InputPort enable = ec; }
  OneHotScanGroup inner { Port pa.out; Port pb.out; }
  Instance forwarded Of ScanForward { InputPort incoming = inner; }
  OneHotScanGroup outer { Port ~forwarded.out; Port pc.out; }
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
    for (const size_t id : {a, b}) {
      for (bool inverted : {false, true}) {
        const auto actual = symbolic.Bits(
            {{id, 1, 'x', inverted}, {id, 0, 'x', inverted}}, scan);
        auto bits = native.Value(id, scan);
        if (inverted) {
          for (char& bit : bits)
            bit = bit == '0' ? '1' : bit == '1' ? '0' : 'x';
        }
        const auto expected = pattern::SymbolicValue::Literal(context, bits);
        z3::solver solver(context);
        solver.add(actual.bits != expected.bits ||
                   actual.known != expected.known);
        Require(solver.check() == z3::unsat,
                "contiguous register runs preserve distinct held/shift views");
      }
    }
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
int main(int argc, char* argv[]) {
  try {
    if (argc != 1) {
      Require(argc == 2 && std::string(argv[1]) == "--mux-scale",
              "expected --mux-scale or no test arguments");
      TestMuxChainScale();
      return 0;
    }
    TestBooleanMuxIdentity();
    TestMalformedMuxWidth();
    TestMuxChainScale();
    TestMuxConsensus();
    TestMuxCoverageBudget();
    TestTernaryAgreement();
    TestSignalRunAgreement();
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
