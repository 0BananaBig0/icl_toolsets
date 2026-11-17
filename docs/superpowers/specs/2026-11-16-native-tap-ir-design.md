# Native ordinary TAP instruction replay

Task115 follows completed Task114 merge7dc8dc1 and branch deletion. User
delegates execution/routine choices without confirmation. These are executor
choices; perform builder check, precise plan and TDD before product edits.

Fresh isolated IR-only baseline confirms the cost: one4-bit IR write on64x4
data registers uses252492/252864KiB and .35s;512 forwarding aliases use
261676/261728KiB and .64/.63s. A1024-unrelated control uses66688/66700KiB,
.25-.29s. Exact commands/inputs/platform results are under
build/native-tap-ir-preflight. Target below128MiB peak RSS for the64x4 and
512alias IR-only profiles, with byte-identical matched-option outputs.

Candidate approach: extend existing TryNativeTapScan to pure instruction goals
using an existing ConcretePath derived by TraceSerialPath in IR mode. Rename
RawDataPath to common private RawTapPath(bool instruction=false), preserving
current data tracing/bound-input/active checks when false. IR mode
requires the complete single instruction register and actual bound TDI. Reuse
NativeTapShape, shared ReplayTapStep/Cycle, current expected/capture observation
handling and per-edge reservation checks. No new FSM/walker/solver or raised
guard. Mixed IR/DR, unsupported resets/control histories and unknown ordinary
fill retain existing joint-planner fallback.

Ordinary X writes continue normal controlling-fill semantics; only known full
loads qualify. This is distinct from literal raw X, whose disposable ordinary
proof might benefit from a faster binary candidate but whose literal replay
must remain exact. Authored SO mismatch must retain old symbolic fallback. Pure
required IR goals should not gain new implicit comparisons; an implicit IR
setup that currently emits BSDL capture must preserve that exact output.
Do not accidentally select IR native path when primary/read/address goals or
access groups remain outside its required set.

Preflight must settle actual instruction update evidence. Existing data native
path requires known ControlEvents and actual writes; inspect whether IR
Synchronize/SetRegister provides equivalent update history before reusing it.
Final IR value alone cannot prove a CSU when the previous instruction already
equals the requested code. Verify actual TAP phase/clock/pin qualification,
especially root TMS/TDI/TRST forwarding or forced sources. Prefer existing
controller/trace evidence, or conservatively fall back for unsupported pins;
do not invent a second TAP transition engine. Preserve unknown IR hardware and
known controlling fill recovery from Task114. Source-family restrictions already
reject scan signals used as data selectors, while legal selectors read held
update storage; do not add speculative per-bit path rescans based on the withdrawn
Task114 review finding.

Acceptance: true measured performance RED before product edits plus exact
ordinary/raw/flat/generated output checks. Literal IR N+6 clock sequence,
capture-mask/ascending/parity where supported, reads/writes/partial fill,
already-equal instruction, unknown instruction recovery, protected transition,
unsupported clock/reset/root-pin fallback and rollback cases. Retain all raw
IR/DR/builtin/multiple-register/literal-X data and automatic-child tests. Target
only measured representative latency/RSS bounds that have a completing old
baseline; do not claim1M IR or whole128packet15s behavior. Fresh independent
review, full/SAN/remote and genuine vendor checks before merge/delete; no push.

## Measured acceptance and chosen approach

Fresh local isolated IR-only4bit baseline two repeats:64x4 data registers
252492/252864KiB,.35s;512 forwarding aliases261676/261728KiB,.64/.63s;
1024 unrelated registers66688/66700KiB,.25-.29s (exact JSON in build/native-tap-ir-preflight).
The first two fail the selected128MiB peak-RSS target; the unrelated case is a
completing control. Require byte-identical same-option output before/after and
measure both supported Linux environments; no speedup or universal bound before
actual measurements. Preserve all existing event/bit/time bounds.

Deleting event constraints or limiting general planner search would weaken
correctness. A separate IR planner/FSM duplicates proven code. Chosen approach:
extend existing TryNativeTapScan for pure known full IR goals/reads, using shared
tracing and ReplayTapStep. Mixed/noncanonical cases keep existing fallback.

## Exact private boundaries

Rename RawDataPath to RawTapPath(bool instruction=false). Default data mode keeps
existing tracing/bound-TDI/active-data checks. Instruction mode drives the copied
IR-select phase and requires exactly the instruction register at full width and
bound TDI, reusing TraceSerialPath. Existing data callers use the default mode.

NativeTapControls(const ConcretePath&, bool selected=true) keeps existing data
callers unchanged. For selected=false, check phase/clock forwarding across all
existing execution RegisterInterfaces, without requiring data SelectPorts active: DR phase
controls are inactive during an IR transaction. Noncanonical control/clock paths
fall back. New IR goals must be exactly the instruction register; no hidden
primary/address/data obligations are skipped.

SetRegister (state374-384) does not emit ControlEvents for IR Synchronize. Do not
require an invented IR event. Known ready-idle contract plus shared ReplayTapStep
uses the actual controller; Drive rejects connected/non-input root pins, and
Edge rejects connected/nonscalar root TCK. Validate actual final IR load and
per-edge preservation, reject reset/unplanned known update events as before, and
adopt complete execution state only after proof. Existing data history checks
remain unchanged. Unknown load/fill, unsupported data/clock/reset requirements
and authored comparison mismatches fall back. Source-kind validation keeps
serial signals out of data selectors; no speculative per-bit route rescans.

## Bounded implementation and verification

Only pure instruction required/goals/expected maps qualify; outer Solve still
validates all groups, end states, types and synthetic privileges before dispatch.
Known full load is mandatory. Reuse current expected mapping and observer
pre-rising sample; capture mismatch returns nullopt to the existing symbolic
planner, preserving authored comparison behavior. Set step.instruction=true,
keep existing data branch unchanged, and skip only the unavailable IR known
ControlEvents requirement. Require actual final known IR load and shared
ReplayTapStep update eligibility, with no reset/unplanned known update events.

Use performance RED measured against the saved Task114 binary before edits.
Maintain functional regression tests under tests/src/pattern and use the
existing benchmark collector for equivalent ordinary programs on both binaries.
Do not impose optimized RSS limits on sanitizer binaries. Record measurements
for4/32/256 IR bits,64x4 data,512aliases,1024unrelated and128 IR groups, with
unchanged guards and complete source/flat/generated artifacts. Existing known
data and literal-X benchmarks are also rechecked; no unmeasured speedups.
One independent final review, full/SAN/remote and genuine binary IR vendor
recertification precede merge/delete. No new source fixtures or certificates
when existing byte-identical accepted evidence suffices; never rewrite failed
X vendor evidence. User-approved task completion is not full acceptance of
remaining topology, clock/reset, strict coverage or release requirements.
