# Workspace and PDL resolution fixes — 2026-10-24

This follow-up addresses six findings reviewed from `dbe6cde`. It changes the
shared LSP workspace layer and PDL semantic indexing, not either language's
grammar, formatter layout rules, or Vim syntax definitions.

## Correctness changes

- Automatic watcher imports now honor the startup exclusions: hidden/build
  directories and directory symlinks below a workspace root. Any explicit root
  can permit a file, independent of workspace-folder order. Explicitly opened
  files remain available even when automatic indexing excludes them.
- Files use canonical filesystem identities internally. File symlinks no longer
  create duplicate module/procedure declarations. Client URIs and per-buffer
  versions/text are retained; disk events cannot overwrite open overlays.
  When two aliases have different unsaved text, semantic requests select the
  requesting buffer's overlay. Closing an alias restores a remaining overlay
  or rereads disk; retargets/deletes remove stale entries. Different physical
  files with the same declarations remain ambiguous.
- Unknown Tcl namespaces stay unknown. Their relative procedures/namespace
  variables cannot become invented globals; explicit absolute declarations
  and procedure-local arguments remain usable. This does not execute Tcl or
  infer runtime namespace values.
- Relative qualified Tcl calls can fall back to the global namespace. Absolute
  calls do not receive an unrelated fallback.
- Static `global` statements inside procedures populate the existing variable
  binding map, including qualified names and array bases. Definition,
  references, reassignment and completion share the resulting bindings.

ICL and PDL retain separate language documents and rules. Both front ends still
use their shared per-language parse/format pipeline. No dependencies were added.

## Reference lookup performance

PDL's ICL-object resolver previously scanned every declaration for every object
use. A reference query over N declarations and N uses consequently performed
quadratic work. The workspace now builds a document/module/name member index
on reindex and performs logarithmic lookups per path segment. Index storage is
linear in the number of members; rebuilding it costs O(N log N). It is rebuilt
with the existing workspace lifecycle, so removed declarations cannot remain
cached. Duplicate members still produce ambiguous results.

Reproduce with:

```sh
python3 tests/src/test_pdl_document.py --server build/pdl-lsp --benchmark
```

Environment: Linux 6.18.33.2 WSL2, glibc 2.43, GCC 16.2.1, dynamic `-O2` build.
Inputs are one ICL module with N ports and one PDL file with N reads; no PDL is
executed. Times below are three live-LSP request wall times on the same host.

| Objects | Combined bytes | Before | After |
|---|---:|---:|---:|
| 4,000 | 117,811 | 32.2–33.5 ms | 0.55–0.89 ms |
| 8,000 | 237,811 | 139.5–141.9 ms | 1.31–1.73 ms |
| 16,000 | 489,811 | 515.1–572.6 ms | 1.97–2.67 ms |
| 32,000 | 1,001,811 | 2,367.8–2,382.7 ms | 4.27–4.48 ms |

These measurements cover reference-query latency, not startup, edit latency,
or peak memory. The normal regression asserts the existing two-second budget
at 32,000 objects and checks stale-index removal and ambiguity handling.

## Tests and failures investigated

Tests extend `tests/src/test_lsp_server.py`, `test_pdl_tools.py`, and
`test_pdl_document.py`; all new inputs are self-constructed and kept with these
existing test scripts. They run through the standard regression targets.

- The initial workspace tests failed for both servers before the fix; Tcl tests
  reproduced all three scope issues. The new 32,000-object regression failed
  before indexing at 2.253 seconds against the unchanged two-second limit.
- Additional lifecycle tests exposed stale symbols after target deletion and
  wrong symbols when querying an inactive unsaved alias; both were fixed.
- One new Tcl assertion initially placed its cursor beyond the end of the
  source line. Its position was corrected; its expected definition was retained.
- Google naming/style checking caught a missing brace pair, which was corrected.
- Independent review found watcher imports through directory symlinks, explicit
  nested-root ordering, and retarget refresh on inactive-alias close. Reproducing
  tests were added, the causes were fixed, and a follow-up review found no
  remaining actionable issues. The reviewer checked both root orders and both
  servers independently.

## Verification

| Check | Result |
|---|---|
| Complete dynamic regression | Final `make check-dynamic BISON=/usr/bin/bison -j4` passed, exit 0, using the existing Honza reference checkout and ANTLR environment. All 334 ICL grammar productions and 119 scanner rules were reached. |
| Complete editor corpus | Vim and Neovim each checked 38,488 ICL files and 728 PDL files without errors; all six standalone/vim-plug/corpus tests passed. |
| Dynamic focused tests | Eight shared-server tests, 49 PDL tests and the 32,000-variable/object reference checks passed. |
| Static build and focused tests | All four static binaries built; eight shared-server tests, 49 PDL tests, 45 ICL LSP tests, both scaling checks and all 19 formatter goldens passed. Static Tcl/glibc emits the previously documented portability warnings. |
| Google style | `make formatcheck namingcheck BISON=/usr/bin/bison -j4` passed after correcting the missing brace pair. |
| Independent review | All three follow-up findings fixed; reviewer verified both workspace-root orders with both servers and reported no remaining actionable defects. |
| Strict authored coverage | Instrumented functional suites passed; `make coverage` exited 2 at its unchanged gate: 4,244/4,355 lines (97.5%) and 7,271/11,833 branches (61.4%), below 100%/90%. |

The prior strict coverage run was 4,141/4,248 lines and 7,002/11,401 branches.
This change increases covered code, but does not resolve the existing overall
coverage shortfall; no thresholds or exclusions were weakened. PDL document
coverage is now 1,311/1,357 lines, and the shared LSP header is 543/553 lines.

Logs remain in ignored `build/workspace-fixes-*.log`.

The final full run used `ICL_REF_DIR=/tmp/icl-reference-honza-20261021` and
`build/combined-audit/reference-venv/bin` on `PATH`; no optional reference-adapter
tests were skipped. Static verification was focused, while the complete
functional regression ran on the dynamic binaries. `git diff --check` and
the generated-artifact cleanliness check passed. No installation, commit or
push was performed for this fix request.
