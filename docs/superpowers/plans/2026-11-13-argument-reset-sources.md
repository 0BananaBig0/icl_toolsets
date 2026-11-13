# Reset prerequisites after main-command argument evaluation

Task28 continues the approved local-reset completion work inline.

## Observed problem and approach

The valid main command iWrite a.value [iReset; expr 3] generates a normal pattern
but fails local-reset rewriting: inserting releases before the command puts
them before its argument reset. A manual Tcl apply/uplevel wrapper verified
that evaluating the original words once, then releasing prerequisites, then
executing the original command in its caller scope preserves the intended write
and produces an unchanged rewrite on a second pass.

Keep existing whole-command and literal-site candidates first. If they fail,
try literal-site prerequisite prefixes as Tcl apply lambdas whose arguments are
the original command words. Their final uplevel 1 executes the argument list
without another round of substitution. Construct both lambda and invocation
with the existing Tcl-list Command helper. Retain parsed vector-bracket source
positions in SourceAction so the wrapper protects PDL targets while preserving
real Tcl substitutions; do not rescan argument text with a second grammar. Do not change procedure definitions,
introduce persistent helper commands, or bypass fresh scheduler/runtime replay.
Only nonempty prerequisite edits need wrappers. Existing replay must still match
the expected ordered operations exactly before any output is published.

The fallback adds one bounded candidate execution and one temporary Tcl frame
per edited invocation. Measure a repeated-call case; retain runtime limits.
Unrepairable immutable procedure bodies and pending-queue conflicts must keep
source-located failure and artifact preservation.

## Work and checks

- [x] Add failing tests for direct writes/calls with argument resets, multiple
  substitutions and once-only side effects, caller variable scope, literal
  argument quoting, prefixes, repeated sites and flat/generated replay.
- [x] Add the fallback in icl_pattern_generation_main.cc using existing source
  actions/edits and Command quoting. Convert the two existing negative tests for
  this now-repairable case; retain genuinely immutable-procedure negatives.
- [x] Run focused old/new source rewrite suites, full dynamic/static regression,
  leak-enabled sanitizers, changed-source style/naming and independent review.
  Verify procedure bytes, rewrite idempotence, output replay and failed-output
  preservation; document precisely the supported transformation and remaining
  general source-rewrite limits.

New tests: tests/src/pattern/test_argument_reset_sources.py, registered once in
the shared Make generator suite. Existing tests remain in place.

Verified evidence and remaining limits: [generator guide](../../pattern-generation.md#reset-releases-after-argument-evaluation-2026-11-13).
