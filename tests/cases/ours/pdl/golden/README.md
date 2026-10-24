# Handwritten PDL formatter expectations

These small input/`.expected` pairs were authored for this repository. Their
organization is inspired by Verilator's focused source/expected-output pairs;
no Verilator HDL, code or golden output was copied. PDL/Tcl syntax follows the
existing level-1 parser contract, IEEE 1687-2014 §8 and Tcl script syntax.
Referenced ICL register names are placeholders; these files are never evaluated.

| Pair | Formatting contract |
|---|---|
| nested_control | iProc, if/else and foreach indent recursively; command separators become newlines. |
| literal_data | Multiline braced data and unmatched literal `[` retain exact bytes. |
| substitutions | Quotes, variable words and command-substitution spacing are preserved. |
| expanded_arguments | Both dynamic and constant `{*}` expansion retain their command spelling. |
| comments | Hash comments stay comments; quoted `#`, semicolons and braces remain data. |
| handlers | namespace/proc/try/on/finally script bodies indent without formatting handler variable lists. |

`make goldencheck` checks exact bytes through filename and stdin input modes.
`pdl_blocks.json` also registers these inputs for syntax diagnostics, CLI/LSP
formatting parity and idempotence. Vim/Neovim corpus tests discover the `.pdl`
inputs automatically. Expectations are maintained by hand; the runner's
explicit `--update` option is only for intentional, reviewed style changes.
