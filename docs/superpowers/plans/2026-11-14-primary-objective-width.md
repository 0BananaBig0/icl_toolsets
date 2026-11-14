# Bound primary solver objectives and symbolic vector expression work

> Execute inline under existing autonomous authorization. This necessary
> performance work was exposed by65 verification; preserve workflow gates.

Problem: native ASan TestPrimaryLimits repeats a2s Optimize timeout at1024 direct
primary bits, including a quiet rerun. A64-netlist baseline also times out (that
sandbox run additionally ends with LSan ptrace failure, so it proves no leak-check
pass). Isolated unsandboxed64/65 both pass near the deadline. Extend the legal
4096-bit scale check; retain4097-bit atomic rejection. No deadline is relaxed.

Final design: preserve contiguous physical source runs as one SymbolicValue slice
instead of rebuilding every bit as nested concat/extract. Merge only one node,
one inversion and decreasing validated vector offsets; keep scan/shift selection,
unknown masking and literal/virtual/reordered/duplicate reference semantics. Reuse
BitOffset checks in Read and Bits. Advance through each reference once, O(refs).
This change reaches all SymbolicNetwork::Bits callers, so native symbolic/state/
TAP/scheduler and full original/flat/generated checks are required.

Primary-write objectives additionally minimize at-most32-bit slices MSB first,
retaining data deviations -> drive count -> pin deviations. Unsigned whole-vector
ordering equals lexicographic chunk ordering. Keep all hard goals, reservations,
activation, horizon/work/time guards and concrete replay unchanged. Reuse current
z3::optimize; no backend, public API or dependency. A direct-wire scheduling bypass
would duplicate semantics and is unnecessary.

Scope: src/pattern/pattern_scheduler.cc and pattern_symbolic.cc; existing native
scheduler/symbolic tests; guide/model/main ledgers and this plan. --primary-limits
selects the existing native scale check for diagnosis; default still runs all.
Tests cover1024/4096 direct input replay, a262-bit reduction choice preferring the
least-significant input,4097-bit atomic rejection, ascending/descending/full/
sliced/reordered/inverted/mixed/duplicate physical references, virtual/unspecified
interruptions and invalid ranges. Symbolic expected values use concrete state.

Evidence and selection: initial raw BV objective check0.840–1.043s at1024;
diagnostic objective omission0.171–0.194s,32-bit chunks0.316–0.350s. Omission is
diagnostic only. Chunk-only32/64/128/256 and slice-only still fail4096; retain their
logs. Explicit unsigned bv2int, already used elsewhere, was ablated too: old
symbolic fails4096; slices permit4096 but the legal reduction preference times out.
The first reduction fixture incorrectly declared a vectored LogicSignal; corrected
to the standard scalar reduction before combined acceptance measurements.

Combined slices+32-bit chunks pass3/3 instrumented runs, including reduction and
resource-bound assertions. Source solver.check1024 takes0.0594–0.0714s;4096 takes
0.4253–0.8338s (under unchanged2s), total4096 transaction2.343–5.062s (under15s).
Measured peak817168KiB ASan RSS.64-bit chunks also pass, with higher measured
check time0.4847–0.5964s on4096 in these runs. Keep32; no global optimum claim.
Exact metrics in build/primary-combined-scale-comparison.json. Other diagnostic
reports: mux-resource-primary-objective-comparison.json, primary-objective-chunk-
scale-comparison.json, primary-signal-runs-sanitizer-comparison.json and
primary-unsigned-int-comparison.json under build. Temporary instrumented variants
stay outside product code, with no probe output/environment setting in the final
implementation. Scope is direct mapped primary performance through4096 bits;
arbitrary-network resource-limit failures remain valid.

Independent design/source review confirmed chunk objective equivalence and complete
nonoverlapping extraction. Slice design review requires offset-based grouping,
validation before arithmetic, preserved scan/inversion and monotonic advancement.
Native symbolic concrete-equivalence tests pass. Final slice source review, native
full/focused checks, naming/style and official final ASan/full regression remain.

- [x] Existing1024 repeated RED plus new legal-scale RED; measured objective ablations.
- [x] Source slice and bounded objective implementation; diagnostic combined GREEN.
- [x] Independent final slice/test review, naming/style, optimized measurements.
- [x] Reviewed final eight-file65 snapshot; affected native7/CLI sanitizers and
  complete regression sequentially, documentation and commit.66 stays isolated.

Final review found no source defect and requested one direct contiguous register
Bits check under distinct held/shift states; added both scan modes/polarities and
cache-order reversal to the existing native test. Focused symbolic, optimized
primary3, main50, merge, mixed-I/O and data reservations pass. Naming initially
required braces; confirmed0. Generic padded-operand budget preflight is also
included by65. Final eight-file checkpoint is shared with65; official sanitizer7
and complete serial regression remain pending.

Official final ASan/UBSan with leaks now passes native symbolic/netlist/flatten/
state/TAP/address/scheduler plus main50/clock41/client18/reset12, all in
build/mux-resource-primary-final-sanitizer-*. Native scheduler passes both direct
scales (1024:0.320s,4096:4.750s total Apply) and the reduction preference/rejection
assertions. Frozen8-file hashes still match. Complete make-j1check is running in
build/mux-resource-primary-confirmed-full.*; do not mark integration complete yet.

Final integration: make-j1check passes in build/mux-resource-primary-confirmed-full
.log/.exit0,1641.867s wall. Dynamic/static generator checks,51 directional goldens,
2523 ICL334/119 rules, LSP62 and editor6/216.032s pass; optional ANTLR5 skipped.
Leak-enabled native7 and main50/clock41/client18/reset12 pass under
build/mux-resource-primary-final-sanitizer-*. All8 SHA-256 values match the reviewed
checkpoint. Naming/style and independent source/test review pass. Initial failed
runs remain preserved. Strict coverage/vendor certification/platform refresh and
previously recorded runtime/conformance requirements remain open.
