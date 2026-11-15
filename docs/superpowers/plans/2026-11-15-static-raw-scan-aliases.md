# Static raw scan aliases implementation plan

Execute inline in the isolated worktree while Root Task94 stays frozen. User's
standing autonomous scope and per-task integration policy apply. Spec:
docs/superpowers/specs/2026-11-15-static-raw-scan-aliases-design.md.

Goal: allow aliases/inversion on qualified modeled static root serial paths.
Files: pattern_scheduler.cc RawProfile/RawChain/ApplyScan; maintain native scheduler
and modeled CLI tests, replace only obsolete blanket-negative expectations; add
static_raw_scan_aliases fixture and update guide/model coverage.

Interface: keep NormalizeScan/ApplyScan signatures unchanged. Profile stores each
register ID/downstream parity and final total parity. Typed native event handling,
shared payload parsing and publication stay unchanged.

Review focus: polarity direction and SO-only inversion; alias index ordering and
wrong taps; terminalSI and cycle/complete-path checks; X fill and final physicalSI;
no alias-fed reset/control or mux/state/end-state widening.

- [x] Independent design review and builder corrections recorded in the spec.
- [x] RED native and CLI direct/input/output/midchain/vector/doubleinverse cases.
- [x] Minimal iterative alias/polarity normalization, unchanged native phases.
- [x] GREEN source/flat/generated/localreset, actual state/fill and unsupported
  topology/rollback plus necessary connected callers.
- [x] Source-matched vendor evidence and wide/deep runtime/memory measurements.
- [x] Independent final review and immutable required sanitizer/fullserial gates.
- [x] Commit feature, fast-forward appropriate integration branch, delete merged
  task branch and preserve only subsequent unfinished work.

FirstremoteRED build2 was a test include omission (NetworkState/ScanControl require pattern_state.h), not the intended feature failure. Added the direct include and rerunRED2 before production edits. Actual2testhashesmatched.

RED2 actual2hashesmatch: build0/native1/4CLI methods1 with12failed subcases, all the intended staticprofile rejection. Implement selected-bit alias tracing and total/downstream parity only; preserve physicalpayloads/nativeevents. Replace obsolete inversion-only negatives with a legal mux topology still outside the boundary. Task94 emptyApply guard independently accepted as exactly equivalent to Solve no-op semantics; its Root source remains frozen.

FirstGREEN actual4hashesmatch build/state/scheduler/all13modeled/all12opaque0. Native16polarity/alias combinations inoriginal/flat compare literal perbitstate/pins andlaterXfill. Independentfinalreviewclean, includingfixtureSOexpectations. Added explicitwrongalias-tap atomicregression to fulfilltheplannedboundary; no productionchange needed.

Advanced onto verified88a55ea after Task94full0(1411.913s/editor184.300s), allSAN0/finalremote52all0actual29. Task94committed21files, fast-forwardmaster, deletedbranch; saved27draftpaths/restored10futurefiles from/tmp/static-raw-alias-task95-before-advance-files. Currentbranch task/static-raw-scan-aliases, onlymaster+active remain. Vendororiginal/normalizedboth0 byteequal, actual6submitted+10preparedhashesmatch; wrongalias-tap boundary0. Scale15commands0: ordinary1M.30-.36s matchesTask94canonicalhash andtiming; inversewide1M.35-.43s/~130452KiB;4095aliasdepth.06-.10s/~64976KiB. No new unsupportedbaseline speedupclaim. Wholechangednative-test lint found5brace pairs (2new,3existing samefile), formatting-only correction; production scheduler lint0.

Localconnected15checksall0(native7/main69/modeled14/opaque12/clock51/client25/reset12/multipleTAP22/feedback10), Source+wholeNativeTest naming/format0 andoffline53all0. Independentreviewclean, vendorbyteequal and15scalecommands0. Finalremote all7native+43CLI inprogress withactual33source/test/fixture packet. Freeze12reviewedfiles (8nondocument) for required leak/full gates; no further production edits until those finish.

Verified Task95: fullserial make check0 in1427.583s;43dynamic and43static
generator suites,53directional goldens,2531ICL files with334/119grammar/scanner
rules,LSP62 andeditor6 in184.278s. OptionalANTLR5 remain skipped. Required
leak-enabled build/native7/main69/modeled14/opaque12/clock51/client25/reset12 all0.
Finalremote52statusesall0 andactual33source/test/fixture hashes match. Local15
connectedchecks, formatting/naming and53offline certificates pass. Independent
review foundno consequential issue. Native per-bit replay validates16polarity/
alias combinations inoriginal/flat, actualregister/pinvalues andpartial-X fill.
Fifteen scalecommands0: ordinary1M .30-.36s matchesTask94canonicalhash andtiming;
inversewide1M .35-.43s/~130452KiB;4095aliasdepth .06-.10s/~64976KiB, canonical
replayhashesmatch. Vendororiginal/normalizedboth0 withbyte-identicalexports,
actual6submitted+10preparedhashesmatch, wrongalias-tapatomic0. Tessent-stable/
synthetic-chain discrepancies remain, no strictcertificate claim.
Commit12files on taskbranch, fast-forwardmaster anddeletecompletedbranch. Preserve
only the next hierarchy draft. The old116-file preserved-draft audit found45current
byte matches,61masterhistory matches and10olderstatus-onlyplandrafts; no unique
unfinishedbehavior, archiveintact at/tmp/icl-branch-cleanup-20261115/content-review.json.
General retargeting/parallel/TAP, phase/reset/resource/clock/lifecycle, broader
vendor, strictcoverage andfinalplatform requirements remain open. No push.
