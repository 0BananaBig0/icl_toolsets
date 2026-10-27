// Copyright 2026 The ICL Toolsets Authors.
// SPDX-License-Identifier: MIT
#include "pattern/pattern_design.h"

#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>

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
            "expected '" + expected + "', got: " + error.what());
    return;
  }
  throw std::runtime_error("expected failure: " + expected);
}

void TestOrderedOverrides() {
  pattern::Design design;
  design.AddIcl("first.icl", "Module M { DataInPort before; }");
  const auto old = design.modules().at("M");
  design.AddIcl("second.icl", "Module M { DataInPort after; }");
  Require(design.modules().size() == 1, "replacement must not add a module");
  Require(design.modules().at("M").document->source().find("after") !=
              std::string::npos,
          "later definition must win");
  Require(old.document->source().find("before") != std::string::npos,
          "retained definition must own its original shared tree");
  Require(design.warnings().size() == 1, "one replacement warning");
  Require(design.warnings()[0].find("first.icl:1:") != std::string::npos &&
              design.warnings()[0].find("second.icl:1:") != std::string::npos,
          "warning identifies both definitions");
  design.AddIcl("third.icl", "NameSpace vendor; Module M {}");
  Require(design.modules().size() == 2 && design.warnings().size() == 1,
          "different namespaces do not override");
  Fails([&] { design.SelectTop("M"); }, "ambiguous");
  Require(design.SelectTop("::M").name() == "M", "explicit default namespace");
  Require(design.SelectTop("vendor::M").name() == "vendor::M",
          "explicit named namespace");
}

void TestTopAfterReplacement() {
  pattern::Design design;
  design.AddIcl("original.icl",
                "Module Old {} Module Top { Instance a Of Old; }");
  design.AddIcl("later.icl", "Module Top {} Module Old { Instance a Of Top; }");
  Require(design.SelectTop("").name() == "Old", "use replacement dependencies");
  Require(design.warnings().back().find("Old") != std::string::npos,
          "warn about inferred top");
}

void TestNamespaces() {
  pattern::Design design;
  design.AddIcl("library.icl", "NameSpace vendor; Module Leaf {}");
  design.AddIcl("top.icl",
                "UseNameSpace vendor; Module Top { Instance a Of Leaf; }");
  Require(design.SelectTop("").name() == "Top", "file namespace resolution");
  const auto& top = design.modules().at("Top");
  Require(top.ast != nullptr && top.ast->header() == "Module Top",
          "registry retains existing AST module");
  Require(
      design.ResolveInstance(top, top.instances.at(0)).name() == "vendor::Leaf",
      "resolve instance through existing declaration metadata");
  design.AddIcl("explicit.icl",
                "Module Wrapper { UseNameSpace; Instance a Of ::Top; }");
  Require(design.SelectTop("").name() == "Wrapper", "explicit namespace reset");
}

void TestErrors() {
  pattern::Design empty;
  Fails([&] { empty.SelectTop(""); }, "no modules");
  pattern::Design design;
  design.AddIcl("good.icl", "Module A {} Module B {}");
  Fails([&] { design.SelectTop(""); }, "--top_module");
  Fails([&] { design.SelectTop("Missing"); }, "Missing");
  Fails([&] { design.AddIcl("broken.icl", "Module A { DataInPort ; }"); },
        "broken.icl:1:");
  Require(
      design.modules().at("A").path == "good.icl" && design.warnings().empty(),
      "invalid later file must not replace good definitions");
  pattern::Design missing;
  missing.AddIcl("missing.icl", "Module A { Instance x Of B; }");
  Fails([&] { missing.SelectTop(""); }, "missing.icl:1:");
  Fails([&] { missing.SelectTop("A"); }, "B");
  pattern::Design recursive;
  recursive.AddIcl(
      "recursive.icl",
      "Module A { Instance b Of B; } Module B { Instance a Of A; }");
  Fails([&] { recursive.SelectTop(""); }, "cycle");
  Fails([&] { recursive.SelectTop("A"); }, "cycle");
  pattern::Design reachable;
  reachable.AddIcl(
      "reachable.icl",
      "Module Top { Instance a Of A; } Module A { Instance a Of A; }");
  Fails([&] { reachable.SelectTop(""); }, "cycle");
}
void TestBsdlRegistry() {
  std::ifstream input(
      "tests/cases/ours/pattern_generation/access_link/simple.bsdl");
  std::string text;
  Require(input.good() && toolsets::ReadTextStream(input, &text),
          "read BSDL companion");
  pattern::Design design;
  design.AddIcl("top.icl", "Module Top {}");
  design.AddBsdl("tap.bsdl", text, "accessprobe");
  const auto& tap = design.ResolveBsdl("ACCESSprobe");
  Require(tap.entity == "AccessProbe" && tap.instruction_width == 4,
          "case-insensitive BSDL entity resolution");
  Fails([&] { design.AddBsdl("bad.bsdl", "entity broken is", "AccessProbe"); },
        "bad.bsdl:");
  Require(&tap == &design.ResolveBsdl("AccessProbe"),
          "failed parse preserves prior metadata");
  Fails([&] { design.AddBsdl("duplicate.bsdl", text, "ACCESSPROBE"); },
        "already loaded");
  Require(design.ResolveBsdl("AccessProbe").origin.find("tap.bsdl:") == 0 &&
              design.warnings().empty(),
          "duplicate BSDL does not override or generate ICL warnings");
  Fails([&] { design.ResolveBsdl("Missing"); }, "Missing");
  Require(design.modules().size() == 1 && design.SelectTop("").name() == "Top",
          "BSDL metadata does not participate in ICL top selection");
}
}  // namespace

int main() {
  try {
    TestOrderedOverrides();
    TestTopAfterReplacement();
    TestNamespaces();
    TestErrors();
    TestBsdlRegistry();
    std::cout
        << "PASS: pattern design registry, namespaces, overrides and roots\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << "FAIL: " << error.what() << '\n';
    return 1;
  }
}
