# Bare scan sources and alias-hidden register taps implementation plan

> **For agentic workers:** Use superpowers:executing-plans inline under existing
> autonomous authorization. Preserve independent review and final verification.

**Goal:** Enforce bare ScanInSource eligibility and rightmost-register taps through aliases.

**Architecture:** Extend the shared terminal classifier with rightmost/intermediate
ScanRegister kinds, retaining both as ordinary data. A strict scan-input family
accepts constants, scan terminals and rightmost register bits. ScanOut Source
rejects intermediate register terminals through aliases, stopping at legitimate
mux/port terminals. Cache remains keyed by alias node and bit; no new graph walk.

**Tech Stack:** C++17 and existing native/Python tests, no new dependency.

**Spec:** [Approved generator requirements](../specs/2026-10-25-icl-pattern-generation-design.md),
illegal connectivity validation. IEEE1687-2014 §6.3.12(c–d), pp.69–71;
§6.4.6.2(b), p.83; §6.4.8(c,u), pp.114–115. Source text inspected at
build/combined-audit/ieee1687.txt lines4483–4584,5280–5286,7111–7205.
Permission6.4.8(u) explicitly permits omitted direct register index selection.

## Global constraints

Integrate Tasks60/61 as one final full-check snapshot after0f40c3e. Retain the
separately reviewed/verified60 checkpoint and each focused RED/GREEN record.
This shared-classifier increment requires one combined full dynamic/static and
affected leak-check gate. Direction, contextual mux constraints and broader
scheduler/event models remain separate mandatory work. Preserve parser/formatter
corpus illustrations; their acceptance does not establish elaboration validity.
Any affected behavioral fixture may express data-driven scan through a legal
ScanMux choice, retaining its original assertions rather than adding exemptions.

## Review focus

Rightmost is declaration order, not numeric minimum. Cover descending/ascending,
nested/inverted/sliced aliases, mixed rightmost/middle aliases, constants and
virtual-reset rejection. Both register classes must remain data-eligible across
existing families. Check restored internal ScanOut origins and atomic artifacts.

## Task61

Files: src/pattern/pattern_netlist.cc; existing tests/src/pattern_netlist_test.cc
and tests/src/test_icl_pattern_generation_main.py; connected behavioral fixtures
only if affected; guide/model/plan ledgers. Elaborate/SerializeIcl interfaces and
flat metadata stay unchanged. Reuse current per-bit cached classification and
existing4096-alias/65536-bit scale measurements; no new performance machinery.

- [x] Inspect exact grammar/permissions, current resolver/checks/callers and design review.
- [x] Qualify19 independent loading-only vendor models; match fetched input hashes.
- [x] Add native/CLI RED for non-scan data terminals and alias-hidden middle bits,
  requiring source-located errors and preserved pattern/reset/debug artifacts.
- [x] Add positive scan/constant/rightmost/implicit/flat/data-context coverage;
  implement classifications and checks with no generated-model exception.
- [x] Run focused native/main/flat/TAP/client and affected fixture checks;
  naming/style, measured scale and independent source review.
- [x] Combined60/61 full dynamic/static and affected sanitizer checks, final evidence,
  docs and verified commit.

## Vendor qualification

build/scan-terminal-qualified-vendor-probe/comparison.json records exact fetched
models/run scripts and SHA256 matches. Tessent2026.2 accepts0/1/X constants,
explicit rightmost and omitted register indices in both directions. It rejects
DataInPort/direct alias, DataRegister, LogicSignal, DataMux, TCK and middle
register sources. Direct/nested/sliced aliases do not hide invalid middle taps
from ScanOut validation. Source-specific ICL65 diagnostics support this scope.
Some positive models warn ICL68 for deliberately inaccessible registers. This is
loading/elaboration only: no analysis mode, PDL, ours output or functional comparison.
The copied clock helper's comments/source.pdl are unused; actual run.tcl and scope.json
define what executed. Remote directory: /tmp/icl-scan-terminal-qualified-20261114.

The earlier19-case batch is retained separately under
build/scan-terminal-vendor-probe: its SI/SO-only explicit interface failed ICL101
classification. Those setup failures establish no source-eligibility conclusion.

Focused RED: strict-scan-source-red-cli.exit1 accepts all44 invalid cases;
confirmed-red-native.exit1 accepts an ordinary-data alias. Initial native baseline
exit0 covered positives and an existing direct flat rightmost check, so it is not
a bug reproducer. GREEN main46 passes9.890s, native scheduler/flatten/netlist/state/
TAP and TAPcli9/client18 pass. Restored virtual reset negatives pass too. Naming,
formatting and independent source review pass; no fixture exemption was needed.
The final three files are backed up with SHA256 at /tmp/strict-scan-source-task61-files
and imported into root for combined60/61 full and leak checks. Root remains frozen
during those checks; further investigation uses the isolated checkout.

Full combined integration passes in build/scan-sources-final-full.log/.exit0,
1121.542s wall:41 dynamic/static suites,51 directional/seven legacy goldens,
2523 ICL334/119 rules, editor6 in198.149s; five optional ANTLR tests skipped.
Leak-enabled native scheduler/flatten/netlist/state/TAP and main46/clock41/client18/
reset12 checks pass in build/scan-sources-final-sanitizer-*. Final three source/test
hashes match the reviewed Task61 checkpoint. Naming/style and independent source
reviews pass. Strict coverage, formal vendor certification and platform refresh
remain open.

Final native0.80s wall,120336KiB RSS;4096 aliases0.0109507s and65536-bit/4096
consumers0.0230771s. No speedup claim or new performance mechanism.
