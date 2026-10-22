# ICL Conformance and Language Services Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Correct all ICL grammar blocks and complete coc.nvim language services, with source-backed tests and verified installation.

**Architecture:** Keep one scanner/parser, AST definition and shared document-processing chain. Add precise source locations and semantic declarations/references to the shared model for LSP navigation; never parse ICL separately in the LSP.

**Tech Stack:** C++17, Bison 3.8.2, Flex 2.6.4, Python standard-library tests, gcov.

**Spec:** ../specs/2026-10-21-icl-conformance-design.md; approved by Proceed, with the user's subsequent clarification that LSP must support completion, definition and implementation navigation.

## Global Constraints

- IEEE 1687-2014 normative clauses precede informative grammar summaries and third-party implementations.
- Complete Enum before DataOutPort; then review each remaining block sequentially, including dependencies and recursion.
- Preserve evidence-backed extensions and the original merged ICL files. No PDL parser or shared ICL/PDL token set.
- All new test scripts mirror tested paths under tests/; corpus is categorized by provenance and block.
- Use BISON=/usr/bin/bison here. Static libraries are absent at baseline; resolve or report that accurately.
- No remote publication or automatic edits to the user's editor settings.

## Review Focus

- Keyword identifiers, multiline strings and inter-token comments must survive formatting (tasks 1–8).
- Invalid members, empty required blocks and illegal nesting must be rejected without overwriting files (tasks 1–7).
- Names must resolve by module, instance hierarchy and namespace across files, not global text matching (task 9).
- Incomplete editor buffers, UTF-16 positions and document changes must not break language services (task 9).
- Coverage must count missing/zero reports and distinct same-line productions honestly; large inputs must not introduce quadratic behavior (task 10).

## Tasks and completion contracts

Each grammar task uses: source review → persisted positive/negative fixtures → observe baseline failure → minimal shared-code fix → cumulative conformance and affected legacy tests → evidence record and commit. No later block is implemented before the current block's targeted checks pass.

### Task 1: Enum and conformance harness

Files: src/icl_parser.yy, src/icl_lexer.l; tests/src/test_icl_conformance.py; tests/cases/conformance.json; tests/cases/{standard,tessent,upstream,ours}/enum/; docs/icl-conformance-review.md.
Interface: `python3 tests/src/test_icl_conformance.py --binary build/icl-format [--block enum]`; manifest fields path, block, accept, source. Accepted inputs must format, preserve content and be idempotent; rejected inputs return 2 with empty stdout.

- [x] Read §6.5.3 and dependent lexical rules, Tessent Enum topic and Honza enum rules.
- [x] Add completed examples, keyword names, concatenation/inversion/comments, empty/nested/bad-value negatives; observe RED.
- [x] Add required Enum item grammar producing existing AstBlock/AstStatement; fix dependent lexical discrepancies; run cumulative tests, update affected old tests only with evidence, commit.

### Task 2: DataOutPort and remaining ports

Files: scanner/parser, per-port fixtures/manifest, tests/block_review.py, review record.
- [x] Audit §6.4.6.7, test its legal shapes and illegal members/counts, fix, verify.
- [x] Repeat separately for DataInPort, ScanInPort, ScanOutPort, each control/clock/reset host/client port. Share only identical semantic rules.
- [x] Verify source preservation, ranges, attributes and all cumulative blocks; commit.

### Task 3: ScanRegister and DataRegister

- [ ] Audit §6.4.8 then §6.4.9, persist examples and invalid alternatives, observe RED, correct bodies including selectable/addressable and callback nesting.
- [ ] Cover every alternative and dependent expression, run cumulative checks and merged corpus; commit.

### Task 4: LogicSignal, ScanMux, DataMux, ClockMux

- [ ] Audit §§6.4.10–13 individually; persist recursive expression, selector, operator and invalid-shape cases before corrections.
- [ ] Implement unambiguous expression/selection rules and only evidenced legacy forms; verify conflict-free Bison and cumulative tests; commit.

### Task 5: OneHot groups, ScanInterface and Chain

- [ ] Audit §§6.4.14–16 individually; test required items, nested instances/chains and invalid nesting before corrections.
- [ ] Replace generic body acceptance with appropriate productions, run cumulative tests and merged corpus; commit.

### Task 6: Instance and AccessLink

- [ ] Audit §6.4.7 then §6.4.17; cover module references, namespace scopes, input connections, parameter overrides and generic/1149.1 forms.
- [ ] For each missing/rejected form add RED test, correct it and validate cumulative suite; commit.

### Task 7: Alias, parameters, attributes, namespaces and module structure

- [ ] Audit §§6.5.2, 6.5.4–5 and 6.4.2–5 sequentially with source-backed cases and RED→GREEN corrections.
- [ ] Remove unsupported fallback productions and unproven extensions, verify keyword identifiers and local cardinality constraints; commit.

### Task 8: Cross-cutting lexer, upstream comparison and provenance

- [ ] Close §6.3 lexical coverage including bases, widths, delimiters, strings, comments, whitespace, EOF and recursive limits using regression fixtures before fixes.
- [ ] Compare every Honza ICL keyword/production and embedded Python input at revision 333f8bc7a54964138ac14a34fde9e942c496436d. Record matchability differences and normative adjudication; import applicable missing cases with source/license.
- [ ] Fix tests/antlr_diff.py to count lexer errors and require full input consumption; distinguish absent dependencies from success.
- [ ] Record public Flex/Bison/parser/formatter/LSP test practices and adopted methods; reclassify erroneous old positive tests with evidence rather than disabling them.
- [ ] Run complete cumulative corpus and grammar checks; commit.

### Task 9: Shared semantic document and real LSP services

Files: src/icl_ast.h, src/icl_driver.{h,cc}, src/format_job.{h,cc}, scanner/parser actions, new src/icl_semantics.{h,cc} only if justified, src/icl_lsp_main.cc, Makefile; tests/src/test_icl_lsp.py and classified multi-file fixtures.
Interfaces: shared parsed document owns AST, exact token/source spans, declaration/reference information; formatting and symbol queries consume that same parse. LSP alone converts results to JSON/UTF-16 positions.

- [ ] Add protocol tests that currently fail for completion, definition, implementation, hover, references and document/workspace symbols.
- [ ] Cover module/instance/port/register/parameter/enum/alias names, qualified references, namespace shadowing, unopened workspace files, invalid/incomplete edits, non-ASCII positions and updates/removal.
- [ ] Extend shared AST/document metadata and scoped resolution; no regex-only LSP parser. Preserve the formatting pipeline and late frontend split.
- [ ] Implement completion (keywords plus in-scope and instance members), definition/declaration lookup, implementation lookup to ICL module bodies, hover, references and symbol lists. Define absence honestly: no inference of unavailable HDL implementations.
- [ ] Add safe rename/prepareRename only with scoped-reference tests and versioned edits; do not advertise unsupported capabilities.
- [ ] Validate CLI/LSP formatting equivalence, lifecycle and coc integration, document actual capabilities and unfinished-feature audit; commit.

### Task 10: Coverage, performance, review and installation

Files: tests/coverage.py, tests/grammar_coverage.py, new tests/tests/test_coverage.py and test_grammar_coverage.py; Makefile; tests/src benchmark; README.md and review record.
- [ ] Add failing coverage-reader tests for missing reports, starred/zero counts, never-executed branches and same-line productions; correct accounting, include handwritten headers/actions and exclude generated implementation.
- [ ] Measure baseline/final merged and scaled inputs with same toolchain and parameters; record wall time and RSS. Fix proven expensive paths and retain correctness checks.
- [ ] Review all shared/CLI/LSP handwritten code and build/install safety using CodeGraph then current source; reproduce material defects before changes.
- [ ] Run complete dynamic/static build, all tests, truthful line/branch and grammar coverage, editor integration. Resolve known toolchain limitations or report them explicitly.
- [ ] Obtain one independent whole-change review, fix material findings with RED→GREEN, run required final checks.
- [ ] Install dynamic pair, verify installed paths/version/behavior and print tested coc.nvim LSP config plus documented CLI formatter adapter alternative. Leave existing editor config untouched.
- [ ] Commit validated changes and report measured results, unsupported semantic/HDL limits and any genuinely incomplete work. No push requested.

## Execution record

Proceed approved implementation; the later LSP clarification explicitly expands its deliverable. Execution stays inline for sequential block ownership. Record progress and decisions in docs/icl-conformance-review.md and the plan ledger; do not stop between ordinary tasks.
