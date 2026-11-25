# Opaque child histories across generated prefixes

Task174 advances inventory6/8. Source execution of explicit fixed opaque child
scans records their local interface histories. Lowering emits one root scan,
so the same appended local getter fails after generated-prefix replay, for
both one and two children. The parent reproducer is retained under
build/opaque-prefix-histories-20261125/initial-reproduction.json.

Preserve explicit request identity with standard quoted iNote -comment text
after the completed root CSU. The comment identifies root client, child client
and that child's canonical compare word. It carries no actual return data.
Only explicitly requested opaque clients get a history annotation; an ordinary
root scan does not create local histories. Keep existing stable-knowledge notes.

The native opaque scheduler caches each committed interval's compare word.
It validates replay metadata against that snapshot and the actual root/client
binding before returning an unknown-data/miscompare delta. A cared opaque bit
has unknown miscompare; a don't-care bit has zero miscompare. Restoration adds
no hardware action and never recounts global failures. Reset invalidates the
snapshot; copy/rollback/subscriber semantics remain native scheduler contracts.

Runtime records only pending metadata intent after a preceding applied data
scan. Existing readback delta validation admits the matching new child key;
the actual provider supplies the data. Invalid/unmatched comments cannot
overwrite old history. No provider means no fabricated actual data. Preserve
history through later roots, resets, split files and prefix continuation.
Direct merge metadata uses the existing notes-first Runtime mechanism.

Approaches: replaying another child iScan duplicates a CSU; inferring every
child from any root scan invents request histories; iState does not describe
interface histories. Validated comments reuse the existing annotation and
observation paths. This is the executor's choice under delegated authority.

Use existing C++17/Tcl/Python components, no dependencies or RTL. Share note
parsing between Runtime and scheduler, retaining the1Mbit bound and declared
opaque width. Cache compare data once per CSU, O(total opaque bits), and keep
literal parsing linear. Avoid per-child full-root copies or network expansion.

Acceptance: actual unknown data, exact sparse compare/status behavior, explicit
scope, idempotence, malformed/stale/unanchored notes preserving histories,
native copies/reset/rollback, source/flat/generated-prefix/local-reset replay,
both tags and unobserved/provider boundaries. Full generator/native/SAN,
quality, corpus and packaging checks pass on exact source/product hashes.
Requalify existing multiple-child and one-child matrices under unchanged
bounds. Parent controls may carry matching informational comments solely to
retain exact whole-output comparisons; no old-parent prefix support or
performance improvement is claimed. Broader topology, timing, callback and
transport work stays in the original inventory; coverage1 remains excluded.

Physical-root anchor clarification: a logical child request also executes the complete native root CSU. Runtime records pending intent after an applied data scan; the native snapshot proves the exact root/client/comparison. Ordinary frames invalidate that snapshot. Already pending observations survive a later reset, while new unanchored notes after reset are ignored. A supplied valid identity comment declares its requested interval; root scans without such comments still create no child histories.

Existing follow-up outside this increment: a child-only source does not expose root-interface history, while its lowered physical-root prefix does. The immutable parent and current candidate both show this difference. This increment restores opaque-child histories; logical root-scope parity remains in original inventory6 and is the next separate task. Evidence: root-scope-followup.json in this task evidence directory.
