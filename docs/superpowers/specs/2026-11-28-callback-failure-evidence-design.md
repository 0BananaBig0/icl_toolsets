# Callback failure evidence (Task220)

Close three inherited findings. The merged-callback benchmark must terminate
its measured process group on deadline, including children. Reuse the installed
GNU timeout pattern already used by benchmark_merged_readback: wrap the native
command with timeout --kill-after=2s 27s inside /usr/bin/time. Retain the Python
30-second outer deadline, 10-second measured limit and existing memory bounds.
The 27+2-second inner limit leaves cleanup time before the outer deadline. Avoid
new wrappers, dependencies, runtime APIs or configurable deadlines in the driver.

Add a real Unix parent/child regression. A test-only PATH shim shortens the GNU
deadline for the deliberate sleeper only; real generator controls retain 27s.
Require proof the candidate started, timed out and did not leave a delayed
child marker. The old driver should allow that marker before failing, exposing
the missing cleanup without leaving a long-running child. No simulated clocks.

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
