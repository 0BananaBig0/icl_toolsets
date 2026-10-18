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
