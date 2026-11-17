# Cached-Wait Package Qualification Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Refresh source/archive and five-tool installation evidence after Task123.
**Architecture:** Reuse existing dist/install/uninstall and installed-tool runners.
**Tech Stack:** Existing Make/C++/Python/tar/SSH; no product edit.
**Spec:** docs/superpowers/specs/2026-11-17-platform-cached-refresh-design.md

## Global Constraints

Parent master2b5eec1; task/platform-cached-refresh. Documentation only in four
files below. Fresh own remote /tmp/icl-platform-2b5eec1-20261117. Preserve old
artifacts; pinned dependencies read-only. Reuse exact Task123 full/SAN/coverage.
No user install, new dependency, remote static source build/full make check,
vendor certificate, strict coverage waiver or push.

## Review Focus

- Source archive content matches clean base; all exclusions/extras accounted for.
- Disabled Bison/Flex/static Tcl cannot mask regeneration or missing dependencies.
- Transferred static ELF/hash evidence is distinguished from a source build.
- Installed tools and manifest uninstall preserve unrelated sentinel files.
- Scope/strict coverage limitations remain explicit after current qualification.

### Task 124: Refresh current packaging and platform evidence

**Files:** docs/pattern-generation.md; docs/pattern-model-coverage.md; this plan;
docs/superpowers/specs/2026-11-17-platform-cached-refresh-design.md.
**Interfaces:** make dist/install-dynamic/install-static/uninstall;
SourceDistribution.check_installed_tools(prefix,extracted,temporary);
tests/lsp_test.py; pattern-generation-check and pattern-generation-static-check.

- [x] Record task-start/parent, read brief; make dist from clean Root, compare
      archive membership/content with base, generated extras and exclusions;
      hash/package all five static ELFs. Expected current runtime3994 and products.
- [x] Transfer verified archives into fresh own remote tmp; execute established
      dynamic build/native/installed tools/LSP/generator/uninstall and transferred
      static ELF/install/tools/LSP/generator/uninstall gates. Expected16 gates0,
      51 generator suites/780 cases per variant and installed LSP62 each.
- [x] Run fresh local temporary five-tool installations/smoke/LSP/uninstall for
      dynamic/static. Expected11 gates0, fingerprints and sentinels preserved.
      Verify exact runtime/product fingerprints and Task123 evidence reuse.
- [x] Document actual outcomes/limits and freeze four-file candidate; perform
      one independent final evidence review and necessary correction pass.
- [x] Verify final bytes, commit/merge master/delete branch/task-done/archive/sync.

Pre-flight: no product interfaces changed; existing runner signatures reused.
Design/plan self-review complete; routine execution delegated by standing user.
TDD ruling: qualification/docs-only work adds no behavior; use existing checks
and explicit provenance assertions rather than inventing a failing product test.


### Cached-wait package qualification (2026-11-17)

Source-archive and five-tool qualification now cover clean master `2b5eec1`,
including the cached oscillator-wait repair. All 4,217 shipped tracked files
match that checkout; only `.gitignore` is excluded. Six generated parser/scanner
sources/headers/location/`.prebuilt` extras are recorded separately. Source
archive SHA256 `7cfbde0b39dbec6b0b7b97efd2c6d2a947685dadffbb7ae0855babfaa5b44ebb` and five-binary static archive
SHA256 `e8c794849779a0b352c6e1d6cfee96623ab3ac45b62091872b26c07f1eff41df` are verified before extraction on Rocky Linux 8.10.

The fresh archive builds five dynamic tools with Bison/Flex disabled and static
Tcl unavailable, reusing pinned Make/GCC/Z3 dependencies read-only. All 16 remote
gates pass: native netlist/runtime/flatten targets, installed versions and PDL/pattern/reset replay,
installed ICL LSP 62 checks per variant, dynamic libraries, transferred-static ELF
checks and sentinel-preserving manifest uninstall. Both generator variants pass
all 51 suites/780 cases. All five transferred static executables match the local
hashes and lack ELF interpreters; these are local builds, not remote static
source builds. Environment and exact system Tcl 8.6 resolutions are preserved. Initial
extraction/build clock-skew warnings (~21s) are retained; all objects were built
in the fresh extraction and all build/test gates exited 0.
All 11 fresh local temporary installation gates pass for the same ten products.

All 3,994 runtime input hashes match Task 123 exactly. Its complete normal
integration passes (1871.732309s, including six editor tests), focused leak-enabled
ASan/UBSan native/clock (53)/main (71) checks and fresh raw coverage are reused by
hash. Optional ANTLR checks remained skipped when unavailable.
Coverage remains FAILED at 17,333/17,963 lines (96.5%) and 29,260/46,667 raw
branches (62.7%), against 100%/90%; no waiver or new coverage result is claimed.
No product code, test, dependency, user installation or editor config changes.
Raw commands/statuses, archive membership, hashes and remote/local evidence are
in `build/qualification-2b5eec1/`; Task 123 evidence remains immutable in
`build/oscillator-cached-20261117/`. This does not claim a full remote make check,
a new vendor certificate, or complete model support. No push.
Independent final review found no Critical, Important, or Minor issues in this package qualification refresh. The current package qualification record is ready for integration.
