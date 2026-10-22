# Literal parameter-width validation — 2026-10-22

Baseline:97bb610. The shared parser now diagnoses significant-bit truncation
when a sized literal's width resolves through local numeric literal defaults
or scalar parameter aliases. Forward references work in declaration order or
reverse order. Zero widths are rejected too. Both icl-format and icl-lsp use the
same AST metadata and driver check; formatting preserves the original spelling.

For example, the following previously passed and now reports the overflowing
literal on its own line:

```icl
Module Example {
  Parameter VALUE = $WIDTH'H1_FF;
  Parameter WIDTH = $BITS;
  LocalParameter BITS = 8;
}
```

Changing BITS to9 clears the diagnostic. IEEE1687-2014 §6.3.8(j,o), printed63–64
(PDF77–78), explicitly permits parameter sizes and requires an error if a set
bit would be truncated. Its SIZE=2 / `$SIZE'h3` and `$SIZE'hF` examples are now
complete fixtures in `tests/cases/standard/lexical/parameter_size_*.icl`.

## Implementation and bounds

`ParsedValue` retains an optional literal magnitude bounded by `size_t`.
Based numbers and decimal integers share the checked conversion helper. Unknown
digits and arithmetic overflow leave the magnitude unresolved; values never wrap.
The existing Kahn dependency traversal propagates scalar aliases in O(V+E).
Parameter-width literals are retained until the module closes, then checked by
the existing arbitrary-width digit validator. Per-module pending checks are
cleared before the next module. Attribute names never enter the parameter map.

This is conservative default-value validation, not instance elaboration. An
Instance override expression refers to the enclosing module's scope; its
left-hand parameter name does not shadow that scope. Overridden child-module
instantiations are not evaluated. Arithmetic expressions, inversion,
concatenations, unknown values, values beyond `size_t` and unresolved external
references stay deferred. In particular, the formatting grammar does not define
an arithmetic evaluator; evaluating its reductions would risk wrong precedence.
Enum width equality and consuming-object widths still need fuller elaboration.

The change adds no token definition, grammar production, alternative AST or
runtime dependency. It fixes the recorded literal/alias failure without claiming
that every parameter-dependent semantic error is now diagnosed.

## Tests and evidence

Added40 handwritten ICL files and40 manifest classifications, including the
previously recorded diagnostic fixture promoted to an ordinary rejection.
The parameter block has90 classified checks; total conformance checks are2,055.
The seven live diagnostic tests now pass without expected failures and include
versioned correction of BITS from8 to9, precise line/severity and clearing.

The constructed cases under `tests/cases/ours/parameter_widths/` cover forward
aliases, all bases, underscores/comments, zero/partial-nibble boundaries, nested
uses, scopes, keywords and bounded-conversion overflow. Deferred-value cases
protect against accidentally using a component's value for the whole expression.
Independent expected values include concat(4'h4,4'h4)=68 and ~8'd8=247.

The focused suite reproduced15 accepted-invalid cases and the original missing
live diagnostic before implementation. Existing valid files continue through
content-preservation and idempotence checks. The first full dynamic/style run passed, then the independent review's eleven
boundary probes were persisted as fixtures. Ten are classified; the arithmetic
undercheck is explicitly documented in `alias_arithmetic_unchecked.icl` and is
not treated as a normative positive. Independent review found no critical or
important defect and independently ran the focused checks and boundary probes.
Generated fixtures/reports remain ignored.

## Performance

Measured the baseline installed binary (97bb610) against the new release build,
using `tests/src/benchmark_icl.py` with three repeats, then seven repeats to
reduce timing noise. Host: Linux6.18.33.2 WSL2 x86-64, glibc2.43, GCC16.2.1.
The table uses the seven-run median wall time and maximum measured RSS.

| Input | Bytes | Baseline / new seconds | Baseline / new RSS KiB |
|---|---:|---:|---:|
| Merged Tessent | 141,671 | 0.0093 / 0.0097 | 7,640 / 7,696 |
| Merged Tessent x16 | 2,266,736 | 0.1189 / 0.1311 | 54,540 / 55,300 |
| 1,000 forward aliases | 23,851 | 0.0050 / 0.0052 | 6,108 / 6,148 |
| 4,000 forward aliases | 101,851 | 0.0136 / 0.0139 | 11,124 / 11,260 |
| 16,000 forward aliases | 425,852 | 0.0489 / 0.0497 | 29,460 / 30,004 |

The extra metadata/checking has a measurable cost (~12ms on the2.27MB merged
input); there is no speedup claim. Forward chains remain iterative, with roughly
linear observed scaling. These measurements cover up to16,000 aliases and the
2.27MB fixture; larger workloads need remeasurement before choosing additional
caching or evaluation machinery. Reproduce before replacing the installed
baseline, or retain another build of97bb610:

```sh
python3 tests/src/benchmark_icl.py --binary build/icl-format \
  --baseline /path/to/97bb610/icl-format --repeats 7 \
  --output build/parameter-width-performance-repeat.json
```

## Final verification

`make check-dynamic BISON=/usr/bin/bison -j4` passes with2,055 conformance
checks,29,868 metamorphic variants across45 blocks,5,568 keyword/context cases,
42 semantic LSP tests and seven diagnostic
tests without expected failures. Google format/naming checks pass. Trace coverage
reaches334/334 Bison productions and119/119 Flex rules across2,324 files.
The refreshed upstream comparison has1,889 agreements,270 local-only and165
reference-only acceptances; all reference-only rejections have maintained
adjudications and none is a classified positive.

Raw handwritten coverage is2,711/2,759 lines(98.3%) and3,913/6,570 branches(59.6%).
Parser actions reach563/563 lines; scanner source reaches211/213 lines, with the
previous Generic-entry mapping misses unchanged. `make coverage` still exits
nonzero against the unchanged100%/90% thresholds. No generated code or compiler
mapping was removed from the existing source-only denominator to improve scores.

Independent review found no critical/important issue; its11 additional probes
were retained and rerun through the final corpus. Arithmetic-derived widths and
full instance elaboration remain explicit limitations, as described above.

Installed with `make install-dynamic prefix=/home/banana/.local
BISON=/usr/bin/bison`. Installed executables byte-match the tested build and pass
2,055 conformance inputs,42 semantic LSP tests and all seven diagnostics tests.
The isolated real Vim/coc integration test passes diagnostics, formatting,
definition, hover, symbols, completion and invalid-input preservation while
explicitly launching `/home/banana/.local/bin/icl-lsp`. No user editor settings
were changed; existing coc configurations remain applicable.

