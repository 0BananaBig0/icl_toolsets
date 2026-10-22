# Independent identifier/context fixtures

These are hand-maintained constructed ICL templates, not copied vendor examples.
`ProbeName` is an ordinary legal name used as the substitution site. Templates
cover all 26 port declarations and 32 additional declaration/reference contexts.
Their acceptance is classified in `tests/cases/conformance.json`. They are syntax
inputs; external module/port/PDL-procedure references are not complete hardware
models, and parameter evaluation/connectivity are not asserted.

The oracle is IEEE 1687-2014 §6.3.3(b), printed 60/PDF 74: keywords are not reserved.
§§6.3.10.2–5, printed 65–68/PDF 79–82, define scalar, parameter, vector and
hierarchical identifiers. `names.txt` contains the 75 word spellings in the pinned
Honza/IEEE comparison (`docs/honza-icl-comparison.md`), supplemented with Generic,
edge/polarity words, time-unit spellings and ordinary scalar-name boundary cases.
All 92 names satisfy the IEEE scalar-name lexical rule. The list is maintained
independently and is not scraped from our lexer/parser at test time.

The runner adds four malformed spellings from §6.3.10.2 examples: `_bus3`,
`5bitbus`, `bit-bus`, and `A$suffix`. They should fail in these identifier slots,
with one explicitly adjudicated exception. In a callback procedure slot,
`ReadCallBack ns A$suffix;` tokenizes as procedure `A` followed by argument
`$suffix`, permitted by §6.4.9 (printed 118/PDF 132) and the free-format lexical
rules. It is a positive test for that complete statement, not evidence that
`A$suffix` is a legal scalar identifier. A first run's three failures were three
views of this same incorrect negative expectation; no production relaxation was
needed after reading the normative callback grammar.

`make keywordcheck` creates combinations beneath ignored
`tests/generated/ours/keyword_contexts/<context>/<index>.icl`, checks individual
acceptance in batches, then tests CLI content preservation, reparsing and
idempotence. A persistent LSP process receives every case as a versioned edit,
checks the independent acceptance expectation, exact formatted text and whole-
document replacement ranges. Formatted templates are cached once; legal name
substitution derives expected output without thousands of CLI subprocesses. The
negative cases must produce no formatting edits. A report records the names,
contexts, case count and failures. Templates deliberately use one-line modules
requiring formatting; silently returning no edits for a positive is a failure.
