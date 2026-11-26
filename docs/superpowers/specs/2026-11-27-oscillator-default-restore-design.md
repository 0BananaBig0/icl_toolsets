# Oscillator default restoration

Task196 advances item5 of the remaining-work inventory, with bounded item12
vendor qualification. Item1 remains excluded. The user has authorized continuous
implementation, repository/owned temporary work, commits, parent merges, pushes
and completed branch deletion. Implementation choices below are the executor's.

## Required behavior

For a valid ToClockPort with an intrinsic ICL Period, `iClockOverride target`
with no options restores that oscillator definition. It must replace a previous
explicit source/ratio or period definition. A previously checked clock keeps its
version until a new `iClock`; the restoration must not rewrite earlier checks.
When the generator retains a period-bearing program, normalize restoration to
`iClockOverride target -period magnitude unit` using the actual declared ICL
period. Otherwise preserve the established lowering of clock checks and waits.

IEEE1687-2014 section6.4.6.20, printed103–104/PDF117–118, defines Period as an
embedded oscillator and excludes simultaneous source, factor and differential
properties. Section7.9.15(b), printed174/PDF188, recognizes that oscillator as a
valid source. Section7.9.16(c–e), printed175/PDF189, takes omitted override
properties from ICL. Restoring the intrinsic oscillator is the native inference
from these rules; IEEE's command grammar has no explicit `-period` option.
The local Tessent Shell Reference v2023.4 iClockOverride topic documents that
extension and different prior-field omission rules. Vendor references therefore
specify the actual default period explicitly, without claiming raw empty vendor
commands implement native defaults.

## Chosen implementation

Reuse SystemClocks::Override's oscillator definition path when the validated
target has `clock_period` and the operation has no options. Read the declared
positive magnitude/unit rather than the last override. Reuse PeriodWords,
the versioned Definition history, safety checks and atomic publication. No new
state, API, parser rule, dependency, algorithm or time/event engine is needed.
Routing empty commands through an invented source would lose oscillator identity;
a general time engine would add unrelated behavior. Neither fits this change.

Keep factor-only oscillator overrides and unsupported hardware fanout guarded.
An unconfigured source-less ToClockPort still requires a source. Preserve cycle,
invalid-period, differential and failed-output checks. No RTL changes are made.

## Verification and scale

Maintain tests in existing root tests files. Check original/associated-flat/main/
both/local-reset forms, exact ordered clock statements, declared period units,
source replacement, vector bit independence, procedures, old/new checked clocks,
unconfigured targets, failed output publication and failed native normalization.
Native snapshots must assert root names/cycle counts independently of generated
PDL string equality. The new positive tests must fail on the parent first.

Measure three rounds of 2/8/32 oscillators by 8/128/1024 alternating restoration
updates, with five replay forms and captured input/output identities. Each point
must finish within10 seconds and512 MiB RSS. Compare matched ordinary parent/
current controls without changing budgets or claiming unmeasured speedups.
Run both generator variants, affected leak-enabled sanitizers, full project,
corpus, C++ quality and source packaging on frozen source/product identities.
Configured Tessent2026.2 reference/generated pairs cover explicit original-period
restoration and relative operations; they do not establish physical timing,
standalone export replay or vendor raw omission equivalence.

## Integration

Parent: master at0eae49b646f995990d498ad8a2aa2e901de72c41.
Branch: codex/oscillator-default-restore. Reuse .worktrees/bsdl-defaults and its
qualified baseline; preserve117 stashes and unrelated worktrees/ignored ledgers.
Use one fresh whole-branch independent review, one test-first Important/Critical
repair pass if needed and no re-review. Inherit38 deferred Minors. After verified
merge/push, remove only the owned task branch and archive only its own ledger.
The broader pending inventory stays open until its actual requirements pass.
