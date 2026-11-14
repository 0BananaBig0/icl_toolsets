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
        "ToIRSelectPort source;", "ToTRSTPort source;",
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
      design.AddIcl("valid_clock_sources.icl",
                    "Module Sink { ClockPort ck; } Module M {" + declaration +
                        " Alias wrapped = source; " + body + "}");
      const auto model = pattern::Elaborate(design, "M");
      pattern::Design flat;
      flat.AddIcl("flat.icl", pattern::SerializeIcl(model));
      pattern::Elaborate(flat, "M");
    }
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
    pattern::Design design;
    design.AddIcl("tck_sources.icl",
                  "Module Child { TCKPort ck; } Module M {" + entry.first +
                      " Alias wrapped = source; Instance child Of Child { "
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
    pattern::Design design;
    design.AddIcl(
        "reset_kinds.icl",
        "Module Child { ResetPort r { ActivePolarity " + std::string(polarity) +
            "; } } Module M { " + declaration +
            " Alias wrapped = source; "
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
void TestScanControlSourceKinds() {
  for (const std::string family : {"CaptureEn", "ShiftEn", "UpdateEn"}) {
    pattern::Design design;
    design.AddIcl(
        "control_sources.icl",
        "Module Child { " + family + "Port ctl[1:0]; } Module M { " + family +
            "Port input[1:0]; To" + family + "Port bare; To" + family +
            "Port output[1:0] { Source input; } DataInPort data; "
            "Alias wrapped[1:0] = output; Instance direct Of Child { InputPort "
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
                      "ToResetPort reset[3:0] { Source upstream; } "
                      "ToCaptureEnPort capture[3:0] { Source upstream; } "
                      "ToShiftEnPort shift[3:0] { Source upstream; } "
                      "ToUpdateEnPort update[3:0] { Source upstream; } "
                      "ToTMSPort tms[3:0] { Source upstream; } "
                      "ToTRSTPort trst[3:0] { Source upstream; } "
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
        "Module Child { " + family + "Port ctl[1:0]; } Module M { " + family +
            "Port input[1:0]; To" + family +
            "Port bare[1:0] { Source input; } To" + family +
            "Port output[1:0] { Source input; } DataInPort data; "
            "Alias wrapped[1:0] = output; Instance direct Of Child { InputPort "
            "ctl = wrapped; } "
            "Instance partial Of Child { InputPort ctl[1] = data; } "
            "Instance constants Of Child { InputPort ctl = 2'b1x; } }");
    const auto original = pattern::Elaborate(design, "M");
    pattern::Design flat;
    flat.AddIcl("flat.icl", pattern::SerializeIcl(original));
    const auto restored = pattern::Elaborate(flat, "M");
    Require(pattern::SerializeIcl(restored) == pattern::SerializeIcl(original),
            "TMS/TRST own/data/constant/partial and inferred output sources "
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
      design.AddIcl("padding_dependencies.icl",
                    "Module M { DataInPort en; OneHotDataGroup g" +
                        std::string(ascending ? "[0:1]" : "[1:0]") +
                        " { Port selected; } DataOutPort selected { Source g[" +
                        std::to_string(source_bit) + "]; Enable en; } }");
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
      "Module M { DataInPort en; DataOutPort out { Source 1'b0; Enable en; } ";
  for (size_t i = 0; i < 3000; ++i) {
    source += "OneHotDataGroup g" + std::to_string(i) + " { Port " +
              (i ? "g" + std::to_string(i - 1) : "out") + "; } ";
  }
  pattern::Design deep;
  deep.AddIcl("deep_data_group.icl", source + "}");
  Require(
      pattern::Elaborate(deep, "M").NodeAt("g2999").one_hot_sources.size() == 1,
      "data enable propagation traverses deep groups without recursion");
  source =
      "Module M { DataInPort en; DataOutPort out[65535:0] { Source 0; Enable "
      "en; } "
      "OneHotDataGroup g[131071:0] { ";
  for (size_t i = 0; i < 65; ++i) source += "Port out; ";
  pattern::Design wide;
  wide.AddIcl("wide_data_group.icl", source + "} }");
  Fails([&] { pattern::Elaborate(wide, "M"); }, "bit limit");
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
                          TestTckInputSources,
                          TestResetSourceKinds,
                          TestScanControlSourceKinds,
                          TestConcatScanSourceKinds,
                          TestStrictScanSources,
                          TestAddressPortSourceKinds,
                          TestDataMuxControlSourceUnion,
                          TestDataMuxMatchingControlUses,
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
                          TestOneHotScanErrors,
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
