# Ordinary logical read scope

Original item6 remains open. Confirmed ordinary prefixes write13 and read only
bit3 with X,then continue with a modeled getter. The original prefix returns
1xxx;the generated prefix returns1101. Standalone clients,interface-free scan
registers and primary outputs reproduce this under the pushed Task189 parent.
Native scans,symbolic scans and primary-input reads lower comparisons separately.
Their physical comparison masks do not encode which X bits were requested.

Preserve the explicit logical target,requested bit scope,comparison and selected
observation frame when generating ordinary logical PDL. Requested X bits must
still update history. Unrequested bits of those explicit observations retain
their previous known/unknown history. Routed DataRegister reads must retain their
logical identity and cared comparison rather than becoming physical output reads.
This increment does not close broader item6 or certify internal monitor histories.
IEEE1687-2014 7.9.10(d-f) requires X reads to be observed and cared values to be
compared using the first observation. Section8.4.1 tracks the latest iRead
separately for each index;unread indices remain unknown. The local normative
extraction was read before implementation;incidental tool shifts are not reads.

Reuse AppendReadScope from Task189. Give the native logical scan path the
requested scope without changing TryNativeTapScan's exact physical/raw contract.
Apply the existing symbolic logical reconstruction to ordinary explicit reads,
preserving documented implicit TAP capture checks. In SolvePrimaryWrites,build
logical comparisons and scope from the actually selected read choices on each
frame;different guarded bits may be observed on different frames. Keep existing
route guards,actual observations,ownership,fill and native replay checks.

The alternatives are a serializer-only filter,which has lost the selected logical
route/frame by emission time,or new public metadata/history syntax. The three
existing lowering boundaries already have the required requests and frame
information. Reuse their shared slice helper instead of introducing either.
No new dependency,public scheduling API,clock assumption or resource increase.
Physical/raw scan masks and existing implicit IR checks remain unchanged.

Acceptance covers original/flat consumers and original/generated prefixes from
default/modeled producers:explicit X and cared bits,ascending/inverted aliases,
discontiguous ranges,known previous history,multiple applies,different selected
primary frames,routed DataRegister targets,single-TAP IR/DR slices,ordinary
client switching,read cancellation and publication safety. Retain Task189's
grouped/other-frame/reset-held preparation and Task188's mixed-TAP cases.
Update the native event oracle to recognize logical read targets while keeping
its independent physical-output and old-storage assertions. Do not weaken an
assertion merely because the implementation changes its output spelling.

Measure sparse primary histories at256/512/1024bits and scan/client cases at
256bits. Pair the existing seven full-read ordinary client control points on
frozen parent/current binaries with identical inputs and output bytes;measure
corrected sparse cases separately because the parent publishes wrong histories.
Use three rounds. Each new point checks both producers/eight continuations in
10CLI processes. Measure each producer's generation/four continuations separately
against10s and report the combined duration too,with512MiB reported runner/children
RSS. Keep larger/unqualified cases explicit;no speedup
claim over the parent's incorrect partial-read output.
Parent/candidate probes retain the existing4096/65536primary scheduling limits;
2048bits is at the solver boundary and remains unqualified. The separate65536bit
history-only tests do not establish active scheduling at that width.

Standing user instructions delegate routine implementation decisions and cover
repository/worktree/tmp edits,Git commits,push,merge and owned branch cleanup.
Parent is pushed master2fa029d372f7042d1249d2caa3ca472c5871dfb4;reuse the clean
.worktrees/bsdl-defaults workspace and preserve117stashes/sibling workspaces.
One final fresh review,one Critical/Important repair pass,fresh required
qualification,merged-result checks and branch cleanup are required. Broader
original2/3/4/5/6/7/8/10/11/12 and excluded1 remain open.

Qualification consumer expansion: five forward certificates need distinct
native logical and vendor physical traces after the identity correction. Keep
schema3 unchanged. Optional schema4 reference-vendor equivalence verifies fresh
exports of the previous schema3-certified program and the current program;
their resolved physical groups,values,writes,reads and ordered applies must match.
Bind the prior certificate,its unchanged source hashes and exact reference
program. Retain both fresh jobs/input bundles and the current native logical
trace,exact regeneration and local idempotency. Contract tests reject altered
provenance,stale inputs,changed cared values,extra applies and trace mutations.

Actual original-source probes yield four successful exports and one pending-
iState rejection (queued_state); independent source schedules may differ. These
are diagnostics,not the reference for this directional refresh. No original
pending-iState vendor acceptance or whole-program vendor-equivalence claim is
added. The reference bundle is prepared using the existing harness,then its
input is replaced by exact previously certified bytes before vendor execution;
its generator command records preparation,while source_reference identifies the
actual prior program and generator. Completed native/SAN/scale checks may be
reused only with unchanged source and product hashes. Preserve all failed full
runs;run the changed contract tests and a fresh full make check.
