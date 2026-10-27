# Persistent and assumed resource values

> Execute inline with Superpowers TDD, independent review and full regression.

**Goal:** Implement documented Tessent iTake defaults, persistent call lifetime,
and protection established by iState using the existing ScanRegister backend.
**Architecture:** Normalize default resources to the runtime's effective instance
path; store persistent ownership in the existing main scope. Record explicit
iState assignments through the same resource-value tracker as applied writes.
**Source:** Tessent Shell Reference Manual v2023.4, `iTake`, local topic
`tshell_ref/topics/Command_Itake_idd8a68c5f.html`; IEEE 1687-2014 §§7.9.19–22
continues to govern ordinary ownership and state assumptions.

## Scope and decisions

- `iTake [resource] [-persistent]` uses the effective iCall/iPrefix instance when
  resource is absent or empty. The parser already recognizes the optional
  resource and persistence switch; add the documented explicit-empty equivalent.
- Persistent acquisitions survive all iCall returns within the ordered CLI
  input stream. Explicit release removes them, including from descendants.
  No open_pattern_set/test_setup/test_end lifecycle is implemented or implied.
- `iState` on a taken ScanRegister establishes or replaces protection. A later
  iTake does not retroactively protect an earlier assumption. Preserve numeric
  and -LastWrittenValue restrictions, pending-queue rejection and unknown-value
  rejection for protected resources. iReset with protected resources remains
  explicitly unsupported.
- Reuse parsers, scope counters, scheduler guards and source traces; no new
  dependency, token family or scheduler path. Ports and slice/alias resource
  names remain outside current execution support.

## Tasks

- [x] Extend `tests/src/pattern/test_resource_reservations.py` with persistent
  return/release, repeated overlapping acquisitions, default instance/prefix,
  assumption-before/after-take, partial assumptions, pending/unknown failures,
  ordered input files, merge barriers and source/flat/generated fixed points.
  Extend existing PDL tool tests for the empty-resource syntax. Observe failures.
- [x] Normalize iTake in `pattern_runtime.cc`, adjust ownership in
  `pattern_resources.cc`, and record explicit iState in Generate. Keep parser
  validation narrow and omit option/default words from object references.
- [x] Run focused dynamic/static tests, independent review, full `make check`,
  format checks and leak-enabled sanitizers. Update README/developer docs with
  documented vendor scope and remaining limitations. No commit/push requested.

## Review focus

Persistent ownership must not be lost at a sibling call; local returns must not
drop an overlapping persistent take; release/reacquisition must not retain a
stale value. Default targets must use the call's effective prefix and survive
flattening. State assumptions must not execute hardware or bypass queue and
unknown-state checks. Reset-only rewriting must retain the reservation guards.

## Evidence

Existing uncommitted work is preserved. No new runtime, solver path or dependency
was introduced. Persistent ownership uses the existing main-scope set, so its
per-acquisition cost remains the existing set/map insertion cost. Scope traversal
and protected scheduling limits are unchanged.

- RED: 17 resource tests exposed eight failures in the new cases;
  explicit-empty parser tests exposed two failures. Logs:
  `build/persistent-reservations-red.log`, `build/persistent-parser-red.log`.
- GREEN: 17 resource tests, shared parser option tests, dynamic/static generator
  and runtime targets passed (`build/persistent-focused-regression.log`).
- Independent review found no defects and reran all 17 resource tests.
- ASan/UBSan with LeakSanitizer enabled passed 17 resource tests, 26 generator
  tests, and native scheduler/runtime tests. Logs:
  `build/persistent-reservations-sanitize.log`,
  `build/persistent-cli-sanitize.log`, `build/persistent-scheduler-sanitize.log`,
  `build/persistent-runtime-sanitize.log`. External Z3 is uninstrumented.
- Full `make check BISON=/usr/bin/bison -j4` passed (exit 0), using
  `ICL_REF_DIR=/tmp/icl-reference-honza-20261021` and the existing reference venv
  on PATH; log: `build/persistent-regression.log`. Includes seven offline Tessent
  goldens, 334/334 grammar alternatives, 119/119 scanner rules, and Vim/Neovim
  checks on 38,644 ICL and 812 PDL inputs each. Style and diff whitespace checks
  passed. README and developer documentation describe the supported scope.
- Fresh vendor execution and strict authored-C++ coverage have not been rerun.
  The full pattern-generation requirements are not yet complete; the remaining
  model, scheduler and vendor-equivalence gaps are retained in the coverage map.
