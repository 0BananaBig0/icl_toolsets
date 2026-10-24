# Runtime and public corpus review — 2026-10-24

Reviewed from `cab5af6`, continuing the five-component audit recorded in
[combined-language-audit.md](combined-language-audit.md). This pass targets
protocol handling, preservation of documents, formatter boundaries, editor
lexical rules and additional public inputs. It is not a new proof of complete
IEEE language conformance.

## Confirmed defects and fixes

1. **Both language servers responded to client responses.** With dynamic file
   watching enabled, a client's reply to `client/registerCapability` was treated
   as a request with an empty method. The server emitted an unsolicited
   `-32601` error with the registration ID. The shared handler now recognizes
   response envelopes before dispatching requests. The regression exercises
   both successful and failed registration replies, followed by a valid request
   to verify the connection remains usable.
2. **A ranged change replaced a full-sync document.** Both servers advertise
   full text synchronization, but accepted the `text` of a ranged change as the
   entire buffer. This discarded symbols and made later formatting operate on
   a fragment. Ranged changes are now ignored; the document and version remain
   unchanged. A following valid full replacement at the rejected change's
   version is accepted. Incremental synchronization is not advertised or added.

The two tests failed for both servers before the shared fix and passed after
rebuilding. An independent review examined the handler, JSON lookup behavior,
version tracking and test transport and found no actionable issues. A follow-up
independent review also verified corpus hashes, retained licenses, the exact PDL
completion, test integration and LSP assertions with no actionable findings.

## Corpus and downstream checks

- Added the [warptap wrapper chain](../tests/cases/pass/warptap/README.md):
  8,076 original bytes, five module definitions absent from the existing corpus.
  The smaller block-only upstream fixture was excluded because its two module
  definitions occur in the chain. CLI acceptance, preservation and idempotence
  use the existing corpus runner; LSP diagnostics use the conformance manifest.
  A new semantic test checks definition, implementation and member completion
  through the public hierarchy.
- Added [Origen PDL original and completed inputs](../tests/cases/upstream/pdl/origen/README.md).
  The original lacks `iProc` argument lists and is rejected under IEEE
  1687-2014 §7.9.5, printed 160/PDF 174. The positive adds empty lists and a
  level/version declaration. Both use the existing procedure-block runner for
  CLI/LSP acceptance and formatting parity. All 21 upstream ICL examples were
  already represented, so no copies were added.
- Added explicit Vim/Neovim syntax probes on the public fixtures. Both editors'
  collected-file runner also visits every new file. Public PDL is only parsed
  and highlighted, never evaluated. Source revisions, hashes and MIT/Apache-2.0
  notices are retained beside the fixtures.
- The maintained ICL deduplication check reports zero duplicate-file groups and
  no new module repetitions; its 25 previously reviewed repetition groups are
  unchanged. Both new PDL streams are byte-unique across `tests/`.

## Structure and performance decisions

ICL retains its shared `ParsedDocument`/AST, semantic workspace and formatting
pipeline. PDL retains its separate Tcl-backed `Document`/command tree and
analysis/formatting pipeline. The fixes are in the existing common LSP transport
and dispatch layer, so both languages receive the same boundary checks without
sharing grammar or token definitions. Formatter and Vim production code needed
no changes for the reproduced issues or the additional inputs.

The protocol guards add constant work per message and no new scans, caches or
subprocesses. Full document reparsing and workspace reindexing still occur on
accepted edits; this pass does not introduce a speculative incremental engine
or claim a measured speedup. Large-workspace interactive latency remains a
separate measurement task. Existing atomic-write, Unicode, nesting, malformed
input and formatter stability checks remain part of regression.

## Verification

Focused protocol, public-hierarchy, PDL procedure and corpus deduplication checks
passed. Static builds and focused checks passed: 12 CLI tests, four shared-server
tests, 45 ICL LSP tests, 46 PDL tool tests, 201 PDL block cases and the PDL document
scaling check. Google formatting and naming checks passed.

The first full dynamic run passed the language/tool suites but failed the new
editor probe: `Enum blk_ops` also matched `RefEnum blk_ops`. The probe was
corrected to the unique declaration `Enum blk_ops {`; production syntax code was
not changed to accommodate a test mistake. The complete editor rerun passed in
185.8 seconds: each of Vim/Neovim checked 38,488 ICL files, 722 PDL files and
50 explicit syntax probes, plus standalone/vim-plug and approximately 1 MiB
input checks. The final complete dynamic rerun then passed with exit 0, including
2,056 conformance cases, 201 PDL block cases, 438 PDL syntax properties, all
334 grammar rules and 119 scanner rules, and the whole editor corpus (173.1
seconds for the final editor run). No optional reference-adapter tests were
skipped; the existing ANTLR reference environment was supplied.

`make coverage BISON=/usr/bin/bison -j4` ran its functional tests successfully,
then failed the unchanged strict coverage gate: **4,141/4,248 lines (97.5%)** and
**7,002/11,401 branches (61.4%)**, below 100%/90%. The authored grammar remains
570/570 lines, the scanner 211/213, and `pdl_document.cc` 1,286/1,329. This is an
explicit remaining test-coverage gap, not a passing result or evidence of full
syntax support. Generated C++ is excluded by the existing coverage reporter.

Commands and retained local evidence:

```sh
make formatcheck namingcheck BISON=/usr/bin/bison
ICL_REF_DIR=/tmp/icl-reference-honza-20261021 \
  PATH="$PWD/build/combined-audit/reference-venv/bin:$PATH" \
  make check-dynamic BISON=/usr/bin/bison -j4
make static BISON=/usr/bin/bison -j4
python3 tests/src/test_format_cli.py --icl build/icl-format-static --pdl build/pdl-format-static
python3 tests/src/test_lsp_server.py --icl build/icl-lsp-static --pdl build/pdl-lsp-static
python3 tests/src/test_icl_lsp.py --binary build/icl-lsp-static
python3 tests/src/test_pdl_tools.py --formatter build/pdl-format-static --server build/pdl-lsp-static
python3 tests/src/test_pdl_blocks.py --formatter build/pdl-format-static --server build/pdl-lsp-static
python3 tests/src/test_pdl_document.py --server build/pdl-lsp-static
make coverage BISON=/usr/bin/bison -j4
python3 tests/tree_clean_test.py
```

Logs remain ignored under `build/review-public/`: `regression-final.log`,
`static.log`, `style.log`, `editor-recheck.log`, and `coverage.log`. Coverage
reports remain under `.worktree/coverage/`. No generated reports, reference
checkouts or binaries are added to Git. This review does not install tools or
change personal editor settings, and does not commit or push changes.
