# Bounded wide decimal ICL constants

Task171 advances inventory11/12 from parent master
21a6aea7f693abb3ad8f82f04b7fe7532e200824, on owned branch
codex/wide-decimal-constants. The existing question opt-out, delegated design
choices and native inline execution apply. This is a small shared literal
decoder change; no separate builder review is needed.

IEEE1687-2014 §6.3.8 (printed62–64) describes unsigned numeric bit patterns,
decimal bases, padding, significant-bit truncation errors and inherited widths.
The existing constant representation already stores arbitrary bounded binary
strings. Eight corpus defaults fail solely because decimal Literal routes
through EvaluateInteger's checked signed64-bit intermediates: four64–256-bit
parameters, one parameter-sized attribute and three96/127-bit enum cases.
Four other wide-width failures exceed existing index/value limits and must
remain failures. AccessLink serialization remains separate binding work.

Choose a private bounded decimal-to-binary conversion at Literal. Reuse the
existing parser's32-bit limb accumulation approach, processing up to nine
decimal digits per multiply/add. Keep EvaluateInteger and Constant::AsInteger
unchanged. Replacing arithmetic with a general big-integer API changes an
unrequested contract; adding a dependency for literal conversion is unnecessary.
The parser's private FitsWidth is a boolean validation fast path, so do not
rewire it just to expose a conversion function.

Preserve the1,048,576-bit value limit, underscore/comment handling, zero and
leading zeros, sized/unsized flags, width inheritance, inversion/concatenation,
unknown binary/hex values and significant-bit rejection. Validate digits and
bounded growth. No expected comparison may become an observed value.

The algorithm uses O(B) storage and O(D*ceil(B/32)/9) accumulation work for
D decimal digits and B significant bits, plus O(B) rendering. Typical corpus
values are64–256 bits. Qualify10 fixed all-ones values through65,536 bits with
native exact-bit assertions and matched hex controls, using three repeated
processes per native parent-hex/current-hex/current-decimal/SAN-decimal mode.
Keep10s/512MiB per process. This does not promise a latency bound for decimal
values near the maximum resource size; larger routine workloads require a
separate measurement and may justify a different conversion algorithm.

Add eight CLI methods covering parameters/defaults, overrides, enum reset/
capture, attributes/concat/inversion, decimal lexical forms, sparse/inverted
values, retained integer/resource limits and atomic rejection/unknowns.
Require original/flat/generated replay, exact actual data/status and retained
outputs on failure. Run the complete normal/static generator variants,
15 normal/15 leak-enabled sanitizer native suites, affected CLI/corpus,
changed-range C++ quality, package and existing work/reporting/allocation probes.
Qualify actual corpus changes rather than assuming every failure is repairable.

One fresh final review, one test-first blocker fix wave, then merge into master,
verify the merged result, push and delete the owned branch. Preserve117 stashes,
ignored products and25 inherited deferred Minors. Item1 remains excluded and
broader original inventory remains open.

## Measured scalar path
An initial SAN mixed-group gate misses its combined10s/512MiB bound. The
retained reproduction passes at521872KiB; three matched generated runs use
521588–521652KiB current versus518012–518528KiB parent. Scalar0 allocation
counts are4 in both, falsifying the added-count hypothesis. A uint64/from_chars
shortcut lowers allocation count to2 but raises matched SAN median RSS to
523704KiB versus518376KiB parent; discard it.

Preserve the original scalar path for tokens of at most18 characters (decimal
values fit signed64); longer tokens use bounded limb conversion. This restores
scalar allocation counts to4 and pilot median RSS to518344KiB current versus
518192KiB parent. Keep signed Binary and all integer APIs unchanged. Empty and
malformed numbers still reject, including short partial numeric prefixes.

Require six final matched leak-enabled SAN runs with exact output and strict
10s/512MiB bounds. The additional relative diagnostic allows1MiB over parent
median, covering observed overlapping RSS ranges; it does not relax either
original bound. Rerun all88 gates and846 measured processes on the final source.
