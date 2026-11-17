# Five-Tool Platform Refresh Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Refresh source archive and Rocky8 five-tool installation evidence.

**Architecture:** Reuse Makefile dist/install/uninstall targets and existing
SourceDistribution.check_installed_tools, LSP and generator runners; no product edit.

**Tech Stack:** Existing Make/C++/Python/tar/SSH.

**Spec:** docs/superpowers/specs/2026-11-17-platform-refresh-design.md

## Global Constraints

Parent masterf018bb2; task/platform-refresh; remote authorized lianghuaxiao fresh
/tmp/icl-platform-f018bb2-20261117. Preserve source/runtime/tests and reuse exact
Task116 local gates. No personal installation, editor config, push, dependency
change, remote static source-build, full remote check or new vendor claim.

## Review Focus

- Archive implementation/test bytes match the declared commit; list exclusions.
- Disabled Bison/Flex and absent static Tcl do not trigger hidden regeneration.
- Transferred-static ELF/hash evidence cannot be called a remote source build.
- Tests use actual installed tools and sentinel-preserving manifest uninstall.
- Current platform qualification does not waive model/strict coverage gaps.

### Task 117: Refresh packaging and platform evidence

**Files:** Documentation-only docs/pattern-generation.md,
docs/pattern-model-coverage.md and this plan/spec. Reuse existing tests; temporary
orchestration/evidence under /tmp and build/ only.

**Interfaces:** Existing make dist, install-dynamic/install-static/uninstall;
SourceDistribution.check_installed_tools(prefix, extracted, temporary),
tests/lsp_test.py, pattern-generation-check/pattern-generation-static-check.

- [x] Record task-start/parent and unchanged runtime hashes; read brief.
- [x] make dist on clean Root; verify archive membership/content vs tracked
      source, generated extras and exclusions; package/hash five static ELFs.
- [x] Hash-verified transfer to fresh remote tmp, preserve read-only dependencies.
      Reuse old command flow with current paths/hashes, serial native checks and
      fresh both generator variants/install/LSP/uninstall/ELF/library checks.
      Expected every actual gate0, all suite tests pass; retain exact logs/counts.
- [x] Fresh local temporary install smoke/uninstall for both variants; verify
      source/binary/runtime hashes and explicit Task116 gate reuse. Independent
      final evidence review and one necessary correction pass.
- [x] Document current actual evidence/limits, commit/merge master/delete branch,
      task-done/final verifier/archive/sync; no push.
