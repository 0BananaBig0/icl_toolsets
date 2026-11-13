# Direct connection hierarchy validation

Continue the approved source-connection acceptance work with inline execution.
The user delegated implementation choices and requested continued completion.

IEEE1687-2014 §6.3.10.5 explicitly limits ordinary references to instances in
the same module and reserves U1.U2.pinA for Alias definitions. §6.3.12 requires
hierarchical hardware signal references to name
appropriate output ports on a local instance. §6.5.2 permits deeper register/port
references in Alias definitions; PDL target resolution is separate. Current
Netlist::Resolve finds any matching descendant node, so direct hardware wiring
can bypass module interfaces. Task39's rejected Tessent `SelectedBy c.value[0]`
probe independently demonstrates the direct-child-register problem.

Use the existing shared resolver rather than separate checks in every source,
mux, logic or binding caller. Replace its scan_input Boolean with a three-value
reference-use enum: connection, scan input and alias definition. The alias
definition path retains the existing descendant resolution. In ordinary
connections, a nonlocal resolved node must be an output port. The shared parser
already rejects multi-level hardware references; retain that rule and diagnostic
without adding a second depth check. Check once per named operand before bit
expansion, with a source-located error. Keep rightmost ScanInSource semantics,
bit budgets and inversion unchanged. PDL target lookup remains unchanged.

This is the direct hierarchy boundary, not complete source-kind validation.
Conditional control/data mux kinds and restrictions on later ICL use of deep
aliases still require separate work; do not claim that this increment closes
those requirements. No RTL, dependency, serializer metadata or solver changes.

Affected files: pattern_netlist.h/.cc, explicit resolver modes in
pattern_logic.cc, existing native netlist tests, existing generator CLI tests
for atomic rejection and original/flat/generated deep-target replay, and the
current guide/model/spec ledgers. Update existing synthetic test wiring only
where it bypasses interfaces; preserve the intended test behavior through ports
or permitted aliases. Stored vendor inputs should already use valid ports.

The check is O(1) per reference and adds no graph traversal or per-bit work.
Use the existing5000-instance netlist benchmark as a scale regression; retain
its input/environment/wall/RSS measurement without asserting a speedup.

- [x] Independently review the direct-hierarchy/alias distinction and callers.
- [x] Add native and CLI cases that fail against30ff3d0: direct child register,
  logic, alias and input port; binding/mux/logic contexts. Preserve the existing
  parser rejection of a grandchild output.
  Preserve immediate child output and deep alias/PDL access, including flat replay.
- [x] Implement the shared check; repair affected synthetic wiring if necessary.
- [x] Run focused checks, full dynamic/static regression, affected leak-enabled
  sanitizers, format/naming, scale check and independent review.
- [x] Record exact evidence and residual source-kind work, integrate and commit.

Independent review confirms the primary depth rule at cached standard lines
4348–4379. Resolve relative to expression.scope so sibling-output bindings stay
valid. Check the declared node before alias expansion, during Connect before
RestoreFlatOrigins; restored original scopes must not be checked a second time.
Add positive control-output and inverted/sliced-output tests alongside sibling
bindings. Existing flat references are encoded local names.

Initial red CLI results contain three genuine accepted-invalid-input failures
(child register/alias/input) and one diagnostic expectation mismatch: the parser
already rejects the grandchild output. Correct the latter expectation and retain
the parser check rather than duplicating it in elaboration. Native positives
pass, followed by the expected failure because child-register wiring is accepted.
Evidence: build/connection-hierarchy-red-{native,cli}.log/.exit(1).

The shared check passes native netlist tests in
build/connection-hierarchy-focused-native.log/.exit(0). The first27-method CLI
run exposes direct trigger.r/gate.r wiring in access_link/idle_reset.icl. Add
ungated eight-bit DataOutPort q forwarding r in those two modules and retarget
only the ICL bindings to q; preserve PDL register targets and bit indices.
The corrected27-method CLI run passes in
build/connection-hierarchy-corrected-cli.log/.exit(0),7.801s. Independent review
confirms the source change, flat replay and fixture equivalence. Full verification was pending at this checkpoint; final evidence is below.

Formatting and naming pass. Three alternating paired benchmark runs on local
Linux/g++16.2.1 -O2 retain the5000-instance/377,286-byte/15,002-node workload,
plus the existing wide sparse/group and5000-child data-bus cases. Baseline whole
wall time is0.82–0.87s/RSS162,568–162,652KiB; changed is0.81–0.88s/
162,584–162,704KiB. The main elaboration times are59.057–66.091ms before and
62.812–85.768ms after. These noisy measurements establish exercised scale, not
a speedup or an arbitrary-input runtime guarantee. Logs:
build/connection-hierarchy-benchmark-{baseline,changed}-{1,2,3}.log/.exit(0).

Leak-enabled ASan/UBSan native netlist, main27 and port-reservation8 checks pass
in build/connection-hierarchy-sanitizer-{native,cli,test_port_reservations}.log/
.exit(0). The wrapper then reports a nonexistent redundant enumeration suite
and exits1; enumeration cases already run in the main suite. The retained
connection-hierarchy-sanitizer-verified.log/.exit(0) audits the actual build,
link and test exits without rerunning them. No check result is rewritten.
The first full run then finds five multiple-client fixture failures caused by
direct child-register inputs. The remaining static jobs were stopped; the
dynamic failures and interruption are retained in build/connection-hierarchy-full.log
and .interrupted. Expose value through ungated parameter-width q ports and
update only ICL connections/replacement strings in the multiple-client, named
group, ShiftEn and self-feedback fixtures. PDL targets and reset assertions
remain unchanged; independent review confirms equivalence. Corrected
multiple-client17 tests and all14 later generator suites pass, with logs under
build/connection-hierarchy-corrected-* and build/connection-hierarchy-tail-*.

The final full run is build/connection-hierarchy-final-full.log/.exit.
Because the resolver declaration changed, final sanitizers rebuild every
dependency in build/connection-hierarchy-sanitize rather than relying on earlier
objects compiled against the previous declaration. Native netlist plus main27,
multiple-client17, named-group11, ShiftEn11, port8 and data-reset13 checks run
with leak detection and UBSan halt-on-error. Evidence is under
build/connection-hierarchy-final-sanitizer-*. The clean build and all selected
checks pass, with connection-hierarchy-final-sanitizer-all.exit(0).

Final full regression exits0:39 dynamic and39 static generator suites, native
checks,50 directional/seven legacy vendor standards,2,519 ICL files exercising
334/119 parser/scanner rules, and six editor tests in177.025s. Five optional
ANTLR checks remain skipped. Independent source/fixture/evidence review is
clean. Task25 strict coverage remains unmet; its latest96.1%/61.7% measurement
predates this change. No new vendor certificate is claimed.
