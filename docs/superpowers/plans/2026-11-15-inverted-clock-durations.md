# Inverted clock-only duration routes

Execute inline in the isolated worktree while RootTask91 source/tests/fixtures
remain frozen for required sanitizer/full checks. Preserve task-first commit,
fast-forward integration and deletion of each completed branch; no extra branch
until Task91 is verified and integrated.

Goal: support explicit inversion in otherwise supported clock-only paths for
relative iClock/-sck cycle counts. IEEE1687 6.3.12 clock_signal permits inversion,
and complementing a periodic binary clock preserves its frequency and cycle
count.6.4.6.20 frequency ratios and7.9.15/.17 duration tracing remain unchanged.
This does not calculate phases, simulate clock events or establish CDC safety.

Current root cause: Resolve rejects every source.inverted, and BuildSafety marks
inverted routing edges unsafe even when they have no non-clock consumer. The
existing declared differential route already normalizes inverse halves to the
same frequency root. Reuse that duration-only argument for explicit inversion.

Smallest design: follow finite inverted references in Resolve, retaining missing/
constant-source rejection; allow inversion on existing clock/alias routing edges
in BuildSafety. Non-routing consumers remain unsafe regardless of inversion.
No new route state, phase field, cache, graph, solver, public flag or metadata.
Keep frontend terminal-kind and negative-source restrictions, bound differential
input rejection, current override versions and all existing numeric/mux checks.
Update the inactive-source diagnostic if its old wording implies inversion itself
is still unsupported, and update affected callers rather than retaining a false
negative expectation.

Reach: src/pattern/pattern_clock.cc Resolve/BuildSafety only; existing native
scheduler clock tests and tests/src/pattern/test_system_clocks.py; guide/model/
help if needed. PDL override -source syntax is unchanged; an omitted Source can
retain an inverted ICL default. Existing Source/inversion bits stay intact.

- [x] Independent design review of frequency-only equivalence, safety and holds.
- [x] RED direct/alias/nested/vector/mux/declared-differential inverted routes;
  original/flat/generated/reset-only equality; update obsolete blanket failures.
- [x] Native held input/output and non-clock downstream propagation through an
  explicit inverse; zero waits and unchanged hardware state; old/new override
  snapshots with an inverted ICL source; atomic disabled/missing-source failures.
- [x] Minimal shared change, focused GREEN and necessary connected callers.
- [x] Source-matched vendor probe and explicit discrepancies; matched ordinary/
  deep/wide scale without a claimed unsupported old functional baseline.
- [x] Style/independent review, then immutable required integration gates after
  Task91 passes. General phases, events, absolute time, ownership and strict
  coverage remain mandatory independent requirements.

Independent design review accepts the two-site change: inversion preserves root/frequency ratio for relativecounts; either polarity still toggles a held input/output; actualnon-clockfanout propagation remains; override defaults already retain BitReference::inverted. No newmodelstate needed. Prepared3CLI andnative source/flattests only; remoteRED waits for finalTask91remote completion.

RED build0/native1/3CLI methods1 with7failed subcases; actual2testhashesmatch. Direct/alias/cancelled/differential/mux/override cases reject inactive-or-inverted, unused inversealias rejects unsafe fanout; nativefails same guard. Implement only the2existing gates plus honest inactive-source diagnostic. Preserve finite-source and non-routing checks. Replace obsolete inversion-only failures with positive replay, add oscillator and same-frequency polarity-reroute cases, and explicitly retain0/1/X/invertedliteral atomic failures.

GREEN build/native/all51clockmethods0, actual3source/testhashesmatch; finalindependentreviewclean. Bothvendororiginal/normalizedexport0 andbyte-equal; orderedwaits7/8/7/5/8 agree,5submitted+6stagedhashesmatch; redundanticlockremoved, nodirectionalcertificateclaim. Matched24scalecommands0: ordinarybaseline.25–.38s/candidate.25–.36s, newinverse4096 .33–.48s/121604KiB, wide65536 .18–.20s/93008KiB, canonicalPDL/flathashesequal. Final22hashremoteall41/native7 running; localchecks waitRootTask91fullcompletion.

Advanced onto verified0bf0fb3 after Task91full0(1418.426s/editor202.606s), leakall0/finalremote50all0actual19. Saved15draftfiles/full.diff/hashes/stashes under/tmp/inverted-duration-task92-before-advance-files; Task91committedonbranch, fast-forwardmaster, completedbranchdeleted. Restoredonly7Task92futurefiles andits new docsections, preserving finalTask91docs; onlymaster+activeTask92branch remain. Localconnected/native7/main69/clock51/client25/reset12/multipleTAP22/feedback10, format/naming and53offlineall0. Freeze9reviewedfiles (6non-document) for finalleak/full; final22hashremote currently running.

Task92 required leak-enabled build/native7/main69/clock51/client25/reset12 all0; finalremote50 statuses all0 with actual22source/test/fixture hashes matching. Root6non-document files remain hash-identical to the reviewed snapshot. Fullserial verification is running in build/inverted-duration-final-full.*; commit/fast-forward/delete only after it passes.

Verified Task92 checkpoint: fullserial make check0 in1346.839s,41dynamic and
41static generator suites,53directional goldens,2527ICL334/119rules,LSP62 and
editor6 in184.735s. OptionalANTLR5 remain skipped. Required leak-enabled build/
native7/main69/clock51/client25/reset12 all0. Finalremote50 statuses all0, actual22
source/test/fixture hashes match;6non-document files stayed immutable. Connected,
style/naming,53offline,24matched scale and independent review pass. Original and
normalized vendor exports0 andbyte-identical withfiveorderedwaits matching;
redundanticlockchecksremoved, no strictdirectionalcertificateclaimed. Commit9files
on taskbranch, fast-forwardmaster anddeletecompletedbranch. Preserve only the
new opaque-root fixtures on the next active branch. No push. Broader scan/clock/
phase/reset/resource, fullvendor, strictcoverage andfinalplatform work remains.
