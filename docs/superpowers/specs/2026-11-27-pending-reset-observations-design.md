# Preserve pending observations across ordinary resets (Task199)

This repairs the existing completion-program observation contract (original
items 3/6): preparation must preserve unread storage, and actual comparisons
must remain independent of expected data. The protected Task197 and Task198
binaries both accept a scan preparation that enables a primary read while an
acyclic DataRegister reset clears its source; actual readback becomes 0 instead
of the original 1. This is a correctness bug, not new topology support.

Use the existing mixed scheduler and pending-read dependency map. Retain a
frame-entry snapshot of pending register storage before pin/control/reset
preparation. After scan/data updates and reset settling, require that storage
to match its entry value until its read has captured. Existing intermediate
feedback checks and capture-before-update semantics remain. Do not constrain
actual data to expected comparison bits. Reuse AST handles, with O(pending
storage) additional handles per frame; no per-delta netlist traversal, new
protocol, dependency, event format or increased resource bound.

Reject impossible preparations atomically; preserve state, observations and
existing output files. Safe preparation, capture before a later destructive
update, unknown comparisons and actual mismatches must continue to work.

Implementation proceeds inline under the existing completion-program approval,
with one fresh whole-branch reviewer and one test-first Critical/Important
repair pass. Broad inventory, item 1 exclusion, 117 stashes and 44 inherited
Minors remain unchanged. New live-device or vendor qualification is unclaimed.
