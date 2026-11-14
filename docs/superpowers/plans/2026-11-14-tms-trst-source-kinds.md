# TMS/TRST source eligibility and unambiguous vector inference

Execute inline under existing autonomous authorization. Integrate Tasks58 and59
as one reviewed control-source increment after Task57's verified commit. Retain
Task58's separately verified checkpoint. One final combined full dynamic/static
and leak-check snapshot must pass before integration is complete.

## Final design

IEEE1687-2014 §6.4.7(n–o), §6.4.6.16 and §6.4.6.22 define child TMS/TRST input
bindings and corresponding To* Sources. Accept matching-family ports, ordinary
data/constants and aliases. TRST additionally accepts inferred kAsyncGlobalReset;
reject kGlobalReset there and both virtual resets for TMS. TMS has no virtual
fallback. Preserve Task57's separately qualified TRST-to-reset acceptance.

Omitted connections require one eligible physical source **bit**, not merely one
declaration. Indexed width-one sources such as source[7] are unique; wider input
candidates are ambiguous. Repeat that unique bit, or an existing virtual reset
function, across destination bits with polarity preserved. Explicit vector
references retain their existing declaration-order resolution. Register-reset
associations stay scalar and now reject an ambiguous vector candidate rather
than selecting its leftmost bit.

Preflight all Source-missing Reset/TRST/TMS output and non-root input widths before
allocation, with a4M inferred-connection-bit budget. Exclude root inputs and
explicit connections, matching the actual inference path. Keep the existing
node/width/graph limits. Larger inferred models require a compact representation
and renewed qualification, not an unexplained limit increase.

Connected files: pattern_netlist.cc and existing native netlist/main CLI tests;
guide/model ledger. No new runner, dependency, grammar, metadata format or graph
traversal for source classification. A linear declaration preflight bounds the new
vector allocation. Contextual DataMux forcing, normal-operation selection,
direction, polarity/phase qualification and other model gaps remain required.

## Acceptance and evidence

- [x] Review exact grammars, canonical kinds and existing inference.
- [x] RED unrelated-family source acceptance; add atomic-output negatives.
- [x] RED valid scalar-to-vector output fails flat reload on Task57's binary
  (tms-trst-source-qualified-vector-red.exit1).
- [x] Implement family checks, scalar/virtual broadcasts, physical ambiguity checks,
  explicit-vector preservation and allocation preflight.
- [x] Native own/data/constant/partial/alias cases; inferred TMS and async TRST with
  polarity; wrong virtual sources; scalar[index7] broadcasts; explicit ascending/
  descending vectors; ambiguous groups/register resets; canonical source/flat replay.
- [x] Compact65×65536 over-budget case rejected before inferred buffers:1757 ICL
  bytes,0.000261687s. Its prior accepted model is retained as budget RED exit1.
- [x]65536-bit scalar-broadcast model (116 ICL bytes), source/flat0.298485s;
  native suite0.90s wall,122936KiB maximum RSS. Measured input, no speedup claim.
- [x] Main43 passes9.127s; naming and corrected independent source/resource review
  pass. Native state/TAP callers pass; full scheduler caller also passes (control-source-focused-scheduler.exit0).
- [x] Combined full dynamic/static, relevant leak checks, guide/model final evidence
  and verified commit. Preserve all prior failures and the separate58 checkpoint.

## Qualification correction

The first trial incorrectly inferred ordinal correspondence from a single vector
declaration. The explicit-reference rule does not establish omitted connections.
Tessent2026.2 rejects every affected TMS/TRST child/output bit with ICL123/124 when
the input vector contains multiple candidate ports. Indexed-port discussion in
§6.3.10.4 and §6.4.5 supports counting bits. Independent review retracted the
original recommendation; the trial was never integrated into root. Exact code is
preserved under /tmp/tms-trst-task59-ordinal-trial and failed vendor evidence under
build/inferred-control-vector-vendor-probe/rejected-remote.

A separate scalar-source probe is accepted for grouped TMS/TRST destinations.
Submitted hashes match fetched files. Its source-only clock-wait export matches
our output, but our output was not executed by the vendor in that run. This is
loading/analysis compatibility, not functional control-value equivalence or a
formal certificate. Evidence and comparison.json are under
build/inferred-control-vector-vendor-probe/scalar-source.

Initial native trial also exposed a test-only scalar Alias/two-bit source mismatch;
its explicit range was corrected. A suspected bare-port serializer issue was
checked against the frozen baseline and all three ports replay correctly, so no
serializer product change was retained. The vector bug that remains fixed is the
one-bit inferred Source array on a grouped destination with a unique scalar source.

Full combined integration passes in build/control-source-final-full.log/.exit0,
1094.986s wall:41 dynamic/static suites,51 directional/seven legacy goldens,
2523 ICL334/119 rules, editor6 in185.044s; five optional ANTLR tests skipped.
Leak-enabled native scheduler/flatten/netlist and main43/clock41/client18/reset12
checks pass in build/control-source-final-sanitizer-*. Final source/test hashes
match the reviewed Task59 checkpoint. Naming/style and independent source/resource
reviews pass. Strict coverage, formal vendor certification and platform refresh
remain open.
