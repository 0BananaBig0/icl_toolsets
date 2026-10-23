# PDL corpus from local EDA projects

Read-only discovery on 2026-10-23 found **114** `.pdl` paths below
`~/remote_eda/lianghuaxiao`. The SSHFS mount maps this directory to
`lianghuaxiao@192.168.6.20:/data/workspaces/lianghuaxiao`. The snapshot was
collected using remote `find -type f -iname "*.pdl" -print0` and a NUL-list tar
stream because traversal over SSHFS was slow. No remote files were changed.

| Category | Source paths | Byte-unique canonical fixtures |
|---|---:|---:|
| Synopsys SLM | 77 | 47 |
| Tessent | 37 | 32 |
| Total | 114 | 79 |

The 79 complete files total **4,483,090 bytes**; the largest is **868,139 bytes**.
35 redundant copies were omitted. SHA-256 of the complete original bytes is the
identity criterion. Copyright headers, line endings, command streams, repeated
procedure definitions and binding order are retained. Individual `iProc`
definitions are deliberately not deduplicated: their meaning depends on context
and order. Source attribution is retained; these vendor files are not relicensed
under the repository's MIT license.

`sources.json` maps every original relative path to its canonical fixture,
category and full SHA-256, and records each fixture's size. Copied
`syn_slm/tessent_example_*` examples are classified as Tessent;
`test/tessent_ijtag/test_syn_slm_icl` belongs to Synopsys SLM. Categories describe
origin; byte-identical copies share one canonical file even across categories.

## Reimport and regression

From the repository root, import a fresh local snapshot into a **new** directory:

```sh
python3 tests/import_pdl_corpus.py --input /path/to/snapshot --output /path/to/new-corpus
make pdlcheck BISON=/usr/bin/bison
```

The importer refuses an existing output. The maintained `expected_diagnostics.json`
and this README are review annotations, not generated vendor content; recreate or
review them separately when importing a new snapshot. The current full-corpus
runner verifies hashes, sizes, deduplication, provenance references, formatter
idempotence, real stdio LSP diagnostics, and CLI/LSP formatting parity. It is also
run against instrumented binaries by `make coverage`. It never evaluates PDL.

**73 files are positive cases. Six are expected diagnostic cases**: generated
Tessent MBIST `iProc` argument lists place required arguments after arguments with
defaults. IEEE 1687-2014 §7.9.5 (printed160/PDF174, rule a) requires the opposite
ordering. Entries in `expected_diagnostics.json` pin each filename, error message
and line; they require CLI rejection without output and the matching LSP error.
Original source is preserved rather than repaired or declared portable. Only the
first diagnostic is checked; later content in rejected files is not fully
validated, and vendor runtime acceptance has not been tested.

## Compatibility evidence

The corpus exposed these frontend gaps, now covered by focused regressions:

- Optional `iNote` flag, `iApply -end_in_pause`, `iTake [resource] [-persistent]`,
  and `iWrite ... -ignore_when_value_already_present {auto|off|on}` are documented
  in Tessent Shell Reference Manual v2023.4, topics `iNote`, `iApply`, `iTake`,
  and `iWrite`. Invalid or incomplete options remain errors.
- `iRunLoop -tck count` is observed in the Synopsys SLM corpus. Its acceptance is
  a corpus compatibility choice, not a claim of IEEE syntax or documented support
  in the Tessent v2023.4 manual (which shows count-first forms).
- Runtime module binding (`iProcsForModule $module`) is valid level-1 Tcl/PDL.
  A declaration is tracked independently from a statically known module name;
  procedures with unresolved bindings have no invented navigation target. Runtime
  procedure namespaces and prefixes likewise remain unknown; explicit `-` resets
  call namespace/prefix to a known default.

Reference HTML topics are under
`~/win11/Desktop/tessent_2023_doc/htmldocs/tshell_ref/topics/`:
`Command_Inote_idfc2e06a5.html`, `Command_Iapply_idd384fc82.html`,
`Command_Itake_idd8a68c5f.html`, `Command_Iwrite_id82cc6234.html`, and
`Command_Irunloop_id1c6d2724.html`.
