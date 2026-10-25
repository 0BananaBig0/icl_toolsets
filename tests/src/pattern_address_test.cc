// Copyright 2026 The ICL Toolsets Authors.
// SPDX-License-Identifier: MIT
#include "pattern_address.h"

#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>

#include "pattern_flatten.h"
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
void Load(pattern::Design* design, const std::string& name) {
  const auto path =
      "tests/cases/ours/pattern_generation/address_domains/" + name + ".icl";
  std::ifstream input(path);
  std::string text;
  Require(input.good() && toolsets::ReadTextStream(input, &text), path);
  design->AddIcl(path, text);
}
void TestDomains() {
  pattern::Design design;
  Load(&design, "leaf");
  Load(&design, "scan_boundary");
  Load(&design, "explicit_parent");
  Load(&design, "wide_zero");
  for (const auto& top :
       {"ScanAddressBoundary", "ExplicitParentAddress", "WideZeroAddress"}) {
    auto netlist = pattern::Elaborate(design, top);
    for (int replay = 0; replay < 2; ++replay) {
      const bool scan = std::string(top) == "ScanAddressBoundary";
      const bool direct = std::string(top) == "WideZeroAddress";
      const std::string target = direct ? "r"
                                 : scan ? "bank.child.r"
                                        : "child.r";
      pattern::AddressResolver resolver(netlist);
      auto write = resolver.Resolve(target, pattern::AddressOperation::kWrite);
      auto read = resolver.Resolve(target, pattern::AddressOperation::kRead);
      Require(write.value.data == (direct ? "1" : "101"), "computed address");
      Require(write.signals == read.signals,
              "share cached logical address bus");
      Require(write.signals->size() == (direct ? 2 : 8), "effective bus width");
      Require(netlist.nodes[write.signals->front().node].name ==
                  (scan ? "control" : "address"),
              "effective address source");
      Require(write.signals->front().index == (direct ? 1 : 7) &&
                  write.signals->back().index == 0,
              "address bit order");
      Require(
          netlist.nodes[write.enable.node].name == (scan ? "control" : "we") &&
              write.enable.index == (scan ? 8 : 0),
          "write enable source");
      Require(
          netlist.nodes[read.enable.node].name == (scan ? "control" : "re") &&
              read.enable.index == (scan ? 9 : 0),
          "read enable source");
      const auto flat = pattern::SerializeIcl(netlist);
      pattern::Design replay_design;
      replay_design.AddIcl("flat.icl", flat);
      netlist = pattern::Elaborate(replay_design, top);
      Require(pattern::SerializeIcl(netlist) == flat,
              "canonical address replay");
    }
  }
}
void TestNestedAndWide() {
  pattern::Design design;
  design.AddIcl("nested.icl", R"(
Module Leaf {
  AddressPort a[1:0]; DataInPort d; WriteEnPort we; ReadEnPort re;
  DataOutPort q { Source g; }
  OneHotDataGroup g { DataRegister r { AddressValue 1; } }
}
Module Bank {
  AddressPort a[3:0]; DataInPort d; WriteEnPort we; ReadEnPort re;
  DataOutPort q { Source g; }
  OneHotDataGroup g { Instance leaf Of Leaf { AddressValue 3; } }
}
Module Top {
  AddressPort zfirst[1:0]; AddressPort asecond[0:69];
  DataInPort d; WriteEnPort we; ReadEnPort re;
  OneHotDataGroup g { Instance bank Of Bank {
    AddressValue 72'h800000000000000000;
  } }
})");
  auto netlist = pattern::Elaborate(design, "Top");
  for (int replay = 0; replay < 2; ++replay) {
    pattern::AddressResolver resolver(netlist);
    const auto result =
        resolver.Resolve("bank.leaf.r", pattern::AddressOperation::kWrite);
    Require(result.value.data == "1" + std::string(68, '0') + "100",
            "arbitrary-width addition and carry, not signed 64-bit truncation");
    Require(
        result.signals->size() == 72 &&
            netlist.nodes[result.signals->at(0).node].name == "zfirst" &&
            result.signals->at(0).index == 1 &&
            result.signals->at(1).index == 0 &&
            netlist.nodes[result.signals->at(2).node].name == "asecond" &&
            result.signals->at(2).index == 0 &&
            result.signals->back().index == 69,
        "logicalAddress follows declaration order, not names or numeric order");
    pattern::Design flat;
    flat.AddIcl("flat.icl", pattern::SerializeIcl(netlist));
    netlist = pattern::Elaborate(flat, "Top");
  }
}
void TestDisconnectedAndInvalid() {
  const std::string leaf = R"(
Module Leaf {
  AddressPort a[1:0]; DataInPort d; WriteEnPort we; ReadEnPort re;
  OneHotDataGroup g { DataRegister r { AddressValue 1; } }
})";
  const auto check = [&](const std::string& declarations,
                         const std::string& binding,
                         const std::string& expected) {
    pattern::Design design;
    design.AddIcl(
        "invalid.icl",
        leaf + "Module Top { " + declarations +
            " DataInPort d; Instance child Of Leaf { InputPort d = d; " +
            binding + " } }");
    const auto netlist = pattern::Elaborate(design, "Top");
    pattern::AddressResolver resolver(netlist);
    // Retry exercises cache state left by a failed query.
    for (int attempt = 0; attempt < 2; ++attempt) {
      Fails(
          [&] {
            resolver.Resolve("child.r", pattern::AddressOperation::kWrite);
          },
          expected);
    }
  };
  check("", "", "child.a: disconnected AddressPort");
  check("AddressPort a[1:0];", "InputPort a = a;", "child.we: disconnected");
  check("AddressPort a[1:0]; WriteEnPort we;",
        "InputPort a = a[0], a[1]; InputPort we = we;",
        "partial or reordered parent logicalAddress mapping");
  check("AddressPort a[1:0]; WriteEnPort we;",
        "InputPort a = ~a; InputPort we = we;",
        "inverted address/control source");
  check("AddressPort a[1:0]; WriteEnPort we;",
        "InputPort a[0] = a[0]; InputPort we = we;",
        "disconnected or constant");
  check("DataInPort a[1:0]; DataInPort we; LogicSignal logic { a[0]; }",
        "InputPort a = logic, logic; InputPort we = we;",
        "intervening address/control logic");
  const std::string scans =
      "ScanInPort si; ScanRegister a[1:0] { ScanInSource si; } "
      "ScanRegister b { ScanInSource a[0]; } ";
  check(scans, "InputPort a = a; InputPort we = b;",
        "must share primary inputs or one ScanRegister");
  check(scans, "InputPort a = a[0], b; InputPort we = b;",
        "span multiple ScanRegisters");
  check(scans, "InputPort a = a[0], d; InputPort we = b;",
        "mix primary inputs and ScanRegister");
  check(scans + "WriteEnPort we;", "InputPort a = a; InputPort we = we;",
        "must share primary inputs or one ScanRegister");
  check(scans + "AddressPort addr[1:0];",
        "InputPort a = addr; InputPort we = b;",
        "must share primary inputs or one ScanRegister");

  pattern::Design design;
  design.AddIcl("unknown.icl", R"(
Module Top { AddressPort a; DataInPort d; WriteEnPort we;
  OneHotDataGroup g { DataRegister r { AddressValue 1'bx; } }
})");
  auto netlist = pattern::Elaborate(design, "Top");
  pattern::AddressResolver resolver(netlist);
  Fails([&] { resolver.Resolve("r", pattern::AddressOperation::kWrite); },
        "known nonnegative number");
}
void TestPrimaryInputDomain() {
  pattern::Design design;
  Load(&design, "leaf");
  design.AddIcl("primary.icl", R"(
Module Top {
  DataInPort address[0:3]; DataInPort data[7:0];
  DataInPort write_enable; DataInPort read_enable;
  Instance child Of AddressDomainLeaf {
    InputPort a = address; InputPort d = data;
    InputPort we = write_enable; InputPort re = read_enable;
  }
})");
  const auto netlist = pattern::Elaborate(design, "Top");
  pattern::AddressResolver resolver(netlist);
  const auto write =
      resolver.Resolve("child.r", pattern::AddressOperation::kWrite);
  const auto read =
      resolver.Resolve("child.r", pattern::AddressOperation::kRead);
  Require(write.signals == read.signals && write.signals->front().index == 0 &&
              write.signals->back().index == 3 &&
              write.value.AsInteger() == 1 &&
              write.enable.node == netlist.node_ids.at("write_enable") &&
              read.enable.node == netlist.node_ids.at("read_enable"),
          "address and controls sourced by top-level DataInPorts");
}
void TestSharedWideBus() {
  constexpr size_t kInstances = 2000;
  std::string source = R"(
Module Leaf { AddressPort a; DataInPort d; WriteEnPort we;
  OneHotDataGroup g { DataRegister r { AddressValue 0; } }
}
Module Top { AddressPort a[4095:0]; DataInPort d; WriteEnPort we; OneHotDataGroup g {
)";
  for (size_t i = 0; i < kInstances; ++i) {
    source += "Instance i" + std::to_string(i) + " Of Leaf { AddressValue " +
              std::to_string(i) + "; }";
  }
  source += "} }";
  pattern::Design design;
  design.AddIcl("wide.icl", source);
  const auto netlist = pattern::Elaborate(design, "Top");
  pattern::AddressResolver resolver(netlist);
  const auto first =
      resolver.Resolve("i0.r", pattern::AddressOperation::kWrite);
  for (size_t i = 0; i < kInstances; ++i) {
    const auto result = resolver.Resolve("i" + std::to_string(i) + ".r",
                                         pattern::AddressOperation::kWrite);
    Require(
        result.signals == first.signals &&
            result.value.AsInteger() == static_cast<int64_t>(i),
        "siblings share wide address storage and retain independent offsets");
  }
}
void TestUnrelatedWideParent() {
  constexpr size_t kInstances = 10000;
  std::string source = R"(
Module Leaf { AddressPort a[1:0]; DataInPort d; WriteEnPort we;
  OneHotDataGroup g { DataRegister r { AddressValue 1; } }
}
Module Top {
  AddressPort unrelated[1048575:0]; DataInPort d;
  ScanInPort si; ScanRegister control[2:0] { ScanInSource si; }
)";
  for (size_t i = 0; i < kInstances; ++i) {
    source += "Instance i" + std::to_string(i) +
              " Of Leaf { "
              "InputPort a = control[2:1]; InputPort we = control[0]; "
              "InputPort d = d; }";
  }
  source += "}";
  pattern::Design design;
  design.AddIcl("unrelated.icl", source);
  const auto netlist = pattern::Elaborate(design, "Top");
  pattern::AddressResolver resolver(netlist);
  for (size_t i = 0; i < kInstances; ++i) {
    const auto result = resolver.Resolve("i" + std::to_string(i) + ".r",
                                         pattern::AddressOperation::kWrite);
    Require(
        result.signals->size() == 2 && result.value.AsInteger() == 1,
        "scan-driven bank ignores unrelated million-bit parent address bus");
  }
}
void TestAliasDomain() {
  for (const bool explicit_bank : {false, true}) {
    pattern::Design design;
    std::string source = R"(
Module Leaf {
  AddressPort a[7:0]; DataInPort d; WriteEnPort we;
  DataOutPort q { Source g; }
  OneHotDataGroup g { DataRegister r { AddressValue 1; } }
}
Module Bank {
  AddressPort a[7:0]; DataInPort d; WriteEnPort we;
  Alias address_alias[7:0] = a;
  Alias enable_alias = we;
  DataOutPort q { Source g; }
  OneHotDataGroup g { Instance leaf Of Leaf {
    AddressValue 2; InputPort a = address_alias; InputPort we = enable_alias;
  } }
}
Module Top { AddressPort a[7:0]; DataInPort d; WriteEnPort we;
  DataOutPort q { Source g; }
  OneHotDataGroup g { Instance bank Of Bank { AddressValue 4;
)";
    if (explicit_bank) source += "InputPort a = a;";
    source += "} } }";
    design.AddIcl("alias.icl", source);
    auto netlist = pattern::Elaborate(design, "Top");
    for (int replay = 0; replay < 2; ++replay) {
      pattern::AddressResolver resolver(netlist);
      Require(resolver.Resolve("bank.leaf.r", pattern::AddressOperation::kWrite)
                      .value.AsInteger() == 7,
              "aliases preserve ancestor offsets, 4 + 2 + 1");
      pattern::Design flat;
      flat.AddIcl("flat.icl", pattern::SerializeIcl(netlist));
      netlist = pattern::Elaborate(flat, "Top");
    }
  }
}
void TestGatedSource() {
  pattern::Design design;
  design.AddIcl("gated.icl", R"(
Module Buffer {
  DataInPort d[1:0]; DataInPort en;
  DataOutPort q[1:0] { Source d; Enable en; }
}
Module Leaf {
  AddressPort a[1:0]; DataInPort d; WriteEnPort we;
  OneHotDataGroup g { DataRegister r { AddressValue 1; } }
}
Module Top {
  ScanInPort si;
  ScanRegister ctl[2:0] { ScanInSource si; }
  DataInPort d;
  Instance buffer Of Buffer { InputPort d = ctl[2:1]; InputPort en = 1'b0; }
  Instance child Of Leaf {
    InputPort a = buffer.q; InputPort we = ctl[0]; InputPort d = d;
  }
})");
  const auto netlist = pattern::Elaborate(design, "Top");
  pattern::AddressResolver resolver(netlist);
  Fails([&] { resolver.Resolve("child.r", pattern::AddressOperation::kWrite); },
        "enabled address/control source requires gating constraints");
}
void TestLimitsAndMissingPorts() {
  const auto check = [&](const std::string& source, const std::string& target,
                         const std::string& expected) {
    pattern::Design design;
    design.AddIcl("limits.icl", source);
    const auto netlist = pattern::Elaborate(design, "Top");
    pattern::AddressResolver resolver(netlist);
    for (int repeat = 0; repeat < 2; ++repeat) {
      Fails(
          [&] { resolver.Resolve(target, pattern::AddressOperation::kWrite); },
          expected);
    }
  };
  const std::string tail =
      "DataInPort d; OneHotDataGroup g { DataRegister r { AddressValue 0; } } "
      "}";
  check("Module Top { " + tail, "r", "missing AddressPort");
  check("Module Top { AddressPort a; " + tail, "r", "expected one WriteEnPort");
  std::string source = "Module Top { ";
  for (int i = 0; i < 5; ++i)
    source += "AddressPort a" + std::to_string(i) + "[1048575:0];";
  check(source + tail, "r", "logicalAddress bit limit exceeded");
  source = R"(
Module Leaf { AddressPort a; DataInPort d; WriteEnPort we;
  OneHotDataGroup g { DataRegister r { AddressValue 0; } }
}
)";
  for (int i = 3; i >= 0; --i) {
    source +=
        "Module Bank" + std::to_string(i) +
        " { AddressPort a; DataInPort d; WriteEnPort we; OneHotDataGroup g { "
        "Instance child Of " +
        (i == 3 ? std::string("Leaf") : "Bank" + std::to_string(i + 1)) +
        " { AddressValue 0; } } }";
  }
  source +=
      "Module Top { AddressPort a[1048575:0]; DataInPort d; WriteEnPort we; "
      "OneHotDataGroup g { Instance child Of Bank0 { "
      "AddressValue 1048576'b1" +
      std::string(1048575, '0') + "; } } }";
  check(source, "child.child.child.child.child.r",
        "resolved address bit limit exceeded");
}
void TestFailures() {
  pattern::Design design;
  Load(&design, "leaf");
  Load(&design, "overflow");
  auto netlist = pattern::Elaborate(design, "AddressOverflow");
  pattern::AddressResolver resolver(netlist);
  Fails([&] { resolver.Resolve("child.r", pattern::AddressOperation::kWrite); },
        "computed address exceeds logicalAddress width");
  Fails([&] { resolver.Resolve("child.r", pattern::AddressOperation::kRead); },
        "computed address exceeds logicalAddress width");
  Fails([&] { resolver.Resolve("data", pattern::AddressOperation::kWrite); },
        "not an addressable DataRegister");
}
}  // namespace
int main() {
  try {
    TestUnrelatedWideParent();
    TestAliasDomain();
    TestDomains();
    TestFailures();
    TestNestedAndWide();
    TestDisconnectedAndInvalid();
    TestPrimaryInputDomain();
    TestSharedWideBus();
    TestGatedSource();
    TestLimitsAndMissingPorts();
    std::cout << "pattern address tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
