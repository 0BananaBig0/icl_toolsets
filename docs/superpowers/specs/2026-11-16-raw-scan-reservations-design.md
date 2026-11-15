# Resource reservations around modeled raw scans

Task103 follows frozen Task102. Permit existing iTake/iRelease/iState operations
around modeled iScan, protect established values through raw CSU replay, and
refresh protected values explicitly assigned by the author. Reuse ownership/call
scope, release, persistent and explicit-reset behavior already implemented. No
opaque/IR/TAP/parallel/merge, closed-child preparation, port/clock ownership or
reset/clock qualification widening.

## Primary evidence and scope

IEEE1687-2014 §7.9.19 printed178–179/PDF192–193 was inspected. In a single stream,
an unwritten take remains free for retargeting; iWrite establishes protection.
Rule c also mentions iScan in the separate merged-call ownership context. This
increment does not enable raw iMerge or invent single-stream protection merely
because an unwritten take was followed by iScan. Preserve the project's existing
controlling-iState convention; expected-only state does not establish control.
An author's explicit raw assignment may update an already protected value, while
automatic surrounding fill must preserve it. Protected unknown assignments reject
under the existing concrete-known-value convention, without inferring from SO.

## Approach

Removing the whitelist alone misses preserved-value checks and leaves ownership
history stale after an explicit raw update. A second reservation engine or callback
transaction layer is unnecessary. Reuse ResourceReservations and CheckPreserved,
with a compact pre-CSU explicit-register footprint returned on successful replay.
Normal generated PDL already consumes iTake/iRelease and encodes chosen values;
local-reset output preserves source procedures. Keep those delivery semantics.

Extend public ApplyScan/ApplyScans with optional known preserved map and optional
std::set<size_t>* assigned output, defaulting to existing behavior. Build explicit
IDs from each latest target's pre-route segment (all active registers for a root
target); validate overlaps/latest/setup before publication. Skip footprint work
when neither preservation nor the output pointer needs it. IDs are collected
before state/route changes and published only after successful replay; failures
leave caller output and scheduler hardware/fill unchanged. Normalization may cache
valid pre-state geometry, but a failed trial publishes no post-state cache.
Collect the footprint from the route revalidated at Apply, since controlling
iState between setup and Apply can change the selected route. Validate IDs,
widths and masks before replay. Established non-register preservation entries
reject explicitly; unwritten instance-owned ports do not introduce constraints.

After physical SI decoding, derive desired preserved masks: keep old values for
outside-target storage, and use actual parity-correct load bits for explicitly
addressed storage. Reject X on an established protected bit. Use CheckPreserved
on the trial state at capture/complete shift/update/idle, allowing only the old
or authored desired value. All checks precede adoption. Existing full-map native
eligibility, resets, stable proof and live-cache lifetime remain intact.

Add ResourceReservations::AssignedScans(ids,scheduler), refreshing only previously
established protected bits. Unwritten takes remain unwritten. Share the existing
known-value recording loop with ordinary assignments using x for untouched mask
positions, avoiding another full-width temporary mask for raw assignments. Main
requests the footprint only when Values is nonempty, applies the raw batch with
those values, refreshes reservations, and renders output after success. Existing
unknown-assignment diagnostics and automatic take/release on call return persist.

## Verification and performance

Maintained RED before product edits: whitelist rejection of take/release/state
with modeled raw scans. Native pre-state/phase/footprint rollback tests; CLI known
write/take/raw reassignment then unrelated access; automatic surrounding protected
bits; single-stream unwritten take stays free; releases/reset/call scope/persistent
behavior; ascending/inverted aliases and partial takes; X on protected vs untaken
bits; malformed discarded setup and unknown SO cannot establish known control;
controlling versus expected-only iState and pending operations. Every supported
case keeps source/flat/generated/local-reset replay and previous output on failure.

Target O(modeled storage + active payload + protected widths + explicit registers)
per CSU, with no per-bit subprocess or repeated full route walk per setup. Existing
1M active/queued bits,4096 registers,65536 graph vertices/16M traversal bounds stay.
Measure1M protected scan,1024 protected child requests/shared gates, partial takes
and no-reservation baseline. Retain all60 existing offline certificates and native,
resource/data-resource/merge/call-scope suites, style/leak/fullserial/remote checks.
Genuine vendor probes retain limits; no checker changes. New fixture module names
unique; new test scripts under tests/. No dependencies or RTL edits.

User authorized continued inline work without confirmation; executor selects this
bounded single-stream reuse. Broader merged/raw ownership and lifecycle acceptance
remain open rather than waived.
