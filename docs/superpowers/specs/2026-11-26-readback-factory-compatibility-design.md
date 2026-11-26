# Readback factory source compatibility

Restore the three-argument MakeModeledReadback function-pointer signature lost
when child-history support added a fourth defaulted parameter. Existing callers
must retain identical observation, status-preview and history behavior.

## Approaches

Keep the four-argument default: ordinary calls work but old function pointers do
not. Change callers to four arguments: forces a public source migration. Restore
the explicit three-argument overload: preserves the original API and matches the
existing one- and two-argument forwarding overloads. Choose the last approach.
Remove the fourth parameter's default to avoid ambiguous three-argument calls.
The new overload forwards with a null history-output pointer; the implementation
that owns model state remains unchanged.

This small C++17 repair follows the standing instruction to finish remaining
tasks autonomously. It adds no dependencies, scheduling behavior or input limits.
The forwarding call has constant overhead; there are no added scans, copies or
per-observation allocations. Existing native tests exercise real returned data.
No speedup or broader hardware support is claimed. Original excluded1 remains.

Acceptance: a typed three-argument pointer compiles, links and passes the existing
status-preview/delta test; one/two/four-argument calls continue to work; the full
project check command passes. Obtain one final branch review, merge and push,
delete the owned branch, and preserve all117stashes and sibling workspaces.

## Regression-gate repair discovered during verification

The full project run exposed five stale forward child-history certificates.
The qualified pre-repair binary and the candidate both fail the same five of77
directional snapshots, solely because newer history annotations are absent from
their recorded flat output. Refresh only these five with fresh licensed Tessent
exports using the existing preparation/record/check pipeline. Preserve the old
certificates as evidence; do not normalize away notes or edit certificate hashes
by hand. No generator or certification-policy logic changes. Retain all passing
unaffected suite results and rerun the failed golden target after certification.
Report the initial full-command failure and the subsequent focused gate repair
separately; do not call this a fresh all-green full-command run.

The review-required full rerun reached the main recipe and exposed an existing
coverage inventory omission: pattern_readback.cc is absent from coverage.SOURCES.
Add that source to the existing inventory and run its existing regression. Do not
change excluded item1 thresholds or attempt coverage closure. All prerequisites
passed in the full rerun; retain their source/product identities, then execute
the entire main check recipe after this one inventory correction. Report that
resumed verification accurately rather than claiming every prerequisite reran.

Final review correction: the initial prerequisite failure also skipped the main
recipe. The fresh full rerun completes every prerequisite and exposes the source
inventory omission described above. Its RED/GREEN repair and complete resumed
main recipe close the gap. Evidence remains composite, not a fresh single-pass
all-green command.
