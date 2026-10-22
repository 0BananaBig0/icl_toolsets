# Whole-test ICL duplicate audit

The 2026-10-22 audit covers all maintained `.icl`/`.ICL` files under `tests`,
including acceptance cases, rejection cases, earlier external sources, the EDA
import and golden formatting inputs. The ignored `tests/generated` benchmark
tree is excluded because its purpose is to repeat modules at larger scales.

| Result | Count |
|---|---:|
| Original maintained ICL files | 2212 |
| Redundant whole-file copies removed | 6 |
| Imported standalone modules already in original baselines | 47 |
| Repeated Tessent mux example copies removed | 2 |
| Remaining maintained ICL files | 2157 |
| Remaining byte-identical file groups | 0 |
| Distinct accepted module definitions, before and after | 1289 |
| Retained module-match groups / extra occurrences | 25 / 35 |

`deduplication.json` records every removed path, original hash, canonical target
and module IDs. Comments from removed non-identical fixtures are preserved there;
the original EDA source comments also remain in `eda_sources.json`. All original
EDA source classifications and module mappings are retained. An EDA definition
can now reference an existing multi-module baseline through its path and module
ID. The seven pinned Tessent baseline files remain byte-identical.

Conformance rows point to canonical paths. Repeated rows for the same path,
block and acceptance expectation are collapsed, while different grammar blocks
can still exercise the same shared input. There are 1876 classified cases after
consolidation. The three removed tree-sitter placeholders contained only a
newline; their filenames did not represent actual namespace/attribute/port tests.

## Why some module matches remain

Matching ICL tokens do not always mean redundant test inputs:

- **Nine groups** exercise comments, BOMs, CRLF, spacing, blank lines, editor
  offsets or exact golden output. Removing those byte-distinct inputs would
  remove lexer/formatter coverage.
- **Sixteen groups** are helper definitions used in complete independently
  sourced examples: benchmark/Honza Instruments, Origen/IJTAG_extension, the two
  published chip variants, three warptap topologies, and the completed Tessent
  Chain examples. Keeping the helpers preserves those whole-design inputs.

Each group has a reviewed reason and exact path/module-index/file-hash set in
the policy. A new occurrence, changed occurrence or obsolete exception fails
the check; no directory or generic module name is allowlisted. Same-name modules
with different definitions remain distinct objects.

## How it is checked

```sh
make corpuscheck BISON=/usr/bin/bison
python3 tests/audit_icl_corpus.py --policy tests/cases/deduplication.json --report build/icl-duplicates.json
```

The checker uses the same parser/AST as the formatter and LSP, through the
test-only inventory executable. It batches byte-unique inputs, compares SHA-256
for file identity, and reuses the EDA importer's namespace-aware module identity.
It includes opaque Generic bodies exactly. Ordinary comments/layout are ignored
only for module matching, not for whole-file matching.

All 1162 rejected fixtures receive whole-file duplicate checks. Their partial
trees are not used for module surgery; doing so could remove the defect the
negative test is meant to exercise. This audit does not establish hardware or
semantic equivalence. It changes test organization, not runtime ICL behavior.
