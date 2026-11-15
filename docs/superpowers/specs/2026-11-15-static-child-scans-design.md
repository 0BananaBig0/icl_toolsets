# Static child-interface raw scans

Task97 follows the frozen Task96 snapshot. IEEE1687-2014 §7.9.12 explicitly allows
modeled iScan; its target is the effective-prefix client interface, and its length
and SI/SO data describe the path between that interface's own SI/SO ports.

Reuse the qualified complete static root chain and native bulk replay. A child
interface is eligible only when explicit, client, scalar, nonparallel, with unique
noninverted local port references and a nonempty complete static serial subpath
between its own SI/SO. Stop tracing at its SI before following the parent binding.
Every subpath register must belong to the root path in the same contiguous order.
Keep Task96 controls/reset/end-state/TAP/mux/conditional-output restrictions.
This extends target selection and payload insertion, not clock or native semantics.

Cache profiles by interface node as contiguous root intervals, width/physical
payload offset and two conversion parities. Do not duplicate register vectors
per interface. The child-to-root SO suffix difference is constant across its
contiguous registers; SI conversion is that difference XOR local total XOR root
total. Validate contiguity and the constant difference while tracing. The root
profile remains authoritative for physical output. Bound total profile tracing
to16M graph steps before allocation/work, including unsuccessful qualification. Qualify once, then work in
O(root bits + queued payload bits + profile traversal) per Apply, with existing
1Mbit/4096register/depth128 limits. Never replay per-bit vector shifts.

NormalizeScan returns a validated logical child operation, not a prematurely
expanded physical scan: surrounding controlling fill may change before Apply.
The setup queue is keyed by interface name; later assignments replace the same
interface only. Distinct disjoint child paths may share an Apply. Reject overlap
between different interfaces explicitly instead of silently dropping either.
Mixed ordinary/raw pending accesses remain rejected in this increment. Reset
clears the whole queue; malformed assignments are validated before replacement
or later reset. Bound the total normalized queued payload to1Mbits (the full
root limit), counting same-target replacement only once. Every supported
disjoint queue fits; this prevents overlapping invalid queues from multiplying
wide payload memory before Apply. Preserve original source operations in
local-reset output.

At ApplyScans, initialize every root register's desired actual load from existing
controlling fill, not sampled readback or capture results. For each target,
slice its normalized logical SI by local register widths, then invert known bits
by local total parity XOR register-to-local-SO parity. X stays X. Translate those
actual loads into the root physical SI slices with root total parity XOR
register-to-root-SO parity. Expected local SO bits translate into the root SO
positions for the same registers using local suffix XOR root suffix; all other
expected bits are X. Never treat expected data as live hardware.

Assemble one canonical root operation, then use the existing internal physical
replay path without recursive child dispatch. One native root capture/bulk-shift/
update/idle transaction executes the assembled payload and updates fill/state only on success. Retain explicit-reset/inactive
reset and all native phase checks. Root requests preserve their existing output;
child requests emit one full root iScan. A root -stable is emitted only when all
queued requests supply -stable; modeled topology remains statically known.
No persistent expected comparisons. No automatic opaque payload reuse claim.

Keep ApplyScan(Operation) as the existing single-request API delegating to new
ApplyScans(vector<Operation>); Generate uses the latter for the per-interface queue.
No new dependency or separate state model. Opaque root-only lowering is unchanged.
Resource ownership, arbitrary topology, TAP/IR, parallel chains and opaque child
networks remain separately recorded acceptance requirements.

Verification: whole-chain child and interior subset surrounded by unequal-width
registers; repeated child instances queued together; same-target overwrite;
parent/child overlap atomic rejection; wrong target/length/typed inputs rejected
before discard; child iProc/prefix resolution; source/flat/generated/unchanged
local-reset replay. Independent literal native shifts check actual state/pins,
input/bridge/output inversion (including odd local-vs-root parity), expected SO
mask positions, partial-X fill and rollback. A source-matched Tessent comparison
and representative wide/many/deep measurements precede immutable final gates.

Independent design/builder review accepts parity conversion and bounded reuse.
Its required refinements are interval-only cached metadata, expansion at Apply,
overlap checks before mutation with named conflicts, one existing physical replay
path, and exact asymmetric native/CLI SI/SO assertions. These are included above.
