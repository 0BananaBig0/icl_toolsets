# Independent TAP instruction/data phases

Support a simultaneous access that writes one TAP's instruction register and a
different TAP's data path. Today CheckAccessGroups rejects every mixed IR/DR
group, and solver frames use one IR-select bit for all controllers. The baseline
two-TAP reproduction fails before any output is published.

## Approach and constraints

Removing only the guard is incorrect: concrete replay would still drive every
TMS through one common path. Serializing the two writes violates -together.
Use a phase choice per TAP, model the resulting controls, and replay distinct
TMS paths with a common capture/shift/update boundary. For mixed groups, the IR
path is1100 and the DR path0100 from Run-Test/Idle; the leading DR idle cycle
aligns capture. Both paths finish their final shift on TMS1, then update on the
same falling edge. Existing common-phase requests keep shared symbolic choices
and their existing schedules. Do not add an independently timed clock API.

Carry selected instruction TAP IDs in ScanStep alongside its existing scalar
instruction flag; this is execution metadata, not generated hardware. Reuse the
executor's existing register ownership rather than retrace the network. Enforce
equal phase choices for a shared TMS pin. A same-TAP IR/DR simultaneous request
remains impossible. Cross-interface controls, captures, reset and held values
must be checked with actual native events; never infer observations from goals.

IEEE1687-2014 7.9.14(c),printed173 requires -together writes on the same TCK cycle;
6.5.2 Alias guidance,printed136 requires an error when AccessTogether cannot be
met. The retained standard extraction /tmp/item11-1687.txt was read. TAP paths
follow the existing native controller and the 1149.1-2001(R2008) edge guidance:
actions depend on the pre-edge state, and Update-IR is a falling-edge action.
The local1149PDF is readable but no PDF text extractor is installed;the bundled
skill reference supplies that edge guidance, not a fresh PDF verification.

## Acceptance

Cover both IR/DR lane orientations, unequal widths, explicit reads/actual model
observations, subsequent instruction/data access, AccessTogether aliases,
preserved lanes, shared-TMS rejection and same-TAP rejection/publication safety.
Original, flat, generated and local-reset forms must agree. Native tests must
prove simultaneous update and distinct controller states at actual edges.
Measure representative2/4/8/16TAP and8/64/256bit inputs within existing work/solver
limits; no limit increase or global optimality claim. Phase variables grow
linearly with TAPs per frame; reuse one variable for unchanged common-phase
requests to avoid unnecessary solver search.

Run focused normal/static and sanitizer checks, the existing native suites,
full project checks, model/source audits, formatting and packaging. Retain
source/product identities and all failures/skips. Obtain one final branch review,
repair blockers once, merge/push and delete the owned branch;preserve117stashes.
This advances original item2. Independent clock timing, shared serial storage,
overlapping returns and broader item2 topologies remain separate open work.
