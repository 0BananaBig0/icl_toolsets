# Real EDA ICL corpus

Initially collected read-only on 2026-10-22 from the user's three project trees under
`/home/banana/remote_eda/lianghuaxiao/`, mapped to
`lianghuaxiao@192.168.6.20:/data/workspaces/lianghuaxiao/`:

| Source tree | ICL paths | Classification |
|---|---:|---|
| `test/tessent_ijtag` | 25 | 24 Tessent; 1 under `test_syn_slm_icl` is Synopsys SLM |
| `tessent_2026.2_usage_examples` | 46 | Tessent |
| `syn_slm` | 109 | Synopsys SLM project, including its Tessent analysis output |

The 180 paths include one symlinked `build/chip_top.icl`; its contents duplicate
another collected file. Following links reported broken simulator `.so` links
and two Verdi directory cycles, but no unreadable ICL. All 180 inputs parse.
Source trees were not modified.

The initial snapshot had 106 distinct whole-file byte sequences: 74 repeated file occurrences.
The 328 module occurrences reduce to **164 distinct definitions**. The initial
import stored 119 Tessent and 45 Synopsys SLM module fixtures. The subsequent
[whole-corpus audit](CORPUS_DEDUPLICATION.md) consolidated 47 definitions already
present in four original baselines. That audit retained 78 standalone Tessent and 39
standalone Synopsys SLM fixtures, plus references to those baseline modules.
All 164 definitions and all 180 source mappings remain represented.
Fifty qualified module names have
multiple differing definitions; all of those variants are retained. The file
suffix identifies the definition, so a same-name variant cannot overwrite it.

## Tessent refresh — 2026-10-26

The following projects were collected with read-only remote `find -L` and a
NUL-delimited tar stream, following source links without modifying their targets:

| Project under `tessent_2026.2_usage_examples/` | ICL paths | PDL paths |
|---|---:|---:|
| `tessent_example_flat_flow_v2026.2` | 21 | 57 |
| `tessent_example_hierarchical_flow_with_ssn_v2026.2` | 50 | 271 |
| `tessent_example_hybrid_tk_lbist_flow_v2026.2` | 21 | 45 |
| Total | 92 | 373 |

All 92 ICL source files parse. Their 463 module occurrences contain 202 distinct
definitions: 78 reuse existing fixtures and **124 new definitions** have been
added. The complete maintained EDA manifest now represents **230 source paths,
31 historical source revisions and 288 distinct modules**. It references 202
standalone Tessent fixtures, 39 standalone Synopsys SLM fixtures, and the 47
definitions shared with original multi-module baselines. Different definitions
with the same name remain separate.

Each source path has one current record. When its bytes changed, its earlier
hash, module order and comment references are retained in `previous_versions`.
The current record's `snapshot` identifies this refresh. A module's `source`
may refer to either the current or a historical revision of that path; the
module ID determines which definition it represents. Existing oracle source
lookups use the current revision. No previous fixture was overwritten or
discarded. The PDL import is described in [pdl_eda/README.md](pdl_eda/README.md).

The global ICL audit still has zero duplicate-file groups and the same 25
previously reviewed repeated-module groups; no duplicate-policy exception was
added for this import. Every new module is registered in `conformance.json`
and enters CLI preservation/idempotence, LSP diagnostics and editor corpus tests.

## What counts as a duplicate

The shared `ParsedDocument` frontend supplies module boundaries, namespace
context and lexer tokens. A definition's identity combines definition namespace,
effective initial `UseNameSpace`, ordered token spellings and exact source of
opaque Generic AccessLink statements. Ordinary layout and standalone/inter-token
comments do not distinguish definitions. Strings, literal spellings, expression
order and Generic payload whitespace remain significant. This is conservative
lexical deduplication, not constant folding or proof of semantic equivalence.
Comments embedded inside a single numeric token also remain significant.

Each standalone fixture contains the original module bytes, outside-module comments
from its selected source, and explicit `NameSpace` / `UseNameSpace` directives.
The first path in lexical order supplies a duplicate's canonical copy; every
origin keeps its own project classification in `eda_sources.json`, even if a
shared definition's canonical copy comes from the other project. After global
deduplication, `path` may instead identify an original multi-module baseline;
`id` selects exactly one module within it. `sha256` hashes that canonical file.

The maintained manifest records each original path and whole-file SHA-256, its
ordered canonical-module IDs, canonical fixture hashes and source comments.
Comments from discarded duplicates are retained there, including notices.
Generic payloads and comments within numeric tokens remain in their canonical
definitions. Fixture import does not change the original authorship or terms.

This collection is a syntax/formatter corpus. Different design snapshots reuse
module names; loading every variant as a single design would be ambiguous.
Cross-file associations can be recovered from the source mappings. The global
audit covers earlier fixtures too; necessary repetitions are individually
documented and pinned in `deduplication.json`.

## Reproduce and test

Stage local source copies with the three relative tree names shown above, then:

```sh
make corpuscheck BISON=/usr/bin/bison
python3 tests/import_eda_corpus.py --input build/eda-import/raw/data/workspaces/lianghuaxiao --output build/eda-import/new-corpus
python3 tests/src/test_icl_conformance.py --binary build/icl-format --block eda-corpus
make check-dynamic BISON=/usr/bin/bison
```

The importer refuses existing output directories and rejects invalid inputs
before writing fixtures. It parses each byte-unique file once, using the shared
frontend; it does not parse ICL with Python regexes. Review updated source
fixtures and provenance before promoting them from staging into `tests/cases`;
resolve existing canonical definitions using the global duplicate audit.
Archives, inventory reports, helper binaries and other generated build output
stay ignored. Imported vendor sources are explicitly maintained regression
inputs, not generated build artifacts. This ICL refresh changes no ICL runtime
behavior; the accompanying PDL compatibility fix is documented separately.
