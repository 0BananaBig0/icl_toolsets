# Benchmark receipt counts implementation plan (Task222)

> **For agentic workers:** Use superpowers:executing-plans inline with one fresh whole-branch review.

**Goal:** Separate measured literal notes from expected query/scan counts.
**Architecture:** Reuse parsed output and rename uninstrumented expectation keys.
**Tech Stack:** Existing Python unittest/subprocess and unchanged native products.
**Spec:** docs/superpowers/specs/2026-11-28-benchmark-receipt-counts-design.md.

## Global constraints

Parent master 85751d4c072ac43a119d16b6e0ee423a2dedebaf; branch codex/benchmark-receipt-counts.
Four Python and three Markdown files only; 4,135 inputs/4,131 unaffected,
48 unchanged products, 117 stashes, seven package members. Preserve all limits,
existing execution/failure checks and shared counts helper semantics.

## Review focus

- Failed rows must report actual missing/partial note output, not desired notes.
- Successful source/flat/generated and unobserved controls retain correct counts.
- Uninstrumented provider/CSU/shift fields must not claim actual execution.
- Read-callback-history driver reuse retains its two-Apply fixture behavior.
- Renamed fields must reach every active consumer without rewriting history.

### Task 1: correct receipt semantics

**Files:** tests/src/pattern/benchmark_merged_readback.py;
tests/src/pattern/benchmark_mixed_callback_groups.py;
tests/src/pattern/test_benchmark_merged_readback.py;
tests/src/pattern/test_mixed_callback_groups.py; tracker/spec/plan;
private tests/.task222-* helpers; build/benchmark-receipt-counts-20261128.
**Interfaces:** Existing JSON receipts, actual_notes parse and two active tests.
No native/API changes or dependencies between separate tasks.

- [ ] Strengthen valid/missing-flat receipt assertions and mixed constant-label
  assertions; full normal modules must fail by assertions, not harness errors.
- [ ] Derive literal_notes from actual_notes; add expected_literal_notes and
  rename expected_provider_queries/expected_csu_count/expected_shifted_bits.
  Repeat full normal modules; expected GREEN.
- [ ] Run both whole modules normal/static/default-leak SAN, expected six reports,
  75 methods/zero skips. Package seven exact files; check 4,135 inputs/48 products,
  retained evidence and whitespace. Commit selected diff; task-done command:
  python3 tests/.task222-check.py.
- [ ] Fresh whole-branch review; regrade and repair blocking findings once.
- [ ] Merge, verify both modules normal/static, push, archive owned state,
  detach retained worktree and retire merged branch; close tracker and push.
