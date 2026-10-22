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
syntax/local-property checks. OneHot groups and ScanInterface/Chain also have verified syntax/local checks; subsequent blocks remain unreviewed; lexer changes must be rechecked in those contexts. Broader numeric evaluation, semantic
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


## OneHot groups and ScanInterface/Chain

IEEE §6.4.14–16, printed 127–133/PDF 141–147, compared individually with
Honza's matching productions and Tessent v2023.4 OneHotScanGroup,
OneHotDataGroup, ScanInterface and Chain topics.

OneHotScanGroup accepts a nonempty Port list. OneHotDataGroup accepts Port,
Instance and DataRegister items, sharing the same signal syntax without admitting
AccessLink or unrelated nested blocks. Register placement checks remain shared.
The Tessent OneHotDataGroup example documents concatenated DataRegister
AddressValue, extending the IEEE number-only form; this newly found evidence
supersedes the earlier restriction. Both its completed multi-level example and
an isolated concatenation fixture now pass.

ScanInterface and Chain have distinct, nonempty bodies. Their Port statement
requires a local scalar/vector name, without hierarchical paths, inversion or
concatenation. Attribute and numeric DefaultLoadValue properties are shared;
Chain cannot nest. Duplicate chain names are rejected within one interface,
while separate interfaces may reuse names. DefaultLoadValue is a singleton in
each body (Tessent describes an optional pair); it must be inside Chain whenever
chains are declared (§6.4.16(l)). Port function/width and topology checks require
symbol resolution and are not claimed here. Completed standard examples and all
six Tessent interface/chain examples are retained by source.

No OneHotScanInterface production was found in IEEE, Honza or Tessent. Its
unsupported declaration/keyword was removed; the word remains a valid identifier.
Old positives with nested/ranged Chain, generic Source, or DefaultLoadValue
outside a chain were corrected with negative counterparts. Original P2654 SIB
files use lowercase `port`, without a documented extension authorizing it; their
bytes remain in fail/p2654, with corrected derived positives under ours.
The PDL-separation test now exercises ICL iApplyEndState in its actual Alias
context rather than an illegal ScanInterface property, which has a negative test.

Focused cases: OneHotScanGroup 33, OneHotDataGroup 42, ScanInterface/Chain 68.
Initial failing cases were 22/33, 24/42 and 39/62, followed by the unsupported-keyword
reproduction. Full dynamic verification results follow below.

The coverage pass also exposed obsolete generic-body Instance/DataRegister alternatives. New negative Alias nesting cases reproduced their illegal acceptance before removal; keyword-as-identifier fixtures cover the surviving names.

Task 5 full verification: `make check-dynamic BISON=/usr/bin/bison` exited 0 with 1256 classified cases, 508 regression assertions, 730 block cases, 12 golden,71 CLI and62 existing LSP checks, plus unit/whitespace/fuzz/separation checks. Current source-line grammar trace is not the final honest action/branch coverage report.


## Instance and AccessLink

IEEE §6.4.7 (printed 110–112/PDF 124–126) and §6.4.17 (printed
133–135/PDF 147–149), with Tessent v2023.4 Instance and AccessLink topics.
Each block was tested independently before implementation.

Instance now has dedicated input connection, parameter override, Attribute,
broadcast and AddressValue productions. Names can be keywords and comments may
separate tokens. A module reference has at most one namespace qualifier, including
the explicit global `::Module` form. The documented Origen lowercase `of` extension
is retained only for Instance. Tessent examples document concatenated addresses,
which are shared with DataRegister. The parent AST construction rejects explicit
addressed instances outside OneHotDataGroup. Missing address properties in otherwise
incomplete declarations, connectivity completeness, overlapping resolved input ranges,
parameter binding and hierarchy cycles remain semantic/elaboration checks.

Broadcast interface lists are accepted because both the normative generic example
and Inst4 example explicitly use commas despite the single-name BNF. This is a
source-backed discrepancy ruling; Tessent documents broadcasting as unsupported.
Standard examples and both vendor Instance examples are completed; the second
vendor example's stray `7` in a WriteEnPort declaration is removed explicitly.
Old invented OutputPort entries, generic properties and deep namespace chains
are corrected or retained as negative fixtures.

AccessLink has separate standard 1149.1 and Generic entry points. The standard
form requires one leading BSDLEntity, one or more named instructions, and nonempty
ScanInterface/ActiveSignals lists. Interface references allow an instance with an
optional single interface component. Active signals are local scalar/vector names.
Duplicate instruction names and access-link names are rejected at their proper
scopes. The grammar permits ActiveSignals-only instructions; the vendor's stronger
ScanInterface requirement is not imposed on IEEE inputs.

Generic controller contents have implementation-defined semantics. A separate
Flex start condition collects balanced nested braces, strings and comments in
linear time and preserves the payload byte-for-byte in the shared AST. The
formatter changes the outer ICL layout only. This is opaque ICL content, not a
second PDL parser or shared PDL token set. Unexpected EOF, unterminated quotes/
comments and nested block comments fail, and the scanner resets before the next
file. A golden fixture checks payload whitespace exactly; the whitespace suite
checks Generic variants for content preservation and idempotence rather than
requiring their intentionally preserved payload whitespace to become identical.
Standard 1149.1 input continues to receive normal nested formatting.

Honza's Generic AccessLink rule does not consume its body (its raw block is an
unused lexer fragment). The normative clause explicitly defines that body, so the
old upstream free-text rejection fixture is now a positive. The generic type
`STD_1149_1` remains an ordinary implementation-defined identifier; legacy payloads
such as BSDL_Entity/ScanPath are preserved there, but are rejected when incorrectly
attached to the standard `STD_1149_1_2001`/`2013` types.

Initial failures: Instance32/63, AccessLink41/65. Final focused totals include
63 Instance and73 AccessLink cases, plus the exact-output and scanner-recovery
checks. Final full dynamic results follow below.

Task 6 full verification: `make check-dynamic BISON=/usr/bin/bison` exited 0 with 1392 classified cases,530 regression assertions,730 block-review cases,13 golden,74 CLI and62 existing LSP checks, plus unit/whitespace/fuzz/separation checks. Current trace:368/368 grammar source lines and123/123 lexer rules over1657 files. Removed obsolete generic assignment/unnamed-block paths after negative Alias dependency tests; retained comments, legal numeric concatenation and all keyword identifiers in persisted fixtures.

## Task 7 — Alias, Parameter, Attribute and module structure

Sources: IEEE1687-2014 §§6.5.2,6.5.4–5 (printed136–140/PDF150–154),
§§6.4.2–5 (printed73–78/PDF87–92), and lexical dependencies6.3.4–8.
Tessent Shell Reference v2023.4 topics Command_Alias_id2ff00152.html,
Command_Parameter_id6095dafa.html, Command_Localparameter_idc06e0fee.html,
Command_Attribute_id811c3f4b.html, Command_Module_idf9751e45.html,
Command_Namespace_id86c09529.html and Command_Usenamespace_id12b11dc1.html.
Honza ICL grammar was compared for each corresponding production. Tessent explicitly
does not implement NameSpace/UseNameSpace; IEEE governs those forms.

Alias now has dedicated signal-reference concatenation and property rules. Numeric,
string, parameter and arbitrary-expression sources are rejected; one inversion per
signal is supported. RefEnum is a singleton. AccessTogether and iApplyEndState are
repeatable according to the normative grammar; literal unknown digits in end-state
values are rejected. Empty braces are accepted solely as observed user merged-ICL
compatibility, not as IEEE grammar or a Tessent-manual claim. Both real user merged
files retain their original hashes. The Alias vendor example's overflowing 4'haa
was corrected to 4'ha in the derived fixture and recorded in its provenance.

Parameter/LocalParameter share typed concatenation rules with Attribute. Named AST
metadata survives comments between keyword/name/value tokens. Direct-child parameter
names are unique across Parameter and LocalParameter, and Attribute names are unique
within each containing element. Repeated distinct attributes remain legal; five old
same-name repeatability cases were corrected with their original text kept as negatives.
Local default parameter types and cycles use a linear O(V+E) dependency traversal,
including forward references and parameter-sized literals. Unsized numeric concatenation
and statically resolved mixed string/number concatenation are rejected. Instance overrides
resolve references in their enclosing scope, so `Parameter Size = $Size` is not
erroneously treated as a self-cycle. Missing modules, instance elaboration, resolved
widths and full hardware consistency are not asserted by these local checks.

NameSpace is file-level only; UseNameSpace is legal at file/module scope. Both support
keyword identifiers, comments and an omitted name for root scope. Module names likewise
support keywords/comments. Module-level AllowBroadcastOnScanInterface was removed;
it belongs in Instance. Empty Module bodies remain accepted because the normative
§6.4.4 examples explicitly use them despite the module_item+ grammar. Redefinition
is also allowed by §6.4.5(a); this formatter preserves source order and both definitions.
Empty documents remain supported for editor/formatter use.

The arbitrary fallback bodies and generic expressions for Reg, Field and Connection
were removed, along with their special tokens. No corresponding declaration exists in
IEEE's exhaustive module-item list, Honza, or Tessent's complete Module statement list.
The alleged tutorial Connection example lacks a retrievable citation and now remains
byte-preserved as a negative under fail/published. Other invalid synthetic positives
were preserved under fail/ours or the classified corpus. PortGroup is retained as a
LiU P1687 Listing8 compatibility form, not as Tessent or IEEE2014 syntax. Its comma
list now rejects leading/doubled/trailing/missing commas and comment-only lists.

A dependency regression exposed legal whitespace/comments between size and based
number tokens (6.3.4(11–12),6.3.5). Flex collects the combined literal with maximal
munch, avoiding a Bison ambiguity with adjacent numeric callback arguments. Numeric
metadata skips only its token's trivia, while formatting keeps the original spelling
and comments. Zero widths, nested comments and two independent sizes still fail.
The normative no-whitespace rule between quote and base remains enforced.

Task7 verification: make check-dynamic BISON=/usr/bin/bison exited0:1602 classified
cases,568 regression assertions,714 remaining valid block-review cases (invalid
legacy cases retained as classified negatives),13 golden,74 CLI,62 existing LSP;
unit,whitespace,fuzz,separation checks green. Current trace:332/332 grammar source
lines and118/118 lexer rules over1866 files. This is not yet production-ID or
handwritten action/branch coverage (Task10). A follow-up PortGroup test reproduced
and fixed comma loss when a comment preceded the comma; token order is preserved.

## Task8: cross-cutting lexical rules and upstream comparison

IEEE1687-2014 §§6.3.5–10 (printed61–68/PDF75–82) reviewed against scanner and
shared literal metadata. Corrected fixed-width nonzero-bit truncation, whitespace
alphabet, literal-base trivia, embedded NUL handling and invalid-byte diagnostics.
Strings preserve arbitrary escapes per normative prose; BNF's narrower examples
are not treated as a prohibition. Hierarchical dots require adjacent identifiers;
paths deeper than one instance are restricted to Alias (§6.3.10.5). Exact scanner
line/column accounting is now centralized, a prerequisite for this check and LSP.
All original user merged files still pass unchanged.

Honza comparison is in `honza-icl-comparison.md`;268 named grammar entries and75
multi-character keywords are mapped, all8 original files retained and all30
numeric API inputs transcribed. The ANTLR adapter counts lexer errors and EOF;
its6 real-reference tests pass. Differential2007 inputs:1625 agree,253 local-only,
129 reference-only; all reference-only cases are expected local negatives with
recorded normative grounds. This is differential evidence, not hardware DRC.

Full `make check-dynamic BISON=/usr/bin/bison` passed:1743 classified cases,
570 regression assertions,714 block cases,13 golden,74 CLI,62 LSP; unit,
whitespace, fuzz and language separation checks passed. The system-Python reference
adapter suite skipped5 dependency tests; the separate venv run executed all6.
Trace reports333/333 grammar source lines and118/118 scanner rules over2007 files;
production-ID/action/branch accounting remains task10. Numeric parameter-width
elaboration and known-target concatenation width inference remain semantic limits.
