# Ordinary preparation before a modeled root raw scan

Task104 follows frozen Task103 and the existing approved pattern-completion goal.
Support an ordinary iWrite/iRead queue followed by a root-client iScan in the same
iApply. The ordinary accesses execute first; the raw CSU uses the route and held
state resulting from those accesses. This enables explicit SIB preparation without
requiring a separate source iApply. Keep the existing scheduler, reservations,
reset rewriting, native replay and one-million-bit limits.

## Evidence and approaches

Tessent Shell Reference v2023.4 iScan, Command_Iscan_idb66e0799.html, explicitly
states that raw scanning follows other queued commands, and that those commands
may configure its route. IEEE1687-2014 §7.9.12 defines setup state and checks scan
length at execution. Current Generate rejects any mixed queue, while NormalizeScan
checks current geometry immediately. Existing ordinary Apply already prepares
SIBs and returns concrete validated steps.

Only removing Main guards would reject valid future lengths or accept unchecked
setups. Solving a scheduler copy at each setup would repeat expensive planning.
Choose a one-time immutable raw qualification helper, with deferred root length
only for an already pending ordinary queue; execute the ordinary queue once at
Apply, then use the existing exact raw replay. No new solver or state engine.
The user has authorized continued inline execution; these are executor choices.

## Bounded delivery

For this increment ordinary accesses must precede the raw setup in source order.
Adding a read/write after any pending raw setup remains an explicit rejection.
A mixed queue targets the qualified root client only. Accessible child-only queues
retain their previous behavior; automatic closed-child raw preparation, opaque,
TAP/IR/parallel, raw merges and clock/reset/port ownership do not expand. Reject
mixed iApply -together: the ordinary preparation and raw update occupy separate
CSUs and cannot satisfy the project's one-update-cycle contract.

Factor private const RawProfile& RawTemplate() from RawChain's immutable topology,
port, reset and control qualification. Static templates retain their existing
state-independent ordered route. Dynamic templates store no trial route. RawChain
uses the template and existing invalidated current-profile cache for live geometry.
Copies and failures retain the existing cold/pre-state cache rules.

Extend NormalizeScan(const Operation&, bool defer_root_length=false). The default
is unchanged. Deferred normalization proves qualified root identity and validates
syntax, options, default-chain spelling, length bounds and SI/SO payloads, while
checking actual route length only at execution. Do not normalize against a future
state by mutating or solving the live scheduler. Numeric/operand/immutable-qualification errors
still reject overwritten or reset-discarded requests. A future route's exact
length is intentionally deferred until Apply; reset may cancel that geometry
check together with the valid queue. Constructor graph validation and immutable
qualification remain early; current selected connectivity/knownness, reset
readiness and exact geometry are Apply-time for deferred setups. Native raw
failure retains a preparation already applied through the separate ordinary API;
combined rollback here means generator/publication atomicity.

When Main sees iScan with a nonempty ordinary queue, request deferred root setup.
At mixed Apply, reject -together, perform existing local-reset checks, apply the
ordinary operations with current reservations, refresh assigned ordinary values
and append their steps. Clear the ordinary queue. Then apply the raw queue with
current reservations, refresh its explicit footprint and append the raw CSU.
Each preparation step remains visible in generated PDL. The generator's existing
local scheduler/reservation copies and staged artifacts preserve publication
atomicity for pattern/local-reset/debug ICL outputs if any preparation or final
raw check fails. Diagnostic --log intentionally records the failure as before. Source local-reset rewriting
retains definitions, source ordering and grouping; introduce no internal rewrite.

## Verification and scale

Maintained RED before product edits: write open=1 then raw root9 in one Apply;
compare with explicit two-Apply reference. Cover static read/write preparation,
reserved selector explicit override versus automatic conflict, expected SO after
preparation, generated/flat/local-reset replay, queue overwrite/cancellation and
malformed discarded setups, deferred wrong length and wrong stable rollback,
pending controlling iState, source calls/scopes and mixed -together rejection.
Root-only/order boundaries need explicit negative tests. Native strict/deferred
normalization and cache/copy/invalidation checks retain every old raw case.

Measure one million active bits without repeated planning,1024 source preparations
and repeated apply groups using equivalent explicit-boundary references. Preserve
Task103's linear whole-target reader and all current61 offline certificates.
Connected ownership/reset/call suites, style/audit, independent final review, leak/
fullserial/remote and honest source-matched vendor checks precede integration.
Task103 Root snapshot stays immutable during its final checks; no local heavy
compile/tests while its timed gates run. Use the remote /tmp candidate only after
its Task103 archive has completed. New scripts live under tests/ and mirror source.
