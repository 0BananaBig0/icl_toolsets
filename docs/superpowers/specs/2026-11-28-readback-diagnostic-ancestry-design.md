# Recorded operation and live getter ancestry (Task215)

Resolve Task214's active-getter readability finding. Keep both original operation
ancestry and live Tcl getter ancestry, including independent source locations
and repeated recursive frames. Render recorded frames in a labelled, indented
block: `recorded operation calls:` followed by four-space `iCall` lines. Leave
the runtime's existing two-space `while iCall` unwinding lines unchanged. The
dedent distinguishes active getter context from the recorded block. Outside
finished calls, only the recorded block appears. Empty stored stacks keep their
exact existing message; sticky read/status queries rethrow identical context.

Choose explicit roles over deduplicating string matches, which can erase valid
recursive calls, or introducing a new public exception framework. Only change
recorded frame formatting inside the existing Process catch. Preserve origin,
cause, callback-preservation prefix, exception caching/type, throw conditions,
all successful execution, validation and Runtime logic. Error work remains
linear in total frame text; successful execution gains no traversal/allocation.

TDD first: native complete-message oracles for empty, distinct nested and
identical recursive frame vectors, each over read/clear/status repeats. Update
the completed-call CLI oracle and add real active getters across three getter
types, direct/merged calls and depths0/2. Pin the two role sections, full frame
multiplicities and ordering, failing/getter locations, exit and preserved output/
debug artifacts. Old source must fail on missing role formatting, not setup.

Verification scope is this diagnostic rendering change. Prove the complete
provider source is byte-identical to Task214 after replacing exactly the old/new
recorded-frame formatting fragment with one marker. Preserve4132 other inputs;
rehash all4135 inputs and rebuilt48 products. Retain Task214's full228-report/
3572-method regression as evidence for unchanged scheduling and successful-path
logic; do not claim a new full-project run or unchanged product hashes. Freshly
run the whole native readback/runtime/opaque suites on normal and default-leak
sanitizer binaries, plus all nine affected CLI modules on normal/static/SAN
(27 reports), six C++ quality checks, source build/install and six exact package
members. This covers the changed diagnostics and their callers; Task214 broad-suite
results remain historical evidence for the unchanged execution logic.
Use detect_leaks1/UBSANhalt1 and existing limits; run defaults before heavier
work and keep SAN execution isolated. No new API, dependency or Makefile changes.

Standing question opt-out and inline execution apply. Parentmaster
b002ea8ae51d3310d46c2ccdaa9e77e3d12ba09f; branchcodex/readback-diagnostic-ancestry.
One fresh whole-branch review, one blocking RED-to-GREEN fix wave, no rereview.
Inherited23 minus1 candidate =22 pending review. Preserve117 stashes/unrelated
state; merge, verify final artifacts/tests, push, archive only owned private
files/ledger, retire branch and retain reusable worktree. Broader original
inventory and excluded strict coverage item1 remain open.
