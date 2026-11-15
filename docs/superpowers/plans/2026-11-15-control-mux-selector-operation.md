# Capture/update mux selector operation qualification plan

Execute inline in the existing isolated worktree; Root Tasks85–87 remain frozen.
Task88 scope code is independently reviewed and in broad remote regression.

Goal: qualify IEEE1687 6.4.6.4(b)/.5(b)/.9/.10 selector justification and zero
suppression for standalone nested clients, beyond shape/possible-identity checks.
Existing ClientScanControls helpers supply original/flat/generated replay.

Scope: existing tests/src/pattern/test_client_scan_controls.py; hand-maintained
vendor fixture/provenance only after a working local case is established; guide/
model ledger/this plan. Production scheduler/state changes only for a reproduced
failure with traced cause. Do not change OneHotReadResolver all-data interpretation
or claim general independent TAP/general-clock event support.

- [x] Typed CE mux: literal0/control arms, dynamic primary selector; CaptureSource
  differs from reset/storage; read requires automatically sensitizing CE path.
- [x] Typed UE mux: dynamic primary selector starts disabled; write/read requires
  selecting UE path and committing correct storage.
- [x] Reserved CE selector held at0: write and read retained shift state while
  capture is suppressed; read requiring CaptureSource cannot violate ownership.
- [x] Reserved UE selector held at0: unreachable write fails atomically, preserving
  pattern/local-reset/debug artifacts.
- [x] Run these cases against current candidate first; if behavior already
  satisfies them, add qualification without unnecessary production changes.
- [x] Inspect relevant scheduler phase constraints before correcting a failure;
  RED/GREEN plus native/connected/style checks and independent review as needed.
- [x] Qualify an actual Tessent directional round trip using existing repository
  vendor flow, with source/runner hashes and explicit direction; preserve failed
  pilots. No claim based solely on loading or native replay.
- [x] Combine the next reviewed acceptance changes for one immutable required
  sanitizer/full integration after verified Tasks85–87; preserve all other
  clock/reset/forcing/TAP/lifecycle/strict-coverage requirements.

Review focus: CaptureSource versus held shift/update state; reservation must
prevent selector changes; synthetic root client phases and child physical pins;
generated replay and flat origin preservation; failures after earlier valid
operations must not replace existing outputs. Small four-bit cases do not
establish broad performance or vendor coverage.

First baseline confirms CE selector can stay0 while generated read compares CaptureSource1010. Local original/flat/generated replay shares the missing requirement. Initial reservation fixtures assumed0 before taking; ResourceReservations::Assigned records only assignments under active ownership, so fix tests to iTake then iState. No new resource snapshot semantics are added. Held-shift positive now removes CaptureSource, preserving existing no-CaptureSource CE-suppressed read support. Source trace: ControlEvents(shadows=true) deliberately permits suppressed capture for writes, while Solve frame.read currently imposes no requirement that a declared CaptureSource is captured. Proposed focused fix: require actual capture event for selected direct/capture-route observations with CaptureSource, leaving write-only suppression and no-CaptureSource held reads intact.

Corrected baseline3:5cases,3failures. Direct/routed reads both emit cap_enable0, and reservedcap0read stillsucceeds. UEselectorjustification, ownedUE0 rejection and no-CaptureSource heldshift positive pass. Actual test and3 candidate-source hashes match. Independent diagnosis review supports requiring the existing effective capture event for selected direct and routed CaptureSource observations; no live expected-value constraint is introduced. Implemented only that shared local implication after RED.

Local combined native7/main69/clock41/client23/reset12/multi22/feedback10 all0, including expected5 comparison against modeled CaptureSource10. Independent final source/test review clean. Naming check caught Solve805statements exceeding800limit; extract only the shared RequireCapture helper into an adjacent method, preserving both callers and the same implication. Do not raise the threshold. First style failure retained.

Scan-driven capture-selector regression against first implication candidate fails1: target is necessarily on the prerequisite scan while capture is disabled. Requiring every active pending target to capture prevents writing the selector before a later observation. Ruling: express capture readiness inside the direct read predicate and selected-route validity instead. Incidental prerequisite scans may leave observation pending; completed observations still require the same effective capture event. Shared pure CaptureReady method also keeps Solve below the existing800statement limit; first805 andextracted801 naming pilots retained.

Final CaptureReady source and24client methods pass; independent reviewclean, style0 after retaining805/801 pilots. Scan-prerequisite RED1 is corrected with readiness in observation predicates and route validity. Two forward vendorcertificates pass unchanged; reversecapturecomparison2 and reverseupdateexport1 retained. All19vendorinputhashesmatch, offline53pass, gated16/32 original/flat/generated scale0equalPDL. Integrate combined88/89 now; required immutable leak/full and final12hashremote remain pending.

Advanced onto verified464b62f; previous14draft files/full diff/manifest and stash retained under/tmp/clock-output-control-tasks88-89-before-advance-files. Final87 docs retained; onlyscope section restored. Combined snapshot source/tests freeze for required final leak/full verification after localconnected/focused/style/offline gates.

Final combined88/89 remote integration passes50 statuses: build/native7/all41CLI/aggregate all0, with actual12source/test hashes matching the submitted immutable snapshot. Required local leak-enabled build/native7/main69/clock41/client24/reset12 all0. Full serial regression remains running; no production/test/case/certificate changes during these final gates. Evidence: build/clock-output-control-final-remote/evidence and build/clock-output-control-final-{sanitizer-*,full.*}.

Verified Tasks88/89 checkpoint: full serial make check exits0 in1381.145s,
41 dynamic and41 static generator suites,53 directional goldens,2525 ICL files
with334/334 grammar and119/119 scanner rules,LSP62 and editor6 in174.760s.
Five optional ANTLR tests remain skipped. Required leak-enabled build/native7/
main69/clock41/client24/reset12 all0. Final remote50 statuses all0 and actual12
source/test hashes match. All26 production/test/case/certificate files remain
unchanged from independent review through final gates. Commit this31-file
checkpoint without push; Task90 remains isolated. Broader model, vendor, strict
coverage and final platform requirements remain mandatory.
