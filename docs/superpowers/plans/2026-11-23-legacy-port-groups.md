# Legacy PortGroup Membership Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Resolve accepted legacy PortGroup membership and preserve valid groups in canonical flat ICL.

**Architecture:** Keep annotation references in a dedicated native bit-reference vector;
reuse scoped signal resolution, flattened signal rendering and existing origin restoration.
Validate representation and endpoint scope before serialization and after restoration.

**Tech Stack:** Existing C++17 frontend/netlist/serializer/native suites; Python3 unittest/Make.

**Spec:** docs/superpowers/specs/2026-11-23-legacy-port-groups-design.md

## Global Constraints

- No grammar/scheduler/storage/hardware transport expansion or new dependency.
- Netlist.port_groups continues to mean existing segmented port functions.
- Existing1MiBit Resolve per-group bound, new4MiBit aggregate legacy-member budget.
- Preserve member bit order/duplicates; members must name actual local ports.
- Invalid/missing members remain visible, with source/group context; no invented definitions.
- All new maintained scripts under tests/src/pattern/; preserve117stashes/unrelated worktrees.
- Corpus3normal/3configuredSAN batches <=10s/512MiB; fullSAN quarantine/leak/UB checking.
- Scale1/64/1024groups x64memberbits, vector and segmented modes, normal/SAN,
  three repeats each, source/flat/generated forms:108 fresh measured invocations,
  each <=10s/512MiB with exact replay; record environment/input/tool/output/RSS.
- One fresh final independent review, one Critical/Important test-first fixpass,
  deferMinors/no rereview. Coverage1 excluded; whole11/12 remains pending.

## Review Focus

- Commented/inline AST layouts and keyword members must share identical membership.
- Flattened child input retyping/segmented physical names must restore local scope.
- Unknown/nonport/out-of-range/malformed public-API members reject atomically.
- An annotation must not become a data-flow edge, wire, storage/client or PDL target.
- Duplicate/reversed ranges and near-limit expansion must preserve order and bounds.

### Task 1: Native resolved membership and canonical serialization

**Files:** Modify src/pattern/pattern_netlist.h, src/pattern/pattern_netlist.cc,
src/pattern/pattern_flatten.cc, tests/src/pattern_netlist_test.cc and
tests/src/pattern_flatten_test.cc.

**Interfaces:** Add std::vector<BitReference> NetlistNode::port_group.
Add void ValidatePortGroupMembers(const Netlist&), enforcing the spec's member
and representation rules. Consume existing Netlist::Resolve, Expression scopes,
BitReference and Signals; SerializeIcl signature/schema stay unchanged.

- [x] Add native positive canonical/membership tests for inline/comments/keywords/
      duplicates/vector/parameter/forward/segmented/repeated-child forms, plus
      unknown/nonport/range negatives. Add malformed payload/type/scope/index/
      constant/inversion/properties/attrs and forged-origin guards, PDL annotation
      target rejection, aggregate4MiBit boundary/beyond.
- [x] Build/run separate netlist and flatten REDs on unchanged implementation.
      Expected: unknown members wrongly accepted; valid groups cannot serialize.
- [x] Add the dedicated payload, skip generic PortGroup body property collection,
      resolve braced source tokens for both AST layouts using existing limits,
      and debit4MiBit aggregate budget before over-expansion.
- [x] Implement shared validation after origin restoration and before serialization;
      emit PortGroup with existing flattened signal rendering. Preserve segmented
      port handling and genuine keyword members.
- [x] Run native netlist/flatten/state and relevant existing CLI source/segmented/
      launch/default/model suites on dynamic/static generators. Expected: all pass,
      exact member references/order/canonical bytes and meaningful negative diagnostics.
- [x] Commit native deliverable; completion command:
      python3 /tmp/legacy-port-groups-task1-check.py.
      Expected: focused native/CLI/quality evidence and exact source/product hashes verified.

### Task 2: Replay, corpus acceptance, scale and full qualification

**Files:** Create tests/src/pattern/test_legacy_port_groups.py.
Modify Makefile, tests/cases/model_source_acceptance.json,
docs/pattern-generation.md and docs/remaining-work.md.

**Interfaces:** Consume Task1 port_group payload/ValidatePortGroupMembers and
unchanged SerializeIcl public behavior. Reuse existing CLI replay helpers and
corpus producer/model inventory/registry protocol.

- [x] Add meaningful CLI tests for nonempty source/flat/generated replay of root/
      vector/commented/keyword/hierarchical/segmented/SIB cases, failed-definition
      prior-artifact protection, and forbidden group access/reservation targets.
      Register the suite in both full generator variants. Expected: all pass on Task1.
- [x] Run fresh full actual corpus, inspect every changed identity, then update only
      proven outcomes in registry. Expected: all valid PortGroups canonical, unknown
      member fixtures original elaboration errors, all other acceptance preserved.
- [x] Measure108 fresh scale/replay invocations at the global counts/modes/budgets
      and6 full-corpus batches. Expected: exact output equality, complete fresh
      artifacts, all measured bounds with environment/input/tool/output hashes.
- [x] Document install/existing command usage, annotation semantics/current counts,
      limits and exact example. Full14native/14leakSAN, both complete generators,
      normal/configuredSAN parser+model corpus/producer8/protocol13/Make3,
      changedC++format/tidy/C++17 and source packaging. Expected: every required
      gate exit0, no skipped required suite; exact source/product receipts.
- [x] Commit and request one fresh independent whole-branch review. Test-first
      fix Critical/Important in one pass, deferMinors, no rereview; rerun affected
      full qualification after relevant source changes. Record all rulings.
- [x] Merge into master, verify exact merged inputs/products and focused integrated
      checks, push/detach preserved worktree/delete merged branch/archive ownledger.
      Completion command: python3 /tmp/legacy-port-groups-completion-check.py.
      Expected: qualified/integrated receipts, clean pushed master, onlymaster,
     117stashes preserved; continue original inventory.
