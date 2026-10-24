# icl_toolsets

Formatters, language servers, and Vim/Neovim highlighting for IEEE 1687-2014
ICL and PDL.

| Tool | What it provides |
|---|---|
| `icl-format` | Language-aware ICL formatting for files, stdin, and CI checks. |
| `icl-lsp` | ICL diagnostics, completion, navigation, symbols, and editor formatting. |
| `pdl-format` | PDL formatting with Tcl-aware handling of scripts, data, and substitutions. |
| `pdl-lsp` | PDL diagnostics, completion, navigation, symbols, and editor formatting. |
| `vim-icl-pdl-modern` | Standalone ICL/PDL highlighting and filetype detection. |

ICL (**Instrument Connectivity Language**) and PDL (**Procedural Description
Language**) have separate syntax and analysis. Each language's formatter and
server share their parsing, analysis, and formatting pipeline. PDL source is
parsed without executing Tcl or generating test patterns.

For diagnostics, navigation, and formatting, install the executables and configure
coc.nvim. For highlighting alone, use the [Vim plugin](#install-highlighting-only)
without building any executable.

## Build and install

For Linux, install a C++17 compiler and standard library supporting
`std::filesystem`, GNU Make 4.3+, Bison 3.8+, Flex 2.6+, `pkg-config`, and Tcl
development headers/libraries (8.6 or newer). PDL needs the real Tcl library;
a stub library alone is insufficient.

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

The build creates all four commands in `build/`. Installation puts them in
`~/.local/bin` by default and prints coc.nvim configuration instructions. It does
not modify editor settings or install the highlighting plugin. All four commands
support `--help` and `--version`; these options exit immediately.

Keep the installation directory on your editor's `PATH`, or configure absolute
server paths. To choose another prefix, use
`make install-dynamic prefix="$HOME/opt/icl_toolsets"`.

Dynamic executables need compatible C/C++ runtime libraries on the target
machine and, for PDL, the matching Tcl shared library. Select a newer Bison when
needed with `make dynamic BISON=/path/to/bison`.

### Build variants

| Command | Outputs |
|---|---|
| `make dynamic` | All four dynamic executables. |
| `make static` | All four static variants. |
| `make icl-dynamic` / `make icl-static` | Only the ICL pair; Tcl is not required. |
| `make pdl-dynamic` / `make pdl-static` | Only the PDL pair. |
| `make` | Both variants of all four tools. |

Static builds also need static C/C++ runtime libraries and a real Tcl archive
compatible with the selected headers:

```sh
make static TCL_STATIC_LIB=/path/to/libtcl9.0.a
make install-static TCL_STATIC_LIB=/path/to/libtcl9.0.a
```

Static build outputs use a `-static` suffix, such as `build/pdl-lsp-static`.
Both installation variants use the normal four command names; installing into
the same prefix replaces the previous variant. See the [PDL build guide](docs/pdl-tools.md)
for local Tcl builds and static Tcl/glibc portability limits. Verify binaries on
the intended target host.

`make install` selects the dynamic variant. Install targets install all four
tools. Use `make uninstall` with the same `prefix` to remove them, and retain
`build/install_manifest.txt` so uninstall can identify installed files.

### Source archives

Maintainer archives produced by `make dist` include generated parser/scanner
sources and build without Bison or Flex. Hosting-site automatic source downloads
contain tracked sources and still need those generators. Compiler, Make, and Tcl
requirements apply to both.

The archive package name is currently `icl-format`, although the repository is
`icl_toolsets`:

```sh
tar xzf icl-format-1.0.0.tar.gz
cd icl-format-1.0.0
make dynamic
make install-dynamic
```

## Format files

Both formatters use the same options:

```sh
icl-format top.icl                   # print formatted source
icl-format -w top.icl                # rewrite in place
icl-format -w block_a.icl block_b.icl # format multiple files
icl-format --check *.icl             # check formatting without rewriting
icl-format --indent 4 top.icl        # four spaces per indentation level
icl-format -o formatted.icl top.icl  # write a separate output file
icl-format < top.icl                 # read standard input

pdl-format procedures.pdl
pdl-format -w procedures.pdl
pdl-format --check *.pdl
pdl-format --indent 4 < procedures.pdl
```

CLI indentation defaults to two spaces and is capped at 16. The CLI does not
read vimrc or `.clang-format`; use LSP formatting to reuse editor indentation
settings. Use `--help` for the full option list.

ICL formatting preserves comments and strings, normalizes spacing/indentation,
and retains at most one consecutive blank line. PDL formatting indents recognized
braced script bodies, including `iProc`, `proc`, and Tcl control flow. Data words
and substitutions retain their spelling; quoted/dynamic bodies are not reflowed.
PDL removes most blank lines.

Invalid input receives a diagnostic and is not rewritten. With multiple inputs,
valid files can still be processed when another fails. Formatting an already
formatted file leaves its contents unchanged; `-w` also preserves its modification
time. `--check` lists files whose formatting would change.

| Exit code | Meaning |
|---|---|
| `0` | Success; with `--check`, all inputs are already formatted. |
| `1` | `--check` found formatting changes. |
| `2` | Invalid source, a detected local semantic error, an I/O error, or invalid arguments. |

Writes use a sibling temporary file and replace the destination only after writing
it successfully, so the destination directory must be writable. Existing symlink
targets, ownership, and permission bits are preserved; dangling symlink outputs
are rejected. Other hard links retain their original contents. Replacement does
not preserve extended attributes or ACLs.

## Configure Vim/Neovim with coc.nvim

Install [coc.nvim](https://github.com/neoclide/coc.nvim) and its required editor
and Node.js versions. Make both language servers available on the editor's `PATH`.

Open `:CocConfig` and merge these entries into your existing `languageserver`
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

Each entry supplies language services **and formatting**, using the same engine
as its CLI. No separate formatter plugin is needed. A copy is available in
[docs/editor/coc-lsp.json](docs/editor/coc-lsp.json).

The highlighting plugin below supplies filetype detection. Without it, put this
in your vimrc or Neovim Vimscript configuration:

```vim
augroup icl_toolsets
  autocmd!
  autocmd BufNewFile,BufRead *.icl setlocal filetype=icl
  autocmd BufNewFile,BufRead *.pdl setlocal filetype=pdl
  autocmd FileType pdl if empty(globpath(&runtimepath, 'syntax/pdl.vim')) | setlocal syntax=tcl | endif
augroup END
```

Explicitly setting `icl` overrides Vim's built-in `.icl` association with the
Clean language. Remove older unconditional `setlocal syntax=tcl` PDL autocmds
when using the highlighting plugin.

Reuse your coc.nvim mappings or add:

```vim
nmap <silent> gd <Plug>(coc-definition)
nmap <silent> gi <Plug>(coc-implementation)
nmap <silent> gr <Plug>(coc-references)
```

Run `:CocRestart`, reopen the source file, and format the buffer with:

```vim
:call CocAction('format')
```

Formatting uses coc.nvim's `tabSize`, normally derived from `shiftwidth`, or
`tabstop` when `shiftwidth` is zero. No `--indent` server argument is needed.
Output uses spaces even with `noexpandtab`; indentation widths are capped at 16.
Invalid buffers receive diagnostics and no formatting edits.

For optional format-on-save:

```vim
augroup icl_pdl_format_on_save
  autocmd!
  autocmd BufWritePre *.icl,*.pdl call CocAction('format')
augroup END
```

An optional [coc-diagnostic CLI adapter](docs/editor/coc-cli.json) is available
for ICL if you specifically want coc.nvim to invoke `icl-format`. It uses an
explicit indentation width and has not been tested end to end.

### Language services

Both servers provide diagnostics, completion, definition/declaration,
implementation, references, hover, document/workspace symbols, and whole-document
formatting.

- **ICL:** keyword and scoped-name completion, instance members, module types,
  and local semantic checks. Implementation navigation leads to ICL module
  definitions.
- **PDL:** Tcl/PDL syntax checks, recognized command options and literal values,
  level-0 restrictions, and static procedure/variable navigation. Tcl namespaces,
  procedure-local variables, namespace variables, and PDL procedure namespaces
  are distinguished. Implementation navigation returns procedure definitions.
- **PDL references to ICL:** module bindings and statically resolvable objects,
  including paths through `iPrefix`. Imported ICL is not formatted as PDL.

Workspace navigation includes unopened `.icl` files for ICL, and `.pdl`, `.tcl`,
and `.icl` files for PDL. Hidden directories, `build/`, and symlinked directories
are skipped. Open buffers take precedence over disk files. Ambiguous definitions
remain unresolved.

### Troubleshooting

- Check `:setlocal filetype?`: it should be `icl` or `pdl`.
- Check `:echo executable('icl-lsp')` or `:echo executable('pdl-lsp')`.
  Use absolute command paths when the editor has a different `PATH`.
- Inspect `:CocInfo` and `:CocCommand workspace.showOutput`.
- A server started without arguments in a terminal waits for LSP messages.
  `--help`, `-h`, and `--version` print information and exit.
- Diagnostics report the first error per document. Correct it to reveal later errors.

## Install highlighting only

The [vim-icl-pdl-modern](vim-icl-pdl-modern/README.md) plugin supplies colors and
filetype detection without coc.nvim, a compiler, or the executables. It follows
your colorscheme and leaves indentation preferences to you.

With [vim-plug](https://github.com/junegunn/vim-plug), add this between your
existing `plug#begin()` and `plug#end()` calls:

```vim
Plug '0BananaBig0/icl_toolsets', {'rtp': 'vim-icl-pdl-modern', 'for': ['icl', 'pdl']}
```

This assumes the repository is published at
https://github.com/0BananaBig0/icl_toolsets. Reload your configuration, run
`:PlugInstall`, and reopen the editor and an `.icl` or `.pdl` file.

The `rtp` option loads only `vim-icl-pdl-modern/` as the plugin. Vim-plug downloads
the repository but does not build or install the formatters, servers, or their
dependencies. To avoid downloading the rest of the repository, install a local
copy of that directory as described in the [plugin guide](vim-icl-pdl-modern/README.md).

The plugin has separate ICL highlighting and extends Vim's Tcl syntax for PDL,
including all 24 IEEE PDL command names and recognized options. Ordinary `.tcl`
filetypes are unchanged. Highlighting does not validate syntax; use the language
servers for diagnostics and navigation.

## Supported scope and verification

The tools target IEEE 1687-2014 and retain documented compatibility cases from
Tessent and Synopsys files. They help edit IJTAG sources; they do not replace
vendor design checks, generate patterns, or retarget procedures.

- ICL analysis does not fully elaborate connectivity, arithmetic-derived widths,
  or child-module parameter overrides. Generic AccessLink payloads remain opaque.
- PDL analysis does not execute Tcl or infer runtime-generated names/scripts.
  Some static forms also have limited analysis: escaped `switch` list bodies and
  scripts assembled from multiple `namespace eval` arguments are not reconstructed.
  NUL input and nesting above 128 levels are rejected. See [PDL syntax limits](docs/pdl-syntax-oracles.md).
- Symbol rename, range formatting, inferred HDL implementation targets, and
  assignment-column alignment are not provided.

Regression includes standard examples, vendor/public fixtures, malformed cases,
CLI/LSP consistency, and Vim/Neovim integration. Formatter tests compare output
with saved expected files and require a second formatting pass to leave the first
output unchanged. See [formatter self-tests](docs/formatter-self-tests.md) for
commands and baseline maintenance.

Passing these checks does not prove complete syntax or semantic conformance.
The strict handwritten-code coverage target remains unmet. ICL has been tested
in a real Vim/coc.nvim session; PDL services are tested through stdio LSP clients,
with a full PDL coc.nvim session still unverified. Recent checks did not repeat
the live coc.nvim session or a Tcl 8.6 build.

## Documentation and license

See the [documentation index](docs/README.md) for architecture, requirements,
compatibility decisions, and dated verification results. The [developer guide](docs/development.md)
contains build/test workflows and the local `.worktrees/` layout; that directory
is optional local state and is not required to use the tools.

The project uses the [MIT License](LICENSE). Third-party components and fixtures
retain their own notices: see [third-party notices](docs/third-party-notices.md)
and [test sources](tests/cases/SOURCES.md).
