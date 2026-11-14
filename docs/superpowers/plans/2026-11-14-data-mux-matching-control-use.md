# Conditional DataMux matching-use implementation plan

> **For agentic workers:** Use superpowers:executing-plans inline under existing
> autonomous authorization. Preserve independent review and final verification.

**Goal:** Reject observed conditional DataMux control inputs without matching control use.

**Architecture:** After source-kind validation, build per-bit use masks only when
special DataMux inputs exist. Typed control Sources seed matching families; reset
associations seed Reset/TRST. Other consumers mark observation without granting
a family. Propagate masks backward through aliases and DataMux data choices only.
Observed output bits require each conditional input's matching family; unused
bits remain accepted. Retain standard-first mixed fanout when matching use exists.

**Tech Stack:** C++17, current native/Python tests, no new dependency or metadata.

**Spec:** [Approved generator requirements](../specs/2026-10-25-icl-pattern-generation-design.md),
illegal connectivity validation; IEEE1687-2014 §6.3.12(d), p.71, inspected at
build/combined-audit/ieee1687.txt lines4559–4584. Task63 supplies the source-kind
union; this increment supplies matching use, not structural/operational signoff.

## Global constraints

Keep root63 frozen during confirmed full regression; work in the existing isolated
checkout and retain its six-file hash checkpoint. Preserve unused/partly unused
muxes and alias/mux-chain dangling outputs, qualified by corrected vendor probes.
Do not propagate matching families through selectors, Enable, ordinary ports,
logic, groups or storage. ScanMux/ClockMux choices are observed data uses.
ReadReference supports lowered reset associations to muxes/aliases, so seed those
semantically as Reset|TRST. Reset Source contexts admit TRST per the independently
qualified direct/ToTRST compatibility cases. Inversion does not change family;
polarity/forcing checks remain separate requirements.

## Scope and interpretation

Qualified27+24 loading probes and the two exact update/data-register fixtures have
matched fetched hashes. Tessent2026.2 accepts reset aliases/chains, five To* outputs,
TRST-to-reset, unused scalar and partly unused vector muxes. It rejects data-only,
wrong-family and ordinary-data-port/group forwarding. It additionally rejects
mixed data fanout even alongside a matching control output. The primary generic
dataMux data-signal list has no explicit universal exclusive-fanout rule; retain
the existing UE-to-ToUpdate plus DataRegister WriteEn behavior as a documented
standard-first interpretation and vendor discrepancy. No functional equivalence
or normal-operation/reset-event proof follows from loading.

Port-specific two-input/force-value/polarity/normal-operation sensitization and
global-reset selection rules remain mandatory follow-up work. Do not claim that
passing matching-use validation establishes complete connection legality.

## Review focus

Per-bit mixed outputs versus node-wide flags; unused bits versus observed data-only
bits; nested/inverted/sliced aliases and mux chains; reset-reference-only uses;
ScanMux/ClockMux choice uses; selectors/Enable/logic/group boundaries; original/
flat replay and atomic output preservation. Preserve existing mixed fanout tests.

## Task64

Files: src/pattern/pattern_netlist.cc, tests/src/pattern_netlist_test.cc,
tests/src/test_icl_pattern_generation_main.py; current guide/model/plan ledgers.
Private ControlContexts/FindMuxControlUses helpers hold offset-indexed byte masks.
Keep the public Elaborate/SerializeIcl interfaces and metadata unchanged.

Performance: reuse the existing16M graph-bit preflight. Dense masks cost at most
16MiB; offset storage scales with node count. Reserve a uint32 worklist for alias/
DataMux bits, at most64MiB under that bound; a queued flag deduplicates pending
entries and each use bit is added once. Processing is bounded by the small family
count and graph edges, with logarithmic owner lookup. Ordinary muxes incur no
context allocation or extra graph pass. A sparse map costs multiple words per
tracked bit and adds logarithmic updates; repeated per-use scans risk quadratic
work. Choose dense masks for the measured wide/deep cases, without new caching.
Aggregate mux expansion/dependency work still needs the separately recorded Task65.

- [x] Inspect primary clauses,27+24+2 probes, current collectors and independent design review.
- [x] Add CLI/native RED for data-only/wrong-family and per-bit observed misuse;
  preserve pattern/reset/debug artifacts.
- [x] Add positive matching/alias/chain/unused/mixed-fanout/TRST-reset/reset-ref
  replay and negative selector/Enable/other-mux-choice boundaries.
- [x] Implement lazy masks with deduplicated worklist after union validation.
- [x] Focused netlist/flat/main/state/TAP/scheduler/client/clock checks; naming/style,
  independent source review and reproducible ordinary/wide/deep baseline comparison.
- [x] Integrate after63 verified commit; full dynamic/static and affected sanitizers,
  measured evidence, documentation and commit.

Focused RED: data-mux-control-use-red-cli.exit1 accepts all17 invalid observed
uses; native RED accepts the data-only reset mux. GREEN main49 passes11.129s;
native netlist/flat/state/TAP/scheduler and TAPcli9/client18/clock41 pass. Naming
initially required braces around a wrapped loop; corrected naming/format pass.
Independent source review confirms collection, reset references, mask monotonicity
and queue/storage bounds. Three final files are SHA-backed up under
/tmp/data-mux-control-use-task64-files; source hash matches measured benchmark.

Three interleaved source/flat runs per variant against4327e27 have identical output
hashes. Exact models, commands, compiler/platform and RSS are in
build/data-mux-control-use-scale/comparison.json. Ordinary655360-reference case
changed0.1133–0.1609s versus0.1087–0.1631s;65536-bit control case changed
0.1174–0.1404s versus0.1113–0.1223s. Larger262144-bit control (290 ICL bytes)
changed0.4373–0.4849s versus0.4029–0.4434s, peak126616KiB RSS.4096 aliases
(83940 ICL bytes) changed0.0645–0.1259s versus0.0689–0.1230s. These measure
overhead and scale, not speedups or worst-case optimality.

Root integration follows63's verified commit4327e27. Matching use alone leaves
port-specific structural/forcing/polarity, operational sensitization and remaining
resource/runtime/qualification requirements open.

Full integration passes in build/data-mux-control-use-final-full.log/.exit0,
1228.810s wall: dynamic/static suites,51 directional/seven legacy goldens,
2523 ICL334/119 rules and editor6 in281.764s. Five optional ANTLR tests remain
skipped. Leak-enabled native scheduler/flatten/netlist/state/TAP/address and
main49/clock41/client18/reset12 pass in build/data-mux-control-use-final-sanitizer-*.
All three source/test SHA-256 values match the independently reviewed checkpoint
and measured scale source. Naming/style and independent review pass. Strict
coverage, full vendor certification and refreshed platform checks remain open.
