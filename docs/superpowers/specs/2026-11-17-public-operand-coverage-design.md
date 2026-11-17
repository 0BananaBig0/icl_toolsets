# Public operand API coverage

Extend the existing expression and symbolic native test runners to cover public
operand validation measured as zero-hit in Task119. The final Task119 run is
17311/17961 lines and29201/46677 raw branches; strict100%/90% remains open.
No product changes, new targets/dependencies, failure injection or exclusions.

Reuse expression Rejects(action,message) and symbolic TestPdlOperands' valid
register/alias/enum fixture. Require the intended runtime-error diagnostic;
an unrelated exception or parser rejection must not satisfy a boundary test.

Expression tests call the public SourceToken APIs directly for malformed
caller-provided tokens/ranges that the source parser rejects earlier: empty,
reversed or out-of-bounds spans; unterminated numeric comment; invalid based
digit; nondecimal operand; missing operand; unmatched close; invalid operator;
incomplete parenthesized expression. Include a valid token/parameter control.
Keep all existing source-parser expression tests.

PDL tests cover numeric width0/1048577 and input longer than1048578 characters,
decimal significant truncation and exact supported width/text boundaries.
Use at most a one-megabit valid payload. Target tests cover unclosed/mismatched
ranges, empty/negative/nondecimal/overflowing/multiple-colon indices, scalar
indexing, out-of-range indices and unknown names. Existing ascending/reversed
alias and enum controls remain; include declaration identity/bit ordering.

The public IR APIs also accept caller-created Netlist/PdlTarget objects. Small
copies of the valid fixture exercise an oversized declared target, an alias
without Source, a self-referencing alias and an empty enum target. Explicitly
label these malformed public-IR tests; they do not claim parser acceptance or
new model support. Do not allocate a million target references merely to reach
an internal guard. Do not force unreachable invariants or remove validation.

Work is a fixed set of small native cases plus linear processing of bounded
strings (1MiB+3 characters for the over-limit rejection), with a few MiB transient memory and no per-case subprocess. Existing
product performance and binary bytes are unchanged; no speedup claim.

Only two existing native test files plus generator/coverage docs and plan/spec
change. Normal native, leak-enabled ASan/UBSan, fresh-profile focused coverage
and Rocky native checks run for both changed runners. Reuse unrelated passing
Task119/116/117 gates only with exact input exclusions and unchanged product
fingerprints. Preserve Task119 reports/profiles and the two native binaries
before rebuilding the existing instrumented directory; reset profiles with
the unchanged complete coverage recipe and report its actual strict result.
One fresh independent final review and necessary correction pass, then commit,
merge master/delete branch/archive ledger/sync. No push. User standing routine
delegation applies; parent master5fcd6a2.
