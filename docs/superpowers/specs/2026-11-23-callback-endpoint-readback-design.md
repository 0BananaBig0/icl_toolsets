# Modeled endpoint observations during write callbacks

Task159 continues remaining inventory4/6 from pushed master1fbe93a. The user
delegated routine choices and opted out of questions; these choices are ours.
Task158 fresh54gate baseline matches current source; onlymaster existed before
this new child branch. Preserve117stashes and unrelated detached worktrees.

## Outcome and evidence

Allow the existing model_readback option to observe actual modeled ordinary
endpoint transactions executed by explicit WriteCallBack procedures. Keep
logical callback-register histories unavailable, applied ReadCallBack histories
unsupported, and physical-PDL callbacks unsupported. Retain existing callback-only
caller groups, merge/together/automatic-downstream limits. Do not close whole4/6.

IEEE1687-2014 clause6.4.9, printed117–121 (local /tmp/item11-1687.txt lines
6141–6166 and6388–6417), describes callback registers outside logical ICL
connectivity, accessed via prewritten PDL procedures on actual start/endpoints.
Read and write callbacks are distinct; <D> carries requested write/read data,
not an actual observation. Procedure correctness is its author's responsibility.
Task153 already expands explicit callbacks, preserves request/context flags
across inner resets, emits logical write postconditions and invalidates logical
read histories. ModelReadback and the CLI currently reject every invoked callback.

## Approaches and selected behavior

Removing the rejection alone would let logical iState postconditions derived
from requested callback data contaminate the observation model. Mapping logical
ReadCallBack data/status to procedure-specific observation frames is a separate
larger feature. Choose existing endpoint replay for write-callback bodies plus
typed read-invocation provenance, keeping read callbacks as an explicit boundary.

ModelReadback replays only actual expanded ordinary endpoint operations. It
skips logical callback request markers. Its independent scheduler keeps every
callback DataRegister's abstract storage unknown at construction and after each
reset, even if source declarations contain resets/defaults. Reuse existing
ScanScheduler::Assume on a precomputed callback-node list; inspected implementation
does per-target bits/maps, not a whole-model scan per call. Native goal generation
continues to honor logical postconditions as before, separately from observations.

For controlling iState -LastWrittenValue (or default state assignment), mask
callback-storage target bits to x in the observation scheduler, including aliases
mixing callback and ordinary bits; preserve ordinary bits and manual read/
miscompare history semantics. Apply the same rule to generated-PDL replays,
where source operation flags are absent but native callback descriptors remain.
Never assign actual callback state from <D>, expected iRead data, the generated
postcondition, fill cache, or endpoint similarity. Reads whose actual endpoints
depend on abstract callback storage must remain unknown/reject appropriately;
a matching expected value cannot make them known.

Actual iWrite/iRead/iApply/iState/iScan/iReset/iTake/iRelease/iNote operations
inside supported write bodies use the current model's real frames and shared
ordinary/raw readback machinery. Keep current unsupported clock/timed history
commands unsupported. Preserve sparse histories, inversion, aliases, mismatch
counts, status clear, callback cancellation, nested write calls, inner resets,
caught errors/recovery and reservation scopes where ordinary behavior supports
them. Model-backend error poisoning stays unchanged. No live-device claim.

## Read-callback boundary and provenance

Add a default-false Operation::callback_read_execution flag alongside existing
callback_execution. It means the operation executes within an actually invoked
ReadCallBack context; queued-but-canceled read requests do not set it. Runtime
tracks read-invocation depth while the actual procedure runs, inherits it across
nested ordinary/write calls and inner resets, restores it on exceptions, and
tags directly appended synthetic operations/postconditions and completed callback
Applies. A sticky ReadCallbacksExecuted() accessor distinguishes actually invoked
read callbacks for the CLI finish guard; canceled requests do not set it.

ModelReadback rejects applied read requests or read-execution context before
exposing observation deltas, preserving the existing read-callback diagnostic.
Keep incremental pending logical-read tracking so cancellation by an ordinary
or write-body reset works; a reset executing within an invoked read callback
cannot erase its unsupported provenance. Native manually supplied unexpanded
callback accesses must not be treated as actual native data transactions.
Do not reject ordinary endpoint iRead inside a WriteCallBack: its expected
bits are compared against independently replayed real endpoint data.

The CLI finish check permits invoked write callbacks with model_readback,
rejects invoked read callbacks, and retains the existing physical callback
rejection. Actual endpoint histories and global comparison counts are exactly
those of executed ordinary endpoint reads, as in generated PDL; no fabricated
logical read comparison or new hidden PDL directive is introduced.

## Tests and performance

TDD native Runtime provenance for nested writes, nested read cancellation,
actual read invocation, resets, tail applies and exceptions. Native readback
tests prove actual known/wrong-expected endpoint data; direct logical callback
data cannot become observed; abstract storage/postconditions stay unknown before/
after reset and through mixed aliases. Reader rejects read-callback provenance
and unexpanded applied logical reads before deltas, including inner reset.

New tests/src/pattern/test_callback_endpoint_readback.py uses current helper/
fixtures for nonempty source/flat/generated and local-reset replay with the
same observation values. Include PIO, real selectable DataRegister, serial/
bound-TAP or raw endpoints already supported by ordinary readback, nesting/
namespace specialization, sparse/inverted histories, mismatch counts/clear,
failed procedure reset recovery, canceled read callback inside write body,
and atomic previous-output/debug preservation on unsupported invoked reads.
Preserve Task153's existing inner-read-callback-reset rejection regression.
A no-op write callback with a requested value must not create actual endpoint/
logical observations. Multi-file replay and generated postconditions must
obey the same actual/unknown distinction. Maintain ordinary callback-free checks.

Expected complexity: O(expanded operations + observed bit count + reset_count
times native model/callback bit count), with static callback membership indexed
once. No replay per queried bit, no full callback scan per operation, no new
dependency. Measure normal64/256/1024 write callbacks/four outer groups,
source/flat/generated with actual endpoint read/getter values, three repeats,
<=10s and<=512MiB each. Measure leak-SAN64targets/fourgroups at the same bounds.
Use matched callback-free/unused-declaration and model-observer-off controls;
retain existing normal1024target callback budget and16Kwide-alias256MiB bound.
Record exact commands, environment/source/tool/output hashes, elapsed/RSS;
no unmeasured speedup claims. Retain existing corpus/helper acceptance unchanged.

## Workflow and completion

Fresh builder check before plan; inline implementation. Scope files:
pattern_runtime.h/.cc, pattern_readback.cc, icl_pattern_generation_main.cc;
existing Runtime/readback native suites; new tests/src/pattern/ script and
Make registration; current guide/tracker. Add scheduler support only if actual
tests show existing Assume cannot maintain unknown abstract storage correctly.

Full14native/14leakSAN, complete dynamic/static generators, current selected
normal/SAN parser+model corpus and producer/protocol/Make checks, C++ quality,
source packaging and measured controls. One fresh final independent review;
one test-first Critical/Important fix pass, deferMinors/no rereview. Exact
merged input/product checks, push, detach preserved worktree/delete merged
branch/archive ownledger. Continue original remaining inventory afterward;
coverage1 remains excluded and no broad item closes from this increment.
