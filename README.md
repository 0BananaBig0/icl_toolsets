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

* A legacy `PortGroup tap_side {si, so, en}` list stays on one line,
  while an embedded line comment requires multiple lines.
* `} // trailing note` — the comment is the parser's *next* sibling, yet it
  belongs on the closing brace's line.

## Requirements

* C++17 compiler (g++ 9+, clang 10+)
* bison ≥ 3.8, flex ≥ 2.6
* GNU make ≥ 4.3 (grouped Bison/Flex outputs)

### Rocky Linux 8

Rocky 8 ships **bison 3.0.4**, below the 3.8 the grammar is tested against.
The build stops with an explanation rather than failing deep inside bison.
Two ways forward:

```bash
# Option A — build from the pre-generated tarball (needs only g++)
tar xzf icl-format-1.0.0.tar.gz && cd icl-format-1.0.0
make dynamic && make install-dynamic

# Option B — install a newer bison, then build from the git tree
make dynamic && make install-dynamic
```

`make dist` produces that tarball: it bundles the generated parser and
scanner, so the target machine needs no bison at all.

## Build and install

```bash
make                 # builds all four binaries (see below)
make dynamic         # only the dynamically linked pair (no static libc needed)
make check-dynamic   # full dynamic regression, including grammar traces
make check           # same checks, plus static build prerequisites
make unitcheck       # AST/layout/JSON unit checks, no bison/flex needed
make nvimcheck       # drive icl-lsp from neovim's built-in LSP client (needs nvim)
make coccheck        # the full vim -> coc.nvim -> icl-lsp chain (see below)
make install         # same as install-dynamic
make install-dynamic # smaller; needs the host's libstdc++/glibc
make install-static  # needs static runtime libraries at build time
make uninstall       # removes exactly what install added
```

`make` produces two programs, each in a dynamically linked and a fully static
flavour:

| Binary | Links | What it is |
|---|---|---|
| `icl-format` | dynamic | command line formatter |
| `icl-format-static` | static | same, includes the C/C++ runtime |
| `icl-lsp` | dynamic | Language Server for editors |
| `icl-lsp-static` | static | same |

Both programs share `ParsedDocument`, its AST definition, validation and formatting
pipeline. LSP formatting reuses the cached parse; only CLI I/O and protocol/editor
services diverge. See [verification and measured limits](docs/icl-verification.md).

### Choosing a build at install time

`make` builds both flavours; installing is a separate decision. Both targets
install the programs under the **same names**, so an editor config that says
`"command": "icl-lsp"` works either way — pick the one that suits the machine:

| Target | Needs |
|---|---|
| `make install-dynamic` | The host's compatible libstdc++ and glibc |
| `make install-static` | Static C/C++ libraries while building |

Static binaries still require a compatible architecture and Linux kernel. The
current host lacks static libc/libm/libstdc++; dynamic builds are verified.

Install layout (GNU conventions, default `prefix = $(HOME)/.local`):

```
$(prefix)/bin/icl-format
$(prefix)/bin/icl-lsp
$(prefix)/include/icl-format/{icl_ast.h,icl_driver.h,format_tree.h,format_job.h,icl_semantics.h,json.h}
$(prefix)/share/doc/icl-format/
$(prefix)/share/man/man1/icl-format.1
```

Override with `make install prefix=/opt/tools`.

`make install` writes `build/install_manifest.txt`; `make uninstall` deletes
the recorded paths. Installation replaces same-named targets under the selected
prefix; use a dedicated prefix when keeping multiple versions.

Every `make install` prints the coc.nvim configuration when it finishes —
the `languageserver` block for icl-lsp, the two vimrc lines that make `*.icl`
an `icl` filetype, and an optional coc-diagnostic adapter for icl-format. See
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

`icl-lsp` speaks LSP over stdio. Both executables consume `ParsedDocument`,
which owns one Flex/Bison parse, the shared AST, exact source token spans and
scoped declarations/references. The LSP caches that document until its text changes;
formatting renders the cached AST with the requested indentation.

| Feature | Behavior |
|---|---|
| Diagnostics and formatting | Shared parser diagnostics; invalid input produces no formatting edits. |
| Completion | ICL keywords, module-local names, instance members and module types in the selected namespace. |
| Definition / declaration | Parameters, ports, registers, aliases, enum members and module/instance references, with exact UTF-16 protocol positions. |
| Implementation | The referenced ICL Module body for an instance/module. No inference of HDL implementations. |
| Hover / references | Declaration source and resolved references, respecting modules, namespaces and instance targets. |
| Document / workspace symbols | Parsed declarations, including unopened `.icl` files under workspace roots. |

Workspace discovery skips hidden directories and `build/`, and does not follow
symlink directories. Open buffers override disk content. Clients supporting dynamic
file-watch registration receive an `**/*.icl` watcher; create/change/delete events
refresh unopened files. Full document synchronization is advertised. Completed
statements in an incomplete Module remain available for completion, while stale
symbols from the previous version are discarded. Ambiguous cross-file module
redefinitions return no guessed target. Rename is not advertised.

`options.tabSize` uses the same clamping as `icl-format -n`. Set `shiftwidth=2`
to match the CLI default. No existing editor configuration is modified.

### coc.nvim

`make coccheck` runs the whole chain against a real editor — nothing is
simulated. Verified with vim 9.2 + coc.nvim 0.0.82: vim loads coc, coc starts
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
autocmd BufWritePre *.icl call CocAction('format')
```

If the server does not start, `:CocCommand workspace.showOutput` shows the
channel log; a useful first check is `:echo CocAction('services')`, which
should list `languageserver.icl` with `state: "running"`.

### Optional CLI formatter through coc.nvim

Use [coc-cli.json](docs/editor/coc-cli.json) after `:CocInstall coc-diagnostic`
if formatting must launch `icl-format`. It keeps `icl-lsp` for language services
and disables its formatting provider to avoid competing formatters. The default
[LSP configuration](docs/editor/coc-lsp.json) needs no additional extension and
reuses the cached AST.

The CLI adapter passes buffer text on stdin and uses stdout only on success;
`ignoreExitCode: false` preserves invalid input. Its keys follow the upstream
[coc-diagnostic configuration](https://github.com/iamcco/coc-diagnostic#config)
and [formatter schema](https://github.com/iamcco/diagnostic-languageserver#formatters).
The real Vim/coc/LSP chain was tested; the optional coc-diagnostic extension was
not installed or tested end to end. Keep both executables on Vim's PATH, or use
absolute command paths. Do not use `:%!icl-format`: a failed shell filter can
replace the buffer with diagnostics.

### Other clients

Any client supporting stdio LSP and full document synchronization can use the
services listed above. Unsupported requests receive `-32601`; unknown
notifications are ignored. JSON nesting is limited to 256, frame headers to
8192 bytes and payloads to 64 MiB. Invalid framing terminates the stream; invalid
JSON in a correctly framed message is rejected without losing the next frame.

## Grammar and compatibility

Each IEEE 1687 ICL construct has a dedicated grammar or shares only identical
property forms. Attributes use `Attribute name [= value];`; arbitrary properties
and nested blocks are rejected. ICL keywords may be identifiers in name positions.
Parameters and attributes carry shared AST metadata for local name/type checks.

The source-backed compatibility forms and their limits are recorded in
[the conformance review](docs/icl-conformance-review.md). They include legacy
P1687 PortGroup lists, omitted ScanMux SelectedBy and ScanOutPort LaunchEdge,
Origen includes/lowercase `of`, documented Tessent extensions, and empty Alias
bodies observed in the user's merged corpus. Reg, Field and Connection are not
supported declarations. ICL and PDL are separate languages; no PDL parser exists.

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
* **The scanner is not reentrant.** Parser
  diagnostics live on `IclDriver` (bison's `%param` puts it in scope). Flex's
  location is file-scope because the scanner is not reentrant; it is reset on
  every `icl_scan_bytes`, which is what keeps error line numbers correct when
  several files are processed in one run.
* **Nodes are non-copyable** and owned by an `AstUnit` arena; children hold
  non-owning pointers. Copying would break ownership and slice derived parts.
* **AST and layout use generator-independent source spans**, not Bison's location type, so
  `make unitcheck` compiles and runs them without generating a parser.
* **Parse errors use explicit results.** C++ exceptions remain enabled; gcov
  includes compiler-generated exception/cleanup edges in its raw branch totals.
* **RAII owns AST nodes and cached documents.**

## What is preserved

Blank lines (capped at one), trailing comments, multi-line block comments
including star alignment, and every non-whitespace character — the test suite
asserts `f(f(x)) == f(x)` and that stripping all whitespace yields identical
text before and after.

## Limits and coverage

The scanner has file-scope state; the CLI and LSP parse serially. Concurrent
parsing would require a reentrant scanner. Local validation is not full hardware
elaboration: parameter-dependent widths, cross-instance connectivity and HDL
implementation inference are not certified. Generic AccessLink payloads remain
opaque. ICL and PDL do not share lexers, grammar or token sets.

See [the final verification report](docs/icl-verification.md) for actual counts,
remaining uncovered paths, compiler artifacts and performance measurements.
`make gramcov` reaches **334/334 Bison productions and 119/119 Flex rules**.
This does not imply every semantic-action branch ran. `make coverage` reports
handwritten `.cc`, `.h`, `.yy` and `.l` lines/branches, excluding generated
parser/scanner implementation files. Its unchanged **100% line / 90% branch**
threshold is currently **not met**; older 100%/92.6% claims used incomplete counting.

`make check-dynamic` includes rule coverage and functional regression;
`make coverage` is a separate strict gate. Coverage-reader tests cover zero and
starred counts, never-executed branches, missing reports and repeated source lines.
Grammar coverage counts distinct production IDs, including multiple productions
on one source line.

## Code graph

`make codegraph` refreshes `.codegraph/`:

| File | What | Produced by |
|---|---|---|
| `tests_codegraph.html` / `.csv` | Python test helpers | `codegraph` (pip) |
| `cpp_deps.makefile` | raw header dependencies | `g++ -MM` |
| `cpp_deps.dot` / `.csv` / `.svg` | C++ dependency graph | from the above |

`codegraph` parses with Python's tokenizer, so it covers only the Python half;
the C++ half comes from the compiler. See REQUIREMENT.md §5.3u.

* **A string literal may not span a line.** The published scanner rule is
  `'"' (~('"'|'\\')|'\\\\'|'\\"')* '"'`, where `~('"'|'\\')` includes a
  newline — so a string crossing a line break is legal to it. We reject it,
  because a formatter that accepted one would have to reproduce the break
  exactly, and no real file in the corpus has one. Rejected with a line
  number, not silently reformatted.

* **A bare `name value;` statement is a block item, not a module item.**
  Module level admits only `attribute_def`, which begins with the keyword
  `Attribute`; inside a block an attribute name may be any keyword or
  identifier, so `Width 8;` is accepted there and rejected at module level.
  This matters because a misspelled port keyword — `CaptureEnport CE;` for
  `CaptureEnPort CE;` — is exactly such a statement, and is refused with a
  line number rather than silently reformatted.

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
  `tests/cases/pass/published/keysight_gpio.icl` is a real third-party ICL
  file and is the
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

MIT License. See [LICENSE](LICENSE).

Third-party test fixtures retain their original copyright and license notices;
see [test sources](tests/cases/SOURCES.md) and accompanying license files.
