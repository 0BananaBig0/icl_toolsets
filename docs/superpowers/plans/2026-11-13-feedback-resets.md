# Feedback reset implementation plan

> **For agentic workers:** Use superpowers:executing-plans for inline implementation.

**Goal:** Schedule reset networks containing register feedback using the existing
simultaneous reset semantics, including mixed ScanRegister/DataRegister storage.

**Architecture:** Condense the existing register reset graph into strongly
connected components. Retain snapshot settlement and concrete replay. Use a
proved bounded number of reset-only passes; group mutually dependent reset
release candidates together and retain ordering between components.

**Tech Stack:** Existing C++17, Z3, Python unittest; no dependencies.
**Spec:** ../specs/2026-10-25-icl-pattern-generation-design.md, Scheduling and local reset.

## Constraints and approaches

The user delegated implementation choices. Reuse the shared scheduler/state
model and current source-output contract. Algebraic fixed-point equations alone
can invent an unreachable stable state; sequential snapshots preserve initial
state and transient resets. Rejecting every cycle is the current missing feature.
The chosen approach adds graph components and a finite bound to the existing
snapshot simulation, without a second solver or event engine.

Each register has one constant ResetValue. During reset-only settlement, each
vector can change at most twice: original to the merge with ResetValue under an
unknown control, then to ResetValue under an asserted control. Once reset, it
cannot change again. Unknown ResetValue bits only erase knowledge. For fixed
external inputs, a component of N registers therefore needs at most 2N changing
passes. Acyclic singleton components retain their one-pass weight; weighted
longest paths account for upstream propagation. Independent review confirms the bound. Add one final event-observation pass
beyond the cyclic hardware bound, because unchanged storage may acquire a
reset event or controlling-fill update. Retain global snapshots across
components to preserve transient upstream assertions.

Keep current solver/traversal/work budgets. A budget diagnostic is not a proof
of physical unreachability. Candidate release writes must still pass scheduling
and concrete replay. Raw scan payloads, arbitrary gated edges and callbacks are
separate remaining requirements. No RTL changes are involved.

## Scope and review focus

- src/pattern/pattern_scheduler.cc: BuildResetOrder, ResetPrerequisites,
  reset component metadata and snapshot-pass bound.
- src/pattern/pattern_state.cc: concrete reset settlement bound.
- tests/src/pattern_scheduler_test.cc and pattern_state_test.cc: concrete state,
  fill, unknown controls, transient assertions, reverse order and large graphs.
- tests/src/pattern/test_feedback_resets.py: new CLI original/flat/generated
  replay, local-reset rewriting, mixed feedback and atomic infeasible output.
- Existing test_data_reset_dependencies.py: replace blanket cycle rejection with
  behavior-specific checks; retain impossible-write diagnostics.
- Makefile/test_makefile, guide/model/spec and vendor fixtures/evidence.

Expected scale: ordinary tens of feedback controls and a 4096-register graph
construction benchmark. Iterative graph traversal avoids recursion proportional
to graph depth. Graph cost O(V+E), apart from existing ordered maps/sets;
settlement is bounded by weighted depth times nodes and storage width, subject
to the existing 1,048,576 work limit. Measure construction and 8/32-register
execution; do not claim unrestricted cyclic input throughput.

## Steps

- [x] Independently review the convergence and component-layering design.
- [x] Write failing native/CLI tests for self feedback, two-register mutual
  feedback, mixed storage, known/unknown initial state, transient reset history,
  and atomic impossible requests. Run against the Task38 binary and retain red.
- [x] Implement iterative components and weighted pass bounds, preserving the
  acyclic fast path and caller-only release output.
- [x] Run focused native/CLI tests and representative scale checks; resolve
  independent implementation review findings.
- [x] Run full dynamic/static regression, leak-enabled sanitizers, format/naming,
  and a fresh vendor probe/certificate where the vendor supports the fixture.
- [x] Record evidence and limitations, update requirement ledgers and commit.

Initial red: seven CLI cases all reject the pre-change blanket feedback check.
The first implementation passes six, exposing a symbolic/concrete mismatch for
an unknown mutual reset. Capture retains the old snapshot; update now consumes a
separately settled snapshot, including reset/fill/write-group checks. Eight cases
then pass. Native state and scheduler checks pass;4096-register construction is
0.003600s acyclic and0.004153s cyclic on local Linux g++16.2.1 -O2.

The ninth test measures8/32-register rings. Recursive expressions hit the existing
two-second optimize limit at32registers (32.007s whole test run including expression
construction). Fresh constrained snapshots reduce the run to5.572s but still
hit the limit on a read; native bit-vector scan objectives also fail and were
reverted. Factoring the one-bit reset knownness test outside its value selection
retains constant known masks. This passes all9cases in2.497s:8/32rings with
801/2475ICL bytes take0.171s/1.124s over three replays, wall2.54s/RSS67144KiB.
An isolated follow-up checks whether that factorization makes the added snapshot
variables unnecessary. No solver budget or test size was increased/reduced.

General reset changes driven by serial/phase pins during shifting remain part of
the outstanding per-edge model work. The post-capture snapshot alone does not
establish arbitrary per-edge support. Existing concrete replay remains mandatory.

The inline-only factored variant passes but32registers take20.828s versus1.124s
with named snapshots. Retain the measured bounded-expression encoding.

Vendor diagnosis expands the affected evaluator scope: an unknown reset-mux
selector currently hides a known0 even when both possible branches are0. Add
UncoveredMuxCodes(const NetlistNode&) in pattern_netlist.h/.cc using bounded
cube subtraction, then join compatible mux branches in both concrete and symbolic
evaluators. Any compatible uncovered encoding retains an all-X result. Charge
full-width scans and copies against4MiB of character work; stop on full coverage;
keep known-selector evaluation on its existing fast path. No serializer or
NetlistNode metadata change, selector-value enumeration or new dependency.
Independent review approves this algorithm with these bounds.

Add an independent small-width binary-completion oracle in
 tests/src/pattern_symbolic_test.cc covering complete/partial/overlapping cases,
unknown data bits and symbolic masks; verify the feedback vendor fixture starts
without an invented preliminary scan. This precision fix is needed for the
self-reset feedback initialization exposed by the vendor comparison.

Final integrated source passes the completion-oracle test and native state/
scheduler checks. Ten CLI cases pass in2.627s, with8/32rings at0.165s/1.144s and
wall2.67s/RSS66940KiB. Reverse certification passes in
build/feedback-reset-mux-reverse-certificate.log/.exit(0), after the prior failed
comparison exposed reset-mux precision. The accepted vendor artifact is unchanged.

The first integrated style check required four brace fixes and reducing Solve
below the configured statement threshold. InitialValues now contains its original
three initialization loops, preserving insertion order and overwrite behavior;
independent review confirms equivalence. Formatting and final scheduler/state
naming checks pass; netlist/symbolic had no errors in the earlier naming run.
Initial full/sanitizer jobs were stopped and recorded as interrupted. Exact-source
verification is restarted in build/feedback-final-full.log and
build/feedback-final-sanitizer-*.log. All three native and eight CLI sanitizer
checks now pass with leak detection and UBSan halt-on-error enabled;
build/feedback-final-sanitizer-all.exit is 0.

Final full regression passes in build/feedback-final-full.log/.exit(0): all39
generator suites under dynamic/static builds, native checks,50 directional and
seven legacy vendor standards,2,518 ICL files covering334/119 parser/scanner
rules, and six editor tests in179.205s. Five optional ANTLR tests are skipped
because ICL_REF_DIR is unset. The final sanitizer suites cover feedback10,
reset-dependency13, reset-release14, reservations35, arguments12, queued-state15,
data scheduling20 and hidden clients8; all pass. Final format/naming/whitespace
checks and independent source/evidence review pass. Review corrected stale
blanket-feedback rejection claims in the current guide and marked Task35's
superseded restrictions as historical.

Task25's unchanged strict coverage gate is running against this implementation
in build/feedback-coverage-suite.log. It is separate unfinished acceptance;
this regression result does not establish coverage or complete retargeting.
