# Bounded symbolic numeral construction implementation plan

Execute inline using superpowers:executing-plans, combined with reviewed Tasks85/86 for a single immutable integration run.

Goal: construct large 0/1/X literals with bounded memory using installed Z3 facilities. Scope: shared SymbolicValue::Literal in src/pattern/pattern_symbolic.cc; existing tests/src/pattern_symbolic_test.cc; guide/model ledger/this plan. No dependency or public CLI option.

Spec: docs/requirements.md A3; native/shared symbolic 0/1/X values must preserve exact width/order/known mask. Existing4M literal and network bounds and invalid/nonempty checks remain.

Observed: three final Task85 eight-case runs complete14.09–14.15s but use4315660–4317088KiB peak RSS; old per-bit path times out20.47–20.52s at4655444–4656860KiB. Inputs512/8192/65536/262144bits; mixed literals and virtual resets; unchanged3s proofs/20s command. Memory is impractical for262144bits. Assume512MiB virtual-memory budget for this benchmark, not a new public resource limit or a promise at4M.

Installed Z3v4.13.3 z3++.h bv_val(unsigned,bool*) copies a full-width Boolean array and calls Z3_mk_bv_numeral. Reuse its uint64_t integer constructor (Z3_mk_unsigned_int64) for at most64 bits, then fold each word through existing Concat and simplify to a numeral before the next word. Decimal conversion would need a new big-integer layer; left-fold concat retains growing prefix work. Bounded right operands avoid the cached-power explosion; final short words retain their exact width. Growing numeral copies remain a measured tradeoff.

- [x] Add independent word-window checks at1/63/64/65/127/128/129/257/8193 bits with0/1/X and mixed patterns. Expected windows use uint64_t API directly, not Literal itself; check masks and order.
- [x] RED existing --literal-scale under512MiB virtual memory, same20s/3s limits; record actual source/binary hashes, output/exit.
- [x] Pack words MSB first, keep partial final width, bounded per-word folding; preserve all existing input validation.
- [x] Same bounded benchmark GREEN plus native word/held/shift/onehot/literal checks, connected callers/style and independent review.
- [x] Final combined native7/CLI41 remote and required local leak/full gate with frozen source/tests, actual evidence and commit.

Review focus:64-bit unsigned shifts, short trailing words and odd word counts, zero/one/unknown masks, upper bound and errors, work/memory scaling and solver behavior on existing symbolic workloads.

Bounded RED exit1: out of memory at262144 after65536 .476/.335s, whole1.13s/457712KiB under512MiB VM. Actual4 source/test hashes and prior qualified binary hash match. Word-window tests added before production.

First word-tree pilot still fails bounded memory at solver assertion, after value and reference construction; phase2 confirms identical source/reference ASTs but non-numeral concat results. Primary installed Z3 source: api_numeral.cpp Z3_mk_bv_numeral adds cached rational::power_of_two for every set bit; rational.cpp mk_power_up_to materializes every power through the requested exponent; bv_rewriter.cpp mk_concat folds adjacent numerals using that same cache for the right operand width. Balanced concat therefore still requests huge powers during assertion preprocessing. Ruling: replace tree reduction with per-word constant folding, keeping each right operand at most64bits and retaining only the accumulated numeral. This avoids the cached-power memory explosion while keeping existing Z3 APIs and no dependency. It copies growing arbitrary-precision numerals per word; measure supported scale honestly rather than claim linear arithmetic work.

Final local bounded GREEN0: original8 cases .71s/49436KiB,262144 .292/.295s; larger10case profile through1048576 bits8.67s/64072KiB,largest3.947/3.953s. Same512MiB VM/20s/3s bounds. Primary-source correction and adjusted reduction independently reviewed clean. Diagnostic phase prints removed after locating the cause. Final default native keeps8 cases; --literal-scale adds the1M pair. Final remote uses identical final test program for baseline/changed comparisons.

Final combined local connected build/native7/main68/clock41/client18/reset12/multi22/feedback10 all0; format/naming0. Final symbolic suite includes corrected empty-instance fixture and all word/mixed-run/unsorted/source/mode checks. Independent reviews clean. Integrate now onto verified2cb4ec8; freeze source/tests for required sanitizer/full. Exact final remote10-hash run is pending.


Final source-matched remote integration:60 statuses; build/native7/all41CLI/aggregate and all3 changed literal/onehot benchmark repetitions pass. Only3 intentionally failing bounded-memory baseline repetitions return1 (outofmemory). Actual10 final and10 baseline source/test hashes match manifests. Identical final benchmark program uses512MiB VM/20s command/3s proof; baseline fails at262144 in1.12s/457980–458192KiB, changed ten-case profile through1M completes13.46–13.50s/65008–65148KiB on the remote host; largest pair6.080–6.104s. Onehot three runs .09s/42128KiB,65536/512 symbolic .02847–.02861s. Host/compiler/binary hashes and commands are retained; these remote figures are distinct from local measurements. All required local leak-enabled build/native7/main68/clock41/client18/reset12 pass; full serial check is still running with source/test3 unchanged. Evidence: build/symbolic-runtime-final-remote/evidence, build/symbolic-runtime-final-sanitizer-* and build/symbolic-runtime-final-full.*.


Verified Tasks85–87 checkpoint: required leak-enabled build/native7/main68/clock41/client18/reset12 all0. Full serial make check exits0 in1327.585s:41 dynamic and41 static generator suites,51 directional goldens,2523 ICL files with334/334 grammar and119/119 scanner rules,LSP62 and editor6 in177.462s. Five optional ANTLR tests remain skipped. Final remote native7/all41 and changed benchmark repetitions pass, with10 final and10 baseline source/test hashes verified; the3 deliberate memory-baseline failures are preserved. Reviewed source/test3 remain unchanged throughout. Commit this eight-file snapshot without push. ScopeTask88 and capture/updateTask89 remain isolated; broader clock/reset/events, resources, vendor, strict coverage and final platform requirements remain mandatory.
