# Addressed-port source terminal eligibility implementation plan

> **For agentic workers:** Use superpowers:executing-plans inline under existing
> autonomous authorization; preserve source review and final integration checks.

**Goal:** Reject unrelated control and cross-family addressed-port input bindings.

**Architecture:** Add distinct Address/WriteEn/ReadEn terminal families to the
existing cached source classifier. Check each child Source against its matching
family plus DataInPort/DataOutPort and both ScanRegister bit kinds. Constants and
other ordinary-data terminals are excluded. Distinguish cached unconnected holes
from constants. Keep data ports ordinary-data eligible in all existing consumers;
keep strict ScanInSource exclusions. Do not globally make addressed ports ordinary
data. AddressResolver checks direct-domain topology, inherited offsets, complete
bus order and shared-register constraints when resolving addressed operations;
complete elaboration-wide direction/no-intervening-logic validation remains open.

**Tech Stack:** C++17; existing native and Python tests, no new dependency.

**Spec:** [Approved generator requirements](../specs/2026-10-25-icl-pattern-generation-design.md),
illegal connectivity validation. IEEE1687-2014 §6.4.7(g), p.111, plus specific
permissions §6.4.6.24(b)/25(c)/26(d), pp.108–110; inherited associations
§6.4.7(s–u), p.112; top-level DataInPort domains §6.4.9(l), p.120.
Inspected primary text: build/combined-audit/ieee1687.txt lines6756–6870,
6969–6977,7007–7018,7518–7522. Independent conceptual review confirms the
matching parent-port permissions override an ordinary-data-only reading.

## Global constraints

Keep root60/61 frozen during verification; work in the existing isolated checkout.
Retain hash-backed verified checkpoints before advancing branches. Preserve
current explicit parent mappings, aliases, AddressValue inheritance, ScanRegister
and top-level DataInPort domains. This is terminal eligibility, not completion of
no-intervening-logic or all address-context restrictions. New scripts belong under
root tests; extend current native/main/address tests instead.

## Review focus

Three distinct matching families; cross-family rejection; per-bit mixed aliases;
partial binding holes and virtual resets; original/flat replay and atomic output
preservation. Verify existing full parent-bus and primary-input domains, not merely
isolated ports. Reuse current alias/fanout scale cases; the shared cached algorithm
and expansion budgets remain unchanged.

## Task62

Files: src/pattern/pattern_netlist.cc, tests/src/pattern_netlist_test.cc,
tests/src/test_icl_pattern_generation_main.py; current guide/model/plan ledgers.
Native pattern_address_test is an affected caller; change it only for a concrete
missing regression. No public interface, grammar, serializer or new graph pass.

- [x] Read primary clauses and current AddressResolver/callers; conceptual review.
- [x] Qualify matching/alias/ScanRegister/DataInPort controls and unrelated-family
  candidates with loading-only vendor probes; classify setup/domain errors separately.
- [x] Add CLI/native RED for unrelated and cross-family Source acceptance;
  source-located errors must preserve pattern/reset/debug artifacts.
- [x] Add matching/alias/data/partial/mixed/flat positives and implement source checks.
- [x] Focused netlist/address/flat/main and affected scheduling checks, naming/style,
  representative scale measurements and independent review.
- [x] Integrate after60/61 verified commit; full dynamic/static and affected
  sanitizers; document measured scope and commit.

Prepared23 reviewed loading-only fixtures under build/address-port-source-vendor-probe.
Child contains only addressed ports, so unused-port checking must be demonstrated
by source-specific negative diagnostics before interpreting acceptance. Individual
scan cases mix a ScanRegister source with parent ports; all_scan/all_data controls
distinguish a terminal rejection from complete-domain constraints. Loading alone
does not establish inherited offsets or read-enable sequencing.

The first23 probes completed:14 loaded (matching/alias/DataIn/ScanRegister plus
all-data/all-scan controls),9 source-specific ICL65 rejections (constants/cross-family/
TCK). Fetched input hashes match. Each ICL65 diagnostic lists matching parent port,
parent DataInPort, child DataOutPort or ScanRegister as the eligible union. This
corrects the initial permissive data/constant design before implementation. Further
nine boundary probes cover other data terminals and DataOutPort forwarding.

Focused qualification: boundary9 inputs match; six source-specific ICL65 rejects
for LogicSignal/DataMux/DataRegister/Select/ToSelect/OneHotDataGroup. Three DataOut
forwarding cases with Enable fail contextual ICL81, while all three without Enable
load successfully with matched hashes. Preserve all three independent evidence
directories and keep operation/direction qualification open.

Corrected CLI RED accepts all60 invalid cases; native RED accepts TCK. Initial
expanded RED/GREEN had nine invalid constant Alias constructions; use direct
literal bindings instead. Mixed positive aliases likewise use named clock bits
rather than illegal literal Alias elements. Earlier failed evidence is retained.
One pre-existing AddressResolver logic fixture now builds a legal model and then
injects a lowered LogicSignal connection, preserving its original rejection and
both failure-cache retries without a frontend exemption.

Confirmed main47 passes9.984s; native netlist/address/flat/scheduler, address CLI20
and scan-data CLI25 pass. Naming/style and independent product/fixture review are
clean. Four final files are hash-backed up at /tmp/address-port-source-task62-files.
Root integration follows the verified60/61 commit3deaafc.

Full integration passes in build/address-port-source-final-full.log/.exit0,
1094.305s wall:41 dynamic/static suites,51 directional/seven legacy goldens,
2523 ICL334/119 rules, editor6 in180.025s; five optional ANTLR tests skipped.
Leak-enabled native scheduler/flatten/netlist/state/TAP/address and main47/clock41/
client18/reset12 pass in build/address-port-source-final-sanitizer-*. Four final
source/test hashes match the reviewed Task62 checkpoint. Naming/style and
independent source/fixture review pass. Strict coverage, formal vendor certification
and platform refresh remain open.

Final netlist0.79s/120704KiB RSS, clock aliases4096:0.00985495s;65536-bit/4096
consumers0.0184732s. Address native0.68s/117048KiB RSS. No speedup claim.
