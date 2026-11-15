# ToIRSelect owner controls implementation plan

Execute inline under existing autonomous authorization; draft follows Task78.

**Goal:** Enforce IEEE1687 6.4.6.23(a,b): every module owning ToIRSelectPort
has exactly one TMSPort definition and at most one TRSTPort definition.
Count declarations, not vector bits. Preserve independent interface, implicit
binding and scheduler ambiguity rules. Parent/sibling/child declarations cannot
satisfy the owning module. Generated BSDL selectors use DataInPort and need no
exemption. RestoreFlatOrigins runs before validation; do not use stale kinds_.

**Approach:** Reuse the per-(scope,kind) port collection already constructed in
ValidateInterfaces. In its existing node loop, validate ToIRSelect owners before
processing interfaces. Avoid another full graph pass, storage or API. Expected
work stays O(nodes log port-kinds); count lookups do not expand vector widths.
Use source-located diagnostics naming ToIRSelect and the required declaration.
Existing wrong-kind negative fixtures containing a ToIRSelect must gain legal
owner context to retain their original diagnostic coverage.

**Evidence:** IEEE printed107 / cached6728. Task78 special-source-context vendor
probe contains15 owner-context cases: missing TMS3 failICL45, TMS1/2 with
TRST0/1/2 all6 load, vector/indexedTMS withTRST0/1/2 all6 load. Vendor is more
permissive than the explicit standard declaration-count rules. Record the
standard/vendor difference; do not infer vector scheduler semantics from loading.

**Reach:** src/pattern/pattern_netlist.cc, existing native netlist tests, main
CLI atomic artifact test, necessary demonstrated fixtures only; generator guide,
model ledger and this plan. No new test scripts outside tests. No RTL changes.

**Steps:**
- [x] Review design and native/CLI RED tests before implementation.
- [x] Test TMS0/1/2 x TRST0/1/2, unused ToIRSelect, parent/sibling/child ownership,
  multiple selectors, indexed/vector definition counts, source-flat replay and
  altered flat restored owners. Keep earlier kind and two-input ScanMux tests.
- [x] Watch missing-TMS and extra-declaration rejection tests fail on unchanged78.
- [x] Add minimal check using existing port map; repair legal-context fixtures.
- [x] Connected GREEN, independent review, style and measured wide/many-owner
  loading. Retain original native limits and optional tools status.
- [x] Integrate only after verified78; leak-enabled and required full checks,
  exact evidence and remaining tasks; commit verified snapshot without pushing.

Ruling: combine Task79 owner controls and Task80 ClockPort input association
into one final verified source-context snapshot after independent RED/GREEN for
each. Both touch the same collector and native/CLI tests. Reuse one immutable
leak/full regression and final review, avoiding duplicate22-minute full runs;
any failure still blocks both. No acceptance criterion is removed.

Test preflight: bind unrelated child TMS inputs explicitly to1'b0, so implicit
source inference does not preempt the intended owner checks. Independent native/
CLI review clean after this correction. Production remains unchanged78.

Combined RED1 build0/native1 records both missing guards, but3 CLI subcases
hit later event-integration diagnostics. Preserved pilot and corrected unused
owner declarations into child modules with explicit TMS/TRST bindings. Corrected
RED2 build0/native1 (both missing guards), CLI1 with all7 invalid models accepted0.
Actual3 source/test hashes match submitted snapshot. Production unchanged until
RED2 was inspected. Added shared count/type/scope checks only afterward.
Independent production/tests/necessary fixture review clean; remote native/full
main65/clock41 GREEN running. Root78 source remains frozen for its full run.

Remote GREEN build0/native netlist0/main65 0/clocks41 0, actual4 source/test
hashes match. Legal local input pairs, all owner/altered-flat cases and atomic
artifact rejection pass. Existing clock safety and bound-negative metadata tests
retain their assertions. Broad remote native7/all41 is running. Local CPU tests
wait until Root78 full verification completes; no source/test mutation in Root.

Task78 committeddf746a0 afterfull0 1345.595s/editor175.874s, allrequiredleak0
and finalremote7/41 actual5match. Saved all12draftfiles/full diff/manifest under
/tmp/source-context-associations-task79-80-before-advance-files and retainedstash.
Advanced onto verified78 and restoredonly4 next source/test files and future
plans; final78 code/docs retained. Local connected/performance/style runs serial.

Local connected: native7/main65/clocks41/client18/reset12/multi22/feedback10 all0.
Format/naming0. Independent production/fixture review clean. Loading comparisons
60 source/flat runs all0/equal PDL/flat hashes versus verified78.5000 owners
.25–.31s both;65536-bit owner controls .03–.07 vs .02–.08;65536-bit clock pairs
.03–.04 both;4096 aliases+4096 pairs .11–.18 vs .10–.18;655360 ordinary refs
.14–.17 vs .11–.17. Empty-PDL loading only; no unmeasured speedup or generic TAP
runtime claim. Environment/commands/input/binary/source hashes/wall/RSS are in
build/source-context-associations-loading-scale/comparison.json. Original wide
owner model loaded successfully; no speculative child-model substitution used.

Immutable integration after verified78 begins now; required leak/full/broad
remote completion remains pending. No push requested.

Broad remote build/native7/all41 CLI/aggregate all0, actual4 source/test hashes
match. Retrieved archive50 status files all0 under build/source-context-
associations-connected-remote/evidence. Required leak-enabled native7/main65/
clocks41/client18/reset12 all0, aggregatefinal-sanitizer-all.exit0. Full serial
regression now running. Root4 source/test hashes remain the reviewed snapshot.

Combined79/80 full0 wall1335.270639s/editor6 174.677s; dynamic41/static41,
51directional,2523ICL334/119,LSP62,optionalANTLR5skip. Reviewed4 source/test
hashes unchanged, leak aggregate0, all50 remote status files0 andactual4match.
Tasks79/80 complete together; no Source-less ToTCK or output-association claim.
