# Observation-state forwarding implementation plan

> **For agentic workers:** Use superpowers:executing-plans for inline implementation.

**Goal:** Retain valid iState -LastReadValue/-LastMiscompareValue in generated
PDL without changing controlling hardware assumptions or pending obligations.

**Architecture:** Reuse resolved targets, numeric sizing and the existing
ordered operation stream. Validate state-point kinds independently from the
controller-owned hardware-assumption restriction. Forward history statements
with their explicit flags; default state continues through Assume, queued-state
overwrites and reservation assignment. Static generation has no device readback:
iGetReadData/iGetMiscompares/iGetStatus remain diagnosed, not fabricated.

**Tech Stack:** Existing C++17, shared ICL/PDL parser, Tcl and Z3 value sizing.
**Spec:** ../specs/2026-10-25-icl-pattern-generation-design.md and IEEE1687-2014
§7.9.22(g–i), printed182 (PDF196), §8.4.1–3.

The user delegated implementation choices and requested uninterrupted work.
This is ordered static forwarding, not an interactive hardware adapter or a
claim of full observation-history runtime support. Preserving operations is
sufficient for the downstream tool to restore history; duplicating an unqueried
history database would add unused state. Runtime getter support requires a
separate explicit design for observed versus merely expected values.

## Constraints and scope

Retain target/width/value/resource validation, aliases and prefix resolution,
no new dependencies, no actual device event, atomic failed outputs, existing
merge stream ordering and reset rewrite validation. Expected width remains
bounded at1,048,576bits. Forwarding costs O(resolved bits) and output size;
no scan of previous operations or new per-register history copies.

Affected files: src/pattern/icl_pattern_generation_main.cc,
src/pattern/pattern_state.cc/.h, tests/src/pattern/test_observation_state.py,
affected existing CLI assertions, Makefile/test_makefile, guide/model/spec.
The shared validator's existing default behavior must stay unchanged; new
history-only validation may allow controller-owned IR because no hardware
assumption is made.

## Review focus

- History must not replace queued read/write values.
- History must not create taken-resource protection or change controlling fill.
- History on reset controls must not release/reset storage.
- Aliases, slices, prefixes and controller-owned IR must retain correct identity.
- Invalid output/internal-pin targets and overflow must fail atomically.

## Tasks

- [x] Add failing CLI tests for each flag on scan/data/top-level state points,
  pending access, protected resources, reset rewrite, aliases/prefixes and IR;
  validate unsupported targets/options/values and readback getter diagnostics.
- [x] Share state-point validation while keeping hardware-assumption checks;
  preserve explicit flags and skip Assume/pending/reservation changes for history.
  Update affected callers/tests only where behavior changes.
- [x] Run focused replay and scale checks, full dynamic/static regression,
  leak-enabled sanitizers, style/naming and independent review. Preserve the
  existing Tessent unsupported-option probe evidence; do not claim portability.
- [x] Record exact verification/limits and commit after passing checks.

Implementation uses ValidateStateTarget for the common state-point contract;
ValidateStateAssumption adds the unchanged controller-owned IR restriction.
History-only forwarding therefore permits IR without assuming TAP state.

Independent review added automatic taken-gate, IR, reset-rewrite and end-state
cases. The last exposed a pre-existing default-state bug: a state-only file
could emit an iApplyEndState restoration transaction. Final restoration now
requires a prior iApply/iReset/iRunLoop; an empty/state-only file does not create
a transaction. The original failure is retained in
build/observation-state-end-state-red.log/.exit(1).

Thirteen new methods and11DataRegister-state/11end-state/15queued-state methods
pass in the isolated build. New13 took2.338s;4,096history statements,
140,800PDL bytes, three replays took0.194s on local Linux g++16.2.1 -O2.
This is supported-scale evidence, not a speedup claim.
Formatting, naming and independent review pass. All isolated leak-enabled checks pass (native state,new13,state11,end-state11,
queued15). Integrated full dynamic/static regression passes in
build/observation-state-full.log/.exit(0), including37generator suites,48directions/
seven legacy standards,334/119rules over2,516ICL files, and six editor tests
in190.833s. Broader requirements and strict coverage remain open.
