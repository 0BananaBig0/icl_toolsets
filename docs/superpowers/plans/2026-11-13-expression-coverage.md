# Numeric evaluator coverage follow-up

Continue Task25's unchanged 100% authored-line/90% raw-branch gate. The user
delegated implementation choices. Use inline execution and independent review.

The previous report identifies untested constant conversion, literal sizing,
comments and checked arithmetic. Compare with the fresh c787380 report before
selecting cases. Extend the existing native expression test; do not add a
parallel runner, dependency, evaluator or coverage exclusion. Prefer parsed
input for language behavior and direct Constant calls for its public conversion
contract. Preserve the checked signed64-bit intermediate arithmetic contract.

Changes reach tests/src/pattern_expression_test.cc and the existing coverage
evidence ledgers. Existing Makefile targets already run this native executable.
Any discovered implementation defect requires a failing regression before its
fix. No implementation change is assumed necessary.

Ordinary inputs remain tiny; test the existing1Mi-bit limit with at most a few
MiB of temporary data. Do not use allocation-failure machinery to inflate raw
branch coverage. Retain compiler cleanup/exception edges in the measured totals.

- [x] Record the fresh baseline and select still-uncovered behavior.
- [x] Extend native assertions for conversion/sizing/comment/arithmetic edges.
- [x] Run native, sanitizer and instrumented checks; inspect incremental coverage.
- [x] Independently review, record residual gaps without weakening the gate,
  integrate and commit the verified tests.

Other model requirements remain separate mandatory work. This bounded follow-up
does not claim that these tests alone can satisfy the full strict coverage gate.

Fresh baseline c787380: all instrumented tests pass, then the strict recipe
fails with exit2:14,366/14,971 lines(96.0%) and24,013/38,965 raw branches(61.6%).
Evidence: build/feedback-coverage-suite.log/.exit and
build/feedback-coverage-baseline-report.log, with gcov files retained in
build/feedback-coverage-report. The evaluator itself reaches203/237 lines and
288/477 branches. Independent plan review confirms the selected cases and
advises against fabricated malformed parser tokens to exercise invariant guards.

The existing native test now covers integer/string/X conversion, leading zeros,
exact/over sizing limits, padding/truncation, numeric comments and escaped
strings, small exact/over string budgets, literal/aggregate bit budgets and
signed intermediate overflow. The optimized native test passes. No product
implementation changes are needed for these cases.

Independent review selected four additional parsed cases: parameter-derived
zero/over-limit literal widths, a sized parameter against a mismatched context,
and two unsized parameters with a known context. Final optimized, instrumented
and host ASan/UBSan tests all pass in
build/expression-coverage-final-{native,instrumented,sanitizer}.log/.exit(0).
Leak detection and UBSan halt-on-error remain enabled. Optimized wall/RSS are
0.03s/15,448KiB; instrumented0.36s/16,604KiB; sanitizer0.15s/58,800KiB on local
Linux, g++16.2.1/Clang. These are single-run test costs, not a speedup claim.
Formatting, whitespace and final independent review pass.

The final report combines the complete c787380 instrumented run with the added
native assertions; no product code changed and the broad suite was not repeated.
Use the unchanged COV_REPORT_OBJECTS plus the final supplementary native gcno,
then tests/coverage.py --min-line 100 --min-branch 90 --strict. Evidence is build/expression-coverage-final-report.log/.exit(2) and
the matching gcov directory. Expression coverage becomes222/237 lines and330/477
branches, gains of19lines/42branches. Aggregate14,385/14,971 lines(96.1%) and
24,055/38,965 raw branches(61.7%) still fail the strict gate.

The expression report retains15 unexecuted lines, including malformed-token
diagnostics outside the already-parsed input contract, unreachable operand-stack
guards and compiler cleanup lines. No fabricated parser tokens, allocation
fault injection, exclusions, thresholds or compiler coverage flags were added.
Task25 acceptance and all other remaining requirements stay open.
