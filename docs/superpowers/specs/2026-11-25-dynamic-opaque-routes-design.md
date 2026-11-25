# Fixed opaque leaves on dynamic scalar routes

Task179 advances original items6/8 after Task178(master4f1977d). A valid SIB
network already executes with a modeled leaf, but replacing the leaf with a
fixed eight-bit opaque client fails before the first one-bit root opening scan.
Reproduction: /tmp/icl-next-dynamic-opaque-xshukt2n. Actual opaque returns must
stay unknown; expected SO and written SI never become observed data.

## Approaches

Extend the existing private opaque-storage expansion and native scalar route
engine. This reuses real capture/shift/update state and existing opaque queues.
A separate opaque state engine would duplicate these contracts. Removing the
static guards alone would complete inactive queues and lose event/route truth.
The implementer selects the shared-engine approach under the user's continuing
delegation; the user did not request another approval discussion.

## Scope and design

Support fixed-width opaque leaf clients behind binary scalar bypass muxes
controlled by modeled held ScanRegisters or existing pure forwarding aliases.
Open and close routes explicitly with root scans; child requests use an already
accessible route. Do not claim automatic opaque-child preparation, arbitrary
black-box behavior, independent clocks, TAPs, parallel lanes or timed execution.

ExpandOpaqueChild keeps its signature. Extend its structural route validation
from a single walk to an iterative cycle-checked graph walk over both mux arms.
Every arm must terminate at the root input, every opaque candidate and modeled
scan register must belong to the structural network, and aggregate storage
stays within1048576bits. Charge each distinct register/opaque leaf once; avoid
recursive depth or repeated complete scans. Native RawTemplate/RawRoute still
validate actual scalar controls and the selected pre-CSU route.

For a conditionally selected opaque leaf, reaching its output must require its
held selection to be active. Preserve canonical selector identity and inversion
across muxes; contradictory assignments cannot establish a bypass. A batched
necessary-condition proof processes64held selector bits together with memory
linear in the graph. Failure to establish this condition rejects the expansion;
it does not enable an uncertain route. Larger unmeasured networks retain the
original item10 performance work.

Extend private-output consumer validation through mux data arms. Opaque output
must remain serial payload: using it as a selector, reset, capture value or
user-visible modeled output remains rejected. Preserve current aliases,
inversion, default-load, leaf-scope and pin validation.

OpaqueRawState uses the actual pre-CSU profile to select active private storage
intervals. Project and complete only their queues; preserve inactive queue
knowledge and histories. Commit queues only after successful execution, using
the existing trial/preparation/copy boundaries. Existing OpaqueRawHistory and
metadata retain unknown read bits, cared-X/uncared-zero masks and once-only
status accounting. Root opening scans need no opaque completion; closure scans
use the old active geometry. Existing static behavior remains qualified.

## Verification and performance

First add immutable-parent failures for root opening, active child history,
closure/reopening and two independently selected opaque leaves. Pin literal
root words, known parent state, unknown child data/masks/status and no extra
CSU. Compare original, flat and generated continuations, default regeneration,
local reset, cancellation, copies/rollback and invalid private-output uses.

Measure1/2/8/32leaves at8/64/256bits and1/2/8leaves at1024bits with sanitizer
samples and three repetitions. Use a fully modeled twin with unknown capture
data as the parent control; the parent does not support the new opaque route.
Compare actual observations and physical operations, normalizing only the
separately validated child-intent family. Do not inject observed data or call
this a byte-identical unsupported-parent source comparison. Preserve the five
existing matrices and10s/512MiB bounds, with single-wide controls256MiB.

Use one inline implementer, test-first changes, complete normal/static/native/
SAN/corpus/quality/package checks, one fresh final reviewer and one blocker-fix
wave without rereview. Merge/push and remove only the completed task branch.
Preserve117stashes and31deferred Minors; coverage1 remains excluded.
