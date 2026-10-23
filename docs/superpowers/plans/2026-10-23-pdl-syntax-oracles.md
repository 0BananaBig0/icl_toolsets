# PDL syntax oracle checks

> Execution: superpowers:executing-plans inline, TDD, one independent final review.

Goal: assess and improve syntax acceptance/rejection independently of the
existing implementation, without pattern generation or retargeting.
Spec: existing 2026-10-23-pdl-tools-design.md plus the user's syntax-only scope.
Standing autonomous execution authorization applies; preserve existing changes.

Architecture: keep the shared pdl::Document AST/parser/analysis. Add a Python
stdlib test runner using separately authored IEEE PDL command seeds and a Tcl
safe interpreter oracle for bounded, constructed Tcl builtin examples. No source
from vendor files is executed. No runtime/library dependency is added.

1. Add tests/src/test_pdl_syntax_properties.py and source-attributed JSON inputs
   under tests/cases/standard/pdl and tests/cases/ours/pdl. Materialize generated
   inputs under ignored tests/generated/pdl_syntax. Check positive/negative PDL
   seeds and layout variants; check Tcl builtin command structure against Tcl.
   Compare CLI/LSP acceptance and formatting; test idempotence and bounded nested
   known scripts. Keep oracle failures distinct from parser mismatches.
2. Run RED and minimize discrepancies. Fix demonstrated shared-parser syntax
   defects with a retained regression first. Do not implement retargeting or
   evaluate arbitrary PDL; do not guess runtime-defined command meanings.
3. Add pdl-syntaxcheck to Makefile's normal PDL checks and coverage. Verify
   malformed controls, script/data roles, static options, LF/CRLF and nesting.
   Keep deterministic budgets (hundreds of inputs, per-process timeout5s,
   no unbounded recursion or random nondeterminism). Review independent evidence.
4. Run full/style checks, static and installed PDL suites, authored coverage,
   and a representative868139-byte performance check. Update docs with exact
   method scope, minimized failures, syntax-only remaining limits and commands.
   Preserve current compatibility extensions; report them separately from IEEE
   syntax instead of silently calling them normative. No commit/push requested.

Review focus: valid Tcl mistaken for PDL; malformed builtin argument structure
accepted; dynamic words wrongly treated as literal options; opaque data parsed
as scripts; tests using the implementation itself as their oracle.
