# Partial period operand diagnostics (Task205)

Task193's deferred M4 identifies a declaration-time diagnostic gap: an invalid
literal period value or unit is ignored when its neighbor is dynamic. Concrete
dispatch already rejects these inputs. Repair the shared frontend so formatter
and LSP users see the independently invalid literal at both PDL levels.

Reuse NormalizeClockPeriod in Document command validation. Validate whichever
operands are concrete, supplying a valid numeric value or unit only for an unknown
neighbor; those placeholders are validation inputs, never emitted or executed.
Keep full validation when both operands are concrete and defer fully dynamic
pairs to dispatch. A separate unit requires a bare positive numeric value, so a
known suffixed value with a dynamic separate unit also rejects.

Alternatives: separate numeric/unit parsing would duplicate the established
validator; waiting for dispatch preserves the reported editor gap. Reusing the
existing validator is the smallest coherent fix. Preserve procedure formals,
valid dynamic substitutions, exact periods, error recovery and output protection.
Change only the shared frontend and its existing formatter/LSP regression file,
plus task documentation. No dependency, public signature or clock event changes.

The work is linear in the period token length with one validator call per known
pair; small existing inputs need no new performance benchmark. Qualify both PDL
levels, both partially dynamic sides, valid controls, formatter idempotence and
LSP correction. Rebuild dependent products; run full frontend suites and focused
normal/static generator clock/reset callers plus default leak-enabled functional
checks, C++17/style/tidy and source package/install checks. Preserve exact input
and product manifests; an unrelated full repository regression is not a claim.

Reuse the completion program's standing opt-out of design questions and inline
execution authorization. One independent whole-branch review, one verified
Critical/Important repair pass if needed, no repaired-code rereview. Inherit 31
findings; resolving this one leaves a candidate 30. Preserve 117 stashes, foreign
ledgers and excluded item1; merge, verify, push and retire the owned branch.
