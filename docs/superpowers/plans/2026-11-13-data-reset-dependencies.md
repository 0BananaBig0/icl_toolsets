# DataRegister reset dependencies

Task35 continues the required acyclic reset/event integration. Task34 exposed
a false feedback diagnosis when a DataRegister drives a ScanRegister reset:
BuildResetOrder visits only scan storage but counts all register dependencies.
Native NetworkState already propagates reset through both storage kinds.

Use one acyclic dependency graph for modeled register resets, preserving the
existing feedback diagnostic for actual cycles. Reuse bounded topological
ordering and concrete reset replay. Include DataRegister reset transitions
before/after data updates, selected-client settlement and controlling fill.
Do not conflate WriteDataSource cycles with reset cycles or claim arbitrary
feedback support. Local-reset prerequisites may only schedule storage whose
existing write path can actually implement the assignment.

Expected workloads retain the current 4096 scan/data register bounds. Build
the graph once; traversal should remain linear in nodes plus dependency edges
after source-cone discovery. Measure a mixed reset chain and retain existing
work budgets. No new clock model or solver is planned.

- [x] Reproduce false feedback on constant/primary-driven DataRegister reset
  controls, mixed cascades, real cycles and primary updates that assert reset.
- [x] Include all necessary storage in reset ordering and event propagation;
  update affected consumers and native/CLI tests without weakening replay.
- [x] Run focused, full dynamic/static, leak-enabled sanitizers, naming/style,
  independent review and Tessent probes; record exact evidence and limitations.

Task33–34 source stays frozen during its combined regression. This plan/test
preparation is separate; implementation starts after that checkpoint.

Design review: keep registers_ scan-only and DataRegisterPlan's write/address/
enable DAG separate. Reset settlement must mirror simultaneous native snapshot
passes and preserve transient assertions, rather than only evaluate the final
topological values. Mirror drives→Reset→ordered data updates→Reset and scan
Update→Reset→ordered data updates→Reset; keep Task34's event qualification.
Include held DataRegister values in prerequisite solving. Initially keep these
controls fixed unless their automatic write path is explicitly supported and
replay-verified; diagnose necessary unsupported changes without rejecting an
already released control. Add mixed Scan/Data cycle, reverse-declared cascade
and native actual-storage/fill tests. Preserve fast4096-scan construction;
charge expanded reset work where it is actually used.

Completed with13 CLI methods and native history/reset/fill checks. Final
dynamic/static checks cover35generator suites,47directions,seven legacy
standards and2,516ICL files. The full command's final editor failure was an
inherited Vim runtime; the isolated editor rerun passes all six tests.
Leak-enabled native/main25/argument12/new13 checks, formatting, naming and
independent review pass. Exact logs, vendor scope and unresolved general
event/reset requirements are recorded in the pattern-generation guide.
