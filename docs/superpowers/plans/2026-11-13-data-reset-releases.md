# DataRegister reset-release implementation plan

> **For agentic workers:** Use superpowers:executing-plans for inline implementation.

**Goal:** Allow local-reset-only rewriting to insert DataRegister control writes
when the existing scheduler can implement their declared write paths.

**Architecture:** Extend ResetPrerequisites' existing symbolic register choices
to DataRegisters present in DataRegisterPlan::writes. Keep read-only data controls
fixed. Reuse preservation masks, deterministic tie-breaking, reset DAG layers,
existing iWrite generation, Apply and fresh rewritten-runtime verification.
No separate data writer, reset solver or source-rewriting mechanism.

**Tech Stack:** Existing C++17, Z3, shared state/scheduler and Python unittest.
**Spec:** ../specs/2026-10-25-icl-pattern-generation-design.md; Task35 explicitly
left automatic DataRegister reset releases unfinished.

## Constraints

Autonomous implementation choices are delegated. Procedure bodies remain
unchanged; insert caller-side releases only. Retain4096reset-solver-bit and
existing traversal/solver budgets, reservations, immutable reassertion
diagnostics, and atomic output behavior. A declared path does not establish
that every payload is physically writable; Apply and concrete replay must
validate the selected payload. Bounded planning failures are not reachability
proofs. Do not invent state or assume writes succeeded.

The existing free-register solve is preferable to a separate data-control
planner: it retains one ordering and preservation model. Data write/address/
enable dependencies remain in the shared scheduler, separate from reset DAGs.
Candidate selection may fail at downstream scheduling within existing bounds;
no new completeness or optimality guarantee is claimed.

Expected scale: up to4096solver bits, ordinary tens of reset controls. Source
discovery and layering keep their existing bounded graph costs. Data write-plan
lookup is cached; measure64/256-bit control payloads and a mixed control chain.

## Review focus

- Read-only/already-released data controls must not produce new writes.
- Mixed Scan/Data reset ancestors must be released before dependent controls.
- Taken/preserved and taken-but-unwritten bits must keep their current contracts.
- A constant/disabled write path must fail without accepting an invented value.
- Caller rewriting, ordered files and pending state must retain operation order.

## Steps

Affected files: src/pattern/pattern_scheduler.cc; existing native scheduler test;
new tests/src/pattern/test_data_reset_releases.py; Makefile/test_makefile;
guide/model/spec and a vendor probe if a normative reset-mux fixture is accepted.

- [x] Write failing CLI/native cases for primary, scan-driven and mixed-ancestor
  data controls, readonly controls, reservations, sliced control bits, source
  rewriting/procedures and atomic failures. Record the original fixed-control
  diagnostic; include scale/replay cases.
- [x] Add writable DataRegister candidates to the existing reset solve and
  restrict fixed-control diagnostics to controls actually held fixed.
- [x] Run focused tests and representative scale checks; resolve independent
  review findings with regression cases.
- [x] Run full dynamic/static checks, leak-enabled sanitizers and style/naming;
  record vendor evidence and limits without weakening comparison.
- [x] Update the completion ledger and commit after verification.

Fourteen focused CLI methods pass in build/data-reset-releases-bv-cli.log/.exit(0)
(17.265s). They include unsigned XOR tie-breaking with zero and all-one fill.
Pinned Z3 4.13.3 minimizes a native bit-vector objective as an unsigned value;
its opt_context.cpp weights each zero-bit preference by 2^i. Keeping XOR as a
bit vector preserves the former bv2int(..., false) ranking and avoids the
conversion that exceeded the unchanged two-second limit under sanitizers.

Paired ordering/scale runs on local Linux g++16.2.1 -O2 used the same 458/460-byte
ICL inputs and six original/flat/generated runs per 64/256-bit case. Baseline
measurements were0.643s/4.880s and6.19s total,64900KiB peak RSS; the bit-vector
candidate measured0.487s/3.729s and4.84s total,64724KiB. These single paired runs
establish neither a universal speedup nor a guarantee for all4096-bit inputs.
Evidence: build/data-reset-releases-{baseline,bv}-benchmark.log/.exit(0).

Normal and reset-rewritten sources can legitimately produce different schedules:
explicit releases change apply boundaries and scan fill. Each is checked against
its own flat/generated replay. Native tests independently verify released control,
target9, differing scan-source fill and subsequent X-write retention;
build/data-reset-releases-bv-native-fixed.log/.exit(0). Independent review confirms
the source/native changes and the unsigned objective equivalence.

The initial full run failed the corpus audit because two new fixtures repeated
the same Child module definition. One shared replay fixture removes the duplicate;
build/data-reset-releases-corpus-fixed.log/.exit(0). The first native candidate run
then exposed a stale fixture filename; the corrected run above passes. The first
sanitizer candidate run failed because the sandbox prevents LeakSanitizer from
running. The same tests with leak detection enabled pass in the host environment:
build/data-reset-releases-bv-sanitizer-cli-host.log/.exit(0),14methods in45.467s.
Final native,reservations35,argument12,data-reset13 andqueued15 sanitizer suites
pass in build/data-reset-releases-final-sanitizer-*.log/.exit(0). Original failure
logs remain; no solver limit, leak check or corpus policy was weakened.

The generated reset-mux artifact is reproduced byte-for-byte from its original
request by a CLI test. Tessent accepts its generated forward output but adds a
read comparison, so forward comparison fails and remains recorded. Fresh reverse
certification succeeds in build/data-reset-release-reverse-certificate.log/.exit(0).
No comparator changed. Formatting, naming and independent review pass.
Complete38suite dynamic/static regression passes in
build/data-reset-releases-final-full.log/.exit(0), including49directional/seven
legacy standards,2,517ICL files with334/119parser/scanner rules and six editor
tests in186.241s. Five optional ANTLR tests remain skipped.
