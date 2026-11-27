# Regression oracle maintenance (Task203)

The completion program retains 38 deferred findings after Task202. Three concern
maintained tests that accept a weaker result than their existing contracts:
the Task187 status-preview ownership check never clears through readback;
Task181 opaque-crossing private payloads are palindromic; and Task192 fanout
scale checks only the upstream literal write. A fourth, Task191's weak topology
rejections, was already corrected by Task192 commit `6f49496` but remains counted.

Preserve production behavior and strengthen those existing tests:

- In the native status-preview case, clear the current five failures through
  readback and require repeated previews to return zero on the same operations.
  A separately owned preview model must fail this assertion.
- In the native opaque-crossing case, use different nonpalindromic stable and
  requested private bytes. Assert their exact prefix, requested frame, copied
  frame and subsequent stable completion. Actual opaque returns stay unknown.
- In the existing fanout scale case, compare the first apply group's complete
  multiset of literal target/value writes with the requested upstream and all
  downstream words. Do not impose an unrequired ordering on commutative writes.
- Verify current shared-input rejection cases use valid four-bit targets and
  require the intended rightmost-bit or overlapping-event diagnostic. Record
  the historical correction rather than changing already-correct tests.

Use controlled negative cases to demonstrate oracle discrimination before and
after the maintained edits. Compile temporary separate-owner and private-payload
reversal mutants against existing objects; restrict execution to each relevant
native case. For fanout, deliberately remove a downstream write from the replay
return. For rejection guards, deliberately supply an unrelated diagnostic.
These are fault-injection controls, not claims that current production is wrong.
The old native/fanout oracles must accept their injected faults before the edits;
the strengthened ones must reject them, while actual production remains green.

No production, public API, syntax, hardware eligibility, solver limit or dependency
change is authorized by this test-maintenance design. All 4133 tracked source/test
paths remain; precisely three test files may change. Forty existing executables,
including all three generators, must retain their Task202 identities; rebuild
only the two affected native programs in normal and default leak-enabled builds.

The fanout grid remains 2/8/32 lanes at 4/64/256 bits, with the existing bound
of 10 seconds for four replay forms. Build the expected writes while constructing the input, then
parse one output group, requiring O(total payload bytes) time and space with no
per-lane subprocess. The small native cases retain their existing eight-bit data.
No performance improvement or broader device coverage is claimed.

Use focused normal/static CLI suites, both complete affected native suites and
their leak-enabled equivalents, changed-C++ quality checks, source packaging,
unchanged-production/product identities and one independent branch review.
Do not repeat unrelated full regression after test-only changes unless new
failures or dependency evidence require it. The candidate inherited remainder
is 38 minus 4 = 34 only after all four dispositions are verified.

Reuse the approved completion contracts, the standing opt-out of design questions
recorded by Task185 and inline execution authorization. Merge, verify, push and
retire this owned branch after one review and at most one Critical/Important
repair wave. Preserve 117 stashes, sibling ledgers and excluded coverage item1.
