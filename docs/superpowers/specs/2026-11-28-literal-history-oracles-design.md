# Literal payload and opaque history oracles (Task213)

Close the two remaining regression gaps from Task171 and Task183: directly
observe a parameter-sized wide decimal default without overriding that value,
and make the direct named omission-note fixture distinguish queued call order.
Reuse current actual source/flat/generated replay helpers and literal expected
payloads. Keep producer behavior and all current comparison/history semantics.

For the decimal case use W160 and value2^64+9, with `$W` sizing on the decimal
literal and an instance that leaves V at its default. Pin the entire zero-padded
160-bit native observation and flattened Source payload. Preserve the existing
instance-override case and hexadecimal control. This adds one method.
For the opaque case replace the first procedure's decimal SI6 with canonical
0b0110, matching the note exactly; retain both comment/status variants and the
literal final0000 mask. Under incorrect eager call execution the marker binds
past the second call and leaves xx00, so the assertion now discriminates order.

Use actual native processes for corruption controls: zero only the default
literal while leaving the old instance override intact, and replace merge
wrappers with eager sequential calls. Private mirrored meta-tests first show
both old tests accept those errors, then show the maintained tests reject them.
Retain raw inner assertion failures and outer RED-to-GREEN logs. Archive private
probes under tests/tests/src/pattern before packaging; no synthetic binary output.

Only two maintained Python test modules and tracker/spec/plan change. All
production sources, dependencies, Makefile and48 qualified products stay exact;
4133 other qualified inputs stay exact and the two changed inputs are rehashed.
Fresh qualification runs both full changed modules on normal, static and
leak-enabled sanitizer generators, plus decimal-module hexadecimal controls on
all three,9 reports total. Default fixtures run before further work. SAN runs
outside tracing sandbox with detect_leaks1 and UBSAN halt1; keep current limits.
Actual inputs are160 bits and fixed4/7-bit opaque chains; no throughput change
or unmeasured speedup is claimed. Retain Task208 broad producer qualification
only by unchanged source/product identities. Run source packaging and exact five
changed members; retain earlier build/install evidence for unchanged build code.

Standing question opt-out and inline execution apply. Direct parent master
693ce851bad93f374be2851e87ad8b5f0af651b2; branch codex/literal-history-oracles.
One fresh whole-branch review, one blocking RED-to-GREEN fix wave, no rereview.
Inherited25 minus2 candidates =23 pending independent confirmation. Preserve
117 stashes and unrelated worktree/sibling state; merge, verify, push, archive
only this task's private files and retire its branch. Broader inventory/item1
stay open/excluded respectively. No new language or hardware support is claimed.
