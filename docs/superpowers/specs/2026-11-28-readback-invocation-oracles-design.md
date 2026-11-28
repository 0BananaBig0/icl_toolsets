# Complete readback invocation-coordinate oracles (Task216)

Close Task215's test-only follow-up. Compare completed-call and active-getter
role blocks with complete expected invocation strings derived from the authored
fixture: actual pdl path plus literal call-site line, column and command number.
Keep all three getters, direct/merged calls and depths0/2. Exact ordered lists
must retain every recursive duplicate and both role boundaries, with original
cause/getter coordinates and protected artifacts unchanged.

Use literal fixture coordinates, not values parsed from the output being tested.
The completed fixture has root call9:1/16(command6/7), inner call7:17(command1).
The active fixture has root11:1/16(command6/7), initial inner9:17(command1),
recursive inner4:17(command1). Its getter7:9 and failing Apply6:45 remain separate.
Replacing prefix checks with complete lists is sufficient; no producer change.

Private mirrored meta-tests execute real existing native failures, then corrupt
only each invocation's filename/line/column/command fields in the diagnostic.
Old tests must still accept these wrong fields (outer2 failures); new tests must
reject all6 completed-call and12 active-getter subcases (outer2 methods pass).
Preserve original/corrupted strings and source hashes. This mutates test input
diagnostics, never modeled observations or binary behavior. Archive/remove the
private tests/tests/src/pattern probe before packaging.

Only one maintained Python module and tracker/spec/plan change. Preserve4134
other qualified inputs and all48 products. Fresh whole-module runs on normal,
static and default-leak sanitizer binaries give3reports/57methods, with no skips.
Keep original limits/flags. Retain Task215 native/quality/source-build evidence
and Task214 broader evidence only by unchanged compilation inputs/products;
report no fresh full project/native/source-build run. Run make dist and verify
four exact changed package members, final inputs/products and whitespace.
Workload is existing fixed4/7-bit fixtures and bounded depth2; no performance
change or speedup is claimed. No dependency/API/Makefile changes.

Standing question opt-out and inline execution apply. Parentmaster
f2d0fa08adc05850ab40238a8c65aa6910a27e46; branchcodex/readback-invocation-oracles.
One fresh whole-branch review, one blocking RED-to-GREEN repair wave if needed,
no rereview. Inherited23 minus1 candidate =22. Preserve117 stashes and unrelated
state; merge, verify, push, archive only owned private files/ledger and retire
branch while preserving reusable worktree. Broader inventory/item1 remain
open/excluded respectively.

Review repair: retain exact block boundaries as well as coordinate values. Four
additional real-native mutation controls reject36 misplaced-frame/blank-gap
cases (RED4 failures, GREEN4 methods). Preserve prefix role-row exclusion and
unfiltered complete suffix comparison; final qualification has8 gates and
6meta methods/54 rejected corruptions. Rerun all3variants on final module bytes.
