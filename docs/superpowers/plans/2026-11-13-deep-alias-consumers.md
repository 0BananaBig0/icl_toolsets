# Deep alias consumer validation

> Use superpowers:executing-plans for authorized inline implementation.

**Goal:** Enforce IEEE1687-2014 §6.5.2(g): an alias whose definition contains a
reference through two or more instance levels remains a PDL target, but cannot
be referenced by another ICL construct, including another Alias definition.

**Architecture:** After Connect and before RestoreFlatOrigins, classify aliases
from their resolved direct Source references and original scope relationships.
Reject consumers of those alias names in the already-resolved connections,
including any slice of a mixed local/deep alias. No serialized metadata or
public API change. A wrapper definition is itself an invalid consumer, so this
narrow rule needs no transitive graph propagation or additional cycle algorithm.

**Tech Stack:** C++17, existing native and Python unittest checks.

**Spec:** [Approved generator requirements](../specs/2026-10-25-icl-pattern-generation-design.md),
remaining alias-consumer validation. Autonomous inline execution is authorized.

## Rule interpretation and evidence

The specific normative §6.5.2(g) restriction takes precedence here over the
broader prose in §6.3.10.5 allowing other alias definitions. The wording is not
unambiguous across these clauses; record that conflict instead of implying it
does not exist. §6.5.2(f)'s broader alias allowance remains outside this narrow
two-level case. Preserve local alias chains and immediate-child cases; this
increment does not close all immediate-child or signal-kind requirements.

Independent review supports this reading after the Tessent probes. Corrected
Tessent2026.2 probes accept local and immediate-child output aliases, directly
and through wrapper aliases. They reject deep port/register aliases consumed
by DataOutPort or Alias, and a local-only slice of a mixed local/deep alias.
They also reject child-register alias consumers; that separate rule is left
open. Evidence: build/alias-consumer-controls-evidence/bundle, four accepted
and seven ICL57-1 rejections. The observational harness exit1 reflects its
all-expected-ok probe baseline, not a regression verdict or an equivalence
certificate. Remote run: /tmp/icl-alias-consumers-controls-20261113 as
lianghuaxiao@192.168.6.20. The initial missing-control ICL71 failures remain in
build/alias-consumer-probe-evidence; they are not alias rejection evidence.

## Scope and checks

Modify src/pattern/pattern_netlist.cc, existing tests/src/pattern_netlist_test.cc
and tests/src/test_icl_pattern_generation_main.py, guide/model/spec and vendor
evidence ledgers. Reuse connection_hierarchy.icl where useful; no new test script
or dependency. Inspect every resolved consumer collection: inputs (including
bindings and Alias Source), mux choices, logic instruction signals, one-hot
source bits, and interface/common-chain ports. PDL resolution is separate.

The classification/consumer passes are O(N+B) for N nodes and B already-resolved
references, with O(N) flags and no per-reference copies. Existing elaboration
budgets bound these structures. Run the existing5000-instance/wide benchmark
and include a representative alias-heavy native case; record wall/RSS without
asserting unmeasured performance gains.

- [x] Add failing native tests for deep register/port aliases in output,
  binding, selector, logic and wrapper contexts; whole-alias restriction on a
  local-only slice; forward and backward declarations. Preserve local alias
  chains, immediate-child output aliases and deep aliases without ICL consumers.
- [x] Add CLI atomic rejection cases and retain original/flat/generated PDL
  replay for deep targets. Demonstrate red failures before implementation.
- [x] Implement the post-Connect check, run focused tests and independent review.
- [x] Run full dynamic/static regression, affected leak-enabled sanitizers,
  format/naming, scale checks; record evidence and unresolved rule boundaries.
- [x] Update current ledgers and commit the verified change.

Implementation is integrated from /tmp/icl-deep-alias-20261113 based2ba288d.
CheckAliasConsumers runs after Connect and before provenance restoration. Its
classification skips literals, checks direct target scope/parent relationships,
and returns early when no deep aliases exist. All resolved consumer collections
are checked without expanding aliases or following port drivers.

Red evidence: build/deep-alias-red-{native,cli}.log/.exit(1), with native
elaboration accepting an invalid consumer and eight accepted-invalid CLI
subcases. Focused native and31CLI methods pass in build/deep-alias-focused-*,
CLI7.875s. Reviewer-requested one-hot/common-interface/chain-interface cases
bring the native negative table to12contexts in both declaration orders;
build/deep-alias-consumer-coverage-native.log/.exit(0). Source/test review is
clean. Initial formatting and subsequent brace-style failures remain recorded;
final-style.exit(0), final-naming.exit(0), and whitespace checks pass.

The native positive chain has5000aliases and124,059ICL bytes, including unused
deep aliases, so it exercises the complete consumer scan. Parse plus elaboration
measured0.021763s initially and0.018140s after the added negative cases; initial
whole-native wall0.42s/119,132KiB RSS. These are scale observations, not speedups.
Three alternating paired Linux/g++16.2.1 -O2 runs of the existing5000-instance/
377,286-byte/15,002-node and wide sparse/bus benchmark pass. Baseline whole
wall0.84–0.89s/RSS162,696–162,716KiB; changed0.83–0.89s/162,708–162,844KiB.
Main elaboration66.971–80.447ms versus58.612–64.474ms; noisy single paired sets
do not establish a general speedup. Logs: build/deep-alias-benchmark-*
with all six exit0 sidecars.

Final full regression runs in build/deep-alias-final-full.log/.exit.
Final leak-enabled ASan/UBSan checks reuse the clean Task40 directory, rebuilding
changed source/test objects with unchanged declarations, and select native
netlist, main31, apply-end-state and merge-observation CLI suites. Evidence:
build/deep-alias-final-sanitizer-*. Build and all selected checks pass: native
netlist, main31(22.589s), apply-end-state11(7.298s), merge-observation20(13.610s),
with final-sanitizer-all.exit(0).

Final full regression exits0:39 dynamic and39 static generator suites, native
checks,50 directional/seven legacy vendor standards,2,519 ICL files exercising
334/119 parser/scanner rules, and six editor tests in177.360s. Five optional
ANTLR integration checks remain skipped. Final source and evidence review is
clean. Strict authored coverage and immediate-child/source-kind rules remain
open; the alias probes add no equivalence certificate.
