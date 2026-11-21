# DataRegister callback model
> REQUIRED SUB-SKILL: superpowers:executing-plans.
Spec: docs/superpowers/specs/2026-11-22-data-register-callback-model-design.md
Base: master 8ce41f4; existing delegated implementation choices authorized.
Global Constraints: no runtime callback execution or invented storage effects;
reuse constant evaluation and flat origin metadata; preserve all bounds.

## Review Focus
- Separate placeholders and literal text; resolve parameters in declaration scope.
- Canonical flat ICL preserves namespace components, values and original scope.
- Invalid type combinations fail before output publication; no broad API churn.

### Task 1: Typed callback declarations and validation
Files: src/pattern/pattern_netlist.{h,cc}; tests/src/pattern_netlist_test.cc.
- [x] RED descriptor and mutually exclusive register-type cases.
- [x] Resolve typed names/arguments once and validate types; GREEN native check.

### Task 2: Canonical flat representation and replay
Files: src/pattern/pattern_flatten.cc; tests/src/pattern_flatten_test.cc;
new tests/src/pattern/test_data_register_callback_model.py; Makefile.
- [ ] RED source/flat replay, parameter specializations and atomic negatives.
- [ ] Reuse serializer literals and origin metadata; GREEN focused checks.
- [ ] Measure representative larger callback models against matching controls.

### Task 3: Qualification and current evidence
Files: current guide/model/tracker plus this spec/plan.
- [ ] Native/dynamic/static/SAN/style/C++17/package checks and hashed identities.
- [ ] Record remaining invocation work and prepare final independent review.
