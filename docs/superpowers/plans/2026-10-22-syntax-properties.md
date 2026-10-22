# Independent Syntax Properties and Live Diagnostics

> **For agentic workers:** Use superpowers:executing-plans for inline execution.

**Goal:** Check syntax with IEEE-derived acceptance-preserving transformations, and make live LSP syntax errors precise and lifecycle-correct.

**Architecture:** Use the existing shared parser/AST and batch inventory executable. Generate tests independently from whitespace/comment permissions in IEEE 1687-2014 §§6.3.2, 6.3.4(11), 6.3.6–7. Preserve parser locations through the driver; publish UTF-16 ranges and document versions without rendering a valid document just to check syntax.

**Constraints:** ICL only; keep PDL separate, Google C++ style, generated cases below ignored tests/generated/ours, minimized regressions maintained under tests/cases/ours. No finite test campaign proves every possible recursive input. Preserve CLI diagnostics and the first-error policy.

- [x] Add and run metamorphic acceptance checks across classified syntax blocks, using bounded batches and a deterministic seed inventory. Adjudicate each unexpected rejection against the standard before fixing it.
- [x] Add failing LSP tests for token/EOF/UTF-16 ranges, versions, correction, close/reopen and corpus-wide diagnostic acceptance.
- [x] Fix shared diagnostic spans and lifecycle behavior; retain first-error reporting without speculative parser recovery.
- [x] Run focused tests, style checks, full dynamic regression, independent review, archive/install verification; record scope and limitations, commit and push.

Verification: 25,938 variants / 45 blocks / no mismatches; all 1,885 conformance
expectations also checked through live LSP; 334/334 productions and 119/119
scanner rules reached; style/naming and full dynamic checks pass. Independent
review's Generic EOF finding was reproduced, fixed and rechecked. Archive builds
with generators disabled; archive and installed binaries pass conformance and
live diagnostics; real coc.nvim integration passes. See
[the report](../../icl-syntax-properties.md) for commands and limitations.
