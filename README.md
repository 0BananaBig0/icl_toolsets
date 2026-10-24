# icl_toolsets

Formatters, language servers, and Vim/Neovim highlighting for IEEE 1687-2014
ICL and PDL.

| Component | Purpose |
|---|---|
| `icl-format` | Format ICL files, rewrite them in place, or check formatting in CI. |
| `icl-lsp` | ICL diagnostics, completion, navigation, symbols, and editor formatting. |
| `pdl-format` | Format PDL with Tcl-aware handling of scripts, data, and substitutions. |
| `pdl-lsp` | PDL diagnostics, completion, navigation, symbols, and editor formatting. |
| `vim-icl-pdl-modern` | Standalone syntax highlighting and filetype detection. |

**ICL (Instrument Connectivity Language)** and **PDL (Procedural Description
Language)** have separate syntax and analysis. PDL analysis parses Tcl without
executing source. These tools help edit IJTAG files; they do not generate test
patterns, retarget procedures, or replace vendor design checks.

## Build and install the executables

### Dynamic build

For a Linux build, install a C++17 compiler and standard library supporting
`std::filesystem`, GNU Make 4.3+, Bison 3.8+, Flex 2.6+, `pkg-config`, and Tcl
development headers/libraries (Tcl 8.6 or newer). PDL requires the real Tcl
library; a stub library alone is insufficient.

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

The build produces all four commands in `build/`; you can run them there before
installing. Installation puts them in `~/.local/bin` by default and prints
coc.nvim setup instructions. It does not edit your editor settings or install
the highlighting plugin.

Keep that directory on your editor's `PATH`, or configure absolute command paths.
To choose another installation location:

```sh
make install-dynamic prefix="$HOME/opt/icl_toolsets"
```

Dynamic executables require compatible C/C++ runtime libraries on the target
machine, plus the matching Tcl shared library for PDL. If your system Bison is
too old, select another with `make dynamic BISON=/path/to/bison`.

### Static and language-specific builds

Static builds additionally require static C/C++ runtime libraries and a real
static Tcl archive compatible with the selected headers:

```sh
make static TCL_STATIC_LIB=/path/to/libtcl9.0.a
make install-static TCL_STATIC_LIB=/path/to/libtcl9.0.a
```

Static outputs have names such as `build/icl-format-static` and
`build/pdl-lsp-static`. Installation uses the same four command names for either
variant; installing into the same prefix replaces the previous variant.
See the [PDL build guide](docs/pdl-tools.md) for building Tcl locally and
static Tcl/glibc portability limits. Test on the intended target host.

| Build command | Outputs |
|---|---|
| `make dynamic` | All four dynamic executables. |
| `make static` | All four static executables. |
| `make icl-dynamic` / `make icl-static` | Only the ICL pair; Tcl is not needed. |
| `make pdl-dynamic` / `make pdl-static` | Only the PDL pair. |
| `make` | Both variants of all four tools. |

The install targets install all four tools. Plain `make install` selects the
dynamic variant. Uninstall with `make uninstall`, using the same `prefix`.
Retain `build/install_manifest.txt` so uninstall can identify installed files.

### Maintainer source archives

An archive produced by `make dist` contains generated parser/scanner sources
and builds without Bison or Flex. Hosting-site automatic source downloads
contain only tracked sources and still require those tools. Compiler, Make,
and Tcl requirements apply to both.

The current archive retains the `icl-format` package name:

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
icl-format --check *.icl             # check formatting for CI
icl-format --indent 4 top.icl        # use four spaces per level
icl-format -o formatted.icl top.icl  # write a separate file
icl-format < top.icl                 # read standard input

pdl-format procedures.pdl
pdl-format -w procedures.pdl
pdl-format --check *.pdl
pdl-format --indent 4 < procedures.pdl
```

Use `--help` for all options. CLI indentation defaults to two spaces and is
capped at 16. The CLI does not read vimrc or `.clang-format`; editor formatting
can use Vim's indentation settings instead.

ICL formatting preserves comments and strings, normalizes spacing and indentation,
and keeps at most one consecutive blank line. PDL formatting indents recognized
braced script bodies, including `iProc`, `proc`, and Tcl control flow, and
normalizes command boundaries. Data words and substitutions retain their spelling;
quoted/dynamic bodies are not reflowed. PDL removes most blank lines.

Invalid input produces a diagnostic and is not rewritten. With multiple inputs,
valid files can still be processed when another fails. Reformatting an already
formatted file leaves its contents unchanged; `-w` also preserves its modification
time. `--check` lists inputs that would change.

| Exit status | Meaning |
|---|---|
| `0` | Success; with `--check`, all inputs are already formatted. |
| `1` | `--check` found an input that would change. |
| `2` | Invalid source, a detected local semantic error, an I/O error, or invalid arguments. |

File writes replace the destination only after a temporary file has been written
successfully, so the destination directory must be writable. Existing symlink
targets, ownership, and permission bits are preserved; dangling symlink outputs
are rejected. Other hard links retain their original contents. Replacement
does not preserve extended attributes or ACLs.

## Configure Vim/Neovim with coc.nvim

Install [coc.nvim](https://github.com/neoclide/coc.nvim) and its required editor
and Node.js versions. Make both language servers available on the editor's
`PATH`.

### Language services and formatting

Open `:CocConfig` and merge these entries into your existing `languageserver`
object, alongside any servers such as `verilog`:

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

One entry per language provides both language services and formatting. The
servers use the same formatting engines as their CLIs; no extra formatter plugin
is required. A copy is in [docs/editor/coc-lsp.json](docs/editor/coc-lsp.json).

The highlighting plugin below supplies filetype detection. If you do not use
it, add this to your vimrc or Neovim Vimscript configuration:

```vim
augroup icl_toolsets
  autocmd!
  autocmd BufNewFile,BufRead *.icl set filetype=icl
  autocmd BufNewFile,BufRead *.pdl set filetype=pdl
  autocmd FileType pdl if empty(globpath(&runtimepath, 'syntax/pdl.vim')) | setlocal syntax=tcl | endif
augroup END
```

The explicit ICL setting overrides Vim's built-in association of `.icl` with
the Clean language. Remove any older unconditional `setlocal syntax=tcl` PDL
autocmd when using the plugin, because it overrides PDL highlighting.

Reuse existing coc.nvim mappings or add:

```vim
nmap <silent> gd <Plug>(coc-definition)
nmap <silent> gi <Plug>(coc-implementation)
nmap <silent> gr <Plug>(coc-references)
```

Run `:CocRestart` and reopen the file. Format the current buffer with:

```vim
:call CocAction('format')
```

Formatting uses the `tabSize` sent by coc.nvim, normally derived from
`shiftwidth`, or `tabstop` when `shiftwidth` is zero. No `--indent` server
argument is needed. Output always uses spaces, even with `noexpandtab`, and
widths are capped at 16. Invalid buffers receive diagnostics and no formatting edits.

For optional format-on-save:

```vim
augroup icl_pdl_format_on_save
  autocmd!
  autocmd BufWritePre *.icl,*.pdl call CocAction('format')
augroup END
```

If you specifically want coc.nvim to launch `icl-format`, an optional
[coc-diagnostic adapter](docs/editor/coc-cli.json) is available for ICL.
That example uses an explicit indentation width and has not been tested end to end.

### What the language servers provide

Both servers offer diagnostics, completion, definition/declaration,
implementation, references, hover, document/workspace symbols, and whole-document
formatting.

| Language | Analysis and navigation |
|---|---|
| ICL | Keywords, local names, instance members, module types, and local semantic checks such as resolvable sized-number overflow. Implementation navigation leads to the ICL module definition. |
| PDL | Tcl/PDL syntax, recognized command options and literal values, explicit level-0 restrictions, static procedure/variable names, array base variables, and explicit namespace-variable bindings. |
| PDL referencing ICL | Module bindings and statically resolvable ICL objects, including paths through `iPrefix`. Imported ICL cannot be formatted as PDL. |

PDL completion keeps Tcl namespaces separate from PDL module/procedure namespaces
and retains argument completion in unfinished procedure bodies. Implementation
navigation for PDL/Tcl procedures returns their definitions.

Workspace navigation includes unopened `.icl` files for ICL, and
`.pdl`, `.tcl`, and `.icl` files for PDL. Hidden directories, `build/`, and
symlinked directories are skipped. Open buffers take precedence over disk files.
Ambiguous definitions are left unresolved rather than selecting an arbitrary target.

### Troubleshooting

- Check `:set filetype?`: it should be `icl` or `pdl`.
- Check `:echo executable('icl-lsp')` or `:echo executable('pdl-lsp')`.
  Configure an absolute command path if the editor has a different `PATH`.
- Inspect `:CocInfo` and `:CocCommand workspace.showOutput`.
- A server started without arguments in a terminal waits for LSP messages.
  `--help`, `-h`, and `--version` print information and exit.
- Diagnostics report the first error per document. Correct it to reveal any
  subsequent errors.

## Install highlighting only

[vim-icl-pdl-modern](vim-icl-pdl-modern/README.md) provides syntax colors and
filetype detection without coc.nvim, a compiler, or any of the executables.
It follows your colorscheme and keeps your indentation preferences.

With [vim-plug](https://github.com/junegunn/vim-plug), add this between your
existing `plug#begin()` and `plug#end()` calls:

```vim
Plug '0BananaBig0/icl_toolsets', {'rtp': 'vim-icl-pdl-modern', 'for': ['icl', 'pdl']}
```

Reload your configuration, run `:PlugInstall`, then reopen the editor and an
`.icl` or `.pdl` file. This assumes the repository is published at
https://github.com/0BananaBig0/icl_toolsets.

The `rtp` option loads only `vim-icl-pdl-modern/` as the Vim plugin. Vim-plug
downloads the repository but does not build or install the formatters, language
servers, or their dependencies. To avoid downloading the rest of the repository,
use a local plugin directory or copy that directory alone as described in the
[plugin installation guide](vim-icl-pdl-modern/README.md).

The plugin is a highlighter, not a syntax validator. It has separate ICL rules
and extends the editor's built-in Tcl highlighting for PDL. Ordinary `.tcl`
filetypes are unchanged. Use the language servers for diagnostics and navigation.

## Support boundaries

The tools target IEEE 1687-2014 with documented compatibility cases from real
Tessent and Synopsys sources. Tests include standard examples, constructed
positive/negative cases, public parser fixtures, vendor files, and editor
integration. This does not establish complete language or semantic conformance.

- ICL checks do not perform full hardware connectivity or width elaboration.
  Arithmetic-derived widths and child-module parameter overrides are not fully
  evaluated; generic AccessLink payloads remain opaque.
- PDL analysis does not execute Tcl, infer runtime-generated names or scripts,
  validate all ICL-dependent values, or schedule/retarget patterns.
- Symbol rename, range formatting, and inferred HDL implementation targets are
  not provided. Formatting does not align assignments into columns.

Vim/Neovim highlighting is corpus-tested. ICL has also been tested in a real
Vim/coc.nvim session; PDL services are tested through stdio LSP clients, with a
full PDL Vim/coc.nvim session still unverified. Current verification evidence,
including the unmet strict coverage gate, is in the
[documentation index](docs/README.md) and [PDL support guide](docs/pdl-tools.md).

## Documentation and license

Development instructions, architecture, requirements, compatibility decisions,
and detailed test reports live in [docs/](docs/README.md).

The project uses the [MIT License](LICENSE). Third-party components and test
fixtures retain their own notices; see [third-party notices](docs/third-party-notices.md)
and [test sources](tests/cases/SOURCES.md).
