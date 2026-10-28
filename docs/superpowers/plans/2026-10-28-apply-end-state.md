# Apply-end-state implementation plan

> **For agentic workers:** Use superpowers:executing-plans with TDD and an independent final review.

**Goal:** Implement IEEE 1687-2014 6.5.2(h) end-state directives in the existing serial and combinational schedulers, continuing the larger retargeter requirements.

**Architecture:** Resolve directives once to storage/primary-input bits. Keep default fill separate from hardware state. Constrain each apply's final values, except explicitly written bits; restore temporary overrides at end of program. Reuse symbolic planning, concrete replay, atomic publication and shared serialization.

**Tech Stack:** Existing C++/Z3 scheduler, Python CLI regression.

**Spec:** User's pattern-generation requirements; IEEE 1687-2014 6.5.2(h), printed pp.136–137; docs/pattern-generation.md.

## Global constraints

- Existing repository edits and inline execution are authorized, including master and push.
- Keep sanitizer/leak checks, immutable standards, and separate ICL/PDL frontends.
- No generated build outputs in Git. New tests under tests/src/pattern.
- Resolve directives in bounded graph traversal; retain existing solver budgets. No exhaustive schedule enumeration.
- This plan does not establish completion of the larger retargeter: addressed models, multi-controller scheduling, event models and remaining runtime commands still require implementation and qualification.

## Review focus

- Partial and inverted aliases must restore only declared bits.
- Overlapping, unknown, reset-incompatible and gated targets must reject atomically.
- Explicit writes last for one apply; final restoration must not add another scan on replay.
- Reserved values must not silently change during automatic restoration.
- SIB control defaults must hold after incidental prerequisite scans, with original/flat/PDL replay consistency.

### Task 1: Directive lifecycle

- [x] Add tests for direct/alias writes, following read/empty apply, final restoration, partial inversion, primary inputs and invalid declarations. Run against old executable: expect rejection of valid directives.
- [x] Resolve directives in ScanScheduler::Impl, preserving provenance and enforcing normative legality.
- [x] Integrate defaults with final-state constraints and default fill; use an empty ScanScheduler::Apply from Generate at completion using the same output serializer.
- [x] Run focused dynamic/static generator and scheduler regression; expect exit 0.

### Task 2: Qualification and publication

- [x] Exercise nested SIBs, reservations, atomic failures and round trips in regression.
- [x] Obtain independent review, address important findings with RED/GREEN evidence.
- [x] Run full make check, Google style checks, ASan/UBSan with LeakSanitizer.
- [x] Update README and docs with supported behavior, evidence and remaining requirements; commit and push.

## Continuation sequence

After directive lifecycle: selectable/addressed DataRegister events using AddressResolver and NetworkState; OneHot contributors; mixed pin/scan scheduling; multi-root/controller scheduling; remaining idle and reservation semantics. Each needs an independently validated event model and directional vendor evidence before claiming completion.

## Outcome

Implemented and reviewed against 97b27d8. Full regression, focused dynamic/static
checks, Google style and ASan/UBSan/LSan passed. One new Tessent 2026.2 forward
certificate qualifies TAP/SIB end-state restoration; the other three vendor
comparisons remain unqualified and are documented without comparator changes.
Reviewer finding (forwarded-port X default) was reproduced and fixed in both
polarities. Final-restoration failures now retain source context, and conflicting
reserved defaults reject before scan search. See docs/pattern-generation.md and
docs/tessent-goldens.md for evidence and remaining retargeter requirements.
