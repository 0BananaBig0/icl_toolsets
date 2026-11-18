# Absolute-time waits on models without clock-driven state
Parent master d7d6d7f; task/invariant-time-waits, Task132. Standing user
authorization covers completing approved PDL requirements and routine scoped
implementation, qualification, merge/delete and push.

IEEE1687-2014 7.7 printed154/PDF168 defines tvalue;7.9.17 printed175-176/
PDF189-190 defines iRunLoop -time tvalue as a separate alternative to cycle
counts with -tck/-sck. It must elapse at least the specified time, keeps clocks
free-running and does not apply queued operations. Do not invent combined
-time/-sck syntax or an external tester period. Local source identity/pages are
recorded in /tmp/icl-time-normative-source.json. Tessent2023.4 iRunLoop topic
Command_Irunloop_id1c6d2724.html separately documents tester quantization and
TCK waveform policy; these external settings are not present in native ICL.

A fresh Tessent2026.2 source control accepts and exports iRunLoop -time 1.25us
unchanged on a clock-only model; the current native generator rejects it.
Preserve exact validated delay text when the elaborated model cannot change
stored state during a free-running interval. Reuse Runtime::Validate's shared
PDL frontend after Tcl substitution, existing output Command and existing
source/flat/generated/reset replay; no second time parser or floating arithmetic.
No conversion to cycles, period selection, metadata/CLI option or dependency.

A SystemClocks CheckTimeWait(preserved) method checks the immutable model once:
reject TAPs, register storage, opaque interfaces or AccessLink nodes, then use
existing clock safety analysis to reject clock/TCK signals feeding non-routing
data/control logic. Pure ports, combinational data, aliases, enums, muxes and
groups without storage remain eligible when no clock feeds data/control.
Check dynamic preserved values on every call and reject any known held
ClockPort/ToClockPort/TCKPort bit. This check applies to zero delays too:
a minimum-zero interval does not promise zero external tester cycles.
Keep existing override/checked-clock snapshots and copy semantics. Cache only
immutable eligibility, and retain dynamic held-clock checks after cache copies
and invalidation. Do not mutate device/reservation state. Clearing clock route/
selector caches after an accepted time wait preserves saved checked versions.

In Generate, keep pending-operation rejection before both iRunLoop alternatives.
Emit exactly iRunLoop -time plus the validated value. Retain loop bookkeeping,
source rewrite records and existing cycle-count/opaque/merge behavior. Stateful
time waits continue to reject with an absolute-period/event-model explanation.
Update help/guides to state this boundary. Existing resource/clock ownership and
all unrelated command restrictions remain unchanged.

Performance: input is existing3994-file runtime set; representative4096-instance/
mux fixtures,1024 time waits,5 replay forms and65536-bit clock routes. Immutable
guard is O(nodes) plus existing safety graph construction once; per wait is
O(delay text + preserved entries/clock-value bytes). O(1) new cached state; reuse
existing safety maps and avoid per-delay full scans. Measure parent/current
existing workloads and the new completing workload; parent rejects -time, so
there is no completing new-workload baseline or speedup claim.

TDD: maintained CLI acceptance tests must show real unsupported-time RED before
production. Add direct native eligibility/cache/copy/held/snapshot tests once the
new API can compile. Validate source/flat/generated/reset forms, units/exponents/
zero/large exact literals, Tcl/procedure expansion, combinational values,
stateful/opaque/clock-fanout/held/invalid-value/pending rejection and artifacts.
Keep new maintained tests in existing tests/ files.

Qualify12 changed-C++ format/tidy/strict17 gates; focused native/dynamic/static/
leak SAN, full normal then fresh raw100/90 coverage; current Rocky suites,
source archive and local/remote installs. Vendor comparison checks all3 submitted
control hashes, complete ordered trace and source/flat native export replays.
It is a single control, not broad vendor certification. One final independent
whole-branch review and one necessary fix pass; merge into parent master, delete
the merged branch, archive evidence, sync CodeGraph and push. General stateful/
event-based absolute time, strict coverage and full release acceptance stay open.
