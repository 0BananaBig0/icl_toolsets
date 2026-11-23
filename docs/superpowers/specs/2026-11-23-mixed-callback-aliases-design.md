# Mixed ordinary and callback alias bits
Task162 continues original remaining items4/6 from pushed master9fc1e55.
Task161 is complete:59 gates,15 normal/15 leak-SAN native,71x1153 methods
per generator,30 quality checks,4033 inputs/38 products,156 measured/example
processes,22 merged checks,117 preserved stashes. Its ledger is archived.
This is the next bounded increment; other pending inventory work continues.

## Intent and decisions
Allow one legal alias iWrite/iRead target to contain ordinary bits and explicit
DataRegister callback bits. Preserve per-bit inversion/ranges/coalescing, ordinary
native requests and callback arguments/order. Avoid dropping ordinary bits merely
because the operation is marked as a callback request.

The user delegated implementation choices and opted out of questions in the
existing completion program. The choices below are the executor's, not specific
user design approval. Use current C++17/Tcl/Z3/Python/Make and the existing native
scheduler/readback/source-action paths; no new dependency or hardware engine.

Eight parent probes are in build/mixed-callback-aliases-20261123/discovery/.
Full/inverted/partial mixed writes and mixed read-callback generation hit the
exact mixed-alias guard. Separate requests, ordinary-only aliases, callback-only
aliases and canceled mixed requests work with actual3/9,9 and0/0 controls.
The immutable parent binary/runtime hashes match master9fc1e55.

## Approaches
1. Clear the callback marker and let the native scheduler handle the whole
   alias: loses explicit callback dispatch and treats abstract storage as hardware.
2. Teach main/readback/merge/history engines mixed-bit callback expansion:
   duplicates partitioning across consumers, complicates source/history identity.
3. Keep the original immutable logical request, append compact ordinary-only
   operations during command normalization, and reuse Task161's mixed queue
   execution. Choose this: every existing consumer already skips logical markers
   and processes ordinary operations, while existing SourceAction spans can own
   all entries emitted by one command.

Builder check: one fresh read-only planning check after this design is written;
settle project-answerable/minor questions here, with no implementer subagents.
One fresh whole-branch final review and one test-first Critical/Important pass
remain mandatory. Defer Minors/no rereview under the existing workflow.

## Scope
Support direction-specific mixed aliases in explicit iWrite and iRead, including
nested/inverted aliases, sparse/reversed/ascending ranges, repeated references,
segmented ordinary ports, named enums, child prefixes/ancestor callback bindings,
ordinary DataIn/DataOut/ScanRegister/DataRegister members already supported by
native access, and source/flat/generated/local-reset caller replay.

An iWrite considers WriteCallBack members; an iRead considers ReadCallBack
members. Other registers retain their existing native access rules, never invoke
an opposite-direction callback. Ordinary-only and callback-only targets must
retain their old stream shape/behavior.

Keep the existing AccessTogether/-together/raw/active-iMerge/automatic-downstream/
physical callback and invoked-ReadCallBack modeled-history guards. Mixed write
callbacks may use actual ordinary observations via --model_readback; logical
callback storage remains unknown. ReadCallBack generation is supported without
inventing an actual callback read history. Canceled uninvoked mixed reads keep
their prior ordinary histories through reset.

## Runtime partition and stream contract
Use a private Runtime::Impl helper, provisionally:
std::vector<Operation> CallbackOrdinaryAccesses(
    const Operation&, const PdlTarget&, const std::string&) const.
No new public Runtime/Operation/Readback field or signature is required.

After resolving the full target and PdlDataValue exactly once, collect only bits
without that direction's callback. Normalize each bit's inversion into data.
Use original (node,index) identity and last occurrence within the command for
duplicate references, matching QueuedAccesses. Coalesce selected consecutive
indices per node into compact canonical ranges in declaration direction, using
NetlistNode::BaseName for segmented/vector names. Never extend to unselected
bits with X, especially for reads: doing so would overwrite unrelated history.
Do not allocate a full node-width mask per selected bit or emit a command per
bit when selected bits form one physical run. Scalars retain scalar spelling.

Prepare all projected operations and validate them before appending anything.
Each inherits origin/call_stack/callback execution and read-invocation context;
callback=false. Apply CheckCallbackCallerWrite to every projected hardware write
before committing the original logical marker and its projections. Conflicting
ordinary bits introduced by a mixed alias inside another callback must not
bypass Task161's caller protection. Do not revise already emitted entries.

Append the original logical callback marker followed by its ordinary projections
within the same Dispatch/SourceAction span, before changing to another source
site. Pure callback markers need no projection; ordinary-only accesses retain
their original single entry. The pending range starts at the marker, covers all
projections, and is flushed/canceled by existing Apply/reset boundaries.

In ExecuteCallbacks, create Request positions for callback bits only, skipping
ordinary bits instead of rejecting them. Continue preflighting the full original
target/payload and every static binding/arity/cycle before procedure effects.
Reuse QueuedAccesses for bit coalescing, including repeated original/projection
ordinary references; Model takes the last identical/correctly assumed value.
No hardware consumer executes a logical marker, so ordinary hardware runs occur
once. Existing consumed original operation indices prevent callback reexecution
on nested Apply, while genuinely new mixed requests still get cycle checks.

Task161's known ordinary caller masks, full native final proof/rollback, guarded
state assumptions, reset clearing and fixed selected callback batch semantics
remain the authority. Do not coalesce existing per-procedure implicit tail
Applies or add preparation CSUs.

## Histories, failures and source rewriting
Ordinary projections participate in existing InvalidateReadHistory and incremental
ModelReadback exactly as separate ordinary requests. Preserve sparse untouched
bits, actual/miscompare/status/-clear behavior and cancellation before Apply.
A getter inside a write procedure and after outer completion must agree on
committed frames. Callback desired data never becomes observed logical storage.

Read invocation provenance remains sticky for actually invoked ReadCallBack,
including nested calls and resets. Preserve canceled/uninvoked behavior and
generation-only read callbacks. No new observation engine or return convention.

SourceAction begin/end must include original marker plus projections. Existing
callback_action_ends and whole-command repair should then cover expansion.
Test once-only evaluated arguments, split files/namespaces/prefixes, repeated
source sites, caller-only local-reset release with byte-immutable iProc bodies.
If a concrete source/readback caller regression appears, fix the shared caller
test-first and record the interface refinement before implementation.

Invalid constants/ports/payloads, missing procedures/arity/cycles and unsupported
group modes reject through existing diagnostics with no published output/debug/
rewritten artifact replacement. Runtime caught failure/reset/retry and prior
committed frame behavior must retain existing contracts.

## Verification and measured acceptance
All new maintained tests under project-root tests/src/pattern; maintain existing
native Runtime tests in place. Plan two inline tasks: core runtime/native plus
first8 CLI regressions; then remaining8 replay/history/scale cases, benchmark,
docs, one final review/fix pass and whole qualification/integration.

Expected16 new CLI methods ->72 suites/1169 methods per normal/static generator,
4035 maintained inputs/38 products and60 final gates (previous59 plus new SAN
suite). Keep15 normal/15 leak-SAN native, selected SAN callback/history/raw/TAP/
legacy/opaque/model suites, corpus/producer/protocol/Make and source packaging.
Changed C++ format/tidy/C++17 count is actual, not assumed; likely6 if only
runtime.cc/native runtime tests change. Run focused checks before whole suites.

After a fresh full build, reuse prior native pass only for exact unchanged
executable/test-source/runtime-fixture hashes with passing-log provenance.
Every changed native product and whole generators/affected CLI/package run
current code. Do not rerun old source-bound Task161 checkers after products change.

Benchmark normal64/256/1024 callback targets andSAN64; four probe groups,3 repeats,
source/flat/generated, actual spare observations1/2/3/4 from fresh getters.
A mixed alias joins spare[3:0] with N4-bit callback registers; compare parent/
current separately queued controls with identical emitted hardware patterns.
Disclose distinct Tcl command counts and declared storage/request dimensions.
Retain existing per-callback tails:4*(N+2) emitted Applies,4*(N+1) PIO frames,
4 empty outer Applies,4*N endpoint writes/postconditions,8 spare writes.
Read the real output to verify these counts; infer zero scan work only after
rejecting unexpected iScan and validating every emitted access as intended PIO.

Target <=10s/512MiB per mixed group process, retain the1024-target existing mixed
and callback budgets and16K alias256MiB budget. Also measure new mixed-width
4096/16384 aliases (4 ordinary bits, remaining callback bits, one callback)
three normal repeats <=256MiB, compact one-run ordinary projection. Retain
four-bit caller-width qualification; masks retained by Task161 scale with caller
node width times annotated boundaries and wider caller scaling is unmeasured.
Use paired parent controls, no unmeasured speedup/same-command-workload claims.
Preserve old mixed81, primary-callback60, pure-wide-alias6, corpus6 and author
guide3 process evidence with final matching binaries; new grid81+mixedwide6
gives243 measured/example processes if all planned forms remain appropriate.

Write one complete guide example and author-execute source/flat/generated actual
3/9; do not claim an independent reader. Update current README/model/tracker
without closing broad items4/6 or excluded coverage1.

Merge into direct parentmaster only after review and gates. Verify identical
source/product hashes and24 focused root integration checks (old22 plus new
suite normal/static),20 copied native products. Push/delete completed branch,
detach the preserved WT, archive only this ledger and preserve117 stashes.
Then continue the original remaining inventory, not end after this increment.

## Review Focus
- Original markers and ordinary projections must not execute ordinary accesses
  twice or reinvoke callbacks on nested Apply; duplicate alias bits are last-wins.
- Canonical segmented/ascending/reversed/inverted sparse runs must not read or
  write unselected bits or lose per-bit history.
- A mixed request inside an active callback must not bypass caller-goal guards;
  retain Task161 mapped-enable/state/reset rollback fixes.
- Canceled mixed reads must preserve prior ordinary history without marking a
  ReadCallBack invoked; actual invoked reads keep existing model guards.
- Source spans must own all projected runs and callback expansion without
  modifying iProc definitions or reevaluating arguments.
- Native state pruning must retain direct/transitive hardware alias consumers
  and explicit goal/read/end/reserved dependencies; PDL-only aliases must not
  force irrelevant callback state into every frame.

## Measured native-index refinement
The real SAN64 mixed-alias source measures2.7601s/536336KiB, exceeding the unchanged
512MiB cap. PrimaryStateCandidates conservatively retains every callback register
named in any Alias Source, even when that alias exists only for PDL addressing.
That defeats the existing irrelevant-callback-state optimization.

Refine the existing index: root retention in non-alias hardware inputs, reset/
choice/logic/interface fields and TAP metadata; follow any encountered aliases
transitively through their sources. An otherwise unused PDL alias does not root
hardware state. Keep explicit goals/reads/end-state/reservation inclusion in
SolvePrimaryWrites unchanged. Add direct/nested alias hardware-consumer controls
before changing the index, then prove the same failing SAN input/output bytes
within the original cap. No new API/engine or sanitizer-setting change.

Separately queued controls do not need a mixed alias declaration; omit that
unused declaration in both parent/current controls and disclose that source
declaration difference alongside command-count differences. Mixed cases retain
the exact real alias workload. Receipt smoke usesN4 and a64-bit wide fixture on
all backends; final acceptance still requires normal64/256/1024,SAN64 and normal
wide4096/16384. The earlier unit composition incorrectly imposed the normal-only
16K wide cap on every sanitizer role. This refinement does not weaken any full
performance target. Additional scheduler/native quality/product checks follow
actual changed files/binaries, not the earlier likely-six count.
