# Restored hardware source scope implementation plan

> Execute inline with superpowers:executing-plans under existing autonomous
> authorization. Root76 remains frozen through full verification. Independent
> design/implementation review is required; no delegated implementation.

**Goal:** Enforce local-source/direct-child-output scope for accepted hardware
terminals after restoring flat origins, preserving generated TAP operation.

**Architecture:** Extend the shared per-consumer accepted-terminal check using
canonical terminals and original expression scope. Legalize generated TAP edges
through normal output ports and root TDI reuse. Reuse AddNode/Connect/Spend;
append cached wrappers after existing generated nodes to retain runtime node
identities. No new metadata version, provenance exemption or whole-graph pass.

**Tech stack:** C++17, existing native/Python tests, serializer and TAP assembler.
**Spec:** ../specs/2026-10-25-icl-pattern-generation-design.md; IEEE1687 6.3.12.

## Evidence and scope

Current reviewed76 accepts both altered-flat cases with exit0: Rootresult Source
changed from child.out to child.incoming, and Rootresult changed from child.out to
child.leaf.out. Original legal models and altered reimports all0; exact commands,
models, binary/source hashes and logs are build/restored-source-scope-baseline-
remote/evidence/comparison.json. Current source hash matches the reviewed76 file.
The initial remote quoting pilot failed before execution and remains separate.

Original direct multi-level hardware syntax is already rejected by parser
signal_reference (Task40); do not add a redundant Resolve guard or claim a new
original-source parser fix. Deep PDL targets use ResolvePdlTarget separately.
The gap is flat semantic reconstruction, where parser names are initially local.

For checked hardware roles, eligible non-output terminals must be in expression
scope; eligible output terminals must be in a direct child scope. Input Source
bindings use their parent scope. Constants/virtuals/holes retain handling; aliases
resolve only to their terminal port/object. Preserve all earlier kind/inversion/
rightmost/direct-DataMux errors. Keep PDL alias declarations, interface membership,
semantic reset associations, DifferentialInvOf, ToIRSelect and unchecked lowered
ToTCK Source under their existing separate contracts. Their unresolved rules are
not waived. ResetSource inference uses local inputs/direct child outputs/virtuals.

## Generated TAP boundaries

SerialAssembler currently uses child IR storage directly in root mux selectors,
child-internal data controls/logic directly in sibling bindings, and child TAP SI
as the initial root-route source. Replace that initial route source with the same
root TDI pin already connected to TAP SI. Keep original runtime control IDs.
Expose IR through one vector DataOut per required TAP; expose raw phase inputs
through matching ToCapture/Shift/Update outputs; expose selection logic through
ToSelect outputs. Preserve bit ranges/inversion and existing PDL/reservation names.

Record rewrites while retaining existing Connect calls during Add so later TAPs
see already-bound controls. Finalize after all old nodes are built: cache one
wrapper per source, append wrappers using existing naming/limit helpers, and
rewrite only recorded bindings/selector vectors. This retains original node IDs
and generated names. Do not allocate per-reference wrappers or scan every graph
edge. Charge new source bits/objects against existing limits. Review this design
before code, especially cross-TAP ownership and deferred rewrite ordering.

## Reach and performance

Modify src/pattern/pattern_netlist.cc and src/pattern/pattern_tap_network.cc;
existing native flatten/netlist/access-link/TAP tests and main CLI tests; necessary
fixtures only after concrete failures. New test scripts belong under tests with
source-relative paths. Update generator guide/model ledger/this plan.

Hoist expression scope once per checked role. Retain O(consumed bits + alias
resolution) validation; wrapper work is O(recorded connections + IR widths),
with bounded per-source reuse. Compare unchanged76 and new loading on four wide
output families, scalar scan, deep/shared aliases and655360 ordinary references;
also measure1000-interface and multiple-TAP construction/replay. No unmeasured
speedup, full vendor equivalence or completed coverage claim.

## Review focus and steps

- [x] Add RED altered-flat input/internal/grandchild/sibling scope matrices,
  alias/vector/partial cases, parent binding and legal original/flat positives.
  Preserve alias-only/PDL/reset associations and exact earlier diagnostics.
- [x] Add generated TAP original/flat scope assertions and IR/control/select/
  multi-TAP directional replay tests before changing lowering.
- [x] Legalize only recorded generated endpoints; verify raw IDs/names, original
  binding ownership, width/inversion and bounded wrapper reuse.
- [x] Enable the accepted-terminal scope predicate and run focused/connected
  GREEN without generated-name or metadata-provenance exemptions.
- [x] Measure representative loading and TAP scale, style, independent review,
  immutable integration after76 commit and required leak/full serial checks.
- [x] Record exact evidence/remaining rules and commit the verified snapshot.


Design review is clean, with a required ownership detail: Finalize cannot use
Connect to replace an already-bound raw source. Store destination IDs/offsets and
expected old bits; check they still match before direct replacement. Node/vector
references cannot survive AddNode reallocation. Wrappers append after all old
nodes, cache by actual source ID and charge full source width once. Disabled and
unselected routes receive the same legality checks. No new Resolve guard.

A consumed cross-scope Alias is itself an invalid hierarchical hardware reference,
even when its canonical terminal is a valid child output or literal. Check raw
alias ownership before the terminal's constant/virtual early return, then check
the canonical terminal scope. Local aliases retain the same terminal checks.
The new RED matrix includes child aliases to outputs and constants, both mixed
vector lanes, partial positives, parent-bound local inputs and deep outputs.

Initial RED build0/TAP1 (old child-TDI endpoint)/CLI1 (altered source accepted),
actual4hashes retained. Native flatten RED first stopped on invalid literal-Alias
fixture syntax; removed that unsupported construction. Literal/virtual aliases
are not representable through current Alias grammar; no serializer extension.
The reachable child.wrapped output-alias case remains. Added multi-TAP wrapper
append-order and per-IR reuse assertions to preserve both TAPs' original IDs.
Corrected native RED follows before production changes.

Corrected RED2 build0/flatten1 (missing scope rejection)/TAP1 (old childTDI)/CLI1
(altered source accepted), actual4 hashes matched. No production change preceded
RED. Root76 committedd891771 after full0 in1318.863s/editor6 174.320s and all
required leak0/current remote native7/41 hash9match; first failed benchmark run
and0.839s corrected benchmark retained.77draft/full diff saved under/tmp/restored-
hardware-source-scope-task77-before-advance-files and stash; advanced ontod891771
and restored only3 next tests and this plan. Final76 docs retained.


### Restored hardware source scope and generated TAP outputs (2026-11-15)

Accepted hardware references are checked after restoring original scopes: local
non-output terminals or outputs of a direct child instance. Input Source bindings
use the parent expression scope; other checked roles use the consumer scope.
Cross-scope raw Alias references also fail, even if the canonical terminal is a
legal child output. Alias declarations remain subject to their existing separate
rules. Kind, inversion, rightmost-register-bit and direct-DataMux diagnostics keep
precedence. Constants, virtuals and holes retain handling; semantic reset fields,
interface membership, DifferentialInvOf, ToIRSelect and the lowered ToTCK Source
extension keep their existing contracts. This does not complete their remaining
source, alias or operational rules.

Two current76 counterexamples are recorded: altered flat Source to child input
or grandchild output loaded0 despite the declared scope. Original deep hardware
syntax was already rejected by the parser; this closes reconstructed-model scope,
not a new parser gap. Native matrices cover10 families, raw nonlocal aliases,
parent bindings, deep/internal sources, partial and both mixed vector lanes;
main62 preserves atomic pattern/reset/debug rejection. The unsupported literal-
Alias RED fixture was removed before implementation; Alias grammar excludes it.
Corrected RED native/CLI/TAP failures demonstrate missing scope and old TAP wiring.

Generated TAP sources now use root TDI for initial routes, a shared vector DataOut
for IR selectors, typed phase outputs and ToSelect for selection logic. Existing
Connect calls retain multi-TAP ownership checks while Add records exact edges.
Finalize checks the recorded old source and appends one cached output per source
after all original nodes. Raw runtime IDs/names, bit shape and inversion remain;
new bits/objects spend the existing budgets. No metadata provenance/name exemption,
new version or blanket graph pass is introduced. Single/multi-TAP assertions and
original/flat execution pass, including wrapper append order and IR reuse.

Local connected native7/main62/clock41/client18/reset12/multi22/feedback10 pass;
independent code/test review finds no consequential issue. The benchmark preflight
passes0 in0.897s under its unchanged10s deadline. Style/naming passes after two
brace-only fixes. All84 paired source/flat loading comparisons againstd891771
preserve output/flat hashes: four8192-bit families0.04–0.08s; scalar scan0.02–
0.04s;262144 Reset mux0.52–0.57s;4096 aliases0.06–0.12s;4096 shared outputs
0.06–0.08s;CE-only2621440.31–0.33s;ordinary655360 references0.13–0.16s versus
0.13–0.17s baseline. Exact commands/environment/compiler/hashes/time/RSS are in
build/restored-source-scope-loading-scale/comparison.json. These measure whole
CLI source/flat loading, not isolated guard cost or a memory-saving claim.

Three interleaved native TAP comparisons retain all tests:3000-interface network
construction0.164–0.183s versus0.144–0.158s baseline; whole TAP test processes
1.12–1.17s versus1.12–1.15s, peak122620 versus122720KiB (allocator variation).
Extra boundary ports have a measured construction cost; no speedup is claimed.
Evidence is build/restored-source-scope-tap-scale/comparison.json. New generated
TAP flat files replay correctly. Older native TAP dumps with direct private
references must be regenerated from original ICL/BSDL: baseline76 emits the
sample0 and new code rejects its private core.select source2, recorded under
build/restored-source-scope-legacy-flat. No legacy migration is added.

Required leak/full serial integration and broader remote qualification are pending.
The remote behavior snapshot precedes the two brace-only style fixes. Complete
source-special-context/alias rules, gating/selector behavior, runtime/TAP/lifecycle,
strict coverage and final vendor/platform/static/install qualification remain
mandatory. This is not full IEEE or vendor PDL equivalence certification.


Final leak-enabled ASan/UBSan native7/main62/clock41/client18/reset12 all0,
aggregate build/restored-source-scope-final-sanitizer-all.exit0. All5 changed
source/test hashes match the reviewed integration snapshot. Broad remote native7/
all41 pass with actual5 hashes matching the submitted pre-brace behavior snapshot;
final brace-formatted source-matched flatten/TAP/main62 also all0 with actual5
hashes matching. Both completed evidence archives fetched0. The focused runner's
inherited copy overwrote a remote scratch baseline name; local76 baseline binaries,
gitd891771, models/source manifests and all measured reports remain retained. That
scratch name is not used as a performance baseline. Full serial integration starts
now; no full-suite pass is claimed until its aggregate completes.


Final Task77 full serial make-j1check0 in1338.899s, recorded in
build/restored-source-scope-final-full.log/.exit/.wall:41 dynamic and41 static
generator suites,51 directional goldens,2523 ICL files/334grammar119scanner rules,
LSP62 and editor6 in174.509s pass. Optional ANTLR5 skipped. Required leak-enabled
ASan/UBSan native7/main62/clock41/client18/reset12 all0. All5 changed source/test
hashes match review. Broad pre-brace remote native7/all41 and final source-matched
flatten/TAP/main62 pass with actual5 hashes matching each submitted snapshot;
completed archives fetched0. Older TAP dumps with direct private references need
regeneration from original ICL/BSDL. Remaining special port-source/association,
alias, gating/selector, runtime/TAP/lifecycle, strict coverage and complete vendor/
platform/static/install requirements stay mandatory; this is not full conformance.

Commit this verified77 snapshot; OneHotScan collector78 tests remain isolated.
