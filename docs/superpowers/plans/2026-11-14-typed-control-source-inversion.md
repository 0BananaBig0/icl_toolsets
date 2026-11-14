# Effective typed-control Source inversion

> Execute inline under existing autonomous authorization. Root69 source/tests
> stay frozen during final SAN/full checks; next code stays in the isolated
> checkout. Preserve independent review, failing-first checks and exact backend
> phase-guard coverage. No new agents or discretionary confirmation loop.

Goal: reject effective inversion of matching typed TCK/TMS/TRST/CaptureEn/ShiftEn/
UpdateEn Source terminals in child bindings and corresponding supported To* Sources.
This is effective typed-control inversion validation, not complete grammar,
hierarchy direction, mux/output polarity or operational conformance.

Primary: IEEE1687 6.3.12 grammar and semantic rules, printed69–72, plus6.4.7(i–o)
for child bindings. reset/scan/data/clock signals permit '~'; the six control
signal productions do not. Data expressions retain their own inversion permission.
An inverted TCK in an ordinary clock_signal context is also permitted by that
grammar, even where current clock runtime later rejects unsupported phases.
Explicit ToTCKPort Source remains the existing lowered-model extension.

Independent design review supports checking matching typed terminals through the
existing canonical alias resolver, with inferred virtual references exempt:
InferResets deliberately inverts AsyncGlobalReset to supply active-low TRST.
Do not reject every inverted BitReference. Ordinary data/mux outputs and literal
sources stay eligible. DataMux choice/shape polarity stays separate.

Alias qualifications are explicit.6.3.12 lists typed-control aliases, whereas
6.5.2(e) lists data ports/registers. Also '~inverted_alias' has zero effective
inversion but contains syntax absent from the typed-control production. Preserve
existing even cancellation as a semantic compatibility interpretation, qualify
it separately from literal grammar enforcement, and probe direct inversion,
hidden odd inversion, inversion at use and two-alias cancellation independently.
Use primary first; record vendor differences rather than silently changing rules.

Design candidate: after existing kind classification, reject an inverted canonical
terminal only when its typed kind matches one of those six Source families.
Reuse cached terminals; no new pass, API, metadata, dependency or dense allocation.
Expected scale includes65536 control bits,4096 aliases and655360 ordinary data
references. Check repeated wide/deep/ordinary loading overhead and output hashes;
no new worst-case runtime promise. Preserve Reset/Scan inversion and ordinary-clock
TCK inversion, source/flat/generated replay and atomic output publication.

Reach: src/pattern/pattern_netlist.cc; existing native netlist/main CLI tests;
test_client_scan_controls.py and test_multiple_tap_scheduling.py contain inverted
child-TCK diagnostics that may move to frontend rejection. Preserve artifact
assertions; retain exact backend phase guard through explicit lowered coverage or
the existing ToTCK Source extension when appropriate. Inspect actual callers and
fixtures before implementation; no generic test-only CLI AST adaptation.

- [x] Complete matched-hash64-case vendor loading qualification, retaining direct,
  hidden-alias, two cancellation forms, ordinary data/mux and child To* baselines.
  These are loading-only experiments, not operational equivalence proof.
- [x] Add native and CLI RED odd-inversion cases in all relevant Source contexts,
  direct/child output terminals, aliases and mixed vector positions. Retain even
  cancellation, data/constant, Reset/Scan, clock-TCK and inferred TRST positives.
- [x] Implement the smallest shared guard, preserving extension/virtual behavior;
  repair only affected negative-fixture diagnostics without losing backend guards.
- [x] Focused/connected, representative loading performance, formatting/naming and
  independent final review. Keep original solver limits and functional assertions.
- [x] Integrate after verified69 commit, run full serial and leak-enabled checks,
  update current support/coverage ledgers and commit the verified snapshot.

Task69 final native7 and main53 leak checks pass; remaining CLI SAN/full checks
are still required. Its immutable3-file snapshot is saved separately; no70 code
has yet been written. Vendor probes run only in/tmp on the authorized account.
The SSH command is supplied as a quoted remote command with stdin closed, following
two completed earlier batches whose stdin-based SSH transports remained open.
This changes experiment delivery, not the vendor models, runner or checking rules.

All64 submitted/fetched model+runner hashes match.51 loading cases pass. Vendor
accepts direct/hidden typed inversions for TCK/TMS/CaptureEn/ShiftEn/UpdateEn,
while TRST inversions fail ICL64; both cancellation forms load in all six families.
Several ordinary-data drivers fail vendor ICL65 despite the generic primary
concat-data permission; inverted qualified capture/update/TMS mux outputs load.
These are vendor extensions/restrictions distinct from the narrow standard-first
guard. No PDL/operational equivalence is inferred. Bounded SSH transport exits124
after remote work; fetched archive and per-model exits remain authoritative.

RED native1 and CLI1 (44 accepted invalid cases) are retained in
build/typed-control-inversion-red-remote/evidence; baseline library remains69.
The shared shape scale loop from69 is also present in these newer native tests.
Initial implementation adds matching-kind/effective-inversion rejection only in
CheckSignalSources, reusing cached terminals and preserving virtual/extension/data
cases. Remote GREEN and connected fixture review remain required. Root69 full
regression is still running with unchanged source/test hashes.

The two existing phase-guard negatives retain their backend intent through the
already-supported explicit ToTCK Source extension. Standalone idle forwards~ck
through a named ToTCK output; the multi-TAP case selects~extra_clock through its
existing ClockMux/ToTCK forwarding. PDL, exact inverted-phase assertion and prior
artifact-preservation checks are unchanged. New main44 negatives separately test
frontend rejection of directly inverted typed controls. No generic test-only CLI
adapter or new production extension was introduced. Connected reruns remain
required before claiming those updated phase cases pass.

Initial remote GREEN passes native netlist and all54 main CLI methods, with current
3-file source hashes recorded. No old generic-kind positive needed modification.
The full54-method run takes14.943s. Independent current code/test review finds no
consequential issue, including virtual TRST, even alias cancellation, data/clock/
Reset/Scan exceptions and the two phase-fixture extensions. Updated remote native7
and official dynamic41 checks are now running. Root69 full remains isolated.

Task69 is committed as14236ce after full serial0 (1618.418233s/editor6 211.728s)
and all required leak checks. Its root snapshot is unchanged. The next worktree
was safely advanced to14236ce on task/typed-control-source-inversion, preserving
five draft files and full old diff in/tmp/typed-control-inversion-task70-before-
advance-files and a retained stash. Only those5 files were restored and formatted.
The baseline69 executable and hash are retained for interleaved loading comparison.
Current local connected native7/main54/clock41 pass; remaining selected CLI cases
are running, including both updated backend phase fixtures. Root code remains69.

Final local connected all0 includes native7/main54/clock41/client18/reset12/
multiple-TAP22/feedback10. Both updated phase fixtures retain their assertions and
pass. Formatting/naming and independent final code/test review pass. All48
interleaved source/flat loading runs pass with matched pattern/flat hashes versus
14236ce. Cases:65536 TCK bits (146 bytes), six8192-bit control families (585 bytes),
4096 alternating/cancelling aliases (88008 bytes),655360 ordinary references
(335 bytes). Wide TCK source/flat0.064–0.115s versus0.064–0.115s, peak53008KiB
versus53080KiB; all-family0.064–0.115s; deep0.114–0.115s; ordinary0.115–0.165s.
These end-to-end empty-PDL measurements include debug-output I/O; no guard-only,
worst-case or speedup claim. Exact commands, environment/compiler, binaries/source
hashes, repetitions, wall/RSS are in build/typed-control-inversion-loading-scale.

The5 final files are saved with SHA256 under
/tmp/typed-control-inversion-task70-reviewed-files and imported unchanged into
root. Root native7 plus main54/clock41/client18/reset12 leak checks now run.
Source/tests freeze until final SAN/full completion. Remote broader checks remain
pending and are not used to claim final platform qualification.

Root final native7 and main54/clock41/client18/reset12 leak-enabled ASan/UBSan all
pass, aggregate typed-control-inversion-final-sanitizer-all.exit0. All5 root hashes
match the immutable reviewed snapshot before starting final serial make-j1check
in build/typed-control-inversion-final-full.log. Source/tests remain frozen.
Remote aggregate/native7/official dynamic41 exits are also0; transport124 is not
relabeled as success. Submitted and actual remote source hashes match, and final
production source plus all3 Python files are identical; only native-test formatting
differs. The completed archive is retained under
build/typed-control-inversion-connected-remote/evidence. This supplies additional
Rocky8/GCC15/Tcl8.6 evidence, not final all-tools/static/install qualification.

Final serial make-j1check succeeds in build/typed-control-inversion-final-full.log/
.exit0,1381.251796s wall:41 dynamic and41 static generator suites,51 directional
goldens,2523 ICL334/119 rules, LSP62 and editor6 in173.740s. Optional ANTLR5 skipped.
All5 final root hashes match the reviewed snapshot. Required leak-enabled native7
and main54/clock41/client18/reset12, connected/performance, naming/style and final
independent reviews pass. No solver deadlines, assertions or coverage thresholds
changed. Remaining reset polarity/defaults, direction/forcing/operation, runtime/
TAP/lifecycle, strict coverage and vendor/platform acceptance remain mandatory.
