# ASAN budget selection implementation plan (Task221)

> **For agentic workers:** Use superpowers:executing-plans inline with one fresh whole-branch review.

**Goal:** Keep normal/static wide budget checks active when ASAN_OPTIONS is set.
**Architecture:** One actual runtime --version/help probe, existing wide exclusion
for ASAN binaries and a native cross-environment regression.
**Tech Stack:** Existing Python unittest, subprocess and unchanged native products.
**Spec:** docs/superpowers/specs/2026-11-28-asan-budget-selection-design.md.

## Global constraints

Parent master c56190a3f6ac6e333c0c1ca229595e5142ad7075;
branch codex/asan-budget-selection. One existing Python module and three Markdown
files change. Preserve 4,134 unaffected inputs, 48 products, 117 stashes and all
limits. SAN wide case/meta-check remain intentionally skipped, never relabelled
passing. No dependency/native API/Makefile changes; retain broader evidence by identity.

## Review focus

- Environment options alone must never classify a normal/static binary as ASAN.
- Actual sanitizer products keep the unqualified wide memory case excluded.
- Probe flag copies preserve the parent's environment and existing leak/UB settings.
- Cross-environment child runs must execute only the intended wide method, not recurse.
- Final reports distinguish executed methods from intentional sanitizer skips.

### Task 1: classify actual instrumentation

**Files:** tests/src/pattern/test_primary_pin_literals.py; tracker/spec/plan;
private tests/.task221-* helpers; build/asan-budget-selection-20261128.
**Interfaces:** Existing native --version and ASAN help stderr, current wide test
and qualified product manifests. No native behavior or API changes.

- [ ] Add eligible-build cross-environment regression. Run against the old
  decorator; expected one assertion failure exposing the child false skip.
- [ ] Probe actual runtime once and replace environment-only decorator.
  Expected focused GREEN and the same sanitizer exclusion.
- [ ] Run full module normal/static with and without ASAN_OPTIONS, plus default-
  leak SAN; expected 5 reports/55 reported methods/53 executed/2 intentional SAN
  skips. Record three native probe responses; package four exact files and verify
  4,135 final inputs/48 unchanged products/whitespace. Commit selected diff;
  task-done runs python3 tests/.task221-check.py.
- [ ] Fresh review; regrade and repair blocking findings once if necessary.
- [ ] Merge, verify, push, archive owned ledger/helpers and retire branch.
