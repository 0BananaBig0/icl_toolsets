# Partial period diagnostics implementation plan (Task205)

> **For agentic workers:** Use superpowers:executing-plans inline with one fresh independent whole-branch review.

**Goal:** Report invalid known period operands even beside dynamic neighbors.
**Architecture:** Reuse the shared period validator at declaration time, without changing concrete dispatch.
**Tech stack:** C++17, existing Python unittest formatter/LSP clients and native generators; no new dependency.
**Spec:** docs/superpowers/specs/2026-11-28-period-partial-diagnostics-design.md.

## Global constraints

- Parent master 5d8a92ad99341f166f02ce9c4c1509a26f0e6646; branch codex/period-partial-diagnostics in .worktrees/bsdl-defaults.
- Preserve 117 stashes, sibling ledgers, foreign worktrees and excluded item1. Inherit 31 findings, candidate remainder 30.
- Change only src/pdl/pdl_document.cc, tests/src/test_pdl_tools.py and this task's documentation. No public signature, dependency or clock event changes.
- Keep fully dynamic procedure operands deferred, literal semantics unchanged and default sanitizer leak detection enabled.

## Review focus

Both levels' formal substitutions must remain valid; either known invalid side
must diagnose; suffixed values cannot gain separate units; fully known errors
must retain validation; LSP correction must clear stale errors. Task1 covers all.

### Task 1: diagnose known operands independently

**Files:** src/pdl/pdl_document.cc, tests/src/test_pdl_tools.py; docs/remaining-work.md and this spec/plan.
**Interfaces:** Document command validation consumes NormalizeClockPeriod(value, optional unit); preserve its signature and the runtime caller in pattern_clock.cc.

- [x] Add shared mixed-operand cases and formatter/LSP methods at both levels, including invalid units/zero/negative/suffixed values and valid one/both-dynamic controls. Run against protected parent products: new methods must fail on missing diagnostics, not tooling errors.
- [x] Replace the both-concrete conjunction with independent known-operand validation using existing validator and valid unknown-neighbor placeholders. Rebuild the shared dependents and require both methods green; valid formatted output remains idempotent and LSP correction clears errors.
- [x] Run complete frontend suites, affected normal/static clock/reset generator modules, default leak-enabled functional checks, three C++ quality checks and source package/install. Record exact manifests and the inherited finding disposition; no unrelated full-regression claim.
- [x] Mark verified implementation progress, read the complete selected patch, commit with grouped messages and run task-done using tests/.task205-check.py.

## Branch completion

- [x] One independent whole-branch review; regrade every finding and declined area, one Critical/Important RED-to-GREEN repair wave if needed, defer Minors without rereview.
- [x] Merge master, verify affected behavior and exact qualified identities, push, archive only this ledger/helpers, retire this branch and continue the original inventory.

Completed at `6a55a9d`, merged and pushed as `f131b02`. Independent review:
0 Critical, 0 Important, 2 new Minor issues; one inherited resolution and
32 remaining findings. All 60 qualification gates and six merged-result checks
pass; 117 stashes are preserved. Evidence:
`build/period-partial-diagnostics-20261128/`.
