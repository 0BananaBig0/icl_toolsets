# Origen public PDL examples

Source: [Origen-SDK/ijtag examples/e22.pdl](https://github.com/Origen-SDK/ijtag/blob/134a5e084b7929342bc77a707a438329c3dbf3c3/examples/e22.pdl),
revision `134a5e084b7929342bc77a707a438329c3dbf3c3`, retrieved 2026-10-24.
The upstream MIT notice is retained in [LICENSE](LICENSE).

- `e22_original.pdl` is byte-identical to upstream, SHA-256
  `2a0f2eeabeb5d2a97d208fd58fdff52781c04b6257453542f6fb6e0a0208cc2e`.
  It is an expected negative: both `iProc` definitions omit the argument list
  required by IEEE 1687-2014 §7.9.5 (printed 160, PDF 174).
- `e22_completed.pdl` adds the level/version declaration from §7.9.1 and empty
  argument lists for the two procedures. All other bytes are retained. SHA-256:
  `83d81d7ea430b6b21123fefcc76325df86bece75643edd28986cbdeb96f2011f`.
  The corresponding module is already in `pass/origen/e22.icl`; it is not copied.

Both files are registered in `pdl_blocks.json`, which checks CLI diagnostics,
LSP diagnostics and formatting parity; the positive also checks idempotence.
Vim and Neovim automatically read both through the collected-file corpus test.
Neither fixture is evaluated. These are syntax tests, not pattern-generation
or hardware-behavior tests.

All 21 upstream ICL examples were compared using the shared AST inventory:
their module definitions are already maintained under `pass/origen/`, so no
duplicate ICL files were imported.
