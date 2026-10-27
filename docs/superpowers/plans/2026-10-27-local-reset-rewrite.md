# Local reset rewrite implementation plan

> Execute inline with Superpowers TDD, independent review and full regression.

**Goal:** Continue the approved generator requirements by implementing reset-only
prerequisites before source writes/calls, preserving all iProc text.
**Architecture:** Reuse Runtime source instrumentation and ScanScheduler's
symbolic reset model. Record outer invocation ranges, plan transitive control
writes by dependency layers, apply them using existing concrete replay, and
splice only reset commands into the original ordered sources. Normal flat
patterns continue using the unmodified source operation stream.
**Tech stack:** existing C++17, shared ICL/PDL trees, Tcl and Z3.
**Spec:** docs/superpowers/specs/2026-10-25-icl-pattern-generation-design.md;
user confirmed all original acceptance requirements remain in force.

- [x] Add red regressions for transitive reset controls, calls, multiple inputs,
  grouped independent releases, prefix handling, and unchanged procedure text.
- [x] Add optional runtime source-site tracing and a reset prerequisite planner
  sharing the scheduler state/dependency graph; preserve no-reset passthrough.
- [x] Integrate source edits with replay and atomic publication; reject cases
  that cannot be corrected without modifying procedure bodies or changing an
  already pending apply group. Do not insert SIB setup into reset-only output.
- [x] Run focused tests, independent review, and full regression; document actual
  support and unresolved requirements without claiming complete retargeting.

Review: avoid flushing queued writes accidentally; use actual reset polarity;
release ancestors on earlier apply boundaries; account for prefix, repeated
source execution and source-order definitions; fail before publishing outputs.
Performance: traverse bounded netlists once for dependency analysis; solve only
requested reset cones; reuse the existing time/resource limits. Rewrite analysis
may perform a separate scheduler replay only when --local_reset is requested.

## Follow-up guidance and validation

The user's 2026-10-27 direction favors shared-reset and IJTAG-select dependency
groups and greedy scheduling over exhaustive pattern enumeration. This change
already batches reset-control releases by dependency depth. Shared signal
identity and depth remain distinct concepts; a complete selector-guided greedy
backend is subsequent work, not established by these reset tests.

- [x] Measure reverse-ordered reset dependencies at the current 4096-register
  structural limit; replace repeated whole-register ordering scans with a
  deterministic ready queue, retaining cycle rejection and existing replay.
- [x] Qualify the rewrite fixture with Tessent, correcting fixture connectivity
  when vendor DRC exposes invalid hierarchy references. Record differences
  without turning them into approved goldens.

## Evidence and remaining limits

- Initial red tests: `build/local-reset-rewrite-red.log`. Independent review
  reproduced four failures (repeated-site queue splits, substitution ordering,
  continued EOF comments and nested AccessTogether aliases). Fixes were checked
  against those reproductions and the expanded CLI suite; the reviewer found
  no remaining issue in those fixes.
- Latest dynamic CLI suite: 26 tests passed, including TAP source/flat/generated
  replay and reset artifact idempotency (`build/local-reset-final-focused.log`).
- Final full `make check BISON=/usr/bin/bison -j4` exited 0, using
  `ICL_REF_DIR=/tmp/icl-reference-honza-20261021` and the existing reference venv
  on PATH. Dynamic/static CLI suites each passed 26 tests; seven offline Tessent
  goldens passed; 334/334 grammar and 119/119 scanner rules were exercised;
  Vim/Neovim each processed 38,637 ICL and 809 PDL files. Corpus audit found no
  duplicate-file group. Log: `build/local-reset-final-regression.log`.
- ASan/UBSan: 26 CLI tests plus native runtime/scheduler tests passed with
  `detect_leaks=1`, using an instrumented Clang build. LSan's sandbox ptrace
  failure was resolved by rerunning outside the sandbox, not by disabling it.
  External Z3 remains uninstrumented. Logs:
  `build/local-reset-rewrite-sanitize-host.log` and
  `build/local-reset-native-sanitize.log`.
- Reverse-order 4096-register scheduler construction: 0.342 s baseline;
  0.010–0.019 s across three new runs. Only the ordering phase changed to a
  ready queue; it visits each dependency once and retains cycle rejection.
  The dependency-cone tracing and full pattern solver are not claimed linear.
  Independent review of the ready queue found no issues.
- Tessent 2026.2 accepted the reset-only rewrite in the explicit TAP fixture.
  Strict traces differ in TAP operations, comparisons, vector spelling and final
  SIB state; this is vendor acceptance, not equivalence or a new golden.
- Ruling: preserve procedure text and reject unsafe insertion sites or internal
  reassertions that cannot be repaired at the main sequence. This costs support
  for those source forms; changing their behavior silently would violate the
  original requirement. Feedback, general selector-guided greedy scheduling,
  iMerge and full model coverage remain outstanding.
