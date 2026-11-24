# Connected callback decoding latency

Task169 advances inventory10's retained 256-target latency work. The user has
delegated implementation decisions and opted out of questions. Parent is
master c56b8ea97eb7969150f7e898d427dbd9fa047e77; owned child is
codex/connected-callback-latency. Native inline execution remains selected.

## Evidence and goal

The previous matched72-process follow-up retained three current256-target
time misses, with byte-identical results. Fresh native runs are faster and can
meet10s already; retain that variation instead of manufacturing a timing RED.
The existing10s/512MiB regular and10s/256MiB wide bounds do not change.

A partial Callgrind profile of the immutable parent hits the existing solver
timeout under instrumentation. It records21.9b instructions,98.3% in primary
solving,24.8% in Decode and21.4% in Z3_model_eval. This is diagnostic evidence.
A separate native diagnostic completes with exact parent output:1,028 models,
793,612 Decode calls,524,292 repeated word/mask pairs within individual models,
995,352 compound-expression evaluations and527,109 repeated expression IDs.
Existing Decode already bypasses model evaluation for numeral expressions.

Reduce repeated decoding while keeping every solver expression, objective,
model, concrete replay check and emitted event. Require the256-target workload
to use at most600,000 real Z3_model_eval calls (a deterministic reduction from
the measured near-million calls), with correct complete output. Also require
all current matched runtime/replay cases to meet the unchanged bounds.

## Approaches and executor call

1. Cache complete decoded word/mask pairs within one primary solver model.
   Choose this small local change: the native probe measures66% repeated pairs.
2. Cache every evaluated expression throughout the shared decoder. This needs
   broader caller/lifetime plumbing; the local measured path is sufficient.
3. Change symbolic storage construction or add a direct concrete backend.
   This requires new solver/event proofs and is unnecessary for this hypothesis.

## Implementation contract

Inside SolvePrimaryWrites, immediately after obtaining a successful model,
create std::map<std::pair<unsigned,unsigned>,std::string> keyed by bits.id()
and known.id(). A local decode lambda returns a const reference to an existing
entry or emplaces Decode(model,value) on a miss. Use it at the three existing
primary replay decode sites. Frames retain all referenced expressions for
the cache's lifetime. The cache dies before any other model is obtained.

No global cache, model/context reuse, public API, dependency, flags, filtering
of state, shortened validation or changed resource limits. Other decoders stay
unchanged. Cost is O(unique decoded pairs and their total word lengths) memory
per model, with logarithmic map lookup. Measure memory, including wide inputs.

## Verification and scope

Add a maintained real-call counter under tests/src/pattern, using the existing
benchmark fixture/frame oracle and GNU linker wrapping. The wrapper increments
a count and calls the actual Z3 API. It must fail the600,000-call bound on the
immutable parent before product changes; all output checks must pass.

Existing word-model tests cover1/31/32/33/63/64/65/127/128/1024-bit words,
leading zeroes, repeated updates, unknown masks and unequal serial fields.
Retain the primary-pin tie, later-frame, reset, reservation and failure controls.
Run the complete72-process word benchmark (64/128/256 targets,512/1024-bit
words,16-target sanitizer cases,three repeats and three replay forms), plus
the378-process merge-readback matrix. The new parent already contains Task168,
so no parent memory miss is expected
in this task's matrix. Retain any actual misses and do not raise bounds.

Run full normal/static generator checks,15 normal/15 leak-enabled sanitizer
native suites, affected CLI, corpus, changed-range C++ quality, source packaging
and allocation/reporting checks. One fresh whole-branch review, one test-first
blocker fix pass, then merge/push/delete the owned branch. Preserve117 stashes,
ignored products and23 inherited deferred Minors. No general optimality or
larger-topology certification follows from these measurements.
