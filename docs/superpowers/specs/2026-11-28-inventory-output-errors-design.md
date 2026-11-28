# Inventory output errors (Task218)

The model corpus inventory must exit 2 with its existing cannot-write diagnostic
when a short buffered report cannot reach stdout. Its final stream check currently
precedes the final flush. Flush once after the batch, then reuse that check. This
keeps one process per corpus batch and avoids per-row flushing. Use the standard
stream operation; no new abstraction, dependency, protocol or runtime-model change.

Add a real /dev/full regression to the existing mirrored helper tests. First run
it against the qualified parent binary and observe an assertion failure because
exit 0 was returned. Then rebuild normal and default-leak sanitizer inventory
helpers; run the complete helper/protocol test module and corpus audits for both.
Check formatting, clang-tidy and C++17 compilation for the changed C++ helper.
Retain broader generator/native regression only with exact unaffected source and
product identities. Run source packaging/build/install and compare the five
changed package members exactly. Preserve all resource limits and default flags.
The existing workload is a single batch over 1,334 accepted files/1,763 default
models. A single end flush preserves buffered throughput; no speedup is claimed.

Also close Task217's public-summary spacing follow-up by adding normal spacing
and punctuation throughout that section, preserving every value and status.
No other prose rewrite. Parent master 54bb3792e28be84787cbcc48a3964af31bc22bd2;
branch codex/inventory-output-errors. Standing question opt-out and inline,
merge/push authorization apply. One fresh whole-branch review, one blocking
repair wave if needed, no rereview. Inherited 19 minus 2 candidate closures = 17.
Preserve 117 stashes and unrelated state. Merge, verify, push, archive only owned
helpers/ledger, detach reusable worktree and safely retire the task branch.
Broader original items remain open and item 1 stays excluded.
