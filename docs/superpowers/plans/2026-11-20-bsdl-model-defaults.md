# BSDL model/default implementation
> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans.
**Goal:** Close the eight concrete BSDL audit groups within remaining item11.
**Architecture:** Existing reader, canonical BSDL identities, shared storage and
existing ICL capture mux/logic generation.
**Spec:** docs/superpowers/specs/2026-11-20-bsdl-model-defaults-design.md
**Global Constraints:** User excludes coverage1; root11 stays pending until its
other requirements are verified. No new parser/dependency/threshold/hardware
behavior. Parent master35c927f after Task139 integration (initial fork66fdaf5).
Task139 is independently reviewed, verified, merged and pushed.
## Review Focus
- Mask alternatives are union/unknown; shared-code constraints are intersection.
- Partial opcode overlap must not strengthen exclusive-region capture bits.
- Private declarations never become fallback BYPASS or freely driven hardware.
- BSDL casefolding must not change ICL signals or break bound ActiveSignals.
- Width omission follows original text order, not map ordering.
- Existing resource limits and large exact/masked opcode behavior remain bounded.
### Task 1: Complete audited BSDL metadata and TAP behavior
**Files:** src/pattern/pattern_bsdl.h/.cc; pattern_access_link.cc; pattern_tap.cc; pattern_tap_execution.cc;
tests/src/pattern_bsdl_test.cc; pattern_tap_test.cc; pattern_access_link_test.cc;
tests/src/pattern/test_raw_tap_builtin_dr.py; current model docs/tracker.
**Interfaces:** BsdlTap owns private-name set; BsdlRegister optional capture;
canonical BSDL instruction lookup; generated ICL retains current public API.
- [x] Extend native and CLI positive/negative cases; Expected RED on current
  reader/builder for all new groups, preserving old valid support.
- [x] Implement bounded reader canonicalization/lists/widths/captures/private
  names and implicit BYPASS. Expected native metadata GREEN.
- [x] Implement opaque private route and instruction-specific captures including
  compatible overlap without strengthening exclusive regions. Expected native
  and source/flat/generated/reset CLI GREEN.
- [x] Run focused/native/dynamic/static/SAN, full generator, style/C++17 and
  package checks. Expected exit0; do not run coverage closure.
- [ ] Document exact evidence/boundaries; commit/task-done, independent review,
  one necessary fix pass, merge/push and delete completed branch.
**Verification:** python3 tests/src/pattern/test_raw_tap_builtin_dr.py build/icl-pattern-generation
