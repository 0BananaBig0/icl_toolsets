# Direct ScanOutPort register source validation

> Use superpowers:executing-plans for authorized inline implementation.

**Goal:** Reject direct non-rightmost ScanRegister sources during elaboration,
under IEEE1687-2014 §6.4.6.2(b), before scheduling or artifact replacement.

**Architecture:** In the existing Connect pass, inspect resolved ScanOutPort
Source bits and compare direct ScanRegister indices with their declared right
index. Keep the existing reference resolver and all other source contexts.
Do not select an implicit bit for a bare vector: §6.4.8(u) grants that permission
only to ScanInSource. No new enum mode, graph walk or dependency is needed.

**Tech Stack:** C++17, existing native and Python unittest checks.

**Spec:** [Approved generator requirements](../specs/2026-10-25-icl-pattern-generation-design.md),
illegal connection validation. Autonomous inline execution remains authorized.

Source evidence: cached IEEE text at build/combined-audit/ieee1687.txt,
lines5240–5292 and7172–7205. Independent review confirms this direct rule.
Current CLI rejects a direct r[2] tap from r[3:0] only later in ScanGraph as
an unsupported intermediate serial tap. Elaboration itself accepts it.

Scope: src/pattern/pattern_netlist.cc, tests/src/pattern_netlist_test.cc,
tests/src/test_icl_pattern_generation_main.py, current guide/model/plan ledgers.
No new test script or fixture is required. Alias and mux traversal, intermediate
paths through other models, and conditional signal-kind rules remain separate
acceptance work; do not infer rejection of those paths from this direct rule.

The check adds O(B) comparisons for B already-resolved ScanOutPort source bits,
with O(1) extra storage. Existing elaboration budgets bound B. Reuse the existing
5000-instance/wide netlist benchmark to check exercised scale; no speedup claim.

Review focus: ascending and descending ranges; bare-vector width semantics;
invalid concat elements; preserved DataOutPort and PDL middle-bit access;
source-located rejection and preserved output artifacts; original/flat/generated
replay. Checking every scan-like context or reusing kScanInput would overextend
this rule and alter implicit bit selection, so use the direct Source check.

- [x] Add failing native tests for non-rightmost scalar, vector and concat
  sources; include correct ascending/descending, inverted and scalar sources.
  Keep data output and Enable middle-bit uses valid.
- [x] Add CLI invalid-source cases requiring the elaboration diagnostic and
  preserving pattern/reset/debug artifacts. Add valid original/flat/generated
  replay with explicit rightmost serial sources and middle-bit PDL operations.
- [x] Run red checks, implement the direct check, run focused checks and review.
- [x] Run full dynamic/static regression, affected leak-enabled sanitizers,
  formatting/naming and the existing scale check; record exact evidence.
- [x] Update current documentation and commit the verified change.

Implementation and tests are integrated from /tmp/icl-scan-output-20261113,
based on9c615af. Product change:11lines in Connect; no public declaration,
source expansion, serializer or scheduling changes.

Red evidence: build/scan-output-red-{native,cli}.log/.exit(1). Native elaboration
accepted the first invalid source; all five CLI negative subcases instead
reached the later intermediate-tap scheduler diagnostic. Valid round trips
already passed. The direct check passes native and29CLI methods in
build/scan-output-focused-{native,cli}.log/.exit(0), CLI7.932s. Independent
source/test review has no findings. Initial format failure was confined to
one test call; final-format.exit(0), naming.exit(0), whitespace clean.

Three paired scale runs use Linux/g++16.2.1 -O2,5000instances/377,286ICL bytes/
15,002nodes plus the existing one-megabit sparse group and5000-child bus cases.
Whole baseline wall0.78–0.84s/RSS162,716–162,776KiB; changed0.83–0.88s/
162,568–162,620KiB. Main elaboration49.911–59.126ms versus46.424–81.145ms.
These noisy runs demonstrate exercised scale, not a speedup or a general
runtime guarantee. All six build/scan-output-benchmark-{baseline,changed}-{1,2,3}
logs retain exit0 sidecars.

Final full regression runs in build/scan-output-final-full.log/.exit.
Leak-enabled ASan/UBSan reuses the clean Task40 build directory and rebuilds
the changed source/test objects; no declarations changed. Selected checks are
native netlist, main29, shared-output9 and named-group11, with logs under
build/scan-output-final-sanitizer-*. All selected checks and the sanitizer build
pass: native netlist, main29(22.249s), shared-output9(3.527s), named-group11
(21.378s); final-sanitizer-all.exit(0).

Final full regression exits0:39 dynamic and39 static generator suites, native
checks,50 directional/seven legacy vendor standards,2,519 ICL files covering
334/119 parser/scanner rules, and six editor tests in177.787s. Five optional
ANTLR integrations remain skipped. Final source and evidence review is clean.
Strict coverage and other source-kind/alias requirements remain open; no new
vendor certificate is claimed.
