# Regression corpus

Every file here is one test case. Adding a case is dropping a `.icl` file in;
no test code changes.

| Directory | Rule |
|---|---|
| `pass/` | accepted (exit 0), idempotent `f(f(x)) == f(x)`, and character-preserving (input and output match once all whitespace is stripped) |
| `fail/` | rejected with exit 2 |

Run it with `make check` (or `python3 tests/regression.py`).

## What to do when an old case fails

A failure is investigated, never skipped. There are exactly two outcomes:

1. **The code is wrong** — fix the code. The case stays.
2. **The case is wrong** — fix or delete it, and say which in the commit
   message along with why.

"Wrong case" has two common shapes:

- it encoded an assumption the code has since deliberately changed (e.g. a
  case asserting a UTF-8 BOM survives formatting, after the tool was changed
  to strip it);
- it never tested what its name claims (e.g. a case named for `iProcsForModule`
  that passed only because the parser had no such keyword, so the text was
  read as an ordinary attribute name — a case that cannot fail is not a test).

Both were removed from this corpus, and both removals are recorded in
`REQUIREMENT.md`.

## Naming

File name = what is being tested, lower case, `_` for spaces. A name that
describes the failure it guards is more useful than one that describes the
input, because it tells you what was being protected when it breaks.
