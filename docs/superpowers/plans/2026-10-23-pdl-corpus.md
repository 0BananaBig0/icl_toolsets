# PDL vendor corpus implementation plan

> **For agentic workers:** Use superpowers:executing-plans inline under the user's standing autonomous-work authorization.

**Goal:** Collect every PDL file found under ~/remote_eda/lianghuaxiao into classified, deduplicated local regression fixtures and test both PDL tools.

**Architecture:** Preserve complete original streams. Deduplicate by exact SHA-256 bytes, retaining all source paths in a maintained provenance manifest. Reuse the existing Python LSP client and PDL executable-boundary tests. Do not execute source.

**Tech Stack:** Python standard library, existing C++17/Tcl parser, GNU Make.

**Spec:** Existing PDL design at ../specs/2026-10-23-pdl-tools-design.md plus the user's explicit corpus request.

## Constraints and review focus

- Remote files are read-only. Import a temporary snapshot; preserve notices and exact source bytes.
- Classify Tessent versus Synopsys SLM by original source; copied Tessent examples inside syn_slm remain Tessent.
- Never deduplicate individual iProc definitions: binding, order and redefinitions affect meaning.
- Keep all cases under tests/cases/pdl_eda and scripts under tests/.
- Treat IEEE rules and vendor extensions separately; consult references before fixing parser false rejections.
- Runtime-generated bindings must not produce invented navigation targets.
- Check both CLI stability and real LSP diagnostics/formatting; never evaluate imported source.
- Existing strict coverage thresholds remain unchanged. Do not claim full PDL conformance.

### Task 1: Import and provenance

- [x] Add failing executable-boundary importer tests for deduplication, source preservation, source classification and refusal to overwrite.
- [x] Implement tests/import_pdl_corpus.py using existing hashing/category helpers where applicable.
- [x] Import all 114 discovered paths, verify exact bytes/hashes and document canonical mappings.

### Task 2: Corpus regression and parser fixes

- [x] Add full-corpus formatter idempotence and LSP parity checks to the regular pdlcheck target; observe real failures.
- [x] Reduce false rejections to focused cases, compare IEEE/Tessent references, fix shared PDL pipeline with RED→GREEN tests.
- [x] Ensure dynamic bindings remain unresolved without rejecting syntax.

### Task 3: Verification and report

- [x] Run importer tests, PDL functional/corpus suites dynamically and statically, then existing ICL regressions and style checks.
- [x] Run current coverage and record unmet gates without hiding failures.
- [x] Obtain one independent read-only review, address material findings, update provenance and developer reports.
- [x] Reinstall changed tools under existing authorization, verify installed executables and retain printed editor configuration.

## Final evidence

- Full dynamic regression + formatcheck + namingcheck passed; five optional ANTLR integrations skipped (ICL_REF_DIR unset). ICL grammar/scanner rule reachability remains334/334 and119/119.
- Importer3/3, functional PDL26/26, corpus73positive+6expectednegative passed against build and installed dynamic/static binaries.
- All114 origin paths and original bytes matched the snapshot;35 redundant copies omitted.
- One independent read-only review completed; its unknown namespace/prefix navigation finding was fixed with3RED→GREEN cases and explicit reset checks. No deferred minor findings.
- Dynamic installation refreshed at /home/banana/.local; static at build/install-static. All8installed binaries match builds, return versions; static ELF binaries have no INTERP/NEEDED entries. coc.nvim configuration was printed by each install.
- Strict coverage remains unmet:3517/3638 lines96.7%,5693/9649 branches59.0%. Thresholds unchanged; no commit/push. Corpus collection/testing is complete, while earlier implementation integration remains gated by coverage.
- Imported PDL was not evaluated, retargeted or run in vendor tools; no new real coc.nvim session was tested.
