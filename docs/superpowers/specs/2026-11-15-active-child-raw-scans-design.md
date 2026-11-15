# Child raw scans within the current gated route

Task101 follows the frozen Task100 snapshot. Continue raw retargeting by allowing
an unnamed static child client interval within the currently known binary SIB
root route. Reuse the existing child interval/parity proof and physical root CSU
composition. A closed/incomplete child remains an atomic error; opening it with
preparatory accesses is separate remaining work. Preserve the original source
scope and model, including all data, comparisons, stable claims and apply order.

## Approaches

Reusing the existing interval proof against the current root profile is the
smallest complete approach for an already accessible static child. A new solver
would add preparatory scans and wide-vector cost to a route already known. Merely
removing the dynamic guard would reuse stale cached offsets and repeat the root
route scan for every queued child. Use explicit current-route cache lifetime and
invalidate it at every scheduler mutation boundary instead.

## Behavior and structure

Root qualification and native phase/stable proofs remain Task100's implementation.
RawSubchain returns a small RawSlice by value. For a dynamic child, prove its static
local register sequence is a contiguous interval of the current ordered root
route; compute fresh root offset and separate SI/SO parities. Cache those slices
only for that current state generation. A child with its own mux or incomplete
local interval remains rejected. Existing static-child behavior/cache is preserved.

RawChain reuses the current dynamic ordered profile while scheduler state is
unchanged. Invalidate dynamic profile/slices at entry to Reset, Idle, Assume and
ordinary Apply (including failure, which may discard caches but cannot change
hardware state), and after successful raw replay adoption. ApplyScan/ApplyScans
recompute once for their setup queue and freeze the route throughout replay. No
mutable state is exposed publicly. Verify copy construction and any internal
mutation helpers before relying on these boundaries; caches never establish
hardware truth. ResetPrerequisites/CheckLocalResets/Value must remain read-only.

Queued disjoint accessible children compose into one physical root CSU using
existing controlling fill for surrounding active storage. Latest target wins;
malformed discarded setup still rejects. Ancestor/descendant overlap rejects.
No implicit child access when outside the route, no duplicate serial events,
no pin/phase/reset widening and no changes to native ShiftComplete semantics.
Any latest stable child request requires the entire physical root route to remain
stable in this bounded increment. Track that proof obligation separately (OR of
latest flags); the emitted physical -stable flag remains the conservative AND of
latest flags. Mixed stable/nonstable siblings with unchanged root route are legal.
Broader local-only stability while another sibling changes the physical root
route remains required future work, rather than being silently misrepresented. Failed interval/phase/stable checks preserve state, pins, shifts and fill.

## Performance and verification

Preserve1M active/queued bit bounds,4096-register/current graph bounds and16M
interval proof work per dynamic state generation. Work for1024 queued one-register
children must be one root derivation plus their local interval proofs; no root
rescan per setup assignment. Test shifted offsets after another SIB changes the
route, stale cache after ordinary Apply/Assume/Reset/Idle, copies, same-width route
switches, source/flat/generated/local-reset forms and inversion parity. Native
reference compares root physical replay with child retargeting, including partial
X surrounding fill, child expected SO and preserved siblings. Test inaccessible
child, child mux, overlap, malformed discarded setup and failure/retry atomicity.
Measure1M child input,1024 disjoint queued children/shared selectors, deep child
and repeated route changes. Source-matched vendor probes retain unsupported
internal-client limitations and all strict certificate policy. Existing57 offline
certificates and all required style/native/leak/fullserial/remote gates precede
merge. No dependency, RTL or public API change.

The user has authorized continued implementation and opted out of confirmation
questions. The executor chooses the bounded current-route approach; broader
closed-child preparation and opaque/IR/TAP/parallel tasks remain required work.

Builder review: mutation/copy audit confirms the proposed boundaries. Copies and
speculative Impl instances rebuild qualification and begin with cold raw caches.
Clear only state-dependent profile/slices/work; immutable qualification and static
caches persist. Normalization and ApplyScans queue validation never invalidate.
Post-trial proof derives directly from trial state and cannot poison live caches.
All returned profile references must finish use before successful-adoption reset.
