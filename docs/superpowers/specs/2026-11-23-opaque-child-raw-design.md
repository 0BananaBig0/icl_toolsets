# One opaque child on a modeled scalar scan route

Task160 continues remaining inventory8/6 from pushed masterbf000ae.
Task159 is complete (55gates/17mergedchecks,69x1117eachgenerator,4028inputs/
36products,117stashes); its branch was deleted and ledger archived. The user
delegated choices and opted out of questions; the choices below are ours.
Keep one reviewable branch and preserve unrelated worktrees/stashes.

## Intended result and evidence

Retarget one black-box child's scalar ScanInterface embedded in a complete
static root scan route, including real ScanRegisters before and after it.
Keep parent controlling values, capture/update/reset/ownership behavior,
source/flat/generated/local-reset replay and actual ordinary parent observations.
Only eligible scan-only routes activate this backend. Unrelated primary/data/
TAP or otherwise unsupported networks retain the existing native backend;
unsupported opaque accesses still fail through its existing route checks.
Do not reject previously supported PIO merely because an unused opaque child
exists. Opaque return bits remain unknown; expected -so data and prior -si/default/stable
controlling data must never become observations.

The exact parent rejects tests/cases/ours/pattern_generation/static_child_scans
with its child's ScanRegister removed and an8-bit DefaultLoadValue on local:
raw-only fails complete-static-root validation; parent-first fails disconnected
route. Immutable parent/probes live under build/opaque-child-raw-20261123/
{parent,discovery}/.

IEEE1687-2014 6.4.5,7.3.5,7.9.12 (local /tmp/item11-1687.txt lines4203–4217,
7633–7643,8739–8788) defines port-only black boxes and lets retargeting use the
supplied precise scan length/integrity while keeping private contents hidden.
Every active black box must have an explicit current request, a prior stable
request, or stable reset defaults. A non-stable request cannot justify a later
implicit access. Retargeting must preserve known modeled registers around the
black box and expose no invented private instrumentation.

## Approaches and choice

- A separate mixed opaque/native CSU engine would duplicate the existing
  scheduler's serial mapping, event execution, ownership and rollback.
- A single immutable private expansion can expose the declared shift interval to
  the existing native scheduler, and reuse the opaque queue's default/stable/
  unknown-length bookkeeping. Choose this: native frames remain the authority.
- A general variable-width/multiple opaque/parallel/TAP reconstruction is a
  separate larger change; do not enable it by weakening the current guards.

This increment supports one scalar opaque leaf with a numeric explicitly sized
DefaultLoadValue, whose currently requested width equals that declared width,
on one complete static scalar root client. Allow pure scalar wrappers/ordinary
modeled ScanRegisters/appropriate port aliases; the existing RawTemplate control,
reset, route and budget checks still apply. Multiple opaque leaves, named parallel
opaque children, changed child widths, muxed/conditionally selected child routes,
TAPs, clocks/overrides/timed opaque execution and physical PDL stay unsupported.
Do not claim whole8/6 or coverage1 complete. Root opaque variable-width behavior
and existing modeled-child routes remain intact.

## Private model, privacy and lifetime

Reuse pattern_opaque.h/.cc for an immutable optional OpaqueChildModel: original
node boundary, child/root interface identity/default and a copied Netlist with one
private ScanRegister appended inside the opaque scope. Preserve original IDs.
Wire its ScanInSource to the child's SI and its rightmost bit to the previously
unsourced child SO. CaptureSource is all literal x; actual hidden capture/update
data is never inferred from requested/default data. Reset state stays x.
DefaultLoadValue supplies controlling fill through the native constructor's
existing fill initialization (which already honors that property). Reuse the
child's bound reset/control/clock metadata and existing scalar route validation.

The copied model is owned/shared immutably across scheduler copies; snapshots
must preserve opaque queue state and reset/stable/unknown tracking. The borrowed
source network retains its existing lifetime contract. Export/debug/flat ICL and
Runtime resolve only the original model. Reject native public target operations
that reference private nodes. Strip private loads/writes/expected names from
returned public ScanSteps; no hidden endpoint may appear in generated PDL or
become publicly addressable. Mask the private update/shift state back to x after
each committed CSU without changing its controlling fill; public Value/state and
observation surfaces must not report requested bits as hidden instrument data.
Filter any private IDs from public logical observation vectors as well as step
maps. Validate that the opaque SO is used only along the
serial path, not as an ordinary data/reset/selector source. No DataRegister/
callback/other unmodeled side channel is admitted by this bounded scan-only route.

A child CSU's capture starts with unknown private bits, then serial shifting uses
the length/integrity contract, allowing native execution to preserve surrounding
registers. This is a private shift-interval model, not a model of the instrument's
contents. Observers see x for explicit opaque returns even after default/reset/
stable writes; real parent capture bits still come from native state.

## Queue integration and failure semantics

Reuse OpaqueScanQueue. Factor its existing completion logic so a forced internal
completion can produce the required scalar implicit default/stable payload even
when no explicit child request is pending, while Apply keeps the root queue's
empty-Apply behavior unchanged. Apply the same stable/unknown bookkeeping to
scalar completion; named root behavior remains unchanged.

For nonempty ordinary parent register accesses, require a known opaque completion
and supply its controlling payload as a private write goal for the native CSU.
Do not create an extra CSU for an empty Apply. Limit this backend to ordinary
ScanRegister accesses; do not force a previously event-free PIO access into a CSU.
No new optimized/clock mode is implied. Private write goals are removed from the
public step maps; replay reinjects the identical required controlling value from
the stable/default queue.

For raw requests, reuse native subchain offsets/inversion to project a full-root
request onto the opaque interval, or use the direct child request. Validate all
requests before replacement as today; native overlap/length/payload checks remain.
A raw request covering only modeled registers still requires the opaque default/
stable completion because its CSU traverses that interval. Full-root -stable
also establishes the opaque interval's stable controlling payload; without it,
later implicit traversal requires another explicit request or reset.

Completion is per physical CSU, not per outer PDL Apply. Mixed ordinary/raw
groups retain the existing ordinary preparation followed by raw execution:
every preparation frame uses only the previously established stable/reset-default
payload, while the current explicit child/full-root request is consumed exactly
on its requested final raw CSU. Reject preparation without prior knowledge even
if a new non-stable request is queued in the same group. A prior stable/default
payload can justify multiple preparation frames because native private write
goals constrain it in every frame. Unexpected raw preparatory CSUs must either
enforce that prior payload or reject transactionally; never use the pending
non-stable request twice. Test mixed known-prior success and unknown-prior failure.

Use transaction-local queue copies and commit queue metadata together with native
state/fill. Failure leaves both reusable snapshots intact. After committed native
work, a subscriber exception must leave both state and opaque metadata committed,
as existing observation APIs require. Explicit-only logical/raw history planes,
global failed-bit counts, sparse/inverted data, reservations and reset cancellation
retain their current meanings. Aggregate width stays within the existing1MiBit
limit; no new dependency, CLI option or fabricated clock/transport.

Pure opaque-child hierarchies must reach the native expansion path rather than
the root-only LowerOpaqueScans/ModelReadback classifier. Unsupported hierarchies
must still fail before publication. Public clock/idle/physical operations fail
closed for the new opaque-child backend.

## Acceptance

TDD:
- Queue completion: no reset/default knowledge, reset/stable implicit completion,
  non-stable unknown length, canceled/replaced/invalid requests, cleared implicit
  expected data, scalar/named/root compatibility and payload budgets.
- Native:3-bit head/8-bit opaque/5-bit tail, actual parent capture and x opaque
  returns, parent writes before/after child scans, full-root/direct-child mapping,
  reset/default/stable reuse, missing/changed width, offsets/inversion, copies,
  rollback and subscriber-failure commit, native private-target rejection and
  public step-map privacy.
- CLI: source/flat/generated/reset-source with nonempty parent/child operations,
  namespaces/prefixes/split files, parent-only CSU with defaults/stable, non-stable
  follow-up rejection, actual histories/status/manual planes, reservations and
  atomic retained output/debug/reset files on unsupported interactions.
- Preserve all root opaque and modeled-child behavior, callback budgets and
  scalar/parallel/TAP observations. New maintained scripts under tests/src/pattern/.
  Add a focused tests/src/pattern/pattern_opaque_test.cc native suite, wired into
  normal/COV check targets, to test this subsystem without rerunning the large
  general scheduler suite for every focused iteration.

Measure four outer groups at opaque widths8/64/256/1024, parent3+5, three repeats
source/flat/generated normal; sanitizer8/64. Each run<=10s/512MiB. Generated
measurement must execute fresh getters at the four group boundaries (not merely
replay literal notes): use an explicit full-root probe CSU per group and verify
its returned parent bits plus x opaque interval with the same expected canonical
output. Match a visible native shift-interval control and root-only opaque
lowering control where semantically comparable; report actual payload/CSU counts,
fresh artifact and tool/input/output hashes, timings/RSS, no unmeasured speedup.
Retain current normal1024callback10s and16Kwide-alias256MiB budgets.

Full15native/15leakSAN (including the new focused opaque suite), both whole generators, selected SAN observation/corpus/
producer/protocol/Make and changedC++ quality/source packaging remain required.
One fresh independent final review; one Critical/Important test-first fix pass,
deferMinors/no rereview. Verify exact merged sources/products and focused checks,
push master/delete task branch/archive only ownledger and continue the inventory.

Builder choices: internal helper names/representation and exact placement of
existing validation calls. Resolve routine code details from current native
source; do not weaken the stated privacy, queue, event, rollback or performance
contracts. No user-specific design approval is claimed.

Builder check /root/opaque_child_raw_builder_check raised one Important question
about per-CSU consumption in mixed groups. The preceding explicit rule settles
it from existing generation/readback behavior; there are no remaining important
questions and no user-specific approval is claimed. The focused native target is
our implementation-economy choice; it adds one suite rather than weakening any
existing verification. Larger variable-width/multiple-child topologies remain
separate inventory work.
