# icl_toolsets

Tools for editing and formatting **ICL (Instrument Connectivity Language)** and
**PDL (Procedural Description Language)** defined by IEEE 1687-2014 (IJTAG).

| Tool | Purpose |
|---|---|
| `icl-format` | Format ICL files from the command line, rewrite files in place, or check formatting in CI. |
| `icl-lsp` | Provide ICL diagnostics, formatting, completion and code navigation in editors such as Vim/Neovim with coc.nvim. |
| `pdl-format` | Format PDL and nested Tcl scripts while preserving data words and substitutions. |
| `pdl-lsp` | Provide PDL diagnostics, formatting, procedure/variable completion and navigation, including static references into ICL. |

The repository is named `icl_toolsets`. Each language pair shares its own parser,
syntax tree, analysis and formatting engine. ICL and PDL remain separate languages;
PDL uses Tcl's parsing API without executing source. Both language servers format
directly, without launching a formatter subprocess. These tools do not execute
IJTAG procedures or replace vendor connectivity and design-rule checks.

## Build and install

Building from Git requires a C++17 compiler and standard library with
`std::filesystem` support (for example, GCC 9+ or Clang 10+ with a suitable
standard library), GNU Make 4.3+, Bison 3.8+, Flex 2.6+, pkg-config and Tcl
development files (Tcl 8.6 or newer). PDL builds require the real Tcl library;
the Tcl stub library alone is insufficient.

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

`make dynamic` produces `build/icl-format`, `build/icl-lsp`, `build/pdl-format`
and `build/pdl-lsp`; you can run these directly before installing.
`make install-dynamic` installs all four commands into
`$HOME/.local/bin` and prints the coc.nvim setup below. Add that directory to your
shell's startup configuration so your editor can also find them. Installation
does not change your editor configuration. A custom installation prefix is
supported:

```sh
make install-dynamic prefix=/opt/icl_toolsets
```

The dynamic binaries require compatible C/C++ runtime libraries on the target
machine, and PDL binaries also require the corresponding Tcl shared library.
For a static build, install static C/C++ runtime libraries and a real static Tcl
archive built for the matching Tcl headers, then run:

```sh
make static TCL_STATIC_LIB=/path/to/libtcl9.0.a
make install-static TCL_STATIC_LIB=/path/to/libtcl9.0.a
```

Static outputs have the `-static` suffix under `build/`. See the
[PDL build guide](docs/pdl-tools.md) for building Tcl locally and platform limits.
Use `make icl-dynamic` or `make icl-static` to build only ICL without Tcl,
or `make pdl-dynamic` / `make pdl-static` for just PDL. Both installation variants
use the same command names. Plain `make` builds both variants; `make dynamic`
works without static runtime libraries. Plain `make install` selects the dynamic
variant.

To remove an installation, run `make uninstall` from the same source directory,
using the same `prefix` if you selected one. Keep `build/install_manifest.txt`
until uninstalling; it records the installed paths. Installation replaces
same-named files under the selected prefix.

### Building from a source archive

An archive produced by `make dist` includes the generated parser and scanner.
Building it requires the compiler, Make and Tcl development files but does not
require Bison or Flex. This is useful
on Rocky Linux 8, whose system Bison is older than the Git build requires.
Use the maintainer's `make dist` archive, rather than a Git hosting site's automatic
source download (which contains only the tracked sources). For the current version:

```sh
tar xzf icl-format-1.0.0.tar.gz
cd icl-format-1.0.0
make dynamic
make install-dynamic
```

The current archive/package name is `icl-format-1.0.0`; the repository name is
`icl_toolsets`. `make dist` writes the archive under `build/dist/`; see the
[developer guide](docs/development.md) for packaging and verification commands.
With a Git checkout, install a suitable Bison version or select one explicitly,
for example `make dynamic BISON=/path/to/bison`.

## Format ICL files

```sh
icl-format top.icl                   # print formatted output
icl-format -w top.icl                # update the file in place
icl-format -w block_a.icl block_b.icl # format multiple files
icl-format --check *.icl             # check formatting without changing files
icl-format --indent 4 top.icl        # use four spaces per indentation level
icl-format -o formatted.icl top.icl  # write to a separate file
icl-format < top.icl                 # read from standard input
icl-format --help
```

The CLI defaults to two spaces per indentation level and does not read vimrc or
`.clang-format` settings. Use `--indent N` to select a width; widths above 16 are
limited to 16. For editor-controlled indentation, use the LSP integration below.

Formatting preserves comments and string contents, normalizes spacing and
indentation, and limits consecutive blank lines
to one. Formatting an already formatted file produces the same result.
With `-w`, an unchanged file keeps its modification time.
Changed files are written to a temporary file in the destination directory and
replaced only after the write succeeds. The destination directory must therefore
be writable. Existing symlinks retain their targets, and file ownership and
permission bits are preserved; dangling symlink outputs are rejected. Replacing
one hard-linked path leaves other hard links with their original contents.
Extended attributes and ACLs are not preserved by replacement.

Input rejected by the parser or its local semantic checks reports a diagnostic
instead of formatted output; `-w` leaves that input unchanged. When formatting
multiple files, valid files can still be processed even if another file fails.
`--check` prints the names of inputs that would change, without rewriting them.

| Exit code | Meaning |
|---|---|
| `0` | Success; with `--check`, all inputs are already formatted. |
| `1` | `--check` found at least one input that would change. |
| `2` | Invalid source (including detected semantic errors), file I/O error or invalid command-line arguments. |

For CI, run `icl-format --check` on the ICL files you want to enforce and use
its exit status as the formatting check result.

## Format PDL files

The PDL formatter supports the same CLI options and exit codes as `icl-format`:

```sh
pdl-format procedures.pdl
pdl-format -w procedures.pdl
pdl-format --check procedures.pdl
pdl-format --indent 4 < procedures.pdl
```

It indents recognized braced script bodies (`iProc`, `proc`, Tcl control flow),
normalizes spaces between words and places separate commands on separate lines.
Braced data, quoted text and substitutions retain their spelling. Invalid input
is diagnosed without rewriting the file. Unknown Tcl command arguments remain
opaque; quoted/dynamic script bodies are preserved rather than evaluated.

## Editor features

`icl-lsp` and `pdl-lsp` run over standard input/output and are started without
arguments by the editor's LSP client. Use `icl-lsp --help` (or `-h`) and
`icl-lsp --version` to print information and exit. Unsupported arguments exit with status 2. Running
`icl-lsp` without arguments in a terminal waits for LSP messages; this is normal.
`pdl-lsp` accepts the same help/version options.

The ICL server provides:

- Syntax and local semantic diagnostics on open and edit, plus whole-document formatting.
- Completion for ICL keywords, local names, instance members and module types.
- Go to definition/declaration, go to implementation, and find references.
- Hover information, document symbols and workspace symbols.

Diagnostics use the same parser and local validation as `icl-format`, including
checks for sized-number overflow and locally resolvable parameter widths.
Syntax errors underline the unexpected token using editor-compatible Unicode
positions; incomplete input
points to the end of the document. Errors clear when corrected or closed. The
server reports the first error per document; fix it to reveal subsequent errors.
Some semantic checks report a whole line when no token location is available.

For ICL, implementation navigation leads to the referenced **ICL module definition**.
HDL implementation inference and symbol renaming are not available.
Workspace navigation includes unopened `.icl` files under the project roots;
hidden directories, `build/` and symlinked directories are skipped. Open buffers
take precedence over files on disk. Ambiguous module definitions do not produce
a guessed navigation target.

PDL diagnostics check Tcl syntax, recognized literal scripts/expressions, PDL
command arities/options, literal values and explicit level-0 restrictions.
PDL navigation resolves statically named procedures, local variables, module
bindings and ICL objects through `iPrefix`. It scans `.pdl`, `.tcl` and `.icl`
workspace files. Dynamic names and runtime-generated code cannot be resolved;
this is not a Tcl runtime, retargeter or complete PDL semantic validator. See
[PDL support and limits](docs/pdl-tools.md).

### Vim/Neovim with coc.nvim

For standalone syntax highlighting, use
[vim-icl-pdl-modern](vim-icl-pdl-modern/README.md). It supports ICL and PDL without
coc.nvim, a compiler or the language servers, and can be installed separately
from the toolsets. Keep coc.nvim for diagnostics, formatting and navigation.

Install [coc.nvim](https://github.com/neoclide/coc.nvim) and its prerequisites,
and ensure both language servers are on the editor's `PATH`. Vim users need a coc.nvim-compatible
Vim version; the tested setup uses Vim 9.2. Follow coc.nvim's current requirements
for your editor and Node.js versions.

Each language needs one entry for language services and formatting. Merge both into
`languageserver` alongside your existing servers (such as `verilog`) in
`:CocConfig` (normally `~/.vim/coc-settings.json` in Vim). A copy is available in
[docs/editor/coc-lsp.json](docs/editor/coc-lsp.json):

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

Add these lines to your Vim configuration:

```vim
augroup icl_toolsets
  autocmd!
  autocmd BufNewFile,BufRead *.icl set filetype=icl
  autocmd BufNewFile,BufRead *.pdl set filetype=pdl
  autocmd FileType pdl if empty(globpath(&runtimepath, 'syntax/pdl.vim')) | setlocal syntax=tcl | endif
augroup END

nmap <silent> gd <Plug>(coc-definition)
nmap <silent> gi <Plug>(coc-implementation)
nmap <silent> gr <Plug>(coc-references)
```

The explicit `set filetype=icl` overrides Vim's built-in association of `.icl`
with the Clean language. Existing coc.nvim navigation mappings can be reused.
The highlighting plugin supplies file detection itself; its users can omit these
filetype autocmds. Replace any old unconditional `setlocal syntax=tcl` PDL autocmd
with the fallback above, or remove it, to retain the plugin's PDL highlighting.
After changing the configuration, run `:CocRestart` and reopen the ICL file, or
set `:set filetype=icl` in the current buffer.

LSP formatting uses the `tabSize` sent by coc.nvim, normally derived from Vim's
`shiftwidth` (or `tabstop` when `shiftwidth` is zero). No `--indent` argument is
needed in the server configuration. The formatter always emits spaces, even with
`noexpandtab`; widths above 16 are limited to 16. The standalone CLI still uses
its own two-space default unless you pass `--indent`.

Format the current buffer with:

```vim
:call CocAction('format')
```

For optional format-on-save, add `autocmd BufWritePre *.icl,*.pdl call CocAction('format')`
inside the `icl_toolsets` group. Invalid buffers receive diagnostics and no
formatting edits.

If the server does not start, check `:set filetype?`,
`:echo executable('icl-lsp')` and `:CocInfo`. Use
`:CocCommand workspace.showOutput` to inspect its log.

### Optional CLI formatting through coc.nvim

To launch `icl-format` for editor formatting, install `coc-diagnostic` with
`:CocInstall coc-diagnostic` and use
[docs/editor/coc-cli.json](docs/editor/coc-cli.json). This configuration keeps
`icl-lsp` for navigation and diagnostics and disables its formatting provider
so only the CLI adapter formats the buffer.

The optional adapter example explicitly passes `--indent 2`; it does not inherit
your Vim indentation width. It has been checked against its upstream schema but
has not been tested end to end. The ICL entry of the recommended configuration has been
tested with a real Vim/coc.nvim session; PDL has been tested through real stdio
LSP clients, with a full Vim/coc.nvim session still unverified. See the
[verification report](docs/parameter-width-validation.md).

## Language support and limitations

The ICL tools target IEEE 1687-2014 ICL and include documented compatibility forms
from Tessent and other existing ICL sources. Tests cover standard examples,
constructed positive/negative cases, public parser fixtures and collected
Tessent/Synopsys SLM ICL. This is tested support, not a guarantee that every
possible ICL input or semantic error is handled. See the
[conformance review](docs/icl-conformance-review.md) and
[verification reports](docs/README.md) for evidence and remaining gaps.

Formatting and local diagnostics do not perform full hardware connectivity or
width elaboration, execute PDL, or replace vendor DRC tools. They check sized
literals against locally resolvable numeric Parameter/LocalParameter defaults
and alias chains, including forward references. Arithmetic-derived widths and
child-module parameter overrides are not fully evaluated. Integer expressions
are preserved rather than evaluated. Generic AccessLink payloads are treated as
opaque content. Formatting does not align assignment operators into columns,
and only whole-document formatting is supported. Symbol rename, range formatting
and inferred HDL implementation targets are not provided.

## Documentation and license

See [the documentation index](docs/README.md) for the developer guide, project
requirements, compatibility decisions and verification reports.

The project is licensed under the [MIT License](LICENSE). Third-party fixtures
retain their original notices and licenses; see [test sources](tests/cases/SOURCES.md).
