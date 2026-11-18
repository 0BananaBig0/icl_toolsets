# Typed flat clock forwarding
Parent master6d7ed76; branch task/flat-differential-clocks. Standing delegation
authorizes routine design choices and incremental commit/merge/delete/push.

Task129 preserved three vendor flat failures at read_icl ICL59-1. Declaration
currently changes an internal positive ClockPort into DataOutPort while leaving
its negative ClockPort linked to that data declaration. The generated association
has the wrong declared type independently of native origin restoration.

Represent internal ClockPort nodes as ToClockPort outputs. A positive forwarding
port keeps its Source; a negative port keeps DifferentialInvOf only. Root clock
ports and every other port family keep current serialization. Original kind,
scope, physical input bindings, inversion, vectors, holes and inactive inputs
remain in the existing version1 metadata. Restore both new and old encodings;
old files canonicalize to the new encoding without rejecting their input.
Before accepting a ToClockPort encoding of an original ClockPort, reject
frequency/Period properties that cannot belong to that original input kind.
Keep actual ToClockPort factor/oscillator declarations and all old validations.

A small private encoding helper shared by serializer/loader prevents drift.
Globally changing source eligibility or introducing synthetic external clocks
would alter semantics; an entire clock-network flattening engine is outside this
port-type correction. No public API, metadata version, new dependency or RTL.

IEEE1687-2014 6.4.6.19/20 defines input/output clock kinds, output Source and
differential association, including exclusion of Source/factors/Period on a
differential output. Parent evidence plus three manual typed prototypes is
retained under build/flat-differential-clocks-20261118/prototype: direct/alias
models elaborate/execute/export0 with encoded root names; the divider passes
read_icl but fails set_current_design ICL130-2 for a local output Source.
This proves a separate general flattening limitation. Do not widen source-scope
rules to conceal it, claim general vendor portability, or replace the runtime's
relative-duration contract. Future measured probes must use flat port names and
distinguish physical-name mapping from raw native export replay.

Tests: new raw declaration-kind/original metadata/vector/nested/unconnected and
legacy migration assertions in existing flatten native runner; new legacy CLI
replay method; new metadata factor/Period rejection assertions with a real RED
after new-kind admission. Existing full source/flat/runtime tests remain.
O(1) additional work per existing node, no graph scans/state; reuse4096-stage/
pair,65536-bit,1024-wait/five-form and5000-instance checks, recording wall/RSS
and exact inputs/products. No speedup claim without completing matched baseline.

Qualification: changed C++ format/tidy/strict17, focused dynamic/static/native,
leak SAN; frozen current inputs, full normal then raw unchanged100/90 coverage;
current Rocky native/CLI variants, exact source archive and installations; fresh
vendor flat source/normalized probes with current clock names. Other directions
may be reused only by exact input/native-output identity. One final independent
review, one necessary fix pass, merge/delete/archive/sync/push. Existing coverage,
general output-source flattening, vendor-negative-target/event and full release
acceptance remain open. Correct the prior file-count wording minor while updating
the current archive qualification description.
