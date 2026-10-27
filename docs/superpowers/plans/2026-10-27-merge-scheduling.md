# iMerge scheduling implementation plan

> **For agentic workers:** Use superpowers:executing-plans inline with TDD and
> one independent final review. Continue in the existing authorized workspace.

**Goal:** Execute nested iMerge blocks and batch compatible calls without
enumerating interleavings or emitting merge artifacts.
**Architecture:** Preserve Runtime operations and their call stacks. A shared
merge-normalization pass removes markers, orders direct notes first and combines
consecutive single-apply calls with disjoint, fully known ScanRegister writes.
The existing scheduler and concrete replay validate the result. If combined
scheduling fails, retry the serial stream in a fresh scheduler.
**Tech Stack:** Existing C++17, Tcl, Z3 and Python regression infrastructure.
**Spec:** `docs/superpowers/specs/2026-10-25-icl-pattern-generation-design.md`,
IEEE 1687-2014 §7.9.18 (printed 176–178), Tessent IJTAG User's Manual v2023.4,
“How to Run iCalls in Parallel”.

## Constraints and scope

- Reuse parsers, operation origins/call stacks, value resolution and scheduler.
- No ICL/PDL token sharing, new dependency, CLI option or generated tracked code.
- Empty queue on merge entry; each direct call must apply its queue before return.
- Reject iReset anywhere inside merged calls and directly nested merge blocks
  at the same procedure depth. Nested merges through iCall remain supported.
- Preserve all non-merge applies. No global shortest-pattern claim. Multi-apply,
  read, partial/X, alias, clock and stateful calls initially remain serial;
  unsupported commands (including iTake/iRelease) remain explicit errors.
- Preprocessing is linear in operations/values apart from ordered-set lookups
  and bounded call nesting. Test 256 simple calls; no factorial search.

## Review focus

- Repeated calls from the same source site must remain distinct streams.
- Notes at merge depth precede calls, while notes inside calls keep their order.
- Partial/X values must not acquire fill from reordered preparation scans.
- Failed batched scheduling must not change the serial fallback's initial state.
- Reset-only output must preserve procedures and never insert writes inside the
  merge body; changed execution ordering must still fail before publication.

## Tasks

- [x] Add `tests/src/pattern/test_merge_scheduling.py` and attributed fixtures
  under `tests/cases/ours/pattern_generation/merge/`. Observe failures for
  independent batching, nesting, notes, conflicts, barriers, resets, local-reset
  hoisting, repeated calls and source/flat/PDL fixed points.
- [x] Tighten Runtime merge validation and trace the entire main merge block as
  one SourceAction. Add `pattern_merge.h/.cc` and `MergeOperations(netlist,
  operations, combine)` to the existing solver sources. Use call stacks to
  recover direct call boundaries; serialize any call not safely batchable.
- [x] Normalize normal generation with combined/serial fallback. For reset-only
  rewriting retain original indices and skip validated merge markers. Reuse the
  existing fresh-runtime verification of inserted releases.
- [x] Integrate tests in dynamic/static checks, run focused tests, independent
  review, full `make check`, format checks and ASan/UBSan with leaks enabled.
  Measure representative input and update README/developer limitations.

## Evidence

- Nine focused tests pass in both dynamic and static executables. Existing
  runtime, 26 CLI and ten dependency-scheduling tests pass as well. Logs:
  `build/merge-scheduling-focused.log` and
  `build/merge-scheduling-pattern-regression.log`.
- Initial fixture defects (missing explicit reset, invalid ResetSource syntax
  and wrong module binding for the TAP test) were corrected against existing
  working fixtures. The corrected tests fail against the saved old executable
  for missing merge execution/reset/depth validation:
  `build/merge-scheduling-corrected-red.log`.
- Independent reviewer found no actionable defects. Additional probes covered
  empty merges, nested notes, reversed full-register ranges, iTake rejection
  and deeply called iReset rejection. Broader verification remains the parent's
  responsibility.
- Scale measurement: 256 calls, 26,471 bytes of ICL+PDL, one output apply,
  0.120 seconds in the CLI; the Python harness took 0.17 seconds and peaked at
  60,908 KiB RSS (`/usr/bin/time -f 'wall=%e peak_rss_kib=%M' python3
  tests/src/pattern/test_merge_scheduling.py build/icl-pattern-generation
  MergeScheduling.test_many_calls`). Linux WSL2, g++ 16.2.1 `-O2`, Z3 4.13.3;
  this is a supported-scale check, not an old/new speedup claim.
- Ruling: limit batching to disjoint full known single-apply calls. Other
  supported streams retain correct serial execution; this costs optimization
  opportunities but avoids fill/barrier changes and interleaving enumeration.
- Full `make check BISON=/usr/bin/bison -j4` exited 0 with the existing Honza
  checkout/venv (`build/merge-scheduling-regression.log`). Dynamic and static
  generator suites, formatter/LSP/editor suites, grammar/scanner coverage,
  corpus checks and seven offline Tessent goldens passed. `make formatcheck`
  and `git diff --check` also passed.
- ASan/UBSan with `ASAN_OPTIONS=detect_leaks=1` passed all nine merge tests,
  all 26 existing generator CLI tests and the native runtime suite, run outside
  the ptrace sandbox. Logs: `build/merge-scheduling-sanitize.log`,
  `build/merge-scheduling-cli-sanitize.log`, and
  `build/merge-scheduling-runtime-sanitize.log`. External Z3 remains
  uninstrumented. Fresh Tessent equivalence and strict authored-C++ coverage
  were not rerun for this increment; no new vendor golden was published.
- Existing source moves/reset/scheduler changes are preserved; no commit or
  push is part of this continuation.
