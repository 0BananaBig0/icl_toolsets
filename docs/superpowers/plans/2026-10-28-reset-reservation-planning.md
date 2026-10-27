# Reservation-aware local-reset planning

Continue the approved pattern-generation reset/resource requirements inline.
The previous round-trip change is committed separately as 9ba89af.

## Reproduced problem and scope

With reset = ~(a | b), both release controls start at zero. If b is taken and
written zero, normal flat generation releases through a. The local-reset planner
ignores reservations while choosing a release, selects b, then rejects the plan
later. A taken-but-unwritten b can likewise be chosen and accidentally acquire
protection if emitted as a source write. A valid alternative must be considered
before reporting failure.

Reuse ResourceReservations, ResetPrerequisites, existing source tracing and
concrete replay. Pass held values and taken-but-unwritten IDs into reset solving.
Constrain held values and keep unwritten takes at their current hardware state;
do not grant automatic releases the permissions of explicit user writes.
Preserve diagnostics, pending-group boundaries, source iProcs and atomic output.
No new parser, runtime callback, dependency, or scheduling search is introduced.
Existing irreparable procedure-body/source-insertion cases remain explicit errors.

## Tasks

- [x] Add self-constructed alternate-reset fixture and failing tests for held,
  unwritten, released and impossible reservations, with source/flat/reset replay.
- [x] Add the reservation inputs to the shared reset planner and its single CLI
  caller. Keep existing defaults for native tests; preserve source attribution.
- [x] Run focused dynamic/static reset/resource suites, independent review,
  complete make check, and ASan/UBSan with LeakSanitizer enabled.
- [x] Update reset documentation with actual results and limits; commit and push.

## Performance

Keep the existing 4096-control-bit and two-second query bounds. Reservation
lookups add logarithmic membership checks to the existing control traversal;
no per-candidate schedule enumeration. Test a 40-control OR-release network
with all but one control unavailable and record end-to-end time/memory.

## Review and focused checks

Four alternate-release cases failed before the change (held, assumed,
taken-but-unwritten and persistent). All pass after fixing the shared planner.
Dynamic/static resource suites now have 21 passing tests; generator CLI and native
scheduler suites pass. Independent review found no blocking issue.

The separate naming/style check exposed existing missing braces and accessor
names outside its allowlist. Mechanical braces and Values/SourceActions renames
at all callers fixed it; no checks were suppressed. Final formatcheck and
namingcheck passed. The 40-control/39-reservation case has a 2663-byte design and
took 0.105 seconds in the CLI (0.14 seconds end-to-end, 76356 KiB peak RSS under
/usr/bin/time, local Linux GCC optimized build, Z3 4.13.3). This is a single-run
scale check, not a speedup or general optimality claim.


Full `make check BISON=/usr/bin/bison -j4` passed with the existing reference
venv and ICL_REF_DIR; see `build/reset-reservations-full-regression.log`.
All 334 grammar alternatives and 119 scanner rules were reached; all seven
legacy and six directional vendor standards passed offline. Vim/Neovim each
checked 38,646 ICL and 815 PDL files. ASan/UBSan with leak detection enabled
passed all 21 resource cases, 26 generator CLI cases and native scheduler checks.
No new Tessent execution was needed for this planner correction; broader
source-rewrite/model limitations remain documented in the generator guide.
