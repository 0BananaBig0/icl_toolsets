# Wide decimal constants implementation plan

> REQUIRED: superpowers:executing-plans, native inline execution.

**Goal:** Decode bounded decimal bit patterns independently of checked integer arithmetic.
**Spec:** docs/superpowers/specs/2026-11-25-wide-decimal-constants-design.md
**Parent:** master21a6aea7f693abb3ad8f82f04b7fe7532e200824.
**Owned branch:** codex/wide-decimal-constants.
**Constraints:** Preserve1,048,576-bit values,64-bit arithmetic/index limits,
10s/512MiB measurements,117 stashes/25 inherited Minors; item1 excluded.

## Review focus

- Decimal digits cannot pass silently after overflow or normalization.
- Zero, leading zeros, chunk boundaries and significant-bit rejection agree with hex/binary.
- Unsized flags and width parameters retain their original semantics.
- Native arithmetic/index errors remain errors and failed outputs remain untouched.
- Performance measures actual decoding and checks complete expected bits in every mode.
- Corpus registry changes reflect actual repaired defaults, without weakening required models.

### Task 1: Qualify wide decimal bit patterns

**Files:** Modify src/pattern/pattern_expression.cc and
tests/src/pattern_expression_test.cc. Create tests/src/pattern/test_wide_decimal_constants.py,
tests/src/pattern/benchmark_wide_decimal_constants.py and
tests/data/pattern/wide_decimal_constants.txt. Modify Makefile, the existing
corpus acceptance registry where required, and current guide/model/tracker docs.

**Interfaces:** Keep EvaluateInteger/Constant::AsInteger and public signatures.
Literal uses bounded limb conversion for plain and based decimal data only.
Native test-only benchmark modes --wide-decimal/--wide-hex verify the same fixed
fixture and print their checked case count. Defaults still run the full suite.

- [ ] Step1: Add native decimal boundary/scale cases and eight CLI methods for
  the spec's contexts; retain10 all-ones width fixtures64/65/96/127/128/256/
  1024/4096/16384/65536. Expected: immutable parent fails decimal positives
  through its64-bit literal limit; hex controls and existing negatives pass.
- [ ] Step2: Add the bounded decoder at Literal using nine-digit chunks and
 32-bit limbs. Expected: exact binary payloads, sizing/flags, resets/captures,
  sparse/inverted values and source/flat/generated replay pass; integer/resource
  limits and atomic rejection remain.
- [ ] Step3: Add the small native performance recorder and compile a parent
  test harness against the immutable parent expression object. Expected:
  twelve measured processes verify all10 fixture values within10s/512MiB;
  parent decimal failure remains RED; no unsupported parent mode is accepted.
- [ ] Step4: Run full normal/static generator and30 fresh native suites,
  affected CLI/corpus, quality, package, existing work/allocation/reporting
  probes. Expected: eight repaired corpus defaults become canonical and the
  other original failures remain explicit; exact input/product hashes retained.
- [ ] Step5: Record actual results, commit and run one fresh final review;
  re-grade/fix blockers once test-first and record deferred Minors/rulings.
  Expected: no unresolved Critical/Important finding.
- [ ] Step6: Merge to master, run merged checks with identical qualified
  inputs/products, push, delete owned branch and archive only this ledger.
  Expected: clean pushed master,117 stashes preserved.

Focused: python3 tests/src/pattern/test_wide_decimal_constants.py build/icl-pattern-generation
Completion: python3 /tmp/wide-decimal-constants-completion.py

Self-review: one shared literal conversion task; no separate builder. No RTL,
new dependency, public configuration, arithmetic widening or AccessLink change.
