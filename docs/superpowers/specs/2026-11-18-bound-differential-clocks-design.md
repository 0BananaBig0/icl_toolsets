# Bound differential clock consistency

Standing user delegation authorizes completion of documented generator gaps and
routine design choices without another approval question. Parent: master
5187a3c; branch: task/bound-differential-clocks.

Accept explicitly connected child ClockPort differential pairs for relative,
clock-only iClock/iRunLoop operations when both physical inputs provably have
opposite phase. IEEE1687-2014 section6.4.6.19, printed102–103, requires the
negative input to be linked to its positive partner; section6.4.6.20,
printed103–104, permits clock conversion at hierarchy boundaries. IEEE6.3.12(e), printed71, restricts clock_signal terminals: an explicitly
bound negative input in these tests uses inversion of a legal positive clock
source, never a declaration already carrying DifferentialInvOf. An unrelated
root or equal frequency alone is insufficient evidence of complementary inputs.
The current blanket bound-input rejection is the concrete gap.

Approaches: removing the guard loses consistency validation; checking only
literal source names misses existing aliases, inversion, mux and hierarchy.
Use the existing versioned route cache, recording a phase origin and inversion
parity alongside the duration root and ratio. A non-unit frequency conversion
starts a new phase origin: downstream branches from that same conversion can
be compared, but distinct converters with equal frequency cannot. This is a
conservative proof boundary, not an IEEE rejection rule or a timing model.

Resolve both the declared differential reference and explicit input binding
using an iterative dependency walk, checking complementarity before caching the
bound node. Retain ratio rounding, snapshot versions, active mux selection,
cycle detection, held clocks, hardware-state isolation and clock-only fanout
checks. Failed checks publish no checked target and no output artifacts.
No dependencies, absolute-time/events model, RTL changes or broad vendor
conformance claim. Existing direct unbound differential behavior remains.

Performance: preserve linear graph visits with ordered-container costs,
O((V+E)log V) time and O(V+E) storage per invalidated cache version. Reuse the
4096-stage/1024-wait and65536-bit fixtures and add a4096-pair shared-route case;
each existing subprocess retains its45-second limit. Avoid recursive depth or
repeated full-route scans per bound pair.

Acceptance: CLI original/flat/generated/reset replay, valid explicit inversions,
aliases, vectors, muxes, shared conversion and oscillator;
invalid same-phase, unrelated, distinct converters, selector drift, inactive
source and cyclic bindings. Native checked snapshot/failure/recovery, unchanged
storage and held-clock checks. Dynamic/static, leak SAN, changed C++ style and
strict C++17, full make check, fresh raw coverage with unchanged100/90 thresholds,
current remote compiler/runtime qualification, archive/install verification and
one independent whole-branch review. Merge into master and delete the task
branch when the scoped change passes; retain broader acceptance gaps.
