# vim-icl-pdl-modern implementation plan

> Execution: superpowers:executing-plans inline, TDD and one independent final review.

Goal: a self-contained Vim/Neovim syntax plugin, maintained in this repository,
installable with vim-plug without building or installing the C++ tools.
Standing autonomous authorization covers repository edits. Distribution preference
was requested; both choices use the same standalone source directory. Remote
publication is a separate final step after verification.

Architecture: vim-icl-pdl-modern/{ftdetect,ftplugin,syntax,doc}, README and MIT
license. ICL uses its own keywords/lexical patterns; PDL sources Vim's builtin
Tcl syntax then adds PDL command/number/name groups. Default highlight links
respect colorschemes. No dependency on other plugins, Python, compiled tools or
LSP services at runtime. No automatic server installation or vimrc edits.

Reference: installed bfrg/vim-c-cpp-modern b194a25209ed99ae73175cc5cbc94b2d5684f657;
use its approach (regex/keywords, default links), not copied licensed code.
IEEE lexical rules and separate local parsers provide the language inventory.
Existing Tcl runtime remains responsible for Tcl highlighting limitations.

- [x] Write headless Vim/Neovim assertions under tests/vim-icl-pdl-modern/ for
  file detection, syntax groups, comments/strings, separate numeric forms,
  declaration names, PDL substitutions, lazy loading and buffer-local settings.
  Watch tests fail before implementing. Use installed vim-plug in integration
  tests when present; report absence as skipped, not passed.
- [x] Implement minimal self-contained runtime. Handle builtin *.icl Clean
  association and avoid changing unrelated *.tcl/C++ buffers. Keep indentation
  preferences intact; provide reversible local comments settings.
- [x] Add plugin user documentation and root README link. Explain highlighting
  is not syntax validation. Update existing editor setup so forcing syntax=tcl
  does not override PDL syntax when the plugin is enabled. Document plugin-only
  publication (subtree split) and local vim-plug installation accurately.
- [x] Test an isolated plugin copy (no source/build/tests dependency), native
  Vim and Neovim, real vim-plug lazy loading, large representative highlighting,
  and standard regression targets affected by installer/documentation changes.
  Request independent review, retain probes, fix demonstrated defects.

Performance: target interactive 100 KB–1 MB files. Prefer short anchored/local
patterns and builtin Tcl syntax; no source-wide AST passes, subprocesses or
synchronous LSP queries. Record end-to-end headless syntax scan time on ~1 MB
input; this is not an interactive redraw benchmark.

Packaging: `rtp` is not a download filter. Installed vim-plug clones shallow
with --no-single-branch, so plugin-only branch checkout still can fetch unrelated
Git objects. Only a dedicated plugin-only remote guarantees the requested
source-download isolation. No generated tags, archive, screenshots or test
outputs are tracked.

Verification: initial headless tests failed on missing ICL/PDL runtime. Native
and vim-plug tests now pass on Vim9.2/Neovim0.12.6, including a real plugin-only
Git clone/install, ~1MB syntax scans, help tags and first-file PDL lazy loading.
Independent review's option-in-substitution issue reproduced RED then fixed;
all probe sources retained. Full dynamic regression/style checks passed, and
plugin tests passed from the extracted full source distribution. Publication
is intentionally pending a plugin-only remote; local installation works now.
Details and exact commands: docs/vim-icl-pdl-modern.md.
