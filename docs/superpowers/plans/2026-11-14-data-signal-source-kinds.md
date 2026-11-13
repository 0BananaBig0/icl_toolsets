# Plain data-signal source kinds implementation plan

> Execute inline with superpowers:executing-plans under the existing autonomous
> authorization, after Task49 is integrated and committed. Independent review
> remains required; no implementation-agent delegation.

**Goal:** Reject non-data source kinds wherever the elaborator consumes an
ordinary data signal, preserving original/flat/generated TAP behavior.

**Architecture:** Validate the connected typed netlist after flat origin
restoration and existing combinational-cycle checks. Reuse node kinds, resolved
bit references and alias inputs; classify alias bits iteratively with a memoized
result. Keep parsing, hardware evaluation and PDL target resolution unchanged.

**Tech stack:** C++17, existing native tests and Python unittest; no dependency.
**Spec:** [Approved generator design](../specs/2026-10-25-icl-pattern-generation-design.md)
and its remaining source-kind acceptance work.

## Evidence and scope decision

IEEE1687-2014 §6.3.12(d), printed pp70–71, lists ordinary data terminals.
The current executable accepts `TCKPort clock; DataOutPort value { Source clock; }`
with exit0; the exact reproducer and command are in build/source-kind-audit.
A clock port is not an ordinary data signal. §6.4.6.23(d), p107, separately permits
ToIRSelectPort only as the selector of a two-input ScanMux. §6.4.6.12 and §6.4.15
also make ToSelectPort Source and OneHotDataGroup explicit Port contributors
ordinary data consumers.

A parser-only check cannot resolve aliases or specialized instances. A new full
scope/direction pass would reject lowered TAP edges: SerialAssembler deliberately
connects child instruction storage to generated route muxes and child controls.
Therefore this increment checks terminal kinds after restoration, with no flat,
name-prefix or generated-model exemptions. Existing original-source hierarchy
and alias-boundary validation remains. Local-versus-child data-direction rules,
complete restored-scope qualification and contextual control-source rules remain
required follow-up; this is not complete §6.3.12 conformance.

Ordinary data kinds are DataInPort, DataOutPort, SelectPort, ToSelectPort,
ScanRegister, DataRegister, LogicSignal, DataMux and OneHotDataGroup. Constant
0/1/x references qualify; existing missing-source placeholders retain their
current handling. A ToIRSelectPort classification is accepted only for a scalar
SelectedBy of a two-input ScanMux. Other clock, scan and control-port kinds are
rejected in ordinary data contexts. Stop at typed terminals, rather than tracing
through output-port drivers. Resolve only consumed alias bits, retaining existing
whole-alias hierarchy restrictions and PDL-only aliases.

Cover DataOutPort/ToSelectPort Source; child DataInPort/SelectPort Source;
CaptureSource, WriteDataSource, ReadDataSource, WriteEnSource and Enable;
mux SelectedBy; LogicSignal operands; explicit OneHotDataGroup Port sources.
Do not apply an ordinary-data rule to DataMux choice inputs, control-port
bindings or AddressPort/WriteEnPort/ReadEnPort binding rules: their special
context rules are separately specified. Do not change PDL readable/writeable
kinds or turn this validation into a connection rewrite.

## Files and implementation

Modify src/pattern/pattern_netlist.cc for one connected-netlist validation pass.
Keep its classifier private, with no new public API. Extend existing
 tests/src/pattern_netlist_test.cc for the classification matrix and alias cases;
 tests/src/pattern_flatten_test.cc for restored types and generated TAP wiring;
 tests/src/test_icl_pattern_generation_main.py for atomic rejection and legal
original/flat/regenerated CLI checks. Update the current guide and model ledger.
Existing Makefile test registrations cover these files.

- [x] Obtain independent design review covering special port clauses, generated
  TAP lowering and all ordinary-data consumers.
- [x] Add native failing tests for wrong-kind terminals across the covered
  consumers, direct and alias/sliced forms; assert source-located diagnostics.
  Include positive terminal kinds, literals, legal hierarchy and typed ports.
- [x] Add two-input ScanMux/ToIRSelect positives and wrong consumer negatives,
  mixed alias valid/invalid slices, nested aliases, and existing cycle rejection.
- [x] Run the new checks against the pre-change implementation and retain RED
  evidence for accepted-invalid inputs, rather than unrelated fixture errors.
- [x] Implement the shared check after origin restoration and cycle validation.
  Cache each referenced alias bit; use an iterative chain walk, preserving
  resolved indices, inversion and scopes rather than re-parsing source text.
- [x] Add CLI output-preservation checks and original/flat/generated equivalence,
  including single/multiple TAP generated route muxes and select logic.
- [x] Measure a representative wide alias with many repeated slice consumers
  and a long alias chain; record input size, runtime and peak memory. Keep work
  proportional to references plus distinct consumed alias bits, with ordered-map
  logarithmic lookup cost and no repeated full-width alias expansion.
- [x] Run focused netlist/flat/main/multiple-TAP checks, independent source review,
  full dynamic/static regression, leak-enabled sanitizers, formatting and naming.
  Document exact scope, preserve known unrelated limits and commit verified work.

Task49 is committed as1c138b7. Task50 is implemented in the isolated worktree
branch task/data-signal-source-kinds, with clean independent source review.
Full integration and the planned leak-enabled sanitizers pass.

Independent design review accepts the narrower kind-union rule and confirms the
ToIRSelect, addressed-port and generated-TAP exceptions above. No generated-name
or metadata-wide bypass is needed. The source-kind union is a permissive filter;
it must not be described as proving every accepted connection legal.

Pre-change optimized CLI measurements (local GCC16.2.1, while Task49 regression
was running):4096 scalar aliases,83873 ICL bytes,0.07s/56720KiB; a65536-bit alias
with4096 slice consumers of256 distinct bits,177434 ICL bytes,0.09s/61648KiB.
Commands and /usr/bin/time -v results are in build/source-kind-audit/*-baseline.log.
These are elaboration/startup baselines, not a scheduler throughput claim.

Focused evidence: native RED rejects no bad data types (two expected-failure
assertions fail); corrected CLI RED accepts invalid original/alias/flat sources.
The first CLI fixture incorrectly mixed an unbound TCK with a serial interface;
that unrelated failure is retained, and the corrected primary-only fixtures prove
the source-kind defect. Main35 and multiple-TAP22 methods now pass. Native netlist,
logic, address, state, symbolic, TAP, scheduler and flat checks pass.
Naming passes. Native scale fixtures cover4096 aliases/one consumer and a65536-bit
alias with4096 consumers of256 distinct bits. Repeated CLI comparison commands,
input sizes and per-run time/RSS are retained in build/source-kind-audit/comparison.json.

Connected fixture corrections additionally touch tests/src/pattern_tap_test.cc
and tests/src/pattern_scheduler_test.cc. A scan-input monitor now uses ScanOutPort;
a forwarded clock uses ToTCKPort. An invalid scan-output-to-data-input fixture
expects the earlier kind error, while a separately constructed lowered graph still
proves cycle detection. The DataRegister enable fixture now uses a two-input UE/zero DataMux that also
drives ToUpdateEnPort, with selector0. This matches §6.3.12(d)(1)(x) and
§6.4.6.10(c–d); DataMux is an ordinary-data terminal and these clauses do not
restrict it to exclusive fanout. Native and CLI update/restoration/rollback
assertions and original/flat/generated checks remain. Independent review confirms these
preserve coverage without allowing invalid user ICL or serializing synthetic
connections as legal ICL. The restored-source test call is in main, outside the
unrelated address-budget catch block; an initial misplaced call was corrected.

The first full run failed only after reaching reservation fixtures that exposed
AddressPort/ReadEnPort through DataOutPort. Corrected tests use same-kind child
control inputs and PDL aliases, retaining distinct ownership identities. The
40-suite optimized sweep passed39 suites and exposed the remaining direct-UE
client fixture; its corrected full18-method suite passes. Keep the failed sweep
all.exit1 and original full.exit2. All initially selected leak-enabled checks
pass, including netlist/logic/address/symbolic and functional20 multiple-TAP cases.
Final full integration and affected scheduler/client/control-reservation sanitizer
checks pass after importing the final fixture changes. Full evidence:
build/data-source-confirmed-full.log/.exit0,907.574s,40 dynamic/static suites,
51 directional/seven legacy goldens,2523 ICL334/119 rules, editor6 in176.629s;
five optional ANTLR tests skipped. Corrected sanitizer evidence:
build/data-source-corrected-sanitizer-*.exit, all0. Both scale methods remain
excluded from the20-functional-method sanitizer subset; optimized22 passes.
Strict coverage, vendor certification and Rocky8 qualification remain open. The product validator has
not changed since the first source review.
