# Mixed clock override defaults

Continue completion items 5/12 by accepting representable partial source/ratio
overrides in a period-bearing clock program. Parent master is bb7d7c3. The user
delegated implementation choices and authorized inline execution and integration.

## Approach

Reuse SystemClocks::Override's validated Definition. Its native semantics restore
omitted source and factors from ICL on each override, as IEEE 1687-2014 section
7.9.16(c–e), printed 175/PDF189, specifies. The installed Tessent Shell Reference
Manual v2023.4 iClockOverride topic instead documents retaining some prior fields.
Emit all three concrete source/ratio fields when main PDL retains clock programs,
for partial forms so replay does not depend on that difference. Fully specified
forms preserve their established argument order and numeric spelling. Previously
the generator rejected every partial mixed form. Ordinary programs keep their
current normalization.

Keeping the guard leaves valid native operations unsupported. Emulating vendor
omission rules would change established native semantics. Explicit expansion is
the selected approach and uses existing validation, names and integer factors.

## Contracts

For a ToClockPort with Source ck, FreqMultiplier 3 and FreqDivider 2, a divider-only
override to 5 emits source ck, multiplier 3 and divider 5. A source-only override
to other emits other, 3 and 2. An empty override emits ck, 3 and 2. Prior explicit
overrides and period versions never supply omitted native defaults.

Expansion requires a non-inverted ClockPort/ToClockPort source bit. An omitted
source resolving to a ClockMux, alias, inverted edge or disconnected node remains
unsupported in a retained program; do not freeze a dynamic route or invent a new
clock port. Explicit -source may replace such an ICL source. Oscillator-only
factor forms and source-free oscillator restoration retain their current guard.

Preserve checked-version histories, cycle and fanout rejection, procedures,
associated-flat vector names, source-preserving reset output, copy/rollback and
failed output publication. No parser, public API, resource limit, dependency,
physical timing or observation-provider changes. Keep item1 excluded. This
increment does not close stateful time or the broader inventory.

## Acceptance

Test all source/multiplier/divider subsets and omitted-field restoration after a
different source, ratio and period. Assert full literal ordered clock programs
through original/flat/generated/flat-generated/reset replay. Check procedure and
nonzero vector names, previous checked targets, and unrepresentable source guards.
Native normalization must emit all seven arguments atomically and leave the
output sentinel and state unchanged on a rejected expansion.

Run focused normal/static tests, legacy 72 clock methods, frontend tests and native
clock snapshots, then full make check, affected leak-enabled ASan/UBSan, changed
C++ format/tidy/C++17 checks, corpus and source packaging. Reuse qualification
machinery without repeating checks after unchanged files merely for reassurance.

Measure 2/8/32 clocks by 8/128/1024 partial overrides in five forms, three rounds,
and matched ordinary parent/current controls. Each case must meet 10 seconds and
512 MiB. Retain actual measured input and five output bytes/hashes and bind the
tools; compare repeated rounds. Record capture overhead and avoid speedup claims.
Fresh bounded Tessent 2026.2 configured runs compare explicit native-default
references with generated PDL; literal expected relative operations must agree.
Do not call the original partial source vendor-equivalent: its omission rules
differ. No standalone export-period or hardware timing claim.

One fresh independent whole-branch review precedes merge. Re-grade every finding
and declined area, fix Critical/Important in one test-first pass, defer Minors,
and do not re-review. Verify the merged result, push and retire only this branch.
