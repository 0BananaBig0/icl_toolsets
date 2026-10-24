# icl_toolsets documentation

The [main README](../README.md) is the user guide: installation, command-line
usage, editor setup, supported features and practical limitations.

## Development and acceptance

| Document | Purpose |
|---|---|
| [Project requirements](requirements.md) | Current product, architecture, quality, testing and delivery requirements. |
| [Combined language/editor audit](combined-language-audit.md) | Sequential ICL/PDL block revalidation, upstream comparison, highlighting fix and verification. |
| [PDL tools](pdl-tools.md) | Separate Tcl/PDL architecture, static dependency build, analysis limits and verification. |
| [PDL block review](pdl-block-review.md) | IEEE clauses 7–8, PDL fixtures, upstream comparison and syntax-review evidence. |
| [Developer guide](development.md) | Shared pipeline, source layout, build/test commands, `.worktrees/` layout and contribution practices. |
| [Coding style](coding-style.md) | Required Google C++ naming/layout, generator exceptions and Makefile conventions. |
| [ICL conformance review](icl-conformance-review.md) | Block-by-block IEEE/vendor evidence and compatibility decisions. |
| [Honza parser comparison](honza-icl-comparison.md) | Keyword, grammar, matching and test-case comparison with the reference repository. |
| [Independent syntax checks](icl-syntax-properties.md) | IEEE-derived metamorphic acceptance checks and live diagnostic verification. |
| [Clang methods and ICL status](clang-testing-and-icl-status.md) | LLVM comparison, systematic identifier contexts and fresh reference-parser results. |
| [Testing practices](icl-testing-practices.md) | Public parser, formatter, LSP, Flex and Bison practices used here. |
| [Formatter self-tests](formatter-self-tests.md) | Two-pass stability, saved expected outputs, CLI/LSP agreement and explicit baseline updates. |
| [Verilator testing comparison](verilator-testing-practices.md) | Adopted golden/scenario practices and test-runner safeguards. |
| [Literal parameter widths](parameter-width-validation.md) | Shared local width diagnostics, bounds and regression evidence. |
| [Verification and limits](icl-verification.md) | Dated correctness, coverage, editor and performance evidence, including unmet gates. |

## Runtime and navigation reviews

| Document | Findings and evidence |
|---|---|
| [Symbol identity and namespace scopes](review-symbol-identity.md) | Collision-free Tcl procedure keys, namespace variable frames and completion. |
| [CLI, namespace and source ownership](review-cli-namespace-copy.md) | Checked indentation arguments, namespace normalization and LSP source-copy reduction. |
| [Expressions, navigation and positions](review-expression-navigation-position.md) | Tcl expression checks, ICL member resolution and UTF-16 lookup scaling. |
| [Encoding and qualified Tcl commands](review-encoding-qualified-tcl.md) | Invalid UTF-8 handling, qualified builtins and workspace URI behavior. |
| [Runtime/corpus review](runtime-corpus-review.md) | Formatter/LSP/editor fixes, corpus verification and remaining limits. |

Read each report's scope and date; the current requirements remain the acceptance
targets. Later runtime reviews supplement the language-block audits rather than
claiming a new proof of complete language conformance.

## Fixtures and editor examples

| Document | Purpose |
|---|---|
| [Test sources](../tests/cases/SOURCES.md) | Fixture provenance and source categories. |
| [EDA corpus](../tests/cases/EDA_CORPUS.md) | Tessent/Synopsys SLM collection and source-to-module mappings. |
| [Corpus deduplication](../tests/cases/CORPUS_DEDUPLICATION.md) | Latest corpus consolidation, canonical references and retained test variants. |
| [coc.nvim LSP settings](editor/coc-lsp.json) | Recommended editor configuration. |
| [coc.nvim CLI formatter settings](editor/coc-cli.json) | Optional coc-diagnostic adapter. |

Source/licensing metadata remains alongside the test fixtures it describes.
New developer guides and project requirements belong in `docs/`.

## History

[Historical requirements and implementation record](history/requirements-legacy.md)
is the former root `REQUIREMENT.md`. It preserves the original requests and work
history; its old completion claims are not current acceptance evidence.
The [current requirements](requirements.md) and dated verification reports take
precedence when they conflict with historical descriptions.

The `superpowers/specs/` and `superpowers/plans/` directories retain task-specific
designs and execution records. A plan describes intended work; use its recorded
checks and the verification reports to determine what was actually verified.
