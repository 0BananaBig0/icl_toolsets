// Copyright 2026 The ICL Toolsets Authors.
// SPDX-License-Identifier: MIT
#include "pattern/pattern_runtime.h"

#include <iostream>
#include <stdexcept>
#include <string>

#include "pattern/pattern_netlist.h"

namespace {
void Require(bool value, const std::string& message) {
  if (!value) throw std::runtime_error(message);
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
  throw std::runtime_error("expected error: " + expected);
}
pattern::Design MakeDesign() {
  pattern::Design design;
  design.AddIcl("top.icl", R"(
Module Top { DataInPort reg[7:0]; Instance a Of vendor::Child; Instance b Of other::Child; }
NameSpace vendor;
Module Child { DataInPort reg[7:0]; }
NameSpace other;
Module Child { DataInPort reg[7:0]; }
)");
  return design;
}
void TestSequentialDefinitions() {
  const auto design = MakeDesign();
  pattern::Runtime runtime(design, "Top");
  runtime.Execute("first.pdl", R"(iProcsForModule Top
iProc configure {} {iWrite reg 1; iApply}
iCall configure
)");
  runtime.Execute("second.pdl", R"(iProcsForModule Top
iProc configure {} {iWrite reg 2; iApply}
iCall configure
)");
  const auto& ops = runtime.operations();
  Require(ops.size() == 4 && ops[0].arguments.at(1) == "1" &&
              ops[2].arguments.at(1) == "2",
          "replacement affects subsequent calls only");
  Require(runtime.warnings().size() == 1 &&
              runtime.warnings()[0].find("first.pdl") != std::string::npos &&
              runtime.warnings()[0].find("second.pdl") != std::string::npos,
          "warning must identify both definitions");
}
void TestBindingAndPrefixes() {
  const auto design = MakeDesign();
  pattern::Runtime runtime(design, "Top");
  runtime.Execute("bindings.pdl", R"(iProcsForModule vendor::Child
iProc configure {v} {iWrite reg $v; iApply}
iProcsForModule other::Child
iProc configure {v} {iWrite reg $v; iApply}
iProcsForModule vendor::Child -iProcNameSpace special
iProc configure {v} {iWrite reg $v; iApply}
iCall a.configure 1
iCall b.configure 2
iCall a.special::configure 3
iPrefix a
iCall configure 4
iPrefix -
iWrite reg 5
iApply
)");
  const auto& ops = runtime.operations();
  Require(runtime.warnings().empty(), "distinct bindings must not override");
  Require(ops.size() == 10 && ops[0].arguments[0] == "a.reg" &&
              ops[2].arguments[0] == "b.reg" &&
              ops[4].arguments[0] == "a.reg" &&
              ops[6].arguments[0] == "a.reg" && ops[8].arguments[0] == "reg",
          "instance and procedure namespace resolution");
}
void TestTclAndVectorSyntax() {
  const auto design = MakeDesign();
  pattern::Runtime runtime(design, "Top");
  runtime.Execute("loop.pdl", R"(iProcsForModule Top
iProc run {{count 3}} {
  for {set i 0} {$i < $count} {incr i} {
    iWrite reg[$i] [expr {$i % 2}]
    iApply
  }
}
iCall run
)");
  const auto& ops = runtime.operations();
  Require(ops.size() == 6, "Tcl loop expands into six operations");
  for (size_t i = 0; i < 3; ++i) {
    Require(ops[2 * i].arguments[0] == "reg[" + std::to_string(i) + "]",
            "PDL vector index must not execute as a Tcl command");
    Require(ops[2 * i].arguments[1] == std::to_string(i % 2),
            "ordinary Tcl substitution executes");
    Require(ops[2 * i].origin.find("loop.pdl:4:") != std::string::npos &&
                !ops[2 * i].call_stack.empty(),
            "nested source and call stack");
  }
}
void TestErrorsAndSafety() {
  const auto design = MakeDesign();
  pattern::Runtime unknown(design, "Top");
  Fails([&] { unknown.Execute("unknown.pdl", "iCall absent\n"); }, "absent");
  pattern::Runtime readback(design, "Top");
  Fails([&] { readback.Execute("read.pdl", "set v [iGetReadData reg]\n"); },
        "hardware readback");
  pattern::Runtime unsafe(design, "Top");
  Fails([&] { unsafe.Execute("unsafe.pdl", "exec echo unsafe\n"); }, "exec");
  for (const auto& command : {"after 1", "update", "interp create"}) {
    pattern::Runtime blocking(design, "Top");
    Fails([&] { blocking.Execute("blocking.pdl", command); },
          "invalid command");
  }
  pattern::Runtime event_loop(design, "Top");
  event_loop.Execute("blocking.pdl", "iNote [llength [info commands vwait]]");
  Require(event_loop.operations()[0].arguments[0] == "0",
          "unbounded vwait is unavailable in the generation interpreter");
  pattern::Runtime pending(design, "Top");
  Fails([&] { pending.Execute("pending.pdl", "iWrite reg 1\niRunLoop 2\n"); },
        "unapplied");
  pattern::Runtime stack(design, "Top");
  Fails(
      [&] {
        stack.Execute(
            "stack.pdl",
            "iProcsForModule Top\niProc run {} {iCall absent}\niCall run\n");
      },
      "iCall run");
}

void TestRuntimeValidationAndNamespaceScope() {
  const auto design = MakeDesign();
  pattern::Runtime runtime(design, "Top");
  runtime.Execute("scope.pdl", R"(iProcsForModule Top
iProc write {} {iWrite reg 1; iApply}
iProcsForModule Top -iProcNameSpace special
iProc write {} {iWrite reg 2; iApply}
iProc run {} {iCall write}
iCall special::run
)");
  Require(runtime.operations()[0].arguments[1] == "1",
          "iUseProcNameSpace is local; a selected procedure namespace is not "
          "inherited");
  pattern::Runtime arity(design, "Top");
  Fails([&] { arity.Execute("arity.pdl", "set cmd iWrite; $cmd reg\n"); },
        "arguments");
  pattern::Runtime nul(design, "Top");
  Fails([&] { nul.Execute("nul.pdl", "iWrite reg [format %c 0]\n"); }, "NUL");
  pattern::Runtime empty(design, "Top");
  Fails([&] { empty.Execute("empty.pdl", "set t {}; iWrite $t 1\n"); },
        "empty target");
}

void TestReturnValueAndInputStream() {
  const auto design = MakeDesign();
  pattern::Runtime runtime(design, "Top");
  runtime.Execute("return.pdl", R"(iProcsForModule Top
iProc value {} {return 7}
iWrite reg [iCall value]
iApply
)");
  Require(runtime.operations()[0].arguments[1] == "7",
          "preserve iProc return value");
  runtime.Execute(
      "continued.pdl",
      "iProc value {} {return 8}\niWrite reg [iCall value]\niApply\n");
  Require(runtime.operations()[2].arguments[1] == "8",
          "module binding spans input files");
}

void TestGeneratedBodies() {
  const auto design = MakeDesign();
  pattern::Runtime runtime(design, "Top");
  runtime.Execute("generated.pdl", R"(iProcsForModule Top
set body {iWrite reg[0] 1; iApply}
iProc p {} $body
iCall p
iProc q {} "iWrite reg\[1\] 0; iApply"
iCall q
)");
  Require(runtime.operations().size() == 4 &&
              runtime.operations()[0].arguments[0] == "reg[0]" &&
              runtime.operations()[2].arguments[0] == "reg[1]",
          "evaluated procedure bodies retain PDL vector semantics");
}

void TestDynamicBinding() {
  const auto design = MakeDesign();
  pattern::Runtime runtime(design, "Top");
  runtime.Execute("binding.pdl", R"(if {1} {iProcsForModule Top}
iProc p {} {iWrite reg 1; iApply}
iCall p
)");
  Require(runtime.operations().size() == 2,
          "execute bindings before procedure definitions");
}

void TestDynamicOptions() {
  const auto design = MakeDesign();
  pattern::Runtime runtime(design, "Top");
  Fails(
      [&] {
        runtime.Execute("options.pdl",
                        "set opt -bogus; iWrite reg 1 $opt on; iApply\n");
      },
      "option");
}

void TestDynamicLevel() {
  const auto design = MakeDesign();
  pattern::Runtime runtime(design, "Top");
  Fails(
      [&] {
        runtime.Execute(
            "level.pdl",
            "set level 0; iPDLLevel $level; set x 1; iWrite reg $x; iApply\n");
      },
      "level-0");
}

void TestMergeScope() {
  const auto design = MakeDesign();
  pattern::Runtime runtime(design, "Top");
  Fails(
      [&] {
        runtime.Execute("merge.pdl",
                        "iProcsForModule Top\niProc p {} {iMerge "
                        "-begin}\niCall p\niWrite reg 1; iApply\n");
      },
      "unclosed iMerge");
}

void TestAllTargetArguments() {
  const auto design = MakeDesign();
  pattern::Runtime runtime(design, "Top");
  runtime.Execute("targets.pdl", R"(iPrefix a
iScan -ir iface 8 -si 0xAA
iApply
iClockOverride clk -source src
iOverrideScanInterface iface iface2 -capture on
iRunLoop 2 -sck clk
iScan iface 8 -si 0x55
iApply
)");
  const auto& ops = runtime.operations();
  Require(ops[0].arguments[1] == "a.iface",
          "qualify iScan interface after -ir");
  Require(ops[2].arguments[0] == "a.clk" && ops[2].arguments[2] == "a.src",
          "qualify clock and source");
  Require(ops[3].arguments[0] == "a.iface" && ops[3].arguments[1] == "a.iface2",
          "qualify every overridden interface");
  Require(ops[4].arguments[2] == "a.clk", "qualify iRunLoop clock");
  Require(ops[5].arguments[0] == "a.iface",
          "qualify ordinary iScan target exactly once");
}

void TestEndOfStream() {
  const auto design = MakeDesign();
  pattern::Runtime runtime(design, "Top");
  runtime.Execute("queued.pdl", "iWrite reg 1\n");
  runtime.Execute("applied.pdl", "iApply\n");
  runtime.Finish();
  runtime.Finish();
  Fails([&] { runtime.Execute("late.pdl", "iApply\n"); }, "finished");

  pattern::Runtime pending(design, "Top");
  pending.Execute("pending.pdl", "iWrite reg 1\n");
  Fails([&] { pending.Finish(); }, "pending.pdl:1:1");
  Fails([&] { pending.Execute("late.pdl", "iApply\n"); }, "error");

  pattern::Runtime merging(design, "Top");
  merging.Execute("unclosed.pdl", "iMerge -begin\n");
  Fails([&] { merging.Finish(); }, "unclosed iMerge");

  pattern::Runtime failed(design, "Top");
  Fails([&] { failed.Execute("bad.pdl", "iBogus\n"); }, "iBogus");
  Fails([&] { failed.Finish(); }, "error");
}
void TestInstanceParameters() {
  pattern::Design design;
  design.AddIcl("params.icl", R"(
Module Child { Parameter W = 4; LocalParameter LAST = $W - 1; DataInPort d[$LAST:0]; }
Module Top { Instance a Of Child { Parameter W = 2; } Instance b Of Child; }
)");
  const auto netlist = pattern::Elaborate(design, "Top");
  pattern::Runtime runtime(design, netlist);
  runtime.Execute("params.pdl", R"(iProcsForModule Child
iProc configure {{value 1}} {iWrite d[$LAST:0] $value; iApply; iNote $W}
iCall a.configure
iCall b.configure 2
)");
  runtime.Finish();
  const auto& ops = runtime.operations();
  Require(
      ops[0].arguments[0] == "a.d[1:0]" && ops[3].arguments[0] == "b.d[3:0]",
      "inject specialized ICL parameters into each iProc call");
  Require(ops[2].arguments[0] == "2" && ops[5].arguments[0] == "4",
          "numeric parameters are Tcl integers rather than bit strings");
  pattern::Runtime collision(design, netlist);
  Fails(
      [&] {
        collision.Execute("collision.pdl",
                          "iProcsForModule Child\niProc p {W} {iNote $W}\n");
      },
      "ICL parameter");
}
void TestParameterNamedArgs() {
  pattern::Design design;
  design.AddIcl("args.icl", "Module Top { Parameter args = \"hello world\"; }");
  const auto netlist = pattern::Elaborate(design, "Top");
  pattern::Runtime runtime(design, netlist);
  runtime.Execute("args.pdl",
                  "iProcsForModule Top\niProc p {} {iNote $args}\niCall p\n");
  Require(runtime.operations()[0].arguments[0] == "hello world",
          "parameter args is an ordinary value, not a variadic Tcl formal");
  Fails([&] { runtime.Execute("excess.pdl", "iCall p 1\n"); }, "wrong # args");
}
void TestResetScope() {
  const auto design = MakeDesign();
  pattern::Runtime child(design, "Top");
  Fails(
      [&] {
        child.Execute("child-reset.pdl",
                      "iProcsForModule vendor::Child\niProc reset {} {iReset}\n"
                      "iCall a.reset\n");
      },
      "cannot retarget iReset");
  pattern::Runtime top(design, "Top");
  top.Execute("top-reset.pdl",
              "iProcsForModule Top\niProc reset {} {iPrefix a; iReset}\n"
              "iCall reset\n");
  Require(
      top.operations().size() == 1 && top.operations()[0].command == "iReset",
      "top-module reset is independent of the current target prefix");
}
}  // namespace
int main() {
  int failures = 0;
  for (const auto test :
       {TestSequentialDefinitions, TestBindingAndPrefixes,
        TestTclAndVectorSyntax, TestErrorsAndSafety,
        TestRuntimeValidationAndNamespaceScope, TestReturnValueAndInputStream,
        TestGeneratedBodies, TestDynamicBinding, TestDynamicOptions,
        TestDynamicLevel, TestMergeScope, TestAllTargetArguments,
        TestEndOfStream, TestInstanceParameters, TestParameterNamedArgs,
        TestResetScope}) {
    try {
      test();
    } catch (const std::exception& error) {
      std::cerr << "FAIL: " << error.what() << '\n';
      ++failures;
    }
  }
  std::cout << "PDL runtime failures: " << failures << '\n';
  return failures ? 1 : 0;
}
