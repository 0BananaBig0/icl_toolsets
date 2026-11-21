# Remaining work: items 2–15

User instruction: finish items 2–15 from the2026-11-20 inventory. Item 1,
strict 100% authored-line / 90% raw-branch coverage closure, is excluded from
this work. Its last measured96.68%/62.97% failure is retained; no acceptance
threshold has changed. The [completion design](superpowers/specs/2026-11-20-completion-program-design.md)
preserves the original requirements.

This is the current completion tracker. Earlier plans and dated evidence remain
historical records; their unchecked boxes are not automatically a current task
list. Completing one supported case does not complete a broad item below.

| Item | Required result | Status / acceptance |
|---|---|---|
| 2 | Independently phased TAPs, shared serial storage, intermediate taps, overlapping returns and independent lanes | Pending. Common-phase/equivalent-TCK multiple TAPs and disjoint lanes already work. Add state/capture/update/ownership cases for the remaining topologies and prove original/flat/generated replay. |
| 3 | General independently gated and serial-derived scan/reset events | Pending. Retain edge ordering, hidden storage and transient resets across source/flat/generated replay; no guard-only enabling. |
| 4 | Cyclic/self-referential DataRegister updates and callbacks | Pending callbacks. Task144 adds bounded simultaneous feedback from real preceding storage, known/unknown convergence, controlling history, capture ordering and intermediate ownership/rollback. All13 native/13 native sanitizer suites,53x890 per generator variant,11 focused source/flat/generated replay methods,39 quality gates and source packaging pass; independent branch review is pending. Changing feedback-derived external reset events remain item3. |
| 5 | Stateful absolute time and remaining clock semantics | Pending. Relative waits, clock overrides and restricted clock-only absolute waits already work; Task141 fixes associated-flat override target/source mapping. Cover stateful events, oscillator restoration/frequency, phases and remaining names/source mappings without inventing periods. |
| 6 | Runtime observations/status and interface/chain history | Pending. Task141 adds never-read/sparse unknown history and provably zero initial/no-compare status; applied reads still need actual observations. Complete model-backed observation and interface/chain history; real device access needs a supplied transport. |
| 7 | Remaining resource ownership and lifecycle | Pending. Task141 fixes compatible modeled-child port ownership with protected intermediate serial states, scope tests and a bounded invariance fast path. Add remaining clock/scan assignment/frequency, pattern-set lifecycle and TAP ownership. |
| 8 | Remaining raw-scan configurations | Pending. Add opaque-child, parallel/multi-TAP and raw-merge support and qualify remaining vendor-export forms. |
| 9 | Additional repairable caller-only reset transformations | Completed in Task143 for the inventory's state-dependent/identical-argument caller cases. Recorded invocations cover empty/caught calls and once-only arguments with unchanged procedure bytes. Five review findings are fixed; full post-fix qualification passes. Impossible queues/ownership/internal reassertions and uncertifiable file-return/cleanup boundaries reject atomically. |
| 10 | Broader merge/scheduling optimization and scale qualification | Pending. Cover resource/timing barriers and representative larger networks, preserving valid schedules and reporting measured costs rather than global optimality. Task142 retains explicit512-function generic-primary and parent4096-bit solver-limit reproductions; no limit was raised. |
| 11 | Remaining model/default/source/reference audit | Pending corpus/specialization and broader source acceptance. Task140 verifies the eight BSDL/default groups with19 replay methods and eight review fixes. Task142's typed vector-port implementation passes17 focused methods per variant, native caller checks and nine vendor models. Two Important review findings were fixed and full post-fix qualification passes; no Critical/Minor or unresolved blocker remains. All11 former duplicate-port corpus failures reach the existing interface-event boundary (item3), not end-to-end generation success. |
| 12 | Broader vendor and model round-trip qualification | Pending wider qualification. Task140 adds six bounded BSDL cases with48 nonempty original/flat/generated replays and12 matching directional apply-group comparisons; Task142 adds nine bounded port cases with72 nonempty replays and18 exact directional canonical comparisons; these are not new maintained golden certificates. Task143 adds a fresh post-fix invocation/reference vendor pair with equal full exports and eight matching physical replay forms. Extend the remaining model/corpus evidence. Physical import currently covers a single bound TAP with scan-only storage, not general physical/vendor equivalence. |
| 13 | Concrete physical replay diagnostics | Completed in Task139 after independent review and one verified fix pass. Serial bit/expected/observed, named closed routes, unknown loads and original reservation causes are covered; ordinary fallback and failed-output protection are preserved. |
| 14 | Execute five optional reference-ANTLR cases | Completed in Task138. All six existing adapter methods executed without skips using the pinned reference and matching runtime; commands below. |
| 15 | Reconcile current documentation/checklists | Completed in Task138. Current guide/model/README summaries distinguish completed support and remaining work; historical plans link here. |

## Reference verification

Reference: Honza255/icl_parser at 333f8bc7a54964138ac14a34fde9e942c496436d;
runtime antlr4-python3-runtime 4.7.2. Both are isolated under owned /tmp paths.
The reference is an independent syntax oracle, not normative authority.

    ICL_REF_DIR=/tmp/icl-reference-completion-20261120 /tmp/icl-reference-runtime-20261120/bin/python tests/tests/test_antlr_diff.py

Result: six tests ran, zero skips, exit 0, including all five formerly skipped
reference cases. Temporary paths are reproduction details, not installed
application dependencies or a guarantee that an external checkout remains
available. Task 138 evidence is retained under build/completion-records-20261120/.

## Existing qualified baseline

Task 137 at400d2ba is complete: 28 reset cases per binary variant, normal and
leak-enabled sanitizer suites, Rocky 51 suites/820 cases per variant, source
archive and dynamic/static installation checks. Four archived vendor exports
replay natively with --physical_pdl and preserve 13 explicit loads/seven applies.
All 13 reset-specific implementation plans are complete. This does not close
the broader items above or strict coverage item 1.

See [model acceptance](pattern-model-coverage.md#current-evidence-and-remaining-work)
and [current verification](pattern-generation.md#current-verification).
