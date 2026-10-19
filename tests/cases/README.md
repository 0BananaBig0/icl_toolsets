# Test corpus

Every `.icl` file here is a regression case. The directory it sits in says
where it came from — see `SOURCES.md` for the full list.

```
pass/standard/    IEEE 1687-2014 itself (clauses and Appendix E)
pass/tessent/     Tessent: real tool output and manual fragments, completed
pass/benchmark/   IJTAG-Ecosystem/Benchmarks, the standard benchmark networks
pass/upstream/    the test files of Honza255/icl_parser
pass/published/   papers, application notes, articles
pass/ours/        written here, each pinning one behaviour
fail/standard/    rejects a construct the standard does not allow
fail/ours/        rejects malformed input
```

## What every case must do

`pass/` — exit 0, and the output must be **idempotent** (formatting it again
changes nothing) and **character-preserving** (stripping all whitespace, input
and output are identical). Those two properties are what make a formatter
trustworthy; a case that only checked "it parses" would miss both the
comment-indent drift bug and the stray-blank-line bug.

`fail/` — exit 2, with a line number. Never a silent reformat.

Both are run twice: once through **stdin** and once through a **real file**.
That matters — the BOM was once stripped from a file but rejected from stdin,
and with stdin alone the corpus could not have seen it.

## Adding a case

Put the file in the directory matching its origin. The runner walks
subdirectories, so nothing else needs editing.

If the source is a publication that prints only a fragment, complete it into a
whole file and say so in a comment — what should be tested is what the
publication actually wrote, and a fragment is not valid input.
