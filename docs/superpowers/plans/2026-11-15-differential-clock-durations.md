# Differential clock-only duration tracing

Execute inline in the isolated worktree while RootTask90 remains frozen for
sanitizer/full checks. No production edits before focused RED and design review.

Goal: follow declared differential clock partners for iClock and relative -sck
waits through the existing clock-only routing model. IEEE1687 6.4.6.19/.20 links
the negative half to the positive half; the two halves have the same cycle count.
ToClockPort DifferentialInvOf excludes Source/ratios/Period.7.9.15 and7.9.17
require tracing valid roots and minimum cycle counts. This adds no waveform,
absolute-time or hardware-event interpretation.

Smallest design: reuse the existing DifferentialInvOf input references as routing
edges in Source/Resolve/BuildSafety and override cycle detection. No new graph,
cache, ratio arithmetic, public option or serialized property. Root negative
ClockPort and negative ToClockPort can route to their declared positive partner;
direct hierarchical PDL targets can name child negative ports. Generic Source
forwarding from a negative terminal remains rejected by the existing6.3.12(e)
source-kind rule; this task does not relax it. Existing ratio,
selector/override versions, reservation/current fanout and output replay remain.
Explicit inverted expression edges retain their existing rejection in this
increment. Negative input ClockPorts with both an external Source binding and a
local partner require a consistency decision; reject direct tracing of that
bound form for now rather than choose one independently. They must not make an
otherwise clock-only root unsafe solely by carrying an association.

Reach: src/pattern/pattern_clock.cc Source/Resolve/BuildSafety/CheckCycle;
existing tests/src/pattern/test_system_clocks.py and native scheduler clock tests;
relevant guide/model/plan. Override of a DifferentialInvOf target retains its
existing diagnostic. Ordinary ToClockPort override sources may use a supported
negative route; newly introduced cycles through differential edges must reject
before publishing a version, preserving old checked waits.

- [x] Independent design review, including bound input association, clock-only
  fanout, hidden non-clock consumers and current held-clock checks.
- [x] RED root pair, negative output conversion, hierarchical child negative targets,
  vector/alias/slice/index/prefix, ratios/mux/oscillator paths and canonical replay.
- [x] Negative safety: non-clock consumers, inverted expressions, bound negative
  input tracing, unknown/changed mux, reservations on either half, introduced
  override cycle with old snapshot and atomic output preservation.
- [x] Minimal shared routing change, focused GREEN and connected native/CLI;
  do not change netlist source eligibility or add phase simulation.
- [x] Source-matched real vendor directional or direct duration probe using the
  existing harness; no certification claim without unchanged comparison.
- [x] Representative shared/wide/deep source/flat timings with baseline hashes,
  unchanged ordinary cases; style, independent final review, then immutable gates
  after verifiedTask90. All remaining event/clock/resource/coverage work stays open.

Independent review caught that generic Source forwarding from a negative terminal is intentionally rejected by shared6.3.12(e) eligibility. Remove that promised positive from this runtime-only scope; direct iClock on negative declared ports (including hierarchical targets) remains the intended gap. No frontend restriction is relaxed.

First RED6methods exit1: direct/hierarchical/mux/override positives reject the existing differential-path diagnostic. Heldclock CLI pilots use unsupported ClockPort iState; replace those with native preserved-map checks, keeping CLI assignment semantics unchanged. One ToTCK negative test guessed test-clock diagnostic; actual is clock-or-data. Preserve first pilot separately and correct fixture expectations before implementation.

Corrected RED2 build0/native1/CLI5methods5failures; actual2testhashesmatch.
Native direct positive pair rejects blanket unsafe fanout; direct root/output,
hierarchical ratio/oscillator, mux and override-source positives reject existing
differential-path diagnostic. Fixture errors from first pilot are not evidence.
Independent revised design approved, with native held-half/unsafe-association and
unused bound-negative positive-root tests required. Implement only shared routing
edges and root/cycle/safety changes; old blanket unsafe expectations superseded.

First GREEN build/native scheduler/all46 clock methods pass; finalindependent
source reviewclean. A brace-only Google-format correction is verified by a final
source-matched main build and all46 clock methods0 in the vendor preparation.
Actual5source/fixture/runner and6staged pilot input hashes match. Vendor rejects
negative iClock targets explicitly (recorded divergence from7.9.15 valid-port
interpretation), while the first normalized export lacks required tester inverse
input constraint. Preserve both failures; add only the diagnosed
add_input_constraints -equivalent root -invert root_n to the probe runner and
retry normalized export independently, without repeating the known source
rejection. This is a direct probe, not a formal certificate.

Add maintained wide65536 paired inputs and4096 stages with negative output
partners,1024 checked waits andfive canonical replays. No old functional baseline
exists for these newly supported paths; record candidate scale and unchanged
ordinary workloads without claiming a speedup. RootTask90 remains frozen.

Branch workflow updated by user: delete completed task branches after integration.
Historical44integrated refs removed;116draftfiles in11oldworktrees preserved and
backedup, thenoldworktreesdetached. RootTask90full0(1345.397s/editor181.414s),
requiredleakall0,remote50all0actual12; committed6702646 onTask90branch, fast-forward
master, deletedcompletedbranch. Saved14draftfiles/full.diff/hashes/stashes under
/tmp/differential-duration-task91-before-advance-files; restoredonly8Task91future
files on task/differential-clock-durations at6702646. Masterclean;only2branches.

Final remote GREEN2 build/native/all47 clockmethods0 withactual3source/testhashes.
New4096stage/1024waits/fiveforms2.204s;65536pairedinputsfiveforms.643s. Interleaved
matched scale and localconnected/style/offline nowrunning. Vendorqualified2
normalized export0 requires both physical pins add_clocks100ns andinverseinput
constraint. All3actualqualifiedinputhashesmatch, andall3orderedwaits match. Vendor
removesone redundant iClockroot; no strict directionalcertificate claimed. Source
negative-targetrejection andbothdiagnosedphysicalsetupfailures retained. No
product Source-kind or phase-model boundary was widened for vendor behavior.

Final help rebuild0, format0 and naming0. First main naming invocation omitted build macro ICL_FORMAT_VERSION; failed pilot retained, corrected invocation0 with no production fix. Core clock source/performance provenance and final3GREEN2source/tests verified. Local connected/native/main69/clock47/client25/reset12/multipleTAP22/feedback10,53offline,24matched scale commands and independent review pass. Integrate11reviewed files onto6702646 and freeze8non-document files for required leak/full checks. Final19hashremote native7/all41 running. Commit on task branch, fast-forward master and delete completed branch after verification.

Required Task91 leak-enabled build/native7/main69/clock47/client25/reset12 all0. Root8non-document files remain hash-identical to the reviewed snapshot; full serial regression is running in build/differential-duration-final-full.*. Final19hashremote verification remains pending.

Final Task91 remote50 statuses all0: build/native7/all41CLI/aggregate, with actual19source/test/fixture hashes matching the frozen submission. Required local leakall0; fullserial stillrunning. No production/test/fixture changes to Root8frozen files.

Verified Task91 checkpoint: full serial make check0 in1418.426s,41 dynamic and
41 static generator suites,53 directional goldens,2526ICL334/119rules,LSP62 and
editor6 in202.606s. OptionalANTLR5 remain skipped. Required leak-enabled build/
native7/main69/clock47/client25/reset12 all0. Final remote50 statuses all0 and
actual19source/test/fixture hashes match. All8non-document files stayed immutable
through the final gates. Connected, style/naming,53offline,24matched scale,
independent review and qualified normalized vendor direct probe pass; vendor
negative-target rejection and redundant-check export difference remain recorded.
Commit this11file snapshot on its task branch, fast-forward master and delete
that completed branch. Preserve only Task92future files and its new docsection
on the next active branch. No push. Remaining phase/event/absolute-time/resource,
vendor, strict coverage and final platform requirements remain mandatory.
