# Pattern generation reference review

## Authority

Follow [L3 in requirements](requirements.md#language-conformance-and-references):
IEEE 1687 first, then applicable release-specific Tessent IJTAG, ATPG, user and
command/reference manuals for behavior the standard does not explicitly resolve.
Ask the user if a material ambiguity remains. External implementations supply
algorithm and test ideas; they do not establish language semantics.

## Honza255/icl_parser

Reviewed the cached checkout at commit
`333f8bc7a54964138ac14a34fde9e942c496436d` from
<https://github.com/Honza255/icl_parser>. This is a source review, not a new run
of its retargeting or HDL simulation suites and not a claim about upstream HEAD.
CodeGraph returned this project's index rather than the external checkout;
targeted source inspection supplied the evidence below.

The repository implements partial retargeting, beyond syntax parsing:

| Source | Observed behavior | Application here |
| --- | --- | --- |
| `src/ijtag.py`: `iWrite`, `iRead`, `iApply`, `getiApplyVectors` | Queues register requirements and exposes calculated scan-vector records. | Compare state transitions and prerequisite selection on supported small networks. |
| `src/icl_register_model.py`: `iApply` | Builds initial/target requirements, invokes the solver, and constructs `stepScanData` records with IR/DR selection. | Keep runtime operations separate from netlist state and emitted output. |
| `src/icl_retargeting.py`: `LVec` | Separate value and defined bit-vectors preserve unknown state. | Retain explicit unknown-state handling; never initialize hardware from ResetValue alone. |
| `set_max_steps`, `retarget` | Tries bounded transition counts in increasing order, then minimizes bit-change cost within the first satisfiable horizon. | Use small bounded models as comparison inputs; retain our scan-load/parallel-operation then scan-cycle cost objective. |
| `tests/test_icl_syntax_7.py`, `tests/config.py` | Drives generated vectors into an HDL DUT using Cocotb and checks observed values. | Add independent execution checks when our scheduler and emitter exist. |

The inspected path produces scan-vector data; no flattened PDL text writer was
found in that path. It does not establish our ordered multi-file iProc semantics,
local-reset rewrite, canonical flat-ICL replay or generated-PDL idempotency.

The README calls the project a prototype and lists unsupported features including
local reset/capture/update, broadcast and data-register retargeting. Current
source also contains data-register constraints, so that list alone cannot prove
the actual coverage. Verify individual capabilities using tests before treating
them as an oracle. Do not copy its use of a side-effecting solver call inside
`assert` in `IclRegisterModel.iApply`: Python optimization can remove that call.
Also distinguish solver `unknown`/budget exhaustion from proven unreachable
targets; the inspected bounded solver path does not make that distinction.

## Addressed models: inspected sources and vendor experiment

IEEE 1687-2014 provides these relevant rules:

- §6.4.6.7(f), printed p.89: concatenate an addressed instance's DataOutPorts in
  declaration order, each left-to-right.
- §6.4.6.24(a), p.108: form logicalAddress in the same declaration/bit order.
- §6.4.7(s–v), p.112: implicit address, read-enable, write-enable and data-input
  associations for addressed instances.
- §6.4.15(a–e), p.130: group source widths may be smaller than the output;
  Port, Instance and DataRegister contributions have distinct enabling rules.

The locally available Tessent manuals are v2023.4, under
`/home/banana/win11/Desktop/tessent_2023_doc/htmldocs/`:

- `tshell_ref/topics/Command_Onehotdatagroup_idd27fdbfc.html`: explicitly gives
  X when no source or multiple sources are selected; documents an external
  enabling condition for top-level input ports. Keep that vendor rule identified.
- `tshell_ref/topics/Command_Instance_id9dd6ea02.html`: documents addressed
  instance defaults and the sum of hierarchical AddressValue contributions.
- `tshell_ref/topics/Command_Addressport_id590ae963.html`: logical address order.
- `tshell_ref/topics/General_Icl75_idcb702e19.html`: enabled contributor checks.
- `tshell_ref/topics/General_Icl65_ided9fd38f.html`: source/sink compatibility.
- `tshell_ijtag_user/topics/Concept_HowModelAddressableRegistersInIcl_id0bc6e9b6.html`:
  explicit mux/register modeling of indirect addressing and iApplyEndState.
- `tshell_ijtag_user/topics/Concept_IclPdlLimitations_id25d0043e.html`: lists
  vendor limitations; these do not remove our IEEE support requirements.

The inspected IEEE rules and Tessent topics do not
explicitly state alignment and padding for a selected source narrower than its
OneHotDataGroup. Searches of the IJTAG, Shell Reference, Shell User and ATPG
manuals did not locate an explicit rule. Numeric-literal padding does not by
itself resolve signal padding. At the user's direction, Tessent Shell **2026.2**
was exercised on 2026-10-25, separately from the v2023.4 manual review.

An enabled eight-bit DataOutPort contributing to a 16-bit OneHotDataGroup is
right-aligned in declaration/significance order; the leading eight group bits
are unknown. `iWrite observed 0bXXXXXXXX10010110` succeeds for both `[15:0]`
and `[0:15]` declarations. `0x0096`, `0xff96` and
`0b10010110XXXXXXXX` fail with `fixed to 'unknown'` on the leading bits.
The equal-width eight-bit control succeeds with `0x0096`.

This resolves the tested **Port contribution** case, not every DataRegister or
addressed Instance contribution. It is not a test of numeric padding for an
ordinary 16-bit ScanRegister. The test inputs, command flow, results and remaining
comparison dependencies are recorded in [Tessent oracle verification](tessent-oracle.md).

## AccessLink and external BSDL dependency

IEEE 1687-2014 §6.4.17, printed pp.133–135, distinguishes standard IEEE 1149.1
AccessLinks from opaque generic controllers. Standard links live at the top
level, refer to an external BSDLEntity and its INSTRUCTION_OPCODE entries, and
select ordered, single-chain child ScanInterfaces. ActiveSignals are named
outputs in the link's namespace and are active only for instructions listing
them. Generic-controller semantics are deliberately left unspecified; accepting
its text is not enough to infer a hardware model.

Annex E.29/E.30, pp.249–252, demonstrates implicit scan-interface attachment,
serial concatenation and qualified `Tap1.toSWIR` connections. E.30 contains
obsolete spellings (`STD_1149_1`, `BSDL_Entity`) and typographical mistakes;
new fixtures must use the normative §6.4.17 grammar and complete missing
instrument definitions rather than copying those mistakes.

Tessent Shell Reference Manual **v2023.4**,
`tshell_ref/topics/Command_Accesslink_id84260660.html`, explicitly requires
`read_icl ... -bsdl_file ...` and a matching BSDL entity. It states support for
the 1149.1-2001 BSDL form, separately from the IEEE grammar accepting 2013.
Elaboration replaces the AccessLink with a generated TAP instance under the same
instance name and adds its external TAP ports. The generated instruction and
scan-path models depend on BSDL attributes; ICL alone does not supply their
widths or opcodes. The documented reset value of that generated model is not an
instruction to reset hardware automatically.

The current repository fixtures under `standard/access_link/` and
`tessent/access_link/` are syntax examples, not complete AccessLink retargeting
inputs: they lack the referenced BSDL and include placeholder instrument ports.
The generator design registry now accepts selected BSDL entities through
`AddBsdl(path, text, entity)`. Its independent metadata reader extracts static
TAP attributes; it does not load VHDL packages or construct TAP hardware.
Duplicate BSDL entities are rejected, preserving the earlier model; this is
separate from the requested ordered ICL overrides. The provisional CLI design
uses a separate `--bsdl` input, but the executable and its options remain
unfinished. TAP construction and native AccessLink retargeting are still needed
for fully expanded debug ICL. No opcode, TAP width, missing default connection
or generic controller behavior should be invented to hide this gap.
