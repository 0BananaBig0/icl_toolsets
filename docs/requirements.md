# Project requirements

These are the acceptance targets for **icl_toolsets**, not a claim that every
target has been met. The installed executables remain `icl-format` and
`icl-lsp`. See [verification and limits](icl-verification.md) for dated evidence
and [corpus deduplication](../tests/cases/CORPUS_DEDUPLICATION.md) for the later
corpus audit. The [historical record](history/requirements-legacy.md) preserves
the original requests and implementation history.

## Product behavior

- **P1 — Formatter:** `icl-format` must format ICL as a language-aware formatter,
  comparable in purpose to clang-format. Support files, standard input, standard
  output, a separate output file, in-place updates, indentation options and a
  non-mutating CI check. Preserve comments, strings and language meaning;
  formatting must be deterministic and idempotent. Report invalid input and I/O
  errors clearly, with reliable exit statuses and without damaging input files.
- **P2 — Language server:** `icl-lsp` must provide coc.nvim-compatible diagnostics,
  formatting, completion, definition/declaration navigation, implementation
  navigation, references, hover, document symbols and workspace symbols. Resolve
  scoped and cross-file ICL names, including unopened workspace documents, and
  update results when files or buffers change. Implementation navigation means
  finding ICL module definitions; do not imply HDL reconstruction or vendor
  elaboration. Ambiguous names must not produce invented targets.
- **P3 — Protocol correctness:** respect LSP framing, UTF-16 positions, document
  versions, request/notification semantics and error responses. Malformed input
  must not corrupt subsequent messages or leave stale document results. Keep
  diagnostics off protocol stdout. Bound resource use for hostile input.

## Language conformance and references

- **L1 — Complete ICL syntax:** support IEEE 1687-2014 ICL grammar, lexical rules,
  keywords and the associated validity constraints implemented by the tools.
  Review one complete syntax block before proceeding to the next: for example,
  Enum before DataOutPort. Continue through all port, register, mux, group,
  instance, alias, parameter, namespace, AccessLink and expression constructs.
  Include recursive/nested productions, optional forms, identifier boundaries,
  numeric/string literals, comments, malformed forms and error paths. Record
  clause references, findings, fixes and tests in the
  [conformance review](icl-conformance-review.md).
- **L2 — Separate languages:** ICL and PDL have different grammars, lexical rules
  and keyword sets. Never reuse a common token definition or mix their rules.
  If PDL support is added, give it separate Flex/Bison sources and tests. There
  is currently no PDL parser; this requirement does not request creating one.
- **L3 — Reference authority:** use the normative IEEE clauses to resolve
  correctness questions. Informative examples, Tessent manuals, real tool output
  and third-party parsers provide additional evidence, not replacements for the
  standard. Keep vendor extensions explicitly identified. Preserve the supplied
  tool-produced, user-modified merged ICL as a regression baseline. Use online
  sources or mirrors when necessary and record source/version information.
- **L4 — Independent comparison:** compare
  [Honza255/icl_parser](https://github.com/Honza255/icl_parser) in three ways:
  tests it has that we lack, keywords it defines that we lack, and grammar or
  regex matching capabilities for which we have no equivalent. Check each gap
  against IEEE 1687, implement justified missing support and record dispositions
  in the [comparison](honza-icl-comparison.md). A keyword count alone is not a
  grammar audit.

## Shared architecture and code quality

- **A1 — One AST and pipeline:** both executables must share the AST definition
  and implementation, parser, document analysis and formatting logic. Share the
  processing chain as far toward its end as their functions allow; do not split
  early and duplicate equivalent operations. CLI argument/options structures,
  file handling and LSP transport/workspace handling may differ. Reuse parsed
  documents where appropriate. See the [developer guide](development.md).
- **A2 — Maintainability:** identify and remove incorrect, dead or redundant
  code; reuse existing helpers and prefer straightforward implementations.
  Refactoring is permitted when it improves correctness or structure. Preserve
  validation and error handling and avoid speculative abstractions. Use
  CodeGraph for structural discovery when available; current source and
  reproducible checks take precedence over stale indexes.
- **A3 — Performance:** verify representative real merged ICL and larger/deeper
  inputs. Record input size, command, environment, wall time and peak memory for
  performance claims. Avoid repeated parsing/scans and unnecessary per-file
  subprocesses where batching or shared state fits the workload. Report tested
  scale and limits; no fixed latency target has yet been specified.
- **A4 — Google style:** handwritten `src/` must follow Google C++ style for
  file, type, class, function, variable and constant naming, header structure,
  class interfaces, comments and layout. Embedded C++ follows the same rules;
  generator-defined interfaces keep their required names. Apply the documented
  Makefile conventions while preserving GNU Make semantics. Maintain style
  checks and review the aspects they cannot enforce. See
  [coding style](coding-style.md) for exact conventions and tooling boundaries.

## Tests, provenance and coverage

- **T1 — Handwritten logic:** aim to execute every handwritten grammar/scanner
  rule and action, including its success and error paths, and maximize coverage
  of all handwritten code. Generated Bison/Flex implementation is excluded;
  handwritten logic in `.yy`/`.y` and `.l` files is not. Measure rule hits and
  line/branch coverage separately. The current strict gate is 100% lines and
  90% raw branches; it remains unmet in the last recorded measurement. Hitting
  all productions/rules does not establish full action or branch coverage.
- **T2 — Behavioral checks:** include positive/negative grammar cases, nesting,
  lexical boundaries, malformed input, golden formatting, content preservation,
  idempotence, CLI errors and file safety. Test LSP framing, malformed messages,
  UTF-16, document lifecycle, workspace resolution and editor integration.
  Borrow applicable public Flex/Bison/parser/formatter/LSP practices, documented
  in [testing practices](icl-testing-practices.md).
- **T3 — Example sources:** turn IEEE grammar/regex examples and Tessent examples
  into test inputs. Complete partial examples with the minimum valid context
  and identify those additions. Keep fixtures in `tests/`, categorized by source
  (IEEE, Tessent, named repositories, Synopsys SLM, self-constructed) and syntax
  block as appropriate. Record origin, license and modifications. New test
  scripts must mirror the tested source path beneath `tests/`.
- **T4 — Real EDA corpus:** include collected ICL from the requested
  `tessent_ijtag`, `tessent_2026.2_usage_examples` and `syn_slm` sources.
  `test_syn_slm_icl` under `tessent_ijtag` belongs to Synopsys SLM; the other
  inputs there belong to Tessent. Never modify external source originals.
  Detect duplicate file contents and namespace-aware module definitions;
  consolidate redundant objects to one canonical copy and preserve source
  mappings. Same-named but different modules are not duplicates. Keep only
  explicitly justified, hash-pinned repeats needed for independent examples,
  formatter variants or other distinct assertions. Treat malformed fixtures
  conservatively. Follow the [corpus policy](../tests/cases/CORPUS_DEDUPLICATION.md).
- **T5 — Honest verification:** run focused checks and the broader checks required
  by the change; record commands, outcomes and skipped/unavailable suites. Do
  not infer full regression or editor success from compilation, schema checks,
  partial suites or historical results. Known acceptance gaps stay visible.

- **T6 — Independent syntax oracle:** complement production/rule coverage with
  IEEE-derived metamorphic tests whose expected acceptance does not come from
  our grammar. Require valid whitespace/comment and recursive-index variants to
  parse, and nested block comments to fail. Exercise every classified corpus
  expectation through live LSP edits. Report exclusions and avoid treating a
  finite corpus as proof of full language support.
- **T7 — Live syntax checking:** share the CLI parser and AST; publish diagnostic
  versions and UTF-16 token/EOF ranges, ignore stale edits, and clear errors on
  correction or close. Preserve the documented first-error policy and line-only
  fallback for semantic checks that lack columns.

## Build, delivery and documentation

- **D1 — Build:** provide dynamic and static variants of both executables and
  target deployment on Rocky Linux 8 and newer compatible environments. Git
  builds require C++17, GNU Make 4.3+, Bison 3.8+ and Flex 2.6+. Static variants
  require static runtime libraries; dynamic builds must work without them.
  Source archives must build without invoking Bison/Flex by including generated
  parser/scanner sources in the archive. Verify compatibility on the actual
  target before claiming it; an archive build on a newer host is insufficient.
- **D2 — Git hygiene:** do not track automatically generated parser/scanner
  output, binaries, build trees, distribution archives, coverage reports, caches,
  CodeGraph outputs or generated stress data. Hand-maintained scripts and the
  explicitly requested vendor/example test inputs and provenance are maintained
  source material. Distribution-only generated files stay outside Git.
- **D3 — Installation:** install both commands with a configurable prefix,
  defaulting to `$HOME/.local`, record installed paths and support uninstall.
  For runtime releases, execute installation commands and verify the installed
  programs, then print coc.nvim configurations for both the LSP and CLI
  formatter integration. Do not overwrite personal editor settings. Distinguish
  the tested LSP setup from the optional CLI adapter's unverified end-to-end
  behavior.
- **D4 — Documentation and license:** use `icl_toolsets` for repository identity
  and retain the executable names `icl-format` and `icl-lsp`. The root README
  must explain features, limitations, installation and use for customers.
  Developer guides and project requirements belong in `docs/`; fixture
  provenance/licenses may stay beside their inputs. Keep the MIT project
  license and third-party notices. Document package names according to the
  actual build output rather than renaming them implicitly.

## Acceptance evidence map

| Requirements | Main evidence/checks |
|---|---|
| P1, L1–L4 | Conformance and Honza comparison records; `make check-dynamic`, golden and CLI tests. |
| P2–P3 | LSP semantic/protocol suites; `make coccheck`; `make nvimcheck` when claiming that client. |
| A1–A4 | Source review, shared AST/unit checks, performance measurements, `make formatcheck`, `make namingcheck`. |
| T1 | `make gramcov` and `make coverage`, with distinct rule/line/branch results. |
| T2–T5 | Source manifests, corpus audit, `make corpuscheck`, dated test reports and limitations. |
| D1–D3 | Dynamic/static builds where supported, `make dist`, extracted archive build with generators disabled, installed-path smoke tests. |
| D4 | README/docs link checks, package contents, license/provenance review. |

See the [documentation index](README.md) for the maintained records. New evidence
must state its scope and date; a requirements checklist is not itself a test result.
