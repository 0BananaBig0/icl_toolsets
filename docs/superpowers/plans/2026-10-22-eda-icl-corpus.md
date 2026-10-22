# EDA ICL corpus import

Use superpowers:executing-plans inline, under the user's existing authorization.

Spec: import all ICL in the three user-selected remote trees, classify by
project, and retain one copy of duplicate files/modules. Never modify originals.

Design: reuse ParsedDocument and its AST/token spans in a test-only inventory
helper. A standard-library Python importer selects canonical modules by lexical
content plus definition/import namespace; opaque Generic payloads remain exact.
Keep differing definitions, all source paths/hashes and source comments. Split
modules into individual fixtures with explicit namespace context. This corpus
tests syntax/formatting, not the elaboration of all variant designs together.
Existing coverage fixtures are outside this import's deduplication scope.

Scale: 180 paths, about 1.1 MB; parse each byte-unique input once in one batch,
use hash maps for canonical selection. No remote build or tool execution.

## Task 1: Inventory, importer and focused tests

- Write failing integration tests for duplication, variants, namespace context,
  nested/string/comment braces, opaque bodies, provenance and invalid inputs.
- Implement tests/icl_corpus_inventory.cc linked to the shared frontend and
  tests/import_eda_corpus.py. Output must be a new directory; reject invalid input
  before creating it. No new runtime features/dependencies.
- Wire build/check targets; run focused tests (expected: all pass).

## Task 2: Import and verify

- Import the collected snapshot into ignored staging; review and promote source
  fixtures and maintained provenance into tests/cases, grouped by source.
- Verify every source maps to canonical modules, no canonical duplicate remains,
  and all fixtures format idempotently with content preserved.
- Run make check-dynamic BISON=/usr/bin/bison (expected: pass).
- Independent review, resolve important findings, commit, integrate and push
  using the user's standing authorization.

## Review focus

Do not conflate same-name variants; preserve namespace semantics, opaque payloads
and comments/notices. Do not delete source files, overwrite output, invent an ICL
regex parser, omit symlinked ICL, or check in build/analysis output.

## Evidence

- Initial inventory: 179 regular files, 106 distinct byte contents; all parse.
- Follow-links inventory: one additional ICL. Broken simulator .so links and two
  Verdi directory cycles reported by rg; no missing ICL link reported.
- Completed: 180 source paths, 106 byte-unique files, 328 module occurrences,
  164 canonical definitions (119 Tessent / 45 Synopsys SLM). Fifty qualified
  names have distinct retained variants. Snapshot and reproduced fixtures match.
- RED: importer tests failed before implementation; maintained-corpus test failed
  before fixture promotion. GREEN: all five focused tests pass.
- `make check-dynamic BISON=/usr/bin/bison -j4`: exit 0; 1921 conformance cases,
  898 corpus assertions, existing LSP suites pass; 334/334 Bison productions and
  119/119 Flex rules over 2199 files. This is rule coverage, not full branch
  coverage of all handwritten C++.
- `/usr/bin/time` import of the 180-path, approximately 1.1 MB local snapshot:
  0.15 seconds elapsed, 26224 KiB peak RSS on this Linux workspace; includes
  Python and the shared C++ frontend. No comparative speedup claimed.
- Independent final review found no substantive issues; independently reproduced
  all source mappings/fixtures and probed namespace resets and opaque payloads.
- No runtime code changed, so no executable reinstallation was needed.
