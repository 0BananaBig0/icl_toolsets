# Callback execution inside merge calls

The user requested completion of the original remaining inventory except item1,
delegated implementation choices, opted out of questions, and authorized commits,
parent merges, pushes and task-branch cleanup. This is Task164 under item4.
These design choices are the executor's; no specific user design approval is claimed.

## Intent and existing evidence

Allow valid iCall bodies inside an active iMerge to invoke explicit ReadCallBack,
WriteCallBack and supported transparent downstream write prerequisites. Preserve
each call's Apply order and callback effects using existing serial fallback.
This does not complete the broader callback or remaining inventory.

Parentmaster6a311dd has clean, pushed Task163 qualification:63 gates,
15normal/15sanitizer native suites,73x1189 tests per generator,4039 input/38
product hashes,324 matrix/example and4 allocation processes,26 merged checks.
All117 existing stashes are preserved. Reuse those exact baseline receipts;
retain an immutable parent executable and source/hash receipt for new controls.

IEEE1687-2014 clause7.9.18, printed176–178, permits trivial serialization,
requires per-call Apply order, prohibits reset inside called procedures and
requires empty entry/return queues. Opened local standard extract:
/tmp/item11-1687.txt:9135–9210. Existing Runtime enforces queue/reset rules.
Its callback dispatcher adds an unconditional active-iMerge rejection.
Merger::Block already emits unbatchable streams in call order; Batchable currently
does not treat callback provenance as a barrier. Write postconditions often make
such streams unbatchable incidentally; that is not sufficient for read callbacks
or callback-empty bodies. Explicit provenance must decide the barrier.

## Approaches and decision

1. Reuse existing serial fallback for complete callback-containing call streams,
   with an explicit Batchable barrier and existing runtime dispatch. Smallest
   change, predictable ordering, no parallel callback speedup promised.
2. Add callback-aware parallel planning, endpoint conflict analysis and negotiated
   callback execution. Larger state machinery without a current requirement that
   callbacks run concurrently; defer this optimization.

Choose1. Keep ordinary neighboring calls eligible for existing batching. No new
dependency, public API, parallel scheduler, speculative cache or transport.

## Contracts and reach

- Runtime executes callbacks in valid merge-call context with existing argument,
  namespace, last-wins, preflight, caller-goal, implicit tail Apply and cycle rules.
- Merger::Batchable treats logical callback and actual callback execution markers
  as barriers. An entire containing call stream remains serial, in source order.
  Existing independent ordinary call batching and nested merging remain valid.
- Preserve iMerge note hoisting, call identity, source origins and append-only
  callback markers; no mutation of original requests or duplicate evaluation.
- Keep reset prohibited anywhere inside merge calls, including callback bodies;
  preserve empty queue rules, failed callback poisoning, binding/cycle errors,
  caller reservations and protected output artifacts. Do not promise recovery of
  a poisoned queue inside a merge: reset remains forbidden there.
- Preserve source/flat/generated fixed points and caller-local reset rewriting,
  including nested merges, repeated/returned sites and literal procedure bytes.
  If an existing source consumer fails, refine this design against that failing
  test before the minimal fix; do not silently drop replay requirements.
- Modeled iMerge histories remain unsupported by the existing observer command
  guard. Actual read-callback/automatic histories and physical callback output
  retain their guards. Broader raw/opaque merge support is separate item8 work.
- Change Runtime and Merger plus necessary callers/tests/docs only. New test and
  benchmark scripts live in tests/src/pattern; maintain existing native tests.

## Verification and scale

Write failing native Runtime/merger tests and CLI cases before implementation.
Cover explicit/inferred/read/mixed aliases, multiple and nested calls, note order,
ownership/goals, failed/cyclic/reset/unapplied cases, source/flat/generated/local
reset replay, namespaces/split inputs and once-only evaluated arguments. Retain
old rejection tests only where their behavior is still unsupported.

Reuse the downstream connected-storage fixture and physical-frame parser for
matched merged versus serial controls; add exact ordered callback postcondition
pairs in the new benchmark without changing Task163's deferred Minor. Use four
value groups, normal16/64/128 targets, sanitizer16 and three source/flat/generated
repeats, plus512/1024-bit normal payloads. Limits remain10s/512MiB regular and
10s/256MiB wide. Parent256 explicit control already takes15.8781s: no256 claim.
Report actual callbacks versus generated literal notes, exact PIO frames, zero
CSU only after rejecting iScan, inputs/products/environment/hashes and timings.

Run focused checks, required full normal/static generator and normal/leak-enabled
sanitizer native suites, current C++ quality, corpus and source packaging checks.
Preserve the Task163 repeated matrices/allocation controls with current products.
One fresh whole-branch independent review, one test-first blocker fix pass;
ledger Minors and declined-behavior costs, no rereview. Merge into direct parent
master, check merged identical inputs/products, push and delete only this task
branch, archive only its ledger, and continue the original inventory.
