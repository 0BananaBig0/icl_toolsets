# Physical clock target names in native flat ICL
Parent master97b3ecc; task/flat-clock-targets. Standing user delegation covers
routine implementation choices and scoped commit/merge/delete/push.

Task130 measured four successful flat vendor exports but eight native replay
failures: vendor uses physical encoded clock names while metadata restores
original identities. Correct the flat-input half of this gap for iClock and
iRunLoop -sck only. Original source files without flat metadata keep their current
lookup contract; their raw-name replays remain unqualified.

Resolve existing original names first. Only an otherwise unknown, canonical
lowercase n+hex physical declaration name may decode to an existing original
ClockPort/ToClockPort when valid flat-origin metadata supplied the netlist.
Preserve any existing bracket/parenthesis suffix and reuse ResolvePdlTarget for
all ranges, scalar/single-bit, type and alias restrictions. A decoded data or
alias declaration does not become a clock. Literal original-name collisions
retain original behavior; this does not provide unambiguous raw mode for such
collisions. Physical names remain global; iPrefix/call qualification is unchanged.
Do not broaden ordinary data/resource lookup, source-scope rules, iClockOverride
target/source spelling, or introduce an option/public API/metadata version.

Use a private spelling helper only on Check/Wait target paths. Existing Target
continues to serve overrides unchanged. Strictly validate and decode the known
hex spelling using the standard library; avoid a per-target graph scan or a new
full index. Cache metadata presence once per immutable-netlist SystemClocks
object (copyable with previews), retaining original names for emitted PDL.
No clock event, phase, snapshot or frequency policy change.

Tests must fail before implementation: scalar/vector root and hierarchical
output physical targets, mixed original/physical Check/Wait, old snapshots and
original-spelled overrides, malformed/nonclock/no-metadata/ordinary-data/physical
override rejection, collision priority, failure artifact preservation. Keep
native source/flat coverage and raw-command original-source rejection. Generated
canonical PDL replays on original/flat models; raw/reset input spellings replay
only with the associated flat metadata. Add a4096pair/stage1024wait five-form
raw-name workload, including late metadata to establish one AST scan rather than
one scan per command. Existing typed/legacy encodings remain supported.

Performance: O(name length + log nodes) per physical lookup, one O(root AST)
metadata-presence check on first applicable use, O(1) additional cached state.
No new dependency or index. Reuse existing4096-stage/65536-bit and5000-instance
fixtures; record completing baselines, input/product hashes, wall/RSS and limits,
without unmeasured speedup claims.

Qualify changed C++ format/tidy/strict17; focused dynamic/static/native/leak SAN,
full normal then fresh raw unchanged100/90 coverage; current Rocky suites and
archive/install products. Repeat raw native replay of the four unchanged vendor
exports on flat and original input separately, checking exact vendor inputs and
complete ordered trace. Reuse vendor run results only by exact physical model/
program/native-output identity, without formal certificate or freshness claims.
One independent final review, one necessary fix pass, merge/delete/archive/sync/
push. General output-source flattening, collision-wide/raw data naming, vendor
negative-clock/event/absolute-time support, strict coverage and release gates
remain open.
