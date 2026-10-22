# ICL conformance and language-service review

Baseline: `44fc3db`. Reference: IEEE 1687-2014, normative Clause 6.
External comparison: Honza255/icl_parser `333f8bc7a54964138ac14a34fde9e942c496436d`.
ICL and PDL remain separate languages; this project has no PDL parser.

## Enum — syntax and local checks verified

Evidence: §6.5.3, printed 137–138 / PDF 151–152; dependencies §§6.3.7–8,
6.3.10.2, 6.3.12. Tessent Shell Reference v2023.4,
`tshell_ref/topics/Command_Enum_id3fd7e051.html`.

- Replaced the generic attribute body with nonempty `enum_item` productions.
- Enum symbols accept keywords, including Instance and AccessLink.
- Numeric values accept the specified concatenation, inversion and recursive
  unsigned integer arithmetic; strings, signal identifiers and boolean operators
  are rejected. Inter-token and standalone comments survive formatting.
- The lexer distinguishes decimal integers, based literals and time values;
  validates base-specific digits; removes unsupported signed/octal/z/? spellings;
  permits decimal separators, parameter widths and whitespace after a base.
- Detects duplicate member names, duplicate enum names within a module, and
  unequal statically known member widths, including sized concatenations.
  Widths depending on parameters or consuming objects require elaboration and
  are not claimed as validated. Widths beyond size_t are preserved without local
  width comparison. This is a syntax tool, not a complete numerical elaborator.
- `tests/cases/conformance.json` records provenance and acceptance independently
  of the directory name. The runner checks acceptance, content and idempotence.
- Original baseline failed 24/39 initial cases, then 5 additional comment/name
  cases and 2 fixed-width cases reproduced further defects. Corrected binary:
  **48/48 conformance cases pass**.
- Moved the fabricated nested-enum positive to the rejection corpus (§6.5.3);
  replaced the signed Verilog literal in the old positive block review. No merged
  Tessent fixture was modified. Tessent's Enum example contains overflowing
  constants; the derived test explicitly records corrections to those values.

## Open work

Enum, all 26 port blocks, registers, LogicSignal and the three mux blocks have
syntax/local-property checks. Subsequent blocks remain unreviewed; lexer changes must be rechecked in those contexts. Broader numeric evaluation, semantic
resolution, complete LSP services, coverage accounting, performance measurement,
whole-change review and final installation remain pending; no full-conformance
claim is made by this record.

## DataOutPort — syntax and local cardinality verified

IEEE §6.4.6.7 printed 88/PDF 102; Tessent Shell Reference v2023.4
`Command_Dataoutport_ide13afa16.html`. Honza `dataOutPort_def` and four item
alternatives agree with the normative grammar. Empty braces are accepted as
IEEE specifies even though Tessent prose recommends the semicolon form.

Added dedicated port productions, unsigned expression ranges, qualified signal
references, optional inversion, numeric sources, typed attribute concatenation,
and unique Source/Enable/RefEnum checks. Keywords work as names and references.
Dependencies reuse the same numeric and comment productions as Enum. String
scanning now supports literal/escaped newlines (§6.3.9), and nested block
comments are rejected (§6.3.7).

Initial result: 27/44 failures; then 3/3 lexical edge failures. Corrected result:
47/47 DataOutPort, 95/95 cumulative cases, 494 regression assertions and 730
legacy block cases pass. Corrected two derived positive fixtures and one inline
case claiming Enable lists were legal; the negative cases preserve that defect.
Cross-object signal widths/types and instance elaboration are not validated here.

## DataInPort and ScanInPort

DataInPort (§6.4.6.6, printed 86–87) was checked separately: 8/16 initial failures, then 19/19 corrected cases including IEEE/Tessent examples. Only Attribute, RefEnum and DefaultLoadValue are admitted; RefEnum is a singleton. ScanInPort (§6.4.6.1, printed 82) then reproduced 16/22 failures and passed 22/22 after restricting its body to Attribute. Cumulative 136 cases, 494 regression assertions and 730 legacy block cases passed. Parameter reference W in an old range test was corrected to $W with a rejection fixture for the original error.


## Remaining port blocks — syntax and local properties

Reviewed each port independently against IEEE 1687-2014 §6.4.6.1–26,
printed 82–110 (PDF 96–124), with failing fixtures before each correction.
Every port uses the same header, range, comment, attribute and signal grammar.
The shared driver enforces the permitted properties and stated singleton rules.
Empty bodies are rejected for ToSelectPort, ToResetPort, ToClockPort and
ToTRSTPort; comments alone do not satisfy their required item.

ToClockPort additionally rejects mutually exclusive properties and frequency
factors without Source. Period accepts a decimal integer with any of the seven
IEEE units, joined or separated, including inter-token comments. ActivePolarity
requires the exact literal 0 or 1. Both merged Tessent fixtures are unchanged.

Evidence-backed compatibility rules:

- ScanOutPort LaunchEdge Falling: Zadegan et al., “Reusing and Retargeting
  On-Chip Instrument Access Procedures in IEEE P1687”, Listings 3 and 8,
  PDF pages 5 and 9, https://www.ida.liu.se/labs/eslab/publications/pap/db/farza_DTM12.pdf.
  No other LaunchEdge value or port placement is inferred from this evidence.
- ToTCKPort Source: Tessent Shell Reference v2023.4,
  `tshell_ref/topics/Command_Totckport_idec26fb0a.html`, explicitly labeled a
  property absent from IEEE 1687-2014. Source remains a singleton.
- Parameterized FreqMultiplier/FreqDivider: Tessent v2023.4
  `Command_Toclockport_ide389c67c.html`, clockGen example. Arbitrary expressions
  and parameterized Period are not admitted without supporting evidence.
- ToTMSPort Source singleton: Tessent v2023.4
  `Command_Totmsport_id112d7e41.html`. The IEEE clause does not explicitly state
  that cardinality. ResetPort ActivePolarity and ClockPort DifferentialInvOf
  remain repeatable because their clauses give no singleton restriction.

Honza `icl.g4` at 333f8bc7a54964138ac14a34fde9e942c496436d defines all
26 port keywords; all are matched here. Its ToCaptureEnPort and ToUpdateEnPort
Source alternatives use a single signal whereas normative §§6.4.6.9–10 allow
concatenation. Classified standard fixtures preserve that broader legal syntax.
Its other port-body alternatives match the normative shapes reviewed here;
its grammar alone does not enforce the local cardinality/exclusivity rules.

Removed the unsupported ToDoPort keyword and the last generic port-body fallback.
ToDoPort remains usable as an ordinary identifier. No matching port declaration
was found in IEEE, Honza or the local Tessent Shell Reference topics. Old
fabricated positives with illegal empty bodies, property combinations, repeated
Period or unprefixed range parameters were corrected, with negative counterparts
in the classified corpus. No hardware elaboration, signal type/width resolution,
retargeting or full semantic DRC is claimed by these checks.

Per-block classified fixtures (each positive also checks content and idempotence):

| Block | Cases |
| --- | ---: |
| enum | 48 |
| data_out_port | 47 |
| data_in_port | 19 |
| scan_in_port | 22 |
| scan_out_port | 29 |
| shift_en_port | 22 |
| capture_en_port | 22 |
| update_en_port | 22 |
| to_shift_en_port | 23 |
| to_capture_en_port | 25 |
| to_update_en_port | 24 |
| select_port | 22 |
| to_select_port | 24 |
| reset_port | 29 |
| to_reset_port | 29 |
| tms_port | 22 |
| to_tms_port | 23 |
| tck_port | 22 |
| to_tck_port | 23 |
| clock_port | 27 |
| to_clock_port | 70 |
| trst_port | 22 |
| to_trst_port | 24 |
| to_ir_select_port | 22 |
| address_port | 22 |
| write_en_port | 22 |
| read_en_port | 22 |
| port_inventory | 6 |

Verification: `make check-dynamic BISON=/usr/bin/bison` exited 0: 734 classified conformance cases, 494 regression assertions, 730 block-review cases, 12 golden cases, 71 CLI checks and 62 existing LSP checks, plus unit/whitespace/fuzz/separation checks. Current trace reader reports 261/261 grammar source lines and 112/112 lexer rules over 981 files; distinct production-ID and handwritten branch accounting remain task 10.


## ScanRegister and DataRegister

IEEE §6.4.8 (printed 113–116/PDF 127–130) and §6.4.9 (printed
117–120/PDF 131–134), compared with Honza's corresponding productions and
Tessent v2023.4 `Command_Scanregister_idc6d7ce0a.html` /
`Command_Dataregister_iddd11b495.html`.

ScanRegister shares the strict declaration/range/signal/attribute machinery with
ports, adding its own permitted properties and singleton counts. ScanInSource
is one signal; CaptureSource is a concatenation; ResetValue and DefaultLoadValue
are numeric concatenations or one enum symbol. Arbitrary members and nested
blocks are rejected. The explicit semicolon and empty-body grammar forms remain
accepted: missing ScanInSource (semantic rule b) is an incomplete-model issue,
not grounds for deleting those syntax alternatives. Resolving signal and enum
references, widths and reset/default value compatibility still needs elaboration.

DataRegister has a distinct body entry point, also used for nested declarations.
Its common properties reuse the same grammar and validation as ScanRegister.
Selectable/addressable/read-callback/write-callback properties are mutually
exclusive; non-Attribute properties are singletons. Empty braces are rejected.
The shared AST construction checks rule e: explicit addressable declarations
belong inside OneHotDataGroup, and other explicit access types belong outside.
Declarations without an access type remain syntactically incomplete rather than
being assigned an invented type.

Callback syntax is IEEE ICL: namespace, then a separate procedure name, then
zero or more numeric/string/parameter/<D>/<R> arguments. Namespace qualification
has the exact depth and parameter positions specified in §6.4.9; it is not an
unbounded `::` sequence. Tessent v2023.4 explicitly says callback functionality
is unsupported, so no vendor shorthand was inferred. The lexer remains ICL-only.
The normative WriteEnSource production permits an extra inversion before a
data_signal, including two consecutive `~` operators; this is tested explicitly.

Source examples include both Tessent ScanRegister examples (with the sensor's
stray semicolon before a ScanOutPort body removed), its DataRegister example,
and completed IEEE selectable/addressable/callback examples. The IEEE callback
example's unquoted string parameter M3 is quoted per §6.5.4, and its missing
instance is supplied. Former synthetic positives with missing callback names,
mixed access types, Selectable/Addressable blocks, unsupported members, nested
indices or string ResetValue are now corrected or classified as negatives.

Register-stage verification: `make check-dynamic BISON=/usr/bin/bison` exited 0 with 886 classified cases (59 ScanRegister, 93 DataRegister/dependencies), 494 regression assertions, 730 legacy block cases and all remaining dynamic recipes passing. Trace reader: 291/291 grammar source lines, 112/112 lexer rules over 1133 files; production-ID and handwritten branch accounting still pending. Removed obsolete generic namespace/iProc-marker value alternatives and unproven bare Reg/Field declarations after reproducing their illegal acceptance. All keyword-name checks now use actual Attribute syntax, including Instance and AccessLink, with no exclusion list.


## LogicSignal and multiplexers

Reviewed IEEE §6.4.10–13, printed 122–127/PDF 136–141, and Tessent Shell
Reference v2023.4 LogicSignal/ScanMux/DataMux/ClockMux topics. Honza's four
corresponding productions have matching operator/selection syntax; its grammar
also omits the Tessent mux Attribute extension.

LogicSignal now has one expression, layered Boolean/bitwise/reduction/comparison/
concatenation/unary/unsigned arithmetic productions, recursive parentheses,
keyword references and comments. Comparisons require a numeric concatenation or
scalar enum symbol on the right. Unknown digits are allowed only in that right
operand. Strings, arbitrary properties, signal arithmetic and unsupported
operators are rejected. Although two semantic rules mention a ternary operator,
neither the normative operator inventory nor the normative grammar defines it;
no ternary syntax is invented. Prose and BNF precedence differ; the formatter
preserves token order and explicit parentheses without evaluation or regrouping.
Widths and enum binding still require semantic elaboration.

Each mux block was verified before moving to the next. All share a numeric
selection-list grammar (`|` between concatenations) and signal concatenation
syntax. At least one actual selection is required. Attributes are explicitly a
Tessent extension; their order and the selection priority order are preserved.
The old arbitrary assignments, bare-source and empty-table alternatives were
removed. The documented pre-standard ScanMux form with a scalar identifier
selector remains; omitted SelectedBy is rejected for DataMux and ClockMux.
The LiU paper could not be downloaded again (DNS failed even with escalation);
this compatibility decision relies on the earlier verified Listings 3 and 8.

Completed standard examples and normalized Tessent chip2 examples are classified
by source. The two genuine merged files retain their original checksums.
The constructed `tessent_style.icl` was incorrectly pinned as a vendor positive:
its original bytes and checksum now remain in the rejection corpus because it
contains `LogicSignal { Source TCK; }`. A corrected derived positive is classified
under ours. Old invented bare ClockMux inputs likewise have negative counterparts.

Focused results: LogicSignal 85 cases, ScanMux 48, DataMux 47 and ClockMux 47;
1113 cumulative classified fixtures. The initial mux suites reproduced 27/48,
28/47 and 28/47 failures respectively before correction. Final full-suite results
are recorded below after the dynamic verification.

Task 4 full verification: `make check-dynamic BISON=/usr/bin/bison` exited 0; 1113 classified cases, 500 regression assertions, 730 block-review cases, 12 golden, 71 CLI, 62 existing LSP checks and remaining dynamic recipes pass. Removed the leftover generic bitwise-operator branch after three new Parameter negatives reproduced illegal acceptance. Current trace accounting is still source-line-based, pending task 10.
