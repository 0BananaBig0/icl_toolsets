# icl_toolsets documentation

The [main README](../README.md) is the user guide: installation, command-line
usage, editor setup, supported features and practical limitations.

## Development and acceptance

| Document | Purpose |
|---|---|
| [Project requirements](requirements.md) | Current product, architecture, quality, testing and delivery requirements. |
| [Developer guide](development.md) | Shared processing pipeline, source layout, build/test commands and contribution practices. |
| [Coding style](coding-style.md) | Required Google C++ naming/layout, generator exceptions and Makefile conventions. |
| [ICL conformance review](icl-conformance-review.md) | Block-by-block IEEE/vendor evidence and compatibility decisions. |
| [Honza parser comparison](honza-icl-comparison.md) | Keyword, grammar, matching and test-case comparison with the reference repository. |
| [Testing practices](icl-testing-practices.md) | Public parser, formatter, LSP, Flex and Bison practices used here. |
| [Verification and limits](icl-verification.md) | Dated correctness, coverage, editor and performance evidence, including unmet gates. |

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
