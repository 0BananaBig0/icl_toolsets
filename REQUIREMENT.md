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

Running the standard's constructs against the parser, after the initial build
was already green.

| Gap | Fix |
|---|---|
| **Operators were missing entirely from the lexer.** `-1`, `~a`, `a == b`, `a << 2` were all "illegal character" — real ICL attribute values are expressions (`ResetValue -1;`, LogicSignal terms like `TDR1[61:32] == 32'b0`), so legal files were being rejected. | One `OPERATOR` token carrying its own spelling, plus unary/binary/parenthesised `value` rules. `%left OPERATOR` resolves the 2 shift/reduce conflicts left-to-right grouping produces. Not a token per operator: the formatter only needs the text back unchanged. |
| **`Json::bool_value` / `number_value` were never called.** | Deleted. Only the types actually read remain (int, string, elements). |
| **`<vector>` included in `icl_lsp_main.cc` but unused.** | Removed. |
| **The stdin error path duplicated the file error path** even though a stdin job is already named `<stdin>`. | Merged into one diagnostic. |

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
| CLI end-to-end | 121 | `make check` |
| LSP protocol | 25 | scripted client, coc.nvim's message sequence |
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
