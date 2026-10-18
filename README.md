# icl-format

Formatter for IEEE 1687-2014 (IJTAG) **Instrument Connectivity Language**.

Built with **flex** and **bison**. Because it parses rather than guesses, it
also acts as a syntax gate: an ICL file that does not conform is *rejected*
with a line number instead of being silently reformatted into something that
looks plausible.

```
source text ──(flex + bison)──▶ AST ──(layout)──▶ format tree ──(render)──▶ text
                                 │
                                 └─ `--ast` prints this
```

## Why three passes

| Pass | Where | Decides |
|---|---|---|
| 1. Abstract syntax tree | `icl_parser.yy`, `icl_ast.h` | *what* each construct is |
| 2. Format tree | `format_tree.cc` | *how it looks*: line breaks, indent levels, blank lines, trailing comments |
| 3. Render | `format_tree.cc` | turns levels into spaces |

Keeping layout out of the parser is what makes the two questions separable.
Two examples of why that matters in ICL:

* `PortGroup tap_side {si, so, en}` is semantically close to
  `PortGroup tap_side { Source si; ... }` but must stay on **one line**.
  That is a visual decision, not a grammar one.
* `} // trailing note` — the comment is the parser's *next* sibling, yet it
  belongs on the closing brace's line.

## Requirements

* C++17 compiler (g++ 9+, clang 10+)
* bison ≥ 3.8, flex ≥ 2.6
* GNU make

### Rocky Linux 8

Rocky 8 ships **bison 3.0.4**, below the 3.8 the grammar is tested against.
The build stops with an explanation rather than failing deep inside bison.
Two ways forward:

```bash
# Option A — build from the pre-generated tarball (needs only g++)
tar xzf icl-format-1.0.0.tar.gz && cd icl-format-1.0.0
make && sudo make install          # or: make install with prefix=$HOME/.local

# Option B — install a newer bison, then build from the git tree
make && make install
```

`make dist` produces that tarball: it bundles the generated parser and
scanner, so the target machine needs no bison at all.

## Build and install

```bash
make                 # builds all four binaries (see below)
make dynamic         # only the dynamically linked pair (no static libc needed)
make check           # unit tests (10) + CLI suite (121) + LSP suite (25)
make unitcheck       # unit tests only: AST + layout, no bison/flex needed
make nvimcheck       # drive icl-lsp from neovim's built-in LSP client (needs nvim)
make coccheck        # the full vim -> coc.nvim -> icl-lsp chain (see below)
make install         # same as install-dynamic
make install-dynamic # smaller; needs the host's libstdc++/glibc
make install-static  # self-contained; runs on any Linux regardless of glibc
make uninstall       # removes exactly what install added
```

`make` produces two programs, each in a dynamically linked and a fully static
flavour:

| Binary | Links | What it is |
|---|---|---|
| `icl-format` | dynamic | command line formatter |
| `icl-format-static` | static | same, runs on any Linux regardless of glibc |
| `icl-lsp` | dynamic | Language Server for editors |
| `icl-lsp-static` | static | same |

Both programs link the **same parser front end**, so a grammar fix reaches the
CLI and the editor at once and neither can drift from the other.

### Choosing a build at install time

`make` builds both flavours; installing is a separate decision. Both targets
install the programs under the **same names**, so an editor config that says
`"command": "icl-lsp"` works either way — pick the one that suits the machine:

| Target | Size | Needs |
|---|---|---|
| `make install-dynamic` | ~172 KB + ~231 KB | the host's libstdc++ and glibc |
| `make install-static` | ~2.6 MB + ~2.7 MB | nothing — runs on any Linux |

On a machine you control, dynamic is enough. Use static when the binary has to
run somewhere whose glibc differs from where it was built (Rocky 8 hosts
building for Rocky 9, a container, a tool box).

Install layout (GNU conventions, default `prefix = $(HOME)/.local`):

```
$(prefix)/bin/icl-format
$(prefix)/bin/icl-lsp
$(prefix)/include/icl-format/{icl_ast.h,icl_driver.h,format_tree.h,json.h}
$(prefix)/share/doc/icl-format/
$(prefix)/share/man/man1/icl-format.1
```

Override with `make install prefix=/opt/tools`.

`make install` writes `build/install_manifest.txt`; `make uninstall` deletes
exactly those paths — it never touches files it did not install.

Every `make install` prints the coc.nvim configuration when it finishes —
the `languageserver` block for icl-lsp, the two vimrc lines that make `*.icl`
an `icl` filetype, and the icl-format command lines. See
"Editor integration" below for the reasoning behind each one.

## Usage

```bash
icl-format top.icl              # print formatted output
icl-format -w top.icl           # rewrite in place
icl-format --check *.icl        # CI gate: list files that would change
icl-format -n 4 top.icl         # 4-space indent
icl-format --ast top.icl        # dump the abstract syntax tree
icl-format < top.icl            # stdin (editor integration)
```

Exit codes:

| Code | Meaning |
|---|---|
| 0 | success (with `--check`: everything already formatted) |
| 1 | `--check` found at least one file that would change |
| 2 | syntax error, unreadable file, or bad command line |

Only files whose content actually changes are rewritten, so mtimes (and build
systems) stay quiet on an already-formatted tree.

## Editor integration: icl-lsp

`icl-lsp` is a Language Server speaking LSP over stdio. It offers two features,
both derived from the same front end the CLI uses:

| Feature | How |
|---|---|
| Syntax diagnostics | `FormatText` fails → publish one diagnostic at the line the parser stopped on |
| Whole-document formatting | `FormatText` succeeds → one `TextEdit` covering the document |

There is no second parser. `options.tabSize` from the client becomes the
formatter's indent width, so the editor's setting and `icl-format -n` are the
same knob — set `shiftwidth=2` to match the CLI default.

A file with a syntax error is **left untouched**: the server returns no edits
rather than a best-effort reformat.

### coc.nvim

`make coccheck` runs the whole chain against a real editor — nothing is
simulated. Verified with vim 9.1 + coc.nvim 0.0.82: vim loads coc, coc starts
its own node server, that server reads `coc-settings.json` and launches
`icl-lsp`, and diagnostics plus formatting come back through coc's API.

```bash
tests/setup_coc.sh          # fetches coc.nvim and builds vim 9 if needed
make coccheck
```

It is a separate target rather than part of `make check` because it needs
coc.nvim and vim 9, neither of which the build otherwise requires.

Three things that are easy to get wrong, all found by actually running it:

**1. coc.nvim needs vim 9 or newer.** `autoload/coc/api.vim` is written in
vim9script and uses `:export`; vim 8.2 rejects it with
`E1043: Invalid command after :export`, after which coc starts its node
process but never finishes its handshake and `CocAction` throws
`coc.nvim not ready`. Nothing looks broken until you check
`g:coc_service_initialized`. Neovim is unaffected (coc takes a different code
path there).

**2. `setfiletype` is not enough for `*.icl`.** vim's own `filetype.vim` maps
`*.icl` to `clean` (the Clean programming language), and `setfiletype` is a
no-op once a filetype is set — so the gentle form silently loses and no
language server is ever started. Use the overriding form:

```vim
autocmd BufNewFile,BufRead *.icl set filetype=icl
autocmd FileType icl setlocal shiftwidth=2 tabstop=2 expandtab
```

**3. Indent width comes from vim.** coc forwards the buffer's `shiftwidth` as
the LSP `tabSize`, and vim's default is 8 — so without the `shiftwidth=2` line
above you get 8-space indent in the editor and 2-space from the CLI. Same knob,
two front ends; set it to keep them in agreement.

Configuration, in `:CocConfig` (`~/.vim/coc-settings.json`):

```json
{
  "languageserver": {
    "icl": {
      "command": "icl-lsp",
      "filetypes": ["icl"],
      "settings": {}
    }
  }
}
```

Then `:call CocAction('format')` reformats the buffer, and diagnostics appear
inline. To format on save:

```vim
autocmd FileType icl autocmd BufWritePre <buffer> call CocAction('format')
```

If the server does not start, `:CocCommand workspace.showOutput` shows the
channel log; a useful first check is `:echo CocAction('services')`, which
should list `languageserver.icl` with `state: "running"`.

### Other clients

Any client that can launch a stdio command works. The advertised capabilities
are `textDocumentSync: 1` (full) and `documentFormattingProvider: true`;
everything else is answered with `-32601`, and unknown notifications are
ignored, so a client probing for features the server lacks stays healthy.

## Grammar design: strict where it counts

The **structural** level is strict, the **attribute** level lenient.

*Strict* — Module, the 13 port kinds, `ScanRegister` / `Reg` / `Field`,
`Instance x Of Y`, `ScanMux` selection tables, `Connection a -> b;`,
`PortGroup` comma lists, `AccessLink` sub-structures. These are what make an
ICL file a network description, and rejecting a non-conforming file is the
point of the tool.

*Lenient* — inside a block an attribute is `NAME ...;` where `NAME` may be any
identifier **or** any ICL keyword. Real ICL writes `Attribute d = "x";` inside
a `Field` block and `DataInPort DI = 8'h00;` inside an `Instance` block, so the
attribute name is a keyword there. Encoding every attribute form rigidly would
reject valid files, and being lenient here loses nothing: attribute spelling
affects neither the network topology nor the layout.

## Style

Follows the [Google C++ Style Guide](https://google.github.io/styleguide/cppguide.html):
2-space indent, `snake_case` variables, `PascalCase` types, `kConstant`
enumerators. ICL output defaults to 2 spaces (`-n` to change).

## Design notes

The AST has exactly three node shapes: comment, one-line statement, and
`header { body }` block. Module, Port and PortGroup started as separate
classes, but each was just a header plus a body (or a one-line text) and
nothing downstream distinguishes them, so they collapsed into the general
shapes. Same for the four-value `FormatKind` enum (only "blank separator" was
ever read) and the `FormatTree` wrapper around a plain vector.

Beyond style, a few decisions are load-bearing:

* **Dispatch is by static type.** Every traversal is an `AstVisitor`, one
  overload per node class. It used to switch on a kind tag and down-cast,
  which crashed: `Alias`, `ScanRegister`, `Reg` and `Field` each have both a
  one-line form (`Alias a = b;`) and a block form (`Alias a = b { ... }`), so
  no tag value identifies a class. With a visitor, adding a node class is a
  compile error until it is handled — no runtime cast can be wrong.
* **No global mutable state, except the scanner's line counter.** Parser
  diagnostics live on `IclDriver` (bison's `%param` puts it in scope). Flex's
  location is file-scope because the scanner is not reentrant; it is reset on
  every `icl_scan_bytes`, which is what keeps error line numbers correct when
  several files are processed in one run.
* **Nodes are non-copyable** and owned by an `AstUnit` arena; children hold
  non-owning pointers. Copying would break ownership and slice derived parts.
* **AST and layout take plain line numbers**, not Bison's location type, so
  `make unitcheck` compiles and runs them without generating a parser.
* **No exceptions** (Google style): errors are `bool` + out-param.
* **RAII throughout**; verified clean under ASan + UBSan with leak detection.

## What is preserved

Blank lines (capped at one), trailing comments, multi-line block comments
including star alignment, and every non-whitespace character — the test suite
asserts `f(f(x)) == f(x)` and that stripping all whitespace yields identical
text before and after.

## Deliberate shortcuts

Each is marked `ponytail:` in the source, with its upgrade path. Run
`grep -rn "ponytail:" src/ Makefile` for the full ledger.

* **The scanner keeps its location in a file-scope variable**, so it is neither
  reentrant nor thread-safe. Fine for one-file-per-process; needs
  `%option reentrant` if this ever becomes a library or language server.
* **Attribute *names* are not validated** — any keyword is accepted inside a
  block. Structure is checked, spelling is not.
* **`-static`** is safe only because the tool uses no NSS, no `dlopen` and no
  locale APIs.

## Coverage

Three different things are measured, because they fail differently.

| what | how | current |
|---|---|---|
| lines of hand-written C++ | `make coverage` (gcov) | 100% |
| branches of hand-written C++ | `make coverage` (gcov) | ~92% |
| rules of `icl_parser.yy` | `make gramcov` (bison `-t`) | 152/152 |
| rules of `icl_lexer.l` | `make gramcov` (flex `-d`) | 82/82 |

`make coverage` instruments the sources with gcov and runs every suite against
the instrumented binaries, including the C++ unit tests, so their counts merge
with the end-to-end runs. It fails below `--min-line 100 --min-branch 90`.

`make gramcov` measures the *hand-written* grammar and scanner rather than the
generated C++, which is machine-written and whose line numbers nobody can act
on. It uses the generators' own traces instead of an instrumenter of ours:
`bison -t` prints `Reducing stack by rule N (line L)` and `flex -d` prints
`--accepting rule at line N`, both naming a line in the `.yy`/`.l`. The set of
lines that are rules at all is read back out of the generated tables
(`yyrline[]`, `yy_rule_linenum[]`), so the report cannot drift from the
grammar it measures.

Both are part of `make check`.

## Code graph

`make codegraph` refreshes `docs/codegraph/`:

| File | What | Produced by |
|---|---|---|
| `tests_codegraph.html` / `.csv` | Python test helpers | `codegraph` (pip) |
| `cpp_deps.makefile` | raw header dependencies | `g++ -MM` |
| `cpp_deps.dot` / `.csv` / `.svg` | C++ dependency graph | from the above |

`codegraph` parses with Python's tokenizer, so it covers only the Python half;
the C++ half comes from the compiler. See REQUIREMENT.md §5.3u.

## Known limitations

Full requirement-by-requirement accounting, including what could not be done
as asked and what was substituted, is in [`REQUIREMENT.md`](REQUIREMENT.md).

* **Real Tessent output was only available late.** `tests/cases/pass/`
  includes `tessent_chip_top.icl` (Tessent Shell 2024.2) and
  `wrapped_pmm_ip.icl`; both revealed grammar gaps that no hand-written
  sample had. See `REQUIREMENT.md` §5.3a.

* **Grammar coverage comes from the standard plus the samples available**, not
  from a corpus of real Tessent output. The element list is IEEE 1687's
  Table 2 (IJTAG Tutorial, 3rd ed.) and the port functions are Tessent's
  Table 3-14; `REQUIREMENT.md` §5 lists what each cross-check found missing
  (five elements, two port functions, and three lexer patterns).
  `samples/keysight_gpio.icl` is a real third-party ICL file and is the
  strongest sample in the tree. Two gaps were found and fixed exactly
  this way: `ScanMux` requires the standard `SelectedBy` keyword (the samples
  had been written against a pre-standard draft that omitted it), and eight
  port functions from the client/host split (`TRSTPort`, `ToTCKPort`,
  `ToTRSTPort`, `ToShiftEnPort`, ...) were missing entirely. A file using an
  uncovered construct is *rejected with a line number* rather than silently
  mangled — if that happens, the report is enough to add the missing rule.
  A real vendor file is still the strongest test; see `REQUIREMENT.md`.
* Attribute bodies are parsed leniently, so a misspelled *attribute name* is
  not an error — only structure is checked.
* Column-level layout (aligning `=` across neighbouring lines) is not done.
* **A comment cannot appear in the middle of a statement.**
  `ScanInPort /* mid */ SI;` and `Module m /* c */ {` are rejected. Comments
  before a statement, after one, and inside a block all work. Accepting them
  inside one would mean adding an optional COMMENT to nearly every production
  — a large change for a rare spelling.

## License

Apache License 2.0.
