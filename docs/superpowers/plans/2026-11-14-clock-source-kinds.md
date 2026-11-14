# Clock source terminal eligibility

Execute inline under existing autonomous authorization. Keep Task54 root production
files frozen during verification; implement in the isolated worktree and preserve
independent review.

IEEE1687-2014 §6.3.12(e) identifies positive ClockPort/ToClockPort, TCKPort,
ToTCKPort, ClockMux and their aliases as clock terminals. The concat_clock_signal
grammar also permits ordinary data_signal terminals; independent conceptual review
caught this requirement before implementation. Preserve that union. This is
elaboration eligibility; SystemClocks retains stricter operable-route checks.

Extend the existing post-restoration alias-bit classifier with clock and constant
categories, preserving all existing data/ToIRSelect rules. Validate ToClockPort and
ClockPort Source inputs, ClockMux choice sources and DifferentialInvOf expressions.
Resolve DifferentialInvOf with the declaration width during Connect, before
flat-origin restoration; retain typed inputs for serialization, validation and
fanout checks. Raw flat property names are not valid after restoration. Differential negative ports
are not positive clock terminals, including through aliases. Do not add local-vs-
child direction checks or contextual control/address rules in this increment.
No generated/flat/name exemptions. Typed terminals stop classification; do not
mistake their transitive fanin for their declared source category.

Connected scope: pattern_netlist.cc; existing native netlist/flatten and main CLI
tests; guide/model ledger. No new runner/dependency. Reuse iterative per-bit alias
memoization, preserving linear graph traversal with ordered-map lookup costs.
Retain4096-deep alias and65536-bit/4096-consumer performance checks.

- [x] Inspect shared classifier and normative grammar; independent conceptual review.
- [x] Add failing invalid clock-source/negative-differential cases and positive
  ordinary-data/clock/alias/constant matrices, original/flat and atomic CLI checks.
- [x] Extend the shared classifier and cover every clock-source consumer.
- [x] Focused native/CLI/scale, formatting/naming and independent source review.
- [x] Integrate after Task54 commit; full dynamic/static and selected leak checks,
  document exact evidence and commit. Broader source conformance remains open.

Execution: first test-edit command used a worktree-relative path while already
in that worktree and failed before editing; its subsequent old-suite run is not
RED evidence. Corrected native matrix fails against Task54 with expected-failure
clock-source diagnostic (build/clock-source-red-native-2.exit1). Shared classifier
now preserves data/constant semantics and adds positive clock terminals.
DifferentialInvOf is connected as typed width-checked input before restoration;
serializer and BuildSafety use it without reparsing flat token names.

Focused tests exposed a connected serializer defect: an internal differential
ClockPort was lowered to DataOutPort while retaining DifferentialInvOf, which is
invalid ICL. Preserve that differential input as ClockPort in flat output and
validate its original kind on restoration; it is not an unconnected placeholder.
No new metadata fields/version are required. The positive matrix's empty
ToSelectPort fixture also used an existing invalid empty-body serialization path;
use its explicit legal constant Source instead of expanding this task's scope.
Initial main37/native failures are retained. Corrected build/tests pending.

Independent review found and corrected explicit parent binding loss on internal
differential inputs. An optional ninth node-origin field now preserves these
Source bits, while ordinary eight-field entries remain unchanged. Exact-width
references/constants are validated; reset reference rules remain unchanged.
The new reader admits OneHotDataGroup only for this source-binding context, rejects
conflicting unconnected flags and restores partial holes using existing special
bit metadata. Alias-consumer validation now runs once after restoration, covering
these injected connections under their original hierarchy. Corrected review is
clean. Native named/constant/partial binding roundtrips and main CLI valid-group,
unconnected-conflict and PDL-only-alias injection checks pass. The initial group
fixture lacked required DataOutPort Enable; adding its explicit constant enable
corrects the fixture, without changing product validation.

Evidence: clock-source-bound-red.exit1 reproduces lost binding; reviewed native
and flatten checks pass. The timed native suite takes0.64s,126456KiB maximum RSS;
4096 clock aliases take0.0177807s and65536-bit/4096-consumer case0.0314348s.
Main37/clock41 checks pass before the last review corrections; final main38 and
naming checks are underway. Root integration remains blocked only on Task54's
ongoing full verification, not user approval.

Final focused main38 passes in9.284s; reviewed naming and source review are clean.
The native group/constant/partial-binding checks and existing flatten suite pass.
Five Task55 files are hash-backed up in /tmp/clock-source-task55-files, ready for
worktree advancement after Task54 commit. No Task55 product imported to root yet.

Full integration passes in build/clock-source-final-full.log/.exit0,1103.136s
wall:41 dynamic/static suites,51 directional/seven legacy goldens,2523 ICL334/119
rules and editor6 in181.921s; five optional ANTLR tests skipped. Leak-enabled
native scheduler/flatten/netlist and main38/clock41/client18/reset12 checks pass.
Formatting, naming and corrected independent source review pass. Strict coverage,
new vendor certification and later platform qualification remain open.
