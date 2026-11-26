# Independent clock programs

Task197 advances remaining items5/12. Item1 remains excluded. The user delegates
continuous implementation, repository/owned temporary work, commits, parent
merges, pushes and completed branch deletion. The following choices are the
executor's; the existing C++17 implementation and dependencies stay in place.

## Confirmed problem

On master8449b13, a clock-only ToClockPort driven by a ClockMux accepts a native
divider-only override and lowers six cycles to ten root cycles. Add an explicit
period override on an independent oscillator and the same valid operation fails
the retained-source guard. The generator currently retains every clock statement
if any period override occurs. Evidence is in
build/clock-program-components-20261127/discovery.json, with the actual parent
binary identity and both complete inputs and results.

## Required behavior and compatibility

In a period-bearing program, lower a previously unsupported partial override
through its existing ordinary path when its clock component has no connection
to any explicit period override. The candidates are ToClockPorts with a valid
but nonrepresentable immediate ICL source (mux, alias or inversion), no explicit
replacement source and no intrinsic Period. All operations on that independent
component use the established root-relative checks/waits and omit its source/
ratio override statements. Other components retain their existing logical PDL.

Preserve byte-identical output for previously accepted programs, including
complete option order/numeric spelling and representable partial forms on
independent routes. A program with no candidate requiring extra support keeps
the existing retained rendering. Do not lower a candidate connected to an
explicit period override, even by a future source override; keep the original
normalization guard. Keep per-bit vector independence, hierarchy/flat name
mapping, checked definition histories, rollback and output publication safety.
Do not change parser grammar, period spelling, oscillator restoration, factor-
only/source-less validation, hardware fanout, event/time or physical-PDL guards.
Compiled help must describe the independent root-relative lowering exception.

IEEE1687-2014 sections7.9.15–17, printed174–176/PDF188–190, describe checked
clock paths, ICL-default override values and root-relative ratio scaling. This
change selects an existing supported representation for independent paths;
it does not add a clock-event or elapsed-time engine. Exact -period output uses
the existing vendor extension. Vendor references must explicitly implement the
native default factors and selected source; raw omission semantics remain
release-specific and unqualified by an explicit reference pair.

## Chosen design

Reuse SystemClocks' existing per-bit fanout graph and target/name mapping.
Prepare rendering policy once for the full operation stream before executing
its clock overrides. Build a temporary undirected graph from the existing
routing edges plus all valid explicit future source-override edges. Do not add
future edges to live resource/fanout state. Compute the closure of period targets,
then the closure of candidate targets outside it. Only the latter components
are marked for ordinary lowering; the default remains retained rendering.

Use two shared APIs, RetainClockPrograms(const vector<Operation>&) and
RetainsClockProgram(const string&) const, plus one set of lowered bit identities.
The generator keeps its existing period-presence flag, prepares the policy on
the initial clock object, and copies that policy into reset previews with the
existing clock state. Override/check/wait rendering consults the per-target
choice. Real operations keep their own argument/name validation; analysis must
not change diagnostic ordering for invalid programs.

The graph is sparse and per bit. It must not expand isolated wide oscillator
declarations into all their bits, or repeatedly scan the network per operation.
Two multi-source traversals and existing ordered maps/sets require O((V+E+P)
log V) work and O(V+E) temporary space for referenced routing bits and operations.
Programs without a period or candidate keep the fast path. A new time engine
or extra generated clock ports would change contracts without being needed.

## Verification and scale

Maintain tests in existing root tests files. New parent-positive/native-valid
cases must fail at the retained-source guard before implementation. Cover mux,
alias and inversion, source replacement and default restoration, checked old/
new ratios, vector bits, procedures/reset/flat replay, future source bridges,
period-connected guard retention, invalid argument ordering and artifact
publication. Native assertions must test per-bit policy, exact root/cycle
results, copies and unmodified definitions/resources independently of emitted
string comparisons. Keep existing complete-argument and representable-partial
output regressions unchanged.

Measure three rounds of2/8/32 ordinary clock bits by8/128/1024 partial updates
with an independent retained oscillator. Capture actual two input/five output
identities and all matrix artifacts before cleanup. Compare matched ordinary
parent/current controls. Every point must meet unchanged10s/512MiB bounds. Require
all27 matrix and18 control coordinates exactly once, rehash each actual control
file, and compare all nine parent/current pairs. No isolated speedup claim.
Run both generators, full project, affected leak-enabled sanitizers, changed-C++
quality, corpus, editor and source packaging against frozen identities. Use
bounded configured vendor reference/generated pairs and literal relative traces.

## Integration

Parent master8449b1370d9e1353751ff4adc051523587f2d901; branch
codex/clock-program-components; reuse .worktrees/bsdl-defaults and the just-verified
Task196 baseline. Preserve117 stashes, unrelated worktrees and ignored sibling
ledgers. Inherit41 deferred Minors; do not reopen them in a review repair pass.
Use one independent whole-branch review, one Critical/Important test-first repair
pass if necessary, no re-review. Verify the merged result before push and safe
owned branch deletion. Archive only this task's own workspace. Broader pending
items stay open until their actual acceptance criteria pass.
