// Copyright 2026 The ICL Toolsets Authors.
// SPDX-License-Identifier: MIT
#include "pattern/pattern_netlist.h"

#include <chrono>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include "pattern/pattern_flatten.h"
#include "utility/text_input.h"

namespace {
void Require(bool condition, const std::string& message) {
  if (!condition) throw std::runtime_error(message);
}

template <typename Action>
void Fails(Action action, const std::string& expected) {
  try {
    action();
  } catch (const std::runtime_error& error) {
    Require(std::string(error.what()).find(expected) != std::string::npos,
            "expected " + expected + ", got " + error.what());
    return;
  }
  throw std::runtime_error("expected failure: " + expected);
}

void TestClockSignalSources() {
  for (const std::string binding : {"~root", "1'b0", "1'b1", "1'bx"}) {
    pattern::Design design;
    design.AddIcl(
        "bound_negative.icl",
        "Module Child { ClockPort p; ClockPort n { DifferentialInvOf p; } } "
        "Module M { ClockPort root; Instance child Of Child { "
        "InputPort p = root; InputPort n = " +
            binding + "; } }");
    const auto original = pattern::Elaborate(design, "M");
    pattern::Design flat_design;
    flat_design.AddIcl("flat.icl", pattern::SerializeIcl(original));
    const auto flat = pattern::Elaborate(flat_design, "M");
    const auto& inputs = flat.NodeAt("child.n").inputs;
    Require(inputs.count("Source") &&
                inputs.at("Source").at(0).inverted == (binding == "~root") &&
                pattern::SerializeIcl(flat) == pattern::SerializeIcl(original),
            "bound differential input survives flat replay");
  }
  {
    pattern::Design design;
    design.AddIcl("partial_negative.icl",
                  "Module Child { ClockPort p[1:0]; ClockPort n[1:0] { "
                  "DifferentialInvOf p; } } "
                  "Module M { ClockPort root[1:0]; Instance child Of Child { "
                  "InputPort p = root; InputPort n[1] = root[1]; } }");
    const auto original = pattern::Elaborate(design, "M");
    pattern::Design flat_design;
    flat_design.AddIcl("flat.icl", pattern::SerializeIcl(original));
    const auto flat = pattern::Elaborate(flat_design, "M");
    Require(flat.NodeAt("child.n").inputs.at("Source").at(1).constant == '?' &&
                pattern::SerializeIcl(flat) == pattern::SerializeIcl(original),
            "partial differential binding preserves missing-bit identity");
  }
  const std::vector<std::string> consumers = {
      "ToClockPort out { Source SOURCE; }",
      "ClockMux out SelectedBy 1'b0 { 1'b0: SOURCE; }",
      "ClockPort out { DifferentialInvOf SOURCE; }",
      "ToClockPort out { DifferentialInvOf SOURCE; }",
      "Instance sink Of Sink { InputPort ck = SOURCE; }"};
  for (const std::string declaration :
       {"ScanInPort source;", "ScanOutPort source { Source 1'b0; }",
        "ResetPort source;", "CaptureEnPort source;", "ShiftEnPort source;",
        "UpdateEnPort source;", "TMSPort source;", "TRSTPort source;",
        "ToResetPort source;", "ToCaptureEnPort source;",
        "ToShiftEnPort source;", "ToUpdateEnPort source;",
        "TMSPort owner_tms; ToIRSelectPort source;", "ToTRSTPort source;",
        "ClockPort positive; ClockPort source { DifferentialInvOf positive; }",
        "ClockPort positive; ToClockPort source { DifferentialInvOf positive; "
        "}"}) {
    for (const auto& consumer : consumers) {
      for (const std::string signal : {"source", "wrapped"}) {
        auto body = consumer;
        body.replace(body.find("SOURCE"), 6, signal);
        pattern::Design design;
        design.AddIcl("clock_sources.icl",
                      "Module Sink { ClockPort ck; } Module M {" + declaration +
                          " Alias wrapped = source; " + body + "}");
        Fails([&] { pattern::Elaborate(design, "M"); }, "clock or data signal");
      }
    }
  }
  for (const std::string declaration :
       {"ClockPort source;", "ToClockPort source;", "TCKPort source;",
        "ToTCKPort source;", "DataInPort source;",
        "DataOutPort source { Source 1'b0; }", "SelectPort source;",
        "ToSelectPort source { Source 1'b0; }", "DataRegister source;",
        "ScanInPort si; ScanRegister source { ScanInSource si; }",
        "LogicSignal source { 1'b0; }",
        "DataMux source SelectedBy 1'b0 { 1'b0: 1'b0; }",
        "ClockPort ck; ClockMux source SelectedBy 1'b0 { 1'b0: ck; }"}) {
    for (const auto& consumer : consumers) {
      auto body = consumer;
      body.replace(body.find("SOURCE"), 6, "wrapped");
      pattern::Design design;
      const bool output = declaration.compare(0, 11, "ToClockPort") == 0 ||
                          declaration.compare(0, 9, "ToTCKPort") == 0 ||
                          declaration.compare(0, 11, "DataOutPort") == 0 ||
                          declaration.compare(0, 12, "ToSelectPort") == 0;
      const auto provider =
          output ? "Module Provider { " +
                       std::string(declaration == "ToTCKPort source;"
                                       ? "TCKPort incoming; "
                                       : "") +
                       declaration + " } "
                 : "";
      design.AddIcl(
          "valid_clock_sources.icl",
          provider + "Module Sink { ClockPort ck; } Module M {" +
              (output ? "Instance provider Of Provider;" : declaration) +
              " Alias wrapped = " + (output ? "provider.source" : "source") +
              "; " + body + "}");
      if (consumer == "ClockPort out { DifferentialInvOf SOURCE; }" &&
          declaration != "ClockPort source;") {
        Fails([&] { pattern::Elaborate(design, "M"); },
              "requires a positive ClockPort");
        continue;
      }
      const auto model = pattern::Elaborate(design, "M");
      pattern::Design flat;
      flat.AddIcl("flat.icl", pattern::SerializeIcl(model));
      pattern::Elaborate(flat, "M");
    }
  }
}

void TestClockInputDifferentialAssociation() {
  for (const std::string source :
       {"positive[1:0]", "~positive[0:1]", "wrapped", "~wrapped"}) {
    pattern::Design design;
    design.AddIcl(
        "input_clock_pair.icl",
        "Module M { ClockPort positive[1:0]; ClockPort other[0:1]; "
        "Alias wrapped[1:0] = ~positive[0:1]; ClockPort negative[1:0] { "
        "DifferentialInvOf " +
            source +
            "; } ClockPort second[0:1] { DifferentialInvOf other; } }");
    const auto text = pattern::SerializeIcl(pattern::Elaborate(design, "M"));
    pattern::Design flat;
    flat.AddIcl("flat.icl", text);
    Require(pattern::SerializeIcl(pattern::Elaborate(flat, "M")) == text,
            "local clock pairs preserve aliases, vector order and inversion "
            "syntax");
  }
  for (const std::string declaration :
       {"DataInPort bad[1:0];", "TCKPort bad[1:0];", "DataRegister bad[1:0];",
        "ToClockPort bad[1:0] { Period 10ns; }",
        "ClockPort ck[1:0]; ClockMux bad[1:0] SelectedBy 1'b0 { 1'b0: ck; }"}) {
    for (const std::string source : {"bad", "wrapped", "~wrapped"}) {
      pattern::Design design;
      design.AddIcl("invalid_input_clock_pair.icl",
                    "Module M { " + declaration +
                        " Alias wrapped[1:0] = bad; "
                        "ClockPort negative[1:0] { DifferentialInvOf " +
                        source + "; } }");
      Fails([&] { pattern::Elaborate(design, "M"); },
            "requires a positive ClockPort");
    }
  }
  for (const std::string source :
       {"2'b00", "positive[1], bad[0]", "bad[1], positive[0]", "mixed"}) {
    pattern::Design design;
    design.AddIcl("mixed_input_clock_pair.icl",
                  "Module M { ClockPort positive[1:0]; DataInPort bad[1:0]; "
                  "Alias mixed[1:0] = positive[1], bad[0]; "
                  "ClockPort negative[1:0] { DifferentialInvOf " +
                      source + "; } }");
    Fails([&] { pattern::Elaborate(design, "M"); },
          "requires a positive ClockPort");
  }
  pattern::Design hierarchy;
  hierarchy.AddIcl(
      "input_clock_pair_scope.icl",
      "Module Child { ClockPort p; ClockPort n { DifferentialInvOf p; } } "
      "Module M { ClockPort root; Instance child Of Child { InputPort p = "
      "root; } "
      "Instance peer Of Child { InputPort p = root; } }");
  const auto original = pattern::Elaborate(hierarchy, "M");
  for (const std::string source : {"root", "peer.p"}) {
    auto altered = original;
    altered.nodes.at(altered.node_ids.at("child.n"))
        .inputs.at("DifferentialInvOf")[0] = {altered.node_ids.at(source), 0};
    pattern::Design flat;
    flat.AddIcl("altered_input_pair.icl", pattern::SerializeIcl(altered));
    Fails([&] { pattern::Elaborate(flat, "M"); }, "reference scope");
  }
}

void TestTckInputSources() {
  for (const auto& entry : std::vector<std::pair<std::string, bool>>{
           {"TCKPort source;", true},
           {"ToTCKPort source;", true},
           {"ClockPort ck; ToTCKPort source { Source ck; }", true},
           {"DataInPort source;", true},
           {"DataRegister source;", true},
           {"LogicSignal source { 1'b0; }", true},
           {"ClockPort source;", false},
           {"ToClockPort source;", false},
           {"ResetPort source;", false},
           {"TRSTPort source;", false},
           {"CaptureEnPort source;", false},
           {"ShiftEnPort source;", false},
           {"UpdateEnPort source;", false},
           {"TMSPort source;", false},
           {"ScanInPort source;", false}}) {
    const bool output =
        entry.second && entry.first.find("ToTCKPort") != std::string::npos;
    const bool clock_extension =
        entry.first.find("ClockPort ck;") != std::string::npos;
    const std::string provider =
        output
            ? "Module Provider { " +
                  (clock_extension ? "" : std::string("TCKPort incoming; ")) +
                  entry.first + " } "
            : "";
    const std::string declaration =
        !output           ? entry.first
        : clock_extension ? "ClockPort ck; Instance provider Of Provider { "
                            "InputPort ck = ck; } "
                          : "TCKPort incoming; Instance provider Of Provider { "
                            "InputPort incoming = incoming; } ";
    pattern::Design design;
    design.AddIcl(
        "tck_sources.icl",
        provider + "Module Child { TCKPort ck; } Module M {" + declaration +
            " Alias wrapped = " + (output ? "provider.source" : "source") +
            "; Instance child Of Child { "
            "InputPort ck = wrapped; } }");
    if (entry.second) {
      const auto original = pattern::Elaborate(design, "M");
      pattern::Design flat;
      flat.AddIcl("flat.icl", pattern::SerializeIcl(original));
      pattern::Elaborate(flat, "M");
    } else {
      Fails([&] { pattern::Elaborate(design, "M"); }, "TCK or data signal");
    }
  }
  for (const std::string source : {"1'b0", "1'b1", "1'bx"}) {
    pattern::Design design;
    design.AddIcl("constant_tck.icl",
                  "Module Child { TCKPort ck; } Module M { "
                  "Instance child Of Child { InputPort ck = " +
                      source + "; } }");
    pattern::Elaborate(design, "M");
  }
}
void TestResetSourceKinds() {
  for (const std::string declaration :
       {"ResetPort source;", "ToResetPort source;", "TRSTPort source;",
        "ToTRSTPort source;", "DataInPort source;", "DataRegister source;",
        "LogicSignal source { 1'b0; }"}) {
    const auto polarity =
        declaration.find("TRST") == std::string::npos ? "1" : "0";
    const bool ordinary =
        declaration.find("Data") == 0 || declaration.find("LogicSignal") == 0;
    const std::string driver =
        ordinary ? "ResetPort global; DataMux route SelectedBy "
                   "1'b0 { 1'b0 : global; 1'b1 : source; } "
                 : "";
    const bool output = declaration.rfind("To", 0) == 0;
    const std::string provider =
        output ? "Module Provider { " + declaration + " } " : "";
    pattern::Design design;
    design.AddIcl(
        "reset_kinds.icl",
        provider + "Module Child { ResetPort r { ActivePolarity " +
            std::string(polarity) + "; } } Module M { " +
            (output ? "Instance provider Of Provider; " : declaration) + " " +
            driver + " Alias wrapped = " +
            (ordinary ? "route"
             : output ? "provider.source"
                      : "source") +
            "; "
            "ToResetPort out { Source wrapped; ActivePolarity " +
            polarity +
            "; } Instance child Of Child { InputPort r = wrapped; } }");
    const auto original = pattern::Elaborate(design, "M");
    pattern::Design flat;
    flat.AddIcl("flat.icl", pattern::SerializeIcl(original));
    pattern::Elaborate(flat, "M");
  }
  pattern::Design design;
  design.AddIcl("inferred_reset.icl",
                "Module Child { ResetPort r; } "
                "Module M { ToResetPort out; Instance child Of Child; }");
  const auto original = pattern::Elaborate(design, "M");
  Require(original.NodeAt("child.r").inputs.at("Source").at(0).virtual_signal ==
              pattern::VirtualSignal::kGlobalReset,
          "inferred global reset remains an eligible Source");
  pattern::Design flat;
  flat.AddIcl("flat.icl", pattern::SerializeIcl(original));
  pattern::Elaborate(flat, "M");
}
void TestResetPortPolarity() {
  for (const std::string declaration :
       {"ResetPort source { ActivePolarity 0; }", "TRSTPort source;",
        "ToResetPort source { ActivePolarity 0; }", "ToTRSTPort source;"}) {
    for (bool inverted : {false, true}) {
      const bool output = declaration.rfind("To", 0) == 0;
      const std::string provider =
          output ? "Module Provider { " + declaration + " } " : "";
      pattern::Design design;
      design.AddIcl(
          "reset_polarity.icl",
          provider +
              "Module Leaf { ResetPort rst; ScanInPort si; ScanRegister r { "
              "ScanInSource si; ResetValue 1'b1; } } "
              "Module Child { ResetPort rst; ScanInPort si; "
              "Instance leaf Of Leaf { InputPort rst = rst; InputPort si = si; "
              "} } "
              "Module M { " +
              (output ? "Instance provider Of Provider; " : declaration) +
              " ScanInPort si; Alias first = ~" +
              (output ? "provider.source" : "source") +
              "; Alias second = ~first; "
              "Instance child Of Child { InputPort rst = " +
              (inverted ? "~second" : "second") + "; InputPort si = si; } }");
      const auto original = pattern::Elaborate(design, "M");
      Require(original.NodeAt("child.rst").active_low == !inverted &&
                  original.NodeAt("child.leaf.rst").active_low == !inverted &&
                  original.NodeAt("child.leaf.r").reset->inverted == !inverted,
              "explicit typed reset source dictates omitted polarity through "
              "hierarchy");
      pattern::Design replay;
      const auto serialized = pattern::SerializeIcl(original);
      replay.AddIcl("flat.icl", serialized);
      Require(
          pattern::SerializeIcl(pattern::Elaborate(replay, "M")) == serialized,
          "inherited reset polarity survives flat replay");
    }
  }
  for (const std::string source :
       {"lo, lo", "lo, ~hi", "lo, 1'b0", "lo, route"}) {
    pattern::Design design;
    design.AddIcl(
        "vector_polarity.icl",
        "Module Child { ResetPort rst[1:0]; } Module M { "
        "ResetPort lo { ActivePolarity 0; } ResetPort hi; DataInPort mode; "
        "DataMux route SelectedBy mode { 1'b0: ~hi; 1'b1: 1'b0; } "
        "Instance child Of Child { InputPort rst = " +
            source + "; } }");
    const auto original = pattern::Elaborate(design, "M");
    Require(original.NodeAt("child.rst").active_low,
            "uniform typed bits dictate grouped reset polarity");
    pattern::Design replay;
    const auto serialized = pattern::SerializeIcl(original);
    replay.AddIcl("flat.icl", serialized);
    Require(
        pattern::SerializeIcl(pattern::Elaborate(replay, "M")) == serialized,
        "mixed reset vector survives flat replay");
  }
  {
    pattern::Design design;
    design.AddIcl("partial_polarity.icl",
                  "Module Child { ResetPort rst[1:0]; } Module M { "
                  "ResetPort lo { ActivePolarity 0; } "
                  "Instance child Of Child { InputPort rst[1] = lo; } }");
    const auto netlist = pattern::Elaborate(design, "M");
    Require(
        netlist.NodeAt("child.rst").active_low &&
            netlist.NodeAt("child.rst").inputs.at("Source")[1].constant == '?',
        "unbound vector bit does not dictate a separate polarity");
  }
  {
    pattern::Design design;
    design.AddIcl("untyped_polarity.icl",
                  "Module Child { ResetPort rst; } Module M { "
                  "Instance child Of Child { InputPort rst = 1'b0; } }");
    Require(!pattern::Elaborate(design, "M").NodeAt("child.rst").active_low,
            "literal alone leaves reset default high");
  }
  {
    pattern::Design design;
    design.AddIcl("flat_polarity.icl",
                  "Module Child { ResetPort rst { ActivePolarity 0; } } "
                  "Module M { ResetPort lo { ActivePolarity 0; } "
                  "ToResetPort out { Source lo; ActivePolarity 0; } "
                  "Instance child Of Child { InputPort rst = lo; } }");
    auto original = pattern::Elaborate(design, "M");
    original.nodes.at(original.node_ids.at("child.rst")).active_low = false;
    pattern::Design altered;
    altered.AddIcl("altered.icl", pattern::SerializeIcl(original));
    Fails([&] { pattern::Elaborate(altered, "M"); }, "reset source polarity");
  }
  for (const std::string consumer :
       {"Instance child Of Child { InputPort rst = lo; }",
        "ToResetPort out { Source lo; }",
        "ToResetPort out { Source ~hi; ActivePolarity 1; }"}) {
    pattern::Design design;
    design.AddIcl(
        "mismatch.icl",
        "Module Child { ResetPort rst { ActivePolarity 1; } } "
        "Module M { ResetPort lo { ActivePolarity 0; } ResetPort hi; " +
            consumer + " }");
    Fails([&] { pattern::Elaborate(design, "M"); }, "reset source polarity");
  }
  {
    pattern::Design design;
    design.AddIcl("mixed_polarity.icl",
                  "Module Child { ResetPort rst[1:0]; } Module M { "
                  "ResetPort lo { ActivePolarity 0; } ResetPort hi; "
                  "Instance child Of Child { InputPort rst = lo, hi; } }");
    Fails([&] { pattern::Elaborate(design, "M"); }, "reset source polarity");
  }
  {
    pattern::Design design;
    design.AddIcl("implicit_polarity.icl",
                  "Module Child { ResetPort rst; ToResetPort out; } "
                  "Module M { ResetPort lo { ActivePolarity 0; } "
                  "Instance child Of Child; ToResetPort out; }");
    const auto netlist = pattern::Elaborate(design, "M");
    Require(!netlist.NodeAt("child.rst").active_low &&
                netlist.NodeAt("child.rst").inputs.at("Source")[0].inverted &&
                !netlist.NodeAt("out").active_low &&
                netlist.NodeAt("out").inputs.at("Source")[0].inverted,
            "implied reset connections preserve automatic polarity conversion");
  }
  {
    pattern::Design design;
    design.AddIcl(
        "bit_alias.icl",
        "Module Child { ResetPort rst; } Module M { "
        "ResetPort lo { ActivePolarity 0; } Alias bits[1:0] = bits[0], lo; "
        "Instance child Of Child { InputPort rst = bits[1]; } }");
    Require(pattern::Elaborate(design, "M").NodeAt("child.rst").active_low,
            "cross-bit alias path is not a cycle");
  }
  {
    pattern::Design design;
    design.AddIcl("cycle.icl",
                  "Module Child { ResetPort rst; } Module M { "
                  "Alias bits[1:0] = bits[0], bits[1]; "
                  "Instance child Of Child { InputPort rst = bits[1]; } }");
    Fails([&] { pattern::Elaborate(design, "M"); }, "combinational cycle");
  }
}
void TestFlatTrstPolarity() {
  pattern::Design design;
  design.AddIcl("trst.icl",
                "Module Child { TRSTPort rst; } Module M { TRSTPort root; "
                "Instance child Of Child { InputPort rst = root; } }");
  auto original = pattern::Elaborate(design, "M");
  original.nodes.at(original.node_ids.at("child.rst")).active_low = false;
  pattern::Design altered;
  altered.AddIcl("altered.icl", pattern::SerializeIcl(original));
  Fails([&] { pattern::Elaborate(altered, "M"); }, "TRST polarity");
}
void TestResetMuxPathPolarity() {
  for (bool low : {false, true}) {
    for (bool branch_inverted : {false, true}) {
      for (bool output_inverted : {false, true}) {
        for (bool consumer_low : {false, true}) {
          for (bool output : {false, true}) {
            const std::string polarity = consumer_low ? "0" : "1";
            const std::string wire = output_inverted ? "~wrapped" : "wrapped";
            const std::string consumer =
                output
                    ? "ToResetPort out { Source " + wire + "; ActivePolarity " +
                          polarity + "; }"
                    : "Instance sink Of Sink { InputPort rst = " + wire + "; }";
            pattern::Design design;
            design.AddIcl(
                "mux_polarity.icl",
                "Module Sink { ResetPort rst { ActivePolarity " + polarity +
                    "; } } Module M { ResetPort rst { ActivePolarity " +
                    (low ? "0" : "1") +
                    "; } DataInPort mode; DataInPort force; Alias branch = " +
                    (branch_inverted ? "~rst" : "rst") +
                    "; DataMux route SelectedBy mode { 1'b0: branch; 1'b1: "
                    "force; } "
                    "Alias wrapped = route; " +
                    consumer + " }");
            if (((low != branch_inverted) != output_inverted) != consumer_low) {
              Fails([&] { pattern::Elaborate(design, "M"); },
                    "reset mux path polarity");
            } else {
              const auto original = pattern::Elaborate(design, "M");
              pattern::Design flat;
              const auto serialized = pattern::SerializeIcl(original);
              flat.AddIcl("flat.icl", serialized);
              Require(pattern::SerializeIcl(pattern::Elaborate(flat, "M")) ==
                          serialized,
                      "matching reset mux path preserves source/flat polarity");
            }
          }
        }
      }
    }
  }
  {
    pattern::Design design;
    design.AddIcl(
        "nested_polarity.icl",
        "Module Sink { ResetPort rst; } Module M { ResetPort rst; "
        "DataInPort mode[2:0]; DataInPort force; "
        "DataMux forcing SelectedBy mode[0] { 1'b0: force; 1'b1: 1'b1; } "
        "DataMux inner SelectedBy mode[1] { 1'b0: ~rst; 1'b1: forcing; } "
        "DataMux outer SelectedBy mode[2] { 1'b0: ~inner; 1'b1: 1'b0; } "
        "Instance sink Of Sink { InputPort rst = outer; } }");
    const auto original = pattern::Elaborate(design, "M");
    pattern::Design flat;
    const auto serialized = pattern::SerializeIcl(original);
    flat.AddIcl("flat.icl", serialized);
    Require(pattern::SerializeIcl(pattern::Elaborate(flat, "M")) == serialized,
            "nested inversions cancel without tracing forcing-only branches");
  }
  for (const std::string declaration : {"TRSTPort rst;", "ToTRSTPort rst;"}) {
    const bool output = declaration.rfind("To", 0) == 0;
    const std::string provider =
        output ? "Module Provider { " + declaration + " } " : "";
    pattern::Design design;
    design.AddIcl(
        "trst_reset_mux.icl",
        provider +
            "Module Sink { ResetPort rst { ActivePolarity 0; } } Module M { " +
            (output ? "Instance provider Of Provider; " : declaration) +
            " DataInPort mode; DataMux route SelectedBy mode { "
            "1'b0: " +
            (output ? "provider.rst" : "rst") +
            "; 1'b1: 1'b1; } ToResetPort out { Source route; "
            "ActivePolarity 0; } Instance sink Of Sink { InputPort rst = "
            "route; } }");
    const auto original = pattern::Elaborate(design, "M");
    pattern::Design flat;
    const auto serialized = pattern::SerializeIcl(original);
    flat.AddIcl("flat.icl", serialized);
    Require(pattern::SerializeIcl(pattern::Elaborate(flat, "M")) == serialized,
            "TRST path can supply an active-low reset mux consumer");
  }
  {
    pattern::Design design;
    design.AddIcl("typed_forcing.icl",
                  "Module Sink { ResetPort rst; } "
                  "Module Low { ResetPort rst { ActivePolarity 0; } } "
                  "Module M { ResetPort hi; ResetPort lo { ActivePolarity 0; } "
                  "DataInPort mode; DataMux route SelectedBy mode { 1'b0: hi; "
                  "1'b1: lo; } "
                  "Instance high Of Sink { InputPort rst = route; } "
                  "Instance low Of Low { InputPort rst = route; } }");
    const auto original = pattern::Elaborate(design, "M");
    pattern::Design flat;
    const auto serialized = pattern::SerializeIcl(original);
    flat.AddIcl("flat.icl", serialized);
    Require(
        pattern::SerializeIcl(pattern::Elaborate(flat, "M")) == serialized,
        "typed forcing and opposite-polarity consumers survive flat replay");
  }
  for (const std::string consumers :
       {"Instance first Of Sink { InputPort rst = route; } "
        "Instance second Of Low { InputPort rst = route; }",
        "Instance vector Of Vector { InputPort rst = route, ~route; }"}) {
    pattern::Design design;
    design.AddIcl(
        "shared_polarity.icl",
        "Module Sink { ResetPort rst; } "
        "Module Low { ResetPort rst { ActivePolarity 0; } } "
        "Module Vector { ResetPort rst[1:0]; } Module M { ResetPort rst; "
        "DataInPort mode; DataMux route SelectedBy mode { "
        "1'b0: rst; 1'b1: 1'b0; } " +
            consumers + " }");
    Fails([&] { pattern::Elaborate(design, "M"); }, "reset mux path polarity");
  }
  {
    pattern::Design design;
    design.AddIcl("mux_default.icl",
                  "Module Sink { ResetPort rst; } Module M { "
                  "ResetPort rst { ActivePolarity 0; } DataInPort mode; "
                  "DataMux route SelectedBy mode { 1'b0: rst; 1'b1: 1'b0; } "
                  "Instance sink Of Sink { InputPort rst = route; } }");
    Fails([&] { pattern::Elaborate(design, "M"); }, "reset mux path polarity");
  }
}
void TestTrstMuxPathPolarity() {
  for (bool child_source : {false, true}) {
    const std::string root = child_source ? "driver.trst" : "trst";
    const std::string producer =
        child_source ? "Instance driver Of Driver; " : "TRSTPort trst; ";
    for (bool output : {false, true}) {
      for (bool branch_inverted : {false, true}) {
        for (bool output_inverted : {false, true}) {
          pattern::Design design;
          const std::string wire = output_inverted ? "~wrapped" : "wrapped";
          const std::string consumer =
              output
                  ? "ToTRSTPort out { Source " + wire + "; }"
                  : "Instance sink Of Sink { InputPort trst = " + wire + "; }";
          design.AddIcl(
              "trst_mux_polarity.icl",
              "Module Driver { ToTRSTPort trst; } Module Sink { TRSTPort trst; "
              "} "
              "Module M { " +
                  producer +
                  "DataInPort mode; DataInPort force; Alias branch = " +
                  (branch_inverted ? "~" : "") + root +
                  "; DataMux route SelectedBy mode { 1'b0: branch; 1'b1: "
                  "force; "
                  "} "
                  "Alias wrapped = route; " +
                  consumer + " }");
          if (branch_inverted != output_inverted) {
            Fails([&] { pattern::Elaborate(design, "M"); },
                  "TRST mux path polarity");
          } else {
            const auto serialized =
                pattern::SerializeIcl(pattern::Elaborate(design, "M"));
            pattern::Design flat;
            flat.AddIcl("flat.icl", serialized);
            Require(pattern::SerializeIcl(pattern::Elaborate(flat, "M")) ==
                        serialized,
                    "TRST mux parity survives source and flat replay");
          }
        }
      }
    }
  }
  {
    pattern::Design design;
    design.AddIcl("trst_mixed_trap.icl",
                  "Module Sink { TRSTPort trst; } Module M { TRSTPort trst; "
                  "ResetPort low { ActivePolarity 0; } DataInPort mode; "
                  "DataMux route SelectedBy mode { 1'b0: ~trst; 1'b1: low; } "
                  "ToResetPort out { Source route; ActivePolarity 0; } "
                  "Instance sink Of Sink { InputPort trst = route; } }");
    Fails([&] { pattern::Elaborate(design, "M"); }, "TRST mux path polarity");
  }
  for (const std::string rows :
       {"1'b0: trst; 1'b1: ~trst;", "1'b0: ~inner; 1'b1: 1'b1;"}) {
    pattern::Design design;
    design.AddIcl("trst_alternatives.icl",
                  "Module Sink { TRSTPort trst; } Module M { TRSTPort trst; "
                  "DataInPort mode; DataMux inner SelectedBy mode { "
                  "1'b0: ~trst; 1'b1: 1'b0; } "
                  "DataMux route SelectedBy mode { " +
                      rows +
                      " } Instance sink Of Sink { InputPort trst = route; } }");
    const auto serialized =
        pattern::SerializeIcl(pattern::Elaborate(design, "M"));
    pattern::Design flat;
    flat.AddIcl("flat.icl", serialized);
    Require(
        pattern::SerializeIcl(pattern::Elaborate(flat, "M")) == serialized,
        "TRST alternatives and nested cancellation retain possible low path");
  }
  {
    pattern::Design design;
    design.AddIcl("implicit_trst.icl", "Module M { ToTRSTPort startup; }");
    const auto original = pattern::Elaborate(design, "M");
    const auto& source = original.NodeAt("startup").inputs.at("Source")[0];
    Require(
        source.virtual_signal == pattern::VirtualSignal::kAsyncGlobalReset &&
            source.inverted,
        "implicit asynchronous TRST retains normalized low polarity");
    pattern::Design flat;
    const auto serialized = pattern::SerializeIcl(original);
    flat.AddIcl("flat.icl", serialized);
    Require(pattern::SerializeIcl(pattern::Elaborate(flat, "M")) == serialized,
            "implicit virtual TRST retains flat identity");
  }
}
void TestControlMuxIdentityPaths() {
  for (const std::string family : {"CaptureEn", "UpdateEn", "TMS"}) {
    const std::string port = family + "Port";
    for (bool child_source : {false, true}) {
      for (bool output : {false, true}) {
        for (bool branch_inverted : {false, true}) {
          for (bool output_inverted : {false, true}) {
            const std::string source = child_source ? "driver.out" : "ctl";
            const std::string producer =
                child_source
                    ? "Instance driver Of Driver { InputPort incoming = ctl; } "
                    : "";
            const std::string wire = output_inverted ? "~wrapped" : "wrapped";
            const std::string consumer =
                output
                    ? "To" + port + " out { Source " + wire + "; }"
                    : "Instance sink Of Sink { InputPort ctl = " + wire + "; }";
            pattern::Design design;
            design.AddIcl(
                "identity_path.icl",
                "Module Driver { " + port + " incoming; To" + port +
                    " out { Source incoming; } } Module Sink { " + port +
                    " ctl; } Module M { " + port +
                    " ctl; DataInPort mode; DataInPort force; " + producer +
                    "Alias branch = " + (branch_inverted ? "~" : "") + source +
                    "; DataMux route SelectedBy mode { 1'b0: branch; 1'b1: " +
                    (family == "TMS" ? "force" : "1'b0") +
                    "; } Alias wrapped = route; " + consumer + " }");
            if (branch_inverted != output_inverted) {
              Fails([&] { pattern::Elaborate(design, "M"); },
                    family + " mux path polarity");
            } else {
              const auto serialized =
                  pattern::SerializeIcl(pattern::Elaborate(design, "M"));
              pattern::Design flat;
              flat.AddIcl("flat.icl", serialized);
              Require(pattern::SerializeIcl(pattern::Elaborate(flat, "M")) ==
                          serialized,
                      "normal control path survives alias/producer and flat "
                      "replay");
            }
          }
        }
      }
    }
    {
      pattern::Design design;
      const std::string forcing = family == "TMS" ? "force" : "1'b0";
      design.AddIcl(
          "nested_identity.icl",
          "Module Sink { " + port + " ctl; } Module M { " + port +
              " ctl; DataInPort mode[1:0]; DataInPort force; "
              "DataMux inner SelectedBy mode[0] { 1'b0: ~ctl; 1'b1: " +
              forcing +
              "; } DataMux outer SelectedBy mode[1] { 1'b0: ~inner; 1'b1: " +
              forcing +
              "; } Instance sink Of Sink { InputPort ctl = outer; } }");
      const auto serialized =
          pattern::SerializeIcl(pattern::Elaborate(design, "M"));
      pattern::Design flat;
      flat.AddIcl("flat.icl", serialized);
      Require(
          pattern::SerializeIcl(pattern::Elaborate(flat, "M")) == serialized,
          "nested cancellation supplies identity; suppression is separate");
    }
    for (const std::string consumers :
         {"Instance first Of Sink { InputPort ctl = route; } "
          "Instance second Of Sink { InputPort ctl = ~route; }",
          "Instance vector Of Vector { InputPort ctl = route, ~route; }"}) {
      pattern::Design design;
      design.AddIcl(
          "shared_identity.icl",
          "Module Sink { " + port + " ctl; } Module Vector { " + port +
              " ctl[1:0]; } Module M { " + port +
              " ctl; DataInPort mode; DataMux route SelectedBy mode { "
              "1'b0: ctl; 1'b1: 1'b0; } " +
              consumers + " }");
      Fails([&] { pattern::Elaborate(design, "M"); },
            family + " mux path polarity");
    }
    {
      pattern::Design design;
      design.AddIcl(
          "unused_identity.icl",
          "Module M { " + port +
              " ctl; DataInPort mode; DataMux unused SelectedBy mode { "
              "1'b0: ~ctl; 1'b1: 1'b0; } }");
      const auto serialized =
          pattern::SerializeIcl(pattern::Elaborate(design, "M"));
      pattern::Design flat;
      flat.AddIcl("flat.icl", serialized);
      Require(
          pattern::SerializeIcl(pattern::Elaborate(flat, "M")) == serialized,
          "unused inverted control mux retains its existing boundary");
    }
    if (family != "TMS") {
      pattern::Design design;
      design.AddIcl(
          "data_enable_scope.icl",
          "Module Sink { " + port +
              " ctl; } Module M { DataInPort data[1:0]; DataInPort mode; "
              "DataMux route SelectedBy mode { 1'b0: data[0]; 1'b1: data[1]; } "
              "Instance sink Of Sink { InputPort ctl = route; } }");
      const auto serialized =
          pattern::SerializeIcl(pattern::Elaborate(design, "M"));
      pattern::Design flat;
      flat.AddIcl("flat.icl", serialized);
      Require(
          pattern::SerializeIcl(pattern::Elaborate(flat, "M")) == serialized,
          "all-data enable consumers retain their existing scope");
    }
  }
}
void TestControlOutputSourceScope() {
  for (const std::string family :
       {"Reset", "TCK", "CaptureEn", "ShiftEn", "UpdateEn", "TMS", "TRST"}) {
    const std::string port = family + "Port";
    for (bool child_source : {false, true}) {
      for (bool alias : {false, true}) {
        for (bool output : {false, true}) {
          for (bool mux : {false, true}) {
            if (output && family == "TCK") continue;
            if (mux && (family == "TCK" || family == "ShiftEn")) continue;
            std::string source = child_source ? "driver.out" : "producer";
            std::string text = "Module Driver { " + port + " incoming; To" +
                               port +
                               " out { Source incoming; } } Module Sink { " +
                               port + " ctl; } Module M { " + port +
                               " incoming; DataInPort mode; DataInPort force; ";
            text += child_source
                        ? "Instance driver Of Driver { InputPort incoming = "
                          "incoming; } "
                        : "To" + port + " producer { Source incoming; } ";
            if (alias) {
              text += "Alias wrapped = " + source + "; ";
              source = "wrapped";
            }
            if (mux) {
              text +=
                  "DataMux route SelectedBy mode { 1'b0: " + source +
                  "; 1'b1: " +
                  (family == "CaptureEn" || family == "UpdateEn" ? "1'b0"
                                                                 : "force") +
                  "; } ";
              source = "route";
            }
            text += output
                        ? "To" + port + " out { Source " + source + "; } }"
                        : "Instance sink Of Sink { InputPort ctl = " + source +
                              "; } }";
            pattern::Design design;
            design.AddIcl("control_scope.icl", text);
            if (!child_source) {
              Fails([&] { pattern::Elaborate(design, "M"); },
                    "output must be referenced from an instance");
            } else {
              const auto serialized =
                  pattern::SerializeIcl(pattern::Elaborate(design, "M"));
              pattern::Design flat;
              flat.AddIcl("flat.icl", serialized);
              Require(pattern::SerializeIcl(pattern::Elaborate(flat, "M")) ==
                          serialized,
                      "instance control outputs preserve original/flat "
                      "reference scopes");
            }
          }
        }
      }
    }
    pattern::Design alias_only;
    alias_only.AddIcl(
        "pdl_control_alias.icl",
        "Module M { " + port + " incoming; To" + port +
            " producer { Source incoming; } Alias wrapped = producer; }");
    const auto serialized =
        pattern::SerializeIcl(pattern::Elaborate(alias_only, "M"));
    pattern::Design flat;
    flat.AddIcl("flat.icl", serialized);
    Require(pattern::SerializeIcl(pattern::Elaborate(flat, "M")) == serialized,
            "own output aliases remain available outside hardware consumption");
  }
  {
    pattern::Design design;
    design.AddIcl(
        "altered_control_scope.icl",
        "Module Driver { ResetPort incoming; ToResetPort out { Source "
        "incoming; } } "
        "Module Sink { ResetPort ctl; } Module M { ResetPort incoming; "
        "ToResetPort producer { Source incoming; } "
        "Instance driver Of Driver { InputPort incoming = incoming; } "
        "Instance sink Of Sink { InputPort ctl = driver.out; } }");
    auto original = pattern::Elaborate(design, "M");
    original.nodes.at(original.node_ids.at("sink.ctl"))
        .inputs.at("Source")[0]
        .node = original.node_ids.at("producer");
    pattern::Design altered;
    altered.AddIcl("altered.icl", pattern::SerializeIcl(original));
    Fails([&] { pattern::Elaborate(altered, "M"); },
          "output must be referenced from an instance");
  }
  for (const std::string binding : {"InputPort ctl = driver.out, producer;",
                                    "InputPort ctl = producer, driver.out;",
                                    "InputPort ctl[1] = driver.out;"}) {
    pattern::Design design;
    design.AddIcl(
        "vector_control_scope.icl",
        "Module Driver { TMSPort incoming; ToTMSPort out { Source incoming; } "
        "} "
        "Module Sink { TMSPort ctl[1:0]; } Module M { TMSPort incoming; "
        "ToTMSPort producer { Source incoming; } "
        "Instance driver Of Driver { InputPort incoming = incoming; } "
        "Instance sink Of Sink { " +
            binding + " } }");
    if (binding.find("producer") != std::string::npos) {
      Fails([&] { pattern::Elaborate(design, "M"); },
            "output must be referenced from an instance");
    } else {
      const auto serialized =
          pattern::SerializeIcl(pattern::Elaborate(design, "M"));
      pattern::Design flat;
      flat.AddIcl("flat.icl", serialized);
      Require(
          pattern::SerializeIcl(pattern::Elaborate(flat, "M")) == serialized,
          "partial instance-output binding retains unbound bits through flat "
          "replay");
    }
  }
  {
    pattern::Design design;
    design.AddIcl(
        "semantic_reset_scope.icl",
        "Module M { ResetPort rst; ToResetPort producer { Source rst; } "
        "ScanInPort si; ScanRegister r { ScanInSource si; ResetValue 1'b0; } "
        "}");
    auto original = pattern::Elaborate(design, "M");
    original.nodes.at(original.node_ids.at("r")).reset =
        pattern::BitReference{original.node_ids.at("producer"), 0, 'x', false};
    pattern::Design flat;
    const auto serialized = pattern::SerializeIcl(original);
    flat.AddIcl("flat.icl", serialized);
    const auto replay = pattern::Elaborate(flat, "M");
    Require(replay.NodeAt("r").reset->node == replay.node_ids.at("producer") &&
                pattern::SerializeIcl(replay) == serialized,
            "semantic reset associations are not hardware Source expressions");
  }
}
void TestOrdinaryOutputSourceScope() {
  const std::vector<std::string> data_consumers = {
      "DataOutPort result { Source SOURCE; }",
      "ToSelectPort result { Source SOURCE; }",
      "ScanRegister r { ScanInSource si; CaptureSource SOURCE; }",
      "DataRegister r { WriteDataSource SOURCE; WriteEnSource 1'b1; }",
      "DataRegister r { WriteDataSource 1'b0; WriteEnSource SOURCE; }",
      "DataRegister r { ReadDataSource SOURCE; }",
      "DataOutPort result { Source 1'b0; Enable SOURCE; }",
      "ScanOutPort result { Source si; Enable SOURCE; }",
      "DataMux route SelectedBy SOURCE { 1'b0: 1'b0; 1'b1: 1'b1; }",
      "DataMux route SelectedBy 1'b0 { 1'b0: SOURCE; }",
      "ScanMux route SelectedBy SOURCE { 1'b0: si; 1'b1: si; }",
      "ClockMux route SelectedBy SOURCE { 1'b0: ck; 1'b1: ck; }",
      "LogicSignal route { SOURCE == 1'b0; }",
      "ToClockPort result { Source SOURCE; }",
      "ToCaptureEnPort result { Source SOURCE; }",
      "ToShiftEnPort result { Source SOURCE; }",
      "ToUpdateEnPort result { Source SOURCE; }",
      "ScanMux route SelectedBy 1'b0 { 1'b0: SOURCE; }",
      "ClockMux route SelectedBy 1'b0 { 1'b0: SOURCE; }"};
  for (const std::string output : {"DataOutPort", "ToSelectPort", "ScanOutPort",
                                   "ToClockPort", "ToTCKPort"}) {
    const bool data = output == "DataOutPort" || output == "ToSelectPort";
    const bool scan = output == "ScanOutPort";
    const std::string input = data                      ? "DataInPort"
                              : scan                    ? "ScanInPort"
                              : output == "ToClockPort" ? "ClockPort"
                                                        : "TCKPort";
    auto consumers = data ? data_consumers : std::vector<std::string>{};
    consumers.push_back("Instance sink Of Sink { InputPort ctl = SOURCE; }");
    if (scan) {
      consumers.insert(consumers.end(),
                       {"ScanOutPort result { Source SOURCE; }",
                        "ScanRegister r { ScanInSource SOURCE; }",
                        "ScanMux route SelectedBy 1'b0 { 1'b0: SOURCE; }"});
    } else if (!data) {
      consumers.insert(consumers.end(),
                       {"ToClockPort result { Source SOURCE; }",
                        "ClockMux route SelectedBy 1'b0 { 1'b0: SOURCE; }"});
    }
    const std::string prefix = "Module Driver { " + input + " incoming; " +
                               output +
                               " out { Source incoming; } } Module Sink { " +
                               input + " ctl; } Module M { " + input +
                               " incoming; ScanInPort si; ClockPort ck; ";
    for (const auto& consumer : consumers) {
      for (bool child : {false, true}) {
        for (bool alias : {false, true}) {
          const std::string producer = child ? "driver.out" : "producer";
          auto body = consumer;
          body.replace(body.find("SOURCE"), 6, alias ? "wrapped" : producer);
          pattern::Design design;
          design.AddIcl(
              "ordinary_scope.icl",
              prefix +
                  (child ? "Instance driver Of Driver { InputPort incoming = "
                           "incoming; } "
                         : output + " producer { Source incoming; } ") +
                  (alias ? "Alias wrapped = " + producer + "; " : "") + body +
                  " }");
          if (!child) {
            Fails([&] { pattern::Elaborate(design, "M"); },
                  "output must be referenced from an instance");
          } else {
            const auto text =
                pattern::SerializeIcl(pattern::Elaborate(design, "M"));
            pattern::Design flat;
            flat.AddIcl("flat.icl", text);
            Require(
                pattern::SerializeIcl(pattern::Elaborate(flat, "M")) == text,
                "ordinary child outputs preserve original/flat scope");
          }
        }
      }
    }
    for (const std::string binding :
         {"InputPort ctl = driver.out[0], producer[0];",
          "InputPort ctl = producer[0], driver.out[0];",
          "InputPort ctl[1] = driver.out[0];"}) {
      pattern::Design design;
      design.AddIcl(
          "ordinary_vector_scope.icl",
          "Module Driver { " + input + " incoming[1:0]; " + output +
              " out[1:0] { Source incoming; } } Module Sink { " + input +
              " ctl[1:0]; } Module M { " + input + " incoming[1:0]; " + output +
              " producer[1:0] { Source incoming; } Instance driver Of Driver { "
              "InputPort incoming = incoming; } Instance sink Of Sink { " +
              binding + " } }");
      if (binding.find("producer") != std::string::npos) {
        Fails([&] { pattern::Elaborate(design, "M"); },
              "output must be referenced from an instance");
      } else {
        const auto original = pattern::Elaborate(design, "M");
        const auto serialized = pattern::SerializeIcl(original);
        pattern::Design restored;
        restored.AddIcl("flat.icl", serialized);
        Require(
            pattern::SerializeIcl(pattern::Elaborate(restored, "M")) ==
                    serialized &&
                original.NodeAt("sink.ctl").inputs.at("Source")[1].constant ==
                    '?',
            "ordinary partial child-output bindings preserve unbound holes");
      }
    }
    pattern::Design alias_only;
    alias_only.AddIcl(
        "ordinary_alias.icl",
        prefix + output +
            " producer { Source incoming; } Alias wrapped = producer; }");
    const auto text =
        pattern::SerializeIcl(pattern::Elaborate(alias_only, "M"));
    pattern::Design flat;
    flat.AddIcl("flat.icl", text);
    Require(pattern::SerializeIcl(pattern::Elaborate(flat, "M")) == text,
            "own ordinary output aliases remain outside hardware consumption");
    pattern::Design altered_source;
    altered_source.AddIcl(
        "ordinary_altered.icl",
        prefix + output +
            " producer { Source incoming; } Instance driver Of Driver { "
            "InputPort incoming = incoming; } Instance sink Of Sink { "
            "InputPort ctl = driver.out; } }");
    auto original = pattern::Elaborate(altered_source, "M");
    original.nodes.at(original.node_ids.at("sink.ctl"))
        .inputs.at("Source")[0] = {original.node_ids.at("producer"), 0};
    pattern::Design altered;
    altered.AddIcl("altered.icl", pattern::SerializeIcl(original));
    Fails([&] { pattern::Elaborate(altered, "M"); },
          "output must be referenced from an instance");
  }
  for (bool child : {false, true}) {
    pattern::Design design;
    design.AddIcl("one_hot_output_scope.icl",
                  "Module Driver { DataInPort incoming; DataOutPort out { "
                  "Source incoming; "
                  "Enable 1'b1; } } Module M { DataInPort incoming; " +
                      std::string(child ? "Instance driver Of Driver { "
                                          "InputPort incoming = incoming; } "
                                        : "DataOutPort producer { Source "
                                          "incoming; Enable 1'b1; } ") +
                      "Alias wrapped = " + (child ? "driver.out" : "producer") +
                      "; OneHotDataGroup group { Port wrapped; } }");
    if (!child) {
      Fails([&] { pattern::Elaborate(design, "M"); },
            "output must be referenced from an instance");
    } else {
      const auto text = pattern::SerializeIcl(pattern::Elaborate(design, "M"));
      pattern::Design flat;
      flat.AddIcl("flat.icl", text);
      Require(
          pattern::SerializeIcl(pattern::Elaborate(flat, "M")) == text,
          "enabled child data contributor retains scope through flat replay");
    }
  }
  pattern::Design wrong_scan;
  wrong_scan.AddIcl(
      "wrong_scan_scope.icl",
      "Module M { DataInPort incoming; "
      "DataOutPort producer { Source incoming; } ScanRegister r { "
      "ScanInSource producer; } }");
  Fails([&] { pattern::Elaborate(wrong_scan, "M"); }, "requires a scan signal");
  pattern::Design differential;
  differential.AddIcl("output_pair.icl",
                      "Module M { ClockPort incoming; "
                      "ToClockPort positive { Source incoming; } "
                      "ToClockPort negative { DifferentialInvOf positive; } }");
  const auto text =
      pattern::SerializeIcl(pattern::Elaborate(differential, "M"));
  pattern::Design flat;
  flat.AddIcl("flat.icl", text);
  Require(pattern::SerializeIcl(pattern::Elaborate(flat, "M")) == text,
          "differential associations retain separate conversion semantics");
}
void TestScanControlSourceKinds() {
  for (const std::string family : {"CaptureEn", "ShiftEn", "UpdateEn"}) {
    pattern::Design design;
    design.AddIcl(
        "control_sources.icl",
        "Module Provider { " + family + "Port input[1:0]; To" + family +
            "Port output[1:0] { Source input; } } Module Child { " + family +
            "Port ctl[1:0]; } Module M { " + family + "Port input[1:0]; To" +
            family + "Port bare; To" + family +
            "Port output[1:0] { Source input; } DataInPort data; "
            "Instance provider Of Provider { InputPort input = input; } "
            "Alias wrapped[1:0] = provider.output; Instance direct Of Child { "
            "InputPort "
            "ctl "
            "= wrapped; } "
            "Instance partial Of Child { InputPort ctl[1] = data; } "
            "Instance constants Of Child { InputPort ctl = 2'b1x; } }");
    const auto original = pattern::Elaborate(design, "M");
    pattern::Design flat;
    flat.AddIcl("flat.icl", pattern::SerializeIcl(original));
    const auto restored = pattern::Elaborate(flat, "M");
    Require(
        restored.NodeAt("partial.ctl").inputs.at("Source").at(1).constant ==
                '?' &&
            pattern::SerializeIcl(restored) == pattern::SerializeIcl(original),
        "scan control family, data, constants and partial bits survive flat "
        "replay");
    for (const auto reset : {pattern::VirtualSignal::kGlobalReset,
                             pattern::VirtualSignal::kAsyncGlobalReset}) {
      auto changed = original;
      changed.nodes.at(changed.node_ids.at("partial.ctl"))
          .inputs.at("Source")
          .at(0) = {std::numeric_limits<size_t>::max(), 0, 'x', false, reset};
      pattern::Design invalid;
      invalid.AddIcl("invalid.icl", pattern::SerializeIcl(changed));
      Fails([&] { pattern::Elaborate(invalid, "M"); }, "or data signal");
    }
  }
}
void TestConcatScanSourceKinds() {
  pattern::Design design;
  design.AddIcl(
      "scan_sources.icl",
      "Module Child { ScanInPort si[1:0]; ScanOutPort so[1:0] { "
      "Source si; Enable 1'b1; } } Module M { ScanInPort si[1:0]; "
      "DataInPort data[1:0]; ScanRegister reg[1:0] { ScanInSource si[0]; } "
      "Alias mixed[1:0] = si[1], data[0]; Alias wrapped[1:0] = ~mixed; "
      "Instance child Of Child { InputPort si = wrapped; } "
      "Instance partial Of Child { InputPort si[1] = 1'bx; } "
      "Instance middle Of Child { InputPort si = reg; } "
      "ScanMux mux[1:0] SelectedBy data[0] { 1'b0 : reg; "
      "1'b1 : child.so; } OneHotScanGroup group[1:0] { Port child.so; } "
      "ScanOutPort out[1:0] { Source mux; } "
      "ScanOutPort grouped[1:0] { Source group; } "
      "ScanOutPort constants[1:0] { Source 2'b1x; } }");
  const auto original = pattern::Elaborate(design, "M");
  pattern::Design flat;
  flat.AddIcl("flat.icl", pattern::SerializeIcl(original));
  const auto restored = pattern::Elaborate(flat, "M");
  Require(
      pattern::SerializeIcl(restored) == pattern::SerializeIcl(original) &&
          restored.NodeAt("partial.si").inputs.at("Source").at(1).constant ==
              '?',
      "concat scan/data/constant/mixed alias and partial sources replay");
  for (const auto reset : {pattern::VirtualSignal::kGlobalReset,
                           pattern::VirtualSignal::kAsyncGlobalReset}) {
    auto changed = original;
    changed.nodes.at(changed.node_ids.at("out")).inputs.at("Source").at(0) = {
        std::numeric_limits<size_t>::max(), 0, 'x', false, reset};
    pattern::Design invalid;
    invalid.AddIcl("invalid.icl", pattern::SerializeIcl(changed));
    Fails([&] { pattern::Elaborate(invalid, "M"); }, "scan or data signal");
  }
  pattern::Design selector;
  selector.AddIcl("selector.icl",
                  "Module M { ScanInPort si; ScanMux mux SelectedBy si { "
                  "1'b0 : 1'b0; 1'b1 : 1'b1; } }");
  Fails([&] { pattern::Elaborate(selector, "M"); }, "requires a data signal");
  for (bool invalid : {false, true}) {
    pattern::Design mixed;
    mixed.AddIcl(
        "mixed_scan.icl",
        "Module M { ScanInPort si; TCKPort tck; "
        "Alias joined[1:0] = si, tck; Alias allowed = joined[1]; "
        "ScanOutPort good { Source allowed; } " +
            std::string(invalid ? "ScanOutPort bad { Source joined[0]; }"
                                : "") +
            " }");
    if (invalid) {
      Fails([&] { pattern::Elaborate(mixed, "M"); }, "scan or data signal");
    } else {
      pattern::Design replay;
      replay.AddIcl("flat.icl",
                    pattern::SerializeIcl(pattern::Elaborate(mixed, "M")));
      pattern::Elaborate(replay, "M");
    }
  }
}
void TestStrictScanSources() {
  pattern::Design wrong;
  wrong.AddIcl("wrong_scan.icl",
               "Module M { DataInPort data; Alias wrapped = data; "
               "ScanRegister target { ScanInSource wrapped; } }");
  Fails([&] { pattern::Elaborate(wrong, "M"); }, "requires a scan signal");
  for (const std::string bounds : {"[3:0]", "[0:3]"}) {
    const auto right = bounds == "[3:0]" ? "0" : "3";
    const auto middle = bounds == "[3:0]" ? "2" : "1";
    pattern::Design design;
    design.AddIcl("strict_scan.icl",
                  "Module M { ScanInPort si; ScanRegister upstream" + bounds +
                      " { ScanInSource si; } Alias parts[1:0] = upstream[" +
                      right + "], upstream[" + middle +
                      "]; Alias right = ~parts[1]; ScanRegister target { "
                      "ScanInSource right; } ScanRegister implicit { "
                      "ScanInSource upstream; } "
                      "DataOutPort data[3:0] { Source upstream; } "
                      "ToClockPort clock[3:0] { Source upstream; } "
                      "ToCaptureEnPort capture[3:0] { Source upstream; } "
                      "ToShiftEnPort shift[3:0] { Source upstream; } "
                      "ToUpdateEnPort update[3:0] { Source upstream; } "
                      "ScanMux mux SelectedBy 1'b0 { 1'b0 : parts[0]; } "
                      "ScanOutPort so { Source target; } }");
    const auto original = pattern::Elaborate(design, "M");
    Require(original.NodeAt("implicit").inputs.at("ScanInSource").at(0).index ==
                std::stoi(right),
            "omitted scan register index selects the declared rightmost bit");
    pattern::Design flat;
    flat.AddIcl("flat.icl", pattern::SerializeIcl(original));
    const auto restored = pattern::Elaborate(flat, "M");
    Require(pattern::SerializeIcl(restored) == pattern::SerializeIcl(original),
            "rightmost aliases and ordinary-data middle bits survive replay");
  }
  pattern::Design design;
  design.AddIcl("scan_terminals.icl",
                "Module Child { ScanInPort si; ScanRegister reg[1:0] { "
                "ScanInSource si; } "
                "ScanOutPort so { Source reg[0]; Enable 1'b1; } } Module M { "
                "ScanInPort si; "
                "Instance child Of Child { InputPort si = si; } "
                "OneHotScanGroup group { Port child.so; } "
                "ScanMux mux SelectedBy 1'b0 { 1'b0 : si; 1'b1 : group; } "
                "ScanRegister a { ScanInSource child.so; } "
                "ScanRegister b { ScanInSource group; } ScanRegister c { "
                "ScanInSource mux; } "
                "ScanRegister zero { ScanInSource 1'b0; } "
                "ScanRegister one { ScanInSource 1'b1; } "
                "ScanRegister unknown { ScanInSource 1'bx; } "
                "ScanOutPort so { Source c; } }");
  const auto original = pattern::Elaborate(design, "M");
  pattern::Design flat;
  flat.AddIcl("flat.icl", pattern::SerializeIcl(original));
  pattern::Elaborate(flat, "M");
  for (const auto reset : {pattern::VirtualSignal::kGlobalReset,
                           pattern::VirtualSignal::kAsyncGlobalReset}) {
    auto changed = original;
    changed.nodes.at(changed.node_ids.at("unknown"))
        .inputs.at("ScanInSource")
        .at(0) = {std::numeric_limits<size_t>::max(), 0, 'x', false, reset};
    pattern::Design invalid;
    invalid.AddIcl("invalid_scan_reset.icl", pattern::SerializeIcl(changed));
    Fails([&] { pattern::Elaborate(invalid, "M"); }, "requires a scan signal");
  }
  auto changed = original;
  changed.nodes.at(changed.node_ids.at("child.so"))
      .inputs.at("Source")
      .at(0)
      .index = 1;
  pattern::Design invalid;
  invalid.AddIcl("invalid_flat.icl", pattern::SerializeIcl(changed));
  Fails([&] { pattern::Elaborate(invalid, "M"); }, "rightmost bit");
}
void TestAddressPortSourceKinds() {
  pattern::Design wrong;
  wrong.AddIcl("wrong_address.icl",
               "Module Child { AddressPort a; } Module M { TCKPort ck; "
               "Instance child Of Child { InputPort a = ck; } }");
  Fails([&] { pattern::Elaborate(wrong, "M"); },
        "AddressPort, DataInPort, DataOutPort or ScanRegister");
  for (const std::string family : {"Address", "WriteEn", "ReadEn"}) {
    pattern::Design design;
    design.AddIcl(
        "address_sources.icl",
        "Module Child { " + family +
            "Port ctl; } Module Proxy { DataInPort d; DataOutPort out { "
            "Source d; } } Module M { " +
            family +
            "Port parent; DataInPort data; ScanInPort si; "
            "ScanRegister reg[1:0] { ScanInSource si; } "
            "ClockPort forbidden; Alias mixed[1:0] = parent, forbidden; "
            "Alias wrapped = mixed[1]; "
            "Instance own Of Child { InputPort ctl = wrapped; } "
            "Instance primary Of Child { InputPort ctl = data; } "
            "Instance right Of Child { InputPort ctl = reg[0]; } "
            "Instance middle Of Child { InputPort ctl = reg[1]; } "
            "Instance proxy Of Proxy { InputPort d = data; } "
            "Instance output Of Child { InputPort ctl = proxy.out; } }");
    const auto original = pattern::Elaborate(design, "M");
    pattern::Design flat;
    flat.AddIcl("flat.icl", pattern::SerializeIcl(original));
    Require(
        pattern::SerializeIcl(pattern::Elaborate(flat, "M")) ==
            pattern::SerializeIcl(original),
        "address families, data ports and register bits retain flat origins");
    auto forbidden = original;
    forbidden.nodes.at(forbidden.node_ids.at("primary.ctl"))
        .inputs.at("Source")
        .at(0) = {forbidden.node_ids.at("mixed"), 0};
    pattern::Design invalid_slice;
    invalid_slice.AddIcl("invalid_slice.icl", pattern::SerializeIcl(forbidden));
    Fails([&] { pattern::Elaborate(invalid_slice, "M"); },
          family + "Port, DataInPort, DataOutPort or ScanRegister");
    for (const auto reset : {pattern::VirtualSignal::kGlobalReset,
                             pattern::VirtualSignal::kAsyncGlobalReset}) {
      auto changed = original;
      changed.nodes.at(changed.node_ids.at("own.ctl"))
          .inputs.at("Source")
          .at(0) = {std::numeric_limits<size_t>::max(), 0, 'x', false, reset};
      pattern::Design invalid;
      invalid.AddIcl("invalid_address.icl", pattern::SerializeIcl(changed));
      Fails([&] { pattern::Elaborate(invalid, "M"); },
            family + "Port, DataInPort, DataOutPort or ScanRegister");
    }
  }
  pattern::Design partial;
  partial.AddIcl("partial_address.icl",
                 "Module Child { AddressPort a[1:0]; } Module M { "
                 "DataInPort data; Instance child Of Child { InputPort a[1] = "
                 "data; } }");
  const auto original = pattern::Elaborate(partial, "M");
  pattern::Design flat;
  flat.AddIcl("flat.icl", pattern::SerializeIcl(original));
  const auto restored = pattern::Elaborate(flat, "M");
  Require(
      restored.NodeAt("child.a").inputs.at("Source").at(1).constant == '?',
      "eligibility preserves binding holes for contextual domain validation");
}
void TestTmsTrstSourceKinds() {
  for (const std::string family : {"TMS", "TRST"}) {
    pattern::Design design;
    design.AddIcl(
        "tap_pin_sources.icl",
        "Module Provider { " + family + "Port input[1:0]; To" + family +
            "Port output[1:0] { Source input; } } Module Child { " + family +
            "Port ctl[1:0]; } Module M { " + family + "Port input[1:0]; To" +
            family + "Port bare[1:0] { Source input; } To" + family +
            "Port output[1:0] { Source input; } DataInPort data; "
            "DataMux gate SelectedBy data { 1'b0 : input[1]; 1'b1 : 1'b0; } "
            "Instance provider Of Provider { InputPort input = input; } "
            "Alias wrapped[1:0] = provider.output; Instance direct Of Child { "
            "InputPort "
            "ctl = wrapped; } "
            "Instance partial Of Child { InputPort ctl[1] = gate; } "
            "Instance constants Of Child { InputPort ctl = 2'b1x; } }");
    const auto original = pattern::Elaborate(design, "M");
    pattern::Design flat;
    flat.AddIcl("flat.icl", pattern::SerializeIcl(original));
    const auto restored = pattern::Elaborate(flat, "M");
    Require(pattern::SerializeIcl(restored) == pattern::SerializeIcl(original),
            "TMS/TRST own/mux/constant/partial and inferred output sources "
            "survive flat replay");
  }
  pattern::Design design;
  design.AddIcl(
      "tap_pin_inference.icl",
      "Module Child { TMSPort tm; TRSTPort tr; ToTMSPort forwarded; } "
      "Module M { TMSPort input; ToTRSTPort powerup; "
      "Instance child Of Child { InputPort tm = input; } }");
  const auto original = pattern::Elaborate(design, "M");
  const auto& async = original.NodeAt("powerup").inputs.at("Source").at(0);
  Require(async.virtual_signal == pattern::VirtualSignal::kAsyncGlobalReset &&
              async.inverted,
          "inferred active-low TRST retains asynchronous reset polarity");
  pattern::Design flat;
  flat.AddIcl("flat.icl", pattern::SerializeIcl(original));
  pattern::Elaborate(flat, "M");
  for (const std::string name : {"powerup", "child.forwarded"}) {
    for (const auto reset : {pattern::VirtualSignal::kGlobalReset,
                             pattern::VirtualSignal::kAsyncGlobalReset}) {
      if (name == "powerup" &&
          reset == pattern::VirtualSignal::kAsyncGlobalReset)
        continue;
      auto changed = original;
      changed.nodes.at(changed.node_ids.at(name)).inputs.at("Source").at(0) = {
          std::numeric_limits<size_t>::max(), 0, 'x', false, reset};
      pattern::Design invalid;
      invalid.AddIcl("invalid.icl", pattern::SerializeIcl(changed));
      Fails([&] { pattern::Elaborate(invalid, "M"); }, "or data signal");
    }
  }
}
void TestInferredControlVectors() {
  for (const std::string type : {"Reset", "TRST", "TMS"}) {
    const std::string polarity =
        type == "Reset" ? " { ActivePolarity 0; }" : ";";
    pattern::Design design;
    design.AddIcl("inferred_vectors.icl",
                  "Module Child { " + type + "Port input[9:8]; To" + type +
                      "Port forwarded[6:7] { Source input; } } Module M { " +
                      type + "Port source[7]" + polarity + " To" + type +
                      "Port bare[1:0]; Instance child Of Child; }");
    const auto original = pattern::Elaborate(design, "M");
    for (const std::string name : {"bare", "child.input"}) {
      const auto& bits = original.NodeAt(name).inputs.at("Source");
      Require(bits.size() == 2 && bits[0].index == 7 && bits[1].index == 7 &&
                  bits[0].inverted == (type == "Reset") &&
                  bits[1].inverted == (type == "Reset"),
              "implicit ports repeat a unique scalar source with polarity");
    }
    pattern::Design flat;
    flat.AddIcl("flat.icl", pattern::SerializeIcl(original));
    const auto restored = pattern::Elaborate(flat, "M");
    Require(pattern::SerializeIcl(restored) == pattern::SerializeIcl(original),
            "inferred scalar-to-vector sources survive flat replay");
    for (bool wide_target : {false, true}) {
      pattern::Design ambiguous;
      ambiguous.AddIcl("ambiguous_inference.icl",
                       "Module M { " + type + "Port source[4:5]; To" + type +
                           "Port target" + (wide_target ? "[1:0]" : "") +
                           "; }");
      Fails([&] { pattern::Elaborate(ambiguous, "M"); }, "ambiguous source");
    }
    for (bool ascending : {false, true}) {
      pattern::Design explicit_design;
      explicit_design.AddIcl("explicit_vectors.icl",
                             "Module M { " + type + "Port source" +
                                 (ascending ? "[4:5]" : "[5:4]") + "; To" +
                                 type +
                                 "Port target[1:0] { Source source; } }");
      const auto explicit_netlist = pattern::Elaborate(explicit_design, "M");
      const auto& bits = explicit_netlist.NodeAt("target").inputs.at("Source");
      Require(bits[0].index == (ascending ? 4 : 5) &&
                  bits[1].index == (ascending ? 5 : 4),
              "explicit vector references retain declaration order");
    }
  }
  pattern::Design ambiguous_reset;
  ambiguous_reset.AddIcl("ambiguous_register_reset.icl",
                         "Module M { ResetPort rst[1:0]; ScanInPort si; "
                         "ScanRegister r { ScanInSource si; ResetValue 0; } }");
  Fails([&] { pattern::Elaborate(ambiguous_reset, "M"); },
        "ambiguous register reset");
  for (const std::string type : {"Reset", "TRST"}) {
    pattern::Design design;
    design.AddIcl("virtual_vectors.icl",
                  "Module Child { " + type +
                      "Port input[3:1]; } Module M { To" + type +
                      "Port output[2:0]; Instance child Of Child; }");
    const auto original = pattern::Elaborate(design, "M");
    for (const std::string name : {"output", "child.input"}) {
      const auto& bits = original.NodeAt(name).inputs.at("Source");
      Require(bits.size() == 3,
              "virtual reset function drives every declared bit");
      for (const auto& bit : bits) {
        Require(
            bit.virtual_signal ==
                    (type == "TRST" ? pattern::VirtualSignal::kAsyncGlobalReset
                                    : pattern::VirtualSignal::kGlobalReset) &&
                bit.inverted == (type == "TRST"),
            "broadcast virtual reset preserves function and polarity");
      }
    }
    pattern::Design flat;
    flat.AddIcl("flat.icl", pattern::SerializeIcl(original));
    pattern::Elaborate(flat, "M");
  }
  const auto start = std::chrono::steady_clock::now();
  const std::string source =
      "Module Child { TRSTPort tr[0:65535]; } Module M { TRSTPort x; "
      "ToTRSTPort output[65535:0]; Instance child Of Child; }";
  pattern::Design design;
  design.AddIcl("wide_inferred.icl", source);
  const auto original = pattern::Elaborate(design, "M");
  Require(original.NodeAt("child.tr").inputs.at("Source").size() == 65536,
          "wide inferred source retains every bit");
  pattern::Design flat;
  flat.AddIcl("flat.icl", pattern::SerializeIcl(original));
  pattern::Elaborate(flat, "M");
  std::cout << "inferred control vectors: bits=65536 bytes=" << source.size()
            << " source/flat seconds="
            << std::chrono::duration<double>(std::chrono::steady_clock::now() -
                                             start)
                   .count()
            << '\n';
}
void TestInferredControlBudget() {
  std::string source = "Module M { ";
  for (size_t i = 0; i < 65; ++i) {
    source += "ToTRSTPort port" + std::to_string(i) + "[65535:0];";
  }
  source += "}";
  const auto start = std::chrono::steady_clock::now();
  pattern::Design design;
  design.AddIcl("inferred_budget.icl", source);
  Fails([&] { pattern::Elaborate(design, "M"); },
        "inferred control connection bit limit");
  std::cout << "inferred control budget: ports=65 width=65536 bytes="
            << source.size() << " seconds="
            << std::chrono::duration<double>(std::chrono::steady_clock::now() -
                                             start)
                   .count()
            << '\n';
}
void TestDataSignalSources() {
  const std::string prefix =
      "Module Sink { DataInPort data; SelectPort selected; } Module M {"
      "ScanInPort si; TCKPort clock; Alias clock_alias = clock;";
  const std::vector<std::string> consumers = {
      "DataOutPort result { Source SOURCE; }",
      "ToSelectPort result { Source SOURCE; }",
      "ScanRegister value { ScanInSource si; CaptureSource SOURCE; }",
      "DataRegister value { WriteDataSource SOURCE; WriteEnSource 1'b1; }",
      "DataRegister value { WriteDataSource 1'b0; WriteEnSource SOURCE; }",
      "DataRegister value { ReadDataSource SOURCE; }",
      "DataOutPort result { Source 1'b0; Enable SOURCE; }",
      "ScanOutPort result { Source si; Enable SOURCE; }",
      "DataMux value SelectedBy SOURCE { 1'b0: 0; 1'b1: 1; }",
      "DataMux value SelectedBy 1'b0 { 1'b0: SOURCE; }",
      "ScanMux value SelectedBy SOURCE { 1'b0: si; 1'b1: si; }",
      "ClockMux value SelectedBy SOURCE { 1'b0: clock; 1'b1: clock; }",
      "LogicSignal value { SOURCE == 1'b0; }",
      "Instance sink Of Sink { InputPort data = SOURCE; }",
      "Instance sink Of Sink { InputPort selected = SOURCE; }"};
  for (const auto& consumer : consumers) {
    for (const std::string source : {"clock", "clock_alias"}) {
      auto body = consumer;
      body.replace(body.find("SOURCE"), 6, source);
      pattern::Design design;
      design.AddIcl("data_sources.icl", prefix + body + "}");
      Fails([&] { pattern::Elaborate(design, "M"); }, "data signal");
    }
  }
  for (const std::string declaration :
       {"ScanInPort source;", "ScanOutPort source { Source 1'b0; }",
        "TCKPort source;", "ClockPort source;", "ResetPort source;",
        "CaptureEnPort source;", "ShiftEnPort source;", "UpdateEnPort source;",
        "TMSPort source;", "TRSTPort source;", "ToTCKPort source;",
        "ToResetPort source;", "ToCaptureEnPort source;",
        "ToShiftEnPort source;", "ToUpdateEnPort source;",
        "ToTRSTPort source;"}) {
    pattern::Design design;
    design.AddIcl(
        "wrong_kind.icl",
        "Module M {" + declaration + "DataOutPort result { Source source; }}");
    Fails([&] { pattern::Elaborate(design, "M"); }, "data signal");
  }
  for (const auto& entry : std::vector<std::pair<std::string, std::string>>{
           {"DataInPort source;", "source"},
           {"SelectPort source;", "source"},
           {"ScanInPort si; ScanRegister source { ScanInSource si; }",
            "source"},
           {"ScanInPort si; ScanRegister source[1:0] { ScanInSource si; }",
            "source[0]"},
           {"ScanInPort si; ScanRegister source[1:0] { ScanInSource si; }",
            "source[1]"},
           {"DataRegister source;", "source"},
           {"LogicSignal source { 1'b1; }", "source"},
           {"DataMux source SelectedBy 1'b0 { 1'b0: 0; 1'b1: 1; }", "source"},
           {"Instance child Of Leaf;", "child.data"},
           {"Instance child Of Leaf;", "child.select"},
           {"Instance child Of Leaf; OneHotDataGroup source { Port child.data; "
            "}",
            "source"},
           {"", "1'b0"},
           {"", "1'b1"},
           {"", "1'bX"}}) {
    for (const std::string consumer :
         {"DataOutPort result { Source " + entry.second + "; }",
          "DataMux result SelectedBy 1'b0 { 1'b0 : " + entry.second + "; }"}) {
      pattern::Design design;
      design.AddIcl(
          "valid_data.icl",
          "Module Leaf { DataOutPort data { Source 1'b1; Enable 1'b1; }"
          "ToSelectPort select { Source 1'b1; }} Module M {" +
              entry.first + consumer + "}");
      const auto original = pattern::Elaborate(design, "M");
      pattern::Design flat;
      flat.AddIcl("flat.icl", pattern::SerializeIcl(original));
      pattern::Elaborate(flat, "M");
    }
  }
}

void TestDataMuxMatchingControlUses() {
  pattern::Design wrong;
  wrong.AddIcl("wrong_mux_use.icl",
               "Module M { ResetPort source; DataInPort mode; "
               "DataMux mux SelectedBy mode { 1'b0 : 1'b1; 1'b1 : source; } "
               "DataOutPort out { Source mux; } }");
  Fails([&] { pattern::Elaborate(wrong, "M"); }, "matching control use");
  for (const std::string family :
       {"Reset", "CaptureEn", "UpdateEn", "TMS", "TRST"}) {
    const std::string force = family == "Reset" ? "1'b1" : "1'b0";
    pattern::Design design;
    design.AddIcl("matching_mux.icl",
                  "Module M { " + family +
                      "Port source; DataInPort mode; DataInPort data; "
                      "DataMux inner SelectedBy mode { 1'b0 : " +
                      force +
                      "; 1'b1 : source; } Alias wrapped = inner; "
                      "DataMux outer SelectedBy mode { 1'b0 : " +
                      force + "; 1'b1 : wrapped; } To" + family +
                      "Port out { Source outer; } DataRegister observed { "
                      "WriteDataSource data; WriteEnSource inner; } }");
    const auto original = pattern::Elaborate(design, "M");
    pattern::Design flat;
    flat.AddIcl("flat.icl", pattern::SerializeIcl(original));
    Require(pattern::SerializeIcl(pattern::Elaborate(flat, "M")) ==
                pattern::SerializeIcl(original),
            "matching mux chains and standard-interpreted mixed fanout replay");
  }
  for (const std::string bounds : {"[1:0]", "[0:1]"}) {
    const std::string used = bounds == "[1:0]" ? "1" : "0";
    pattern::Design design;
    design.AddIcl(
        "partial_mux_use.icl",
        "Module Child { ResetPort ctl { ActivePolarity 0; } } "
        "Module M { ResetPort source" +
            bounds + "; DataInPort mode; DataMux mux" + bounds +
            " SelectedBy mode { 1'b0 : 2'b11; 1'b1 : source; } Alias wrapped" +
            bounds +
            " = ~mux; Instance child Of Child { InputPort ctl = wrapped[" +
            used + "]; } }");
    const auto original = pattern::Elaborate(design, "M");
    pattern::Design flat;
    flat.AddIcl("flat.icl", pattern::SerializeIcl(original));
    pattern::Elaborate(flat, "M");
  }
  pattern::Design dangling;
  dangling.AddIcl(
      "unused_mux.icl",
      "Module M { ResetPort source; DataInPort mode; "
      "DataMux first SelectedBy mode { 1'b0 : 1'b1; 1'b1 : source; } "
      "Alias middle = first; DataMux second SelectedBy mode { "
      "1'b0 : 1'b1; 1'b1 : middle; } Alias unused = second; }");
  pattern::Elaborate(dangling, "M");
  for (bool forward : {false, true}) {
    pattern::Design design;
    design.AddIcl(
        "trst_reset_mux.icl",
        "Module Provider { TRSTPort input; ToTRSTPort out { Source input; } } "
        "Module M { TRSTPort input; DataInPort mode; Instance provider Of "
        "Provider { InputPort input = input; } DataMux mux SelectedBy mode { "
        "1'b0 : 1'b0; 1'b1 : " +
            std::string(forward ? "provider.out" : "input") +
            "; } ToResetPort out { Source mux; ActivePolarity 0; } }");
    const auto original = pattern::Elaborate(design, "M");
    pattern::Design flat;
    flat.AddIcl("flat.icl", pattern::SerializeIcl(original));
    pattern::Elaborate(flat, "M");
  }
  pattern::Design reset_reference;
  reset_reference.AddIcl(
      "mux_reset_reference.icl",
      "Module M { ResetPort source; DataInPort mode; ScanInPort si; "
      "DataMux mux SelectedBy mode { 1'b0 : 1'b1; 1'b1 : source; } "
      "Alias reset_alias = mux; ScanRegister value { ScanInSource si; "
      "ResetValue 0; } ScanOutPort so { Source value; } }");
  const auto original = pattern::Elaborate(reset_reference, "M");
  for (const std::string name : {"mux", "reset_alias"}) {
    auto lowered = original;
    lowered.nodes.at(lowered.node_ids.at("value")).reset = {
        lowered.node_ids.at(name), 0};
    pattern::Design flat;
    flat.AddIcl("lowered_reset.icl", pattern::SerializeIcl(lowered));
    const auto restored = pattern::Elaborate(flat, "M");
    Require(
        restored.nodes.at(restored.NodeAt("value").reset->node).name == name,
        "lowered reset association supplies matching reset use");
  }
}
void TestDirectControlDataDrivers() {
  for (const std::string family : {"Reset", "TMS", "TRST"}) {
    const auto force = family == "Reset" ? "1'b1" : "1'b0";
    for (bool output : {false, true}) {
      const auto child =
          output ? "" : "Module Child { " + family + "Port ctl; } ";
      const auto consumer = [&](const std::string& source) {
        return output ? "To" + family + "Port out { Source " + source + "; }"
                      : "Instance child Of Child { InputPort ctl = " + source +
                            "; }";
      };
      for (const std::string declaration :
           {"DataInPort src;",
            "DataInPort data; DataOutPort src { Source data; }",
            "DataInPort data; LogicSignal src { data; }",
            "DataRegister src { WriteDataSource 1'b0; WriteEnSource 1'b0; "
            "ResetValue 0; }",
            "ScanInPort si; ScanRegister src { ScanInSource si; ResetValue 0; "
            "}",
            "ScanInPort si; ScanRegister src[1:0] { ScanInSource si; "
            "ResetValue 0; }"}) {
        const std::string reference =
            declaration.find("src[1:0]") == std::string::npos ? "src"
                                                              : "src[1]";
        for (const std::string source : {reference, std::string("wrapped")}) {
          pattern::Design design;
          design.AddIcl("direct_driver.icl",
                        child + "Module M { " + declaration +
                            " Alias wrapped = " + reference + "; " +
                            consumer(source) + " }");
          Fails([&] { pattern::Elaborate(design, "M"); },
                "requires a direct DataMux data driver");
        }
        pattern::Design mux;
        const bool output_source =
            declaration.find("DataOutPort src") != std::string::npos;
        const auto provider =
            output_source ? "Module Provider { " + declaration + " } " : "";
        const auto local =
            output_source ? "DataInPort data; Instance provider Of Provider { "
                            "InputPort data = data; } "
                          : declaration;
        mux.AddIcl("mux_driver.icl",
                   provider + child + "Module M { " + local + " " + family +
                       "Port ctrl; DataMux route SelectedBy " +
                       (output_source ? "provider.src" : reference) +
                       " { 1'b0 : ctrl; 1'b1 : " + force +
                       "; } Alias forwarded = route; " + consumer("forwarded") +
                       " }");
        const auto flat = pattern::SerializeIcl(pattern::Elaborate(mux, "M"));
        pattern::Design replay;
        replay.AddIcl("flat.icl", flat);
        Require(pattern::SerializeIcl(pattern::Elaborate(replay, "M")) == flat,
                "direct mux through aliases survives flat replay");
      }
      for (const std::string source :
           {std::string("typed"), std::string(force)}) {
        pattern::Design valid;
        valid.AddIcl("typed_driver.icl", child + "Module M { " + family +
                                             "Port typed; " + consumer(source) +
                                             " }");
        pattern::Elaborate(valid, "M");
      }
      const auto vector_child =
          output ? "" : "Module Child { " + family + "Port ctl[1:0]; } ";
      for (const std::string source : {"mux, data", "data, mux"}) {
        pattern::Design mixed;
        const auto sink =
            output ? "To" + family + "Port out[1:0] { Source wrapped; }"
                   : "Instance child Of Child { InputPort ctl = wrapped; }";
        mixed.AddIcl("mixed_driver.icl",
                     vector_child + "Module M { " + family +
                         "Port typed; DataInPort data; DataMux mux SelectedBy "
                         "data { 1'b0: typed; 1'b1: " +
                         force + "; } Alias wrapped[1:0] = " + source + "; " +
                         sink + " }");
        Fails([&] { pattern::Elaborate(mixed, "M"); },
              "requires a direct DataMux data driver");
      }
    }
  }
}

void TestTypedControlSourceInversion() {
  for (const std::string family :
       {"TCK", "TMS", "TRST", "CaptureEn", "ShiftEn", "UpdateEn"}) {
    const auto forwarded = family == "TCK"
                               ? "ToTCKPort out;"
                               : "To" + family + "Port out { Source ctl; }";
    const auto modules = "Module Driver { " + family + "Port ctl; " +
                         forwarded + " } Module Sink { " + family +
                         "Port ctl; } ";
    for (bool output : {false, true}) {
      if (output && family == "TCK") continue;
      const auto consumer = [&](const std::string& source) {
        return output ? "To" + family + "Port out { Source " + source + "; }"
                      : "Instance sink Of Sink { InputPort ctl = " + source +
                            "; }";
      };
      for (const std::string producer : {"ctl", "driver.out"}) {
        const auto prefix =
            modules + "Module M { " + family +
            "Port ctl; Instance driver Of Driver { InputPort ctl = ctl; } "
            "Alias odd = ~" +
            producer + "; Alias even = ~odd; ";
        for (const std::string source : {"~" + producer, std::string("odd")}) {
          pattern::Design invalid;
          invalid.AddIcl("inverted_control.icl",
                         prefix + consumer(source) + " }");
          Fails([&] { pattern::Elaborate(invalid, "M"); },
                "requires a non-inverted typed control source");
        }
        for (const std::string source :
             {producer, std::string("even"), std::string("~odd")}) {
          pattern::Design valid;
          valid.AddIcl("control_cancellation.icl",
                       prefix + consumer(source) + " }");
          const auto text =
              pattern::SerializeIcl(pattern::Elaborate(valid, "M"));
          pattern::Design flat;
          flat.AddIcl("flat.icl", text);
          Require(pattern::SerializeIcl(pattern::Elaborate(flat, "M")) == text,
                  "even control alias inversion retains flat identity");
        }
      }
      if (family != "TMS" && family != "TRST") {
        for (const std::string source : {"~data", "~1'b0"}) {
          pattern::Design data;
          data.AddIcl("inverted_control_data.icl",
                      modules + "Module M { DataInPort data; " +
                          consumer(source) + " }");
          const auto text =
              pattern::SerializeIcl(pattern::Elaborate(data, "M"));
          pattern::Design flat;
          flat.AddIcl("flat.icl", text);
          pattern::Elaborate(flat, "M");
        }
      }
    }
    for (const std::string source : {"ctl[1], ~ctl[0]", "~ctl[1], ctl[0]"}) {
      pattern::Design mixed;
      mixed.AddIcl("mixed_control_inversion.icl",
                   "Module Sink { " + family + "Port ctl[1:0]; } Module M { " +
                       family + "Port ctl[1:0]; Alias mixed[1:0] = " + source +
                       "; Instance sink Of Sink { InputPort ctl = mixed; } }");
      Fails([&] { pattern::Elaborate(mixed, "M"); },
            "requires a non-inverted typed control source");
    }
  }
  pattern::Design allowed;
  allowed.AddIcl(
      "other_signal_inversion.icl",
      "Module Sink { ClockPort clock; ResetPort reset { "
      "ActivePolarity 0; } ScanInPort si; TCKPort tck; ShiftEnPort se; "
      "} Module M { TCKPort clock; ResetPort reset; ScanInPort si; "
      "DataInPort mode; DataMux data SelectedBy mode { "
      "1'b0: mode; 1'b1: 1'b0; } "
      "ToTCKPort extension { Source ~clock; } "
      "Instance sink Of Sink { InputPort clock = ~clock; "
      "InputPort reset = ~reset; InputPort si = ~si; "
      "InputPort tck = ~data; InputPort se = ~data; } }");
  const auto text = pattern::SerializeIcl(pattern::Elaborate(allowed, "M"));
  pattern::Design flat;
  flat.AddIcl("flat.icl", text);
  Require(
      pattern::SerializeIcl(pattern::Elaborate(flat, "M")) == text,
      "data, clock, reset, scan and ToTCK extension inversions retain replay");
  pattern::Design inferred;
  inferred.AddIcl("inferred_trst_polarity.icl",
                  "Module Sink { TRSTPort trst; } Module M { "
                  "ToTRSTPort global; Instance sink Of Sink; }");
  const auto model = pattern::Elaborate(inferred, "M");
  const auto& bit = model.NodeAt("sink.trst").inputs.at("Source").at(0);
  Require(bit.virtual_signal == pattern::VirtualSignal::kAsyncGlobalReset &&
              bit.inverted,
          "inferred virtual TRST retains its active-low inversion");
  pattern::Design restored;
  restored.AddIcl("flat.icl", pattern::SerializeIcl(model));
  pattern::Elaborate(restored, "M");
}

void TestResetMuxStructure() {
  for (const std::string family : {"Reset", "TMS", "TRST"}) {
    for (bool output : {false, true}) {
      const auto child =
          output ? "" : "Module Child { " + family + "Port ctl; } ";
      const auto consumer = [&](const std::string& source) {
        return output ? "To" + family + "Port out { Source " + source + "; }"
                      : "Instance child Of Child { InputPort ctl = " + source +
                            "; }";
      };
      const auto prefix = child + "Module M { " + family +
                          "Port ctl; DataInPort mode[1:0]; DataInPort d0; "
                          "DataInPort d1; ";
      for (const std::string rows :
           {"2'd0: d0; 2'd1: d1;", "2'd0: 1'b0; 2'd1: 1'b1;",
            "2'd0: ctl; 2'd1: d0; 2'd2: d1;"}) {
        for (const std::string source : {"mux", "wrapped"}) {
          pattern::Design invalid;
          invalid.AddIcl("reset_mux_shape.icl",
                         prefix + "DataMux mux SelectedBy mode { " + rows +
                             " } Alias wrapped = ~mux; " + consumer(source) +
                             " }");
          Fails([&] { pattern::Elaborate(invalid, "M"); },
                "requires a corresponding control path and at most two data "
                "inputs");
        }
      }
      // A forcing value may itself be computed by an ordinary-data mux.
      pattern::Design forcing;
      forcing.AddIcl("reset_mux_data_branch.icl",
                     prefix +
                         "DataMux local SelectedBy mode[0] { "
                         "1'b0: d0; 1'b1: d1; } "
                         "DataMux unused SelectedBy mode { "
                         "2'd0: ctl; 2'd1: d0; 2'd2: d1; } "
                         "DataMux mux SelectedBy mode[1] { "
                         "1'b0: local; 1'b1: ctl; } " +
                         consumer("mux") + " }");
      const auto text = pattern::SerializeIcl(pattern::Elaborate(forcing, "M"));
      pattern::Design flat;
      flat.AddIcl("flat.icl", text);
      Require(pattern::SerializeIcl(pattern::Elaborate(flat, "M")) == text,
              "ordinary data forcing mux survives source and flat replay");
      for (const std::string rows :
           {"2'd0: ctl;", "2'd0: ctl; 2'd1: ctl;",
            "2'd0: ctl; 2'd1: same; 2'd2: d0;", "2'd0: ctl; 2'd1: ctl2;",
            "2'd0: ctl; 2'd1: 1'b1; 2'd2: ~1'b0;"}) {
        pattern::Design valid;
        valid.AddIcl("reset_mux_semantic_inputs.icl",
                     prefix + family +
                         "Port ctl2; Alias inverted = ~ctl; "
                         "Alias same = ~inverted; "
                         "DataMux mux SelectedBy mode { " +
                         rows + " } " + consumer("mux") + " }");
        const auto serialized =
            pattern::SerializeIcl(pattern::Elaborate(valid, "M"));
        pattern::Design restored;
        restored.AddIcl("flat.icl", serialized);
        Require(pattern::SerializeIcl(pattern::Elaborate(restored, "M")) ==
                    serialized,
                "semantic mux inputs retain identity and canonical duplicates");
      }
      pattern::Design nested;
      nested.AddIcl("nested_reset_mux_shape.icl",
                    prefix +
                        "DataMux inner SelectedBy mode { "
                        "2'd0: ctl; 2'd1: d0; 2'd2: d1; } "
                        "DataMux outer SelectedBy mode[0] { "
                        "1'b0: inner; 1'b1: d0; } " +
                        consumer("outer") + " }");
      Fails(
          [&] { pattern::Elaborate(nested, "M"); },
          "requires a corresponding control path and at most two data inputs");
      const auto vector_child =
          output ? "" : "Module Child { " + family + "Port ctl[1:0]; } ";
      for (const std::string first : {"ctl, d0", "d0, ctl"}) {
        pattern::Design mixed;
        const auto sink = output
                              ? "To" + family + "Port out[1:0] { Source mux; }"
                              : consumer("mux");
        mixed.AddIcl("vector_reset_mux_shape.icl",
                     vector_child + "Module M { " + family +
                         "Port ctl; DataInPort mode; DataInPort d0; "
                         "DataMux mux[1:0] SelectedBy mode { 1'b0: " +
                         first + "; 1'b1: 2'b00; } " + sink + " }");
        Fails([&] { pattern::Elaborate(mixed, "M"); },
              "requires a corresponding control path and at most two data "
              "inputs");
      }
    }
  }
  for (const size_t stages : {512, 4096}) {
    const auto start = std::chrono::steady_clock::now();
    std::string source = "Module M { ResetPort root; DataInPort mode; ";
    for (size_t i = 0; i < stages; ++i) {
      const auto name = "m" + std::to_string(i);
      const auto previous = i ? "m" + std::to_string(i - 1) : "root";
      source += "DataMux " + name + " SelectedBy mode { 1'b0: " + previous +
                "; 1'b1: 1'b1; } ToResetPort out" + std::to_string(i) +
                " { Source " + name + "; } ";
    }
    source += "}";
    pattern::Design design;
    design.AddIcl("reset_mux_chain.icl", source);
    const auto text = pattern::SerializeIcl(pattern::Elaborate(design, "M"));
    pattern::Design flat;
    flat.AddIcl("flat.icl", text);
    Require(pattern::SerializeIcl(pattern::Elaborate(flat, "M")) == text,
            "shared deep reset muxes retain source and flat identity");
    std::cout << "Reset mux shape: stages=" << stages << " ports=" << stages
              << " bytes=" << source.size() << " source/flat seconds="
              << std::chrono::duration<double>(
                     std::chrono::steady_clock::now() - start)
                     .count()
              << '\n';
  }
}

void TestCaptureUpdateMuxStructure() {
  for (const std::string family : {"CaptureEn", "UpdateEn"}) {
    const auto prefix = "Module M { " + family + "Port src; " + family +
                        "Port other; DataInPort data; DataInPort mode[1:0]; "
                        "Alias wrapped = src; ";
    const auto output = "To" + family + "Port out { Source mux; } }";
    for (const std::string rows :
         {"0 : 1'b1; 1 : src;", "0 : 1'bx; 1 : src;",
          "0 : 1'b0; 1 : src; 2 : data;", "0 : 1'b0; 1 : src; 2 : other;",
          "0 : 1'b0; 1 : src; 2 : ~wrapped;",
          "0 : 1'b0; 1 : ~wrapped; 2 : wrapped;"}) {
      pattern::Design design;
      design.AddIcl(
          "gate_shape.icl",
          prefix + "DataMux mux SelectedBy mode { " + rows + " } " + output);
      Fails([&] { pattern::Elaborate(design, "M"); },
            "requires zero and one corresponding control input");
    }
    for (const auto& [rows, inverted] :
         std::vector<std::pair<std::string, bool>>{
             {"0 : 1'b0; 1 : src;", false},
             {"0 : 1'b0; 1 | 2 : src;", false},
             {"0 : 1'b0; 1 : src; 2 : wrapped;", false},
             {"0 : src; 1 : wrapped;", false},
             {"0 : 1'b0; 1 : ~src;", true},
             {"0 : 1'b0; 1 : src; 2 : 1'b0;", false},
             {"0 : ~1'b1; 1 : double_inverse; 2 : src;", false},
             {"0 : 1'b0; 1 : ~wrapped; 2 : inverse;", true},
             {"0 : 1'b0; 1 : inverse; 2 : ~wrapped;", true},
             {"0 : 1'b0; 1 : wrapped; 2 : ~inverse;", false}}) {
      pattern::Design design;
      // Balance canonical inverse inputs for identity; suppression is separate.
      const auto consumer =
          inverted ? "To" + family + "Port out { Source ~mux; } }" : output;
      design.AddIcl("gate_shape.icl",
                    prefix +
                        "Alias inverse = ~src; Alias double_inverse = "
                        "~inverse; DataMux mux SelectedBy mode { " +
                        rows + " } " + consumer);
      const auto original = pattern::Elaborate(design, "M");
      const auto flat = pattern::SerializeIcl(original);
      pattern::Design replay;
      replay.AddIcl("flat.icl", flat);
      Require(pattern::SerializeIcl(pattern::Elaborate(replay, "M")) == flat,
              "canonical control inputs survive flat replay");
    }
    for (const std::string force : {"1'b0", "1'b1"}) {
      pattern::Design design;
      design.AddIcl("gate_chain.icl",
                    prefix +
                        "DataMux inner SelectedBy mode { 0 : 1'b0; "
                        "1 : src; } Alias forwarded = inner; "
                        "DataMux mux SelectedBy mode { 0 : " +
                        force + "; 1 : forwarded; } " + output);
      if (force == "1'b1") {
        Fails([&] { pattern::Elaborate(design, "M"); },
              "requires zero and one corresponding control input");
      } else {
        pattern::Elaborate(design, "M");
      }
    }
    pattern::Design unused;
    unused.AddIcl(
        "unused_gate.icl",
        prefix + "DataMux mux SelectedBy mode { 0 : 1'b1; 1 : src; } }");
    Fails([&] { pattern::Elaborate(unused, "M"); },
          "requires zero and one corresponding control input");
    pattern::Design partial;
    partial.AddIcl("partial_gate.icl",
                   prefix +
                       "DataMux mux[1:0] SelectedBy mode { 0 : 2'b10; "
                       "1 : data, src; } To" +
                       family + "Port out { Source mux[0]; } }");
    pattern::Elaborate(partial, "M");
    for (const std::string range : {"[1:0]", "[0:1]"}) {
      pattern::Design vector;
      vector.AddIcl("vector_gate.icl",
                    "Module M { " + family + "Port src" + range +
                        "; DataInPort mode[1:0]; DataMux mux" + range +
                        " SelectedBy mode { 0 : 2'b00; "
                        "1 : src[1], 1'b0; 2 : 1'b0, src[0]; } To" +
                        family + "Port out" + range + " { Source mux; } }");
      const auto flat = pattern::SerializeIcl(pattern::Elaborate(vector, "M"));
      pattern::Design replay;
      replay.AddIcl("vector_flat.icl", flat);
      Require(pattern::SerializeIcl(pattern::Elaborate(replay, "M")) == flat,
              "scalar gate shape permits three distinct vector rows");
    }
  }
  pattern::Design ordinary;
  ordinary.AddIcl("ordinary.icl",
                  "Module M { DataInPort mode; DataInPort data; "
                  "DataMux mux SelectedBy mode { 0 : 1'b0; 1 : data; } "
                  "ToCaptureEnPort out { Source mux; } }");
  pattern::Elaborate(ordinary, "M");
}

void TestDataMuxControlSourceUnion() {
  for (const std::string family :
       {"Reset", "CaptureEn", "UpdateEn", "TMS", "TRST"}) {
    pattern::Design design;
    design.AddIcl(
        "mux_control_union.icl",
        "Module Provider { " + family + "Port in; To" + family +
            "Port out { Source in; } } Module M { " + family +
            "Port input; DataInPort mode; Instance provider Of Provider { "
            "InputPort in = input; } Alias wrapped = provider.out; "
            "DataMux mux SelectedBy mode { 1'b0 : " +
            (family == "Reset" ? "1'b1" : "1'b0") + "; 1'b1 : wrapped; } To" +
            family + "Port out { Source mux; } }");
    const auto original = pattern::Elaborate(design, "M");
    pattern::Design flat;
    flat.AddIcl("flat.icl", pattern::SerializeIcl(original));
    Require(pattern::SerializeIcl(pattern::Elaborate(flat, "M")) ==
                pattern::SerializeIcl(original),
            "conditional control source union survives alias/flat replay");
    for (const auto reset : {pattern::VirtualSignal::kGlobalReset,
                             pattern::VirtualSignal::kAsyncGlobalReset}) {
      auto changed = original;
      changed.nodes.at(changed.node_ids.at("mux"))
          .inputs.at("SelectedBy")
          .at(0) = {std::numeric_limits<size_t>::max(), 0, 'x', false, reset};
      pattern::Design invalid;
      invalid.AddIcl("invalid_mux.icl", pattern::SerializeIcl(changed));
      Fails([&] { pattern::Elaborate(invalid, "M"); },
            "requires a data signal");
    }
  }
}

void TestIrSelectOwnerControls() {
  for (int tms = 0; tms <= 2; ++tms) {
    for (int trst = 0; trst <= 2; ++trst) {
      std::string source = "Module M { ToIRSelectPort selector; ";
      for (int i = 0; i < tms; ++i)
        source += "TMSPort t" + std::to_string(i) + "; ";
      for (int i = 0; i < trst; ++i)
        source += "TRSTPort r" + std::to_string(i) + "; ";
      source += "}";
      pattern::Design design;
      design.AddIcl("ir_owner_counts.icl", source);
      if (tms != 1 || trst > 1) {
        Fails([&] { pattern::Elaborate(design, "M"); },
              tms != 1 ? "exactly one TMSPort declaration"
                       : "at most one TRSTPort declaration");
      } else {
        const auto text =
            pattern::SerializeIcl(pattern::Elaborate(design, "M"));
        pattern::Design flat;
        flat.AddIcl("flat.icl", text);
        Require(pattern::SerializeIcl(pattern::Elaborate(flat, "M")) == text,
                "valid unused ToIRSelect owner controls replay");
      }
    }
  }
  for (const std::string range : {"[7]", "[1:0]", "[0:1]"}) {
    pattern::Design design;
    design.AddIcl(
        "ir_owner_vectors.icl",
        "Module M { TMSPort tms" + range + "; TRSTPort trst" + range +
            "; ToIRSelectPort selector[1:0]; ToIRSelectPort another; }");
    const auto text = pattern::SerializeIcl(pattern::Elaborate(design, "M"));
    pattern::Design flat;
    flat.AddIcl("flat.icl", text);
    Require(pattern::SerializeIcl(pattern::Elaborate(flat, "M")) == text,
            "owner controls count definitions rather than vector bits or "
            "selectors");
  }
  for (const std::string source :
       {"Module Owner { ToIRSelectPort ir; } Module M { TMSPort tms; Instance "
        "owner Of Owner; }",
        "Module Controls { TMSPort tms; } Module M { ToIRSelectPort ir; "
        "Instance controls Of Controls { InputPort tms = 1'b0; } }",
        "Module Controls { TMSPort tms; } Module Owner { ToIRSelectPort ir; } "
        "Module M { Instance controls Of Controls { InputPort tms = 1'b0; } "
        "Instance owner Of Owner; "
        "}"}) {
    pattern::Design design;
    design.AddIcl("ir_owner_scope.icl", source);
    Fails([&] { pattern::Elaborate(design, "M"); },
          "exactly one TMSPort declaration");
  }
  pattern::Design owners;
  owners.AddIcl("ir_owner_restored.icl",
                "Module Owner { TMSPort tms; TRSTPort trst; DataInPort extra; "
                "ToIRSelectPort ir; } "
                "Module M { Instance first Of Owner { InputPort tms = 1'b0; } "
                "Instance second Of Owner { InputPort tms = 1'b0; } }");
  const auto original = pattern::Elaborate(owners, "M");
  const auto text = pattern::SerializeIcl(original);
  pattern::Design flat;
  flat.AddIcl("flat.icl", text);
  Require(pattern::SerializeIcl(pattern::Elaborate(flat, "M")) == text,
          "sibling owners retain independent declaration counts after flat "
          "restoration");
  for (const std::string name : {"first.tms", "first.extra"}) {
    auto altered = original;
    auto& node = altered.nodes.at(altered.node_ids.at(name));
    node.kind = name == "first.tms" ? "DataInPort" : "TRSTPort";
    node.active_low = name != "first.tms";
    pattern::Design changed;
    changed.AddIcl("altered_owner.icl", pattern::SerializeIcl(altered));
    Fails([&] { pattern::Elaborate(changed, "M"); },
          name == "first.tms" ? "exactly one TMSPort declaration"
                              : "at most one TRSTPort declaration");
  }
  pattern::Design unrelated;
  unrelated.AddIcl(
      "no_ir_owner.icl",
      "Module M { TMSPort a; TMSPort b; TRSTPort c; TRSTPort d; }");
  pattern::Elaborate(unrelated, "M");
}

void TestDataSignalAliasSlices() {
  const std::string body =
      "Module M { DataInPort data; TCKPort clock;"
      "Alias mixed[0:1] = data, clock; Alias wrapped[1:0] = ~mixed[1:0];";
  for (const std::string consumer :
       {"DataOutPort result { Source ",
        "DataMux result SelectedBy 1'b0 { 1'b0 : "}) {
    pattern::Design valid;
    valid.AddIcl("alias_data.icl", body + consumer + "wrapped[0]; }}");
    const auto original = pattern::Elaborate(valid, "M");
    pattern::Design flat;
    flat.AddIcl("flat.icl", pattern::SerializeIcl(original));
    pattern::Elaborate(flat, "M");
    pattern::Design invalid;
    invalid.AddIcl("alias_clock.icl", body + consumer + "wrapped[1]; }}");
    Fails([&] { pattern::Elaborate(invalid, "M"); }, "data signal");
  }

  for (const std::string consumer :
       {"ScanMux route SelectedBy selected { 1'b0: si; 1'b1: si; }",
        "DataOutPort result { Source selected; }",
        "ScanMux route SelectedBy selected { 1'b0: si; }"}) {
    pattern::Design design;
    design.AddIcl("ir_select.icl",
                  "Module M { ScanInPort si; TMSPort tms; ToIRSelectPort ir;"
                  "Alias selected = ir;" +
                      consumer + "}");
    if (consumer.find("1'b1") != std::string::npos) {
      pattern::Elaborate(design, "M");
    } else {
      Fails([&] { pattern::Elaborate(design, "M"); }, "data signal");
    }
  }
}

void TestDataSourceScale() {
  for (bool clock : {false, true}) {
    for (bool wide : {false, true}) {
      std::string source = wide
                               ? "Module M { DataInPort data[65535:0];"
                                 "Alias packed[65535:0] = data;"
                               : "Module M { DataInPort data; Alias a0 = data;";
      for (size_t i = wide ? 0 : 1; i < 4096; ++i) {
        source += wide ? "DataOutPort out" + std::to_string(i) +
                             " { Source packed[" + std::to_string(i % 256) +
                             "]; }"
                       : "Alias a" + std::to_string(i) + " = a" +
                             std::to_string(i - 1) + ";";
      }
      if (!wide) source += "DataOutPort result { Source a4095; }";
      source += "}";
      if (clock) {
        for (const auto& replacement :
             {std::make_pair(std::string("DataInPort"),
                             std::string("ClockPort")),
              std::make_pair(std::string("DataOutPort"),
                             std::string("ToClockPort"))}) {
          size_t at = 0;
          while ((at = source.find(replacement.first, at)) !=
                 std::string::npos) {
            source.replace(at, replacement.first.size(), replacement.second);
            at += replacement.second.size();
          }
        }
      }
      const auto start = std::chrono::steady_clock::now();
      pattern::Design design;
      design.AddIcl("data_source_scale.icl", source);
      const auto netlist = pattern::Elaborate(design, "M");
      Require(netlist.nodes.size() == 4098,
              "all long-chain or repeated-slice data consumers are retained");
      std::cout << (clock ? "Clock source aliases: " : "Data source aliases: ")
                << (wide ? "width=65536 consumers=4096 distinct_bits=256"
                         : "aliases=4096 consumers=1")
                << " bytes=" << source.size() << " seconds="
                << std::chrono::duration<double>(
                       std::chrono::steady_clock::now() - start)
                       .count()
                << '\n';
    }
  }
}

void TestAddressedDataDefaults() {
  pattern::Design design;
  for (const auto* file :
       {"tests/cases/tessent/pattern_generation/address_defaults.icl",
        "tests/cases/ours/pattern_generation/address_partial_data.icl",
        "tests/cases/ours/pattern_generation/address_data_hierarchy.icl"}) {
    std::ifstream input(file);
    std::string source;
    Require(input.good() && toolsets::ReadTextStream(input, &source),
            "read addressed data fixture");
    design.AddIcl(file, source);
  }
  auto netlist = pattern::Elaborate(design, "AddressDefaultsTop");
  const auto& data = netlist.NodeAt("child.data");
  Require(data.inputs.count("Source"), "addressed data defaults are connected");
  const auto& bits = data.inputs.at("Source");
  Require(bits.size() == 8 &&
              bits.front().node == netlist.node_ids.at("data") &&
              bits.front().index == 15 && bits.back().index == 8,
          "narrow child consumes leading parent data bits (Tessent 2026.2)");

  netlist = pattern::Elaborate(design, "SplitDefaultTop");
  const auto& upper = netlist.NodeAt("child.upper").inputs.at("Source");
  const auto& lower = netlist.NodeAt("child.lower").inputs.at("Source");
  Require(upper[0].node == netlist.node_ids.at("zfirst") &&
              upper[0].index == 1 && upper[1].index == 0 &&
              upper[2].node == netlist.node_ids.at("asecond") &&
              upper[2].index == 0 && upper[3].index == 1 &&
              lower[0].node == netlist.node_ids.at("asecond") &&
              lower[0].index == 2 && lower[3].index == 5,
          "logical data buses follow declaration order and vector direction");
  netlist = pattern::Elaborate(design, "ExplicitDataTop");
  Require(netlist.NodeAt("child.upper").inputs.at("Source")[0].node ==
                  netlist.node_ids.at("override_data") &&
              !netlist.NodeAt("child.lower").inputs.count("Source"),
          "any explicit data binding suppresses all implicit data binding");
  netlist = pattern::Elaborate(design, "PartialDataTop");
  const auto& partial = netlist.NodeAt("child.data").inputs.at("Source");
  Require(partial[0].node == netlist.node_ids.at("override_data") &&
              partial[4].constant == '?' && partial[7].constant == '?',
          "partial explicit input leaves holes disconnected");
  Fails([&] { pattern::Elaborate(design, "ShortDataTop"); },
        "logical DataIn width");
  Fails([&] { pattern::Elaborate(design, "MissingDataTop"); },
        "child.data: parent logical DataIn width");
  netlist = pattern::Elaborate(design, "NestedDataTop");
  Require(netlist.NodeAt("unaddressed.data").inputs.empty(),
          "ordinary instances do not acquire addressed data defaults");
  for (const std::string instance : {"a", "b", "c"}) {
    const auto& middle = netlist.NodeAt(instance + ".data").inputs.at("Source");
    const auto& leaf =
        netlist.NodeAt(instance + ".leaf.data").inputs.at("Source");
    Require(middle[0].node == netlist.node_ids.at("data") &&
                middle[0].index == 7 && middle[7].index == 0 &&
                leaf[0].node == netlist.node_ids.at(instance + ".data") &&
                leaf[0].index == 0 && leaf[7].index == 7,
            "nested and sibling data defaults retain immediate parent mapping");
  }
  Fails([&] { pattern::Elaborate(design, "DataBudgetTop"); },
        "implicit data connection bit limit");
}

void TestAccessLinkModelInsertion() {
  pattern::Design design, models;
  const std::string source =
      "NameSpace lib; Module Child { Parameter W = 1; DataInPort mode[$W-1:0]; "
      "}"
      "Module Top { Parameter W = 2; Enum Names { tap = 1'b0; }"
      "Instance core Of Child { Parameter W = $W;"
      "InputPort mode = tap.active; } AccessLink tap Of STD_1149_1_2001 {"
      "BSDLEntity Probe; ON { ScanInterface { core.client; } } } }";
  design.AddIcl("original.icl", source);
  design.AddIcl(
      "later.icl",
      "NameSpace lib; Module Child { Parameter W = 1;"
      "DataInPort mode[$W-1:0]; DataOutPort newer { Source mode[0]; } }");
  models.AddIcl(
      "model.icl",
      "Module BehavioralTap { DataOutPort active[1:0] { Source 2'b10; } }");
  const auto root = design.modules().at("lib::Top").document;
  const auto netlist = pattern::Elaborate(
      design, "lib::Top", {{"tap", models.modules().at("BehavioralTap")}});
  const auto& input = netlist.NodeAt("core.mode");
  Require(input.Width() == 2 && input.inputs.at("Source")[0].node ==
                                    netlist.node_ids.at("tap.active"),
          "model ports resolve in original instance bindings");
  Require(netlist.NodeAt("core.newer").origin.find("later.icl:") == 0 &&
              netlist.NodeAt("tap.active").origin.find("model.icl:") == 0,
          "ordered module replacements and source locations remain intact");
  Require(
      !netlist.node_ids.count("tap") &&
          netlist.scopes[0].definition.document == root &&
          root->source() == source && design.modules().size() == 2 &&
          design.warnings().size() == 1,
      "replace only the AccessLink node, without changing source or registry");
  Require(netlist.module_parameters.count("BehavioralTap"),
          "inserted module identity is available to flat replay");
  Fails([&] { pattern::Elaborate(design, "lib::Top"); },
        "unknown signal tap.active");
  Fails(
      [&] {
        pattern::Elaborate(design, "lib::Top",
                           {{"missing", models.modules().at("BehavioralTap")}});
      },
      "unknown top-level AccessLink missing");
}

void TestAccessLinkModelErrors() {
  pattern::Design design, models;
  design.AddIcl("top.icl",
                "Module Top { AccessLink tap Of STD_1149_1_2001 {"
                "BSDLEntity Probe; ON { ScanInterface { child.iface; } } } }");
  models.AddIcl("model.icl",
                "Module Leaf {} Module Nested { Instance child Of Leaf; }"
                "Module Linked { AccessLink inner Of Generic { opaque; } }"
                "Module Top { DataInPort collision; }");
  const auto reject = [&](const pattern::ModuleDefinition& model,
                          const std::string& message) {
    Fails([&] { pattern::Elaborate(design, "Top", {{"tap", model}}); },
          message);
  };
  reject({}, "invalid behavioral module");
  reject(models.modules().at("Nested"), "must be a leaf module");
  reject(models.modules().at("Linked"), "cannot contain AccessLink");
  reject(models.modules().at("Top"), "conflicting behavioral module identity");
  auto malformed = models.modules().at("Leaf");
  malformed.declaration_index = 100;
  reject(malformed, "invalid behavioral module");
  pattern::Design duplicate;
  duplicate.AddIcl("duplicate.icl",
                   "Module Top { DataInPort tap; AccessLink tap Of"
                   " STD_1149_1_2001 { BSDLEntity Probe; ON { ScanInterface { "
                   "child.iface; } } } }");
  Fails(
      [&] {
        pattern::Elaborate(duplicate, "Top",
                           {{"tap", models.modules().at("Leaf")}});
      },
      "duplicate top-level declaration tap");

  pattern::Design grouped;
  grouped.AddIcl("grouped.icl",
                 "Module Child {} Module Top { OneHotDataGroup g {"
                 "Instance tap Of Child; } AccessLink tap Of STD_1149_1_2001 {"
                 "BSDLEntity Probe; ON { ScanInterface { child.iface; } } } }");
  Fails(
      [&] {
        pattern::Elaborate(grouped, "Top",
                           {{"tap", models.modules().at("Leaf")}});
      },
      "duplicate top-level declaration tap");

  pattern::Design metadata;
  metadata.AddIcl("flat.icl",
                  "Module Flat { Attribute icl_pattern_origins_v1 = \"{}\"; }");
  reject(metadata.modules().at("Flat"),
         "flat origin metadata requires a root module");
}

void TestSharedAccessLinkModels() {
  pattern::Design design, models;
  models.AddIcl("shared.icl",
                "Module Shared { Parameter W = 2;"
                "DataOutPort active[$W-1:0] { Source 2'b10; } }");
  const auto& model = models.modules().at("Shared");
  constexpr size_t kCount = 3000;
  std::map<std::string, pattern::ModuleDefinition> replacements;
  std::string source = "Module Top {";
  for (size_t i = 0; i < kCount; ++i) {
    const auto name = "tap" + std::to_string(i);
    source += "AccessLink " + name +
              " Of STD_1149_1_2001 { BSDLEntity Probe;"
              " ON { ScanInterface { child.client; } } }";
    replacements.emplace(name, model);
  }
  source += "}";
  design.AddIcl("many.icl", source);
  const auto start = std::chrono::steady_clock::now();
  const auto netlist = pattern::Elaborate(design, "Top", replacements);
  const std::chrono::duration<double> elapsed =
      std::chrono::steady_clock::now() - start;
  Require(netlist.scopes.size() == kCount + 1 &&
              netlist.nodes.size() == kCount &&
              netlist.NodeAt("tap0.active").ast ==
                  netlist.NodeAt("tap2999.active").ast &&
              netlist.module_parameters.at("Shared").count("W"),
          "many inserted instances share one model AST and template");
  std::cout << "AccessLink model insertion: " << kCount
            << " instances, seconds=" << elapsed.count() << '\n';
  pattern::Design other;
  other.AddIcl("other.icl", "Module Shared { DataInPort other; }");
  replacements.at("tap1") = other.modules().at("Shared");
  Fails([&] { pattern::Elaborate(design, "Top", replacements); },
        "conflicting behavioral module identity Shared");
}

void TestGeneratedTapPorts() {
  pattern::Design design;
  design.AddIcl("pins.icl",
                "Module Child { TMSPort mode; } Module Top {"
                "Instance child Of Child; ScanOutPort monitor { Source TDI; } "
                "Parameter reserved = 0; }");
  const auto netlist = pattern::Elaborate(
      design, "Top", {}, {{"TDI", "ScanInPort"}, {"TMS", "TMSPort"}});
  Require(
      netlist.NodeAt("monitor").inputs.at("Source")[0].node ==
              netlist.node_ids.at("TDI") &&
          netlist.NodeAt("child.mode").inputs.at("Source")[0].node ==
              netlist.node_ids.at("TMS"),
      "synthesized pins precede explicit connections and reset/TMS inference");
  for (const auto& port :
       std::map<std::string, std::string>{{"reserved", "ScanInPort"},
                                          {"child", "TCKPort"},
                                          {"unsupported", "DataRegister"}}) {
    Fails([&] { pattern::Elaborate(design, "Top", {}, {port}); },
          "invalid generated TAP port " + port.first);
  }
}

void TestAddressedGroupErrors() {
  const std::vector<std::pair<std::string, std::string>> cases = {
      {"OneHotDataGroup g[7:0] { DataRegister r[15:0] { AddressValue 1; } }",
       "source width exceeds group"},
      {"OneHotDataGroup g[7:0] { Instance child Of Leaf { AddressValue 0; } }",
       "source width exceeds group"},
      {"DataRegister r { AddressValue 1; }", "belong"},
      {"OneHotDataGroup g { DataRegister r { AddressValue 1; WriteEnSource "
       "1'b1; } }",
       "access types"},
      {"OneHotDataGroup g[15:0] { DataRegister r[15:0] { AddressValue 1; } }",
       "register exceeds module logical DataIn width"},
      {"Parameter A = \"address\"; OneHotDataGroup g { DataRegister r { "
       "AddressValue $A; } }",
       "AddressValue must be numeric"},
      {"OneHotDataGroup g { DataRegister r; }", "requires AddressValue"}};
  for (const auto& item : cases) {
    pattern::Design design;
    Fails(
        [&] {
          design.AddIcl("addressed_invalid.icl",
                        "Module Leaf { DataOutPort hi[7:0] { Source 8'b0; } "
                        "DataOutPort lo { Source 1'b0; } } "
                        "Module M { DataInPort d[7:0]; " +
                            item.first + " }");
          pattern::Elaborate(design, "M");
        },
        item.second);
  }
  std::string source =
      "Module M { DataInPort d[65535:0]; OneHotDataGroup g[65535:0] { ";
  for (size_t i = 0; i < 65; ++i) {
    source += "DataRegister r" + std::to_string(i) +
              "[65535:0] { AddressValue " + std::to_string(i) + "; } ";
  }
  source += "} }";
  pattern::Design large;
  large.AddIcl("addressed_limit.icl", source);
  Fails([&] { pattern::Elaborate(large, "M"); }, "source bit limit");
  for (bool instances : {false, true}) {
    pattern::Design addresses;
    source = "Module Leaf {} Module M { DataInPort d; OneHotDataGroup g { ";
    for (size_t i = 0; i < 5; ++i) {
      source += instances && i < 3
                    ? "Instance c" + std::to_string(i) + " Of Leaf"
                    : "DataRegister r" + std::to_string(i);
      source += " { AddressValue 1048576'b0; } ";
    }
    source += "} }";
    addresses.AddIcl("address_constants.icl", source);
    Fails([&] { pattern::Elaborate(addresses, "M"); },
          "address constant bit limit");
  }
}

void TestSpecialization() {
  pattern::Design design;
  design.AddIcl("parameters.icl", R"(
Module Leaf {
  Parameter W = 8;
  LocalParameter HI = $W - 1;
  Parameter label = "instrument";
  ScanInPort si;
  ScanRegister r[$HI:0] { ScanInSource si; ResetValue 0; }
  DataInPort ascending[0:$HI];
  ScanOutPort so { Source r[0]; }
}
Module Top {
  Parameter SIZE = 4;
  ScanInPort si;
  ScanOutPort so { Source b.so; }
  Instance a Of Leaf { Parameter W = $SIZE; InputPort si = si; }
  Instance b Of Leaf { Parameter W = 2 * $SIZE; InputPort si = a.so; }
}
)");
  const auto netlist = pattern::Elaborate(design, "Top");
  const auto& a = netlist.NodeAt("a.r");
  const auto& b = netlist.NodeAt("b.r");
  Require(a.Width() == 4 && b.Width() == 8, "per-instance widths");
  Require(a.left == 3 && a.right == 0, "descending bit order");
  Require(netlist.NodeAt("a.ascending").left == 0 &&
              netlist.NodeAt("a.ascending").right == 3,
          "ascending bit order");
  Require(
      netlist.scopes.at(a.scope).parameters.at("label").data == "instrument",
      "string parameters retain type and value");
  Require(a.origin.find("parameters.icl:") == 0 && a.ast != nullptr,
          "elaborated nodes retain source and shared AST");
  Require(netlist.NodeAt("b.si").inputs.at("Source")[0].node ==
              netlist.node_ids.at("a.so"),
          "cross-instance source resolved");
  Require(netlist.NodeAt("a.r").inputs.at("ScanInSource")[0].node ==
              netlist.node_ids.at("a.si"),
          "local scan source resolved");
}

void TestErrors() {
  pattern::Design unknown;
  unknown.AddIcl("unknown.icl", "Module M { DataInPort x[$missing:0]; }");
  Fails([&] { pattern::Elaborate(unknown, "M"); }, "unknown.icl:");
  pattern::Design local;
  local.AddIcl("local.icl",
               "Module C { LocalParameter W = 1; } Module M { Instance c Of C "
               "{ Parameter W = 2; } }");
  Fails([&] { pattern::Elaborate(local, "M"); }, "LocalParameter");
  pattern::Design missing;
  missing.AddIcl("missing.icl",
                 "Module M { ScanOutPort so { Source missing; } }");
  Fails([&] { pattern::Elaborate(missing, "M"); }, "missing");
  pattern::Design width;
  width.AddIcl(
      "width.icl",
      "Module M { DataInPort a[3:0]; DataOutPort b[1:0] { Source a; } }");
  Fails([&] { pattern::Elaborate(width, "M"); }, "width");
}

void TestBitConnections() {
  pattern::Design design;
  design.AddIcl("bits.icl", R"(
Module Child {
  DataInPort d[3:0];
  DataOutPort q[3:0] { Source d; }
}
Module Top {
  Parameter LO = 0;
  DataInPort value[3:0];
  Instance c Of Child {
    InputPort d[3:2] = ~value[$LO:1];
    InputPort d[1:0] = 2'b01;
  }
  Alias reversed[0:3] = c.q[0:3];
}
)");
  const auto netlist = pattern::Elaborate(design, "Top");
  const auto& source = netlist.NodeAt("c.d").inputs.at("Source");
  Require(source.size() == 4 && source[0].index == 0 && source[1].index == 1 &&
              source[0].inverted && source[2].constant == '0' &&
              source[3].constant == '1',
          "partial inverted/reversed/constant port connections");
  const auto& alias = netlist.NodeAt("reversed").inputs.at("Source");
  Require(alias.size() == 4 && alias[0].index == 0 && alias[3].index == 3,
          "alias retains explicit ordering");
}

void TestResetInference() {
  pattern::Design design;
  design.AddIcl("reset.icl", R"(
Module Child {
  ResetPort rst { ActivePolarity 0; }
  ScanInPort si;
  ScanRegister r { ScanInSource si; ResetValue 0; }
}

Module Top {
  ResetPort rst;
  ScanInPort si;
  Instance c Of Child { InputPort si = si; }
}
)");
  const auto netlist = pattern::Elaborate(design, "Top");
  const auto& rst = netlist.NodeAt("c.rst").inputs.at("Source").at(0);
  Require(rst.node == netlist.node_ids.at("rst") && rst.inverted,
          "inferred reset connection adjusts active polarity");
  const auto& reg = netlist.NodeAt("c.r");
  Require(reg.reset.has_value() &&
              reg.reset->node == netlist.node_ids.at("c.rst") &&
              reg.reset->inverted,
          "register reset expression is normalized active-high");
  Require(reg.constants.at("ResetValue").data == "0",
          "register reset value elaborated");

  pattern::Design implicit;
  implicit.AddIcl("implicit.icl",
                  "Module M { ScanInPort si; ScanRegister r { ScanInSource si; "
                  "ResetValue 0; } }");
  const auto inferred = pattern::Elaborate(implicit, "M");
  Require(inferred.NodeAt("r").reset->virtual_signal ==
              pattern::VirtualSignal::kGlobalReset,
          "unconnected reset falls back to virtual global reset");

  pattern::Design ambiguous;
  ambiguous.AddIcl("ambiguous.icl",
                   "Module M { ResetPort a; ResetPort b; ScanInPort si; "
                   "ScanRegister r { ScanInSource si; ResetValue 0; } }");
  Fails([&] { pattern::Elaborate(ambiguous, "M"); },
        "ambiguous register reset");
}
void TestMuxExpansionBudget() {
  for (const std::string kind : {"DataMux", "ScanMux", "ClockMux"}) {
    std::string source =
        "Module M { DataInPort data[65535:0]; "
        "DataInPort mode[6:0]; " +
        kind + " mux[65535:0] SelectedBy mode { ";
    for (size_t i = 0; i < 65; ++i) {
      source += std::to_string(i) + ": data; ";
    }
    pattern::Design design;
    design.AddIcl("mux_sources.icl", source + "} }");
    Fails([&] { pattern::Elaborate(design, "M"); },
          "mux expansion bit limit exceeded");
  }
  for (const std::string body :
       {"DataInPort mode[1048575:0]; DataMux mux SelectedBy mode { "
        "0 | 1 | 2 : 0; }",
        "DataInPort mode[1048574:0]; DataMux mux[4:0] SelectedBy mode { "
        "0 | 1 | 2 : 0; }",
        "Instance a Of Child; Instance b Of Child;",
        "DataInPort mode[1048574:0]; DataMux mux[3:0] SelectedBy mode { "
        "0 | 1 | 2 : 0; } DataMux extra SelectedBy 1'b0 { 0 : 0; }"}) {
    pattern::Design design;
    design.AddIcl("mux_budget.icl",
                  "Module Child { DataInPort mode[524287:0]; "
                  "DataMux mux SelectedBy mode { 0 | 1 | 2 : 0; } } "
                  "Module M { " +
                      body + " }");
    Fails([&] { pattern::Elaborate(design, "M"); },
          "mux expansion bit limit exceeded");
  }
  pattern::Design boundary;
  boundary.AddIcl("mux_boundary.icl",
                  "Module M { DataInPort mode[1048574:0]; "
                  "DataMux mux[3:0] SelectedBy mode { 0 | 1 | 2 : 0; } }");
  const auto netlist = pattern::Elaborate(boundary, "M");
  const auto flat = pattern::SerializeIcl(netlist);
  pattern::Design replay;
  replay.AddIcl("mux_boundary_flat.icl", flat);
  Require(pattern::SerializeIcl(pattern::Elaborate(replay, "M")) == flat,
          "exact aggregate mux budget survives original/flat replay");
}

void TestMuxNumericCodes() {
  for (const std::string kind : {"DataMux", "ScanMux", "ClockMux"}) {
    for (const std::string value : {"\"0\"", "1'b0"}) {
      pattern::Design design;
      design.AddIcl("mux_codes.icl",
                    "Module M { Parameter CODE = " + value +
                        "; DataInPort mode; " + kind +
                        " mux SelectedBy mode { $CODE : 0; } }");
      if (value.front() == '"') {
        Fails([&] { pattern::Elaborate(design, "M"); },
              "mux selector code must be numeric");
      } else {
        Require(pattern::Elaborate(design, "M")
                        .NodeAt("mux")
                        .choices[0]
                        .codes[0]
                        .data == "0",
                "numeric parameter selector code remains supported");
      }
    }
  }
}

void TestMuxSelectorGraph() {
  const std::vector<std::pair<std::string, bool>> cases = {
      {"DataInPort mode[1:0]; DataMux mux[1:0] SelectedBy mode { "
       "2'bxx : 1'b0, mux[1]; }",
       false},
      {"DataInPort mode[1:0]; DataMux mux[1:0] SelectedBy mode { "
       "2'bxx : mux[0], mux[1]; }",
       true},
      {"DataMux mux[1:0] SelectedBy mux { 2'bxx : 0; }", true},
      {"Alias mode[1:0] = mux; DataMux mux[1:0] SelectedBy mode { "
       "2'bxx : 0; }",
       true},
      {"ScanInPort si; ScanRegister r[1:0] { ScanInSource si; "
       "CaptureSource mux; } DataMux mux[1:0] SelectedBy r { "
       "2'bxx : r; }",
       false}};
  for (const auto& [body, cycle] : cases) {
    pattern::Design design;
    design.AddIcl("mux_graph.icl", "Module M { " + body + " }");
    if (cycle) {
      Fails([&] { pattern::Elaborate(design, "M"); },
            "combinational connection cycle");
    } else {
      const auto original = pattern::Elaborate(design, "M");
      pattern::Design replay;
      replay.AddIcl("flat.icl", pattern::SerializeIcl(original));
      Require(pattern::SerializeIcl(pattern::Elaborate(replay, "M")) ==
                  pattern::SerializeIcl(original),
              "shared selector graph preserves bit DAGs and register cuts");
    }
  }
  pattern::Design design;
  design.AddIcl("mux_enable.icl", R"(
Module Leaf {
  ScanInPort si[1:0]; DataInPort en;
  ScanOutPort so[1:0] { Source si; Enable en; }
}
Module M {
  ScanInPort si[1:0]; DataInPort mode[1:0]; DataInPort en;
  Instance data Of Leaf { InputPort si = si; InputPort en = en; }
  Instance selector Of Leaf { InputPort si = si; InputPort en = en; }
  ScanMux mux[1:0] SelectedBy mode { 2'bxx : data.so; }
  OneHotScanGroup group[1:0] { Port mux; }
}
)");
  auto graph = pattern::Elaborate(design, "M");
  // Isolate enabled-path validation on a lowered graph with a scan selector.
  graph.nodes.at(graph.node_ids.at("data.so")).inputs.erase("Enable");
  auto& selector =
      graph.nodes.at(graph.node_ids.at("mux")).inputs.at("SelectedBy");
  for (auto& bit : selector) bit.node = graph.node_ids.at("selector.so");
  Fails([&] { pattern::CheckCombinationalConnections(graph); },
        "group source needs an enabled ScanOutPort");
}

void TestMuxAndEnum() {
  pattern::Design design;
  design.AddIcl("mux.icl", R"(
Module Top {
  ScanInPort si;
  Enum states { closed = 1'b0; open = 1'b1; }
  ScanRegister sib { ScanInSource si; RefEnum states; ResetValue closed; }
  ScanRegister data[3:0] { ScanInSource sib; ResetValue 0; }
  ScanMux route SelectedBy sib { 1'b0: sib; 1'b1: data[0]; }
  ScanOutPort so { Source route; }
}
)");
  const auto netlist = pattern::Elaborate(design, "Top");
  const auto& mux = netlist.NodeAt("route");
  Require(mux.model == pattern::ModelKind::kMux && mux.choices.size() == 2,
          "typed mux with both arms");
  Require(mux.inputs.at("SelectedBy")[0].node == netlist.node_ids.at("sib"),
          "mux selector connectivity");
  Require(mux.choices[1].codes[0].data == "1" &&
              mux.choices[1].source[0].node == netlist.node_ids.at("data") &&
              mux.choices[1].source[0].index == 0,
          "selected scan path");
  Require(netlist.NodeAt("sib").constants.at("ResetValue").data == "0",
          "enum reset resolves in instance scope");
}
void TestConnectionErrors() {
  const std::vector<std::pair<std::string, std::string>> cases = {
      {"Module M { Alias a = b; Alias b = a; }", "cycle"},
      {"Module C { DataOutPort o { Source 1'b0; } } Module M { Instance c Of C "
       "{ InputPort o = 1'b1; } }",
       "input port"},
      {"Module C { DataInPort d[1:0]; } Module M { Instance c Of C { InputPort "
       "d[1] = 1'b0; InputPort d[1:0] = 2'b00; } }",
       "duplicate"},
      {"Module C { TMSPort tms; } Module M { Instance c Of C; }",
       "missing source"},
      {"Module M { DataInPort d[3:0]; Alias a[1:0] = d[4:3]; }",
       "out of range"},
  };
  for (const auto& test : cases) {
    pattern::Design design;
    design.AddIcl("invalid.icl", test.first);
    Fails([&] { pattern::Elaborate(design, "M"); }, test.second);
  }
}

void TestUnsizedSources() {
  pattern::Design design;
  design.AddIcl("unsized.icl",
                "Module M { DataOutPort one { Source 'h01; } DataOutPort "
                "unknown { Source 'hx; } }");
  const auto netlist = pattern::Elaborate(design, "M");
  Require(netlist.NodeAt("one").inputs.at("Source")[0].constant == '1' &&
              netlist.NodeAt("unknown").inputs.at("Source")[0].constant == 'x',
          "context sizing may discard leading zero or unknown digits");
  const auto expression = netlist.NodeAt("one").properties.at("Source");
  Require(netlist.Resolve(expression, 4, pattern::ReferenceUse::kConnection, 4)
                  .size() == 4,
          "unsized padding respects the exact operand budget");
  Fails(
      [&] {
        netlist.Resolve(expression, 4, pattern::ReferenceUse::kConnection, 3);
      },
      "signal operand bit limit exceeded");
}

void TestConnectionHierarchy() {
  std::ifstream input(
      "tests/cases/ours/pattern_generation/connection_hierarchy.icl");
  std::string source;
  Require(input.good() && toolsets::ReadTextStream(input, &source),
          "read connection hierarchy fixture");
  pattern::Design valid;
  valid.AddIcl("hierarchy.icl", source);
  const auto netlist = pattern::Elaborate(valid, "ConnectionHierarchy");
  Require(netlist.NodeAt("middle.si").inputs.at("Source")[0].node ==
                  netlist.node_ids.at("child.so") &&
              netlist.NodeAt("child.rst").inputs.at("Source")[0].node ==
                  netlist.node_ids.at("middle.reset_out"),
          "sibling output bindings use their parent's scope");
  const auto& reversed = netlist.NodeAt("reversed").inputs.at("Source");
  Require(reversed.size() == 2 && reversed[0].inverted &&
              reversed[1].inverted && reversed[0].index == 1 &&
              reversed[1].index == 0 &&
              reversed[0].node == netlist.node_ids.at("child.q"),
          "child output slices retain bit order and inversion");
  Require(netlist.NodeAt("deep").inputs.at("Source")[0].node ==
                  netlist.node_ids.at("middle.leaf.value") &&
              netlist.NodeAt("deep_port").inputs.at("Source")[0].node ==
                  netlist.node_ids.at("middle.leaf.q"),
          "alias definitions retain deep register and port references");
  for (const std::string extra :
       {"DataOutPort bad[3:0] { Source child.value; }",
        "DataOutPort bad { Source child.flag; }",
        "DataOutPort bad[3:0] { Source child.local; }",
        "DataOutPort bad[3:0] { Source child.data; }",
        "LogicSignal bad { child.value[0] == 1'b0; }",
        "DataMux bad SelectedBy child.value[0] { 1'b0: 0; 1'b1: 1; }",
        "DataMux bad[3:0] SelectedBy 1'b0 { 1'b0: child.value; 1'b1: 0; }",
        "ScanRegister bad { ScanInSource child.value; }",
        "Instance sink Of ConnectionHierarchyLeaf { InputPort si = child.so;"
        "InputPort data = child.value; InputPort rst = rst; }"}) {
    auto invalid_source = source;
    invalid_source.insert(invalid_source.rfind('}'), extra);
    pattern::Design invalid;
    invalid.AddIcl("invalid_hierarchy.icl", invalid_source);
    Fails([&] { pattern::Elaborate(invalid, "ConnectionHierarchy"); },
          "must name an output port");
  }
}
void TestAliasInstanceBoundaries() {
  std::ifstream input(
      "tests/cases/ours/pattern_generation/connection_hierarchy.icl");
  std::string source;
  Require(input.good() && toolsets::ReadTextStream(input, &source),
          "read alias boundary fixture");
  for (const std::string target :
       {"child.value", "child.data", "child.local"}) {
    for (const std::string consumer :
         {"DataOutPort bad[3:0] { Source a; }",
          "Alias b[3:0] = a; Alias c[3:0] = ~b[0:3]; "
          "DataOutPort bad[3:0] { Source c; }",
          "LogicSignal bad { a[0] == 1'b0; }",
          "DataMux bad SelectedBy a[0] { 1'b0: 0; 1'b1: 1; }",
          "DataMux bad[3:0] SelectedBy 1'b0 { 1'b0: a; 1'b1: 0; }",
          "ScanRegister bad[3:0] { ScanInSource si; CaptureSource a; }",
          "Instance sink Of ConnectionHierarchyLeaf { InputPort si = middle.so;"
          "InputPort data = a; InputPort rst = rst; }"}) {
      for (bool forward : {false, true}) {
        const auto alias = "Alias a[3:0] = " + target + ";";
        auto invalid_source = source;
        invalid_source.insert(invalid_source.rfind('}'),
                              forward ? consumer + alias : alias + consumer);
        pattern::Design invalid;
        invalid.AddIcl("alias_boundary.icl", invalid_source);
        Fails([&] { pattern::Elaborate(invalid, "ConnectionHierarchy"); },
              "only available to aliases or PDL");
      }
    }
  }
  auto mixed = source;
  mixed.insert(mixed.rfind('}'),
               "Alias mixed[7:0] = child.q, child.value; "
               "Alias slice[3:0] = mixed[7:4]; "
               "DataOutPort bad[3:0] { Source slice; }");
  pattern::Design invalid;
  invalid.AddIcl("alias_slice.icl", mixed);
  Fails([&] { pattern::Elaborate(invalid, "ConnectionHierarchy"); },
        "only available to aliases or PDL");

  for (size_t count : {size_t{256}, size_t{4096}}) {
    auto large = source;
    std::string aliases = "Alias a0[3:0] = child.value;";
    for (size_t i = 1; i < count; ++i) {
      aliases += "Alias a" + std::to_string(i) + "[3:0] = a" +
                 std::to_string(i - 1) + ";";
    }
    aliases +=
        "DataOutPort bad[3:0] { Source a" + std::to_string(count - 1) + "; }";
    large.insert(large.rfind('}'), aliases);
    const auto start = std::chrono::steady_clock::now();
    pattern::Design chained;
    chained.AddIcl("restricted_chain.icl", large);
    Fails([&] { pattern::Elaborate(chained, "ConnectionHierarchy"); },
          "only available to aliases or PDL");
    std::cout << "Restricted alias chain: " << count << " aliases, "
              << large.size() << " bytes, seconds="
              << std::chrono::duration<double>(
                     std::chrono::steady_clock::now() - start)
                     .count()
              << '\n';
  }
  pattern::Design wide;
  wide.AddIcl("wide_alias.icl",
              "Module Leaf { DataInPort d[65535:0]; } Module Wide {"
              " Instance child Of Leaf; Alias a[65535:0] = child.d;"
              " DataOutPort bad[65535:0] { Source a; } }");
  Fails([&] { pattern::Elaborate(wide, "Wide"); },
        "only available to aliases or PDL");

  source.insert(source.rfind('}'),
                "Alias wrapper[3:0] = hidden; Alias hidden[3:0] = child.value; "
                "Alias input_wrapper[3:0] = child.data;");
  pattern::Design valid;
  valid.AddIcl("pdl_alias.icl", source);
  const auto netlist = pattern::Elaborate(valid, "ConnectionHierarchy");
  Require(netlist.NodeAt("wrapper").inputs.at("Source")[0].node ==
              netlist.node_ids.at("hidden"),
          "single-level hidden aliases retain wrapper definitions for PDL");
}

void TestDeepAliasConsumers() {
  std::ifstream input(
      "tests/cases/ours/pattern_generation/connection_hierarchy.icl");
  std::string source;
  Require(input.good() && toolsets::ReadTextStream(input, &source),
          "read deep alias fixture");
  for (const std::string extra :
       {"DataOutPort bad[3:0] { Source deep; }",
        "DataOutPort bad[3:0] { Source ~deep_port[0:3]; }",
        "Alias wrapper[3:0] = deep;", "LogicSignal bad { deep[0] == 1'b0; }",
        "DataMux bad SelectedBy deep[0] { 1'b0: 0; 1'b1: 1; }",
        "DataMux bad[3:0] SelectedBy 1'b0 { 1'b0: deep_port; 1'b1: 0; }",
        "ScanRegister bad[3:0] { ScanInSource si; CaptureSource deep; }",
        "OneHotDataGroup bad[3:0] { Port deep_port; }",
        "ScanInterface bad { Port deep[0]; }",
        "ScanInterface bad { Chain c { Port deep[0]; } }",
        "Instance sink Of ConnectionHierarchyLeaf { InputPort si = middle.so;"
        "InputPort data = deep; InputPort rst = rst; }",
        "Alias mixed[7:0] = child.q, middle.leaf.value;"
        "DataOutPort bad[3:0] { Source mixed[7:4]; }"}) {
    for (bool forward : {false, true}) {
      auto invalid_source = source;
      invalid_source.insert(forward ? invalid_source.find("  Alias deep[")
                                    : invalid_source.rfind('}'),
                            extra);
      pattern::Design invalid;
      invalid.AddIcl("deep_alias.icl", invalid_source);
      Fails([&] { pattern::Elaborate(invalid, "ConnectionHierarchy"); },
            "multi-level alias");
    }
  }

  const size_t aliases = 5000;
  std::string extra = "Alias a0[3:0] = child.q;";
  for (size_t i = 1; i < aliases; ++i) {
    extra += "Alias a" + std::to_string(i) + "[3:0] = a" +
             std::to_string(i - 1) + ";";
  }
  extra += "DataOutPort alias_output[3:0] { Source a4999; }";
  source.insert(source.rfind('}'), extra);
  const auto start = std::chrono::steady_clock::now();
  pattern::Design valid;
  valid.AddIcl("local_alias_chain.icl", source);
  const auto netlist = pattern::Elaborate(valid, "ConnectionHierarchy");
  Require(netlist.NodeAt("alias_output").inputs.at("Source")[0].node ==
              netlist.node_ids.at("a4999"),
          "local alias chains and child outputs remain hardware connections");
  Require(netlist.NodeAt("deep").inputs.at("Source")[0].node ==
              netlist.node_ids.at("middle.leaf.value"),
          "unconsumed deep alias remains available to PDL");
  std::cout << "Alias chain: " << aliases << " aliases, " << source.size()
            << " bytes, seconds="
            << std::chrono::duration<double>(std::chrono::steady_clock::now() -
                                             start)
                   .count()
            << '\n';
}

void TestInvertedUnsizedSources() {
  for (const auto& source : {"~0", "~'h0", "~'bx", "~$P"}) {
    pattern::Design design;
    design.AddIcl("inverted-unsized.icl",
                  "Module M { Parameter P = 0; DataOutPort q[3:0] { Source " +
                      std::string(source) + "; } }");
    Fails([&] { pattern::Elaborate(design, "M"); }, "unsized");
  }
  pattern::Design doubled;
  doubled.AddIcl("double-inversion.icl",
                 "Module M { DataRegister d { WriteDataSource 1'b1; "
                 "WriteEnSource ~~0; } }");
  Fails([&] { pattern::Elaborate(doubled, "M"); }, "unsized");
  pattern::Design design;
  design.AddIcl("sized-inversion.icl",
                "Module M { Parameter P = 4'h0; DataOutPort q[3:0] { Source "
                "~$P; } DataOutPort x[3:0] { Source 1'b1, 'bx, 1'b1; } }");
  const auto netlist = pattern::Elaborate(design, "M");
  for (const auto& bit : netlist.NodeAt("q").inputs.at("Source")) {
    Require(bit.constant == '1', "sized parameter source inversion");
  }
  const auto& bits = netlist.NodeAt("x").inputs.at("Source");
  Require(bits[0].constant == '1' && bits[1].constant == 'x' &&
              bits[2].constant == 'x' && bits[3].constant == '1',
          "unsized X source retains contextual padding");
}
void TestScanOutputBits() {
  pattern::Design valid;
  valid.AddIcl("scan_output.icl", R"(
Module M {
  ScanInPort si;
  ScanRegister descending[3:0] { ScanInSource si; }
  ScanRegister ascending[0:3] { ScanInSource descending; }
  ScanRegister scalar { ScanInSource ascending; }
  ScanOutPort outputs[2:0] { Source descending[0], ~ascending[3], scalar; }
  ScanOutPort enabled { Source descending[0]; Enable descending[2]; }
  DataOutPort middle { Source ascending[2]; }
}
)");
  const auto netlist = pattern::Elaborate(valid, "M");
  const auto& bits = netlist.NodeAt("outputs").inputs.at("Source");
  Require(bits.size() == 3 && bits[0].index == 0 && bits[1].index == 3 &&
              bits[1].inverted && bits[2].node == netlist.node_ids.at("scalar"),
          "ScanOutPort preserves declared rightmost bits and inversion");
  Require(netlist.NodeAt("middle").inputs.at("Source")[0].index == 2 &&
              netlist.NodeAt("enabled").inputs.at("Enable")[0].index == 2,
          "data and enable sources may use intermediate register bits");

  const std::pair<std::string, std::string> cases[] = {
      {"[3:0]", "ScanOutPort so { Source r[2]; }"},
      {"[0:3]", "ScanOutPort so { Source ~r[0]; }"},
      {"[3:0]", "ScanOutPort so[3:0] { Source r; }"},
      {"[0:3]", "ScanOutPort so[1:0] { Source r[2:3]; }"},
      {"[3:0]", "ScanOutPort so[1:0] { Source r[0], r[2]; }"}};
  for (const auto& test : cases) {
    pattern::Design invalid;
    invalid.AddIcl("bad_scan_output.icl",
                   "Module M { ScanInPort si; ScanRegister r" + test.first +
                       " { ScanInSource si; } " + test.second + " }");
    Fails([&] { pattern::Elaborate(invalid, "M"); },
          "ScanOutPort Source must use the rightmost bit of r");
  }
  pattern::Design bare;
  bare.AddIcl(
      "bare_scan_output.icl",
      "Module M { ScanInPort si; ScanRegister r[3:0] { ScanInSource si; } "
      "ScanOutPort so { Source r; } }");
  Fails([&] { pattern::Elaborate(bare, "M"); }, "width");
}

void TestImplicitScanBit() {
  pattern::Design design;
  design.AddIcl("bare_scan.icl", R"(
Module M {
  ScanInPort si;
  ScanRegister descending[7:0] { ScanInSource si; }
  ScanRegister ascending[0:7] { ScanInSource descending; }
  ScanRegister last { ScanInSource ~ascending; }
}
)");
  const auto netlist = pattern::Elaborate(design, "M");
  Require(netlist.NodeAt("ascending").inputs.at("ScanInSource")[0].index == 0 &&
              netlist.NodeAt("last").inputs.at("ScanInSource")[0].index == 7 &&
              netlist.NodeAt("last").inputs.at("ScanInSource")[0].inverted,
          "IEEE 6.4.8(u): bare ScanInSource selects declared rightmost bit");
}

void TestEnumAndPhysicalScanNames() {
  pattern::Design design;
  design.AddIcl("enum_capture.icl",
                "Module M { Enum E { si = 2'b01; } ScanInPort si; ScanRegister "
                "a[1:0] { ScanInSource si; RefEnum E; CaptureSource si; } }");
  const auto netlist = pattern::Elaborate(design, "M");
  const auto& reg = netlist.NodeAt("a");
  const auto& source = reg.inputs.at("ScanInSource");
  Require(source.size() == 1 && source[0].node == netlist.node_ids.at("si"),
          "ScanInSource uses physical signal despite RefEnum symbol collision");
  Require(reg.inputs.at("CaptureSource")[1].constant == '1',
          "CaptureSource retains IEEE 6.4.8(t) enum precedence");
}

void TestPerBitGraph() {
  pattern::Design design;
  design.AddIcl(
      "cross_bit.icl",
      "Module C { DataInPort d[1:0]; DataOutPort q[1:0] { Source d; } } Module "
      "M { Instance c Of C { InputPort d = 1'b0,c.q[1]; } }");
  const auto netlist = pattern::Elaborate(design, "M");
  Require(netlist.NodeAt("c.d").inputs.at("Source")[1].index == 1,
          "acyclic dependencies between bits of the same bus remain valid");
}

void TestOverrideTypes() {
  for (const auto& parameter :
       {std::string("Parameter P = \"foo\";"),
        std::string("Parameter P = 2'b01;"),
        std::string("Parameter Q = \"foo\"; Parameter P = $Q;")}) {
    pattern::Design design;
    design.AddIcl("override_type.icl",
                  "Module C { " + parameter +
                      " } Module M { Instance c Of C { Parameter P = 2; } }");
    Fails([&] { pattern::Elaborate(design, "M"); }, "parameter override type");
  }
}

void TestKeywordEnumSymbols() {
  pattern::Design design;
  design.AddIcl(
      "enum_keywords.icl",
      "Module M { Enum E { Source = 1'b0; ResetValue = 1'b1; Attribute = 1'b0; "
      "Instance = 1'b1; DataRegister = 1'b0; } ScanInPort si; "
      "ScanRegister r { ScanInSource si; RefEnum E; ResetValue Source; } }");
  const auto netlist = pattern::Elaborate(design, "M");
  Require(netlist.NodeAt("r").constants.at("ResetValue").data == "0",
          "enum symbols named after properties remain ordinary symbols");
  Require(netlist.NodeAt("E").constants.size() == 5,
          "all keyword-named enum symbols are preserved");
}

void TestDataRegisterControls() {
  pattern::Design design;
  design.AddIcl("selectable.icl", R"(
Module M {
  DataInPort data[3:0];
  DataInPort gate;
  DataRegister r[3:0] { WriteDataSource data; WriteEnSource ~gate; }
  DataRegister s[3:0] { WriteDataSource r; WriteEnSource ~~gate; }
  DataOutPort q[3:0] { Source s; Enable gate; }
}

)");
  const auto netlist = pattern::Elaborate(design, "M");
  const auto& r = netlist.NodeAt("r");
  Require(r.inputs.count("WriteEnSource") == 1,
          "selectable register enable is elaborated");
  Require(r.inputs.at("WriteEnSource")[0].node == netlist.node_ids.at("gate") &&
              r.inputs.at("WriteEnSource")[0].inverted,
          "selectable write enable preserves inversion");
  Require(!netlist.NodeAt("s").inputs.at("WriteEnSource")[0].inverted,
          "two legal WriteEnSource inversions cancel");
  Require(netlist.NodeAt("q").inputs.at("Enable")[0].node ==
              netlist.node_ids.at("gate"),
          "output enable is retained independently of data");
  pattern::Design wide;
  wide.AddIcl("wide_enable.icl", R"(
Module M {
  DataInPort data[3:0];
  DataRegister r[3:0] { WriteDataSource data; WriteEnSource data; }
}
)");
  Fails([&] { pattern::Elaborate(wide, "M"); }, "width");
}

void TestRegisterRules() {
  const std::pair<std::string, std::string> cases[] = {
      {"ScanRegister r;", "ScanInSource"},
      {"ScanInPort si; ScanRegister r[3:0] { ScanInSource si; } "
       "ScanRegister s { ScanInSource r[1]; }",
       "rightmost"},
      {"ScanInPort si; ScanRegister r { ScanInSource si; ResetValue 1'b0; "
       "DefaultLoadValue 1'b1; }",
       "ResetValue"},
      {"DataInPort d; DataRegister r { WriteDataSource d; ResetValue 1'b1; "
       "DefaultLoadValue 1'b0; }",
       "ResetValue"},
      {"Enum E { a = 2'b00; } ScanInPort si; ScanRegister r { "
       "ScanInSource si; RefEnum E; }",
       "enum value width"}};
  for (const auto& test : cases) {
    pattern::Design design;
    design.AddIcl("register_rules.icl", "Module M { " + test.first + " }");
    Fails([&] { pattern::Elaborate(design, "M"); }, test.second);
  }
}
void TestInterfaceKinds() {
  using Kind = pattern::ScanInterfaceKind;
  const std::vector<std::pair<std::string, Kind>> cases = {
      {"ShiftEnPort shift; ScanInterface bus { Port si; Port so; "
       "DefaultLoadValue 8'hda; }",
       Kind::kClient},
      {"ToShiftEnPort shift { Source 1'b1; } "
       "ScanInterface bus { Port so; }",
       Kind::kHost},
      {"TMSPort tms; TRSTPort trst; "
       "ScanInterface bus { Port si; Port so; Port tms; Port trst; }",
       Kind::kClientTap},
      {"ToTMSPort tms { Source 1'b0; } "
       "ScanInterface bus { Port so; Port tms; }",
       Kind::kHostTap}};
  for (const auto& test : cases) {
    pattern::Design design;
    design.AddIcl("interface.icl",
                  "Module M { ScanInPort si; "
                  "ScanOutPort so { Source si; } " +
                      test.first + " }");
    const auto netlist = pattern::Elaborate(design, "M");
    Require(netlist.NodeAt("bus").scan_interface->kind == test.second,
            "IEEE client/host/TAP interface classification");
  }
  const std::vector<std::pair<std::string, std::string>> invalid = {
      {"ShiftEnPort shift; ScanInterface bus { Port si; }",
       "valid client/host"},
      {"ShiftEnPort shift; ScanInterface bus { Port si; Port si; Port so; }",
       "repeated ScanInterface port"},
      {"ScanRegister r { ScanInSource si; } ScanInterface bus { Port r; }",
       "invalid or repeated"},
      {"ShiftEnPort shift; ToSelectPort select { Source 1'b1; } "
       "ScanInterface bus { Port si; Port so; Port shift; Port select; }",
       "valid client/host"},
      {"ShiftEnPort shift; ScanInterface bus { Port si; "
       "Chain c { Port si; Port so; } }",
       "repeated ScanInterface port"},
      {"ShiftEnPort shift; ScanInterface bus { DefaultLoadValue 1'b0; "
       "Chain c { Port si; Port so; } }",
       "DefaultLoadValue"},
      {"ShiftEnPort shift; ScanInterface bus { "
       "Chain c { Port si; Port shift; } }",
       "one SI/SO pair"},
      {"ShiftEnPort shift; ToShiftEnPort to_shift { Source 1'b1; } "
       "ScanInterface bus { Port si; Port so; }",
       "one valid"}};
  for (const auto& test : invalid) {
    pattern::Design design;
    Fails(
        [&] {
          design.AddIcl("interface_invalid.icl",
                        "Module M { ScanInPort si; "
                        "ScanOutPort so { Source si; } " +
                            test.first + " }");
          pattern::Elaborate(design, "M");
        },
        test.second);
  }
}
void TestAddressedInstanceMetadata() {
  pattern::Design design;
  design.AddIcl("addressed.icl", R"(
Module Leaf { Parameter A = 1; Attribute label = "module"; }
Module Top {
  Parameter A = 8'h80;
  OneHotDataGroup group {
    Instance child Of Leaf {
      AddressValue $A;
      Attribute label = "instance";
      Attribute address = $A;
      Attribute flag;
    }
  }
}
)");
  const auto netlist = pattern::Elaborate(design, "Top");
  const auto& child = netlist.scopes.at(1);
  Require(child.address_value && child.address_value->data == "10000000" &&
              child.address_value->sized,
          "instance address is evaluated in the enclosing parameter scope");
  Require(child.attributes.at("label")->data == "module" &&
              child.instance_attributes.at("label")->data == "instance" &&
              child.instance_attributes.at("address")->data == "10000000" &&
              !child.instance_attributes.at("flag"),
          "instance attributes retain their own values without replacing "
          "module attributes");
}

void TestOneHotScanErrors() {
  const std::vector<std::pair<std::string, std::string>> cases = {
      {"OneHotScanGroup group[1:0] { Port si; }", "width"},
      {"OneHotScanGroup group { Port group; }", "cycle"},
      {"Alias a = group; OneHotScanGroup group { Port a; }", "cycle"},
      {"OneHotScanGroup group { Port bare.so; }", "enabled ScanOutPort"},
      {"OneHotScanGroup group { Port wrong.so; }", "enabled ScanOutPort"}};
  for (const auto& test : cases) {
    pattern::Design design;
    design.AddIcl(
        "one_hot_invalid.icl",
        "Module Bare { ScanInPort si; ScanOutPort so { Source si; } } "
        "Module Wrong { DataOutPort so { Source 1'b0; Enable 1'b1; } } "
        "Module M { ScanInPort si; Instance bare Of Bare { InputPort si = si; "
        "} "
        "Instance wrong Of Wrong; " +
            test.first + " }");
    Fails([&] { pattern::Elaborate(design, "M"); }, test.second);
  }
}

void TestOneHotDataPorts() {
  std::ifstream input("tests/cases/ours/pattern_generation/one_hot_width.icl");
  std::string source;
  Require(toolsets::ReadTextStream(input, &source),
          "read Tessent width fixture");
  pattern::Design design;
  design.AddIcl("one_hot_width.icl", source);
  for (const std::string top :
       {"EqualWidthProbe", "WidthProbe", "AscendingWidthProbe"}) {
    const auto netlist = pattern::Elaborate(design, top);
    const auto& group = netlist.NodeAt("wide");
    Require(
        group.one_hot_sources.size() == 1 &&
            group.one_hot_sources[0].bits.size() == 8,
        "retain the original narrower contributor without synthetic signals");
    const auto& bits = group.one_hot_sources[0].bits;
    Require(netlist.nodes[bits[0].node].name == "driver.value" &&
                bits.front().index == 7 && bits.back().index == 0,
            "contributor order follows the declared signal direction");
    const auto first = group.OneHotSourceBit(0, 0);
    if (top != "EqualWidthProbe") {
      Require(first.node == std::numeric_limits<size_t>::max() &&
                  first.constant == 'x',
              "Tessent: leading excess bits are unknown, not zero-filled");
    } else {
      Require(first.node == bits.front().node && first.index == 7,
              "equal widths need no padding");
    }
    Require(group.OneHotSourceBit(0, group.Width() - 1).index == 0,
            "right alignment follows significance even for ascending groups");
    bool rejected = false;
    try {
      group.OneHotSourceBit(0, group.Width());
    } catch (const std::out_of_range&) {
      rejected = true;
    }
    Require(rejected, "out-of-range group bit offsets are rejected");
  }
}

void TestOneHotDataErrors() {
  for (const auto& test : std::vector<std::pair<std::string, std::string>>{
           {"OneHotDataGroup g { Port wide; }", "width"},
           {"OneHotDataGroup g { Port g; }", "cycle"},
           {"OneHotDataGroup g { Port plain; }", "enabled DataOutPort"},
           {"OneHotDataGroup g { Port scan; }", "enabled DataOutPort"},
           {"OneHotDataGroup g { Port via; } DataOutPort via { Source g; }",
            "cycle"},
           {"DataOutPort small { Source 1'b0; Enable en; } "
            "DataOutPort large[1:0] { Source g[1],1'b0; Enable en; } "
            "OneHotDataGroup g[1:0] { Port small; Port large; }",
            "cycle"}}) {
    pattern::Design design;
    design.AddIcl("bad_data_group.icl",
                  "Module M { DataInPort en; DataInPort d; ScanInPort si; "
                  "DataOutPort wide[1:0] { Source 2'b00; Enable en; } "
                  "DataOutPort plain { Source d; } "
                  "ScanOutPort scan { Source si; Enable en; } " +
                      test.first + " }");
    Fails([&] { pattern::Elaborate(design, "M"); }, test.second);
  }
}

void TestOneHotDataBitDependencies() {
  // Only the rightmost output bit depends on this scalar contributor. The
  // leading X bit breaks the apparent object-level feedback loop.
  for (const bool ascending : {false, true}) {
    for (const bool cyclic : {false, true}) {
      const int source_bit = cyclic ? (ascending ? 1 : 0) : (ascending ? 0 : 1);
      pattern::Design design;
      design.AddIcl(
          "padding_dependencies.icl",
          "Module Producer { DataInPort value; DataInPort en; "
          "DataOutPort selected { Source value; Enable en; } } "
          "Module M { DataInPort en; OneHotDataGroup g" +
              std::string(ascending ? "[0:1]" : "[1:0]") +
              " { Port producer.selected; } Instance producer Of Producer { "
              "InputPort value = g[" +
              std::to_string(source_bit) + "]; InputPort en = en; } }");
      if (cyclic) {
        Fails([&] { pattern::Elaborate(design, "M"); }, "cycle");
      } else {
        const auto netlist = pattern::Elaborate(design, "M");
        Require(netlist.NodeAt("g").one_hot_sources[0].bits.size() == 1,
                "unknown leading bits do not create false connection cycles");
      }
    }
  }
}

void TestOneHotAddressedPort() {
  pattern::Design design;
  design.AddIcl("address_enable.icl", R"(
Module Leaf { DataOutPort out { Source 1'b0; } }
Module M {
  OneHotDataGroup addressed { Instance child Of Leaf { AddressValue 1; } }
  OneHotDataGroup g { Port child.out; }
}
)");
  const auto netlist = pattern::Elaborate(design, "M");
  Require(netlist.NodeAt("g").one_hot_sources.size() == 1,
          "IEEE 6.4.15(d)(3) recognizes instance-addressed output ports");
}

void TestOneHotDataLimits() {
  std::string source =
      "Module Producer { DataInPort en; DataOutPort out { Source 1'b0; "
      "Enable en; } } Module M { DataInPort en; "
      "Instance producer Of Producer { InputPort en = en; } ";
  for (size_t i = 0; i < 3000; ++i) {
    source += "OneHotDataGroup g" + std::to_string(i) + " { Port " +
              (i ? "g" + std::to_string(i - 1) : "producer.out") + "; } ";
  }
  pattern::Design deep;
  deep.AddIcl("deep_data_group.icl", source + "}");
  Require(
      pattern::Elaborate(deep, "M").NodeAt("g2999").one_hot_sources.size() == 1,
      "data enable propagation traverses deep groups without recursion");
  source =
      "Module Producer { DataInPort en; DataOutPort out[65535:0] { Source 0; "
      "Enable en; } } Module M { DataInPort en; "
      "Instance producer Of Producer { InputPort en = en; } "
      "OneHotDataGroup g[131071:0] { ";
  for (size_t i = 0; i < 65; ++i) source += "Port producer.out; ";
  pattern::Design wide;
  wide.AddIcl("wide_data_group.icl", source + "} }");
  Fails([&] { pattern::Elaborate(wide, "M"); }, "bit limit");
}

void TestOneHotScanSourceScope() {
  for (bool child : {false, true}) {
    for (bool alias : {false, true}) {
      for (bool inverted : {false, true}) {
        pattern::Design design;
        const std::string source = alias   ? "wrapped"
                                   : child ? "producer.out"
                                           : "producer";
        design.AddIcl(
            "one_hot_scan_scope.icl",
            "Module Producer { ScanInPort incoming[1:0]; DataInPort en; "
            "ScanOutPort out[1:0] { Source incoming; Enable en; } } "
            "Module M { ScanInPort incoming[1:0]; DataInPort en; " +
                std::string(child ? "Instance producer Of Producer { InputPort "
                                    "incoming = incoming; "
                                    "InputPort en = en; } "
                                  : "ScanOutPort producer[1:0] { Source "
                                    "incoming; Enable en; } ") +
                (alias ? "Alias wrapped[1:0] = " +
                             std::string(child ? "producer.out" : "producer") +
                             "; "
                       : "") +
                "OneHotScanGroup group[1:0] { Port " + (inverted ? "~" : "") +
                source + "; } }");
        if (!child) {
          Fails([&] { pattern::Elaborate(design, "M"); },
                "output must be referenced from an instance");
        } else {
          const auto text =
              pattern::SerializeIcl(pattern::Elaborate(design, "M"));
          pattern::Design flat;
          flat.AddIcl("flat.icl", text);
          Require(
              pattern::SerializeIcl(pattern::Elaborate(flat, "M")) == text,
              "child scan contributors preserve alias parity and flat scope");
        }
      }
    }
  }
  pattern::Design alias_only;
  alias_only.AddIcl(
      "scan_alias_only.icl",
      "Module M { ScanInPort incoming; ScanOutPort producer { Source incoming; "
      "Enable 1'b1; } Alias wrapped = producer; }");
  const auto text = pattern::SerializeIcl(pattern::Elaborate(alias_only, "M"));
  pattern::Design flat;
  flat.AddIcl("flat.icl", text);
  Require(pattern::SerializeIcl(pattern::Elaborate(flat, "M")) == text,
          "unused own scan-output aliases remain available to PDL");
  pattern::Design hierarchy;
  hierarchy.AddIcl(
      "scan_group_hierarchy.icl",
      "Module Leaf { ScanInPort incoming; ScanOutPort out { Source incoming; "
      "Enable 1'b1; } } Module Middle { ScanInPort incoming; Instance leaf Of "
      "Leaf { "
      "InputPort incoming = incoming; } ScanOutPort out { Source leaf.out; } "
      "Alias wrapped = out; } Module M { ScanInPort incoming; Instance child "
      "Of Middle { "
      "InputPort incoming = incoming; } OneHotScanGroup group { Port "
      "child.out; } }");
  const auto original = pattern::Elaborate(hierarchy, "M");
  for (const std::string source : {"child.leaf.out", "child.wrapped"}) {
    auto changed = original;
    changed.nodes.at(changed.node_ids.at("group")).one_hot_sources[0].bits[0] =
        {changed.node_ids.at(source), 0};
    pattern::Design altered;
    altered.AddIcl("altered_scan_group.icl", pattern::SerializeIcl(changed));
    Fails([&] { pattern::Elaborate(altered, "M"); }, source == "child.wrapped"
                                                         ? "hierarchical source"
                                                         : "reference scope");
  }
  for (const std::string source :
       {"child.out[1], producer[0]", "producer[1], child.out[0]"}) {
    pattern::Design mixed;
    mixed.AddIcl(
        "mixed_scan_group_scope.icl",
        "Module Leaf { ScanInPort incoming[1:0]; ScanOutPort out[1:0] { "
        "Source incoming; Enable 1'b1; } } Module M { ScanInPort "
        "incoming[1:0]; "
        "Instance child Of Leaf { InputPort incoming[1] = incoming[1]; } "
        "ScanOutPort producer[1:0] { Source incoming; Enable 1'b1; } "
        "OneHotScanGroup group[1:0] { Port " +
            source + "; } }");
    Fails([&] { pattern::Elaborate(mixed, "M"); },
          "output must be referenced from an instance");
  }
}
void TestOneHotScanMuxFanIn() {
  pattern::Design design;
  design.AddIcl("group_mux.icl", R"(
Module Leaf {
  ScanInPort si;
  DataInPort enable;
  ScanOutPort so { Source si; Enable enable; }
}
Module M {
  ScanInPort si;
  DataInPort enable;
  DataInPort select;
  Instance child Of Leaf { InputPort si = si; InputPort enable = enable; }
  ScanMux route SelectedBy select { 1'b0: si; 1'b1: child.so; }
  OneHotScanGroup group { Port route; }
}
)");
  const auto netlist = pattern::Elaborate(design, "M");
  Require(netlist.NodeAt("group").one_hot_sources.size() == 1,
          "scan group preserves a mux with an enabled source in its fan-in");
}

void TestOneHotScanLimits() {
  const std::string leaf =
      "Module Leaf { Parameter W = 1; ScanInPort si[$W-1:0]; DataInPort en; "
      "ScanOutPort so[$W-1:0] { Source si; Enable en; } } ";
  std::string source =
      leaf +
      "Module M { ScanInPort si; DataInPort en; "
      "Instance child Of Leaf { InputPort si = si; InputPort en = en; } ";
  for (size_t i = 0; i < 3000; ++i) {
    source += "OneHotScanGroup g" + std::to_string(i) + " { Port " +
              (i ? "g" + std::to_string(i - 1) : "child.so") + "; } ";
  }
  source += "}";
  pattern::Design deep;
  deep.AddIcl("deep_group.icl", source);
  const auto netlist = pattern::Elaborate(deep, "M");
  Require(netlist.NodeAt("g2999").one_hot_sources.size() == 1,
          "large group DAG traverses without recursive stack growth");

  pattern::Design wide;
  source = leaf +
           "Module M { ScanInPort si[65535:0]; DataInPort en; "
           "Instance child Of Leaf { Parameter W = 65536; InputPort si = si; "
           "InputPort en = en; } "
           "OneHotScanGroup group[65535:0] { ";
  for (size_t i = 0; i < 65; ++i) source += "Port child.so; ";
  source += "} }";
  wide.AddIcl("wide_group.icl", source);
  Fails([&] { pattern::Elaborate(wide, "M"); }, "bit limit");
}
}  // namespace

int main() {
  int failures = 0;
  for (const auto test : {TestClockSignalSources,
                          TestClockInputDifferentialAssociation,
                          TestTckInputSources,
                          TestResetSourceKinds,
                          TestResetPortPolarity,
                          TestFlatTrstPolarity,
                          TestResetMuxPathPolarity,
                          TestTrstMuxPathPolarity,
                          TestControlMuxIdentityPaths,
                          TestControlOutputSourceScope,
                          TestOrdinaryOutputSourceScope,
                          TestScanControlSourceKinds,
                          TestConcatScanSourceKinds,
                          TestStrictScanSources,
                          TestAddressPortSourceKinds,
                          TestDataMuxControlSourceUnion,
                          TestDataMuxMatchingControlUses,
                          TestCaptureUpdateMuxStructure,
                          TestDirectControlDataDrivers,
                          TestResetMuxStructure,
                          TestTypedControlSourceInversion,
                          TestTmsTrstSourceKinds,
                          TestInferredControlVectors,
                          TestInferredControlBudget,
                          TestDataSignalSources,
                          TestDataSignalAliasSlices,
                          TestDataSourceScale,
                          TestSpecialization,
                          TestAccessLinkModelInsertion,
                          TestAccessLinkModelErrors,
                          TestSharedAccessLinkModels,
                          TestGeneratedTapPorts,
                          TestAddressedDataDefaults,
                          TestAddressedGroupErrors,
                          TestErrors,
                          TestBitConnections,
                          TestResetInference,
                          TestMuxExpansionBudget,
                          TestMuxNumericCodes,
                          TestMuxSelectorGraph,
                          TestMuxAndEnum,
                          TestConnectionErrors,
                          TestConnectionHierarchy,
                          TestAliasInstanceBoundaries,
                          TestDeepAliasConsumers,
                          TestUnsizedSources,
                          TestInvertedUnsizedSources,
                          TestImplicitScanBit,
                          TestScanOutputBits,
                          TestEnumAndPhysicalScanNames,
                          TestPerBitGraph,
                          TestOverrideTypes,
                          TestKeywordEnumSymbols,
                          TestDataRegisterControls,
                          TestRegisterRules,
                          TestInterfaceKinds,
                          TestIrSelectOwnerControls,
                          TestOneHotScanErrors,
                          TestOneHotScanSourceScope,
                          TestOneHotScanMuxFanIn,
                          TestOneHotScanLimits,
                          TestAddressedInstanceMetadata,
                          TestOneHotDataPorts,
                          TestOneHotDataErrors,
                          TestOneHotDataBitDependencies,
                          TestOneHotAddressedPort,
                          TestOneHotDataLimits}) {
    try {
      test();
    } catch (const std::exception& error) {
      std::cerr << "FAIL: " << error.what() << '\n';
      ++failures;
    }
  }
  std::cout << "ICL elaboration failures: " << failures << '\n';
  return failures ? 1 : 0;
}
