# Explicit observation-history getters

> Use superpowers:executing-plans for authorized inline implementation.

**Goal:** Evaluate iGetReadData and iGetMiscompares when every requested bit is
available from an explicit iState history assumption. Never substitute an
expected comparison or current hardware model for an observation.

**Architecture:** Extend the existing Runtime with two sparse maps from node ID
to history strings, using the elaborated netlist's resolved bit identity. Keep
an unavailable sentinel distinct from explicit x (which §7.9.22(c) identifies
as never read/compared). iState history records supplied bits; applied iRead
operations invalidate those bits in both maps. Pending reads do not invalidate
history until iApply; iReset discards the pending queue without inventing new
observations. Getters return literal Tcl strings and emit no device operation.

**Tech Stack:** Existing C++17/Tcl/Z3 libraries; existing native runtime and
observation-state CLI tests. No new dependency or public Runtime API.

**Spec:** [Approved generator requirements](../specs/2026-10-25-icl-pattern-generation-design.md),
bounded static PDL computation and observation history. Autonomous inline
execution is authorized. Source rules: IEEE1687 §7.9.22(c,g,h,i), §8.4.1 and
§8.4.2. Specific rule(h) describes miscompare history; its introductory prose
instead mentions expected values. Follow the specific rule and document this
distinction. Tessent2026.2 previously rejected history flags, so no new vendor
portability claim is justified without a successful probe.

## Implementation choices

Reuse ResolvePdlTarget, ValidatePdlAccessTarget and PdlNumber from pattern_values,
and ValidateStateTarget from pattern_state for history assignments;
do not create another alias/range or numeric parser. Save the constructor's
existing optional Netlist pointer in the implementation. The Design/top-only
Runtime overload retains its hardware-readback diagnostic for getters. Lazily
create a Z3 context only for supplied history values. Link pattern_values and
existing Z3 into normal/static/instrumented runtime and flatten tests and add
matching compile flags/check prerequisites to normal/instrumented runtime
objects. Flatten tests actually invoke Runtime and cannot exclude its object.
All six test links need values/Z3; static links need the static archive. Editor
binaries remain independent of Runtime.

History is recorded by physical node/index, including slices, prefixes and
aliases. Read-value inversion is applied when storing/retrieving alias bits.
Miscompare masks retain bit positions but do not invert: complementing both
observed and expected values leaves their XOR unchanged. Repeated assignments
overwrite earlier values. A missing bit or an applied read requires the existing
source-located hardware-readback error, even if the comparison value is known.
An explicit later history assumption restores static availability.

Reuse the existing pending-operation boundary; on iApply inspect only that
pending range for iRead. Each applied range is visited once, and only existing
cache entries need invalidation. Writes, controlling-state assumptions, resets,
notes, reservations and history in the other category do not create observations.
iGetStatus and ScanInterface/-chain history remain unsupported in this increment.
Do not infer unread initial history for bits that were never explicitly supplied.

Format default/-bin with 0b and one bit per requested index. For -hex, use one
digit per group of up to four bits, X for an entirely unread group and x for a
partly unread group, otherwise lowercase hexadecimal digits. Decimal unknown
returns X; known decimal conversion reuses Tcl's installed bignum support through
Tcl_ExprString with a generated numeric expression. Do not interpolate source text into that
expression. Width and input limits remain those of the shared PDL helpers.

History storage is bounded by elaborated node/bit budgets; allocate vectors only
for nodes with supplied history. Use sparse keyed lookup, avoiding prior-command
searches at getter time. Exercise repeated getters, many small targets and a
65536-bit value, recording wall/RSS and output sizes before claiming scale.

## Files and verification

Modify src/pattern/pattern_runtime.cc and Makefile; maintain existing
tests/src/pattern_runtime_test.cc and tests/src/pattern/test_observation_state.py.
Update the old rejection case to apply a real read after supplied history, so
it continues testing unavailable live readback. Update current guide/model/spec
and preserve historical Task37 evidence with a clear pointer to the new scope.

- [x] Review this design against the concrete runtime callback, pending queue,
  target/value helpers and build dependencies before implementation.
- [x] Add failing native/CLI tests for both supplied categories, binary/hex/
  decimal and x formats, alias inversion/mask invariance, slices/prefixes and
  multiple input files. Getters must expand to literals without hardware work.
- [x] Cover pending-versus-applied reads, reset-discarded reads, partial cache
  invalidation, explicit restoration, independent categories, malformed targets,
  output preservation, unchanged iGetStatus, original/flat/generated replay.
- [x] Implement with shared resolvers and run focused tests, style/naming,
  representative scale checks and independent review.
- [x] Run full dynamic/static regression, native/CLI leak-enabled sanitizers,
  and instrumented runtime checks for the changed build linkage; retain evidence.
- [x] Update ledgers and commit; keep live readback, interface history and other
  remaining model requirements open.

Independent design review approves the explicit-history model and mask inversion
rule. §8.4.1 says non-iRead operations do not update read history. Keep observed
output identity distinct from a register driving it; do not follow DataOutPort
sources. The review found the flatten-test dependency paths, confirmed by direct
Runtime calls in pattern_flatten_test.cc; update their three build variants too.

Required edge cases added to the test plan: history after a pending read must
still become unavailable at apply; reset-discarded reads followed by empty apply
must not invalidate history; no-expected/all-X reads invalidate supplied history;
partially overlapping inverted/reordered aliases, repeated physical bits,
independent categories, partial x overwrites, 1/3/5-bit hexadecimal groups, zero
and >64-bit decimal, invalid DataOutPort history assignments, Design-only getter
rejection and iGetStatus rejection. The existing Runtime EvalWords(expr,0b+bits)
conversion for numeric ICL parameters supplies the decimal formatting pattern.

Implementation ruling: the 65536-bit history replay exposed a pre-existing flat
serializer defect: a legal empty DataRegister was emitted with an illegal empty
body. Retain the failing wide CLI case and add a focused scalar/child/vector/
attribute roundtrip test; emit a semicolon for an otherwise empty DataRegister.
This is necessary for history-only register inputs and does not invent a reset
value or change the hardware model. Extend scope to pattern_flatten.cc and its
existing native tests. Preserve the initial incorrectly placed native test run
as failed evidence; move its invocation to main before rerunning the red test.

Independent implementation review found that invoking the Tcl expr command
allowed a user definition to change decimal getter values (9 became77). The
native regression failed against that implementation. Use Tcl_ExprString on the
generated binary literal; its C API bypasses command lookup and returns the
formatted bignum through the interpreter result. Other review areas were clean.

Focused evidence: observation-getters-red-build.exit0, red-native.exit1 and
red-cli.exit1 (six positive getter cases); initial green native0/CLI1 exposed
empty DataRegister serialization. Chain-option red exit1 (two accepted invalid
requests); corrected flatten red2 exit1 identifies the empty-body diagnostic.
Focused runtime/flatten/20CLI checks pass, with scale evidence in
build/observation-getters-focused-cli.log/.exit0. Initial manual build failed
with an incorrect lib/libz3.a path; corrected lib64 link passes. Review regression
observation-getters-review-red.exit1 confirms user-defined expr corruption;
review-green-build.exit0 includes dynamic/static runtime/flatten tests and the
rebuilt generator. Final full/sanitizer/instrumented verification is running.

Final verification: build/observation-getters-final-full.log/.exit (0) includes
39 generator suites in each dynamic/static build, native checks, 50 directional
and seven legacy standards, 2,519 ICL files covering all 334/119 parser/scanner
rules, and six editor tests in 180.590s. Five optional ANTLR checks were skipped.
Final leak-enabled ASan/UBSan build/runtime/flatten/observation20/queued15/
argument12/compound12 logs all have exit 0; the observation suite took9.170s,
argument suite66.244s and compound suite17.401s. Fresh instrumented runtime and
flatten builds/tests pass in observation-getters-instrumented-* logs. Final
format/diff and naming checks pass. No new vendor certificate, strict coverage
pass or later-change Rocky8 qualification is claimed. The independent review's
sole defect was fixed by the retained expr-redefinition RED→GREEN regression;
the complete suite ran after that correction.
