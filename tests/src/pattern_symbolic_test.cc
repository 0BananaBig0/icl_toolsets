// Copyright 2026 The ICL Toolsets Authors.
// SPDX-License-Identifier: MIT
#include "pattern/pattern_symbolic.h"

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <utility>

#include "pattern/pattern_design.h"
#include "pattern/pattern_state.h"
#include "pattern/pattern_values.h"

namespace {
void Require(bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

void TestImpliedToTckValues() {
  pattern::Design design;
  design.AddIcl("implied_clock_values.icl",
                "Module Child { TCKPort ck[3]; ToTCKPort out[0:1]; } "
                "Module M { TCKPort ck[7]; ToTCKPort out[3:0]; "
                "Instance child Of Child { InputPort ck[3] = ck[7]; } }");
  const auto netlist = pattern::Elaborate(design, "M");
  z3::context context;
  for (char clock : std::string("01x")) {
    pattern::SymbolicNetwork state(
        netlist, context,
        {{netlist.node_ids.at("ck"),
          pattern::SymbolicValue::Literal(context, std::string(1, clock))}});
    for (const std::string name : {"out", "child.out"}) {
      const auto id = netlist.node_ids.at(name);
      const auto got = state.Value(id);
      const auto want = pattern::SymbolicValue::Literal(
          context, std::string(netlist.nodes.at(id).Width(), clock));
      z3::solver solver(context);
      solver.add(got.bits != want.bits || got.known != want.known);
      Require(solver.check() == z3::unsat,
              "implied ToTCK scalar forwarding preserves symbolic values and "
              "unknowns");
    }
  }
}

void TestRepeatedSignalRuns() {
  for (size_t width : {size_t{512}, size_t{8192}, size_t{65536}}) {
    pattern::Design design;
    design.AddIcl("repeated_signal.icl",
                  "Module M { DataInPort source; DataOutPort out[" +
                      std::to_string(width - 1) + ":0] { Source " +
                      std::to_string(width) + "'b0; } }");
    auto netlist = pattern::Elaborate(design, "M");
    const auto input = netlist.node_ids.at("source");
    const auto output = netlist.node_ids.at("out");
    for (bool inverted : {false, true}) {
      netlist.nodes.at(output).inputs.at("Source").assign(
          width, {input, 0, 'x', inverted});
      z3::context context;
      const auto bit = context.bv_const("repeated_bit", 1);
      pattern::SymbolicNetwork values(netlist, context,
                                      {{input, {bit, context.bv_val(1, 1)}}});
      const auto started = std::chrono::steady_clock::now();
      const auto got = values.Value(output);
      const auto want = (inverted ? ~bit : bit).repeat(width);
      z3::solver solver(context);
      solver.set("timeout", 3000u);
      solver.add(got.bits != want ||
                 got.known != ~context.bv_val(0, static_cast<unsigned>(width)));
      const auto result = solver.check();
      const auto seconds = std::chrono::duration<double>(
                               std::chrono::steady_clock::now() - started)
                               .count();
      std::cout << "repeated signal: bits=" << width << " inverted=" << inverted
                << " seconds=" << seconds << " result=" << result << '\n';
      Require(result == z3::unsat,
              "repeated scalar references retain symbolic values within the "
              "proof budget");
    }
  }
}

void TestLiteralWordBoundaries() {
  z3::context context;
  for (size_t width :
       {size_t{1}, size_t{63}, size_t{64}, size_t{65}, size_t{127}, size_t{128},
        size_t{129}, size_t{257}, size_t{8193}}) {
    for (const std::string pattern : {"0", "1", "x", "01x", "10x0011"}) {
      std::string input;
      for (size_t i = 0; i < width; ++i) input += pattern[i % pattern.size()];
      const auto got = pattern::SymbolicValue::Literal(context, input);
      z3::expr_vector different(context);
      for (size_t begin = 0; begin < width; begin += 64) {
        const unsigned size = std::min(size_t{64}, width - begin);
        uint64_t bits = 0, known = 0;
        for (size_t i = begin; i < begin + size; ++i) {
          bits = (bits << 1) | (input[i] == '1');
          known = (known << 1) | (input[i] != 'x');
        }
        const unsigned high = width - begin - 1;
        const unsigned low = width - begin - size;
        different.push_back(got.bits.extract(high, low) !=
                            context.bv_val(bits, size));
        different.push_back(got.known.extract(high, low) !=
                            context.bv_val(known, size));
      }
      z3::solver solver(context);
      solver.add(z3::mk_or(different));
      Require(
          solver.check() == z3::unsat,
          "literal word boundaries retain bit order, masks and odd lengths");
    }
  }
}

void TestSparseOneHotAgreement() {
  std::string text;
  for (size_t width : {size_t{1}, size_t{3}, size_t{7}}) {
    text += "Module P" + std::to_string(width) + " { DataInPort value[" +
            std::to_string(width - 1) +
            ":0]; DataInPort enabled; DataOutPort out[" +
            std::to_string(width - 1) +
            ":0] { Source value; Enable enabled; } } ";
  }
  text += "Module M { DataInPort data[6:0]; DataInPort enable[2:0]; ";
  size_t lane = 0;
  for (size_t width : {size_t{1}, size_t{3}, size_t{7}}) {
    text += "Instance p" + std::to_string(width) + " Of P" +
            std::to_string(width) + " { InputPort value = data[" +
            std::to_string(width - 1) + ":0]; InputPort enabled = enable[" +
            std::to_string(lane++) + "]; } ";
  }
  text +=
      "OneHotDataGroup down[9:0] { Port p3.out; Port p1.out; Port ~p7.out; } "
      "OneHotDataGroup up[0:9] { Port ~p7.out; Port p1.out; Port p3.out; } }";
  pattern::Design design;
  design.AddIcl("sparse_onehot_values.icl", text);
  auto netlist = pattern::Elaborate(design, "M");
  auto& unsorted = netlist.nodes.at(netlist.node_ids.at("up")).one_hot_sources;
  std::reverse(unsorted.begin(), unsorted.end());
  z3::context context;
  for (const std::string data : {"10x0011", "0000000", "1111111", "xxxxxxx"}) {
    for (char a : std::string("01x")) {
      for (char b : std::string("01x")) {
        for (char c : std::string("01x")) {
          const std::string enables{c, b, a};
          std::string expected(10, 'x');
          if (enables.find('x') == std::string::npos &&
              std::count(enables.begin(), enables.end(), '1') == 1) {
            const size_t width = a == '1' ? 1 : b == '1' ? 3 : 7;
            auto payload = data.substr(data.size() - width);
            if (c == '1') {
              for (auto& bit : payload) {
                bit = bit == '0' ? '1' : bit == '1' ? '0' : 'x';
              }
            }
            expected = std::string(10 - width, 'x') + payload;
          }
          pattern::NetworkState concrete(netlist);
          concrete.Drive(netlist.node_ids.at("data"), data);
          concrete.Drive(netlist.node_ids.at("enable"), enables);
          pattern::SymbolicNetwork symbolic(
              netlist, context,
              {{netlist.node_ids.at("data"),
                pattern::SymbolicValue::Literal(context, data)},
               {netlist.node_ids.at("enable"),
                pattern::SymbolicValue::Literal(context, enables)}});
          for (const std::string group : {"up", "down"}) {
            const auto id = netlist.node_ids.at(group);
            Require(
                concrete.Value(id) == expected,
                "mixed-width one-hot concrete values retain sparse padding");
            const auto got = symbolic.Value(id);
            const auto want =
                pattern::SymbolicValue::Literal(context, expected);
            z3::solver solver(context);
            solver.add(got.bits != want.bits || got.known != want.known);
            Require(solver.check() == z3::unsat,
                    "mixed-width one-hot symbolic values retain enables, masks "
                    "and order");
          }
        }
      }
    }
  }
  pattern::Design empty_design;
  empty_design.AddIcl(
      "empty_onehot.icl",
      "Module Empty {} Module M { "
      "OneHotDataGroup group[9:0] { Instance child Of Empty; } }");
  const auto empty = pattern::Elaborate(empty_design, "M");
  for (bool symbolic : {false, true}) {
    bool rejected = false;
    try {
      if (symbolic) {
        pattern::SymbolicNetwork values(empty, context, {});
        values.Value(empty.node_ids.at("group"));
      } else {
        pattern::NetworkState values(empty);
        values.Value(empty.node_ids.at("group"));
      }
    } catch (const std::runtime_error& error) {
      rejected = std::string(error.what()).find("no selection condition") !=
                 std::string::npos;
    }
    Require(rejected, "empty one-hot contributors retain selection validation");
  }
}

void TestSparseOneHotScale() {
  for (const auto [width, count] :
       {std::pair<size_t, size_t>{512, 8}, {8192, 64}, {65536, 512}}) {
    std::string text =
        "Module Producer { DataInPort value; DataInPort enabled; "
        "DataOutPort out { Source value; Enable enabled; } } Module M { "
        "DataInPort data[" +
        std::to_string(count - 1) + ":0]; DataInPort enable[" +
        std::to_string(count - 1) + ":0]; ";
    for (size_t i = 0; i < count; ++i) {
      text += "Instance p" + std::to_string(i) +
              " Of Producer { InputPort value = data[" + std::to_string(i) +
              "]; InputPort enabled = enable[" + std::to_string(i) + "]; } ";
    }
    text += "OneHotDataGroup group[" + std::to_string(width - 1) + ":0] { ";
    for (size_t i = 0; i < count; ++i) {
      text += "Port p" + std::to_string(i) + ".out; ";
    }
    text += "} }";
    pattern::Design design;
    design.AddIcl("sparse_onehot.icl", text);
    const auto netlist = pattern::Elaborate(design, "M");
    const auto data = netlist.node_ids.at("data");
    const auto enable = netlist.node_ids.at("enable");
    const auto group = netlist.node_ids.at("group");
    std::string selected(count, '0');
    selected.back() = '1';
    pattern::NetworkState concrete(netlist);
    concrete.Drive(data, std::string(count, '1'));
    concrete.Drive(enable, selected);
    const auto concrete_started = std::chrono::steady_clock::now();
    Require(concrete.Value(group) == std::string(width - 1, 'x') + "1",
            "sparse one-hot concrete selection retains unknown padding");
    concrete.Drive(enable, std::string(count, '1'));
    Require(concrete.Value(group) == std::string(width, 'x'),
            "multiple enabled one-hot contributors remain unknown");
    std::cout << "sparse onehot concrete: width=" << width << " count=" << count
              << " bytes=" << text.size() << " seconds="
              << std::chrono::duration<double>(
                     std::chrono::steady_clock::now() - concrete_started)
                     .count()
              << '\n';
    z3::context context;
    const auto data_bits = context.bv_const("onehot_data", count);
    pattern::SymbolicNetwork symbolic(
        netlist, context,
        {{data, {data_bits, ~context.bv_val(0, count)}},
         {enable, pattern::SymbolicValue::Literal(context, selected)}});
    const auto symbolic_started = std::chrono::steady_clock::now();
    const auto got = symbolic.Value(group);
    const auto want =
        z3::concat(context.bv_val(0, width - 1), data_bits.extract(0, 0));
    z3::solver solver(context);
    solver.set("timeout", 3000u);
    solver.add(got.bits != want || got.known != context.bv_val(1, width));
    const auto result = solver.check();
    std::cout << "sparse onehot symbolic: width=" << width << " count=" << count
              << " seconds="
              << std::chrono::duration<double>(
                     std::chrono::steady_clock::now() - symbolic_started)
                     .count()
              << " result=" << result << '\n';
    Require(result == z3::unsat,
            "sparse one-hot symbolic selection retains data and masks within "
            "proof budget");
  }
}

void TestLiteralRunAgreement() {
  pattern::Design design;
  design.AddIcl("literal_runs.icl", "Module M { DataInPort a[2:0]; }");
  const auto netlist = pattern::Elaborate(design, "M");
  const auto input = netlist.node_ids.at("a");
  const size_t missing = std::numeric_limits<size_t>::max();
  z3::context context;
  for (char synchronous : std::string("01x")) {
    for (char asynchronous : std::string("01x")) {
      pattern::SymbolicNetwork values(
          netlist, context,
          {{input, pattern::SymbolicValue::Literal(context, "10x")}},
          std::string{synchronous, asynchronous});
      const std::vector<pattern::BitReference> references{
          {input, 2},
          {missing, 0, '0'},
          {missing, 0, '0', true},
          {missing, 0, 'x', true},
          {missing, 0, '?', false, pattern::VirtualSignal::kGlobalReset},
          {missing, 0, '1', true, pattern::VirtualSignal::kAsyncGlobalReset},
          {input, 1},
          {missing, 0, '?', true},
          {input, 0}};
      const char inverted_async = asynchronous == 'x'   ? 'x'
                                  : asynchronous == '0' ? '1'
                                                        : '0';
      const std::string expected =
          std::string("101x") + synchronous + inverted_async + "0xx";
      for (bool scan : {false, true}) {
        const auto got = values.Bits(references, scan);
        const auto want = pattern::SymbolicValue::Literal(context, expected);
        z3::solver solver(context);
        solver.add(got.bits != want.bits || got.known != want.known);
        Require(solver.check() == z3::unsat,
                "literal runs retain physical boundaries and all reset states");
        for (size_t i = 0; i < references.size(); ++i) {
          const auto bit = values.Read(references[i], scan);
          const auto wanted = pattern::SymbolicValue::Literal(
              context, std::string(1, expected[i]));
          z3::solver scalar(context);
          scalar.add(bit.bits != wanted.bits || bit.known != wanted.known);
          Require(
              scalar.check() == z3::unsat,
              "single-bit literal and virtual reset reads retain semantics");
        }
      }
      for (bool inverted : {false, true}) {
        const pattern::BitReference invalid{missing, 0, 'q', inverted};
        for (bool run : {false, true}) {
          bool rejected = false;
          try {
            if (run) {
              values.Bits({{missing, 0, '0'}, invalid, {missing, 0, '1'}});
            } else {
              values.Read(invalid);
            }
          } catch (const std::runtime_error& error) {
            rejected = std::string(error.what()).find("0/1/x bits") !=
                       std::string::npos;
          }
          Require(rejected, "literal grouping retains invalid-value rejection");
        }
      }
    }
  }
}

void TestLiteralSignalRuns(bool large = false) {
  const size_t missing = std::numeric_limits<size_t>::max();
  const std::vector<pattern::BitReference> mixed{
      {missing, 0, '0'},
      {missing, 0, '1', true},
      {missing, 0, 'x'},
      {missing, 0, '?', true},
      {missing, 0, '?', false, pattern::VirtualSignal::kGlobalReset},
      {missing, 0, '?', true, pattern::VirtualSignal::kAsyncGlobalReset},
      {missing, 0, '0', true},
      {missing, 0, '1'}};
  std::vector<size_t> widths{512, 8192, 65536, 262144};
  if (large) widths.push_back(1048576);
  for (size_t width : widths) {
    pattern::Design design;
    design.AddIcl("literal_signal.icl",
                  "Module M { DataOutPort out[" + std::to_string(width - 1) +
                      ":0] { Source " + std::to_string(width) + "'b0; } }");
    auto netlist = pattern::Elaborate(design, "M");
    const auto output = netlist.node_ids.at("out");
    for (bool virtual_resets : {false, true}) {
      auto& source = netlist.nodes.at(output).inputs.at("Source");
      std::string expected;
      expected.reserve(width);
      for (size_t i = 0; i < width; ++i) {
        source[i] = virtual_resets
                        ? mixed[i % mixed.size()]
                        : pattern::BitReference{missing, 0, "01x"[i % 3]};
        expected +=
            virtual_resets ? "00xx1111"[i % mixed.size()] : "01x"[i % 3];
      }
      z3::context context;
      pattern::SymbolicNetwork values(netlist, context, {}, "10");
      const auto started = std::chrono::steady_clock::now();
      const auto got = values.Value(output);
      const auto want = pattern::SymbolicValue::Literal(context, expected);
      z3::solver solver(context);
      solver.set("timeout", 3000u);
      solver.add(got.bits != want.bits || got.known != want.known);
      const auto result = solver.check();
      const auto seconds = std::chrono::duration<double>(
                               std::chrono::steady_clock::now() - started)
                               .count();
      std::cout << "literal signal: bits=" << width
                << " virtual_resets=" << virtual_resets
                << " seconds=" << seconds << " result=" << result << '\n';
      Require(result == z3::unsat,
              "literal and virtual reset runs preserve values within the proof "
              "budget");
    }
  }
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
    const std::vector<pattern::BitReference> mixed{
        {a, 1}, {a, 1}, {a, 0}, {a, 0, 'x', true}, {a, 0, 'x', true},
        {b, 1}, {b, 0}, {a, 1}};
    auto individual = symbolic.Read(mixed.front(), scan);
    for (size_t i = 1; i < mixed.size(); ++i) {
      const auto next = symbolic.Read(mixed[i], scan);
      individual = {z3::concat(individual.bits, next.bits),
                    z3::concat(individual.known, next.known)};
    }
    const auto grouped = symbolic.Bits(mixed, scan);
    z3::solver grouped_solver(context);
    grouped_solver.add(grouped.bits != individual.bits ||
                       grouped.known != individual.known);
    Require(grouped_solver.check() == z3::unsat,
            "repeat/slice/inversion boundaries preserve individual held/shift "
            "reads");
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
  const auto rejects = [](const auto& action, const char* message) {
    bool rejected = false;
    try {
      action();
    } catch (const std::runtime_error& error) {
      rejected = std::string(error.what()).find(message) != std::string::npos;
    }
    Require(rejected, message);
  };
  constexpr size_t kMaxBits = 1048576;
  for (size_t width : {size_t{0}, kMaxBits + 1}) {
    rejects([&] { pattern::PdlNumber(context, "0", width); },
            "invalid or oversized PDL numeric value");
  }
  rejects(
      [&] {
        pattern::PdlNumber(context, "0b" + std::string(kMaxBits + 1, '0'), 1);
      },
      "invalid or oversized PDL numeric value");
  rejects([&] { pattern::PdlNumber(context, "999", 1); },
          "PDL value truncates significant bits");
  Require(pattern::PdlNumber(context, "0b1", kMaxBits) ==
                  std::string(kMaxBits - 1, '0') + "1" &&
              pattern::PdlNumber(context, "0b" + std::string(kMaxBits, '0'),
                                 1) == "0",
          "maximum PDL width and numeric text retain supported boundaries");
  for (const auto* text : {"r[1", "r[1)"}) {
    rejects([&] { pattern::ResolvePdlTarget(netlist, text); },
            "unclosed PDL target range");
  }
  for (const auto* text :
       {"r[]", "r[-1]", "r[a]", "r[18446744073709551616]", "r[1:2:3]"}) {
    rejects([&] { pattern::ResolvePdlTarget(netlist, text); },
            "invalid PDL target bit index");
  }
  rejects([&] { pattern::ResolvePdlTarget(netlist, "si[0]"); },
          "PDL scalar target cannot be indexed");
  rejects([&] { pattern::ResolvePdlTarget(netlist, "r[8]"); },
          "PDL target index outside r");
  rejects([&] { pattern::ResolvePdlTarget(netlist, "missing"); },
          "unknown PDL target missing");
  Require(target.declaration == netlist.node_ids.at("a") &&
              reverse.declaration == netlist.node_ids.at("r"),
          "target declaration identity survives alias resolution");
  // Public IR callers can provide malformed objects directly.
  auto oversized = netlist;
  oversized.nodes.at(oversized.node_ids.at("r")).right = kMaxBits;
  rejects([&] { pattern::ResolvePdlTarget(oversized, "r"); },
          "PDL target width limit exceeded");
  auto missing_source = netlist;
  missing_source.nodes.at(missing_source.node_ids.at("a"))
      .inputs.erase("Source");
  rejects([&] { pattern::ResolvePdlTarget(missing_source, "a"); },
          "alias target has no source: a");
  auto cyclic = netlist;
  const auto alias = cyclic.node_ids.at("a");
  cyclic.nodes.at(alias).inputs.at("Source") = {
      {alias, 3}, {alias, 2}, {alias, 1}, {alias, 0}};
  rejects([&] { pattern::ResolvePdlTarget(cyclic, "a"); },
          "cyclic PDL alias target");
  auto empty = target;
  empty.bits.clear();
  rejects([&] { pattern::PdlDataValue(context, netlist, empty, "Active"); },
          "invalid PDL value width");
}
}  // namespace
int main(int argc, char* argv[]) {
  try {
    if (argc != 1) {
      Require(argc == 2 && (std::string(argv[1]) == "--mux-scale" ||
                            std::string(argv[1]) == "--repeat-scale" ||
                            std::string(argv[1]) == "--literal-scale" ||
                            std::string(argv[1]) == "--onehot-scale"),
              "expected --mux-scale, --repeat-scale, --literal-scale, "
              "--onehot-scale or no "
              "test arguments");
      if (std::string(argv[1]) == "--onehot-scale") {
        TestSparseOneHotScale();
      } else if (std::string(argv[1]) == "--literal-scale") {
        TestLiteralSignalRuns(true);
      } else if (std::string(argv[1]) == "--repeat-scale") {
        TestRepeatedSignalRuns();
      } else {
        TestMuxChainScale();
      }
      return 0;
    }
    TestLiteralWordBoundaries();
    TestSparseOneHotAgreement();
    TestSparseOneHotScale();
    TestLiteralRunAgreement();
    TestLiteralSignalRuns();
    TestImpliedToTckValues();
    TestRepeatedSignalRuns();
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
