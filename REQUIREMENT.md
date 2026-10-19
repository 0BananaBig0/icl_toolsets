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

### 5.3 Architecture: how deep the shared chain goes

Both binaries are required to share the AST and the layout pass. They do, and
they now share more than that. The chain, and where it splits:

```
CLI:  argv --ParseArgs--> indent --+
                                   |
LSP:  JSON --> options.tabSize ----+--> FormatDocument --> consume
                                   |    (format_job.cc)
      ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^   ^^^^^^^^^^^^^^^^^   ^^^^^^^^
      genuinely different              shared              differs
```

`format_job.cc` holds everything between "an indent width was requested" and
"here is the result": style construction, clamping, parse, AST, layout,
render, and the phrasing of the failure. The front ends differ only in where
the options come from and what they do with the outcome — the CLI writes
files, the LSP returns a `TextEdit` or a diagnostic.

Two things this bought immediately, both of which had already gone wrong:

| | |
|---|---|
| **Clamping** | `tabSize` was clamped in the LSP but `-n` was not. Shared now, so `-n 99999` is bounded exactly like the editor's `tabSize`. |
| **Error phrasing** | The CLI printed `name: line N: msg`; the LSP rebuilt the parts itself. `FormatOutcome::Diagnostic()` is the one source. |

`IclDriver::FormatFile` was deleted. File reading is the one step the LSP
never performs, so it belongs to the CLI, and the BOM stripping it used to
hide has moved into `FormatText` where every path gets it.

### 5.3z A generated test for the one bug class that keeps recurring

The single recurring failure mode in this project is **two lists that must
agree and don't** — the element list (§5.1), the `attr_name` keyword list
(§5.2's aftermath). A manual review catches it once; it does not stop it
coming back.

So the scanner is now the source of truth and the grammar is checked against
it: `run_tests.py` parses the keyword rules out of `icl_lexer.l` and asserts
that **every one of them** (46 today) works as an attribute name inside an
`Instance`. Add a keyword to the scanner and forget `any_keyword` in the
grammar, and the test names the keyword.

Verified by injecting a keyword into the scanner only:

```
[FAIL] every scanner keyword works as an attribute name -> drifted: ['Zznewkw']
```

### 5.3v Four more gaps, found by reading the ANTLR grammar rule by rule

Comparing the published `icl.g4` production by production rather than by
keyword list surfaced four things keyword comparison missed:

| Gap | Evidence |
|---|---|
| `Period 10ns;` — a number with a time unit | `period_def` allows `s/ms/us/ns/ps/fs/as` |
| `ReadCallBack m::p <D>;` — iProc argument markers | `iProc_args : '<D>' \| '<R>' \| number \| STRING \| parameter_ref` |
| `a::b::c` — more than one `::` in a name | `iProc_namespace` has two separators |
| space-separated values after an attribute name | `ReadCallBack` takes name + namespace + args with nothing between |

The first two are **lexer** gaps, and worth stating precisely: `10ns` lexed as
NUMBER followed by IDENT, and `<D>` as `<` `D` `>`, so both reached the parser
as tokens no rule can join — a valid literal rejected for a lexing reason.
Fixed where they arise, one rule each.

The last two are grammar. `attr_value` gained a space-separated alternative and
`::` became left-recursive. **Both are zero-conflict**, which is what made them
cheap to accept; a change that had cost a conflict would have been weighed
against the fact that no real file here uses `ReadCallBack` at all.

**Deliberately not added:** `Instance` inside `OneHotDataGroup`, which `g4`
permits. No file here uses it, and allowing it means allowing `Instance` deep
inside any block — contradicting the (verified) rule that Instance is a
module-level construct. One real file would settle it.

### 5.3u codegraph, and what it can and cannot see here

`pip install codegraph` gives version 1.2.0. It parses with **Python's own
tokenizer**, so it only sees `.py` files — pointed at `src/` it returns an
empty graph. It is genuinely useful on `tests/` (three helpers), and there it
found a real dead function:

```
load_samples    run_tests.py    links_in=0  out=0  14 lines
```

Left over from when the corpus lived in `samples/`; deleted.

It also misreports `check` in `run_tests.py` as `links_in=0` when it is called
28 times — same-named functions in different modules confuse its resolution.
So: worth running, not worth trusting without a grep.

**The C++ half comes from the compiler instead.** `g++ -MM` already knows every
header each translation unit includes, so `make codegraph` uses that — no
second parser to keep in sync. This is the ponytail ladder applied: reuse what
the platform already computes.

The output answers the architecture question directly:

```
icl_format_main.cc -> format_job.h
icl_lsp_main.cc    -> format_job.h, json.h
```

Neither main includes `icl_ast.h`, `icl_driver.h` or `format_tree.h`. The
split is at the outermost point there is: both binaries call one function, and
differ only in where the options came from and what they do with the result.

### 5.3w Eleven ICL files from two IJTAG repositories: nine more gaps

`Honza255/icl_parser` and `bradfordvt/P2654Simulations` were reachable through
`codeload.github.com` even though plain GitHub is blocked. They contain
**eleven `.icl` files**, and — decisively — `icl_parser` ships an **ANTLR
grammar for ICL** (`src/icl_parser/icl.g4`, 440 lines). `Instruments.icl`
states in its header that it holds snippets from **IEEE 1687-2014 Appendix E**,
so at last there are samples that are the standard's own.

Six of eleven files were rejected. Nine gaps, each traced to that evidence:

| Gap | Evidence |
|---|---|
| `Attribute lic = 'h ef931832;` — space after the base | Appendix E, verbatim |
| `Instance aa Of X::x` — namespace-qualified module | Appendix E |
| `NameSpace X;`, bare `NameSpace;`, `UseNameSpace ns;` | Appendix E; g4 makes the name optional |
| `LocalParameter ABC = 4;` | Appendix E |
| `ToIRSelectPort` | `port_def` in g4, used in a file |
| `ScanMux mx[1:0] SelectedBy ...` — a mux with a bit range | g4 `scanMux_name : reg_port_signal_id` |
| `LogicSignal x { \|(~(a), b); }` — reduction op + parenthesised list | g4 `logic_expr_lvl2` |
| `Attribute foo;` — attribute with no value | g4 makes `= value` optional |
| `AddressPort` / `WriteEnPort` / `ReadEnPort` | `port_def` in g4 (no file here uses them) |

All eleven files now round-trip: character conservation and idempotence
verified for each.

**The generated keyword test earned its keep.** After adding the three
statement keywords it failed:

```
[FAIL] every scanner keyword works as an attribute name
    -> drifted: ['NameSpace', 'UseNameSpace', 'LocalParameter']
```

Exactly the bug class it was written for (§5.3z), caught automatically rather
than by review.

**Also confirmed, and deliberately unchanged:** the g4 lexes block comments
as `'/*' .*? '*/'` — non-greedy, so comments do **not** nest, which is what
this formatter does.

### 5.3x Recursion and nesting, checked deliberately

Flagged as worth checking explicitly, so enumerated rather than assumed:

| Kind | Depth verified | Result |
|---|---|---|
| parenthesised expressions | 10 / 50 / 100 / 300 | all fine, 3 ms |
| nested blocks | 10 / 50 / 100 | all fine, 3 ms |
| chained indexes `a[0][1]`, `a[(i)]` | 50 | fine |
| ranges holding expressions `r[a+1:b-1]`, `r[p[0]:0]` | — | fine |
| unterminated block comment / string (10 KB) | — | rejected, no crash |
| 100 KB single line | — | 5 ms |

No stack growth on depth: the parser stack is heap-allocated, so nesting
costs memory, not a fixed-size C stack.

**One real gap found here.** A named nested block could not carry a bit
range:

```
Reg r { Field f[3:0] { ... } }   ← rejected
Reg r { Field f     { ... } }   ← accepted
```

The rule was `attr_name IDENT opt_range '{' ...`, which forces the decision
at `[` — one token too early, and genuinely ambiguous, because
`Source a[3:0];` and `Field f[3:0] {` start identically. Spelled as
`attr_name postfix '{'`, the decision moves to `{` vs `;`, where it is
unambiguous. (Written as `IDENT opt_range` it is a real shift/reduce
conflict, not a stylistic choice.)

**Block comments do not nest, and that is correct.** `/* a /* b */ c */`
ends at the first `*/`, matching C and Verilog. A test case asserting
nesting was written and then deleted — it encoded a wrong expectation.

### 5.3y Two merged Tessent files (2945 and 3431 lines): five gaps

Running the two merged files found five things, four of them the same shape —
**a comma where the grammar expected a single value**:

| Construct | Real form |
|---|---|
| `LogicSignal` body | `a && b;`, `(x && y) \|\| (!z && !w);`, `enable, sib == 2'b11;` |
| attribute value | `Attribute p = "cell_0/A", "cell_1/A";` |
| `SelectedBy` | `SelectedBy config_sib_shift_reg, enable_shift_reg` |
| mux selection | `1'b1, 3'b00x : STEP_COUNTER_REG[0];` |
| mux body | an `Attribute ...;` line among the selection entries |

**The LogicSignal one needed a structural change, not another production.**
`a && b` in an attribute body is genuinely ambiguous: `a` is either an
attribute name followed by a unary value (`ResetValue -1;`) or the start of an
expression. LALR(1) sees only the operator. So LogicSignal got its own body,
where expressions are unrestricted and there are no attribute values to be
ambiguous with.

That still would not compile: LALR **merges** the attribute-body and
expression-body states, so a lookahead legal in one looks legal in the other.
Switching to **IELR** (`%define lr.type ielr`) splits them, and the grammar is
conflict-free with no duplicated expression rules. (The alternative — a second
copy of the expression grammar used only by LogicSignal — would have been
~25 duplicated lines and two things to keep in sync.)

### 5.3a A real Tessent file: two more gaps, one of them a layout bug

`chip_top.txt` is Tessent Shell 2024.2 output — the first genuine Tessent
file this tool has ever been run on. It was rejected on the first try:

```
line 412: syntax error, unexpected ,
    CaptureSource Temp, tdr[2:0];
```

**Gap 1 — a comma-separated value list.** Fixed with `value_list`, and
`CaptureSource`, `ScanInSource` and `Source` all accept it now.

**Gap 2 — a blank line after every wrapped statement.** Found while checking
the fix, and it had nothing to do with commas:

```
input                        output (before)
  CaptureSource a,             CaptureSource a, b;
      b;                    <-- spurious blank line
  }                            }
```

`Append()` advanced `last_source_line_` to the line a node *started* on. A
statement that wraps across source lines renders as one line, so the lines it
consumed looked like gaps before the next node. `Append()` now takes the end
line as well: the gap to the previous node is measured from the start, the gap
to the next one from the end.

Worth noting this is invisible in every hand-written sample: they are all
already one-statement-per-line. Only a file produced by a tool that wraps
long lines reaches it — and Tessent does exactly that.

Both files (`tessent_chip_top`, `wrapped_pmm_ip`) are in the corpus, each
checked for character conservation and idempotence: 415 and 72 source lines,
both round-trip exactly.

### 5.3b Alias is a concatenation list, per the standard BNF

`alias_def : 'Alias' alias_name '=' concat_hier_data_signal ...`, and
`concat_hier_data_signal : '~'? hier_data_signal (',' '~'? hier_data_signal)*`.
So the right side is a *list*:

```
Alias MyName1[2:0] = u1.a[2], u2.u3.b[1:0];
Alias CNTL[42:0]   = ADDR[7:0], ~DATA[31:0], CMD[2:0] { AccessTogether; }
```

Both rejected at the comma. Fixed by giving Alias `value_list`, the same
nonterminal §5.3a added for `CaptureSource` — a second caller, no second
rule.

### 5.3c Three gaps found by running two more published ICL files

The Gitee repository (`git@gitee.com:banana33/test.git`) could not be read —
HTTPS, SSH and the web UI all return 403 at the network egress, so it was not
a permissions problem on the repository side. Two further real files were
found by search instead, and both were rejected:

```
ScanMux mux1 sr { ... }          ← "Reusing and Retargeting On-Chip
                                    Instrument Access Procedures in IEEE
                                    P1687" (2012): the SIB module
ScanInterface { Top.Gateway; }   ← an ASSET/ScanWorks AccessLink example
```

| Gap | Fix |
|---|---|
| `ScanMux` without `SelectedBy` | Tolerated. The keyword *is* required by IEEE 1687.1, but the bare form is how pre-standard drafts wrote it and it survives in published ICL. Same reasoning as tolerating `Reg` and `Field`. |
| A bare qualified name as a statement | `attr_stmt_name : any_keyword \| postfix`, replacing `attr_name ;`. `attr_name` stays narrow (`IDENT \| keyword`) so `attr_name value ;` stays unambiguous; one nonterminal rather than two competing rules, because `attr_name ;` plus `value ;` would reduce a bare identifier two ways. |

That second one is worth spelling out: **`Source a.b.c;` worked but `a.b.c;`
did not.** They look like the same shape, and my samples only ever had the
first.

`fail/scanmux_without_selectedby.icl` was deleted — the code's answer changed
deliberately, so the case was wrong, not the code.

A module-level `Chain` is *rejected*, and a case asserts that: per the BNF,
`scanInterfaceChain_def` only appears inside a `ScanInterface`.

### 5.3d Range bounds are expressions, not a fixed list

`r[-1]` was rejected: range bounds were `NUMBER | PARAMREF`. Adding a third
literal form would have meant a third name and two more productions, and the
next bound shape would have needed a fourth.

Replaced with `range_bound : value`, which already covers literals, parameter
references, hierarchical names and unary expressions. The nonterminal became a
pure alias and was deleted.

Any narrower set eventually turns out to be too narrow — that is the general
form of §5.3b's two findings, and of the operators gap before them.

### 5.3e "Did it change?" is a question about the result, not the front end

`ChangedFrom()` moved into the shared layer. `--check` and `-w` both need it,
and the LSP now uses the same fact to return **no edit at all** for an
already-formatted document, instead of an edit that replaces the text with
itself and marks the buffer dirty for nothing. Three callers, one definition.

### 5.3f Two constructs the standard's BNF requires, and a real module needs

Checking the grammar against IEEE 1687's *formal* syntax (the BNF in the
ScanInterface clause) rather than against prose descriptions:

| Construct | Standard says | Was |
|---|---|---|
| `Chain name { Port ...; DefaultLoadValue ...; }` | `scanInterfaceChain_def` — required when a ScanInterface has more than one scan chain | rejected: `syntax error, unexpected {` |
| `Alias mode[1:0] = DI[6:5];` | the alias name may carry a bit range | rejected: `unexpected [, expecting =` |

Both added; a real instrument module that uses `Alias ... {RefEnum ...;}`
with ranged names now parses, and is in the corpus as
`instrument_with_refenum.icl`.

Worth noting: the named-nested-block form was added once before **on
speculation** and withdrawn for want of evidence. The standard's BNF is that
evidence. Same rule, added twice — the difference is only whether it was
guessed or read.

### 5.5 The regression corpus, and the two cases that had to be deleted

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

### 5.6 A BOM was accepted from a file and rejected from stdin

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

### 5.7 A single integer field could amplify output 50,000x

`textDocument/formatting` honours the client's `options.tabSize`. It is
applied per indent level, so a nested file with `tabSize: 99999` produced a
**7.8 MB reply from a 157-byte input**. No crash — just unbounded memory from
one field. Clamped to 16 (real editors send 2, 4 or 8), with a regression
test asserting the reply stays under 10 KB regardless of what is requested.

### 5.8 A comment bug that only a ragged-indent sample could show

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

### 5.9 Two port functions were missing, found by running a real ICL file

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

### 5.10 The lexer's regular expressions did not match vendor ICL

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

### 5.11 Earlier in the same pass

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

### 5.12 Coverage of the grammar and scanner files

The instruction was: the generated C++ from bison and flex need not be
covered, but every line of the hand-written `.y` and `.l` should be — and the
coverage of my own code should be as high as possible.

Measured with `make gramcov`, using the generators' own traces rather than an
instrumenter written for the purpose: `bison -t` prints
`Reducing stack by rule N (line L)` for each reduction, `flex -d` prints
`--accepting rule at line N` for each token. The set of lines that count as
rules comes from the generated `yyrline[]` and `yy_rule_linenum[]` tables, so
the expected set cannot drift from the grammar.

It started at 127/150 grammar rules and found three kinds of thing:

* **Two productions the standard does not have.** `postfix DOT NUMBER`
  (`a.0`) — the published grammar's `hier_port` is
  `(instance_name '.')+ port_name`, an identifier after every dot, and no file
  in the corpus used it. And `value_no_lead_op : LPAREN value_list RPAREN`,
  duplicating `value : LPAREN value_list RPAREN`. Both deleted; zero conflicts
  afterwards.
* **Constructs with no test.** Comments and assignments inside mux, logic and
  connection bodies; a unary attribute value (`Source ~a;`); keywords used as
  attribute names; a bare `UseNameSpace;`; `Instance i Of M;` with no body.
  All legal — the published grammar allows the semicolon form of `instance_def`
  — so five corpus files were added rather than the rules deleted.
* **One real inconsistency.** `Instance i Of a::b::c;` was rejected while
  `Source a::b::c;` was accepted, because the two positions used different
  rules and only one allowed more than one `::`. `module_ref` is now left
  recursive, like `value`.

Result: 147/147 grammar rules, 81/81 scanner rules.

For the hand-written C++, `make coverage` reports lines and branches
separately, because a line counts as covered once it runs at all, which says
nothing about the other arm of its `if`. Current: 100% of lines, ~92% of
branches. Two branches remain that no test reaches, and both are documented in
the code where they sit.

### 5.13 `make install` prints the editor configuration

The coc.nvim snippet is the only thing a user has to write by hand, and it
differs between the two programs, so it is emitted at the end of every install:
the `languageserver` block for icl-lsp, the `icl-format` command lines, and the
two vimrc lines.

`set filetype=icl` rather than `setfiletype icl`, because vim's own
filetype.vim maps `*.icl` to the Clean language and `setfiletype` silently does
nothing once a filetype is set — which leaves coc never starting the server.
`shiftwidth=2` is included because coc forwards it as LSP `tabSize`; vim
defaults to 8, so without it the editor and the CLI disagree on indentation.

### 5.14 Cross-checking block by block against the published grammar

The request was to work through one syntactic block at a time — finish `Enum`
before moving to `DataOutPort` — rather than auditing the grammar as a whole.
That is what was done: all 55 `*_def` rules in the published grammar were
enumerated and each was checked separately.

Results, in the order the blocks were worked:

* **Every port form.** All 26 ports in `port_def`, each in four shapes —
  `name;`, `name { }`, `name[3:0];`, `name[3:0] { }`. 104 combinations, all
  accepted.
* **The element bodies.** Enum, ScanRegister, DataRegister (selectable /
  addressable / readCallBack / writeCallBack variants), ScanInterface and its
  named `Chain`, the three muxes with comma lists on both sides of `:`, Alias
  with its item forms, Instance with every `instance_item`, the two one-hot
  groups, LogicSignal, NameSpace / UseNameSpace with and without a name,
  Parameter / LocalParameter, Attribute with and without a value, AccessLink
  for both 1149.1 years, ToClockPort with all six item kinds, ResetPort and
  ToResetPort polarity. 41 cases, all accepted.
* **iProc callbacks.** `iProc_namespace` allows an optional leading `::`, and
  `iProc_args` may repeat: `WriteCallBack m::p <D> <R> 5 "s" $X;` — all
  accepted.

Two things came out of it:

* **One real gap: a literal sized by a parameter.** `size : pos_int | '$'
  SCALAR_ID` in the published grammar, so `$W'd5` is how a module
  parameterised on width writes a literal. The scanner only accepted digits
  before the base. Fixed, with a corpus case.
* **One LSP bug found while testing header handling.** `Content-Length` was
  compared exactly, but header names are case-insensitive — so `content-length:`
  made the frame unreadable and the server silently dropped the whole message
  and moved to the next. Now folded to lower case, with an assertion.

One asymmetry was left deliberately. The published grammar allows a leading
`::` after `Of` (global namespace) but not in a signal reference; allowing it
in both was tried and costs three shift/reduce conflicts, because `attr_value`
also permits space-separated values and a bare `::` then has two readings. The
standard's asymmetry is kept, and the reason is recorded in the grammar.

### 5.15 A comment inside a PortGroup list was a syntax error

Found while working through the blocks one at a time, testing each construct
with a comment in every position.

`PortGroup p { a, // c\n b }` was rejected: the list was parsed as
`ident_list`, which is names and commas only. Two things were wrong about
that — a comment can appear wherever whitespace can, and a formatter must not
lose content — so the list now holds names, commas and comments in source
order.

When a comment is present the group is emitted across several lines, because a
`//` comment cannot stay inline without commenting out the closing brace.
Groups without a comment still format on one line, exactly as before.

Two things had to be right for the output to settle, and both are recorded in
the code where they bite:

* **The comma is folded onto the name before it.** Kept as its own list item it
  would have become a line of its own, and the text would not round-trip.
* **Each item carries the span of its own token.** Giving every item the span of
  the whole group was silently wrong in a way only idempotency testing caught:
  the layout pass decides whether a comment is trailing by comparing it with the
  previous node's end line, so with one shared span a comment always looked
  attached to the group. `a, // c` was emitted trailing on the first pass and
  moved to its own line on the second, and never settled.

This construct appears nowhere in the ~7,000 lines of real tool output in the
corpus; it was found by testing rather than by observation.

### 5.16 Block-by-block sweep: comments in every position

The corpus has never contained a comment inside a `PortGroup` list, nor in
several other positions, so each construct was swept separately rather than
relying on the files on hand.

* **14 element blocks** (Enum, ScanRegister, DataRegister, ScanInterface,
  Alias, LogicSignal, Instance, both one-hot groups, the three muxes,
  AccessLink, Module): with and without a range, in both `;` and `{ }` form,
  and with a comment before the block, at the top, in the middle and after an
  item. 0 failures.
* **26 ports × 4 shapes** (with/without range, `;`/`{ }`) plus five comment
  positions = 260 cases. 0 failures.
* **Recursion and nesting**: parentheses at 10/100/300 levels, blocks at
  50/100, 50 chained indices, a range whose bounds are themselves indexed or
  expressions, nested unary operators, `::` chains 20 long, a `Chain` inside a
  `Chain`. Depth costs memory, not C stack: all complete in milliseconds.
* **Regular expressions**: escapes, `//` and `/*` inside strings, empty
  strings, `/*` inside a block comment, `**/`, `/**/`, a space after the base
  (`'h ef931832`), digit separators, a parameter as the width (`$W'd5`), time
  units, BOM, CRLF, tabs. The five malformed shapes (unterminated comment,
  unterminated string, illegal character, stray brace, missing semicolon) are
  correctly rejected rather than silently reformatted.

Two notes on what "failure" means here. The BOM is stripped by design, so it
shows as a difference against the input — that is the intended behaviour, and
the corpus says so explicitly (`bom_stripped_not_kept.icl`). And an early run of
the sweep reported 42 failures that were all the same mistake in the sweep
itself: it generated `ScanMux n { ... }` without the `SelectedBy` clause the
standard requires. The sweep was corrected before any conclusion was drawn.

### 5.17 Cross-checking against the icl_parser repository's tests

`github.com/Honza255/icl_parser` carries an ANTLR grammar (`icl.g4`, 440 lines)
and eight ICL test files. Both were used as a reference.

**The eight ICL files are all already in the corpus** (verified by content
hash, not by name): `Instruments.icl` — whose header says it contains snippets
from IEEE 1687-2014 Appendix E — `scan_mux_001/002`, `test_icl_instance`,
`test_icl_scan_register`, and `test_icl_syntax_5/6/7`. All pass.

**Its Python tests are semantic, not syntactic**: they build a scan graph and
assert the driver of a register, or run cocotb simulations and assert register
values. None of that applies to a formatter, so none of it was ported as-is.
What *was* taken from them is the input space they exercise — in particular
`test_icl_number.py`, which works through `'b11_00` (unsized, with a digit
separator), `x`/`z` digits, and unsized numbers in a concatenation.

Working through `icl.g4` block by block then found two real gaps:

* **A `~` on any element of a signal list was rejected.** Every signal type is
  written `'~'? signal` and the concatenation rules put that per element
  (`concat_data_signal : data_signal (',' data_signal)*`), so both
  `Source ~a, b;` and `Source a, ~b;` are legal. The value list accepted a
  leading operator only in first position. Fixed by letting a comma element
  carry its own operator, and by letting the unary-attribute rule take a list.
* **A leading `::` in an iProc namespace was rejected.** `iProc_namespace` is
  `(namespace_name? '::')? ref_module_name ...`, the prefix optional, so
  `WriteCallBack ::mod p <R>;` is legal. Placed at the *start* of an attribute
  value rather than inside `value_no_lead_op`: there, `a :: b` is genuinely
  ambiguous between one qualified name and two space-separated values, and
  those render differently. At the start of a value there is nothing before it
  to attach to, so there is one reading and no new conflicts.

One deliberate deviation, unchanged from before: `Enum E { }` with an empty
body is accepted here, where `enum_item+` requires at least one. For a
formatter, accepting one more shape costs nothing and rejecting a real file
would cost a great deal; the same reasoning already covers tolerating
draft-era `ScanMux` without `SelectedBy`.

The repository's `benchmarks` submodule (a suite of IEEE 1687 benchmark
networks, on GitLab) could not be fetched — 403.

### 5.17 Cross-check against the ANTLR grammar's test suite

The request was to look at what `Honza255/icl_parser` tests and check whether
we test the same things. The repository's tests are of three kinds, and only
the first is comparable:

* **Eight ICL files** under `tests/test_icls/` (Instruments.icl, which carries
  code from IEEE 1687-2014 Appendix E, plus scan_mux_001/002,
  test_icl_instance, test_icl_scan_register and three benchmark conversions).
  All eight are in our corpus and all eight pass, idempotent and
  character-preserving. Run against them directly, not against our copies.
* **`test_icl_number.py`**, which exercises literal spellings rather than files.
  Every spelling it uses was tried: `'b11_00`, `'B11`, `xx01_x11`, `'d13`,
  `'HE`, `1_2_3`, `0235`, `'d 134`, `'h 23ff`, `'b0101`, `'D 3`, `'b0_1x`,
  `'hx`, `'h3`, `'o7`, `'sd5`, `16'shFF`. All accepted.
* **Semantic tests** — retargeting, scan-graph construction, driver values.
  These have no counterpart here, because a formatter checks structure and
  reproduces text; it does not build a network or simulate it.

Two constructs came out of reading the grammar's rules rather than its tests,
and both were added to the corpus:

* **A mux selection separated by `|`.** The grammar writes `concat_number_list
  : concat_number ('|' concat_number)*`, so `1'b0 | 2'b01 : si_a;` is the
  standard spelling and `,` is the variant. Both are accepted, and neither is
  rewritten into the other.
* **`ScanOutPort` takes an `Enable`**, which the other port blocks do not.
  Our port sweep had covered 26 ports × 4 shapes but not the extra item on
  this one.

Also checked: integer expressions in range bounds (`+ - * / %`, parentheses,
parameter references — `bus[$W - 1:0]`, `[(1 + 2) * 2:0]`), `DefaultLoadValue`
taking an enum symbol, and `RefEnum` on Alias and DataInPort.

One shape is rejected and stays rejected: a parenthesised expression as a
literal's width, `($W+1)'d3`. The grammar's `size` is `pos_int | '$' SCALAR_ID`;
it does not allow one either.

**What could not be obtained.** The benchmark ICL files (BasicSCB, Mingle,
TreeFlat, the Classic set — about twenty files listed in `tests/config.py`) live
in a GitLab submodule, and that host is unreachable from here; there is no
GitHub mirror. That remains the largest gap in coverage: everything above is
the published grammar plus eight files, and a formatter meets the standard by
meeting real output.

### 5.18 Completing the standard's and the manual's partial examples

Publications rarely print a whole ICL file: they show a fragment, with the
rest elided or implied. Fragments cannot be fed to a formatter, so each was
completed into a file, and completing them is what exposed the questions
worth answering.

| Source | Fragment | What completing it required |
|---|---|---|
| IEEE 1687-2014 §6.5.2 | four `Alias` examples | the signals each aliases — `u1.a`, `ADDR/DATA/CMD`, `TDR1[63]` — are declared nowhere in the standard, so a module was written around each |
| Tessent IJTAG manual, parameterised modules | `Module bus`, two `Instance`s | the instances are printed outside any module; `Instance` is module-level, so they were placed in a top module, and `toplevel_datain` declared |
| Tessent IJTAG manual, ICL extraction | `Module block1 { ... DataInPort din { Attribute connection_rule_option = ... } ... }` | the elided ports and registers |
| SemiWiki, IEEE P1687 | `Module TDR` | the instrument's surroundings |
| A Chinese ICL/PDL tutorial | `Module my_instr` with `Connection` | see below |

Two things came out of this that reading the fragments did not show:

**`Instance` outside a module is rejected.** The manual prints the instances
at file scope because it is illustrating them one at a time. In a file they
must sit inside a module. The corpus entry says so, rather than silently
moving them.

**`CaptureEnport CE;` — a misspelled port keyword — is refused at module
level, and that is correct.** The SemiWiki article prints it with a
lower-case p. Module level admits only `attribute_def`, which begins with the
keyword `Attribute`, so a bare `name value;` is not a module item there. The
same line is accepted inside a block, where an attribute name may be any
identifier. The asymmetry is recorded under Known limitations; both
directions are now pinned by cases (`pass/semiwiki_tdr_module.icl`,
`fail/module_level_bare_attribute.icl`).

The tutorial's `DataIn sc;` / `DataOut so;` hit the same rule, and are
exercised where they are legal — inside a block.

### 5.19 The benchmark suite — and a bug in the runner that hid it

`IJTAG-Ecosystem/Benchmarks` is *A Suite of IEEE 1687 Benchmark Networks*, the
reference set the research community uses. Its GitLab mirror is unreachable
from here, but the GitHub copy is not, and it carries **26 ICL files** across
five categories: Basic (BasicSCB, Mingle, TreeFlat, TreeFlat_Ex,
TreeBalanced, TreeUnbalanced), Classic (a586710, p22810, p34392, p93791,
q12710, t512505), ITC02, Standard (CAD, Kernel, MultiCoreAccessLink, MultiTAP,
SOC_DAP), Advanced (FlexScan, TrapOrFlap), plus Instruments.icl,
NetworkStructs.icl and ScanCell.icl.

All 26 pass — idempotent and character-preserving. FlexScan is 25,492 lines
and formats in 0.08 s. This is the largest body of real ICL the formatter has
been checked against, and it is all now in the corpus under
`tests/cases/pass/benchmarks/`.

**Adding them exposed a bug in the test runner itself.** `cases()` listed only
the direct children of `pass/`, so the 26 files sat in a subdirectory and were
never opened — the run reported 151 cases and all green, with the new files
making no contribution at all. It now walks subdirectories and reports names
relative to the folder. Verified by dropping a deliberately broken file into
`benchmarks/` and confirming it is named in the failure, then removing it.

The count is now 177.

Two things worth keeping from reading the benchmark files themselves:

* **They write `BSDLEntity`, where the standard says `BSDL_Entity`.** Both are
  accepted — `BSDLEntity` as an identifier, `BSDL_Entity` as a keyword. Had it
  been pinned to the standard spelling, 2 of the 26 files would fail.
* **34 of the ANTLR grammar's keyword literals are not keywords here** —
  `Source`, `Port`, `ResetValue`, `InputPort`, `RefEnum`, `Enable` and the
  rest are plain identifiers. That is deliberate and it is why typos and
  variant spellings survive, but it has a cost: a genuinely misspelled keyword
  is not reported. Both directions are pinned by cases.

Spellings found in the benchmarks but in no file of ours — a data register
sized by a parameter expression, a one-hot group containing a whole data
register, a mux with two selection signals, an indexed signal as a selector,
a bare integer as a mux selection, a concatenation with no space after the
comma, and `&` in a LogicSignal — are collected into
`pass/benchmark_derived_spellings.icl`, so a regression names the construct
rather than a 25k-line file.

### 5.20 Sorting the corpus by source — and two bugs the sorting exposed

The corpus is now arranged by where each case came from, with a manifest in
`tests/cases/SOURCES.md`:

| directory | origin | files |
|---|---|---|
| `pass/standard/` | IEEE 1687-2014 itself: clause text and Appendix E | 2 |
| `pass/tessent/` | Tessent Shell output and manual fragments, completed | 7 |
| `pass/benchmark/` | IJTAG-Ecosystem/Benchmarks | 26 |
| `pass/upstream/` | the test files of Honza255/icl_parser | 8 |
| `pass/published/` | papers, application notes, articles | 11 |
| `pass/ours/` | written here | 103 |
| `fail/standard/`, `fail/ours/` | must be rejected | 2, 15 |

Upstream and benchmark files were verified byte-identical to their
repositories by content hash, so the directories say more than a comment
could. Three files that were exact duplicates of others were dropped: two
BOM cases that differed only in how they were read (now one case run both
ways), and one that had been copied under two names.

**Sorting them exposed two bugs in the tooling, both of the "reports success
without doing the work" kind.**

`grammar_coverage.py` listed only the direct children of `pass/`, so once the
files moved into subdirectories it found nothing — and printed "no .icl cases
found" only because it happened to check. A version that did not check would
have printed 157/157 on an empty corpus. The `Makefile`'s `dist` target had
the same shape: `cp tests/cases/pass/*.icl` would have shipped an empty
corpus and reported success. Both now walk subdirectories.

**And one in the tool itself.** With every case running through a real file
as well as stdin, `SplitLines` in `format_tree.cc` turned out to loop with
`start <= text.size()` rather than `<`. At `start == text.size()` every
character has been consumed, and one more pass appended the trailing segment
a second time: `"a\n"` split into three entries instead of one. Unreachable
in practice — a comment token never ends in a newline — but it is a real
off-by-one and the branch was flagged as never-taken, which is how it was
found.

Branch coverage rose to 92.8% with it. Two branches that no server response
reaches — the comma inside a JSON array of more than one element, since both
arrays the LSP sends hold at most one entry — are now covered by unit tests
on `Serialize` directly.

### 5.21 Counting, not reading: keyword coverage against the published grammar

Reading a grammar and asking "do we support this?" tends to find what one
already believes. Counting is less polite: it lists what is there and asks what
the corpus never mentions.

So every keyword literal in the ANTLR grammar — 81 of them — was counted
against the whole corpus. Five had **zero** occurrences: four time units
(`fs`, `ps`, `us`, `ms`; only `ns` had ever appeared in a file) and
`STD_1149_1_2013` (only the 2001 revision had been exercised). All five work;
they were simply never written down. They are now pinned by
`pass/standard/time_units_and_2013_accesslink.icl`.

The same was done one level up: for every alternative of every rule, the first
keyword was counted. Zero gaps. And for the 55 `*_def` rules — one per ICL
construct — zero gaps.

**The upstream number test became a file.** `tests/test_icl_number.py` calls
`IclNumber(text, base, width)` directly, so it supplies the base as a argument
rather than in the text — `'bxx` rather than the `("xx", "bin")` the test
writes. Transcribing each of those values into how it would appear in a file
gave `pass/upstream/number_literals_from_upstream.icl`: unsized unknown-digit
literals, a space after the base, an upper-case base, underscore separators,
and a neighbour's `1_2_3`.

**What the upstream repository uses that we cannot have.** Its benchmark
directory is a GitLab submodule (now `Honza255/benchmarks`, formerly
`gitlab.com/IJTAG/benchmarks`), and gitlab.com is unreachable from here — every
archive URL returns 403. That is where `N17D3`, `N32D6`, `N73D14`, `N132D4`,
`NE600P150`, `NE1200P430`, `E30` and `TreeBalanced`'s `H953` live. These are the
large synthetic networks: NE1200P430 carries 1,629 instruments. The 26 files of
`IJTAG-Ecosystem/Benchmarks` on GitHub overlap with that set but do not contain
them. So the Advanced category is represented by FlexScan and TrapOrFlap only.

### 5.22 What the invariants cannot see, and the three tests that fix it

The corpus checks two invariants: formatting is idempotent, and stripping all
whitespace from input and output gives the same string. Both are necessary.
**Neither is sufficient.** A formatter that put every token on one line, or
dropped all indentation, satisfies both — it is idempotent, and the whitespace
comparison still matches. What the invariants cannot see is whether the output
looks right.

Three tests were added, each for one blind spot:

**Golden files** (`tests/golden/`, 12 cases). Input and expected output stored
side by side, compared byte for byte. This is what gofmt, clang-format and
prettier do. `--update` re-records the expectations; that is for after an
intentional change, and the diff it produces is the thing to review, not a way
to silence a failure. The cases cover what the invariants are blind to: two
space indent, blank line collapsing, comments in five positions, star alignment
and ragged stars, PortGroup with a comment, a wrapped statement, a mux table.

**Mutation fuzzing** (`tests/fuzz.py`, 600 mutants, fixed seed). Mutations are
derived from real corpus files rather than generated from scratch, so each
mutant starts from something the grammar accepts — that is where the
boundaries are. The contract is narrow but absolute: exit 0 or 2, never a
signal, never an abort; if accepted, both invariants still hold; if rejected,
the message names a line. A crash would also be a rejection, just an unusable
one, and it is what makes "rejected with a line number" worth saying. It was
verified against a binary that raises SIGSEGV on every input, to confirm it
fails when it should.

**Dead grammar rules now fail the build.** `-Wall -Werror=other` on bison and a
warning check on flex. The grammar carried a dead `ident_list` — left behind
when PortGroup was rewritten to accept comments between names — and the
previous check did not notice, because it only greps for conflicts. flex warns
on "rule cannot be matched", a scanner rule shadowed by an earlier one. Verified
by injecting a dead rule and watching the build fail.

### 5.23 Forks and mirrors: the benchmark submodule does not exist

Checked as suggested. `Honza255/benchmarks` — the repository the submodule
points at — returns **404** from codeload, while the same user's `icl_parser`
returns 200 from the same host, so the URL form is right and the repository is
not there. The only fork, `ranaumarnadeem/icl_parser`, was downloaded
successfully but contains **6** ICL files (fewer than upstream's 8) and none of
the missing networks. Every mirror tried — ghproxy.com, gh-proxy.com,
mirror.ghproxy.com, gh.llkk.cc, gitclone.com, ghfast.top — returns a 403 whose
body is byte-identical to gitlab.com's (`policy_default_denied`, `main-policy`),
so the block is the egress proxy, not the mirror.

The upstream repository's own `USERGUIDE` settles it: `N17D3`, `N32D6`,
`N73D14`, `N132D4`, `NE600P150` and `NE1200P430` are each marked **"TO BE
ADDED"**. They were never published in that repository. It also references
`E30.icl`, which the repository does not contain.

### 5.24 One more gap from the 55 blocks: AllowBroadcastOnScanInterface

Going through the 55 `*_def` rules of the published grammar one at a time,
rather than asking "do we support this?", turned up one construct that was
rejected outright:

```
allowBroadcast_def : 'AllowBroadcastOnScanInterface' scanInterface_name ';' ;
```

The keyword was not in the scanner at all, so it arrived as an identifier and
then failed as a module item. Added as a keyword, a module-level statement, and
a member of `any_keyword` so it can still be an attribute name inside a block —
the same treatment every other keyword gets.

Note the name is a single `SCALAR_ID`, not a list: there is no comma form. So
`AllowBroadcastOnScanInterface si_a, si_b;` is not ICL, and remains rejected.

Two things the same pass confirmed as **correctly rejected**, recorded so they
are not "fixed" later:

* `ScanInterface si;` — `scanInterface_def` requires `'{' item+ '}'`, so a bare
  semicolon form does not exist.
* A `Module` nested inside a `Module` — `module_def` appears only in
  `iclSource_items`, never in `module_item`.

### 5.25 Provenance corrected: three files were misattributed

The P2654 files were in the corpus but under the wrong directories —
`SReg.icl` under `ours/` (it is not ours) and the two `sib_mux_*` files under
`published/` with an `ijtag_` prefix implying the other repository. All three
were also reflowed: same tokens, different line breaks, so a byte comparison
with upstream failed and only a whitespace-insensitive one matched.

They now live in `pass/p2654/`, byte-identical to
`bradfordvt/P2654Simulations/icl/standards/s1687/`, and the reflowed copies are
gone. Byte-identity is what makes a directory name a claim rather than an
assertion.

The second fork of `Honza255/icl_parser` (the repository page shows two) could
not be identified — only `ranaumarnadeem/icl_parser` is reachable, and it has
fewer ICL files than upstream, not more.

### 5.26 The keyword comparison: 27 it defines that we did not

Comparing the two scanners keyword by keyword — the ANTLR grammar's 81 literal
keywords against the 55 rules of our `.l` — left 48 names present there and
absent here. They split into two groups.

**21 are not keywords at all.** Single letters (`A`, `b`, `h`, `x`, `s`, `f`…)
that are lexer fragments in that grammar: the alternatives of `BIN_BASE`,
`DEC_BASE`, `HEX_BASE`, `HEX_DIGIT`, `UNKNOWN_DIGIT` and `SCALAR_ID`. Plus the
seven time units `as fs ps ns us ms s`, which `period_def` spells as separate
literals. Our scanner handles all of them inside the number and identifier
regexes instead — `10ns` is one token, `'h23ff` one token — which is why
`Period 10s;` already worked even though `s` was never a rule of its own.

**27 are real ICL keywords**, and the standard's own BNF confirms the spelling
of several. `accessLink1149_def` reads:

```
'AccessLink' accessLink_name 'Of' ('STD_1149_1_2001' | 'STD_1149_1_2013')
'{' 'BSDLEntity' bsdlEntity_name ';' bsdl_instr_ref+ '}'
bsdl_instr_selected_item : 'ScanInterface' '{' … '}'
                           ('ActiveSignals' '{' … '}') ;
```

Note **BSDLEntity**, with no underscore — the benchmark files use that
spelling, and we had been tolerating it as an identifier. The others:
AccessTogether, ActivePolarity, AddressValue, CaptureSource, Chain,
DefaultLoadValue, DifferentialInvOf, Enable, FreqDivider, FreqMultiplier,
Period, InputPort, Port, ReadCallBack, ReadDataSource, RefEnum, ResetValue,
ScanInSource, Source, WriteCallBack, WriteDataSource, WriteEnSource,
iApplyEndState.

**All 27 already parsed.** That is the honest part: because `attr_name` accepts
any identifier, `Source s;` parsed the same way whether `Source` was a keyword
or not. So this is not 27 bug fixes. What changes is that they are now
*reserved*, which is what the standard says they are.

Making them reserved broke three corpus files immediately:
`AccessLink t Of STD_1149_1_2001` stopped matching, because `module_ref` is
identifiers only. They now have their own group, `std_tap_kw`, used by
`module_ref` — a group rather than an inline alternative, for the same reason
every other keyword family here is a group.

### 5.27 Two unreached rules, and why a strange file was written for them

After the change, grammar coverage was 187/189. The two unreached rules were
the `any_keyword` alternatives for the two STD names — nothing uses them as an
attribute name, because in real ICL they only ever follow `Of`.

The tempting fix is to exempt them, growing the exception list in the generative
keyword test from two entries to four. That is the wrong direction: that list
*is* the drift detector, and every name added to it is a name that can silently
fall out of sync again. So instead both are treated like every other keyword —
one uniform rule — and `std_tap_names_as_attribute_names.icl` pins the uniform
treatment. The file looks odd on purpose, and says why.

## 6. What was verified, and what was not

**Verified**

| Suite | Count | How |
|---|---|---|
| C++ unit tests | 10 | `make unitcheck` — AST and layout, no bison/flex |
| Regression corpus | 124 | `tests/cases/`, run by `tests/regression.py` |
| CLI behaviour | 54 | `tests/run_tests.py` |
| LSP protocol | 27 | scripted client, coc.nvim's message sequence |
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
