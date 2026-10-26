// Copyright 2026 The ICL Toolsets Authors.
// SPDX-License-Identifier: MIT
#include "pattern_netlist.h"

#include <chrono>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>

#include "text_input.h"

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
  for (const auto test : {TestSpecialization,
                          TestAccessLinkModelInsertion,
                          TestAccessLinkModelErrors,
                          TestSharedAccessLinkModels,
                          TestAddressedDataDefaults,
                          TestAddressedGroupErrors,
                          TestErrors,
                          TestBitConnections,
                          TestResetInference,
                          TestMuxAndEnum,
                          TestConnectionErrors,
                          TestUnsizedSources,
                          TestImplicitScanBit,
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
