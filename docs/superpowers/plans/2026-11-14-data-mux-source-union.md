# DataMux impossible source-kind rejection implementation plan

> **For agentic workers:** Use superpowers:executing-plans inline under existing
> autonomous authorization, preserving independent review and final verification.

**Goal:** Reject DataMux input terminal kinds excluded by every applicable data rule.

**Architecture:** Add a DataMux-choice family to the existing cached classifier.
Its permitted superset is ordinary data/constants and the five conditional
Reset/CaptureEn/UpdateEn/TMS/TRST families. Keep SelectedBy ordinary data. No new
graph pass, metadata, grammar or dependency. This excludes impossible terminals;
it does not establish the five conditional matching-output predicates.

**Tech Stack:** C++17, existing native and Python tests.

**Spec:** [Approved generator requirements](../specs/2026-10-25-icl-pattern-generation-design.md),
illegal source validation. IEEE1687-2014 §6.3.12(d), pp.70–71 and §6.4.12,
pp.125–126, inspected at build/combined-audit/ieee1687.txt lines4543–4584,
7790–7848. Independent conceptual review approves this bounded union check.

## Global constraints

Work in the isolated branch on544c522. Preserve all previously verified checks
and checkpoints. Do not equate this permitted union with complete context legality.
Matching-output use, mux structure/forcing/operational sensitization and qualified
consumer restrictions remain mandatory Task64 work. Do not reject existing mixed
fanout or dangling mux outputs based on unqualified backward reachability.
New tests belong under root tests; extend existing scripts only.

## Review focus

All ordinary-data kinds and both register bit classes; five conditional families
including To* aliases; forbidden ShiftEn/clock/TCK/scan/address/IRSelect terminals;
mixed alias slices; virtual resets versus literal constants/unconnected holes;
source/flat replay and preserved output artifacts. SelectedBy retains its existing
ordinary-data rule. Reuse the existing alias/wide-input scale checks.

## Task63

Files: src/pattern/pattern_netlist.cc, tests/src/pattern_netlist_test.cc,
tests/src/test_icl_pattern_generation_main.py; guide/model/plan ledgers.
CheckSignalSources already owns callers and flat restoration ordering. Add one
family branch and one DataMux choice loop, retaining its existing memoized
O(nodes + checked bits + alias expansion) structure and budgets.

- [x] Inspect shared caller path, exact primary grammar and conceptual review.
- [x] Add CLI/native RED for unrelated terminal acceptance; preserve artifacts.
- [x] Add ordinary-data/constant and conditional-family positive source/flat cases,
  alias slices and virtual-reset negatives; implement the union check.
- [x] Focused netlist/flat/main, affected register/TAP/client suites, style/naming,
  measured scale and independent source review.
- [x] Full dynamic/static and affected sanitizers; record final evidence and commit.

## Required Task64 context work and evidence

The27 initial and24 corrected loading probes under build/data-mux-context-*-vendor-probe
distinguish ICL65 source/consumer errors, ICL115 capture forcing, ICL64 polarity
and ICL129 reset-state failures. Correct active-high local reset forcing to all
ones before using reset probes as positive loading evidence. Duplicate choice rows
are not the same as three distinct inputs. Exact fetched input hashes must match.

Current corrected results accept reset aliases/chains, five To* outputs, paired
polarity cases, TRST/ToTRST-to-reset and unused/partly unused mux outputs. They
reject DataOut-only or wrong-family consumers and data-port/group forwarding.
The vendor also rejects DataOut fanout beside a matching reset sink. The primary
generic DataMux data-signal list has no explicit universal exclusive-fanout rule;
preserve existing UpdateEn-to-DataRegister behavior pending its exact probe.
Document discrepancies and choose a standard-first rule from actual evidence.
Loading alone proves neither normal-operation sensitization nor reset-event behavior.

Connected fixture files also include tests/src/pattern/test_system_clocks.py and
tests/src/pattern_scheduler_test.cc. Two CLI clock-source negatives now fail at
the new source-kind boundary; explicit lowered native cases preserve both prior
clock-fanout guards, including an unselected clock branch. No product exception.

CLI RED accepts all10 unrelated terminals. Native RED has one genuine unchecked
clock-choice case and an unsupported virtual-choice mutation. The first GREEN
retains that test-only failure: flat metadata carries node.inputs virtual refs,
not choice refs. Corrected virtual-reset tests exercise supported SelectedBy
metadata and keep the selector data-only; no serializer extension is introduced.

Confirmed main48 passes10.057s; netlist/flat/state/TAP/scheduler, TAPcli9/client18
and corrected clock41 pass. Naming/style and independent source/fixture review
are clean. Native netlist0.76s/120656KiB RSS;4096 clock aliases0.0101215s and
65536-bit/4096 consumers0.0182843s. Five files are hash-backed up under
/tmp/data-mux-source-union-task63-files and imported for final integration.

Larger input:335 ICL bytes,65536-bit data/alias/mux,10 choices (655360 source refs).
Three interleaved source+flat runs against544c522 preserve identical output hashes.
Baseline source0.0595–0.1210s; changed0.1369–0.1667s. Changed flat0.1191–0.1489s.
Peak changed RSS67344KiB; exact commands/environment/hashes are recorded in
build/data-mux-source-union-scale/comparison.json. This measures the validation
overhead, not a speedup. No new graph pass or performance machinery.

Further required resource work: ConnectMux currently expands per-choice source
vectors and select-code masks without an aggregate mux-bit budget; the node-bit
graph bound alone does not bound these arrays or selector dependency work. A
compact65536-bit/4096-choice model can request268M source references. Do not run
that unconstrained. Qualify aggregate allocation/work limits or compact storage
as Task65, preserving supported scale and an explicit upgrade trigger.

First full integration exit2 (553.343s) exposes an additional invalid gated-TCK
fixture in test_multiple_tap_scheduling.py. Its DataMux is now ClockMux with an
explicit lowered ToTCK Source bridge; all1/0/X/inverted assertions and source/flat/
generated replay remain. Focused22 passes24.363s and fixture review is clean.
Six-file checkpoint includes this correction. Root receives only that sixth file;
the first five hash-match the already passing selected sanitizer snapshot.
Selected native6/main48/clock41/client18/reset12 sanitizer checks passed; the
multi-TAP CLI fixture is optimized qualification, not an instrumented scale claim.
A fresh complete confirmed full regression is required; preserve first exit2.

Confirmed full integration passes in build/data-mux-source-union-confirmed-full.log/
.exit0,1061.125s wall:41 dynamic/static suites,51 directional/seven legacy goldens,
2523 ICL334/119 rules, editor6 in187.584s; five optional ANTLR tests skipped.
Selected leak-enabled native scheduler/flatten/netlist/state/TAP/address and
main48/clock41/client18/reset12 pass in build/data-mux-source-union-final-sanitizer-*.
The five selected source/test inputs hash-match that snapshot; the sixth corrected
multi-TAP CLI fixture passes optimized22, including source/flat/generated replay.
No instrumented multi-TAP scale claim is made. Initial full exit2 and test-only
metadata assumptions remain recorded. Naming/style and independent product/fixture
reviews pass. Strict coverage, formal vendor certification and platform refresh
remain open.
