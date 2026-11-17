# Oscillator cached waits

Close the two concrete native gaps measured in Task122: Tessent2026.2 accepts
factor-only oscillator overrides followed by waits using an earlier iClock.
Its exports retain child4/child4 and child4/root12/root12. Native currently
rejects the override early. IEEE7.9.16 and the audit establish saved check
behavior; no intrinsic frequency or self-reference is needed.

Reuse existing versioned Definition/Checked state. In Override, a Period target
with a valid positive factor option may publish its omitted ICL source (missing)
without invalidating earlier checked versions. Resolve already rejects that
inactive current route at a fresh iClock. The model already handles missing or
literal sources; add no new state representation, route cache or header/API.
Keep optionless Period overrides rejected, and unconnected non-Period targets
requiring a source. Preserve positive/known factors, duplicate-option, unsafe
fanout, cycle and atomic-failure checks. An explicit subsequent source restores
a valid current route. Unchanged vector bits remain independent roots.

Add two CLI methods for multiplier/divider variants, before and after an
explicit source, retaining all five original/flat/generated/reset replay forms.
Update the existing rejection method: factor-only followed by fresh iClock now
fails at inactive routing; optionless and cycle errors remain. Extend native
oscillator snapshot checks for saved ratios, failed fresh checks, invalid-factor
rollback, explicit recovery, untouched vector bits and immutable hardware.
Add a --clock-snapshots selector to the existing native runner for focused SAN;
default full execution still runs these checks. No new maintained test script.

Work retains existing ordered-map route lookups and binary search within each
bit's history, with one version record per accepted override. No
new scan/model event support or performance claim. Run existing4096-stage/1024
wait and65536-bit clock cases; retain45s workload target and existing limits.

Only clock source, existing CLI/native tests, two global docs and plan/spec change.
Real CLI/native RED before product edit, then focused GREEN/strict C++17/style,
dynamic/static clock suites and leak-enabled native/clock/main checks. Freeze
exact3994 runtime inputs (delta only these3 files vs Task122), complete normal
integration and fresh-profile raw coverage serially, qualifying updated remote
dynamic/transferred-static generators/native groups with exact hashes. Compare
the two new exports with recorded vendor traces and rerun our normalized forms
through the unchanged vendor driver. Reuse unrelated evidence only explicitly;
new binaries must not be called byte-identical. Keep100%/90% strict gate intact.

One independent final review and necessary correction pass, then exact commit,
direct-parent master merge/delete branch/archive/sync. No push. Parent e2d6bdc.
Standing user opt-out of routine questions/execution delegation applies; this
is the smallest existing-state solution to measured supported input patterns.
