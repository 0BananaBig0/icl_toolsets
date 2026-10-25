// Copyright 2026 The ICL Toolsets Authors.
// SPDX-License-Identifier: MIT
#include "pattern_flatten.h"

#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include "pattern_logic.h"
#include "pattern_runtime.h"

namespace {
void Require(bool condition, const std::string& message) {
  if (!condition) throw std::runtime_error(message);
}

void TestHierarchyRoundTrip() {
  pattern::Design design;
  design.AddIcl("hierarchy.icl", R"(
Module Leaf {
  Parameter W = 4;
  ResetPort reset { ActivePolarity 0; }
  ScanInPort si;
  ScanRegister r[$W-1:0] { ScanInSource si; ResetValue 0; }
  LogicSignal active { r == 1; }
  ScanOutPort so { Source r[0]; }
}
Module Top {
  ScanInPort si;
  ResetPort reset;
  ScanOutPort so { Source b.so; }
  Instance a Of Leaf { InputPort si = si; }
  Instance b Of Leaf { Parameter W = 8; InputPort si = a.so; }
}
)");
  const auto original = pattern::Elaborate(design, "Top");
  const auto text = pattern::SerializeIcl(original);
  const icl::ParsedDocument parsed(text);
  Require(parsed.ok(), "flat ICL parses with the shared frontend");
  for (const auto& declaration : parsed.declarations()) {
    Require(declaration.kind != "Instance", "flat ICL contains no instances");
  }
  pattern::Design flat_design;
  flat_design.AddIcl("flat.icl", text);
  const auto flat = pattern::Elaborate(flat_design, "Top");
  Require(pattern::SerializeIcl(flat) == text,
          "flat ICL is byte stable on reimport");
  Require(flat.nodes.size() == original.nodes.size() &&
              flat.scopes.size() == original.scopes.size(),
          "original object and binding identities survive flattening");
  Require(flat.NodeAt("a.r").Width() == 4 && flat.NodeAt("b.r").Width() == 8,
          "parameter specializations survive flattening");
  const auto& reset = *flat.NodeAt("a.r").reset;
  Require(flat.nodes[reset.node].name == "a.reset" && reset.inverted,
          "per-register local reset domain and polarity survive flattening");
  const std::string pdl = R"(iProcsForModule Leaf
 iProc run {} { iWrite r $W; iApply }
 iCall a.run
 iCall b.run
)";
  pattern::Runtime before(design, original), after(flat_design, flat);
  before.Execute("original.pdl", pdl);
  after.Execute("original.pdl", pdl);
  before.Finish();
  after.Finish();
  Require(before.operations().size() == after.operations().size(),
          "same original PDL runs on flat ICL");
  for (size_t i = 0; i < before.operations().size(); ++i) {
    Require(
        before.operations()[i].command == after.operations()[i].command &&
            before.operations()[i].arguments == after.operations()[i].arguments,
        "flat PDL binding preserves original targets and parameter values");
  }
}
void TestPropertiesAndNames() {
  pattern::Design design;
  design.AddIcl("properties.icl", R"(
NameSpace vendor;
Module Leaf {
  Parameter TAG = "instrument";
  Attribute leaf_tag = $TAG;
  DataInPort a[1:0] { Attribute pin_tag = $TAG; }
  DataOutPort q[1:0] { Source a; }
}
Module Top {
  Attribute top_tag = "test";
  DataInPort n612e61;
  ScanInPort si;
  Enum Modes { off = 2'b00; on = 2'b11; }
  ScanRegister ctrl[1:0] { ScanInSource si; RefEnum Modes; ResetValue off; }
  Instance a Of Leaf { InputPort a = ctrl; }
  Alias status[1:0] = a.q { iApplyEndState 2'b01; Attribute note = "alias"; }
  ScanMux route SelectedBy ctrl[0] { 1'b0: si; 1'b1: ctrl[0]; }
  ScanOutPort so { Source route; }
  LogicSignal parity { ^ctrl; }
  LogicSignal same { ctrl == on; }
}
)");
  const auto original = pattern::Elaborate(design, "vendor::Top");
  const auto first = pattern::SerializeIcl(original);
  pattern::Design flat_design;
  flat_design.AddIcl("flat.icl", first);
  const auto flat = pattern::Elaborate(flat_design, "vendor::Top");
  Require(pattern::SerializeIcl(flat) == first,
          "enum, aliases, mux, logic and attributes are canonical");
  Require(flat.NodeAt("a.a").name != flat.NodeAt("n612e61").name,
          "encoded names cannot collide with original user names");
  Require(flat.NodeAt("ctrl").constants.at("ResetValue").data == "00" &&
              flat.NodeAt("status").constants.at("iApplyEndState").data == "01",
          "enumerated reset and alias end-state values survive");
  Require(flat.NodeAt("route").choices[1].codes[0].data == "1",
          "mux selection survives");
  const auto ones = [](const pattern::BitReference&) { return '1'; };
  Require(pattern::EvaluateLogic(flat.NodeAt("parity"), ones) == '0' &&
              pattern::EvaluateLogic(flat.NodeAt("same"), ones) == '1',
          "compiled logic retains its truth table");
}

void TestUnconnectedInputs() {
  pattern::Design design;
  design.AddIcl("inputs.icl", R"(
Module Leaf {
  DataInPort data[3:0];
  DataInPort unused;
  ResetPort reset;
  ScanInPort si;
  ScanRegister r { ScanInSource si; }
}

Module Top {
  ScanInPort si;
  Instance a Of Leaf { InputPort si = si; InputPort data[1:0] = 2'b10; }
}
)");
  const auto original = pattern::Elaborate(design, "Top");
  const auto text = pattern::SerializeIcl(original);
  pattern::Design design_flat;
  design_flat.AddIcl("flat.icl", text);
  const auto flat = pattern::Elaborate(design_flat, "Top");
  Require(pattern::SerializeIcl(flat) == text, "disconnected inputs replay");
  Require(flat.NodeAt("a.unused").inputs.empty(),
          "unconnected stays unconnected");
  const auto& data = flat.NodeAt("a.data").inputs.at("Source");
  Require(data[0].constant == '?' && data[1].constant == '?' &&
              data[2].constant == '1' && data[3].constant == '0',
          "partial input mapping keeps missing bits separate from X");
  Require(flat.NodeAt("a.reset").inputs.at("Source")[0].virtual_signal ==
              pattern::VirtualSignal::kGlobalReset,
          "implicit reset remains virtual");
}

void TestNamespaceBinding() {
  pattern::Design design;
  design.AddIcl("namespaces.icl", R"(
Module Unused { Parameter U = 5; }
NameSpace vendor1;
Module Leaf { Parameter V = 3; ScanInPort si;
  ScanRegister r[7:0] { ScanInSource si; } }
NameSpace vendor2;
Module Leaf { Parameter V = 7; ScanInPort si;
  ScanRegister r[7:0] { ScanInSource si; } }
NameSpace;
Module Top {
  Parameter V = 9;
  Attribute flag;
  ScanInPort si;
  ScanRegister root[7:0] { ScanInSource si; }
  Instance a Of vendor1::Leaf { InputPort si = si; }
  Instance b Of vendor2::Leaf { InputPort si = si; }
}
)");
  const auto original = pattern::Elaborate(design, "Top");
  const auto text = pattern::SerializeIcl(original);
  pattern::Design flat_design;
  flat_design.AddIcl("flat.icl", text);
  const auto flat = pattern::Elaborate(flat_design, "Top");
  Require(pattern::SerializeIcl(flat) == text,
          "namespace metadata is canonical");
  const std::string pdl = R"(iProcsForModule Top
iProc root_run {} { iWrite root $V; iApply }
iCall root_run
iProcsForModule vendor1::Leaf
iProc run {} { iWrite r $V; iApply }
iCall a.run
iProc run {} { iWrite r [expr {$V+1}]; iApply }
iCall a.run
iProcsForModule vendor2::Leaf
iProc run {} { iWrite r $V; iApply }
iCall b.run
iProcsForModule Unused
iProc dormant {} { iNote $U }
)";
  pattern::Runtime before(design, original), after(flat_design, flat);
  before.Execute("binding.pdl", pdl);
  after.Execute("binding.pdl", pdl);
  before.Finish();
  after.Finish();
  Require(before.operations().size() == 8 && after.operations().size() == 8,
          "root and child procedures run before and after overrides");
  for (size_t i = 0; i < before.operations().size(); ++i) {
    Require(before.operations()[i].arguments == after.operations()[i].arguments,
            "qualified bindings preserve procedure parameters on replay");
  }
}

std::string WithMetadata(const std::string& body, const std::string& nodes) {
  const std::string json =
      "{\"version\":1,\"modules\":[[\"Top\",[]]],"
      "\"scopes\":[[\"\",\"Top\",[],[]]],\"nodes\":" +
      nodes + "}";
  std::string quoted;
  for (char c : json) {
    if (c == '"' || c == '\\') quoted += '\\';
    quoted += c;
  }
  return "Module Top { Attribute " +
         std::string(pattern::kFlatOriginAttribute) + " = \"" + quoted +
         "\"; " + body + " }";
}

void TestIndependentResetDomains() {
  pattern::Design design;
  design.AddIcl("resets.icl", R"(
Module Leaf {
  ResetPort reset;
  ScanInPort si;
  ScanRegister r { ScanInSource si; ResetValue 0; }
}
Module Top {
  ResetPort reset_a;
  ResetPort reset_b;
  ScanInPort si;
  Instance a Of Leaf { InputPort si = si; InputPort reset = reset_a; }
  Instance b Of Leaf { InputPort si = si; InputPort reset = reset_b; }
}
)");
  const auto original = pattern::Elaborate(design, "Top");
  const auto text = pattern::SerializeIcl(original);
  pattern::Design flat_design;
  flat_design.AddIcl("flat.icl", text);
  const auto flat = pattern::Elaborate(flat_design, "Top");
  Require(pattern::SerializeIcl(flat) == text,
          "independent reset domains survive flat-module ambiguity");
  Require(flat.NodeAt("a.r").reset->node != flat.NodeAt("b.r").reset->node,
          "separate reset domains remain separate");
}

void TestInvalidMetadata() {
  const std::vector<std::pair<std::string, std::string>> cases = {
      {WithMetadata("Enum n65 { off = 1'b0; } DataInPort n61;",
                    R"([["n65","e",0,"Enum",false,null,[],false],
                        ["n61","a",0,"DataInPort",false,
                         ["n65","0",false,0],[],false]])"),
       "non-signal reset reference"},
      {WithMetadata("DataOutPort n61 { Source 1'bx; }",
                    R"([["n61","a",0,"DataOutPort",false,null,
                        [["Source",0,1,false,false],
                         ["Source",0,2,false,false]],false]])"),
       "duplicate virtual connection"},
      {WithMetadata("DataOutPort n61 { Source 1'bx; }",
                    R"([["n61","a",0,"DataOutPort",false,null,
                        [["Source",0,0,false,false]],false]])"),
       "empty special connection"},
      {WithMetadata("ResetPort n61;",
                    R"([["n61","a",0,"ResetPort",false,null,[],true]])"),
       "polarity contradicts ICL"},
      {WithMetadata("ScanInPort n61; ScanRegister n72 { ScanInSource n61; "
                    "ResetValue 0; }",
                    R"([["n61","a",0,"ScanInPort",false,null,[],false],
                        ["n72","r",0,"ScanRegister",false,null,[],false]])"),
       "missing register reset association"},
      {"Module Top { DataInPort a { Attribute " +
           std::string(pattern::kFlatOriginAttribute) + " = \"ignored\"; } }",
       "reserved metadata on a node"}};
  std::string failures;
  for (const auto& item : cases) {
    try {
      pattern::Design design;
      design.AddIcl("malformed.icl", item.first);
      pattern::Elaborate(design, "Top");
      failures += item.second + "; ";
    } catch (const std::exception& error) {
      std::cout << "Rejected " << item.second << ": " << error.what() << '\n';
    }
  }
  Require(failures.empty(), "accepted malformed metadata: " + failures);
}

void TestScanInterfaces() {
  // IEEE 1687-2014 ScanInterface parallel-client example, completed with TDRs.
  pattern::Design design;
  design.AddIcl("interfaces.icl", R"(
Module Leaf {
  ScanInPort si1;
  ScanInPort si2;
  SelectPort sel;
  ShiftEnPort shift;
  ScanRegister r1[7:0] { ScanInSource si1; }
  ScanRegister r2[5:0] { ScanInSource si2; }
  ScanOutPort so1 { Source r1[0]; }
  ScanOutPort so2 { Source r2[0]; }
  ScanInterface bus {
    Port sel;
    Chain c1 { Attribute first; Port si1; Port so1; DefaultLoadValue 8'hda; }
    Chain c2 { Port si2; Port so2; DefaultLoadValue 6'b001001; }
  }
}
Module Top {
  ScanInPort si;
  ShiftEnPort shift;
  SelectPort sel;
  Instance a Of Leaf {
    InputPort si1 = si; InputPort si2 = si;
    InputPort shift = shift; InputPort sel = sel;
  }
}
)");
  const auto original = pattern::Elaborate(design, "Top");
  const auto text = pattern::SerializeIcl(original);
  pattern::Design flat_design;
  flat_design.AddIcl("flat.icl", text);
  const auto flat = pattern::Elaborate(flat_design, "Top");
  Require(pattern::SerializeIcl(flat) == text,
          "interface chains and implicit controls replay");
  const auto& interface = *flat.NodeAt("a.bus").scan_interface;
  Require(interface.kind == pattern::ScanInterfaceKind::kClient &&
              interface.ports.size() == 2 && interface.chains.size() == 2 &&
              interface.chains[0].default_load->data == "11011010" &&
              interface.chains[1].default_load->data == "001001" &&
              interface.chains[0].attributes.count("first"),
          "parallel chains preserve controls, values and attributes");
}
}  // namespace

int main() {
  try {
    TestHierarchyRoundTrip();
    TestPropertiesAndNames();
    TestInvalidMetadata();
    TestUnconnectedInputs();
    TestNamespaceBinding();
    TestIndependentResetDomains();
    TestScanInterfaces();
    std::cout << "PASS: canonical flat ICL and original PDL binding\n";
  } catch (const std::exception& error) {
    std::cerr << "FAIL: " << error.what() << '\n';
    return 1;
  }
}
