# iWrite option payload correctness
> REQUIRED SUB-SKILL: superpowers:executing-plans.
Spec: docs/superpowers/specs/2026-11-22-write-ignore-value-design.md
Base: master c558013; existing delegated implementation decisions authorized.
Global Constraints: retain existing explicit-load policy, original evaluated
source arguments, procedure bytes, queues/reset semantics and resource limits.

## Review Focus
- Normalize only validated iWrite operands, before all internal consumers.
- Preserve once-only evaluated trace arguments and required data/care bits.
- Invalid operands still fail when reset cancels their queue.
- No promise of load elimination, observations or vendor equivalence.

### Task 1: Shared runtime boundary and native regression
Files: src/pattern/pattern_runtime.cc; tests/src/pattern_runtime_test.cc.
- [x] RED valid modes retain their data/original trace and validate numeric width.
- [x] Normalize to the existing literal operation pair after validation; GREEN.

### Task 2: CLI consumers, replay and scale
Files: new tests/src/pattern/test_write_value_options.py; Makefile.
- [ ] RED primary/scan/data/alias/enum/merged value loss and atomic invalids.
- [ ] GREEN source/flat/generated replay, once-only evaluation and cancellations.
- [ ] Measure matched16/128-apply controls and flagged inputs, record hashes/costs.

### Task 3: Qualification and current evidence
Files: current guide/model/tracker, this spec/plan.
- [ ] Required native/dynamic/static/SAN/quality/package checks and frozen inputs.
- [ ] Record supported scope, permission/policy distinction and verification.
