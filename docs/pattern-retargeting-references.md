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

The local **IEEE Std 1149.1-2001 (R2008)** PDF is
`~/win11/Zotero/storage/RLZEUHNS/IEEE Standard Test Access Port and Boundary-Scan Architecture.pdf`.
Annex B.8.13.3(a), printed157/PDF165, specifies the nine mandatory register
associations and requires validation when they are explicitly repeated in BSDL.
Section8.14.1(b–c), printed58/PDF66, selects DEVICE_ID for USERCODE with its own
capture data. Section8.1.1(d), printed36/PDF44, requires otherwise unused binary
codes to act as BYPASS. These rules now inform the native datapath; see
[model coverage and remaining limitations](pattern-model-coverage.md).

### Native TAP controller timing

IEEE Std 1149.1-2001 (R2008), local Zotero `RLZEUHNS`: Figure 6-1 and
§6.1.2 (printed 18–23 / PDF 26–31) define the state machine and five-TMS reset.
§6.2.1/Table 6-2 (printed 24–27 / PDF 32–35) specify falling-edge TDO controls;
§6.3.1 (printed 31 / PDF 39) specifies asynchronous active-low TRST;
§7.2.1/Table 7-1 (printed 34–35 / PDF 42–43) separates rising capture/shift,
falling instruction update/reset, and asynchronous instruction reset. These
govern `TapController`; a runtime consuming its actions is still required.

The instruction-register continuation consumes those IR actions in
`TapInstructionRegister`. Table 7-1's separate shift/parallel stages and
§7.2.1(c–f) govern capture, shift, update and reset timing; undefined shift-stage
values are not treated as retained hardware knowledge. BSDL opcode alternatives
remain alternatives on reset rather than selecting an arbitrary encoding.
Optional TRST is handled from the already parsed `TAP_SCAN_RESET` role; an absent
pin is inactive, not an observed power-up reset. DR execution and child wiring
remain separate unfinished work.

### AccessLink connection precedence probes

IEEE 1687-2014 §6.4.17(f), printed p. 135, fixes interface ordering from TDI
toward TDO. The Tessent Shell Reference Manual v2023.4, *AccessLink*, describes
the synthesized TAP instance and top-level ports. Before implementing connection
precedence, eight self-constructed variants of the maintained AccessLink fixture
were elaborated in isolated **Tessent Shell 2026.2** processes under the authorized
`lianghuaxiao` account. No original EDA project files were edited.

| Probe | Observed 2026.2 result |
| --- | --- |
| Baseline | Elaborates and emits expanded ICL |
| Existing correctly typed TAP ports, TDO without Source | Reuses those ports and supplies TDO's TAP source |
| Existing TDO Source | Rejected during `read_icl`; TAP_SCAN_OUT must not have a Source statement |
| TDI declared as DataInPort | Rejected during `read_icl`; wrong port function |
| Explicit child ScanInPort InputPort binding | Rejected during elaboration, ICL98 |
| Explicit feedback binding on first member of a two-interface chain | Rejected during elaboration, ICL98 |
| Explicit child select/capture/shift/update bindings | Preserved in expanded ICL |
| Two listed child interfaces | First child feeds the second, and the second returns to the TAP |

The four accepted cases returned zero, printed the exact completion marker, and
produced fresh expanded ICL. The four rejected cases returned one and produced
no expanded ICL; the cause was checked in each console log. These are measured
elaboration rules, not evidence that a network with explicit disabled controls
can generate patterns. They informed the native serial assembly below; they are
not a native/vendor pattern-equivalence result.

Reproduce the inputs with
`python3 tests/src/probe_pattern_access_link_connections.py <fresh-directory>`.
The eight complete ICL/BSDL/Tcl pairs are staged from source-controlled,
self-constructed fixtures without storing duplicate module files in Git. The
stager rejects existing destinations; its safety checks run in `make check`.
Execute each `run.tcl` in its own fresh remote process/directory, never locally.
The commands use `patterns -ijtag / setup`; their legality was checked in the
v2023.4 *set_context*, *read_icl*, *set_current_design*, and *write_icl* topics.
Generated evidence is ignored under
`build/access-link-connections-evidence-20261026/`; the isolated remote directory
is `/data/workspaces/lianghuaxiao/test/tessent_ijtag/icl-toolsets-connections.h7xp5L`.

The native AccessLink binder now applies the observed conflict rules before
model construction: existing BSDL-named TAP ports must have the matching ICL
port kind, an existing TAP output cannot specify `Source`, and a selected
child scan input cannot already have an instance `InputPort` binding. Explicit
control bindings remain accepted. These checks borrow the shared AST and
preserve its source. They provide preflight for the serial assembly below;
controller event/control connections remain unfinished.
Absent optional BSDL TAP roles do not reserve unrelated ICL identifiers.

`ElaborateTapNetwork` now implements the probed serial topology for IJTAG client
interfaces: reuse/synthesize scalar BSDL TAP pins, connect TDI to the behavioral
datapath, select each child SI's predecessor by instruction opcode, return the
last child SO to that instruction's datapath route, and connect the TAP output
to TDO. The two-interface regression checks core→peer→TAP directly against the
retained probe's order; shared instruction routes deduplicate identical codes.
Explicit control bindings remain intact. No new Tessent process or pattern
comparison was run for this native implementation. Controller event wiring and
runtime association replay remain separate unfinished obligations.
