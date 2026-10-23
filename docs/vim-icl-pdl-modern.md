# Maintaining the standalone Vim plugin

User installation and feature documentation lives in
[vim-icl-pdl-modern/README.md](../vim-icl-pdl-modern/README.md).

## Requirements and architecture

- Maintain all installable files in `vim-icl-pdl-modern/`. Runtime files must not
  read the parent repository, invoke compiled tools, or need other plugins.
- Keep ICL and PDL syntax groups and lexical rules separate. Reuse the editor's
  builtin Tcl syntax only for PDL. No Tcl syntax implementation is vendored.
- Support Vim and Neovim, native packages and vim-plug's `for` lazy loading.
- Correct the builtin *.icl/Clean association; do not alter ordinary Tcl files.
- Preserve indentation and colorscheme preferences; buffer-local comment options
  must have undo commands. Highlighting is heuristic, not parser validation.
- Tests live under `tests/vim-icl-pdl-modern/`, outside the installable directory.
  Generated tags, archives, test outputs and benchmark inputs stay untracked.
- A remote vim-plug installation must use a plugin-only repository if unrelated
  source must not be downloaded. Its Git history must also be plugin-only.

The design follows the keyword/regex and default-highlight-link approach of
bfrg/vim-c-cpp-modern, inspected locally at commit
`b194a25209ed99ae73175cc5cbc94b2d5684f657`. No code was copied from its Vim-licensed
implementation. Online retrieval failed; the installed copy supplied the actual
README and syntax files. The ICL lexical inventory follows IEEE 1687 clause 6
and `src/icl_lexer.l`; PDL follows clauses 7–8 and its separate command table.

## Verification and packaging

```sh
make vim-plugin-check
make vim-plugin-dist
```

The test runner copies only the plugin into a temporary location and checks
syntax groups, cross-language separation, strings/comments, declaration names,
keyword coverage, filetype detection, user highlight overrides, local-option
undo and indentation preservation in Vim and Neovim. It also uses real installed
vim-plug with filetype lazy loading. Set `VIM_PLUG=/path/to/plug.vim` if needed;
missing editors or vim-plug are reported as skipped. No network is needed.
The tests are included in `make check-dynamic`.

The archive `build/dist/vim-icl-pdl-modern.tar.gz` contains only the standalone
plugin. It can be unpacked into a native package directory. It is a generated
artifact and must not be committed.

## Publish without the toolset sources

A subdirectory `rtp` setting only changes Vim's runtime path. The installed
vim-plug invokes shallow clone with `--no-single-branch`; selecting a plugin-only
branch can still fetch other branches' Git objects. A dedicated plugin-only
repository avoids this problem.

After reviewing and committing the plugin on the main branch, create a subtree
history containing only that directory, then push it to an empty plugin-only
remote (replace the placeholder URL):

```sh
git subtree split --prefix=vim-icl-pdl-modern --branch=vim-icl-pdl-modern
git push https://YOUR-HOST/YOUR-ACCOUNT/vim-icl-pdl-modern.git \
  vim-icl-pdl-modern:main
```

Use the same commands to publish later updates; do not force-push. This does not
switch the working checkout or copy generated files into Git. The split branch
contains the plugin's files at its root; tests and developer docs remain here.
Verify that exported history contains no `src/`, `tests/` or parent build files
before publishing. Update the plugin README with the real installation URL only
when that remote exists. No plugin-only remote has been created by this change.

## Verification on 2026-10-24

Tested with Vim 9.2 and Neovim 0.12.6 on Linux/WSL2:

- `make vim-plugin-check`: all four test methods pass, exercising seven editor
  configurations, including PDL as the first opened file and the Tcl fallback
  registered before or after vim-plug. No editor or vim-plug checks were skipped.
- A real `PlugInstall --sync` clones an isolated plugin-only `file://` Git source.
  The installed tracked-file manifest matches the source and has no parent
  `src/`, `tests/`, `build/` or Makefile. Help tags and help lookup also pass.
- The standalone tarball contains exactly the nine maintained plugin files.
  Tests rerun successfully from the extracted full toolset source distribution.
- `make check-dynamic formatcheck namingcheck BISON=/usr/bin/bison -j4`: passed.
  Five existing external ANTLR integration tests skip when `ICL_REF_DIR` is unset.
  No C++ parser or formatter behavior changed in this task.
- Independent review found an option highlight missing inside quoted command
  substitutions. A failing `-hex` regression was retained before the fix;
  review of the fix found no outstanding issues. All additional probe inputs
  and loading-order controls are retained in `test_syntax.vim`.
- The tests query syntax across 10,000 lines of ICL (1,050,000 bytes) and PDL
  (1,040,000 bytes). Final standalone scan times were approximately 0.38 seconds
  per language in Vim and 0.52 seconds in Neovim. These measure sequential
  headless syntax queries, not interactive redraw latency or a speedup over
  another plugin. Commands and input construction are in the maintained tests.

Online publication, a GUI colorscheme review and older editor versions were not
verified. No plugin-only remote was configured or published by this change; local
vim-plug usage and native package installation are available now. The existing C++ strict-coverage
shortfall is unchanged; that coverage run was not repeated for Vimscript work.
