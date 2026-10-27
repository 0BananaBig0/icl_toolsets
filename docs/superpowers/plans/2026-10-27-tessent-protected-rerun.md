# Tessent rerun after protected scheduling

Execute the existing approved differential-oracle design inline. No new product
behavior is planned until reproducible vendor evidence identifies a defect.

- [x] Prepare fresh 36-case reference, generator/local-reset comparison,
  12-job formatter/pattern golden, and transitive-local-reset bundles.
- [x] Run all vendor jobs serially under the mapped lianghuaxiao account in
  test_icl_toolsets/test_protected_scheduling, preserving existing runs.
- [x] Add small source-classified ScanRegister reservation cases covering
  persistent/default takes and explicit protected updates, using the existing
  BSDL and oracle harness. Test original/flat/generated replay locally, then
  execute Tessent on the local-reset artifact and compare strict traces.
- [x] Retrieve and verify staged input, result and testbench hashes; record
  golden qualification into ignored scratch directories. Do not silently replace
  standards or normalize away reset, instruction or final-state differences.
- [x] Extend the static trace helper to retain literal iState commands after
  observed unsupported_trace results. Test wrapper equivalence, different/missing
  assumptions and dynamic rejection before adding the command to its whitelist.
  Do not drop assumptions or normalize them into hardware writes.
- [x] Fix the shared parser initialization crash exposed by ASan in the trace
  vector case: Tcl_ParseCommand currently runs before ParserInterpreter exists.
  Pass the existing lazily initialized parser interpreter and confirm the
  failing sanitized trace suite passes, then rerun all PDL consumers.
- [x] Diagnose any unexpected outcomes, run appropriate focused/full regression,
  update README and developer evidence, then commit and push verified changes.

Fresh vendor outcomes are separate from generator equivalence. Known negative
cases and unsupported features remain visible as such. Generated vendor files
stay outside Git; only authored fixtures/scripts and evidence summaries are added.

## Evidence

- All 60 serial vendor jobs completed: 40 exports and 20 expected errors. The
  maintained 36 reference cases matched expectations; maintained comparison
  remains 30 generator blockers, 1 vendor blocker, 5 differing traces, 0 exact matches.
  Three new reservation cases and the reset probe export but differ strictly.
- Hash checks: 307 input/script files, 40 PDL exports, 48 testbench files; see
  `build/tessent-reservations-verification.json`. Final rebuilt tools reproduced
  the same inputs for all 10 ready comparison/reset/reservation jobs.
- Seven standards freshly qualified under their existing policies and matched
  committed artifacts/traces/testbenches; no expected file replaced. Six use
  formatter testbench qualification; ScalarPins uses its existing schema-1 PDL
  policy. The optional stronger ScalarPins testbench failure remains recorded.
- RED: static iState test rejected before trace extension;
  `build/tessent-reservations-trace-red.log`. GREEN: four trace tests pass.
- Sanitizer exposed a separate initialization crash (Tcl_MutexLock/TclpAlloc)
  with vector tokens in `build/tessent-reservations-trace-sanitize.log`.
  Passing ParserInterpreter to Tcl_ParseCommand fixes the failing startup path.
  Leak-enabled ASan/UBSan then passed four trace tests and 18 resource tests:
  `build/tessent-reservations-trace-sanitize-fixed.log` and
  `build/tessent-reservations-parser-resource-sanitize.log`.
- Independent review found no blocking defects or unsupported evidence claims.
  Style and whitespace checks passed. Final full regression after the shared
  parser fix passed (exit 0): `build/tessent-reservations-parser-final-regression.log`.
  Command: `ICL_REF_DIR=/tmp/icl-reference-honza-20261021` with the existing
  reference venv on PATH, `make check BISON=/usr/bin/bison -j4`. Includes
  dynamic/static generator tests, PDL tools, seven offline vendor goldens,
  334/334 grammar alternatives, 119/119 scanner rules, and both editor corpora.
  Strict authored-C++ coverage was not rerun.
