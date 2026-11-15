# Hierarchical root scans implementation plan

Execute inline in the isolated worktree while Root Task95 stays frozen. Spec:
docs/superpowers/specs/2026-11-15-hierarchical-root-scans-design.md.

Goal: execute a full root raw scan through nested static modeled instances.
Files: pattern_scheduler.cc RawChain typed port/terminal qualification; maintain
native scheduler and modeled CLI tests; hierarchical_root_scan fixture; guide/model
coverage. No new state/event model, public API or dependency.

Interface: NormalizeScan/ApplyScan unchanged, root interface remains the target.
Profile qualifies root vs nested interface ownership, direct physical root pins,
forwarded data links and invariant control/reset endpoints. Existing native bulk
replay and field names remain authoritative.

Review focus: undriven nested control fallback must not qualify; clocks must trace
the root clock rather than merely share frequency; no internal reset dependency;
forwarded alias polarity and root SI index; full-path coverage and unchanged source
rewriting/atomicity. Use cached typed forwarding and existing structural budgets.

- [x] Independent design/builder review before production edits.
- [x] RED nested original/flat/generated/localreset, repeated instances/procedures
  and native state/fill; atomic invalid control/clock/reset cases.
- [x] Minimal typed qualification and serial port forwarding, no native phase change.
- [x] GREEN focused/connected checks and obsolete-scope expectation updates only
  where newly qualified.
- [x] Source-matched vendor and representative wide/many/deep measurements.
- [x] Independent final review and immutable required leak/fullserial gates.
- [x] Commit feature, fast-forward master, delete merged task branch and preserve
  only subsequent unfinished work.

RemoteRED actual6hashesmatch build0/native1/5CLI1. Positivecases hit the existing root-scope guard. Four inversion-negative diagnostics are already enforced by the frontend typed-control rule; align assertions, preserve rejection. Implement typed scalar forwarding and cached terminal/parity qualification only; native phases unchanged. No96production before the maintained intended RED.

GREEN2: actual7 hashes match; build/native state/native scheduler/modeled19/opaque12 all0. Corrected invalid fixtures (parameter reset width, forwarded output reference scope) and preserved real inferred parent reset semantics. Independent final source review found no defect; mixed incoming reset-parity cache coverage is a nonblocking test opportunity.

Scale evidence: fifteen commands pass on the source-matched remote candidate.
The1048576-bit leaf takes .26-.33s with peak104796KiB;1024 repeated8-bit leaves
take .19-.32s with peak99296KiB;97 levels take .05-.11s with peak64580KiB.
Source repetitions, flat input and generated-PDL replay have identical canonical
output hashes within each scenario. These qualify newly supported shapes; no
old unsupported-path speedup is claimed. Vendoractual6submitted/10preparedhashes
match, original/normalized exits0 and byte-identical exports. Local15 connected
checks, formatting/naming and53offline directional goldens pass.

Verified Task96: required leak-enabled build/native7/main69/modeled19/opaque12/clock51/client25/reset12 all0. Full serial make check passes in 1364.230889235041s. Final remote52 statuses all0 and actual37 source/test/fixture hashes match. Local15 connected checks, formatting/naming and53 offline directional goldens pass. Independent final review found no concrete defect. Native literal replay checks original/flat nested register and pin state plus partial-X controlling fill. Fifteen source-matched scale commands pass:1M leaf .26-.33s/~104796KiB,1024 leaves .19-.32s/~99296KiB,97 levels .05-.11s/~64580KiB; canonical replay hashes match per scenario. Vendor original/normalized exits0, byte-identical exports and actual6 submitted/10 prepared hashes match. Existing Tessent stable/default-chain differences prevent a strict certificate claim. Commit11 files, fast-forward master, delete completed task branch and preserve only the next child-scan draft. General child/opaque/mux/TAP/parallel/resource/event/reset/clock/lifecycle, strict coverage and final vendor/platform requirements remain open. No push.
