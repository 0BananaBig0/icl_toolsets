# Tessent testbench qualification continuation

> Use superpowers:executing-plans with TDD, independent review and full regression.
> The user authorizes this extension, documentation, commit and push.

**Goal:** Require original/formatted ICL/PDL to generate matching Tessent Verilog testbench bundles as well as matching PDL before certifying formatter standards. Normal regression remains offline.

**Architecture:** Reuse tests/tessent_oracle.py export/remote-run helpers and tests/tessent_golden.py certification. Add optional Verilog export with complete result.v* artifact hashes. Compare all executable testbench/vector/config content; permit only specific, observed non-executable header provenance normalization if needed. Preserve raw exports and the exact comparison policy in certificates. No RTL simulator execution or new parser dependency is required by this task.

- [x] Inspect Tessent write_patterns documentation and probe actual export behavior in a fresh descriptive remote test directory; do not guess sidecars or volatile fields.
- [x] Add failing tests in tests/tests/test_tessent_oracle.py and test_tessent_golden.py for missing/changed testbench, changed sidecar, changed artifact inventory, old uncertified formatter snapshots and offline tampering; implement minimal shared export and certification extensions.
- [x] Run actual original/formatted qualifications; explicitly replace six formatter standards only after both PDL and testbench checks pass. Keep generator-only standards and their historical PDL criterion separate.
- [x] Independent review, focused checks and full make check; fix reproducible failures without weakening comparison. Update README and docs with exact evidence and limits; commit/push.

Review focus: normalization must never erase vectors, timing or executable text; no omitted/added sidecars; report hashes must bind the actual exported files; legacy PDL-only snapshots must not masquerade as testbench-certified; normal checks must remain license-free. Existing large-corpus tests run once after focused fixes; exports use serial licenses and bounded processes.

## Evidence

- Tessent 2026.2: all 12 export jobs passed; input, PDL and all testbench hashes verified after retrieval. Six formatter snapshots recertified; local formatted artifacts and PDL traces unchanged.
- The four-file bundles match after normalization of the leading Date comment only. Raw bundles are committed as explicit `.expected` standards.
- Independent review found an offline inventory omission; added red/green tests and exact agreement among filesystem inventory, raw hash inventory and vendor-output inventory.
- Focused checks: 17 golden-harness tests and 23 oracle tests passed. Full `make check BISON=/usr/bin/bison -j4` passed (exit 0), including both editors and all 334 grammar/119 scanner rules; evidence is in `build/testbench-goldens-regression.log`.
- Offline recheck after full regression: seven standards passed; 0.21 seconds / 55,516 KiB maximum RSS. No product C++ changes or sanitizer-setting changes. Existing C++ coverage limits remain documented.
