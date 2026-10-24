# vim-icl-pdl-modern

ICL and PDL syntax highlighting for Vim and Neovim, inspired by
[bfrg/vim-c-cpp-modern](https://github.com/bfrg/vim-c-cpp-modern).

- ICL modules, enums, ports, declarations, properties and referenced type names.
- ICL parameters, apostrophe-based numbers, strings and C-style comments.
- All 24 IEEE 1687 PDL commands, procedure names, options and PDL binary/hex values.
- PDL builds on the editor's builtin Tcl highlighting for control flow, variables,
  strings, comments and substitutions. ICL uses a separate syntax definition.
- Automatic `.icl`/`.pdl` detection, including overriding Vim's `.icl` → Clean
  association. Ordinary `.tcl` files are unaffected.
- Colors follow your colorscheme; indentation preferences are preserved.

No other plugin, compiler, executable or language server is required. This is a
keyword/regex highlighter, not a parser or a syntax checker. ICL keywords may
also be names, so keyword-like references can retain keyword colors. Computed
PDL/Tcl scripts and opaque braced data inherit the builtin Tcl highlighter's
heuristics. Use `icl-lsp`/`pdl-lsp` separately if you also want diagnostics,
formatting and navigation.

## vim-plug installation

Assuming `icl_toolsets` is published under the GitHub account `0BananaBig0`,
add this between your existing `plug#begin()` and `plug#end()` calls:

```vim
Plug '0BananaBig0/icl_toolsets', {'rtp': 'vim-icl-pdl-modern', 'for': ['icl', 'pdl']}
```

Reload your vimrc or Neovim configuration, run `:PlugInstall`, then reopen the
editor and an `.icl` or `.pdl` file. Vim-plug resolves the repository to
https://github.com/0BananaBig0/icl_toolsets.

Only `vim-icl-pdl-modern/` is loaded as the plugin runtime. The repository is
downloaded, but no formatters, language servers, or compiler dependencies are
built or installed. Avoiding the download of unrelated source requires a
separately published plugin-only repository or a local directory.

For an existing local checkout, use this alternative (adjust the absolute path):

```vim
Plug '/absolute/path/to/icl_toolsets/vim-icl-pdl-modern', {'for': ['icl', 'pdl']}
```

The local alternative downloads nothing. Keep that directory available and
reopen the editor after adding the line.

If your vimrc previously had this line, remove it:

```vim
" Remove this old fallback when using the plugin:
" autocmd FileType pdl setlocal syntax=tcl
```

It would override PDL-specific highlighting. Ensure `syntax enable` and
`filetype plugin on` are enabled (vim-plug normally enables them).

## Native Vim/Neovim packages

Copy this directory alone into a `pack/plugins/start/` directory on your
`packpath`, for example `~/.vim/pack/plugins/start/vim-icl-pdl-modern` for Vim or
`~/.local/share/nvim/site/pack/plugins/start/vim-icl-pdl-modern` for Neovim.
Enable `syntax enable` and `filetype plugin on` in your editor configuration.

## Customize colors

Groups use `highlight default link`, so your colorscheme or overrides win:

```vim
augroup my_icl_pdl_colors
  autocmd!
  autocmd ColorScheme * highlight link iclTypeName Type
  autocmd ColorScheme * highlight link pdlCommand Statement
  autocmd ColorScheme * highlight link pdlProcedure Function
augroup END
```

See `:help vim-icl-pdl-modern` for the groups. Vim-plug generates help tags;
manual installations can run `:helptags /path/to/vim-icl-pdl-modern/doc`.

## License

MIT; see [LICENSE](LICENSE). The syntax definitions are original code. The
reference plugin's implementation is not copied. Tcl syntax is loaded from your
editor's existing runtime, not bundled here.
