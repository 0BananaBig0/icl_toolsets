# Period diagnostic spans and edits (Task206)

Task205's review confirms validation but identifies two usability/test gaps.
Highlight a known invalid separate unit, rather than an adjacent valid dynamic
value. Maintain an actual versioned full-sync didChange regression for editing
an open document, including clearing and reintroducing the diagnostic.

Use the existing IsTimeUnit predicate only to choose the diagnostic range after
NormalizeClockPeriod fails. A known bad separate unit gets its own word range;
invalid numeric values and suffixed values with valid separate units retain the
value range. Validation, normalization and runtime arguments remain unchanged.
Reuse the existing real LSP client for one didOpen followed by versioned
didChange notifications. Check published versions, literal token spans and valid
corrections at both PDL levels. Keep formatter idempotence and dynamic operands.

Alternatives: parsing period components again duplicates validation; repeated
didOpen does not cover edits. Existing predicates and the current client suffice,
without dependencies or public signatures. Add no general LSP framework.

Prove the old span fails new literal-range assertions. Separately inject a
controlled stale edit through the test client: the old regression accepts it,
the updated regression must reject it, and the real server must pass. Preserve
exact driver, input and output evidence; this is test effectiveness, not a
claim that the real server ignores edits.

Rebuild the 48 qualified dependents. Run complete normal/static frontend suites,
focused generator period diagnostics, default leak-enabled edited-document
checks, three C++ quality checks and source build/install. The previous native
event suites are historical Task205 evidence; this range changes diagnostic
location and tests only, so no new full native/event/vendor/performance claim.
The additional unit predicate is linear in token bytes and adds no I/O.

Reuse standing completion-program, opted-out design questions and inline
authorization. One independent whole-branch review, at most one verified
Critical/Important repair pass, no rereview. Inherit 32 findings, candidate 30
after these two closures. Preserve 117 stashes, sibling ledgers, foreign worktrees
and excluded item1; merge, verify, push and retire the owned branch.
