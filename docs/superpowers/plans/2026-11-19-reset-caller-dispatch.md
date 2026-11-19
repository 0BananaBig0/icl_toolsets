# Reset caller-site dispatch implementation plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans. Inline execution, one fresh whole-branch review.

**Goal:** Finish argument-dependent reset releases at repeated main caller sites while preserving procedure bodies.
**Architecture:** Extend existing SourceAction with evaluated arguments excluding the command word; collect signature-to-release variants during the fourth rewrite candidate; reuse the argument apply wrapper with a literal Tcl exact switch and existing replay.
**Tech Stack:** Existing C++17/Tcl/Z3/Python unittest.
**Spec:** docs/superpowers/specs/2026-11-19-reset-caller-dispatch-design.md

## Global Constraints
Preserve iProc bytes, call/resource ownership, once-only argument evaluation, main Tcl side effects and current solver/runtime limits. No SIB setup, helpers/global state, dependencies or RTL. New maintained tests only under tests/. Preserve strict100/90 raw gate.

## Review Focus
- Ambiguous identical signatures and pending groups fail atomically, without an early apply.
- Tcl8.6/9 list escaping, Unicode/literal argument bytes and default-named procedures select the intended case.
- Caller variable scope, prefixes, once-only substitutions and procedure definition order remain intact.
- Existing fixed-preamble output, immutable internal-reset diagnostics and reservations stay valid.
- Repeated-call performance uses a literal switch, not per-invocation dictionary rebuilding or full graph analysis.

### Task 136: Reset caller dispatch
**Files:** src/pattern/pattern_runtime.h/.cc; src/pattern/icl_pattern_generation_main.cc; tests/src/pattern/test_argument_reset_sources.py; tests/src/pattern_runtime_test.cc if needed for argument metadata; README.md and current guide/model ledger; this plan/spec.
**Interfaces:** Consume SourceAction and Generate/ResetEdits, Runtime source tracing, Command Tcl quoting and ScanScheduler::ResetPrerequisites. Produce an unchanged CLI contract with an additional validated fourth fallback.

- [x] Step1: Add repeated opposite reset-domain write/call cases to existing argument-reset suite, run against basefe28cec; Expected: local-reset joint or different-release diagnostic, no candidate acceptance.
- [x] Step2: Capture evaluated arguments; collect variants only for fourth fallback; reject conflicting preambles for identical signatures, build exact switch and reuse apply/uplevel wrapper. Expected: new cases pass with preserved procedure bytes, same ordered values and rewrite idempotence.
- [x] Step3: Add quoted/expanded/Unicode/default-name, namespace/prefix, multiple files, reservations/queue/immutable rejection, side effects and64/256-call scale tests in existing files. Expected: all relevant reset and source-runtime tests pass; record input sizes/wall/RSS and no speedup claim for previously rejected inputs.
- [x] Step4: Run changed-line format/tidy/strict17, focused native, leak-enabled reset/source/main CLI, normal full check, fresh unfiltered coverage, Rocky51 generator suites (actual updated counts), and fresh archive/build/install checks. Expected: executable gates0; raw gate2 remains visibly failed if below100/90; no waiver.
- [x] Step5: Correct current reset backlog/qualification summaries from actual evidence; commit candidate, archive source/provenance, run evidence verifier/task-done and one fresh whole-branch review. Expected: no unresolved Critical/Important findings; defer minors and rule on every declined topic.
- [ ] Step6: Refresh final source archive, merge into direct parentmaster, sync qualified products, push/delete task branch and preserve other worktrees/stashes. Expected: clean synchronizedmaster and completed plan ledger archived.

Self-review: one task; metadata is consumed only by the new fallback, all existing candidates retain prior behavior. Existing mandatory immutable-procedure rejection is preserved.
