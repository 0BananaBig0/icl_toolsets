# DataRegister state assumptions

Task34 continues missing PDL/state-model semantics. IEEE1687-2014 §7.9.22
explicitly permits scan or data registers and top-level DataInPort/SelectPort
as iState targets. It describes an event-free state snapshot. The current
shared validator and NetworkState::Assume accept only ScanRegister storage.

Allow DataRegister in the shared validator. Validate the value and assign its
held storage directly, preserving ScanRegister shift storage and avoiding
resets, data-register updates, clock edges or callbacks. Reuse scheduler alias/
slice handling, controlling-fill updates, reservations and TAP delegation.
Controller-owned instruction state and invalid port/observation targets remain
rejected. LastReadValue/LastMiscompareValue observation history remains a
separate unfinished command feature.

Touch state validation/assignment, native state and scheduler tests, CLI tests,
Make registration, vendor fixture/evidence and model ledger. The operation
remains O(changed bits), with existing bit/state limits and no hierarchy scan
per assignment. Check wide state assumptions without claiming solver throughput.

- [x] Add failing native/CLI cases for DataRegister whole/sliced/aliased
  assumptions, no-event behavior, partial unknowns, reservations, hierarchy,
  reset dependencies, rollback and unchanged invalid target rules.
- [x] Extend the shared state boundary and reuse callers unchanged.
- [x] Run focused/full dynamic/static, leak-enabled sanitizer, style/naming
  and independent review; record vendor behavior and model limits.

Validation batching: Tasks33–34 share one final full dynamic/static regression
after both focused/native/sanitizer checks. The source is fixed before that run;
intermediate or stopped runs are not completion evidence.

Review exposed no-event reset transitions in pure, mixed and hidden-event
planning. Gate state settlement and controlling-fill reset by an actual scan
or primary drive, matching concrete replay. Red regressions were recorded before
each correction. DataRegister-controlled scan resets require the separate
reset-dependency completion step; retain an explicit boundary test here.

Combined Task33–34 regression: build/access-state-regression.log/.exit0;
final access-state sanitizers, naming/format and independent review pass.
See the generator guide for exact counts, vendor evidence and remaining limits.
