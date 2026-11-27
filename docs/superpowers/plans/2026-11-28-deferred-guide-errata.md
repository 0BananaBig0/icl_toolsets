# Deferred guide errata (Task202)

> **For agentic workers:** Use superpowers:executing-plans inline, with one fresh whole-branch review at completion.

**Goal:** Correct seven recorded documentation findings against the current implementation and retained evidence.
**Spec:** The existing completion-program design, docs/superpowers/specs/2026-11-20-completion-program-design.md, and the source-backed contracts of Tasks194–197/200. No behavior is being designed or changed.
**Architecture:** Edit affected guide/tracker paragraphs and historical design/plan statements; preserve the measurements and their dated checkpoint counts.
**Tech stack:** Existing Markdown, native executables and maintained Python capability tests.

## Constraints and acceptance

- Parent master ae8cd835b45962672bb5cd57f3c062e8d1c97ffd; branch codex/deferred-guide-errata; reusable .worktrees/bsdl-defaults.
- Preserve 117 stashes, foreign worktrees and sibling ledgers. Coverage item1 remains excluded.
- Resolve precisely seven inherited Minors: Task194 exponent wording; Task195 vendor omission attribution; Task196 factor-only plan wording and numeric spacing; Task197 whole-program guide statement and O(V+E+P) temporary-space description; Task200 reset-held overview.
- The inherited count is 45; the candidate remainder is 38. Do not count a historical receipt or missing-driver artifact as repaired by changing guide prose.
- No production, test logic, dependency, API, limit, vendor certificate or historical measurement change. Source/executable identities must match completed Task201.
- Use local factual checks for these errata. TDD applies to production behavior changes; no new test is needed to mirror corrected prose. Reuse the completion program's standing design-question opt-out and inline authorization.

### Task 1: correct the affected statements

**Files:** docs/pattern-generation.md, docs/remaining-work.md, docs/superpowers/plans/2026-11-27-oscillator-default-restore.md, docs/superpowers/specs/2026-11-27-clock-period-serialization-design.md, docs/superpowers/specs/2026-11-27-clock-program-components-design.md, and this plan.
**Interfaces:** Preserve every documented native/default/retained-clock distinction and actual observation contract. No producer/consumer interface changes.

- [x] Match each finding to its current paragraph, implementation and retained review/evidence; record the seven-item disposition before editing.
- [x] Correct requested versus incidental reset-held observations, component-specific retention, exact exponent cancellation, version-specific vendor claims, deferred versus retained factor validation, Task196 number spacing and operation-dependent temporary space.
- [ ] Run existing normal/static read-scope, period and ordinary-clock capabilities plus native clock snapshots/policy. Verify unchanged tracked source/test inputs and all 44 qualified executable hashes. Check whitespace and build/install the source distribution containing the final documentation.
- [ ] Perform an author walkthrough for the affected generator-user decisions and record actual results; preserve all historical counts and unresolved requirements.
- [ ] Stage/read the full raw diff, write the grouped commit and run task-done with the artifact checker.

## Completion

- [ ] One independent full-branch review, including technical facts and the affected reader decisions. Regrade all findings and declined areas; one Critical/Important repair pass if needed, then no re-review; defer new Minors.
- [ ] Merge into master, check the actual merged documentation/source identities and affected capabilities, push, retire only this task branch and archive only this plan workspace.
