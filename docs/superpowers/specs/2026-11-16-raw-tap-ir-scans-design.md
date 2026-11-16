# Raw instruction scans through one bound TAP

Task108 adds iScan -ir for a qualified root client-TAP backed by one bound BSDL
TAP. Reuse the existing ordinary instruction planner, TapExecution and instruction
register model. Initially require concrete0/1 SI; SO may contain x. Source raw
IR is queued and executes after ordinary preparation, with no added DR access.
Unknown-SI raw IR, raw DR/TAP, multi-TAP/parallel/child-TAP and raw merge remain
explicitly unsupported. Existing ordinary IR capabilities and limits stay intact.

Qualify an explicit scalar root client-TAP or its unambiguous inferred unnamed
interface against the sole TapRuntimeModel's direct physical SI/SO/TMS/TCK pins;
optional listed TRST must match its bound pin. No chains or inverted interface
references. Fixed IR width comes from the bound BSDL/controller register, never
from DR instruction selection. Reuse constructor EffectiveScanInterfaces for
implicit binding; do not rescan the complete model per request.

Normalize a leading -ir by stripping it for the shared existing single-chain
operand/default-chain/padding parser, then prepend it to the canonical result.
Keep canonical operations valid PDL with leading -ir. Source queue key/payload
accounting therefore uses target index1 and payload index4 for IR, versus0/3
for existing data scans. Malformed operands/metadata and unknown SI reject early;
current readiness and exact width reject at Apply, allowing reset cancellation.
Default strict native normalization checks width immediately.

Build logical full-IR iWrite/iRead operations from normalized SI/SO, and execute
on a copied ScanScheduler with the existing ordinary Apply. Before adopting the
trial, require exactly one instruction ScanStep, one lane of the exact width,
only the bound IR in loads/writes/expected, exact numeric SI/SO and corresponding
reversed physical serial streams. Reject plans needing additional accesses or
setup. Swap the complete trial implementation only after all checks and output
construction. Publish the explicitly assigned IR ID only on success. This reuses
all existing clock/reset/reservation proofs and cannot silently add an IR/DR CSU.

Render the canonical raw iScan -ir rather than its internal ordinary proof. Raw
state/fill and owner refresh reflect authoritative known SI; expected SO remains
observation metadata. Mixed ordinary-before-raw ordering, last same-interface
request, reset cancellation, atomic pattern/reset/debug publication and diagnostic
logs retain their contracts. Distinct interfaces sharing the IR reject overlap;
IR/data raw mixtures are unsupported. Stable refers to the fixed IR chain, not DR
length. Do not invent unknown reset opcode bits. Update apply-group trace
normalization for leading -ir/default-chain/numeric spelling when bound context
is supplied, reusing the shared parser.

IEEE1687-2014 §7.9.12 and IEEE1149.1-2001(R2008) IR behavior apply. A single IR
from Run-Test/Idle returns to Idle with N+6 clocks; TMS1100, N-1 zeros,110 includes
the final shift on the exit edge. Numeric instruction fields shift LSB first.
Independent literal TapExecution pin-event tests must check all endpoints/state,
expected capture masks and final idle behavior against the reused backend.

Assume real BSDL instruction widths2–32 bits; characterize4/32/256/4096 and larger
instrument models with matching command options, time/RSS/input/binary/source
hashes and explicit resource rejections. The existing symbolic backend limits
remain; do not claim1M IR throughput or raise budgets. Upgrade trigger is required
width/model size beyond measured successful range. No new controller/evaluator,
clock/reset semantics, dependencies or RTL. New scripts under tests/ mirror source.

Native/CLI RED before product, source/flat/generated/local-reset replay, owner/
queue/call/malformed/cancellation and scope regressions, genuine strict vendor
qualification, independent review and focused/full/sanitizer/remote verification
precede commit/ff/delete. Task107 frozen22 stays immutable through its gates.
Executor selects transaction reuse plus exact one-CSU proof under the user's
continuing inline authorization/confirmation opt-out. A new TAP replay engine
would duplicate existing tested behavior and is unnecessary for this first scope.

Preflight: existing single-lane ScanStep uses outer input/output/serial fields
and an empty lanes vector; populated lanes represents multiple lanes. Exact IR
proof must require that established representation, not vector size1. Observed
ordinary reference: one instruction step, TDI/TDO, serial1100, expected10xx,
one IR load/write/read. Canonical IR stays leading-ir; no runtime trailing-flag
variant or new public command is added.
