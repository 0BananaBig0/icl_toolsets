# Raw data scans on one bound TAP

Task109 extends the existing qualified raw IR transaction path to known-binary
raw DR on one bound scalar root client-TAP, explicit or inferred unnamed. The
currently selected DR must consist of exactly one full modeled ScanRegister.
Do not silently retarget the IR or open a child: ordinary queued preparation
may do so before the raw CSU, in either source order.

Reuse the existing TapExecution active-register observation and ordinary Apply
on a copied scheduler. Add a read-only ActiveDataRegisters(tap-name) query that
returns the existing Active calculation, without new route/FSM/event logic.
Ignore inactive entries, reject unknown selection and multiple active registers.
The existing exact-one-CSU proof must now check instruction=false for DR and
exact whole-register loads/writes/expected, bound endpoints and reversed SI/SO.
This proof also rejects partial/aliased/inverted/extra-event routes that cannot
be represented by the one full register. Adopt full trial only after proof and
optional post-stable qualification; publish only the pre-CSU assigned register.

Share root TAP qualification, normalization and private request/proof with IR.
Source defers readiness, selected geometry and width to Apply, but immutable
binding/options/known-SI/bounds stay early. Strict native default checks active
geometry immediately. Keep IR/data queue keys distinct so a mixed raw group
rejects explicitly instead of silently overwriting a different scan kind.
Same-kind last request and reset cancellation remain. -stable requires a
supported single-register post-route with unchanged width; broader post-route
shapes are explicitly outside this first scope. Unknown SI, multi-register DR,
multi-TAP/parallel/child-TAP and raw merge remain open. Builtin DRs remain unsupported because the existing ordinary backend forbids
synthetic TAP data registers as PDL targets; do not bypass that contract.

Numeric/default-chain bound trace normalization shares the existing parser for
DR as well as IR. IEEE1687-2014 7.9.12 and1149.1 DR from Idle uses TMS100,
N-1 zeros,110: N+5 clocks, LSB first, final shift exits on its last bit.
Independent literal pin replay checks every endpoint, retained state and idle.
Known SI authoritatively refreshes owner/partial history; SO stays observer.

Executor choice under continuing inline/confirmation opt-out: reuse active
observation and existing ordinary exact-CSU proof. New TAP replay engines or
per-register speculative solver trials would duplicate behavior or scale poorly.
Assume real DR8-32 bits; characterize8/32/256/4096,1024 inactive registers and128
repeats against same-option prior ordinary planner. Retain inherited limits and
record expected resource rejection rather than claiming1M TAP throughput.
TDD, focused/full/SAN/remote, genuine strict vendor qualification, independent
review, then commit/ff/delete as in Task108. No dependencies, RTL or new clock/
reset semantics. Maintained new scripts under tests/.
