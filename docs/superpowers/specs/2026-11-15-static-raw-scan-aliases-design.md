# Static raw-scan aliases and polarity

Next bounded increment after verified Task94. Goal: normalize pure root aliases
and inversion on the existing complete static modeled serial path without changing
physical iScan SI/SO data or the native event model.

Primary IEEE1687 6.3 scan_signal permits optional inversion;7.9.12 uses physical
SI/SO payloads. Black-box interface non-inversion remains separate. Existing
scheduler Replay already maps loads with total path parity XOR downstream parity.
Reuse that algebra for bulk raw loads. Expanding the full solver or introducing
another simulator would duplicate working phase/control/reset handling.

Allow pure root Alias nodes with complete Source vectors, no reset/end-state
metadata, and selected-bit forwarding using existing Offset. Walk SO to SI
iteratively, accumulating every reference inversion before recording each
register's downstream parity. Keep visited(node,index) keys and separate register
uniqueness. Resolve each register to its actual rightmost bit and verify the
terminal scalar SI index. Require all registers on the complete path as before.

For each right-end payload slice, invert known bits iff total parity differs from
the register's downstream parity; retain X. Native state/fill receives these actual
register values. Emitted raw SI/SO and final SI pin are unchanged. SO-only inversion
changes no loads, and double inversions cancel. ShiftComplete/capture/update/idle
are reused unchanged. Pure aliases add no events; controls and resets remain
qualified direct root signals, so alias-fed resets/controls remain excluded.

No mux, hierarchy, TAP/IR, parallel, opaque behavior, ownership, mixed pending
access or end-state widening. Existing explicit reset/length/budget checks remain.
Supported1..1048576bits and4096registers, bounded iterative alias traversal under
existing graph limits; no recursion or per-bit full-vector shifting. Use standard
maps/sets with logarithmic lookup costs and linear stored metadata/payloads.

Independent builder review accepts the parity proof. It requires inversion to be
accumulated before recording registers, visited bit keys plus register uniqueness,
actual rightmost-bit validation after alias resolution and terminal SI index.
No additional model/API is needed.

Verification: native literal per-bit hardware comparison for asymmetric known/X
payloads and unequal ascending chains; all held values/pins and subsequent fill;
source/flat/generated/unchanged-local-reset replay of input/output/midchain aliases,
vector ordering, double inversions, wrong taps and unsupported mux/reset/endstate.
Source-matched vendor probe and wide/deep measurements; required sanitizer/native/
CLI/fullserial gates before commit/fast-forward/delete of the task branch.
