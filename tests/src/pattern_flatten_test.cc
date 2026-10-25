// Copyright 2026 The ICL Toolsets Authors.
// SPDX-License-Identifier: MIT
#include "pattern_flatten.h"

#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include "pattern_logic.h"
#include "pattern_runtime.h"
#include "text_input.h"

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

std::string WithMetadata(const std::string& body, const std::string& nodes,
                         const std::string& scopes = R"([["","Top",[],[]]])",
                         const std::string& contributors = "") {
  const std::string json =
      "{\"version\":1,\"modules\":[[\"Top\",[]]],"
      "\"scopes\":" +
      scopes + ",\"nodes\":" + nodes +
      (contributors.empty() ? ""
                            : ",\"instance_contributors\":" + contributors) +
      "}";
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
void TestInstanceMetadataValidation() {
  const std::string root = R"(["","Top",[],[]])";
  const auto load = [&](const std::string& scopes) {
    pattern::Design design;
    design.AddIcl("metadata.icl", WithMetadata("", "[]", scopes));
    return pattern::Elaborate(design, "Top");
  };
  const auto old = load("[" + root + "]");
  Require(old.scopes[0].instance_attributes.empty(),
          "earlier four-field scope metadata remains readable");
  const auto addressed =
      load("[" + root + R"(,["child","Top",[],[],[],[false,true,"1000"]]])");
  Require(addressed.scopes[1].address_value &&
              addressed.scopes[1].address_value->data == "1000",
          "numeric address metadata retains its value");
  const auto text = pattern::SerializeIcl(addressed);
  pattern::Design replay;
  replay.AddIcl("replay.icl", text);
  Require(pattern::SerializeIcl(pattern::Elaborate(replay, "Top")) == text,
          "address metadata replay is canonical");
  const std::vector<std::pair<std::string, std::string>> invalid = {
      {R"([["","Top",[],[],[],null]])", "root cannot"},
      {R"([["","Top",[],[],[]]])", "scope fields"},
      {"[" + root + R"(,["child","Top",[],[],[["a",null],["a",null]],null]])",
       "scope attribute"},
      {"[" + root + R"(,["child","Top",[],[],[["bad.name",null]],null]])",
       "scope attribute"},
      {"[" + root + R"(,["child","Top",[],[],[],[true,false,"8"]]])",
       "must be numeric"},
      {"[" + root + R"(,["child","Top",[],[],[],[false,true,"2"]]])",
       "parameter value"}};
  for (const auto& item : invalid) {
    bool rejected = false;
    try {
      load(item.first);
    } catch (const std::runtime_error& error) {
      rejected = true;
      Require(std::string(error.what()).find(item.second) != std::string::npos,
              "unexpected metadata error: " + std::string(error.what()));
    }
    Require(rejected, "malformed instance metadata must be rejected");
  }
}

void TestInstanceAttributeReplay() {
  pattern::Design design;
  design.AddIcl("instance_attributes.icl", R"(
Module Leaf {
  Parameter VALUE = 3;
  Attribute label = "module";
  DataOutPort result { Source 1'b0; }
}
Module Top {
  Parameter VALUE = 9;
  Instance child Of Leaf {
    Attribute label = "instance";
    Attribute enclosing = $VALUE;
    Attribute flag;
  }
  Instance sibling Of Leaf;
}
)");
  const auto original = pattern::Elaborate(design, "Top");
  const auto text = pattern::SerializeIcl(original);
  pattern::Design flat_design;
  flat_design.AddIcl("flat.icl", text);
  const auto flat = pattern::Elaborate(flat_design, "Top");
  const auto& child = flat.scopes.at(flat.NodeAt("child.result").scope);
  const auto& sibling = flat.scopes.at(flat.NodeAt("sibling.result").scope);
  Require(
      child.attributes.at("label")->data == "module" &&
          child.instance_attributes.at("label")->data == "instance" &&
          child.instance_attributes.at("enclosing")->AsInteger() == 9 &&
          !child.instance_attributes.at("flag") &&
          sibling.instance_attributes.empty(),
      "flat replay preserves per-instance attributes and parent evaluation");
  Require(pattern::SerializeIcl(flat) == text,
          "instance attribute metadata is byte stable");
}

void TestOneHotScanGroups() {
  // Completed/cascaded IEEE 1687-2014 6.4.14 example with vector directions.
  pattern::Design design;
  design.AddIcl("one_hot_scan.icl", R"(
Module Leaf {
  ScanInPort si[1:0];
  DataInPort select;
  ScanOutPort so[1:0] { Source si; Enable select; }
}

Module Top {
  ScanInPort si[1:0];
  DataInPort selects[1:0];
  Instance a Of Leaf { InputPort si = si; InputPort select = selects[0]; }
  Instance b Of Leaf { InputPort si = si; InputPort select = selects[1]; }
  OneHotScanGroup group[1:0] { Port a.so; Port ~b.so; }
  Alias reversed[0:1] = group[0:1];
  OneHotScanGroup nested[0:1] { Port reversed; }
  ScanOutPort so[0:1] { Source nested; }
}
)");
  const auto original = pattern::Elaborate(design, "Top");
  const auto text = pattern::SerializeIcl(original);
  pattern::Design flat_design;
  flat_design.AddIcl("flat.icl", text);
  const auto flat = pattern::Elaborate(flat_design, "Top");
  Require(pattern::SerializeIcl(flat) == text,
          "cascaded one-hot scan groups replay with inverted vector sources");
  const auto& sources = flat.NodeAt("group").one_hot_sources;
  Require(sources.size() == 2 && sources[0].bits.size() == 2 &&
              flat.nodes[sources[0].bits[0].node].name == "a.so" &&
              sources[0].bits[0].index == 1 && sources[0].bits[1].index == 0 &&
              sources[1].bits[0].inverted && sources[1].bits[1].inverted,
          "all contributors, bit order and inversions survive flattening");
}
void TestAddressedGroupReplay() {
  const std::string path =
      "tests/cases/ours/pattern_generation/addressed_contributors.icl";
  std::ifstream input(path);
  std::string source;
  Require(input.good() && toolsets::ReadTextStream(input, &source),
          "read addressed contributor fixture");
  pattern::Design design;
  design.AddIcl(path, source);
  for (const std::string top :
       {"RegisterContribution", "InstanceContribution"}) {
    const auto original = pattern::Elaborate(design, top);
    Require(original.NodeAt("wide").one_hot_sources.size() == 1,
            "addressed contributors must have real bit connections");
    const auto text = pattern::SerializeIcl(original);
    pattern::Design replay;
    replay.AddIcl("flat.icl", text);
    const auto flat = pattern::Elaborate(replay, top);
    Require(pattern::SerializeIcl(flat) == text,
            "addressed contributors replay byte identically");
    const std::string name = top == "RegisterContribution" ? "r" : "child.r";
    Require(flat.NodeAt(name).constants.at("AddressValue").data == "1",
            "register address survives flat replay");
    Require(flat.NodeAt(name).inputs.count("WriteDataSource"),
            "addressed register has an implicit write-data source");
    const auto& write_data = flat.NodeAt(name).inputs.at("WriteDataSource");
    Require(write_data.size() == 8 && write_data.front().index == 7 &&
                write_data.back().index == 0 &&
                flat.nodes[write_data[0].node].name ==
                    (top == "RegisterContribution" ? "data" : "child.data"),
            "addressed register consumes its own module logical data bus");
    Require(flat.NodeAt("wide").OneHotSourceBit(0, 0).constant == 'x' &&
                flat.NodeAt("wide").OneHotSourceBit(0, 15).index == 0,
            "addressed contributors preserve padding and bit order");
    pattern::Runtime before(design, original), after(replay, flat);
    const std::string pdl = "iWrite " + name + " 0x96\niApply\n";
    before.Execute("source.pdl", pdl);
    after.Execute("source.pdl", pdl);
    before.Finish();
    after.Finish();
    Require(
        before.operations().size() == after.operations().size() &&
            before.operations()[0].arguments == after.operations()[0].arguments,
        "original PDL targets still bind on the flat addressed netlist");
  }
  const auto original = pattern::Elaborate(design, "MixedContribution");
  const auto text = pattern::SerializeIcl(original);
  pattern::Design replay;
  replay.AddIcl("mixed_flat.icl", text);
  Require(pattern::SerializeIcl(
              pattern::Elaborate(replay, "MixedContribution")) == text,
          "width-sorted nested registers replay canonically");
  const auto split = pattern::Elaborate(design, "SplitContribution");
  const auto split_text = pattern::SerializeIcl(split);
  pattern::Design split_replay;
  split_replay.AddIcl("split_flat.icl", split_text);
  const auto flat_split = pattern::Elaborate(split_replay, "SplitContribution");
  Require(pattern::SerializeIcl(flat_split) == split_text,
          "mixed register/instance contributors replay canonically");
  const auto& bits = flat_split.NodeAt("child.r").inputs.at("WriteDataSource");
  Require(bits.size() == 8 &&
              bits[0].node == flat_split.node_ids.at("child.zfirst") &&
              bits[0].index == 3 && bits[3].index == 0 &&
              bits[4].node == flat_split.node_ids.at("child.asecond") &&
              bits[4].index == 0 && bits[7].index == 3,
          "register loads rightmost bits across declaration-ordered ports");
  const auto& outputs = flat_split.NodeAt("wide").one_hot_sources[1];
  Require(outputs.kind == pattern::OneHotSourceKind::kInstance &&
              outputs.bits.size() == 8 &&
              outputs.bits[0].node == flat_split.node_ids.at("child.zupper") &&
              outputs.bits[0].index == 3 &&
              outputs.bits[4].node == flat_split.node_ids.at("child.alower") &&
              outputs.bits[4].index == 0 && outputs.bits[7].index == 3,
          "instance contributes logicalDataOut in declaration order");
}

void TestInstanceContributorMetadata() {
  const std::string scopes = R"([["","Top",[],[]],["child","Top",[],[]]])";
  const std::string nodes =
      R"([["n67","g",0,"OneHotDataGroup",false,null,[],false],
          ["n6368696c642e6f","child.o",1,"DataOutPort",false,null,[],false]])";
  const std::string body =
      "DataOutPort n6368696c642e6f[1:0] { Source 2'b00; Enable 1'b1; } "
      "OneHotDataGroup n67[1:0] { Port n6368696c642e6f; }";
  for (const std::string contributors :
       {R"([["g",0,""]])", R"([["g",1,"child"]])",
        R"([["g",0,"child"],["g",0,"child"]])", R"([["child.o",0,"child"]])",
        R"([["g",0,"missing"]])"}) {
    bool rejected = false;
    try {
      pattern::Design design;
      design.AddIcl("invalid_origin.icl",
                    WithMetadata(body, nodes, scopes, contributors));
      pattern::Elaborate(design, "Top");
    } catch (const std::exception&) {
      rejected = true;
    }
    Require(rejected, "invalid instance contributor metadata is rejected");
  }
  for (const std::string reference :
       {"~n6368696c642e6f", "n6368696c642e6f[0:1]", "n6368696c642e6f[0]"}) {
    bool rejected = false;
    try {
      pattern::Design design;
      design.AddIcl(
          "mismatched_origin.icl",
          WithMetadata(
              "DataOutPort n6368696c642e6f[1:0] { Source 2'b00; Enable 1'b1; } "
              "OneHotDataGroup n67[1:0] { Port " +
                  reference + "; }",
              nodes, scopes, R"([["g",0,"child"]])"));
      pattern::Elaborate(design, "Top");
    } catch (const std::exception& error) {
      rejected = std::string(error.what()).find("contributor conflicts") !=
                 std::string::npos;
    }
    Require(rejected,
            "flat instance contribution must match complete logicalDataOut");
  }
}

void TestFlatAddressBudget() {
  std::string scopes = R"([["","Top",[],[]]])";
  scopes.pop_back();
  const std::string address(1048576, '0');
  for (size_t i = 0; i < 5; ++i) {
    scopes += ",[\"c" + std::to_string(i) +
              "\",\"Top\",[],[],[],[false,true,\"" + address + "\"]]";
  }
  scopes += "]";
  bool rejected = false;
  try {
    pattern::Design design;
    design.AddIcl("flat_address_budget.icl", WithMetadata("", "[]", scopes));
    pattern::Elaborate(design, "Top");
  } catch (const std::exception& error) {
    rejected = std::string(error.what()).find("address constant bit limit") !=
               std::string::npos;
  }
  Require(rejected, "flat metadata cannot bypass retained address limits");
}

void TestOneHotDataGroups() {
  pattern::Design design;
  design.AddIcl("one_hot_data.icl", R"(
Module Leaf {
  DataInPort d[7:0]; DataInPort en;
  DataOutPort raw[7:0] { Source d; Enable en; }
  DataOutPort forwarded[7:0] { Source raw; }
}
Module Top {
  DataInPort d[7:0]; DataInPort en;
  Instance child Of Leaf { InputPort d = d; InputPort en = en; }
  OneHotDataGroup g[15:0] { Port child.raw[3]; Port child.forwarded; Port ~child.raw[0:7]; }
  OneHotDataGroup cascaded[0:31] { Port g; }
  DataOutPort observed[0:31] { Source cascaded; }
}
)");
  const auto original = pattern::Elaborate(design, "Top");
  const auto text = pattern::SerializeIcl(original);
  pattern::Design replay;
  replay.AddIcl("flat.icl", text);
  const auto flat = pattern::Elaborate(replay, "Top");
  Require(pattern::SerializeIcl(flat) == text,
          "one-hot data groups replay byte identically");
  const auto& sources = flat.NodeAt("g").one_hot_sources;
  bool scalar = false, forwarded = false, reversed = false;
  for (const auto& contributor : sources) {
    const auto& source = contributor.bits;
    const auto name = flat.nodes[source[0].node].name;
    scalar |= source.size() == 1 && name == "child.raw" && source[0].index == 3;
    forwarded |= source.size() == 8 && name == "child.forwarded";
    reversed |= source.size() == 8 && name == "child.raw" &&
                source[0].inverted && source[0].index == 0;
  }
  Require(sources.size() == 3 && scalar && forwarded && reversed &&
              flat.NodeAt("cascaded").one_hot_sources[0].bits.size() == 16,
          "short source widths, inversions and cascades survive flattening");
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
    TestOneHotScanGroups();
    TestOneHotDataGroups();
    TestAddressedGroupReplay();
    TestInstanceContributorMetadata();
    TestFlatAddressBudget();
    for (const std::string member :
         {"DataRegister r;", "Instance child Of Leaf;",
          "Instance child Of Leaf { AddressValue 0; }"}) {
      pattern::Design design;
      design.AddIcl(
          "addressed_group.icl",
          "Module Leaf {} Module M { OneHotDataGroup g { " + member + " } }");
      bool rejected = false;
      try {
        const auto netlist = pattern::Elaborate(design, "M");
        pattern::SerializeIcl(netlist);
      } catch (const std::runtime_error& error) {
        rejected = std::string(error.what()).find("addressed one-hot") !=
                   std::string::npos;
      }
      Require(rejected,
              "unsupported addressed members must not disappear from flat ICL");
    }
    TestInstanceAttributeReplay();
    TestInstanceMetadataValidation();
    std::cout << "PASS: canonical flat ICL and original PDL binding\n";
  } catch (const std::exception& error) {
    std::cerr << "FAIL: " << error.what() << '\n';
    return 1;
  }
}
