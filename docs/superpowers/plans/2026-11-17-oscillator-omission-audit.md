# Oscillator Omission Audit Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Record measured omitted-source oscillator failures without inventing semantics.

**Architecture:** Documentation-only audit of existing source/standard and reused
vendor driver; retain existing rejection behavior and preserve raw evidence.

**Tech Stack:** Existing Tessent2026.2 probe, IEEE1687-2014, current native CLI tests.

**Spec:** docs/superpowers/specs/2026-11-17-oscillator-omission-audit-design.md

## Global Constraints

Parentcee5dcf, task/oscillator-omission-audit. Four docs only; no product/test/build
changes or new maintained test scripts. Retain raw negative exits and exact
failure stages; no vendor certificate, IEEE prohibition or full support claim.
Strict100%lines/90%rawbranches remains required and unmet.

## Review Focus

- ClockOverride acceptance distinguished from later iClock failure.
- Optionless syntax rejection distinguished from undriven-source failure.
- Fresh successful control and exact fetched inputs establish probe validity.
- v2023.4 manual, measured2026.2 and normative IEEE rules kept separate.
- Native early guard is retained; no inferred frequency or restoration model.

### Task 122: Record oscillator omission evidence

**Files:** docs/pattern-generation.md; docs/pattern-model-coverage.md; this plan
and its spec only. Evidence build/oscillator-omission-probe-cee5dcf/.

**Interfaces:** Existing vendor run.tcl/model.icl copied unchanged; current
SystemClocks::Override and existing oscillator target rejection method unchanged.

- [x] Record base/brief and existing discovery evidence. Expected: source guard
      inspected; IEEE clauses/manual Context/Mode verified; no implementation.
- [x] Verify7 vendor runs/21 fetched hashes and actual abort lines. Expected:
      control0/export; three subsequent-iClock failures1; optionless syntax1.
- [x] Verify existing native rejection method dynamic/static/SAN. Expected: all3
      exit0 with leak detection;3994 runtime inputs and product hashes unchanged.
- [ ] Document versioned results and limitations; one independent final review
      and necessary correction pass. Candidate/final verified4docs, merge master,
      delete branch/task-done/archive/sync. Expected: audit complete, broader
      strict/model/vendor work remains open. No push.

Pre-flight:no shared implementation interfaces. Reuse completed authorized
investigation and checks; no implementation brainstorming/TDD change is involved.
Plan/spec self-review complete; routine documentation execution delegated.

Adaptive stage check: two additional no-refresh cached-wait patterns succeed
in Tessent2026.2, with child4/child4 and child4/root12/root12 traces. The native
tool currently rejects both early (exit2); record this remaining compatibility
gap explicitly. Do not implement it within this documentation-only audit.

### Oscillator omitted-source audit (2026-11-17)

Seven isolated Tessent2026.2 probes distinguish a factor-only override from a
subsequent clock check. They reuse the unchanged explicit-oscillator driver and
model: child.osc has Period10ns, and root is configured at100ns. The context is
patterns -ijtag / analysis with an open pattern set. All21 fetched input hashes
match the submitted files.

| Pattern | Vendor result | Observed stage or exported waits |
| --- | --- | --- |
| Existing explicit-source control | exit0 | child.osc2, child.osc2, root1 |
| Multiplier2, then fresh iClock | exit1 | iClock fails with undriven child.osc source |
| Divider3, then fresh iClock | exit1 | iClock fails with undriven child.osc source |
| Explicit source/divider3, then multiplier2 and fresh iClock | exit1 | final iClock fails with undriven child.osc source |
| Explicit source, then optionless override | exit1 | iClockOverride syntax rejection |
| Multiplier2, retain previous clock check | exit0 | child.osc4, child.osc4 |
| Explicit source/divider3, then multiplier2, retain previous check | exit0 | child.osc4, root12, root12 |

The override itself is accepted in the factor-only cases. A previous checked
clock remains usable in the two cached-wait patterns; those patterns currently
fail in our tool at the earlier explicit-source guard (exit2). This is an
identified compatibility gap, not a completed clock feature. The guard stays
unchanged in this documentation-only audit; a separate implementation must
preserve the cached duration without inventing an intrinsic frequency or
self-referencing clock source. Fresh-check failures and optionless rejection
must remain distinct from the successful cached-wait paths.

IEEE1687-2014 clauses7.9.15b and7.9.16c-e (printed174-175/PDF188-189) define
oscillator roots and optional override fields/defaults. Vendor failure does not
prove an IEEE prohibition. The v2023.4 Shell Reference Manual iClockOverride
topic (Command_Iclockoverride_id87a56b58.html) describes different omitted-field
behavior; it is not evidence for measured2026.2 behavior. IEEE-only intrinsic
restoration and general oscillator override semantics remain unqualified.

Evidence is build/oscillator-omission-probe-cee5dcf/ and its snapshot-probes/:
original inputs, raw logs/exits/exports, native gap reproductions, reference
hashes and audit facts. Negative runs have no export. The existing native
oscillator-target rejection method passes dynamic/static/leak-enabled SAN;
that verifies the present restriction and does not close the cached-wait gap.
All3994 runtime inputs and20 product fingerprints remain unchanged. No formal
certificate, broader equivalence, post-edit coverage or performance claim.
Independent final review and integration are pending. Strict coverage and
broader model/vendor requirements remain open. No push.
