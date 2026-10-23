# Independent PDL syntax validation

This review checks source parsing and static syntax diagnostics only. Both
`pdl-format` and `pdl-lsp` retain the same `pdl::Document` AST, parser, analysis
and formatting implementation. ICL retains its separate parser and tokens.
No pattern generation or retargeting is part of this work.

## Methods

The new runner is [test_pdl_syntax_properties.py](../tests/src/test_pdl_syntax_properties.py).
Its expectations do not import production command tables or validators.

| Method | Scope and independent evidence |
|---|---|
| IEEE production instances | All 24 PDL commands, valid/invalid numeric and object spellings, command-specific negative forms; expectations authored from clauses 7–8. iNote intentionally has no parameter-rejection case because its normative rule prohibits such errors. |
| Metamorphic layout tests | LF, CRLF, tabs and backslash-newline variants must retain the seed's acceptance result. |
| Recursive construction | Valid and invalid iWrite inside nested known script bodies at depths 1, 2, 8 and 32. Existing functional cases also check the 128-level limit. |
| Differential Tcl oracle | Tcl itself accepts/rejects 110 bounded constructed examples of builtin control structures, lists, expressions and lexical boundaries. |
| Tool consistency | Every case exercises formatter exit status, refusal to output invalid source, real stdio LSP diagnostics and CLI/LSP formatting parity. Accepted formatting must be idempotent. |
| Independent review | A separate reviewer examined static false positives, unchecked literal structure, fresh-process initialization and oracle independence. All 43 probe inputs, including passing controls, are retained. |

The final property suite has **438 cases**: 320 IEEE/layout instances, 110 Tcl
examples and eight nested scripts. Maintained seeds are classified under
`tests/cases/standard/pdl/` and `tests/cases/ours/pdl/`. Generated concrete inputs
are reproducible under ignored `tests/generated/pdl_syntax/`; JSON reports go
under ignored `build/`. Generated files are not added to Git.

The oracle creates a fresh safe Tcl interpreter for each maintained example,
with a 10,000-command limit and a 10-second batch subprocess timeout. Each tool
process has a five-second timeout. Oracle failures or budget exhaustion fail the
runner instead of becoming acceptance results. No vendor PDL is evaluated.
A Tcl execution error is an acceptance oracle only for these carefully chosen
examples: undefined variables/commands and hardware errors generally say nothing
about syntax. For example, `try {} finally -` is syntactically accepted but fails
at runtime; it is retained separately in `review_probes.json` with static
acceptance expected. PDL vector spellings are tested against IEEE seeds, not
against ordinary Tcl command-substitution behavior.

The [prior block review](pdl-block-review.md) documents the Honza255 comparison.
Its separate PDL grammar has all 24 command names but unresolved productions and
no runnable PDL fixture suite at the reviewed revision; it is a checklist, not
an independent acceptance oracle.

## Failures found and corrected

The initial 280-case run exposed **31 false accepts**, before corrections.
The shared parser now checks builtin argument counts and positional bodies for
if/elseif/else, for/while, foreach/lmap, catch, proc, set and try. Switch validates
literal options, list/direct pattern-body pairs and terminal fallthrough.
Tcl's unambiguous option abbreviations remain accepted. Lexical errors retain
priority over argument-structure errors; existing exact-range block tests caught
and prevented a diagnostic-precedence regression during development.

The independent review found additional errors, all retained before fixes:

- Fresh-process numeric try codes could crash because integer conversion used
  Tcl before its interpreter/thread initialization. Conversion now uses the
  same non-evaluating parser interpreter as expression parsing.
- Valid abbreviated try handlers and multiword expressions were rejected.
  Handler prefixes follow Tcl; literal expression words are joined before
  validation, while dynamic construction defers dependent checks.
- Invalid try variable/prefix lists and trailing handler fallthrough, malformed
  foreach/lmap value lists, conflicting/repeated switch modes and match/index
  variables without regexp mode were accepted. Literal structure is now checked.
- Missing expr operands and malformed quoted expressions escaped checking.

Dynamic options, bodies, lists and argument expansions retain conservative
handling. Opaque data is not parsed as a script. Both tools receive the fixes
through their shared frontend; no second grammar, evaluator or new runtime
dependency was introduced. Fix-only independent review found no further blockers.

## Reproduce

```sh
make pdl-syntaxcheck BISON=/usr/bin/bison
make pdlcheck BISON=/usr/bin/bison
make check-dynamic formatcheck namingcheck BISON=/usr/bin/bison -j4
make coverage BISON=/usr/bin/bison -j4
make pdl-static BISON=/usr/bin/bison -j4
python3 tests/src/test_pdl_syntax_properties.py \
  --formatter build/pdl-format-static --server build/pdl-lsp-static
```

`pdlcheck` includes the property suite once; coverage runs it against instrumented
binaries with a separate report path. Dynamic and static builds share ordinary
object files and should not be built concurrently.

## Verification on 2026-10-23

Environment: x86-64 WSL2 Linux, GCC 16, C++17, Tcl 9.0.2.

- PDL dynamic, static, installed dynamic and installed static suites each pass:
  34 functional methods, 192 block cases, 79 vendor streams (73 positive and six
  expected negative), and 438 syntax properties.
- Google formatting and naming/style checks pass.
- `make check-dynamic BISON=/usr/bin/bison -j4` passes. Five optional external
  ANTLR reference tests skip because `ICL_REF_DIR` is unset; the separate ICL
  reachability check covers 334/334 grammar productions and 119/119 scanner rules.
- Coverage regression tests pass, but the unchanged strict **100% line / 90%
  branch gate fails**: 3,913/4,002 authored lines (97.8%) and 6,621/10,831 branches
  (61.1%). `pdl_document.cc`: 1,128/1,166 lines (96.7%) and 2,536/3,925 branches
  (64.6%). Generated Bison/Flex internals and third-party Tcl are excluded.
- Installed dynamic tools are under `/home/banana/.local/bin`; static tools are
  under `build/install-static/bin`. All eight installed hashes match their build
  outputs; all help/version commands terminate successfully. All four static
  ELF files lack INTERP/NEEDED entries. Static portability still depends on the
  target environment as described in [PDL tools](pdl-tools.md).

With regression load finished, three native formatter runs on the existing
868,139-byte `wrapped_pmm_ip_characterization_debug_869fdacdb5f1cd49.pdl` fixture
take 0.02310/0.02139/0.02121 seconds (median 0.02139 s), with maximum child RSS
18,596 KiB. Output was sent to `/dev/null`; timing used `time.monotonic` and
`resource.getrusage(RUSAGE_CHILDREN)`. The preceding review's median was
0.02246 s; these short process timings establish no measured speedup. This is
a CLI measurement, not an LSP workspace-navigation benchmark.

Installation commands:

```sh
make install-dynamic prefix=/home/banana/.local BISON=/usr/bin/bison
make install-static \
  prefix=/home/banana/cpp_workspace/icl-format/build/install-static \
  MANIFEST=build/install-static-manifest.txt BISON=/usr/bin/bison
```

## What these results establish

The tests provide independent evidence for the listed static forms and found
real bugs missed by keyword/block inventories. They do **not** prove complete
PDL syntax coverage. Known limits relevant to syntax remain:

- Runtime-created command names, aliases, expanded arguments and computed script
  bodies cannot always be checked without evaluation. Known Tcl builtin names
  are interpreted using their ordinary meanings, not runtime redefinitions.
- Escaped switch-list elements needing decoded-to-source mapping remain opaque.
  Concatenated literal expr text receives syntax checking, but nested expression
  substitutions do not yet receive the same source-mapped traversal as a single
  braced expression. Other script construction such as multiword namespace eval
  is not reconstructed.
- Only the first diagnostic is reported. Nesting above 128 levels and NUL input
  are intentionally rejected.
- Missing/abbreviated level declarations and recorded vendor extensions remain
  accepted compatibility forms, not proof of strict IEEE-only acceptance.
- Tcl 8.6 and a live coc.nvim PDL session were not rerun in this phase. The LSP
  tests exercise the actual stdio service. Strict authored coverage remains unmet.

A broader grammar-generated corpus, new minimized failures and tests on another
supported Tcl version can further raise confidence. No finite passing corpus is
presented as a completeness proof.
