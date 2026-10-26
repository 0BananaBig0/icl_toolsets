# icl_toolsets

Format, inspect, and navigate IEEE 1687-2014 ICL and PDL in the terminal,
Vim, or Neovim.

| Tool | Purpose |
|---|---|
| `icl-format` | Format ICL files or standard input; check formatting in CI. |
| `icl-lsp` | ICL diagnostics, completion, navigation, symbols, and editor formatting. |
| `pdl-format` | Format PDL while respecting Tcl scripts, data, and substitutions. |
| `pdl-lsp` | PDL diagnostics, completion, navigation, symbols, and editor formatting. |
| `vim-icl-pdl-modern` | Standalone ICL/PDL highlighting and filetype detection. |

ICL (**Instrument Connectivity Language**) and PDL (**Procedural Description
Language**) are separate languages. Each language's formatter and language server
share a frontend and formatting engine. The editor tools parse source without
executing PDL/Tcl or generating patterns.

Install the executables for formatting and language services. For highlighting
alone, [install the Vim plugin](#highlighting-only) without building anything.
The proposed `icl-pattern-generation` executable is [still under development](#pattern-generation-status).

## Build and install

On Linux, the build requires:

- A C++17 compiler and standard library with `std::filesystem` support.
- GNU Make 4.3+, Bison 3.8+, and Flex 2.6+.
- For PDL: `pkg-config` and Tcl development headers/libraries, version 8.6 or
  newer. A Tcl stub library alone is insufficient.

```sh
git clone https://gitee.com/banana33/icl_toolsets.git
cd icl_toolsets
make dynamic
make install-dynamic
export PATH="$HOME/.local/bin:$PATH"

icl-format --version
icl-lsp --version
pdl-format --version
pdl-lsp --version
```

`make dynamic` builds all four commands in `build/`. Installation puts them in
`~/.local/bin` and prints coc.nvim configuration instructions; it does not modify
editor settings or install the highlighting plugin. All four commands support
`--help`, `-h`, and `--version`, which print information and exit.

Keep the installation directory on your editor's `PATH`, or use absolute server
paths. To install elsewhere:

```sh
make install-dynamic prefix="$HOME/opt/icl_toolsets"
```

Dynamic binaries require compatible C/C++ runtime libraries and, for PDL,
the matching Tcl shared library on the target machine. Use
`BISON=/path/to/bison` when selecting a newer Bison installation.

### Build variants

| Command | Result |
|---|---|
| `make dynamic` | All four dynamic binaries. |
| `make static` | All four static binaries, named with a `-static` suffix. |
| `make icl-dynamic` / `make icl-static` | Only the ICL pair; no Tcl dependency. |
| `make pdl-dynamic` / `make pdl-static` | Only the PDL pair. |
| `make` | Both variants of all four tools. |

Static builds also need static C/C++ runtime libraries and a real Tcl archive
compatible with the selected Tcl headers:

```sh
make static TCL_STATIC_LIB=/path/to/libtcl9.0.a
make install-static TCL_STATIC_LIB=/path/to/libtcl9.0.a
```

Both install variants use the normal four command names. Installing a different
variant into the same prefix replaces the previous installation. `make install`
selects the dynamic variant. See the [PDL build guide](docs/pdl-tools.md) for Tcl
build instructions and static Tcl/glibc portability limits; verify binaries on
the intended target host.

Use `make uninstall` with the same `prefix` to remove installed tools. Retain
`build/install_manifest.txt` so uninstall can identify installed files.

### Source archives

Archives created by `make dist` include generated parser/scanner sources and can
build without Bison or Flex. Automatic source downloads from GitHub/Gitee contain
tracked sources and require those generators. Generated build artifacts are not
committed to the repository.

The archive package name remains `icl-format`, despite the repository name:

```sh
tar xzf icl-format-1.0.0.tar.gz
cd icl-format-1.0.0
make dynamic
make install-dynamic
```

Compiler, Make, and Tcl requirements apply to both archive types.

## Format files

Both formatters accept the same options:

```sh
icl-format top.icl                   # print formatted source
icl-format -w top.icl                # rewrite in place
icl-format -w block_a.icl block_b.icl # format several files
icl-format --check *.icl             # check without rewriting
icl-format --indent 4 top.icl        # four-space indentation
icl-format -o formatted.icl top.icl  # write a separate file
icl-format < top.icl                 # read standard input

pdl-format procedures.pdl
pdl-format -w procedures.pdl
pdl-format --check *.pdl
pdl-format --indent 4 < procedures.pdl
```

CLI indentation defaults to two spaces, with a maximum of 16. The CLI does not
read vimrc or `.clang-format`. Use [LSP formatting](#vimneovim-with-cocnvim) to
reuse editor indentation settings without specifying `--indent`.

ICL formatting preserves comments and strings, normalizes spacing and
indentation, and retains at most one consecutive blank line. PDL formatting
indents recognized braced script bodies, including `iProc`, `proc`, and Tcl
control flow. It preserves the spelling of data words and substitutions; quoted
or dynamic script bodies are not reflowed. Most PDL blank lines are removed.

Invalid inputs receive diagnostics and are not rewritten. With several inputs,
valid files can still be processed if another fails. A second formatting pass
leaves the first pass's output unchanged; `-w` also preserves the modification
time of an unchanged file.

| Exit code | Meaning |
|---|---|
| `0` | Success; with `--check`, all files are already formatted. |
| `1` | `--check` found formatting changes. |
| `2` | Invalid source, a detected local semantic error, an I/O error, or invalid arguments. |

File writes use a sibling temporary file, so the destination directory must be
writable. Existing symlink targets, ownership, and permission bits are preserved;
dangling symlink outputs are rejected. Other hard links retain their original
contents. Replacement does not preserve extended attributes or ACLs.

## Vim/Neovim with coc.nvim

Install [coc.nvim](https://github.com/neoclide/coc.nvim) and its editor/Node.js
requirements. Make the language servers available on the editor's `PATH`.

Open `:CocConfig` and merge these entries into the existing `languageserver`
object, alongside any other servers such as `verilog`:

```json
{
  "languageserver": {
    "icl": {
      "command": "icl-lsp",
      "filetypes": ["icl"],
      "rootPatterns": [".git"]
    },
    "pdl": {
      "command": "pdl-lsp",
      "filetypes": ["pdl"],
      "rootPatterns": [".git"]
    }
  }
}
```

Each entry supplies both language services and formatting. A separate formatter
plugin is unnecessary. The same configuration is provided in
[docs/editor/coc-lsp.json](docs/editor/coc-lsp.json).

The highlighting plugin supplies filetype detection. Without it, add this to
vimrc or your Neovim Vimscript configuration:

```vim
augroup icl_toolsets
  autocmd!
  autocmd BufNewFile,BufRead *.icl setlocal filetype=icl
  autocmd BufNewFile,BufRead *.pdl setlocal filetype=pdl
  autocmd FileType pdl if empty(globpath(&runtimepath, 'syntax/pdl.vim')) | setlocal syntax=tcl | endif
augroup END
```

Setting `icl` explicitly overrides Vim's built-in `.icl` association with the
Clean language. Remove older unconditional `setlocal syntax=tcl` PDL autocmds
when using the highlighting plugin.

Reuse your coc.nvim mappings or add:

```vim
nmap <silent> gd <Plug>(coc-definition)
nmap <silent> gi <Plug>(coc-implementation)
nmap <silent> gr <Plug>(coc-references)
```

Run `:CocRestart`, reopen the file, and format the buffer with:

```vim
:call CocAction('format')
```

Formatting uses coc.nvim's `tabSize`, normally derived from `shiftwidth`, or
`tabstop` when `shiftwidth` is zero. No `--indent` server argument is needed.
Output uses spaces even with `noexpandtab`; indentation is capped at 16.
Invalid buffers receive diagnostics and no formatting edits.

For optional format-on-save:

```vim
augroup icl_pdl_format_on_save
  autocmd!
  autocmd BufWritePre *.icl,*.pdl call CocAction('format')
augroup END
```

An optional [coc-diagnostic CLI adapter](docs/editor/coc-cli.json) invokes
`icl-format` with explicit indentation. It has not been tested end to end.

### Language services

Both servers provide diagnostics, completion, definition/declaration,
implementation, references, hover, document/workspace symbols, and whole-document
formatting.

- **ICL:** keyword and scoped-name completion, instance members, module types,
  and local semantic checks. Implementation navigation finds ICL module definitions.
- **PDL:** Tcl/PDL syntax checks, recognized command options and literal values,
  level-0 restrictions, and static procedure/variable navigation. Tcl and PDL
  namespaces and procedure-local variables are distinguished.
- **PDL references to ICL:** module bindings and statically resolvable objects,
  including paths through `iPrefix`.

Workspace navigation includes unopened `.icl` files for ICL, and `.pdl`, `.tcl`,
and `.icl` files for PDL. Hidden directories, `build/`, and symlinked directories
are skipped. Open buffers take precedence over disk files. Ambiguous definitions
remain unresolved.

### Troubleshooting

- Check `:setlocal filetype?`: it should be `icl` or `pdl`.
- Check `:echo executable('icl-lsp')` or `:echo executable('pdl-lsp')`.
  Configure absolute command paths if the editor has a different `PATH`.
- Inspect `:CocInfo` and `:CocCommand workspace.showOutput`.
- A server started without arguments in a terminal waits for LSP messages.
  Use `--help` or `--version` for an immediate response.
- Diagnostics report the first error per document. Correct it to reveal later errors.

## Highlighting only

[vim-icl-pdl-modern](vim-icl-pdl-modern/README.md) provides colors and filetype
detection without coc.nvim, a compiler, or the executables. It follows your
colorscheme and leaves indentation preferences to you.

With [vim-plug](https://github.com/junegunn/vim-plug), add this between your
existing `plug#begin()` and `plug#end()` calls:

```vim
Plug '0BananaBig0/icl_toolsets', {'rtp': 'vim-icl-pdl-modern', 'for': ['icl', 'pdl']}
```

This assumes publication at https://github.com/0BananaBig0/icl_toolsets. Reload
your configuration, run `:PlugInstall`, and reopen the editor and a source file.

`rtp` loads only the `vim-icl-pdl-modern/` directory as a plugin. Vim-plug downloads
the repository but does not build or install the executables or their
dependencies. To avoid downloading the rest of the repository, install a local
copy of that directory using the [plugin guide](vim-icl-pdl-modern/README.md).

The plugin has separate ICL highlighting and extends Vim's Tcl syntax for PDL,
including all 24 IEEE PDL command names and recognized options. Ordinary `.tcl`
filetypes are unchanged. Highlighting does not validate syntax; use a language
server for diagnostics and navigation.

## Scope and limitations

The editor tools target IEEE 1687-2014 and retain documented compatibility cases
from Tessent and Synopsys files. They do not replace vendor design checks.

- ICL editor analysis does not fully elaborate connectivity, arithmetic-derived
  widths, or child-module parameter overrides. Generic AccessLink payloads are opaque.
- PDL editor analysis does not execute Tcl or resolve runtime-generated names
  and scripts. Escaped `switch` list bodies and scripts assembled from multiple
  `namespace eval` arguments have limited static analysis. NUL input and nesting
  above 128 levels are rejected. See [PDL syntax limits](docs/pdl-syntax-oracles.md).
- Symbol rename, range formatting, inferred HDL implementation targets, and
  assignment-column alignment are not provided.

Regression covers standard examples, vendor/public fixtures, malformed input,
CLI/LSP consistency, and Vim/Neovim integration. Formatter tests compare reviewed
expected output and check that formatting it again makes no changes. See
[formatter self-tests](docs/formatter-self-tests.md).

Passing regression does not establish complete language conformance. The strict
handwritten-code coverage target remains unmet. ICL has been tested in a live
Vim/coc.nvim session; a full PDL coc.nvim session remains unverified. Detailed
verification results and development requirements belong in [docs/](docs/README.md).

## Pattern-generation status

`icl-pattern-generation` is under development and is **not yet an available
build or install target**. The four editor executables above do not generate or
retarget patterns.

The repository contains tested internal components for ordered source loading,
netlist elaboration and flattening, PDL execution, BSDL metadata, AccessLink
binding, and instruction-register/ActiveSignal modeling. These components are
not a complete generator. Full TAP integration, scheduling, local-reset output,
the CLI, and end-to-end generated-pattern comparisons remain unfinished.

See the [requirements and design](docs/superpowers/specs/2026-10-25-icl-pattern-generation-design.md),
[implementation plan](docs/superpowers/plans/2026-10-25-icl-pattern-generation.md),
and [model coverage](docs/pattern-model-coverage.md) for current progress and limits.

## Documentation and license

The [documentation index](docs/README.md) links user guides, architecture,
requirements, compatibility decisions, and verification records. The
[developer guide](docs/development.md) covers tests and the optional local
`.worktrees/` layout; no worktree directory is required to use the tools.

The project uses the [MIT License](LICENSE). Third-party components and fixtures
retain their notices; see [third-party notices](docs/third-party-notices.md) and
[test sources](tests/cases/SOURCES.md).
