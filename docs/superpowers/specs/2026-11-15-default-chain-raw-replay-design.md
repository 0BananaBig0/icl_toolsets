# Tessent default-chain raw replay and strict comparison

Task99 follows frozen Task98. Fresh Tessent2026.2 exports repeatedly use
`iScan client -chain default N ...` for a single unnamed client. Current shared
NormalizeScan rejects every -chain, preventing local replay. IEEE1687§7.9.12(b)
defines named subchain selection for parallel interfaces; recognizing this
synthetic default on an already proven single unnamed non-TAP client is a
specifically documented vendor compatibility extension, not a general chain model.

Reuse pattern_opaque.cc NormalizeScan with an optional exact -chain default
length cursor. Both product callers already establish the single unnamed context:
OpaqueClient rejects hierarchy/TAP/parallel/chains, and RawChain/RawSubchain reject
TAP/parallel/named chains before normalization. Other names, malformed selection,
IR/TAP and actual parallel networks retain rejection. Canonical output omits the
synthetic spelling. Preserve length/data validation, zero/X padding, expected SO,
-stable, validation before overwritten/discarded assignments and state atomicity.
No new state, topology, dependency or public scheduler API.

The maintained pdl_pattern_trace ApplyGroups tool currently compares raw iScan
words literally, so harmless chain spelling, numeric format and uppercase X also
block strict comparison. For eligible explicit root non-TAP single unnamed
clients, use the same pure NormalizeScan helper to canonicalize only iScan words.
Retain legacy unsupported-context trace events verbatim; do not change their
meaning or imply hardware validation. Preserve every SI/SO bit, stable flag,
target, wait, reset, order and Apply boundary. Do not execute source Tcl or run
retargeting inside the comparison tool. Keep its aggregate262144-bit expansion budget, charging both payload widths
and the shared1Mbit normalization bound. Existing53goldens contain no iScan; their
resolved-bit comparison policy and traces remain unchanged.

Performance: product parsing adds constant option-prefix work to existing O(B)
normalization. Comparison handles small certification fixtures, charges both SI and SO widths
to its existing aggregate budget and produces bounded canonical strings; no per-bit
simulation or extra subprocess is introduced. pdl_pattern_trace already links pattern_values; add the existing pattern_opaque
object explicitly because that helper is in SOLVER_OBJECTS, not PATTERN_OBJECTS.

Attempt a genuine strict reverse certificate: original PDL -> Tessent export ->
local generation, using a small uniquely named modeled-root fixture without
-stable. Tessent ignores stable and may add capture reads during writes; those
remain meaningful, so do not project them away or force a forward certificate.
Reverse replay must retain all explicit vendor reads/writes and raw operations.
Use existing tessent_roundtrip prepare/run/record/check and source/artifact hashes;
add a golden only after the unmodified strict checks pass. Failed or unavailable
directions remain explicitly recorded. Add the case to the existing golden
manifest only with genuine qualifying evidence, without policy weakening.

Verification: opaque/root/child default syntax, canonical output and full replay;
source/flat/generated/local-reset equivalence; other names/malformed/IR/parallel,
wrong length and malformed discarded assignments atomic rejection. Trace default/
plain/numeric/X equivalence while changed SI, SO, stable, target, waits and Apply
boundaries remain distinguishable. Raw expansion budget and legacy trace behavior
remain covered. TDD RED, independent review, required leak/fullserial/platform
checks and prompt merge/deletion follow the existing workflow.

Independent design review approves subject to eligibility-before-normalization,
no fallback after malformed eligible input, unchanged raw event/order/stable
semantics and accounting for both SI/SO. Ruling: the existing aggregate budget
limits total expansion/output, while one normalization may allocate up to the
existing1Mbit shared bound before charging; this matches existing per-target
resolution behavior and avoids duplicating length/data parsing. It is not a
peak-memory guarantee. All normalized widths are charged before emission.
