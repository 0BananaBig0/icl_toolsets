# Opaque root scans implementation plan

Execute inline in the existing isolated task worktree. The user's standing
instruction is to finish the requirements autonomously and integrate each verified
task before deleting its branch. This is one bounded deliverable within the full
scan requirement, not a waiver of modeled, nested, TAP or parallel scan support.

Goal: lower IEEE1687 7.9.12 raw scans for a genuinely opaque root single-chain
client into validated Level0 PDL, preserving variable-length controlling/expected
payloads and explicit transaction boundaries.

Architecture: reuse Runtime for Tcl execution, call qualification, operation
ordering and pending-operation validation. Add a dedicated root-opaque path before
the modeled scheduler; validate its topology and reject unrepresented interactions.
Extract the existing shared frontend scan whitespace normalization for reuse,
then canonicalize data with existing PDL-number padding. No new dependency.

Alternatives: extending the general serial solver now would require representing
variable-length opaque edges, ownership and mixed modeled state at once. Blind
textual passthrough would omit length, target and payload validation. Choose a
bounded validated lowering path first; modeled scans remain explicit failures.

Constraints: no register, nested scope, TAP or modeled mux/state on this path;
exactly one explicit root client with unsourced scalar ScanOutPort and scalar
ScanInPort. No -ir/-chain/vendor skip, merge, take, clock, state, override or
modeled reads/writes. Existing global reset and notes are allowed. Local-reset
rewriting preserves source unchanged because this path has no modeled local reset.

Review focus: overwritten/discarded malformed payloads still fail; reset clears
pending scans; based-data width cannot silently truncate leading zero/X digits;
empty applies cannot invent scans or repeat comparisons; atomic failure preserves
all outputs and debug/reset artifacts. Length1..1048576; work linear in total input
and emitted payload size, no per-bit solver/subprocess. Large-data test and a
reproducible timing/memory probe qualify scale.

Primary: IEEE1687 7.9.12 and 6.4.5/6.4.6.2/6.4.16. Tessent2026.2 accepts the explicit
register reference fixture but rejects the genuine opaque fixture at Apply because
the path is unconfigured, and ignores -stable. Preserve these discrepancies; no
vendor certificate is claimed for this subset.

Files: pdl_document.h/.cc exports shared scan normalization; generator main owns
opaque validation/queue/lowering; tests/src/pattern/test_opaque_root_scan.py and
Makefile add maintained source/flat/generated/local-reset and atomic regressions;
opaque_root_scan fixtures retain the original primary examples and separate
explicit-model reference probe. Update guide/model coverage after verification.

- [x] Independent design review, resolve blockers before production edits.
- [x] Write maintained RED tests and verify missing-iScan failure.
- [x] Implement minimal validation, canonicalization and transaction queue.
- [x] GREEN focused/connected checks, topology and unsupported combinations.
- [x] Source-matched remote regression, vendor discrepancy and scale evidence.
- [x] Independent final review and required immutable sanitizer/full gates.
- [x] Commit on task branch, fast-forward master, delete merged branch.

Review accepted bounded lowering; dispatch before merge and require unconditional direct interface pins. RED maintained payload test fails exactly at missing iScan scheduling. Default/stable state is not automatically replayed by this subset: empty applies perform no scan. This does not claim general blackbox lifecycle support.

Implementation adds pattern_opaque.h/.cc to the existing solver object list; main serializes validated operations through the existing Tcl command writer. Added focused RED for based lengths (frontend already accepts PDL numbers) and exact1048576-bit generated replay. The latter exposed PdlNumber counting its two-character prefix against the bit limit; allow prefix overhead while keeping the same numeric bit bound. The first suite had8/9pass; remaining active-polarity fixture used SelectPort illegally, corrected to ResetPort and added conditional SO Enable rejection.

Final review found missing DefaultLoadValue validation and discard-only replay non-idempotence. Both reproduced RED in maintained tests before fixes. Require numeric explicitly sized default1..1048576bits as a documented subset restriction (explicit sizing is not a general IEEE grammar rule). Drop empty applies consistently with ordinary scheduler; no transaction/compare is invented. First connected/remote runs are pilots, final hashes/regressions must use corrected source/tests.

Final focused12methods0(1.55s), prior sevennative/main69/clock51/client25/reset12/multipleTAP22/feedback10all0; onlytheknownnewreviewtests failed on the pilot binary and passed after fixes. Format/naming/offline53all0. Six reproducible local scalecommands0:65536bits .06s/~30980KiB,1048576bits .07-.08s/~38636KiB, outputhashesequal perwidth; no unsupported oldbaseline or speedupclaim. Reviewfixes have RED/GREEN evidence. Freeze20files for required leak-enabled and fullserial gates; seventeen source/test/fixture hashes go to finalremote separately.

First fullserial run failed2 after1021.167s at the maintained coverage source-completeness test: pattern_opaque.cc missing from tests/coverage.py SOURCES. Existing test provided RED. Added the actual new implementation to measurement; no exclusion/waiver. Production/native/CLI files remain unchanged. Snapshot now21files and the focused coverage tests must pass before restarting fullserial check.

Verified Task93: fullserial make check0 in1320.089s;42dynamic and42static
generator suites,53directional goldens,2529ICL files with334/119grammar/scanner
rules,LSP62 andeditor6 in186.379s. OptionalANTLR5 remain skipped. Required
leak-enabled build/native7/main69/opaque12/clock51/client25/reset12 all0.
Finalremote51statusesall0 withactual17source/test/fixture hashes matching;
supplementalcoverage7tests0 andactualsubmittedhashesmatch. Firstfullrun2 at
1021.167s caught the missing coverage source entry; tests/coverage.py now measures
pattern_opaque.cc, with no exclusion or waiver. Reviewed production/test/fixture
bytes stayed immutable. Sixlocal scale commands0 at65536/1048576bits; maximum
.07-.08s/~38636KiB, repeatedoutputhashesequal. Opaquevendorpathfailsconfiguration
whilemodeledreferenceexports0; -stableignored byTessent, no opaquevendorcertificate.
Commit21files on taskbranch, fast-forwardmaster, deletecompletedbranch andpreserve
the nextmodeled-scan draft. Broader retargeted/parallel/TAP scan, phases/reset/
resources/vendor/strictcoverage/finalplatform requirements remain open. No push.
