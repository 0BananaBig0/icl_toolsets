# icl_toolsets

Tools for editing and formatting **ICL (Instrument Connectivity Language)**,
the instrument connectivity language defined by IEEE 1687-2014 (IJTAG).

| Tool | Purpose |
|---|---|
| `icl-format` | Format ICL files from the command line, rewrite files in place, or check formatting in CI. |
| `icl-lsp` | Provide diagnostics, formatting, completion and code navigation in editors such as Vim/Neovim with coc.nvim. |

The repository is named `icl_toolsets`; the installed commands are `icl-format`
and `icl-lsp`. Both tools apply the same ICL parsing and formatting rules.
PDL (Procedural Description Language) is a separate language and is not supported.

## Build and install

Building from Git requires a C++17 compiler (GCC 9+ or Clang 10+), GNU Make 4.3+,
Bison 3.8+ and Flex 2.6+.

```sh
git clone https://gitee.com/banana33/icl_toolsets.git
cd icl_toolsets
make dynamic
make install-dynamic
export PATH="$HOME/.local/bin:$PATH"
icl-format --version
```

This installs both commands into `$HOME/.local/bin`. Add that directory to your
shell's startup configuration so your editor can also find them. A custom
installation prefix is supported:

```sh
make install-dynamic prefix=/opt/icl_toolsets
```

The dynamic binaries require compatible C/C++ runtime libraries on the target
machine. For a static build, install the static C/C++ runtime libraries first,
then run `make static` and `make install-static`. Both installation variants
use the same command names. Plain `make` builds both variants; `make dynamic`
works without static runtime libraries.

To remove an installation, run `make uninstall` from the build directory,
using the same `prefix` if you selected one. Keep `build/install_manifest.txt`
until uninstalling; it records the installed paths. Installation replaces
same-named files under the selected prefix.

### Building from a source archive

The source archive includes the generated parser and scanner, so building it
requires the compiler and Make but does not require Bison or Flex. This is useful
on Rocky Linux 8, whose system Bison is older than the Git build requires.
Use an archive provided by the project maintainer:

```sh
tar xzf icl-format-1.0.0.tar.gz
cd icl-format-1.0.0
make dynamic
make install-dynamic
```

The current archive/package name is `icl-format-1.0.0`; the repository name is
`icl_toolsets`. With a Git checkout, install a suitable Bison version or select
one explicitly, for example `make dynamic BISON=/path/to/bison`.

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

The default indentation is two spaces. Formatting preserves comments and string
contents, normalizes spacing and indentation, and limits consecutive blank lines
to one. Formatting an already formatted file produces the same result.
With `-w`, an unchanged file keeps its modification time.

Invalid input reports a diagnostic instead of formatted output. When formatting
multiple files, valid files can still be processed even if another file fails.

| Exit code | Meaning |
|---|---|
| `0` | Success; with `--check`, all inputs are already formatted. |
| `1` | `--check` found at least one input that would change. |
| `2` | Syntax error, file I/O error or invalid command-line arguments. |

For CI, run `icl-format --check` on the ICL files you want to enforce and use
its exit status as the formatting check result.

## Editor features

`icl-lsp` runs over standard input/output and is started by the editor's LSP
client. It provides:

- Syntax diagnostics on open and edit, plus whole-document formatting.
- Completion for ICL keywords, local names, instance members and module types.
- Go to definition/declaration, go to implementation, and find references.
- Hover information, document symbols and workspace symbols.

Syntax checking uses the same ICL parser as `icl-format`. Errors underline the
unexpected token using editor-compatible Unicode positions; incomplete input
points to the end of the document. Errors clear when corrected or closed. The
server reports the first error per document; fix it to reveal subsequent errors.
Some semantic checks report a whole line when no token location is available.

Implementation navigation leads to the referenced **ICL module definition**.
HDL implementation inference and symbol renaming are not available.
Workspace navigation includes unopened `.icl` files under the project roots;
hidden directories, `build/` and symlinked directories are skipped. Open buffers
take precedence over files on disk. Ambiguous module definitions do not produce
a guessed navigation target.

### Vim/Neovim with coc.nvim

Install [coc.nvim](https://github.com/neoclide/coc.nvim) and its prerequisites,
and ensure `icl-lsp` is on the editor's `PATH`. Vim users need a coc.nvim-compatible
Vim version; the tested setup uses Vim 9.2. Follow coc.nvim's current requirements
for your editor and Node.js versions.

Merge this configuration into `:CocConfig`. A copy is available in
[docs/editor/coc-lsp.json](docs/editor/coc-lsp.json):

```json
{
  "languageserver": {
    "icl": {
      "command": "icl-lsp",
      "filetypes": ["icl"],
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
  autocmd FileType icl setlocal shiftwidth=2 tabstop=2 expandtab
augroup END

nmap <silent> gd <Plug>(coc-definition)
nmap <silent> gi <Plug>(coc-implementation)
nmap <silent> gr <Plug>(coc-references)
```

The explicit `set filetype=icl` overrides Vim's built-in association of `.icl`
with the Clean language. The indentation settings keep editor formatting aligned
with the CLI default. Existing coc.nvim navigation mappings can be reused.

Format the current buffer with:

```vim
:call CocAction('format')
```

For optional format-on-save, add `autocmd BufWritePre *.icl call CocAction('format')`
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

The optional adapter configuration has been checked against its upstream schema;
it has not been tested end to end. The default LSP configuration above has been
tested with a real Vim/coc.nvim session. Installation prints both configurations
for reference; it does not edit your editor settings.

## Language support and limitations

The tools target IEEE 1687-2014 ICL and include documented compatibility forms
from Tessent and other existing ICL sources. Details are recorded in the
[conformance review](docs/icl-conformance-review.md).

Formatting and local diagnostics do not perform full hardware connectivity or
width elaboration, execute PDL, or replace vendor DRC tools. Integer expressions
are preserved rather than evaluated. Generic AccessLink payloads are treated as
opaque content. Formatting does not align assignment operators into columns.

## Documentation and license

See [the documentation index](docs/README.md) for the developer guide, project
requirements, compatibility decisions and verification reports.

The project is licensed under the [MIT License](LICENSE). Third-party fixtures
retain their original notices and licenses; see [test sources](tests/cases/SOURCES.md).
