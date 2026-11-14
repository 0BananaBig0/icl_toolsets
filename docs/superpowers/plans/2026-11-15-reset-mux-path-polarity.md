# Reset/TRST mux path polarity implementation plan

> Execute inline under the existing autonomous authorization. Root Task71
> source/tests remain frozen through leak/full checks. Only the isolated checkout
> contains this next draft; preserve the immutable Task71 snapshot in/tmp.

Goal: reject consumed ResetPort/ToResetPort DataMux bits with no corresponding
typed reset path of the consumer's effective polarity, and TRSTPort/ToTRSTPort
DataMux bits with no possible typed TRST-low path. This establishes possible
structural compatibility, not reachability, selector justification, associated
reset ownership, assertion behavior or full operational conformance.

Primary: IEEE1687 6.4.6.13(a,d–f),6.4.6.14(e,g–i). Explicit direct consistency
and default inheritance were handled in71. A DataMux remains a data_signal;
do not infer omitted port polarity through it. Without a dictating direct typed
bit in the grouped declaration, omitted port polarity stays high.
Existing82 vendor cases already reject low consumers fed from high reset muxes;
40 additional input/output and nested inversion loading probes qualify this.

Approaches: per-consumer graph walks repeat work on shared networks; requiring
every typed ancestor to match would reject the existing valid interpretation
where a second typed control serves as forcing data. Memoize the possible low/
high reset path mask per mux bit in the existing iterative origins traversal.
Inversion swaps the bits, branches union them, and a consumed Reset/ToReset bit
requires its effective polarity to exist. Stop at ordinary ports/storage/logic/
groups; ignore forcing-only branches. Keep ambiguous two-control acceptance and
document that operational path selection remains unproved. TRST/ToTRST consumers use a separate qualified TRST polarity pair; Reset forcing
paths cannot satisfy it.

Reach: src/pattern/pattern_netlist.cc, tests/src/pattern_netlist_test.cc,
tests/src/test_icl_pattern_generation_main.py and the current two generator docs.
Reuse the bounded origins array, widened to uint16_t with separate Reset and TRST
low/high pairs and unknown512. This adds at most16MiB at the16M-bit bound, with
no new array, traversal, public API, metadata, solver deadline or dependency.

Review focus: nested double inversion; typed forcing of opposite polarity;
shared consumers demanding incompatible polarities; aliases and vector positions;
ordinary forcing branches and omitted default staying independent of mux ancestry.

- [x] Add native direct/alias/input/output and nested XOR polarity truth tables,
  mixed vectors, shared consumers, typed forcing and source/flat positives; CLI
  negatives retain pattern/reset/debug artifacts. Observe RED before production.
- [x] Accumulate possible typed Reset/TRST path polarities in the existing mux
  traversal and validate consumed Reset/ToReset Source bits. Run focused GREEN.
- [x] Finish matched-input vendor loading probes; record differences explicitly.
  Measure wide/shared/deep/ordinary loading and matching replay outputs versus71.
- [x] Connected/style and independent review; immutable snapshot, integration,
  full serial/leak checks, truthful ledger update and verified commit after71.

Independent design review supports the existential-path mask. It specifically
rejects treating all typed ancestors as associated reset sources. No72 product
code exists when this plan is written. Task71 corrected internal TRST metadata
via a RED/GREEN test; all7 native leak suites pass and selected CLI leak checks
are still running before its full serial regression.

RED on remote frozen71: native1 and CLI1 accepted incompatible mux polarity.
First GREEN exposed the now-invalid low+high-mux mixed vector and earlier shape
error precedence. Normalize only that fixture to~hi/0; retain the new incompatible
truth table/default negatives. Defer one polarity error until the existing shape
walk finishes, without extra traversal/allocation. GREEN2 native netlist and all57
main CLI tests pass; all3 actual remote source/test hashes match the submitted
manifest. TRST producer/reset-consumer positives also pass original/flat replay.
40 matched vendor models/runners:20load/20ICL64-1; all nested inversion parity
cases agree. Loading-only; ambiguous typed forcing remains separately qualified.
Independent review and remote interleaved performance checks are now running.

Independent review finds no consequential implementation issue; family/polarity/
sentinel masks and constant traversal overhead are sound. Its two qualifications
are applied: typed-forcing/shared-consumer positive now includes a flat roundtrip,
and omitted-high wording excludes grouped ports whose other direct typed bits
dictate the group under71. The new native replay assertion still needs its final
run; source algorithm is unchanged after GREEN2.

Scope ruling: combine the closely coupled TRST consumer extension with this mux
polarity verification batch. Both use the same traversal and polarity carrier;
this avoids a duplicate full regression without waiving RED/GREEN, review,
performance, leak or full checks. The completed Reset-only snapshot is preserved
under/tmp/reset-mux-polarity-task72-before-trst-files. Its remote connected run
continues on immutable source; new TRST native/CLI negatives are written locally
and must fail against that snapshot before the extension is implemented.

TRST needs its own possible-low/high pair: a low Reset forcing path must not
satisfy a TRST consumer when the actual TRST path has inverted-high polarity.
Widen origins, GateVisit and transform carriers to uint16_t, with TRST bits128/256
and unknown512. Reset consumers retain either Reset/TRST path; TRST consumers
require a possible TRST-low path. All family consumers remain masked to their
family bits. No extra traversal/array, but the existing origins allocation grows
by up to16MiB at the16M-bit graph bound; measure wide/shared peak memory. Keep
existential typed-forcing interpretation and all operational limits explicit.
Independent design review supports this representation and the scope boundaries.

Task71 is committed as5714c7c after full serial0 (1339.731s/editor6 181.382s),
all required leak checks and final source/test hash identity. Root is clean.
The combined mux draft was saved under/tmp/reset-mux-polarity-task72-before-
advance-files with hashes/full diff and a retained stash, then advanced onto
5714c7c on task/reset-mux-path-polarity. Only the3 source/tests plus this plan
were restored; final71 docs remain intact. The baseline71 CLI/hash is retained.

Combined TRST extension has native1/CLI1 RED against reset-only code; GREEN build,
native and all58 main CLI tests pass. uint16_t carriers/sentinel and family-only
shape masks retain earlier behavior. Shared Reset-low/TRST-high trap rejects;
nested/alias parity, opposite TRST alternatives and implied virtual TRST replay
pass. No assertion/selector/power-up operation claim is added. TRST vendor40
matched inputs yield12load/12ICL64 polarity/8same-module ToTRST source-context
ICL65/130/8mixed-special-input ICL115.8 legal child-ToTRST cases are separately
running to distinguish hierarchy context from polarity. Same-module and special-
input vendor restrictions remain distinct from the retained source compatibility
interpretation. Combined local connected checks and final review are required.

The legal child-ToTRST follow-up has8 matched model/runner hashes:4load/4ICL64
polarity failures, following inversion parity. Native TRST truth-table coverage
now includes both parent TRST and child ToTRST producers. The added native cases
still need their final run; production is unchanged from the combined GREEN58.
Reset-only remote native7 and official dynamic41 each pass, with actual source/
test hashes matching that earlier snapshot and completed archive fetched0. It
is supplementary evidence, not final combined-source platform qualification.
Combined local native6 pass and scheduler/selected CLI checks are still running.

Combined scale acceptance retains655360 ordinary references,4096 aliases and
512/4096 shared mux/consumer chains, plus262144 reset bits and a joint262144 Reset
+262144 TRST workload. Compare3 interleaved source/flat repetitions against the
retained5714c7c binary, with output/flat hashes and exact commands/time/RSS. The
joint workload measures the widened origins memory tradeoff; the theoretical
additional16MiB at the16M-bit bound is not claimed as an observed RSS figure.

Combined local connected native7/main58/clock41/client18/reset12/multiple-TAP22/
feedback10 all pass. The native-only child-ToTRST truth-table addition was made
after the connected native binary was built, so it now receives its final focused
native rerun alongside style/naming. No product change after combined GREEN58.
Root remains clean at5714c7c.72 production/tests stay isolated until the required
review/performance/final integration checks complete.

Final child-source native rerun, format and naming checks exit0. Independent
combined implementation/test review finds no consequential issue: carriers,
sentinel, inversion and family masks are consistent; separate TRST cannot borrow
Reset forcing polarity.60 local interleaved comparisons on five valid wide/deep/
shared/ordinary workloads pass output/flat hash identity versus5714c7c. The first
joint Reset/TRST pilot is rejected by the baseline because its scalar constants
do not match vector widths; retain the failed pilot logs and correct the fixture
to262144-bit constants. Only that corrected joint workload is rerun separately;
no product code or other measurements change.

All72 valid interleaved source/flat comparisons pass identical output/flat hashes
against5714c7c.262144 reset bits: changed source0.616–0.717s versus0.567–0.617s,
peak146528KiB versus145288KiB. Joint262144 Reset+262144 TRST bits:0.617–0.667s,
peak171424KiB versus171704KiB baseline; allocator variation is not a memory-saving
claim.4096 shared mux/consumer source0.215–0.265s,4096 aliases0.065–0.166s,
655360 ordinary references0.115–0.166s. Exact commands, compiler/environment,
source/binary/input hashes and time/RSS repetitions are retained in
build/reset-mux-path-polarity-loading-scale/comparison-valid.json. Two malformed
joint fixture pilots are preserved separately; only corrected valid runs count.
These measure whole empty-PDL CLI/debug I/O, not isolated traversal cost or a
speedup. The up-to16MiB extra origins storage is the theoretical16M-bit-bound cost.
Final native/connected/style/naming and independent combined reviews pass; final
immutable integration, full serial and required leak checks remain.

Final required leak-enabled ASan/UBSan passes: native7 and main58/clock41/client18/
reset12, aggregate build/reset-mux-path-polarity-final-sanitizer-all.exit0. All6
root hashes match the immutable reviewed snapshot before full serial execution.
Final make-j1check now runs in build/reset-mux-path-polarity-final-full.log;
production/tests remain frozen. Subsequent draft work stays isolated.

Final combined serial make-j1check exits0,1341.861993s,41 dynamic/static generator
suites,51 directional goldens,2523 ICL334/119 rules, LSP62 and editor6 in175.000s.
Optional ANTLR5 skipped. All3 final production/test hashes match the reviewed
snapshot. Required leak-native7/main58/clock41/client18/reset12, connected,
performance, style/naming and independent combined review pass. Commit the verified
combined72/73 increment; all recorded broader requirements remain open.
