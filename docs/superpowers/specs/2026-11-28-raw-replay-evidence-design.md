# Raw replay evidence (Task219)

Close three inherited test-evidence gaps. Keep independent source and flat reset
rewrite paths in RawMergeCalls.check and compare the two texts before replaying
the source rewrite. Preserve all procedure bytes and existing checks. For raw
merge benchmark widths above 4, set a high bit in the first payload and a distinct
middle bit in the second, retaining the low nibble sequence and narrow fixtures.
For the merged-readback wide matrix, use at least two raw calls so the second
capture observes the first high-bit payload. Keep PIO wide-call counts unchanged
and honor larger explicitly requested raw counts. No producer/API/dependency or
resource-limit changes; no speedup claim.

Use a real-native reset artifact-corruption control to show the old helper misses
a changed flat rewrite and the new helper rejects it. Add maintained fixture/matrix
tests before the benchmark edits: independently pin complete wide ordered frames
and actual high-bit captured notes in source/flat/generated/control forms. Prove
RED then GREEN. Run complete affected modules on normal/static/default-leak SAN
binaries, plus representative maintained 512/1,024-bit raw benchmark matrices.
Do not run or relabel the known unsupported 1,024-bit PIO memory case as passing;
this change neither closes that finding nor raises its bound.

Only four existing Python modules plus tracker/spec/plan change: raw helper,
two benchmark drivers and benchmark reporting tests. Preserve 4,131 unaffected
qualified inputs and all 48 product bytes from Task218. Package seven exact files;
retain native/build/broad evidence by unchanged identities with no fresh such run.
Private reset probe mirrors the source under tests/tests/src/pattern, is archived
and removed before packaging. Existing 65/90-second subprocess limits remain.
Parent master fc5837a65c850406f4a686a41b69cd08ea04a32c;
branch codex/raw-replay-evidence. Standing question opt-out/inline/merge/push
applies. One fresh whole-branch review, one blocking repair wave if needed,
no rereview. Inherited 17 minus 3 candidate closures = 14 before new findings.
Preserve 117 stashes and unrelated state; integrate, verify, push and retire the
owned branch after archiving only owned helpers/ledger. Item 1 stays excluded;
broader original inventory remains open.

Review repair: preserve the low nibble at widths 5–7 by choosing the second
added bit at max(4, width//2). At width 5 only bit 4 lies above the low nibble,
so both payloads use that upper position while their low nibbles distinguish
them. Widths 6–7 use bit 4 for the second payload. Literal three-call checks
cover widths 5–16; a native 5/6/7-bit matrix and all three full-module variants
verify the fix. Previously measured 4/512/1,024-bit fixtures remain byte-identical.
