# Model Source Audit Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans inline, task-by-task. Steps use checkbox syntax.

**Goal:** Repair empty control-output flattening and missing RefEnum diagnostics, and continuously audit default model/canonical round-trip acceptance across the maintained corpus.
**Architecture:** Surgical production validation/rendering fixes; one public-API native batch inventory plus a Python registry/protocol checker; existing corpus/build/regression workflows own execution.
**Tech Stack:** Existing C++17/Python standard library/shared frontend; no dependencies, RTL or public product API.
**Spec:** docs/superpowers/specs/2026-11-23-model-source-audit-design.md

## Global Constraints
- Parentmaster24aac8a; reused isolated .worktrees/bsdl-defaults, one branch codex/model-source-audit, preserve117 stashes and unrelated worktrees/ledgers.
- User delegated choices/opted out of questions; choices are ours. Fresh builder /root/model_source_audit_builder_check: nothing important left.
- Empty To* port bodies use semicolons; other existing empty-port spellings and nonempty property/attribute/source bodies remain.
- RefEnum errors must be scoped/source-located; preserve valid forward/namespace/specialized/symbolic enum behavior and wrong-kind/width guards.
- Native helper uses Design/Elaborate/SerializeIcl, one load per file/effective qualified module default. Do not invent missing definitions, bindings or parameter values.
- Corpus manifest currently2562files/1228parser-rejected/1334accepted;1763effective default modules:1004existing canonical+12empty-To failures become1016required canonical;56declared unsupported serializations;691 original default elaboration errors remain reported.
- Registry identities join corpus-relative file and qualified effective module key; module source overrides use existing Design ordering.
- Audit one native subprocess, verify current input hashes and complete protocol, actual canonical byte equality, fresh reports, required successes/precise unsupported boundaries and no raw map::at. No per-module process/padded data/cache machinery.
- Large SAN audit measurements use ASAN_OPTIONS=detect_leaks=1:allocator_release_to_os_interval_ms=500 and UBSAN_OPTIONS=halt_on_error=1; actual256MiB/1024KiB quarantine/redzones remain unchanged. Default SAN policy does not meet the measured memory ceiling.
- Representative1334files/1763module batch<=10s and<=512MiB; record real measurements/environment/hashes; no broader performance/equivalence claim.
- New scripts under tests/ mirrored paths. Review JSON under owned build/ or /tmp. No modifications to external/vendor inputs or old artifacts.
- One fresh most-capable final reviewer; one Important/Critical test-first fix pass, defer Minors, rule on declined-to-judge lines, no rereview.
- Do not close whole inventory11/12 or excluded coverage1 from this increment; continue original authorized inventory after merge.

## Review Focus
- Bare root/hierarchical default control outputs retain semantic defaults after canonical flatten/reparse; attributed/sourced ports retain their bodies.
- Missing RefEnum names do not escape as STL failures, while valid same-named scoped/forward/specialized enum identities and symbolic values remain correct.
- Corpus identity/coverage cannot confuse duplicate module names, namespaces, overrides, missing/changed files, incomplete backend output or stale reports.
- Incomplete grammar models and unsupported AccessLink/PortGroup/LaunchEdge contexts are visible and correctly distinguished from required canonical successes.
- Public-API inventory performs every required actual roundtrip, remains bounded at maintained scale, and handles failing modules without losing unrelated results.

### Task 1: Contextual model validation and canonical empty output ports
**Files:** Modify src/pattern/pattern_netlist.cc, src/pattern/pattern_flatten.cc; maintain tests/src/pattern_netlist_test.cc, tests/src/pattern_flatten_test.cc and tests/src/pattern_state_test.cc as justified; create tests/src/pattern/test_model_source_audit.py and register it in Makefile.
**Interfaces:** Existing Connect scoped RefEnum resolution and Declaration empty-body branch; Design/Elaborate/SerializeIcl/RestoreFlatOrigins remain unchanged publicly.
- [x] Write native/CLI RED cases for all supported bare To* port forms, root and hierarchical defaults, attributed/nonempty/explicit/inferred Source forms, canonical idempotence and nonempty ordinary source/flat/generated PDL. Native default state/control associations must agree across restored metadata. Preserve other empty-port spelling. Retain meaningful parent failure evidence, including the12 corpus cases.
- [x] Write missing RefEnum RED tests requiring source/node/enum-name diagnostics and unchanged prior output. Cover alias/register/typed vector-port forms, forward declarations, wrong-kind/width, scoped same names, parameterized instances and symbolic reset/default values. Reuse existing enum helpers/fixtures.
- [x] Extend only the existing empty-body semicolon condition to empty To* ports; make the initial RefEnum scoped lookup checked before assigning enumeration ID. Keep existing error wrapping and validation, no broad exception translation or guessed enums.
- [x] Build dynamic/static generators, native netlist/flatten/state suites; run the new CLI plus source literals, typed port, source/reference/default/enum/flat relevant existing modules. Run twelve corpus regressions through actual canonical roundtrip and all14 missing-RefEnum examples through contextual diagnostics. Expected source/flat/generated valid behavior unchanged and atomic rejection correct. Commit/task-done with full focused evidence.

### Task 2: Reproducible batch default-model inventory and registry
**Files:** Maintain tests/audit_icl_corpus.py and tests/tests/test_audit_icl_corpus.py with backward-compatible effective module keys; create tests/src/pattern/pattern_netlist_inventory.cc, tests/src/pattern/audit_model_corpus.py, tests/cases/model_source_acceptance.json and tests/tests/src/pattern/test_audit_model_corpus.py; modify Makefile and maintain Task1 CLI as needed.
**Interfaces:** Helper executable build/pattern_netlist_inventory accepts file arguments; emits one JSON record per input file with path and module records. Each module includes qualified key, actual stage, success/error, node/flat-byte counts as applicable. Stages load/elaborate/serialize/flat_load/flat_elaborate/canonical distinguish expected boundary from broken roundtrip. File I/O/setup errors are process failures; per-model source/serialization failures remain data. Actual canonical success means SerializeIcl(restored)==first SerializeIcl, never assumed.
- [x] Define/test the minimal JSON protocol with multiple files/modules/namespaces, ordered overrides, dependency/parameter failures, known unsupported serialization, valid roundtrip and malformed protocol/empty successful backend/failed backend. Tests reject incomplete coverage and do not treat errors as success.
- [x] Implement the native helper using public APIs and existing JSON/shared build objects. Reuse Design per file, explicit qualified module keys, true flat load/elaboration/canonical comparison, bounded per-model data; no scheduler/solver or copied private simulation.
- [x] Implement CLI --root (default repository tests), --corpus-report (fresh existing parser audit JSON), --inventory, --registry and --report. Validate input digests, backend completion/order/identity/stages, duplicate/missing/unexpected results and registry identities. Report all actual results, source/product/registry hashes, timing/counts; write a current failed report on audit rejection, never reuse prior success. Reject unknown internal map::at diagnostics. Expected original nonrequired elaboration failures stay reported without fake repairs.
- [x] Create registry from actual baseline evidence:1016required canonical module identities and56known unsupported serialization identities with precise diagnostics. Pair module key with corpus-relative file, retain namespace/override semantics, no guessed values or cross-file aggregation. Require deliberate update for changed unsupported boundary; no automatic promotion.
- [x] Add model-corpus-check with both native support dependencies and existing parser/dedup audit producing a fresh corpus report before model audit. Have corpuscheck consume that target, avoiding duplicate parser audit invocation in one run. Add maintained unit check to existing corpuscheck; no install/runtime dependency.
- [x] Run unit/protocol tests RED→GREEN, full fresh maintained audit, default/namespace/reference native and CLI regressions. Verify expected1016canonical/56unsupported/691original failures and1763effective models/1334files; investigate real disagreements rather than changing expectations. Measure normal/sanitizer batch<=10s/512MiB and retain hashes. Commit/task-done.

### Task 3: Full qualification, independent review and integration
**Files:** Maintain selected BUILD_DIR parser passthrough in Makefile with tests/test_makefile_model_corpus.py; update README.md as warranted, docs/pattern-generation.md, docs/pattern-model-coverage.md, docs/remaining-work.md, this spec/plan and owned evidence.
**Interfaces:** Native/CLI Task1 behavior and Task2 actual audited registry; exact executable/input hashes qualify results.
- [ ] Run14 native/14 leak-enabled sanitizer suites, full dynamic/static generator regressions including new module, source/enum/flat/corpus affected sanitizer CLI and audit unit/protocol/helper checks, changed-code format/tidy/strict C++17, existing parser/dedup policy and source packaging. Expected zero unexpected failures; excluded strict coverage1 remains unchanged/unrun.
- [ ] Measure three fresh normal/sanitizer corpus invocations at maintained size, record wall/RSS/environment/all input/product hashes and representative case outcomes; each<=10s/512MiB. Preserve current callback1024-target<=10s and wide-alias<=256MiB using existing measured drivers/controls on current binaries; do not reuse prior whole-tree acceptance claims against changed source.
- [ ] Document commands, file/module/default/roundtrip boundaries, real counts, precise incomplete/unsupported contexts and remaining source specialization/event/vendor work. Verify runnable nonempty valid examples source/flat/generated and diagnostic/old-artifact failures. Distinguish author validation from independent reader testing.
- [ ] Obtain one fresh independent most-capable whole-branch review with Review Focus. Re-grade by user effects; Critical/Important one test-first fix pass and required verification, Minors ledger/defer, declined-to-judge lines get explicit rulings; no rereview.
- [ ] Commit qualified result, merge into direct parentmaster, verify integrated actual inputs/products/focused checks, push, detach reused worktree and delete only completed branch. Verify clean master==origin/master,117stashes, archive/remove only own ledger. Continue remaining authorized inventory.
