# Clock replay cleanup (Task211)

Close Task210's remaining temporary-script cleanup finding. The published
wrapper owns one active script under tests/. If copying it into scratch or
waiting for its child raises, remove that active file while preserving the
original exception, scratch copy and available log. Keep the assertion-enabled
preflight and native replay logic, commands, outputs and limits unchanged.

Use one try/finally around write/copy/run after the active filename is known;
unlink that exact owned path in finally. Existing straight-line cleanup misses
exception paths. A general temporary-file manager or new replay API adds no
value for this single resource.

Before editing, use a mirrored private test under tests/docs/ to execute the
real published code with controlled scratch-copy and child-wait exceptions.
Create real active/scratch files; assert actual active-path absence and retained
diagnostic artifacts. Mock only the boundary failure to avoid a race or running
native work during an intentional interruption. Observe both failures on old
code, then both passing with finally. Keep the existing optimization preflight
regression and run the final published normal command: 270 native processes,
16 parser calls, unchanged 10s/512MiB bounds and canonical hashes.

No C++ or maintained test/Makefile changes. All 4135 qualified inputs and 48
products stay identical. Archive private mirrored tests before packaging so
they do not enter the distribution. Run source build/install and four exact
document-member checks for guide/tracker/spec/plan. Reuse original broader
qualification only by unchanged identities. This local wrapper change needs
technical review, not another reader-design phase; user steps stay the same.

Standing opted-out questions and inline execution authorization apply. One
fresh whole-branch review, one verified Critical/Important repair wave if needed,
no rereview. Candidate findings: 26 minus 1 = 25. Preserve 117 stashes, foreign
worktree state and sibling ledgers. Merge, verify, push, retire only this task
branch and continue the original inventory.
