# Native clock API contract coverage
Parent master869f31d; Task133. User standing instruction delegates routine
choices, execution, commit/push and prompt merge/delete. Extend existing tests
for reachable gaps in the unchanged strict coverage acceptance work.

Add one native TestClockApiContracts in tests/src/pattern_scheduler_test.cc.
Use a valid elaborated model with two root clocks, divided output and a child
clock mux whose inactive input leads back to that output. Before each malformed
Override, Check(out) and save first-root/divide2 timing. Empty/even arguments,
duplicate options after a valid mutation, unknown options after a valid mutation
and an override creating a cycle through an inactive mux alternative must reject
with the intended diagnostic. After each failure, saved and freshly checked
three-cycle waits remain exactly(first,6). A valid subsequent override to second
with divider3 retains the old snapshot then yields(second,9) after Check.
Data selector and clock hardware values must remain unchanged. These tests catch
removed argument validation, partial override publication and selected-only cycle
detection; expectations are literal hand-derived durations.

Add TestClockUnrelatedConsumers: real elaborated clocks plus unrelated a&b logic,
then a variant adding a valid named-chain scan client on separate pins. Both
original and serialized/loaded forms must Check(root)/Wait(root,7)==(root,7)
without changing data or interface pins. The pure combinational form also accepts
CheckTimeWait; the opaque interface variant still rejects time waits explicitly.
These additions catch safety propagation that wrongly marks unrelated clocks
unsafe; existing clock-fanout negative tests remain in both runner modes. Do not manufacture invalid Netlist objects.
Call both additions from default main and existing --clock-snapshots mode.

Baseline coverage has zero hits at clock73,80,104,125,387,392-394. This is an
existing-behavior test addition: measured missing coverage is RED, not a claimed
failing product behavior. GREEN requires intended errors, literal outcomes,
unchanged hardware and actual hits on all8 lines. No product changes, dependencies,
new target, failure injection, source exclusions or weakened100/90 thresholds.

Three alternative approaches considered: add CLI cases (shared frontend prevents
these native argument guards being reached); add fabricated IR (unnecessary);
reuse real native public APIs (chosen, compact and exposes atomicity). Workload
is fixed small models and a dozen API calls, bounded memory, no hot-path change
or speedup claim.

Only existing native test plus this plan/spec and human qualification docs change.
Fresh normal project check, changed-native ASan/UBSan, format/tidy/strict17,
fresh focused profiles and full raw coverage, Rocky changed-native checks run.
Reuse other Task132 qualification only by3994-file input comparison excluding
the single changed native test and exact product fingerprints; no fresh vendor,
remote CLI or archive/install run is claimed. Refresh source archive and compare
all tracked shipped bytes; unmodified shipped products remain identity-qualified.
One fresh independent final review, at most one necessary fix pass, then commit,
merge direct parent master/delete branch/archive/sync/push. Strict release
coverage remains open if measured below100/90.
