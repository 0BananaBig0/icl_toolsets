# Raw builtin data registers on one TAP

Task110 closes the builtin DR boundary left by Task109, starting with bound
BYPASS and IDCODE on one currently selected BSDL TAP. Reuse the exact single-DR
CSU transaction proof, active register observation, stable connectivity checks
and existing ordinary symbolic/hardware planner. No new TAP replay engine.

The existing public iWrite/iRead prohibition on synthetic TAP data-register
targets remains. Executor choice: add a private call-local optional allowed
register ID to Impl Apply/Solve, supplied only by a qualified raw TAP request
for that exact active builtin. The normal public Apply leaves it empty. Do not
store privilege in scheduler state or retain it after adoption; each resolved
synthetic operand must equal that one allowed ID. Preserve default callers.
Source output remains raw iScan and never exposes internal synthetic writes.

Do not silently retarget an instruction: exact one non-instruction CSU, physical
endpoints/serial streams/masks and one register's loads/writes/expected must match
before full trial adoption. A prior raw IR may select BYPASS; reset selects the
actual BSDL IDCODE opcode. Unsupported controller opcodes/associations that need
extra setup still reject. Known SI only, fixed standard widths from metadata,
explicit/inferred root client/default-chain and ordinary prep/cancel/queue/owner
semantics remain. Multi-register, multi-TAP, parallel/child and raw merge stay open.

Native independent literal TMS100/N-1 zeros/110, N+5 clocks must prove BYPASS1
and IDCODE32 at all endpoints/state, capture masks and subsequent idle/capture.
Include protected IR during DR, failure/retry/assigned publication and public
synthetic-target rejection before/after raw success and on copied schedulers.
CLI source/flat/generated/reset replay, source order/cancel/both builtin modes,
unchanged instrument DR and IR regressions. Genuine strict vendor directions,
matched-option repeat/model timings, independent review and full/SAN/remote
gates precede commit/ff/delete. No dependencies, budgets or clock/reset semantics
change. This is a small private authorization parameter inside existing logic,
not a public command or relaxed source-target contract. New scripts under tests/.
