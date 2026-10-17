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
make                 # builds ./build/icl-format
make check           # unit tests (10) + end-to-end suite (107)
make unitcheck       # unit tests only: AST + layout, no bison/flex needed
make install         # installs into $HOME/.local
make uninstall       # removes exactly what install added
make static          # fully static binary (runs on any Linux, no glibc req.)
```

Install layout (GNU conventions, default `prefix = $(HOME)/.local`):

```
$(prefix)/bin/icl-format
$(prefix)/include/icl-format/{icl_ast.h,icl_driver.h,format_tree.h}
$(prefix)/share/doc/icl-format/
$(prefix)/share/man/man1/icl-format.1
```

Override with `make install prefix=/opt/tools`.

`make install` writes `build/install_manifest.txt`; `make uninstall` deletes
exactly those paths — it never touches files it did not install.

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
`header { body }` block. Module, Port and PortGroup used to be separate
classes, but each was just a header plus a body (or a one-line text), and
nothing downstream distinguishes them — so they collapsed into the general
shapes. Same for the four-value `FormatKind` enum (only "blank separator" was
ever read) and the `FormatTree` wrapper around a plain vector.

Beyond style, a few decisions are load-bearing:

* **Dispatch is by static type, never by `AstKind`.** Every traversal is an
  `AstVisitor`. `AstKind` exists for diagnostics and `--ast` only. It is *not*
  a 1:1 map onto the C++ classes: `Alias`, `ScanRegister`, `Reg` and `Field`
  each have both a one-line form (`Alias a = b;`) and a block form
  (`Alias a = b { ... }`), which are different classes sharing one kind value.
  Switching on kind and down-casting crashed on exactly those; with a visitor,
  adding a node class is a compile error until it is handled.
* **No global mutable state.** Parser diagnostics live on `IclDriver` (bison's
  `%param` puts it in scope). The only file-scope variable left is the flex
  scanner's location, which is inherent to a non-reentrant scanner.
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

## Known limitations

* The grammar is validated against the two files in `samples/`. Vendor ICL
  (Tessent output in particular) may contain constructs not yet covered; such
  a file is *rejected with a line number* rather than mis-formatted. Send the
  message and the construct gets added.
* Attribute bodies are parsed leniently, so a misspelled *attribute name* is
  not an error — only structure is checked.
* Column-level layout (aligning `=` across neighbouring lines) is not done.

## License

Apache License 2.0.
