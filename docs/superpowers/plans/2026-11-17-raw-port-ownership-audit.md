# Raw Port Ownership Audit Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Establish measured native/vendor boundaries for taken ports and raw scans.
**Architecture:** Reuse existing vendor model/driver and current native CLI; docs only.
**Tech Stack:** Existing Python/SSH/Tessent2026.2 and native dynamic/static tools.
**Spec:** docs/superpowers/specs/2026-11-17-raw-port-ownership-audit-design.md

## Global Constraints

Parent master26ce069; task/raw-port-ownership-audit. Only four docs below change.
Eight initial probes plus two adaptive follow-ups under own /tmp/icl-raw-port-ownership-20261117; sequential
120s bounded Tessent calls. Do not mutate product/tests or waive100%/90% gate.
Reuse current full/SAN/platform evidence only after exact input/product hashes.

## Review Focus

- Distinguish resource establishment, raw queue/apply failures and valid exports.
- Compare vendor/native on identical declared model and PDL bytes.
- Preserve unchanged driver and identify the two deliberate model additions.
- Separate normative rules, older manual and measured current vendor behavior.
- Bounded probe results must not imply general port/IEEE/vendor qualification.

### Task 125: Audit established port reservation raw scans

**Files:** docs/pattern-generation.md; docs/pattern-model-coverage.md; this plan;
docs/superpowers/specs/2026-11-17-raw-port-ownership-audit-design.md.
**Interfaces:** Existing ReservedRawTop run.tcl; current CLI; archived source and
runtime manifests. Existing test_raw_established_port_ownership_is_explicitly_unsupported.

- [x] Record base/brief, IEEE7.9.19/manual evidence, current guard/callers and
      eight initial bounded programs plus two follow-ups; hash unchanged driver/model additions and runtime.
- [x] Run native dynamic/static on identical inputs and isolated vendor probes;
      retain actual statuses/exports/failure stages and submitted/fetched hashes.
      Expected evidence for all ten, with actual outcomes determining findings.
- [x] Verify exact runtime/product/platform/full/SAN evidence reuse; document
      bounded findings and open gaps. Freeze four-file candidate; independent
      final review and one necessary correction pass.
- [x] Verify final bytes, commit/merge master/delete branch/task-done/archive/sync.

Pre-flight: no product interfaces changed. Design/plan self-review complete.
Ruling: audit/docs only, no behavior edit or manufactured failing TDD test.
Routine execution and bounded temporary vendor probes delegated by standing user.


### Raw port ownership audit (2026-11-17)

A documentation-only audit at `26ce069` measures established port ownership
around root raw scans. IEEE1687-2014 sections 7.9.12 and 7.9.19 (printed 168–169,
178–179) describe full network control by iScan and the distinction between
explicit PDL assignments and retargeter changes to taken resources. The local
Tessent Shell Reference Manual v2023.4 iTake topic idd8a68c5f also documents
establishment by iState; its iScan topic idb66e0799 describes scan loads after
ordinary retargeting and modeled state simulation. These older references are
kept separate from the following measured Tessent 2026.2 results.

Ten isolated programs reuse the unchanged qualified ReservedRawTop driver and
model, adding only scalar DataOutPort hold (constant 1) and held (value[0]). The
initial model with an extra DataInPort and wide DataOutPort was rejected by the
native raw-profile guard before reservations; those attempts are preserved and
are not counted as ownership results. The corrected control passes both tools.

| Corrected program | Tessent 2026.2 | Current native dynamic/static |
|---|---|---|
| Baseline reset and eight-bit raw load | Export succeeds | Both pass |
| Taken select established at 1 or 0, then raw load | Both exports succeed | Both reject at final Apply: ScanRegister-storage-only reservation guard |
| Taken constant output established by applied write, then raw load | Export succeeds | Same raw reservation guard |
| Taken observed value[0] established at 0; raw 0x5a or 0xa5 | Both exports succeed | Same raw reservation guard |
| Taken ScanInPort si, then iWrite si 0, before raw 0 or transient-changing raw data | Both fail at setup line 5: invalid iWrite target | Both fail at that write; raw reservation behavior is not reached |
| Taken inactive select, successful raw load, then ordinary iWrite value 1 | Fails later Apply at line 9, reporting iTake sel value 0 | Rejects earlier raw Apply at line 7 |
| Taken observed output, successful raw 0xa5, then ordinary iWrite value[1] 1 | Fails later Apply at line 10, reporting iTake held value “-” | Rejects earlier raw Apply at line 8 |

There are six vendor successes and four stage-specific failures. Five vendor
successes expose native source-acceptance differences at the raw guard. The two
follow-ups show that acceptance of a raw operation does not establish release
of the earlier reservation: later ordinary retargeting still reports its iTake
constraint. The held-output diagnostic does not specify a binary locked value;
this audit does not infer that value. In particular, accepting a raw operation
must not be generalized into automatic release or refresh of every port lock.
The original native unsupported test remains an implementation boundary, not an
IEEE prohibition. This audit does not choose a general port-ownership policy.

All 30 submitted/fetched input hashes match. Vendor calls run serially with 120s
timeouts and complete/error anchors. Initial SSH launch timed out locally after
30s; the existing run was inspected and fetched without relaunching it. Actual
probe exits, not transport status, determine results. All six vendor exports
replay through both current native variants after an explicit call to their
exported procedure. This is replay evidence, not a new directional certificate
or a full ordered-group equivalence claim. Native failed runs retain their
previous result artifact. Exact programs, logs, stages, hashes and exports are
in `build/raw-port-ownership-probe-26ce069/`; first-model evidence is adjacent.

All 3,994 runtime inputs and ten local product fingerprints remain unchanged.
Task 123 full integration/SAN and Task 124 five-tool platform evidence are reused
by hash; no new full regression, coverage or performance measurement is claimed.
Strict coverage remains unmet at 96.5% lines and 62.7% raw branches against 100%/90%.
General port/raw ownership, broader model/vendor equivalence and other recorded
acceptance work remain open. No product/test/Makefile edits or push.
Independent final review found no Critical, Important, or Minor issues in this documentation-only ownership audit. The bounded raw port ownership audit is ready for integration.
