# Explicit reset-port polarity implementation plan

> Execute inline under existing autonomous authorization, with failing-first
> checks and independent final review. BASE:525f1b6. Use the existing isolated
> task/reset-port-polarity checkout. No new dependency or public metadata.

Goal: apply IEEE1687 6.4.6.13(a,b) to explicitly driven ResetPorts and reject
inconsistent direct reset wiring. ToResetPort retains its independent high
default under6.4.6.14(f). Existing implied connections retain their automatic
polarity conversion, confirmed by Tessent2023.4's global/local reset chapter.

Approaches: changing the default to low globally breaks undriven/high resets;
per-bit polarity metadata has no established requirement for the single
ActivePolarity attribute on a grouped declaration. Reuse the existing canonical
alias resolver, infer one consistent declared-port polarity from dictating typed
sources, and reject conflicting typed bits. Data/mux/literal/hole bits do not
dictate polarity. An entirely untyped port retains the high default.

Reach: src/pattern/pattern_netlist.cc, tests/src/pattern_netlist_test.cc,
tests/src/test_icl_pattern_generation_main.py, current support/coverage ledgers.
No RTL, grammar, scheduler limits, reset values or pattern assertions change.

Architecture: make the existing alias-terminal cache reusable and bit-cycle safe.
Create it after RestoreFlatOrigins; infer explicit ResetPort defaults parent-first
before InferResets. ResetSource/InferResets read resolved active_low. Validate
direct ResetPort/ToResetPort Source polarity after ordinary graph checks, including
restored flat metadata. Do not re-infer restored flags. Cache aliases only; never
cache port polarity or traverse mux/logic as transparent wires.

Review focus: multilevel inheritance, alias inversion/cancellation and cross-bit
cycles; explicit mismatch including output default; uniform/mixed vector sources;
implicit conversion; source/flat behavioral equivalence and tampered metadata.

- [x] Add native inference/conflict/vector/alias/implicit/flat tests and CLI
  inherited-low reset/write/read replay and atomic rejection checks. Run RED
  against525f1b6; retain individual outcomes and the existing functional baseline.
- [x] Implement shared private resolver and resolved-polarity inference/validation.
  Run native and main CLI focused checks GREEN; fix necessary affected callers.
- [x] Finish matched-input vendor loading qualification, explicitly recording
  standards/vendor differences and transfer failures. Loading is not PDL proof.
- [x] Measure representative wide ports,4096 aliases/hierarchy and ordinary data
  against retained525f1b6 binary; record time/RSS and equivalent output hashes.
- [x] Format/style, connected checks and independent review; save immutable files,
  integrate, run required full serial and leak-enabled checks, update ledgers,
  and commit only the verified snapshot. Broader remaining work stays mandatory.

Evidence: build/reset-polarity-functional-baseline reproduces failure at default
high child explicitly bound to low parent; explicit-low child and inverted-high
controls pass.82 loading cases already run locally:68 accepted,14 rejected by
existing grammar. No redundant numeric-polarity/TRST-property checks are needed.
Vendor probe initially failed SCP before execution because remote startup emits
Innovus messages; retry uses text transfer with separate transport/archive status.

Native inheritance and main56 pass on initial implementation. Connected native7
and main56/clock41/client18/reset12/multiple-TAP22/feedback10 all pass. Independent
review found one fixed-low TRST metadata issue; TestFlatTrstPolarity reproduces it
RED and a guard was added to the existing kind-classification pass. Final focused
GREEN/style checks are now running. Alias lookup retains find-before-emplace to
avoid allocation on cached references. Initial native fixture corrections are
recorded in the execution ledger; no production behavior was relaxed.

Vendor82 submitted/fetched model+runner hashes all match:37 load,31 fail ICL64
polarity checks and14 fail existing grammar/property checks (ICL1/13). The vendor
keeps omitted explicit ResetPort polarity high; the primary6.4.6.13(b) exception
instead dictates it from the source. Normalized, literal and hole vectors with
explicit low load; conflicting typed vectors reject. A low output driven by a
high reset mux also rejects vendor polarity checks; mux-output polarity/forcing
is separate remaining work, not silently included here. No PDL equivalence claim.
Legacy and SFTP transfers failed on Innovus startup text before execution; the
text-safe upload/fetch succeeded.82 individual exits plus final archive retained
under build/reset-polarity-vendor-probe; original bounded SSH transport remains
separate from the manually fetched completed evidence.

Final focused native netlist and main56 pass after TRST correction; formatting and
naming exit0.48 interleaved source/flat loading runs pass with identical pattern
and flat hashes against525f1b6.65536 reset bits (156 bytes) take0.065–0.115s source,
peak54128KiB versus52916KiB;4096 aliases (88004 bytes) take0.115s;4096 child scopes
(99383 bytes) take0.115s;655360 ordinary references (335 bytes) take0.115–0.165s.
Exact commands, environment/compiler, inputs, source/binary hashes, repetitions
and RSS remain in build/reset-polarity-loading-scale/comparison.json. Whole
empty-PDL CLI/debug I/O is measured, not isolated guard time or a speedup claim.
All82 candidate loading cases were rerun:46 accept,36 reject; baseline68 accept/
14 grammar rejects. Additional typed conflicts reject, inherited defaults remain
valid per primary. Vendor differences stay explicit. Atomic conflict tests also
retain pattern, reset and debug artifacts together.

All required final leak-enabled ASan/UBSan checks pass: native7 plus main56/clock41/
client18/reset12, aggregate build/reset-polarity-final-sanitizer-all.exit0. All6
root files matched the immutable reviewed manifest before this full serial run.
Source/tests stay frozen. Final make-j1check now runs in
build/reset-polarity-final-full.log. Additional remote current-source native/main
checks run separately; no platform qualification is claimed before their exits.
The original vendor transport times out124; its later fetch command returns0 but
its wrapper marker parsing fails with IndexError. The independently fetched
archive has manual-fetch.exit0,82 checked individual exits and matched inputs;
that manual evidence is authoritative, not the failed wrapper.

Additional current-source remote build/native netlist/main56 each exit0, transport0 and fetched archive0. All3 submitted/remote hashes match the frozen source and tests. This focused Rocky8 evidence does not establish all-tools/static/install qualification.

Final serial make-j1check exits0 in build/reset-polarity-final-full.log/.exit0,
1339.731474s,41 dynamic/static generator suites,51 directional goldens,2523 ICL
334/119 rules, LSP62 and editor6 in181.382s. Optional ANTLR5 skipped. All3 final
production/test hashes match the immutable reviewed manifest; docs are updated
with the final evidence. Required leak/connected/performance/style/review checks
pass. No deadlines, assertions, reset values or coverage thresholds changed.
Broader remaining requirements are retained explicitly. Commit this verified71
snapshot before advancing the isolated mux draft.
