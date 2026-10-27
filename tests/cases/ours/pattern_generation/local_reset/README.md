# Local-reset rewrite fixtures

Self-constructed from the user's transitive-reset requirement. `root` releases
`branch.release`, which releases both leaf registers. `gate` controls access
only and must not be inserted into the reset-only PDL artifact. Original
procedure text and ordered multi-file execution must survive rewriting.

`parallel.icl` shares `Leaf` and checks independent reset signals of opposite
polarity grouped under one apply. `vendor.icl` shares `Leaf`/`ResetStage` and
adds an explicit AccessLink/BSDL interface and a TRST reset binding. The
reset sources use DataMux gating and ToResetPort forwarding across hierarchy.
`vendor.pdl` exercises a preserved procedure call through that interface.

The separate `oracle_cases.json` runs through `tests/tessent_oracle.py` in
comparison mode: our generator consumes the source PDL, while Tessent consumes
the generated `local_reset.pdl`. Tessent 2026.2 accepted the explicit TAP case
on 2026-10-27. Strict exported traces differ, so this is not a golden-equivalence
fixture; see `docs/tessent-oracle.md` for the observed differences. The CLI
regression independently checks source/flat replay and output idempotency.
