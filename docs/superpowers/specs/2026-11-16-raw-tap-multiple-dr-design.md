# Ordered multi-register raw TAP DR

Task111 extends qualified root TAP raw data scans from one full register to a
complete known serial path containing multiple modeled ScanRegisters, including
internal serial alias/inversion. Keep one bound scalar root client-TAP and known
binary SI. No implicit instruction/child retargeting, opaque/parallel/multi-TAP
scan, raw merge or new clock/reset/FSM/solver engine. Task110 frozen17 stayed
immutable through its completed gates; this is the Task111 implementation scope.

## Chosen approach

Factor the existing concrete graph traversal used while decoding ordinary scan
steps and the existing ActivePath traversal into a shared ordered-path result.
It records the physical input bit, ordered full registers from SO toward SI,
each register's output-prefix inversion, and total inversion. Reuse existing
Match and graph edges; do not add a third walker. Preserve legacy callers'
selection/enable behavior explicitly instead of silently broadening them. The
raw path requires known unique selection, enables, the bound scalar TDI/TDO,
full register boundaries, no repeated/overlapping storage and positive bounded
total width. Inspect actual decode function before selecting helper signatures.

Read the current data route in a copied NetworkState with the existing synthetic
ir_select forced0, without executing a phase. Root binding remains indexed and
constructor-cached as before. Shared operand normalization stays unchanged;
source current geometry/readiness is deferred to Apply, strict native default
checks immediately. Ordinary preparation may make the route current first.

## Packet mapping and exact proof

A printed numeric packet is MSB-left; its suffix belongs to the register nearest
SO. For the SO-to-SI register list, subtract each width from a descending offset.
SI piece inversion is total parity XOR that register's output-prefix parity;
SO piece inversion is the output-prefix parity. Use the existing InvertBits
helper so X observer masks retain X. Known SI remains required. Ascending index
ranges use the existing whole-register packed value order, not ad hoc index math.

Build one full logical iWrite/iRead for each pre-CSU path register, execute the
existing ordinary planner on a copied scheduler, and require exactly one data
CSU with the exact bound endpoints, complete load/write/expected maps and raw
physical reversed SI/SO streams. No extra IR/setup/primary access may pass. A
single builtin keeps Task110's exact-ID private allowance; do not widen it to a
set or expose public synthetic targets unless a real supported mixed-builtin
route exists. Unsupported builtin combinations reject before adoption.

Adopt full trial state only after every proof, and publish the set of precisely
those pre-CSU authored registers. SO never controls SI fill. Owner refresh and
partial writes reflect actual parity-adjusted stored values. -stable compares
known complete supported post-route total length with pre-length; route identity
and parity may change if still representable. Unknown/disconnected/alternate
physical-input post-routes reject. An inverted post-input becomes supported by
this task, so replace that old negative with a retained-state positive and keep
constant/alternate negatives. Nonstable topology changes may change later length.

## Verification and performance

Native literal TMS100, N-1 zeros,110 gives N+5 clocks. Use unequal3/5-bit registers,
nonpalindromic SI96, explicit self capture, all endpoint/state checks and SI,
inter-register and SO inversion combinations. Add ascending indices, alias chains,
owner/partial history, pre-CSU assigned sets, stable geometry changes, rollback/
retry, and ordinary preparation in both source orders. Preserve IR and builtin
public-target restrictions. Source/flat/generated/reset replay must agree.

Compare equivalent ordinary/manual programs with matching flags at3/16/64
registers and up to256 active bits (separate geometries),1024 unrelated model registers,128 repeats
and representative forwarding aliases. Record inherited larger-input resource
rejection; no raised limits or unmeasured1M TAP throughput promise. Avoid per-bit
full scans or per-register solver trials; traverse once, assemble maps, plan once.
Genuine strict vendor evidence, independent review and fresh full/SAN/remote
gates precede commit/ff/delete. All new scripts under tests/. No dependencies/RTL.

Builder check should specifically settle ordered-path extraction boundaries,
legacy enable checks, builtin private allowance, and post-route parity support.
Executor selects this reuse under continuing inline/confirmation opt-out.

Measured qualification geometries separate register count from total width:
64 one-bit registers and16 sixteen-bit registers complete. The combined64
four-bit-register case exceeded120 seconds in the prior ordinary reference
and15 seconds in a candidate probe. This inherited symbolic-event construction
limit remains open; neither timeout is a successful resource rejection. The
4096-bit case retains its explicit existing event-work guard. No new solver
engine or larger budgets are introduced.
