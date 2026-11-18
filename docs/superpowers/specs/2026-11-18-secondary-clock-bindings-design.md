# Secondary clock input bindings

Parent mastere6ca6b3; task/secondary-clock-bindings. Standing user delegation
covers routine choices and incremental commit/merge/delete/push after completion.

Accept Tessent2026.2's named-negative source form only for an explicitly bound
negative ClockPort input. Its ICL65-1 diagnostic accepts a secondary ClockPort
in the parent or a secondary ToClockPort from a direct child. Native source-kind
validation currently rejects that model, even when PDL operates its positive
clock. This is an identified vendor extension to literal IEEE1687-2014
6.3.12(e)/6.4.7(h) clock_signal lists, not a reinterpretation of those lists.
IEEE7.9.15(a), printed174, permits any valid ClockPort/ToClockPort target;
Tessent2026.2 separately rejects negative iClock targets after elaboration.
Source-admission qualification therefore uses positive-clock PDL. Preserve the
native IEEE negative-target behavior and record the distinct vendor restriction.

Approaches: globally classifying secondary ports as ordinary clocks would widen
all source contexts; inserting a model rewrite would duplicate elaboration.
Use a destination/role-specific predicate in existing CheckSignalSources:
Source of ClockPort with DifferentialInvOf, driven by a ClockPort/ToClockPort
carrying DifferentialInvOf. Keep ordinary source kind, scope/direction, aliases,
flat-origin restoration, width/bit identity and runtime phase proof unchanged.
No new public API/command/enum/cache/dependency. Existing valid IEEE forms remain.

Test original/flat metadata, direct/aliased/child-output/vector/partial/nested
bindings, ordinary positive/ClockMux/output/data/DifferentialInvOf rejection,
wrong hierarchical aliases/output depth, and runtime disagreement/cycles.
Source eligibility does not assert a valid waveform; current clock backend
remains relative/duration-only and its existing phase proof applies when traced.

Performance: O(1) additional checks per existing source edge, no new scans or
per-bit state. Reuse4096-pair/stage1024-wait/65536-bit clock fixtures and5000-instance
netlist benchmark; existing45s subprocess limits remain. Record input sizes,
environment, wall/RSS and source/product identity; no unmeasured speedup claim.

Acceptance: native source/flat assertions and CLI replay RED then GREEN;
focused whole-source-kind/runtime/clock dynamic/static and leak SAN; changed
C++ style/strict17; full normal integration then fresh raw coverage with unchanged
100%/90% thresholds; current Rocky compiler/suites, source archive/install;
fresh vendor positive-clock source/export/replay results with stage-specific
failures retained. Claim a directional certificate only if ordered commands
actually satisfy existing acceptance. One independent whole-branch review,
necessary one fix pass, merge parent/delete branch, archive/sync and push.
Broader source/clock/vendor coverage remains explicit when unqualified.
