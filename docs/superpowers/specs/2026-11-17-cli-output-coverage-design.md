# CLI and staged-output coverage

Add behavior tests for real gaps measured in Task118, without product changes.
Current baseline is17300/17961 lines and29171/46677 raw branches. The strict
100%/90% requirement remains open; this task closes specific observed paths,
not the entire denominator. No exclusions, threshold changes or failure injection.

Reuse Generator.run_tool in tests/src/test_icl_pattern_generation_main.py.
Add invalid --icl/--pdl empty lists, unknown flag, missing --output value at end
or before another option, and missing required inputs. Each returns2, names the
problem option and preserves existing generated artifacts. Test missing and
NUL-containing ICL/PDL files with valid generation options: return2, name the
input and preserve result/reset/debug artifacts. Expectations are public behavior,
not source-string assertions. Existing69 generator cases remain.

Use existing tests/unit_tests.cc Check/CheckEq and unitcheck/coverage runners.
Test public StagedTextFile with real files in one mkdtemp-owned directory:
unprepared Publish rejects; repeated Prepare rejects while retaining the first
stage and other destination; Publish commits the original data; abandoning a
prepared stage removes temporary files without changing the destination; replacing
an initially absent destination with a directory/sentinel before Publish makes
publication fail and leaves that directory intact, with temporary cleanup on
object destruction. Use stdlib file reads and directory entry counts. No mocks,
new production seams, global allocators, new framework or Makefile target.

Only two existing test files, one necessary existing-target header dependency
and documentation change. Baseline RED is the
recorded zero-hit guards in current coverage; the functionality already exists,
so added behavior tests should pass. GREEN requires observed hits of CLI
66/80/84/90/99 and staged-output27/28/114/115/119/120, plus behavior assertions.
Do not turn line locations into maintained test assertions. Build fresh focused
instrumented outputs, preserving Task118 reports/profiles, then run the complete
existing coverage recipe unchanged and report its actual still-strict result.

Normal dynamic/static generator cases, native unit tests and leak-enabled SAN
run freshly for changed test groups. Reuse unchanged normal/SAN/platform groups
only with exact runtime input exclusions for Makefile and these two test files;
prove all
product source and binary fingerprints unchanged. Qualify changed groups on
Rocky dynamic/transferred-static as needed. One final independent review and
necessary correction pass, then exact commit/merge master/delete branch; no push.
Parent masterde758f6; current isolated worktree. Small test-only change, user
standing instruction delegates design/routine choices and questions.

Preflight refinement: native unitcheck manually lists header prerequisites and
did not include file_output.h. Real make -n -W header unitcheck RED schedules no
compile; add that header to the existing target only, then GREEN schedules the
unit binary rebuild. Makefile is the third amended input, with product build
recipes/flags unchanged. No new test target or runtime behavior.

Single review fix pass: use otherwise-valid ICL/PDL plus NUL and require the
input-reading diagnostic, so downstream syntax failures cannot satisfy the NUL
test. Empty file lists require their specific missing-files diagnostic. Initial
full-run evidence is retained but is not final qualification of this correction.
