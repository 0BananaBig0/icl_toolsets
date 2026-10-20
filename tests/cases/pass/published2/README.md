# Two more ICL sources found by searching GitHub for IJTAG

## sklp-eda-lab/ieee_jtag_ijtag_reference — 3 files

`ICL/TapController.icl`, `ICL/picorv32_top_ctrl_tdr.icl`, `ICL/sib.icl`.
An IEEE JTAG/IJTAG reference built around a picorv32 — the ICL describes a
real TAP controller and a real SIB, not a synthetic network.

Worth noting for what it does *not* contain: no `Enum`, no `Alias`, no
`Attribute`, no `AccessLink`. Three files that exercise the ordinary port and
register blocks only, which is what most hand-written ICL looks like.

## du2693742095/IJTAG_extension — 1 file

`test/featureTest.icl`. Most of it is commented out — several complete module
definitions inside `//` — and the live part is a `Module Context`.

That shape is itself worth a case: **a file whose content is mostly comments**.
The formatter has to reproduce the commented-out modules byte for byte, since
they are not tokens it can re-emit from a tree. It does, and the file is
idempotent and character-preserving.

All four pass.
