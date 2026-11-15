# Modeled root scans implementation plan

Execute inline in the existing isolated worktree after Task93 integration. Native
Superpowers workflow, Ponytail economy and the standing autonomous instructions
apply. Spec: docs/superpowers/specs/2026-11-15-modeled-root-scan-design.md.

Goal: support raw scans on a static modeled root client, with native state and
controlling fill retained for later ordinary read/write Applies.

Files: pattern_state.h/.cc adds ShiftComplete; pattern_scheduler.h/.cc adds
NormalizeScan/ApplyScan integration and immutable profile; pattern_opaque.h/.cc
shares existing scan normalization while retaining genuine opaque dispatch;
generator main queues scans and rejects unsupported combinations before merge;
maintain state/scheduler native tests, new test_modeled_root_scan.py and Makefile;
update the old modeled-rejection expectation only when that model becomes valid.

Interfaces: NetworkState::ShiftComplete(map<size_t,string> loads,
map<size_t,ScanControl> controls) validates all before one rising Clock and
SetRegister shift replacement. ScanScheduler::NormalizeScan(Operation) validates
request/profile without state mutation; ApplyScan(Operation) returns canonical
Operation and adopts native trial only after capture/shift/update/idle succeeds.
No new dependency, scheduler snapshot test API or alternate event model.

Review focus: native complete snapshot/events equivalence; final SI and ascending
index slicing; unknown input controlling fill; local-reset unchanged replay and
rollback; explicit end-state/resources/merge/computed reset/clock rejection.
Supported total width1..1048576, O((bits+registers) log(registers+1)) conservative work and linear space;
per-bit solver/native vector copying is excluded by the performance requirement.

- [x] Independent design review and builder refinements recorded in the spec.
- [x] RED native ShiftComplete vs literal per-bit replay, including update shadow
  bookkeeping, known/X payloads, reset history and invalid-before-mutation cases.
- [x] RED CLI visible model, unequal ascending chain, sequential raw/ordinary
  accesses, partial-X later masked write, overwrite/reset and unsupported atomicity.
- [x] Implement the shared normalization/profile, native bulk primitive and
  transactional scheduler/main path; no mixed accesses in one Apply.
- [x] GREEN native/CLI original/flat/generated/local-reset and connected suites.
- [x] Source-matched vendor reference and reproducible 65536/1048576-bit probes.
- [x] Independent final review, then immutable required leak/fullserial gates.
- [x] Commit feature, fast-forward master, delete merged task branch.

RemoteRED actual3testhashesmatch: CLI1 rejects modeledstorage at the existingopaqueguard; nativebuild2 reports missing ShiftComplete/NormalizeScan/ApplyScan. Production changes now in futureWT only, Root93 remains immutable. Shared NormalizeScan retains93payloadgrammar; modeled requests use static profile and explicitreset.

FirstGREENremote actual12hashesmatch butbuild2: shared-parser extraction retained a wrapped client.name reference after replacing its argument with target. Corrected that identifier; no native/CLI tests ran in the failedbuild. GREEN2 uses a fresh archive/outputprefix.

GREEN2 actual12hashesmatch: build/nativeState/nativeScheduler/opaque12 all0. ModeledCLI9 has2testissues: expected lowercase reset but diagnostic usesiReset, and an end-state directive was placed illegally on ScanRegister instead of Alias. Corrected fixture and assertion, retained explicitend-state rejection. Added native register-driven reset rejection afterglobalreset to pin the reviewed structural requirement.

Independentfinalreviewclean. Vendororiginal/normalizedboth0 andexportsbyteequal; actual7submitted+10preparedhashesmatch, coveragehelper7tests0. Scale65536fiveforms0(.07s/~57324KiB), but1048576failed at final empty Apply: Apply runs NeedsScanEvents and symbolicresetresourcecheck before Solve already returns on nooperations/noendgoal. Preserve that existing no-opcondition before event proof; firstraise maintainedlargeCLItest to1M andrecordRED. No reset limit is widened or waived.

Advanced onto verified6d23ea3 after Task93 full2passed1320.089s/editor186.379s. Committed21files on taskbranch, fast-forwardmaster anddeletedcompletedbranch. All18future files restored from /tmp/modeled-root-task94-before-advance-files; onlymaster+activeTask94 remain. GREEN4 actual12hashesmatch build/state/scheduler/modeled9(max1M)/opaque12all0. Style corrected eight new RawChain brace pairs and nowpasses; final scale2actual12hashesmatch:10commands0,65536allforms.05s/~51548KiB;1048576.30-.36s/~103940KiB, outputhashesmatch perwidth. No old functionalbaseline or speedupclaim.

Long-chain scale5commands0:4096registers .16-.23s/~86300KiB and canonicalsource/flat/generatedhashesmatch. Localconnected15checksall0(native7/main69/modeled9/opaque12/clock51/client25/reset12/multipleTAP22/feedback10); format/naming/offline53all0. Finalremote allnative7+43CLI running onactual29hash packet, extractedfile timestamps normalized to remote time to avoid transportclockskew while preserving bytes. Freeze21reviewedfiles next for required leak-enabled/fullserial checks.

Verified Task94: fullserial make check0 in1411.913s;43dynamic and43static
generator suites,53directional goldens,2530ICL files with334/119grammar/scanner
rules,LSP62 andeditor6 in184.300s. OptionalANTLR5 remain skipped. Required
leak-enabled build/native7/main69/modeled9/opaque12/clock51/client25/reset12 all0.
Finalremote52statusesall0 andactual29source/test/fixture hashes match. Local15
connectedchecks, formatting/naming and53offline certificates pass. Fifteen scale
commands0:65536bits .05s/~51548KiB;1048576bits .30-.36s/~103940KiB;4096registers
.16-.23s/~86300KiB, canonicalreplayhashesequal. Vendororiginal/normalizedboth0 and
byte-identical exports, actual7submitted+10preparedhashesmatch; -stableignored
and synthetic-chain policy prevents a strictcertificate claim.
The million-bit maintained RED exposed unnecessary event proof before Solve's
existing empty/no-end-goal return. Moving that condition after fill update and
before NeedsScanEvents passed GREEN and focused independent equivalence review;
no reset limit was widened and no transaction is skipped. Native bulk shifting
retains update shadows, final SI, event behavior and partial-X controlling fill.
Commit21files on taskbranch, fast-forwardmaster, deletecompletedbranch andpreserve
only the next static-alias draft. General retargeting/parallel/TAP scans, phase/
reset/resource/clock/pattern lifecycle, broader vendor, strictcoverage andfinal
platform requirements remain open. No push.
