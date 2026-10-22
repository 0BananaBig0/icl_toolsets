# Real EDA ICL corpus

Collected read-only on 2026-10-22 from the user's three project trees under
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

There are 106 distinct whole-file byte sequences: 74 repeated file occurrences.
The 328 module occurrences reduce to **164 distinct definitions**: 119 stored in
`pass/tessent/eda/`, 45 in `pass/syn_slm/eda/`. Fifty qualified module names have
multiple differing definitions; all of those variants are retained. The file
suffix identifies the definition, so a same-name variant cannot overwrite it.

## What counts as a duplicate

The shared `ParsedDocument` frontend supplies module boundaries, namespace
context and lexer tokens. A definition's identity combines definition namespace,
effective initial `UseNameSpace`, ordered token spellings and exact source of
opaque Generic AccessLink statements. Ordinary layout and standalone/inter-token
comments do not distinguish definitions. Strings, literal spellings, expression
order and Generic payload whitespace remain significant. This is conservative
lexical deduplication, not constant folding or proof of semantic equivalence.
Comments embedded inside a single numeric token also remain significant.

Each retained fixture contains the original module bytes, outside-module comments
from its selected source, and explicit `NameSpace` / `UseNameSpace` directives.
The first path in lexical order supplies a duplicate's canonical copy; every
origin keeps its own project classification in `eda_sources.json`, even if a
shared definition's canonical copy comes from the other project.

The maintained manifest records each original path and whole-file SHA-256, its
ordered canonical-module IDs, canonical fixture hashes and source comments.
Comments from discarded duplicates are retained there, including notices.
Generic payloads and comments within numeric tokens remain in their canonical
definitions. Fixture import does not change the original authorship or terms.

This collection is a syntax/formatter corpus. Different design snapshots reuse
module names; loading every variant as a single design would be ambiguous.
Cross-file associations can be recovered from the source mappings. Prior
standard, constructed, upstream and real-tool regression fixtures were not
deduplicated: their repetition may exercise separate coverage requirements.

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
fixtures and provenance before promoting them from staging into `tests/cases`.
Archives, inventory reports, helper binaries and other generated build output
stay ignored. The two installed executables and their runtime behavior are
unchanged by this corpus addition.
