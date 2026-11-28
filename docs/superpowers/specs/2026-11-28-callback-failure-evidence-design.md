# Callback failure evidence (Task220)

Close three inherited findings. The merged-callback benchmark must terminate
its measured process group on deadline, including children whose parent has
already exited. Reuse the repository's Popen/start_new_session/killpg pattern.
Retain the 30-second communicate deadline, 10-second measured limit and existing
memory bounds. Always kill the isolated group when communication finishes or
times out, then drain output after timeout and report the specific deadline
failure. A parent exiting must not remove authority to stop remaining children.
No new wrappers, dependencies, runtime APIs or configurable driver deadlines.

A real Unix parent/child regression gives the child TERM resistance and writes
readiness only after fork and signal setup. Its parent exits before the child's
delayed write. A test-only communicate wrapper shortens the deadline solely for
this deliberate sleeper and preserves real generator controls. Require child
readiness, the actual driver deadline diagnostic and absence of the delayed
marker after its scheduled time. A private fork-failure control must prove the
old oracle falsely accepts an early error and the new one rejects it.

MixedCallbackGroups.reset_replay must replay its combined reset rewrite once,
without adding its already-incorporated second PDL input again. A maintained
split-file counter must fail on the old helper and pass after removing that
extra argument. Preserve procedure bytes and source/flat/generated replay.
PrimaryCallbackState ownership rejection must assert the actual storage cause,
not just any failed process and protected output. Use actual native failure
and a private diagnostic-corruption control to prove old acceptance/new rejection.

Only four existing Python files plus tracker/spec/plan change. Preserve 4,131
unaffected qualified inputs and all 48 products from Task219. Fresh complete
three-module normal/static/default-leak sanitizer runs, exact seven-member
source package, identities and whitespace qualify final bytes. Retain prior
native/build/broad evidence by identity, without fresh such claims. Archive
private mirrored probe before packaging. Existing subprocess/resource limits
stay in place. The task alters only bounded test/benchmark execution and adds
no performance claim. Parent master 27996489119825e19b7666efcaa326ecff91f207;
branch codex/callback-failure-evidence. Standing question opt-out and inline,
merge/push authorization apply. One fresh review, one blocking repair wave if
needed, no rereview. Inherited 14 minus 3 candidate closures leaves 11 before
new findings. Preserve 117 stashes and unrelated state; merge, verify, push,
archive only owned helpers/ledger and retire the branch. Broader original tasks
remain open and item 1 stays excluded.

Review repair supersedes the original GNU timeout choice: it can return when
its direct child exits while a TERM-resistant group member survives. The new
isolated group remains under Python cleanup authority after parent exit.
Descendants deliberately leaving that group remain outside this contract.
