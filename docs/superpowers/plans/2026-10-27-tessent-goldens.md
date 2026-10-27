# Tessent-validated offline goldens implementation plan

> **For agentic workers:** Use superpowers:executing-plans; prior user authorization permits implementation and commit/push without another permission round.

**Goal:** Explicitly certify generated flat PDL and formatted ICL/PDL with Tessent, then run byte-exact offline regression against immutable snapshots.

**Architecture:** Reuse tests/tessent_oracle.py staging, serial remote execution, input hashing and Tcl scripts, plus the parser-backed pdl_pattern_trace. Add tests/tessent_golden.py for candidate preparation, certification and offline checks. Preserve separate languages and existing product binaries.

**Spec:** The user's 2026-10-27 request in this session. Decisions below make comparison/publication semantics explicit.

## Constraints and decisions

- Pattern candidate is the actual flattened output, never --local_reset output. Test it with the same ICL/BSDL, then require both candidate/vendor fixed-point agreement and original/vendor agreement.
- Formatter candidates change only ICL or only PDL in separate jobs. Same top, source order, PDL/ICL counterparts, BSDL and no implicit reset.
- Vendor comparison uses the existing static trace parser: ignores comments, notes and exported oracle wrapper only; preserves binding, ordered commands, operands and apply boundaries. Vector/bit grouping differences fail certification; no new semantic canonicalizer.
- Offline comparisons are byte-exact tool-output checks and idempotency checks. They never discover or launch Tessent/SSH and never update standards.
- Record only explicitly selected successful cases. Existing snapshots are immutable; recording refuses existing destination. Input changes make certificates stale rather than silently updating expected outputs.
- User explicitly authorizes stored generated regression standards. Keep these under tests/golden/tessent; do not commit logs, executables, generated parsers or general vendor workspaces.
- Negative or unsupported cases remain blocked, never become successful goldens. Fixed points do not prove semantic correctness for all possible inputs/hardware.
- Reuse descriptive remote test_icl_toolsets/test_<topic> directories. Run license jobs sequentially.

## Task 1: prepare, certify and check

Files: tests/tessent_golden.py, tests/tests/test_tessent_golden.py,
tests/cases/ours/tessent_golden/{scalar.icl,scalar.pdl,cases.json}, Makefile.

- [x] Add failing tests using real local formatters/generator/trace: staged flat pattern (no local-reset), separate source/ICL/PDL jobs; offline stability; tampered input/output rejection; unsupported vendor result; mismatched traces; stale snapshot; immutable publication; no partial snapshot on failure.
- [x] Implement prepare by extending existing prepared reference bundles with tool candidates; bounded local subprocesses, hashed candidates and Tcl; record validates both reports and current source hashes before writing snapshots; check uses temporary local candidate outputs.
- [x] Run focused harness tests and make tessent-oracle-check; retain existing oracle behavior.

## Task 2: actual vendor qualification and integration

- [x] Prepare scalar and maintained OneHot-width examples. Run Tessent 2026.2 on remote account and retrieve results with hashes verified. Certify only matching candidates; report vector grouping failures without weakening comparison.
- [x] Save explicitly certified outputs and compact version/hash/trace provenance; make check runs their local comparison without licenses.
- [x] Run sanitizer tests with leak detection ENABLED; report actual environment/tool findings.
- [x] Independent review, full make check and README/developer documentation. All affected checks rerun after fixes; user-authorized commit/push follows verified completion.

## Review focus

Certificate staleness after source/order/top changes; changed artifacts before record;
forged pass status without matching traces; output/source aliasing or symlinks;
pattern fixed point that differs from the original request. Each is covered by
validation tests. Baseline scale: a small paired semantic corpus, linear file
copy/hash work with bounded subprocesses and serial license use.


## Evidence and rulings

- Actual Tessent 2026.2 exported 12/12 jobs; verified 39 input/script and 12
  export hashes. Seven standards qualified. Two vector-generator fixed points
  failed literal bit/vector grouping and were not recorded.
- Independent review exposed false JSON pass status, modified prepared metadata
  and modified certified top. Reproduced failures before fixing console/status/
  version checks, immutable preparation comparison and offline context binding.
  Current manifest cases must also agree. Eleven harness tests pass.
- Full regression initially found expected ICL outputs duplicated source modules.
  Changed only golden artifact storage to the existing `.expected` convention;
  duplicate corpus policy is unchanged. Focused corpus/golden checks passed.
- LeakSanitizer failed in the sandbox with its ptrace/environment error. Same
  ASan/UBSan command with detect_leaks=1 passed outside it, including 19 CLI cases.
  External Z3 is uninstrumented; no sanitizer was disabled for the successful run.
- Ruling: retain strict ordered operands instead of normalizing vector/bit writes.
  Cost: equivalent alternative patterns can remain unqualified; no false golden
  acceptance. Fixed-point agreement alone is never treated as complete correctness.

Final make check passed: all seven offline goldens, 334 grammar/119 scanner rules, both editors on 38,634 ICL and 806 PDL files. No product C++ changed; the existing strict C++ coverage gap remains recorded.
