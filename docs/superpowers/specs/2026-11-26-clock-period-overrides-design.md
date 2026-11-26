# Explicit clock period overrides

This bounded increment of original item 5 adds the documented Tessent v2023.4
iClockOverride target -period value [unit] form. Parent master: ea779141.
The valid oscillator/control probe in /tmp/icl-next-period-probe-20261126 proves
the current frontend rejects the option and preserves prior output. User standing
completion/native execution and per-task integration authorization applies.

## Design and approaches

Retain clock definitions, logical iClock targets and logical -sck waits in any
executed program containing a period override. This preserves independently
checked clock snapshots when one oscillator changes period; collapsing them to
one root would lose which definition each target checked. Existing routing,
ratio, cycle, ownership and fanout checks still run. Programs without a period
override keep their existing normalization and omission rules.

A complete absolute-time/phase lowering engine would need additional event and
quantization semantics; it is unnecessary for preserving explicit clock models.
Accepting and dropping the new option loses information. Retaining the clock
program is the selected engineering approach within the delegated task.

## Contract

- Accept a positive PDL time literal, attached or separate supported time unit,
  or a unitless value defaulting to ns. Reuse current PDL time spelling rules:
  integer/decimal mantissas, supported negative scientific exponents, and
  s/ms/us/ns/ps/fs/as. Normalize emitted period values to one compact token.
- The period form is exclusive of source/frequency options. A period override
  makes that ToClockPort bit an explicit oscillator root at its checked version.
  Reuse existing name/index/flat-origin resolution; support hierarchy and vectors.
- In period-bearing programs, mixed source/ratio overrides require explicit
  -source, -freqMultiplier and -freqDivider. Tessent v2023.4 omission semantics
  differ from the existing portable-core contract, so partial mixed forms reject
  with a precise diagnostic instead of emitting ambiguous definitions.
- Keep declarations and logical waits before and after overrides, procedure
  return and reset. Preserve every target's checked version and ordinary
  pre-period behavior. Propagate preservation into local-reset rendering groups.
- Validate Tcl-resolved values through the shared frontend; preserve the existing modeled-clock exclusion and independent PIO
  observations, source/flat/generated/local-reset replay and failed artifacts.
- Retain physical-PDL, modeled raw/opaque, differential override, stateful time
  and unsafe clock-event boundaries. No new dependency, resource limit or
  invented hardware period. Native evidence validates models and replay; it does
  not establish physical elapsed-time accuracy or timing signoff.

## Scope and verification

Extend shared PDL validation and SystemClocks' existing versioned definitions;
add only a period-root marker. Use an optional normalized-operation output from
Override so ordinary programs avoid extra operation copies. Expose canonical
clock-target naming for retained statements. Renderer detection is one linear
scan of executed operations, with its decision passed to recursive rendering.

TDD parser/default/unit handling, multiple checked targets across period changes,
mixed fully explicit overrides, hierarchy/vector/flat names, procedure/reset
replay, independent modeled PIO, guarded mixed histories and atomic invalid-input publication. Native tests
check root routing, retained checked versions, copying and rejected-update state.
Measure 2/8/32 oscillator bits with 8/128/1024 updates, five replay forms under
10 seconds/512 MiB, and matched ordinary programs against the parent. Full check,
affected sanitizers, changed C++ quality and packaging precede one fresh review,
then merge/master checks, push and owned-branch deletion.

Technical reference: Tessent Shell Reference Manual v2023.4, iClockOverride,
iClock and iTake topics under
/home/banana/win11/Desktop/tessent_2023_doc/htmldocs/tshell_ref/topics/.
