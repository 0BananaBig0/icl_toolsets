# Current authored-code coverage audit

Replace the stale Task45 authored-code measurement with a complete current
instrumented run. Keep required100%lines/90%rawbranches, the source list and
exception/template branch counting unchanged. This task measures and identifies
concrete remaining paths; a failing threshold is not completion of the broader
coverage requirement. Test improvements follow actual source-located results,
not guessed missing branches or speculative product changes.

Use existing make coverage with a fresh owned COV_DIR and report directory;
preserve old reports/profiles and any root gcov files. All actual instrumented
suites must complete before calling a measurement complete. Preserve any earlier
failure as incomplete evidence and investigate it. Record commands/toolchain/
walltime/input hashes, per-file and total counts and representative missing
semantic paths. Identify compiler exception edges separately without excluding
or relabeling them. No lowered thresholds, production changes, new dependencies,
new test infrastructure, synthetic failure mechanisms or new vendor claim.

Parent master94bc043, source/runtime identical Task116 and platform-qualified
through f018bb2. Reuse those exact normal/SAN/platform results for unchanged
code. New work is a coverage build and run, not another normal full regression.
Use existing isolated worktree, serial tests and at most2 compiler jobs. Review
the evidence independently, document actual results and scoped next work,
merge/delete completed audit branch promptly; no push. The broader strict gate
remains open unless the measured current run actually passes100%/90%.
