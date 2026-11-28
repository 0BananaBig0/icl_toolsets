# Inventory output errors implementation plan (Task218)

> **For agentic workers:** Use superpowers:executing-plans inline, with one fresh whole-branch review.

**Goal:** Report buffered stdout failures from the inventory helper accurately.
**Architecture:** One final flush before the existing stream-state check, real
native failure regression, and affected helper/corpus verification.
**Tech Stack:** C++17 iostream, existing Python unittest and Makefile targets.
**Spec:** docs/superpowers/specs/2026-11-28-inventory-output-errors-design.md.

## Global constraints

Parent master 54bb3792e28be84787cbcc48a3964af31bc22bd2;
branch codex/inventory-output-errors. No new dependencies/API/runtime changes.
Preserve 117 stashes, default sanitizer flags and existing resource limits.
Only the C++ inventory helper, its existing mirrored Python tests, tracker and
this spec/plan change. All 4,133 unaffected inputs and 46 unaffected products
stay identical; rebuild the normal and sanitizer inventory products only.

## Review focus

- Small buffered writes must fail at final flush, not silently at process exit.
- Successful JSON batches retain the existing order/content and one-batch I/O.
- Missing input and backend/protocol errors keep existing diagnostics and guards.
- Linux /dev/full evidence must exercise the actual native helper in both builds.
- Packaging and retained evidence must bind to the final inputs and products.

### Task 1: detect buffered output failure

**Files:** tests/src/pattern/pattern_netlist_inventory.cc;
tests/tests/src/pattern/test_audit_model_corpus.py; docs/remaining-work.md;
this spec/plan; private tests/.task218-* helpers and ignored evidence directory
build/inventory-output-errors-20261128.
**Interfaces:** Existing line-delimited JSON inventory and Python audit consumer;
parent manifests. Produces unchanged success protocol and accurate exit 2 on failure.

- [x] Add real short-output /dev/full test; run against parent binary. Expected
  one assertion failure (actual exit 0 rather than 2), not a harness error.
- [x] Add one final stdout flush; rebuild both inventory variants. Expected
  complete helper/protocol module and actual corpus audits pass in both variants.
- [x] Fix Task217 summary spacing while preserving all values. Run three C++
  quality checks, source package/build/install, exact five-member package and
  final input/product/whitespace verification. Commit selected diff; task-done
  runs python3 tests/.task218-check.py, expected all checks pass.
- [x] Fresh whole-branch review; regrade and fix blocking issues once if needed.
- [x] Merge master, verify merged results, push, archive owned ledger/helpers,
  retire branch and preserve reusable detached worktree.

Completed at `b16483e`, merged and pushed as `cfc0637`. Independent
review: 0 Critical, 0 Important, 0 new Minor issues. Two inherited findings
are resolved and 17 remain deferred. All 13 qualification gates and five
merged-result checks pass. All 117 stashes are preserved. Broader generator
evidence remains retained by identity.
Evidence: `build/inventory-output-errors-20261128/`.
