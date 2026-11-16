# Task114: literal unknown SI on qualified raw TAP paths

Task113 merged into master f37363d and its branch was deleted. The user
delegates execution and routine design choices without confirmation; these are
executor choices. Builder/preflight must settle qualification details before
implementation. Source discovery used current Root NormalizeTapScan/TapScans,
ApplyScans, NativeTapControls/TryNativeTapScan, ReplayTapStep and ReplayScan.

IEEE1687-2014 7.9.12(c,e), printed170: SI is controlling data and uses normal
PDL numbers/padding. Existing modeled non-TAP ReplayScan shifts literal X into
hardware state, updates controlling fill only for known SI bits, and rejects X
for already protected explicit bits. TAP NormalizeTapScan currently rejects all
X before replay. Removing the rejection alone is incorrect: ordinary symbolic
Apply chooses binary loads and exact raw packet proof then fails.

Reuse the existing TAP cycle/replay engine for a literal raw candidate on a
complete copy. Construct the existing ScanStep/ScanLane from RawTapRequest and
reuse ReplayTapStep. Retain exact IR N+6 / DR N+5 cycles and numeric LSB-first
mapping/parity. Do not replace X with current storage or arbitrary solver bits.
Preserve source SO as authored comparison, including comparisons differing from
cached capture, as existing binary TAP and non-TAP raw emission allow.

Reuse or factor existing canonical single-root TAP/inactive immutable reset and
phase/clock/held-IR selection qualification, keeping the current event-work
guard. For IR qualify the fixed instruction path; for DR reuse RawDataPath and
NativeTapControls. A changing data/control-dependent selection or unsupported
reset/clock remains bounded failure for the new X profile. Existing all-binary
ApplyTapScan behavior and fallback remain unchanged. No new path walker, TAP
FSM, symbolic engine, dependency or raised limit.

Replay the complete execution copy with per-edge reservation checks. Validate
actual final addressed loads and native update eligibility; X transitions may
not appear as known ControlEvents, so absence of a known event is not proof of
no update. Copy the complete final hardware state, including off-path effects;
refresh only known addressed controlling-fill bits as existing ReplayScan does.
Reject X on established protected addressed bits, while surrounding protected
bits retain their requirements. Existing stable actual post-route check and
pre-CSU assigned footprint remain. State/fill/execution/caller outputs publish
only on complete success. Queue cancellation, last-value, source-order, scope,
flat/generated/local-reset output reuse existing front-end flow.

Before implementation: save Task113 reference binaries; prove true native and
CLI runtime RED on a simple canonical 8-bit modeled DR and 4-bit IR X input,
with known binary control cases succeeding. Check IR/data/builtin execution
details and all consumers; builder check must resolve unknown qualification
details rather than assume this draft is executable.

Acceptance: independent original/flat literal three-valued pins/storage/TAP
phase checks for 0/1/X SI, final-exit bit, all-X, masks, parity, unequal multiple
registers, ascending indices, later partial writes exposing retained fill,
unknown IR followed by known instruction recovery, stable post-unknown failure
and valid retry, preservation/caller sentinel rollback, builtin behavior where
the existing model represents actual update. Leave unsupported builtin or gated
profiles explicit; do not claim broad equivalence. Strict existing API tests and
all binary raw suites must remain unchanged apart from obsolete X rejections.

Measure representative8/32/256-bit routes,64x4 fragments,512 aliases,1024
unrelated nodes and repeated packets with matched flags, artifact/state hashes,
wall/RSS and existing guards. Unknown-SI old binary is an intended RED, not a
completing baseline or a speedup claim. Use known-SI equivalent controls only
where equivalence really holds. Fresh independent review, full/SAN/remote and
genuine vendor runs are required; record actual vendor support/trace limits.

## Approaches and decision

Removing the X rejection alone leaves ordinary Apply choosing binary data and
fails exact raw proof. Extending the entire symbolic event engine to model
literal unknown serial input would alter unrelated ordinary scheduling. The
selected approach is a dedicated raw candidate constructed from existing
RawTapRequest, replayed by existing ReplayTapStep/Cycle under existing native
qualification. It preserves exact serial input and leaves ordinary/binary
planning unchanged. A builder check precedes the implementation plan.

## Final preflight decisions

Builder check: nothing important left. Reuse ordinary Apply on a disposable
proof copy to validate existing target privileges, access groups, end-state
obligations and one exact binary CSU shape. This binary proof is discarded;
its chosen fill never replaces literal SI. Require that its endpoints, phase,
loads/writes keys and all known SI positions, full expected maps and serial
length agree with the raw request. Rebuild only load/write/serial fields with
the exact literal request, then replay from the original execution copy. This
retains existing validation without duplicating Solve's front end. The returned
step and actual hardware are the literal replay, never the binary proof.

Extract only the existing native scalar-root/ready/inactive immutable resets
and event-work qualifier into NativeTapShape(); ordinary native callers reuse
it with unchanged required-empty behavior. IR uses fixed instruction geometry;
DR reuses RawDataPath. NativeTapControls qualifies bound phase/clock and held-IR
selection. Leave unsupported profiles explicit. After literal replay check all
actual selected loads, reset/update eligibility and NeedsEndState(EndGoals).
Use DesiredPreserved for existing value validation/known authored refresh,
add the same explicit protected-X rejection as modeled ReplayScan, and check
every edge. Unknown hardware writes update known-only fill, matching ReplayScan.
No ControlEvents-presence requirement is invented for X updates.

## Verified vendor boundary

Tessent2026.2 rejects literal X in SI in both IR and DR, forward and reverse.
This local three-valued feature has no new vendor certificate. Keep literal
input and strict comparison unchanged; preserve the four genuine failed runs.
Unknown instruction masks remain compact when possible, as the32-bit17-X
recovery test demonstrates. The4096 event-work rejection remains unchanged.
