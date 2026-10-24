# Combined language and editor audit plan

> Execute inline using Superpowers TDD/debugging/verification, then obtain one
> independent integrated review. Continue the user's authorized current checkout.

**Goal:** Recheck ICL and PDL block by block, close reproducible syntax/editor
gaps, verify shared pipelines and install the verified tools.

**Architecture:** Retain the shared ICL ParsedDocument/AST and separate PDL
Document/command tree. CLI and LSP share formatting/analysis to their adapters.
Vim syntax remains an independent highlighter, checked against both languages.

**Tech stack:** C++17, Bison/Flex for ICL, Tcl parsing APIs for PDL, Vimscript,
Python executable/protocol tests. No new runtime dependencies or vendor execution.

**Spec:** docs/requirements.md and the user's current eight-part review request.
Baseline: 8424ac1. Existing conformance decisions and fixtures are reused.

## Constraints and review focus

- Normative IEEE clauses 6/7/8 govern; Honza and Tessent are comparison evidence.
- Enum and DataOutPort are ICL blocks. PDL is reviewed in lexical/command order.
- Complete and validate one block before moving to the next within each language.
- New cases go under tests/cases by origin; new runners mirror source paths under
  tests/. Preserve collected originals and document completed example context.
- Generated code/reports/manual extracts remain ignored. Google source style.
- Review nested expressions, escaped lexical delimiters, EOF/incomplete editor
  buffers, command/body distinction, and cross-language namespace isolation.
- Preserve failure diagnostics, byte content and formatter idempotence. Do not
  weaken existing assertions or strict coverage gates to obtain a green result.
- Supported benchmark scale is the existing 0.1–1 MiB editing workload. Reuse
  the bounded reference test and editor scan benchmarks; no speculative caching.
- No new commit/push requested in this task. Installation is explicitly requested.

## Tasks

### 1. ICL blocks and reference comparison
- [x] Re-extract the supplied IEEE PDF; verify source identity and page mapping.
- [x] Check Enum, DataOutPort, remaining port blocks, registers, expressions,
  muxes/groups/interfaces, hierarchy, aliases/parameters/attributes and lexical
  rules against source-backed manifests and current implementations.
- [x] Run tests/src/test_icl_conformance.py --block for each manifest block in
  order; stop on a failure, reproduce, fix and rerun before advancing.
- [x] Recheck pinned Honza keywords, production mapping and fixture inventory;
  use the real reference adapter if its dependencies are available.
- [x] Add missing source-backed fixtures before any production correction.

### 2. PDL blocks and recursive Tcl syntax
- [x] Review normative clauses 7–8 and separate upstream pdl.g4; compare all
  24 command names, lexical forms, optional/repeated operands and nested bodies.
- [x] Run tests/src/test_pdl_blocks.py --block in manifest order; resolve each
  failure before moving on. Extend the existing source-attributed manifest for
  missing cases, verifying CLI rejection/acceptance and matching LSP behavior.
- [x] Check shared AST ownership, dead code, repeated scans and completion/
  navigation context. Keep parsing separate from runtime execution.
- [x] Run PDL executable, syntax-property and reference-performance regressions.

### 3. Highlighting, integrated verification and installation
- [x] Compare ICL/PDL highlighter groups and lexical boundaries with reviewed
  grammar blocks; add any missing reproductions to existing Vim tests.
- [x] Independently review the integrated changes and evidence; resolve material
  findings with failing tests first.
- [x] Run formatcheck/namingcheck and full check-dynamic; build static variants
  sequentially and run focused shared CLI/LSP checks. Run coverage without
  changing thresholds; report functional and coverage results separately.
- [x] Run installation, verify installed versions/help and focused behavior,
  then print the shared ICL/PDL coc.nvim configuration.
- [x] Record findings, per-block results, upstream deltas and remaining limits
  in docs/; verify diff hygiene and absence of generated tracked outputs.

## Evidence and decisions

- Existing baseline: dynamic regression passed; strict coverage 97.4% lines /
  61.4% branches failed. All 334 ICL productions / 119 scanner rules were reached.
- Pre-flight interfaces: ICL/PDL retain separate AST/token definitions; both
  adapter pairs share formatting. Vim compares keyword inventories but does not
  define parser acceptance. No interface conflict identified.
- Cached /tmp/ieee1687-standard.txt is an HTML challenge response, not the
  standard; it must not be used as normative evidence. Read the supplied PDF.

## Final execution evidence

All listed audit actions were executed; checked boxes mean the action ran, not
that the strict coverage gate passed. Results are detailed in
[the audit report](../../combined-language-audit.md).

- ICL sequential checks: 46 categories / 2,055 cases; PDL: 22 / 192, all passed.
- Honza comparison: 75 ICL keywords, 268 ICL productions/fragments, eight ICL
  fixtures and 24 PDL commands inventoried; six real ANTLR adapter tests passed.
  All 165 reference-only accepts have existing rejection adjudications.
- Source-backed correction: iNote -comment/-status highlighting; existing IEEE
  fixture reused, standalone tests failed before the fix and passed afterward.
- Independent integrated review completed without an actionable finding.
- Style/naming and full dynamic regression passed without optional ANTLR skips.
  Vim and Neovim each scanned 39,200 files / 525,625 lines with no errors.
- Four static executables built and focused CLI/LSP checks passed.
- Strict coverage failed at 97.4% lines / 61.4% branches, identical to the
  pre-audit baseline; thresholds remain unchanged at 100% / 90%. Authored ICL
  grammar/scanner rule reachability is separately 334/334 and 119/119.
- Installed dynamic executables into /home/banana/.local/bin and verified CLI,
  LSP, help and version behavior. Installer printed combined coc.nvim settings.
- Diff whitespace and generated-file tracking checks passed. Generated evidence
  remains under ignored build/combined-audit/. No commit or push was requested.
