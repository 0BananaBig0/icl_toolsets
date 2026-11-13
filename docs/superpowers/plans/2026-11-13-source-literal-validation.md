# ICL literal source validation

Task32 continues the approved source/default model-validation work.
IEEE1687-2014 §6.3.12(l,m) requires an unsized concatenation element to occupy
at least one bit and forbids inversion of unsized elements. Both the shared
constant evaluator and netlist signal resolver currently permit inversion
after applying a target width. Reject it before contextual sizing; keep sized
literal inversion, signal inversion, parameter substitution, X extension and
legal unsized contextual padding unchanged.

Inspect both evaluator paths and their callers: enum/default/reset/address/mux
constants and connection/capture/data/scan sources. Reuse existing parsing and
sizing. No new representation or dependency; complexity remains linear in
operand bits. Existing million-bit operand limits remain.

- [x] Add failing native and CLI cases across literal source/constant contexts,
  including parameters, sized controls, X values, zero available padding width
  and atomic output preservation.
- [x] Correct both evaluator paths at their shared literal boundaries.
- [x] Run connected native/CLI tests, full dynamic/static checks, leak-enabled
  sanitizers, formatting/naming and independent review. Check Tessent behavior
  for a representative rejected/accepted pair; record exact evidence.

Source-kind/role/hierarchy validation remains a distinct next step. Its alias
and control-mux rules need vendor checks before changing existing accepted
hierarchical models.

Ruling: follow the explicit normative prohibition in6.3.12(m). Section6.3.11's
known-width example contradicts it, and Tessent2026.2 accepts both unsized and
sized inversions. Document that compatibility difference; do not describe the
probe as matching vendor rejection. No unresolved behavior is invented: this
implements the explicit normative rule under the approved standard-first policy.
Review also found WriteEnSource ~~0: parity cancellation must not conceal the
presence of an inversion operator. Preserve sized/signal double inversion.

All steps passed. Final full regression: build/source-literals-regression.log
and .exit(0),32CLI suites,44directions/seven legacy,2512ICL,6editor208.212s.
Eight CLI and native expression/netlist cases passed leak-enabled sanitizers;
naming/format/whitespace and independent review passed. Five corrected Tessent
probes accept unsized/sized variants and the exact conflicting example, as
documented above. The generator guide records this compatibility difference.
