# ScanRegister resource reservations implementation plan

> Execute inline with Superpowers TDD, one independent review and full regression.

**Goal:** Support standard iTake/iRelease for ScanRegisters and instance storage,
without allowing the retargeter to change protected applied values.
**Architecture:** Track resource ownership by the existing operation call stack.
Keep taken-but-unwritten resources free. After an explicit applied write, pass
protected register values into the shared scheduler; enforce them at every scan
state and concrete replay. Explicit user writes may replace a protected value.
**Spec:** IEEE 1687-2014 §§7.9.18–20; approved pattern-generation design.
**Tech stack:** Existing C++17, Z3, Tcl runtime and Python tests.

## Decisions and scope

- IEEE §7.9.20(e) and Tessent IJTAG User's Manual v2023.4 “How to Run iCalls in
  Parallel” permit a callee to explicitly release inherited ownership. An
  implicit return releases only the callee's own acquisitions.
- Preserve runtime operations and source-action indices. Merge calls containing
  resource commands remain serial. Fully generated output resolves reservations
  into concrete writes/applies; it does not reintroduce iProcs or resource scopes.
- Initially support full named ScanRegister resources and ScanRegisters within
  instance scopes. Reject port/alias/slice/persistent resource requests explicitly.
  Taking an instance does not enable currently unsupported port transactions.
- Use the bounded scheduler when active written reservations exist; ordinary
  unreserved large groups retain the greedy path. No global optimality claim.
- Reset or state assumptions with active written reservations remain unsupported
  until their reservation-state semantics are implemented. Idle replay must
  preserve protected register values.

## Tasks

- [x] Add regression cases before implementation: protected closed SIB rejects
  target access without publishing; release enables access; take-before-write is
  free; take within the pending group works; explicit updates are allowed;
  inheritance, overlapping acquisitions, implicit and explicit releases;
  instance scope; alias/partial user writes; nested merges; fixed points;
  unsupported targets; reset-only preambles cannot override a reservation.
- [x] Add `pattern_resources.h/.cc` with call-scope ownership and applied-write
  recording. Integrate into Generate without changing Runtime operations.
- [x] Extend ScanScheduler::Apply with protected values and explicit-write
  permission. Constrain old/new monotonic transitions, concretely validate
  protected states, and retain atomic failure. Automatic reset-only writes must
  use strict preservation. Add native rollback/guard tests if needed.
- [x] Run focused, dynamic/static and full regression, leak-enabled ASan/UBSan,
  independent review and a representative many-register measurement. Update
  customer/developer docs with exact support and remaining limits.

## Review focus

No premature lock after iTake; no implicit acquisition from preparatory writes;
ancestor ownership survives child return; inherited explicit release works;
repeated calls have distinct identities; failed schedules and reset-only helpers
cannot corrupt ownership/state; partial writes do not change untouched locked
bits; generated-pattern and flattened-netlist replay remain stable.

## Evidence and rulings

- Thirteen focused tests pass (`build/resource-reservations-reviewed.log`).
  The initial scalar alias fixture was corrected to a width-matched vector;
  corrected tests fail against the saved prior binary for unsupported iTake
  (`build/resource-reservations-corrected-red.log`).
- Independent review found reset-only preambles could accidentally acquire a
  new lock when a resource was taken but not yet explicitly written. The new
  reproducer failed before the fix (`build/resource-reservations-review-red.log`)
  and now passes. Reject that unsafe insertion before artifact publication;
  normal flattened generation remains supported. Re-ingestion without the take
  also passes, and failed publication preserves prior outputs.
- Fixed a Clang C++17 warning by capturing a local expression instead of a
  structured binding; no C++20 extension is required.
- Ruling: protect ScanRegister storage first, serialize reservation-bearing
  calls, and use bounded solving for written locks. Broader resource types,
  protected reset/state assumptions and a reservation-aware greedy path remain
  explicit outstanding work, not silently accepted behavior.
- Work continues in the authorized existing workspace; earlier uncommitted
  source organization and scheduler changes are preserved.
- Leak-enabled ASan/UBSan passed thirteen resource cases, 26 existing generator
  cases and native scheduler tests, including atomic failed-apply and failed-idle
  checks. Logs: `build/resource-reservations-sanitize.log`,
  `build/resource-cli-sanitize.log`, `build/resource-scheduler-sanitize.log`.
  Tests ran outside the ptrace sandbox with `ASAN_OPTIONS=detect_leaks=1`;
  the external Z3 library remains uninstrumented.
- Measured 128 protected registers plus one target, 10,338 source bytes: 0.073 s
  CLI time; 0.13 s / 57,616 KiB peak RSS including the Python harness. Command:
  `/usr/bin/time -f 'wall=%e peak_rss_kib=%M' python3
  tests/src/pattern/test_resource_reservations.py build/icl-pattern-generation
  Reservations.test_many_reserved_registers`. WSL2 Linux, g++ 16.2.1 `-O2`,
  Z3 4.13.3. Log: `build/resource-reservations-measurements.log`. This is a
  supported-scale measurement, not a comparative speedup or general bound;
  replay checks cost work proportional to protected bits at each clock edge.
- Final full `make check BISON=/usr/bin/bison -j4` exited 0 after the review fix,
  with `ICL_REF_DIR=/tmp/icl-reference-honza-20261021` and the existing reference
  venv on PATH. Log: `build/resource-reservations-final-regression.log`.
  Dynamic/static resource tests passed; seven offline vendor goldens matched;
  grammar 334/334 and scanner 119/119 rules were exercised; Vim and Neovim each
  checked 38,644 ICL and 812 PDL files. Corpus audit found zero duplicate-file
  groups. Format checks and `git diff --check` passed; sanitizer compilation
  reported no warnings.
- Final ruling on review scope: inherited explicit release follows the cited
  Tessent guidance and IEEE §7.9.20(e); the review did not independently resolve
  the tension with §7.9.20(f). This is documented vendor-compatible behavior,
  not a claim that every reading of the standard agrees.
- Fresh Tessent equivalence and strict authored-C++ coverage were not rerun;
  no vendor golden was added. No commit or push was requested in this increment.
