# Embedded oscillator relative waits

Execute inline under the existing autonomous authorization and approved generator
design. Keep Task52 root production files frozen until its regression and commit;
prepare Task53 in the existing isolated worktree. Preserve independent review.

## Scope and design

IEEE1687-2014 §6.4.6.20(d,f,j) and §7.9.15(b) permit a ToClockPort with Period
as a clock source, with omitted units meaning ns. Support relative checks/waits
on that root and its positive clock-only descendants, retaining all current
snapshot, fanout, reservation and arithmetic checks. Root identity is node+bit,
never period equality. Normalized PDL names the hierarchical oscillator bit;
relative waits do not need unit conversion or physical phase changes.

Add optional typed ClockPeriod metadata (positive signed64 magnitude and validated
unit) to NetlistNode, populated from the already parsed Period tokens. Reuse
EvaluateInteger for checked decimal magnitudes, including underscores. Accept
bare, attached and separated unit forms already allowed by the frontend. Keep
the existing frontend exclusion of Source/factors/DifferentialInvOf and test it.
Serialize a canonical decimal magnitude and explicit unit. No new grammar,
parameter-period extension, dependency, synthetic property or unit conversion.

SystemClocks::Resolve accepts a typed ToClockPort oscillator as a terminal root;
BuildSafety removes only blanket Period rejection. Override sources may reference
oscillators; direct oscillator-target overrides remain explicitly unsupported.
Do not change storage, TAP or pin levels. Absolute-time waits, general clock events,
oscillator-target overrides and differential/inverted paths remain follow-ups.

Connected scope: pattern_netlist.h/.cc, pattern_flatten.cc, pattern_clock.cc,
CLI help, existing clock CLI/native scheduler/flatten tests, guide and model ledger.
No new test runner. Optional metadata adds fixed per-node storage; measure a
representative4096-instance model and existing1024-wait chain, plus a65536-bit
oscillator. Preserve the45s per-case target; do not add a cache or optimizer.

## Steps and acceptance

- [x] Independent conceptual review and existing parser/source/serializer analysis.
- [x] Add failing positive CLI cases for all seven units/default ns, attached and
  separated forms, decimal limits, child/vector roots, descendant scaling,
  override sources, procedures and flat/generated/reset-only replay.
- [x] Implement typed metadata, canonical serialization and oscillator roots.
- [x] Cover zero/overflow, conflicting properties, equal-period distinct roots,
  held bits, unsafe fanout, old snapshots and unchanged hardware in CLI/native tests.
- [x] Measure representative scale, run focused checks and independent source review.
- [x] Integrate after Task52 commit, run full dynamic/static regression and relevant
  leak-enabled sanitizers, record evidence and commit. Keep coverage/platform and
  broader clock requirements open until their own acceptance checks pass.

Isolated implementation complete and independently reviewed clean. Four positive
methods fail against Task52 at the explicit unsupported oscillator guard (19
subcases), retained in build/oscillator-red-cli.log/.exit1. Period metadata uses
owned optional magnitude/unit values; frontend conflict checks already apply.
No parser extension or synthetic flat property is needed.

The first37-method attempt overlapped an executable relink and failed with launch
PermissionErrors; it is retained as failed evidence. The subsequent serialized
run passes all37 methods in9.991s (10.04s wall,94764KiB maximum RSS), including
4096 stages/219107 ICL bytes/1024 waits/five forms in1.033372s and a65536-bit
oscillator/55 ICL bytes/1024 waits/five forms in0.231470s. Existing external-clock
chain and wide cases take1.085373s/.238851s in the same run. These measurements
meet the workload target and do not claim speedup. Full native scheduler/source
and flat oscillator reservation/state tests pass; existing flatten, naming and
formatting checks pass. Root integration/full/SAN pending Task52 commit.

Final integration passes: build/oscillator-final-full.log/.exit0,999.946s wall,
41 dynamic/static suites,51 directional/seven legacy goldens,2523 ICL334/119
rules, editor6 in176.843s; five optional ANTLR tests skipped. Leak-enabled native
scheduler/flatten/netlist and main35/clock37/client18/reset12 checks pass. Naming,
formatting and independent source review pass. Tessent2026.2 source/normalized
exports and our output have identical child.osc7/10 waits; fetched input hashes
match. Evidence: build/oscillator-vendor-probe/comparison.json. Strict coverage
and Rocky8 qualification remain open.
