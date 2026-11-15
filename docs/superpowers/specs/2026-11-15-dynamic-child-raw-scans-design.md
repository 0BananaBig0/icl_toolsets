# Raw scans inside an accessible binary child network

Task102 follows frozen Task101. Generalize child retargeting to a currently known
binary ScanMux/SIB route inside the child, and scope a child stable claim to that
child. Preserve the root's stronger ordered-route/phase proof when the user names
the root. No preparatory opening of an inaccessible ancestor, TAP/IR, parallel,
arbitrary logic, serial-valued selector or new reset/clock semantics.

## Evidence and approaches

IEEE1687-2014 §7.9.12 printed168–170 (PDF182–184), inspected in the repository's
primary-standard text/PDF: length is the addressed ScanInterface's current chain
length; -stable promises repeatability without changing that black-box serial
network's length. SI is controlling data and SO is comparison data. This modeled
implementation deliberately proves ordered route/parity and native eligibility,
a stronger bounded criterion than length alone; it does not reinterpret a local
claim as a promise about unrelated ancestor/sibling geometry.

Reuse the qualified current root profile and native CSU. Record chosen mux edges
while deriving that profile and let the child walker follow those same edges.
A new solver or independent signal evaluator adds no value for an already known
route. Simply removing the child-mux guard would lose local stability validation;
retaining Task101's whole-root OR proof would reject legitimate local claims when
another route changes. Use explicit local segments plus physical flag projection.

## Data and execution

Add selected mux choices to RawProfile, populated only for the current ordered
root path. Cache a RawSegment per child/current generation: existing small RawSlice
plus first/count indices into the supplied ordered root profile and total local
SI/SO parity. Do not duplicate per-child register vectors: local register order is
the root subrange, and each local suffix parity is rootSuffix XOR slice.output_inverted.
This exact view avoids quadratic cached metadata in overlapping/deep interfaces.
RawSubchain still returns only RawSlice by value. The child walker follows root
chosen edges and requires its complete selected interval in the root; missing
choices or noncontiguous/inaccessible paths reject. Static child behavior remains.

Factor the existing child proof into RawSegmentFor(client, rootProfile, work*),
which reads immutable topology and the supplied profile, returns a segment, and
charges an explicit16M traversal budget. Live setup caches its result. Post-trial
proof uses a local counter and never changes live profiles, slices or counters.
Keep all existing mutation/copy invalidation boundaries and1M active/queued bit,
4096-register and65536 graph-vertex bounds.

ApplyScans collects latest stable client IDs rather than collapsing proof scope
into one boolean. Replay freezes the pre-CSU root route, samples old native update
eligibility, and loads exactly those registers as before. After trial update/idle:
- If the root was claimed stable, require the same root ordered path/parity.
- For each stable child, require the same local ordered path/parity and a complete
  accessible interval in the known post-root route. Width alone is insufficient.
- Prove native phase eligibility for the entire post-root active set on a separate
  state copy, including zero events for omitted storage. Reset rules are unchanged.

The physical output may retain -stable only if every latest input claimed it AND
the whole physical root route remains stable. Remove the projected physical flag
when only local claims hold and the root changes. Keep the root's original scan
length and SI/SO bits for that completed CSU; subsequent setup derives the new
root profile. Mixed flags stay nonstable in physical output. All checks precede
state/fill adoption and publication; failure/retry leaves live data, shifts, pins,
events, controlling fill and caches valid for the original state.

## Verification

Nested outer/inner SIB: open ancestor, child1bit opens its leaf, child9bit loads
leaf/stays open, child9bit closes, child1bit reopens retaining leaf data. Source,
flat, generated and local-reset forms; native literal per-bit reference; partialX,
SO/parity and current length after ordinary mutation/copy. Stable child path change
rejects even when another sibling compensates and total root length stays equal.
A static child may stably change an unrelated sibling route if it remains
accessible/eligible: physical root -stable is removed, and next setup uses the new
width. Root stable claims still reject root geometry changes. Test unknown route,
closed ancestor, overlap, latest/discarded setup and failed stable rollback.

Measure1M active bits,1024 queued dynamic children/shared gates, nested route
changes and repeated open/close. Target topology plus active payload/segment work,
not a root scan per setup or per-item subprocesses. Reuse58 existing offline
certificates and all connected/style/leak/fullserial/remote gates. Vendor probes
must retain genuine internal-client limits; no checker exceptions. Unique fixture
module names and new test scripts beneath tests/. No dependency or RTL changes.

User authorized continued inline work without confirmation. Executor selects this
bounded reuse design; closed-child preparation and broader standard/release/strict
coverage acceptance remain open, without waiver.

Exact stable comparison uses first/count against pre/post root profiles: count,
register IDs, local suffix parity (rootSuffix XOR slice.output_inverted), and total
local SI/SO parity must agree. Pre/post profiles remain alive throughout proof;
segments store indices, never references. Storage is O(root route + client count),
with O(total claimed segment registers) comparison work. Root flag removal occurs
on the returned operation before adoption/publication, never by editing source PDL.

Independent builder confirms buildability and parity-span math. Both native control
fallback selection and loaded/omitted checks take the correct pre/post active set.
Post proof never calls cached live accessors; one local work counter covers all
claims. Wholly nonstable accesses keep existing behavior even if they leave an
unknown post route. Only project output flags after successful scoped proofs.
