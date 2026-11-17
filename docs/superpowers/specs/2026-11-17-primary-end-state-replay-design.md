# Primary end-state replay stability

Fix the inherited extra initialization during generated PDL replay. IEEE1687
6.5.2(h)(6) limits a known temporary override to one iApply. Generate emits each
ScanStep as a separate apply. The joint scheduler permits unknown primary
end-state bits in an intermediate scan, before a later source-goal update;
replaying that scan adds default initialization.

Extend existing ConstrainEndStateFrames and the primary-only solver using one
private ConstrainPrimaryEndStates helper: declared primary end-state bits must
be known on an emitted frame. Existing AppendDataEndOverrides already emits
known nondefault values as overrides. Preserve the scan-register proof, concrete
hardware replay, reservations, source override lifetime and unrelated unknowns.
No repair loop, new solver/API, fake iState, dependency or raised guard.

Approaches: post-generation repair repeats solves and hides proof mismatch;
blind initial defaults can alter resets/controls. Extend per-frame constraints
so the current hardware/control/reservation proof chooses a legal drive. This is
an agent decision under the user's standing instruction to proceed without
questions. Parent master85961e9, existing isolated worktree.

Remove Task115's explicit marker initialization; require exact source/flat/
generated/flat-generated replay. Cover default0/1, indexed/inverted aliases,
known overrides and read-only IR. Check a supported pure-primary multi-frame
fixture before deciding whether its solver also needs changes. Existing mixed,
data, reset and protected-resource suites guard callers and rollback. Constrain
only declared default bits; never concretize unrelated unknowns.

Work is linear in declared default width per existing frame, using existing
Z3 masks; retain1024-bit timing and existing bounds. Fresh focused/style/native,
full, SAN, remote and independent final review precede merge/delete. Existing77
certificates must pass; recertify affected vendor semantics if any.

Confirmed scope: a supported shared-address a3/b5 plus marker1 apply also fails
primary-only generated replay. Its first frame defers marker, causing an inserted
default write on replay. Both solvers require the same knownness mask proof.

Review fix pass: a known nondefault primary on a comparison-only frame is also
unrepresented. Shared helper requires an actual proved input event or matching
default on emitted frames. Existing override emitter stays unchanged; never add
an unproved physical drive after replay. Addressed read-a/write-b with initial
marker1 reproduces extra marker0 on generated replay before this fix.
