# Requirements

What was asked for, what was delivered, and — where something could not be
done as asked — what was done instead and why.

This file exists because the gap between "requested" and "delivered" is easy
to lose. Every entry below is either a requirement that was met, or a
requirement that was not, with the substitution stated explicitly. Nothing
here is implied to work beyond what is written.

---

## 1. The original brief

Asked for, as a single specification:

| # | Requirement | Where it lives | Status |
|---|---|---|---|
| 1 | Treat the subject as **ICL and C++** expertise | whole tree | Met |
| 2 | Grammar follows the **IEEE 1687** standard | `src/icl_parser.yy` | Met, with two corrections (§4) |
| 3 | Use **flex and bison** to generate the parser | `src/icl_lexer.l`, `src/icl_parser.yy` | Met (flex 2.6.4, bison 3.8.2) |
| 4 | Pipeline must be **AST → format tree → formatted output**, in that order | `icl_parser.yy` → `format_tree.cc` → `RenderFormatTree` | Met |
| 5 | Target **Rocky Linux 8+** | `Makefile` | Met with a caveat (§4) |
| 6 | Install default `$HOME/.local/bin`, libraries to the matching `$HOME/.local` directories | `Makefile` (`prefix ?= $(HOME)/.local`) | Met |
| 7 | `make uninstall` must remove what was installed | `Makefile` manifest | Met, verified by round trip |
| 7b | *(added later)* Build both flavours, but **install them as separate options** | `install-dynamic` / `install-static` | Met — both install under the same names, so it is a straight choice |
| 8 | **Google** naming and formatting conventions | all sources | Met |
| 9 | *(added later)* Produce **two programs**, `icl-format` and `icl-lsp`, each in **static and dynamic** form | `Makefile` | Met — 4 binaries |
| 10 | *(added later)* The LSP server must work with **coc.nvim**, driven as vim → coc → icl-lsp | `src/icl_lsp_main.cc`, `tests/coc_vim_check.vim` | Met, verified in a real editor |

### On requirement 9 — one assumption

"同态程序" was read as **the dynamically linked counterpart** of each static
binary, i.e. static/dynamic pairs. If it meant something else, this is the
entry to change: `STATIC_BIN` / `LSP_STATIC_BIN` in `Makefile`.

---

## 2. Requirements added during the work

Each was a separate instruction, not part of the original brief.

| Round | Asked | Delivered |
|---|---|---|
| a | Close the verification gaps I had listed | 82 assertions; found 3 real bugs |
| b | "Just make it conform to 1687" | Looked up the standard; found 2 bugs where the code was self-consistent but wrong against the standard |
| c | Re-check for bugs and reusable code, repeatedly | ~10 bugs fixed; ~400 lines removed by merging duplicated logic |
| d | Follow C++ design principles | Visitor replaced 7 `dynamic_cast` sites; global state moved into `IclDriver`; C++ unit tests added |
| e | Optimise per `karpathy-guidelines` and `ponytail` | Applied from the published text (§3); `ponytail:` ledger added to the source |
| f | Report binary sizes, including a single-file bundle | dynamic 146 KB stripped / static 2.1 MB / xz 46 KB |
| g | Fetch the guidelines from their sites rather than declining | Done — see §3 |
| h | Package everything as one file if needed | `make dist` → tarball with the parser pre-generated |

---

## 3. When a request could not be fulfilled as stated

### 3.1 `karpathy-guidelines` and `ponytail` were not available as skills

**What happened.** Asked to follow both. Neither is in the set of skills
available to me — I attempted to load each one twice, and both attempts were
refused.

**How it was handled.** When told the sources could be read from the web, I
searched for and read the published text of both, then applied them:

* **karpathy-guidelines** (4 rules: think before acting, prefer boring and
  minimal, surgical edits, goal-driven verification) — drove the removals and
  the decision *not* to refactor working code.
* **ponytail** (the 6-rung laziness ladder: does-not-exist → reuse → standard
  library → platform-native → existing dependency → one line → minimal; plus a
  never-cut list and a `ponytail:` ledger for deliberate shortcuts) — drove
  the deletion of speculative abstractions and the ledger itself.

**Not claimed.** I never claimed to "use" those two skills, because I did not.
The work follows the published documents, read from the network. If the
canonical text differs from what I read, the code follows the wrong version —
that is the residual risk of this substitution.

### 3.2 bison and flex could not be installed at first

**What happened.** `apt` sources returned 403.

**How it was handled.** You suggested changing the mirror. The Tencent mirror
worked and both tools installed. The C parser stopped being a paper exercise
and became the thing actually built and tested.

### 3.3 Rocky Linux 8 ships bison 3.0.4, below the tested 3.8

**What happened.** The grammar is tested against bison 3.8 (it uses the C++
variant API with constructors, plus `%define parse.error detailed` and
`%define parse.lac full`). Rocky 8's 3.0.4 is below that. I could not install
3.0.4 to test a downgrade.

**How it was handled.** Two things, chosen so that neither guesses:

* a **version guard** in `Makefile` — bison < 3.8 stops the build with an
  explanation, instead of failing somewhere inside bison;
* **`make dist`**, which bundles the generated parser and scanner so the
  target machine needs only g++. Verified by building with
  `BISON=/nonexistent FLEX=/nonexistent`: succeeds, full suite passes.

**Not done.** I did not lower `%require` and hope for the best. A grammar that
silently resolves differently on an older bison is worse than a build that
refuses.

### 3.4 No real Tessent-generated ICL was ever available

**What happened.** This is the largest open gap. Every sample came from a
public paper (IEEE P1687 SIB + AccessLink examples) or from ICL I wrote
myself.

**How it was handled.**

* Looked the standard up on the network rather than trusting my own samples —
  which is how two genuine bugs surfaced: `ScanMux` was rejecting the standard
  `SelectedBy` keyword, and eight client/host port functions
  (`TRSTPort`, `ToTCKPort`, `ToTRSTPort`, `ToShiftEnPort`, ...) were absent.
  Both were wrong-but-self-consistent: my samples and my grammar agreed with
  each other and disagreed with the standard, so no test could catch them.
* Designed the failure mode to be safe: an uncovered construct is **rejected
  with a line and a message naming what was expected**, never silently
  reformatted. The report is enough to add the missing rule.

**Residual.** Vendor dialect coverage is still inference, not measurement.
One real file from your flow would be worth more than everything else here.

### 3.5 GitHub returned 403 for coc.nvim

**What happened.** Needed for requirement 10. `github.com`, `ghproxy.com`,
`gitclone.com`, `gitee.com/mirrors`, `hub.fastgit.org` and `gh.llkk.cc` all
refused. The `coc.nvim` package on npm is TypeScript type declarations only —
not the plugin.

**How it was handled.** `codeload.github.com` serves repository tarballs and
was reachable, so the `release` branch (2.2 MB, with a prebuilt
`build/index.js`) was downloaded through it. `tests/setup_coc.sh` encodes this
with a fallback list.

**Residual.** Depends on that mirror staying reachable.

### 3.6 coc.nvim cannot run on vim 8.2

**What happened.** `autoload/coc/api.vim` is vim9script using `:export`, which
vim 8.2 — the version Ubuntu 22.04 ships — rejects with
`E1043: Invalid command after :export`. The failure is quiet: coc's node
process starts and `coc#rpc#ready()` returns 1, but the handshake never
completes and every `CocAction` throws `coc.nvim not ready`.

**How it was handled.** Built vim 9.1.1000 from source (also via
`codeload.github.com`). `tests/setup_coc.sh` does this automatically when the
system vim is older than 9. Neovim is unaffected — coc takes a different code
path there.

### 3.7 Two rounds where a script corrupted a source file

**What happened.** Editing generated sources with substring indexing: once
`s.index('attr_body')` matched a `%type` declaration instead of the rule,
inflating the file to ~370k lines; once a slice end fell *before* its start,
producing an empty replacement that inserted text between every character
(21,475 lines). Both times I restored from the previous round's tarball.

**How it was handled.** Switched to whole-line exact matching, added
`assert` guards on slice non-emptiness and anchor uniqueness, and ran
`git init` so the tree had real rollback.

**Recommendation.** Keep this in git and run `make check` in CI. Both corruptions
were of a kind a single `git checkout` would have undone.

---

## 4. Three assumptions that were made for you

Stated here because each is a place where I decided rather than asked.

1. **"同态程序" = the dynamically linked build.** Each program is produced
   twice, static and dynamic. See §1.
2. **Attribute names are not validated.** Inside a block any keyword is
   accepted as an attribute name, because real ICL writes things like
   `DataInPort DI = 8'h00;` inside an `Instance`. Consequence: a misspelled
   attribute name is not an error — only structure is checked.
3. **`--ast` is kept.** Requirement 4 says the AST must exist; without `--ast`
   there is no way to observe that it does. Removing it would also drop the
   second `AstVisitor` implementation, which would make the visitor pattern
   itself an abstraction with a single user — worth deleting at that point.

---

## 5. Later rounds: coverage gaps found by probing

### 5.1 The element list was wrong, and checking it found five missing constructs

The grammar had been built from samples and from the standard as remembered.
Checking it against IEEE 1687's canonical element list (Table 2, *IJTAG
Tutorial*, 3rd ed.) showed the two did not agree:

| | |
|---|---|
| **Missing** | `DataRegister`, `DataMux`, `ClockMux`, `OneHotScanGroup`, `OneHotDataGroup` |
| **Wrong name** | `OneHotScanInterface` (the standard says `OneHotScanGroup`); `Reg` (the standard says `DataRegister`) |
| **Invented** | `Field` — not in the element list. `Field f[1:0] Of r { }` in particular was my own construction, and it was in the test suite, so the tests agreed with the bug |
| **Not ICL at all** | `iProcsForModule` — it is a **PDL** keyword (Procedural Description Language, IEEE 1687's companion language), not ICL. It had been lexed as an ICL keyword, which is simply wrong |

All five were added; `iProcsForModule` was removed. `Reg`, `Field` and
`OneHotScanInterface` were **kept** — they cost one line each once grouped,
and rejecting a file we cannot verify is the worse failure for a formatter.

### 5.2 The refactor the gap forced: group keywords by *shape*

Adding five keywords meant ten new productions under the old layout, because
each element had its own pair of rules (block form and `;` form). That is what
made the omission likely in the first place.

The grammar now has five shape groups, and a keyword is one line in the right
group:

| Group | Shape | Members |
|---|---|---|
| `name_block_kw` | `K name { }` | Enum, ScanInterface, OneHotScanInterface, OneHotScanGroup, OneHotDataGroup |
| `range_block_kw` | `K name [range] { }` / `;` | ScanRegister, DataRegister, Reg, Field |
| `sel_block_kw` | `K name SelectedBy v { }` | ScanMux, DataMux, ClockMux |
| `of_block_kw` | `K name Of n { }` / `;` | Instance, AccessLink |
| `port_keyword` | `K name [range] { }` / `;` | the 22 port functions |

16 productions collapsed to 7, and the groups cannot drift apart because each
shape exists in exactly one place.

### 5.3 The regression corpus, and the two cases that had to be deleted

The corpus now lives in `tests/cases/` — one `.icl` file per case, `pass/` and
`fail/`, run by `tests/regression.py`. Adding a case means dropping in a file.
See `tests/cases/README.md` for the rule on failure.

Building it required exporting the inline dicts from `run_tests.py` into
files, which immediately found two cases that should not have been there.
Both are the second kind of "wrong case" in that README — a case that passes
for the wrong reason:

| Case | Why it had to go |
|---|---|
| `alias_with_iprocs` | Named for `iProcsForModule`, but that keyword had been **removed** as PDL-only (§5.1). It still passed because the text was then read as an ordinary attribute name. Substituting any nonsense word passes identically — the case could not fail, so it tested nothing. Replaced with a real nested-block case. |
| `utf8_bom` (in `run_tests.py`) | Asserted a UTF-8 BOM survives formatting. The tool was later changed to strip it (§5.4), so the case encoded an assumption the code had deliberately reversed. |

`run_tests.py` no longer holds the corpus: for a while it did, alongside
`tests/cases/`, and the two copies disagreed within one commit — the copy in
`run_tests.py` still asserted the BOM survives while `tests/cases/` expected
it stripped. One corpus, one place.

### 5.4 A BOM was accepted from a file and rejected from stdin

The shape groups (§5.2) removed the duplicated per-element productions, but
one list was left hand-written: `attr_name`, the keywords allowed as an
attribute name or nested-block header. It had not been updated when the five
elements and two port functions were added, so:

```
ClockPort clk;                        ← accepted (module level)
Instance i Of M { ClockPort clk = 1; } ← rejected (inside a block)
```

Same keyword, two answers, depending only on where it appeared. Seven
keywords were affected (`DataRegister`, `DataMux`, `ClockMux`,
`OneHotScanGroup`, `OneHotDataGroup`, `ClockPort`, `ToClockPort`).

Fixed by building `attr_name` from the groups:

```
attr_name : IDENT | any_keyword ;
any_keyword : name_block_kw | range_block_kw | sel_block_kw
            | of_block_kw | port_keyword | <the 9 singletons> ;
```

It is no longer possible for the two to disagree, because there is one list.

This is the second time a duplicated list drifted (§5.1 was the first). The
rule worth keeping: **if two places must agree, make them the same place.**

### 5.5 A single integer field could amplify output 50,000x

`textDocument/formatting` honours the client's `options.tabSize`. It is
applied per indent level, so a nested file with `tabSize: 99999` produced a
**7.8 MB reply from a 157-byte input**. No crash — just unbounded memory from
one field. Clamped to 16 (real editors send 2, 4 or 8), with a regression
test asserting the reply stays under 10 KB regardless of what is requested.

### 5.6 A comment bug that only a ragged-indent sample could show

Block comments keep their relative indentation across continuation lines.
The code computed the common indent, then rebuilt each line as

```cpp
piece = base + std::string(extra, ' ') + piece.substr(common);
```

`piece.substr(common)` **already** carries the relative indent — cutting at
`common` leaves `indent - common` leading spaces in the text. Adding `extra`
on top counted it twice, so a comment with ragged continuation indent drifted
right on every run:

```
input     * c        (5 spaces)
run 1         * c    (7)
run 2             * c(11)
```

Every existing sample had uniform continuation indent, where `extra` is 0 and
the double count is invisible. One sample with ragged indent exposed it. Fixed
to `piece = base + piece.substr(common)`, with three regression tests
(ragged star comment, comment that opens with a star, comment with an internal
blank line).

This is the same lesson as §5.1 in miniature: the samples were too uniform to
reach the bug.

### 5.7 Two port functions were missing, found by running a real ICL file

A Keysight application note (*IEEE 1687 Silicon Test to Board Test*) contains
a complete ICL file for a GPIO instrument. Running it through the parser
rejected it at line 13:

```
ClockPort CLOCK;          ← syntax error, unexpected identifier
```

`ClockPort` and `ToClockPort` are both in Tessent's *ICL Port Types* table
(Table 3-14, mapping `ClockPort`→`clock` input, `ToClockPort`→`to_clock`
output) and in the ICL statement descriptions ("ClockPort declares a module's
functional clock; non-test-clock ports are declared with ClockPort"). Neither
was in the grammar.

Both added. The file is now `samples/keysight_gpio.icl` and is checked for
idempotence and character conservation like the other samples — it is the
first sample in the tree that was written by someone else.

Two further real-world files (a `DataMux` module and a multi-instance module)
were run and pass unchanged.

**Still rejected, and believed correct:** Verilog escaped identifiers
(`\si `), instance arrays (`Instance i[3] Of T;`), and chained connections
(`a -> b -> c;`). ICL is not Verilog, and none of the three appears in any
ICL source consulted. They are recorded here rather than added on a guess.

### 5.8 The lexer's regular expressions did not match vendor ICL

Auditing each pattern against documented vendor syntax, not against my own
samples. Two gaps, both from the Tessent IJTAG User's Manual (§"ICL Network
Extraction of Parameterized Modules"), which is the closest thing to real
vendor ICL available:

| Pattern | Was | Now |
|---|---|---|
| Number literal | `{DIGIT}+'[sS]?[bBhHdDoO]...` — requires a width | width made optional; `'b1` accepted, as in the manual's `InputPort din = 'b1;` |
| Decimal | `{DIGIT}+` | `{DIGIT}(_?{DIGIT})*` — accepts Verilog digit separators (`1_000`) |
| Parameter reference | `$` was an illegal character | `"$"{ID}` → `PARAMREF`, usable anywhere a range bound or value is: `DataInPort datain[ $MSB:0 ];` |

The parameter case matters more than the others: a parameterised module is a
normal way to write ICL, and every range in such a file was being rejected.

**Checked and found correct:** block-comment pattern against `*/` inside a
string, `/* /*` nesting, `**/` endings, and `/**/`; keyword longest-match
against names that begin with a keyword (`Regs`, `Modules`, `Instance1`,
`ScanInPortd` all lex as one identifier); BOM, CRLF and tab input; a string
containing a newline is rejected rather than silently spanning lines.

### 5.9 Earlier in the same pass

Running the standard's constructs against the parser, after the initial build
was already green.

| Gap | Fix |
|---|---|
| **Operators were missing entirely from the lexer.** `-1`, `~a`, `a == b`, `a << 2` were all "illegal character" — real ICL attribute values are expressions (`ResetValue -1;`, LogicSignal terms like `TDR1[61:32] == 32'b0`), so legal files were being rejected. | One `OPERATOR` token carrying its own spelling, plus unary/binary/parenthesised `value` rules. `%left OPERATOR` resolves the 2 shift/reduce conflicts left-to-right grouping produces. Not a token per operator: the formatter only needs the text back unchanged. |
| **`Json::bool_value` / `number_value` were never called.** | Deleted. Only the types actually read remain (int, string, elements). |
| **`<vector>` included in `icl_lsp_main.cc` but unused.** | Removed. |
| **The stdin error path duplicated the file error path** even though a stdin job is already named `<stdin>`. | Merged into one diagnostic. |

**Considered and not extracted:** both front ends repeat the same five lines
(create driver, set style, format, check for error). They genuinely diverge
after it — the CLI writes files, the LSP returns a `TextEdit` — and two
occurrences of a five-line pattern do not pay for a new module.

**Known and deliberately not fixed:** a comment cannot appear mid-statement
(`ScanInPort /* mid */ SI;`). Fixing it means adding an optional `COMMENT` to
nearly every production — a large change for a rare spelling. It is recorded
under Known limitations in `README.md`, and the failure is a rejection with a
line number, never a silent misformat.

`Instance` nested inside another `Instance` block is also rejected, which is
correct per the standard — `Instance` belongs at module level only.

## 6. What was verified, and what was not

**Verified**

| Suite | Count | How |
|---|---|---|
| C++ unit tests | 10 | `make unitcheck` — AST and layout, no bison/flex |
| Regression corpus | 87 | `tests/cases/`, run by `tests/regression.py` |
| CLI behaviour | 49 | `tests/run_tests.py` |
| LSP protocol | 26 | scripted client, coc.nvim's message sequence |
| neovim built-in client | 10 | real client, real handshake |
| vim 9 → coc.nvim → icl-lsp | 10 | real editor, nothing simulated |

Plus: ASan + UBSan + leak detection clean on both programs; idempotence and
character conservation on every sample; `make install` → `make uninstall` with
no residue; 230 KB / 9,000 modules in 11 ms.

**Not verified**

* Real Tessent output — see §3.4.
* bison 3.0.4 (Rocky 8's) — see §3.3; the supported path is `make dist`.
* coc.nvim versions other than 0.0.82, and other editors' clients.
* UTF-16 column offsets in LSP positions. Columns are counted in bytes, which
  is correct for ASCII ICL and under-reports on a line containing non-ASCII
  before the reported point. Formatting is unaffected (it replaces the whole
  document); a diagnostic's *range* on a non-ASCII line can be short.

---

## 7. If you pick one thing up from this file

The two bugs in §3.4 — `SelectedBy` and the eight missing port functions —
were not found by reading code or by writing more tests. They were found by
comparing the code against the standard and against real ICL. Everything
before that had gone green.

The tool is built so that the same class of problem announces itself: an
uncovered construct is rejected with a file, line, and the token that was
expected, rather than reformatted into something plausible. Send that message
and the gap closes.
