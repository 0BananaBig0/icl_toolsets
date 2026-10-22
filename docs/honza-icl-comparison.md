# Honza ICL comparison — 2026-10-22

Reference: https://github.com/Honza255/icl_parser at `333f8bc7a54964138ac14a34fde9e942c496436d`.
Only `src/icl_parser/icl.g4` is compared; the separate PDL grammar supplies no ICL tokens.

## Keywords and productions

75 multi-character ICL keyword/fixed-word spellings have explicit Flex rules. The six multi-character time-unit literals are handled by TIME/clock_period; the seventh unit `s` is also covered. No missing ICL keyword was found.

`AccessLink`, `AccessTogether`, `ActivePolarity`, `ActiveSignals`, `AddressPort`, `AddressValue`, `Alias`, `AllowBroadcastOnScanInterface`, `Attribute`, `BSDLEntity`, `CaptureEnPort`, `CaptureSource`, `Chain`, `ClockMux`, `ClockPort`, `DataInPort`, `DataMux`, `DataOutPort`, `DataRegister`, `DefaultLoadValue`, `DifferentialInvOf`, `Enable`, `Enum`, `FreqDivider`, `FreqMultiplier`, `InputPort`, `Instance`, `LocalParameter`, `LogicSignal`, `Module`, `NameSpace`, `Of`, `OneHotDataGroup`, `OneHotScanGroup`, `Parameter`, `Period`, `Port`, `ReadCallBack`, `ReadDataSource`, `ReadEnPort`, `RefEnum`, `ResetPort`, `ResetValue`, `STD_1149_1_2001`, `STD_1149_1_2013`, `ScanInPort`, `ScanInSource`, `ScanInterface`, `ScanMux`, `ScanOutPort`, `ScanRegister`, `SelectPort`, `SelectedBy`, `ShiftEnPort`, `Source`, `TCKPort`, `TMSPort`, `TRSTPort`, `ToCaptureEnPort`, `ToClockPort`, `ToIRSelectPort`, `ToResetPort`, `ToSelectPort`, `ToShiftEnPort`, `ToTCKPort`, `ToTMSPort`, `ToTRSTPort`, `ToUpdateEnPort`, `UpdateEnPort`, `UseNameSpace`, `WriteCallBack`, `WriteDataSource`, `WriteEnPort`, `WriteEnSource`, `iApplyEndState`

The table inventories all 268 named productions/fragments, including the two unused Generic fragments. Entries name shared implementation rules, not a second parser. Cardinality, context and recursive dependencies are covered in the per-block review and classified manifest.

| Honza production | Local matching path |
|---|---|
| `SCALAR_ID` | Flex ID; scalar_name |
| `pos_int` | Flex DECIMAL; integer_arg |
| `POS_INT` | Flex DECIMAL; integer_arg |
| `size` | Flex WIDTH; NumberLiteral |
| `UNKNOWN_DIGIT` | Flex BASED; NumberLiteral |
| `DEC_DIGIT` | Flex DECIMAL |
| `BIN_DIGIT` | Flex BASED |
| `HEX_DIGIT` | Flex BASED |
| `DEC_BASE` | Flex BASED |
| `BIN_BASE` | Flex BASED |
| `HEX_BASE` | Flex BASED |
| `UNSIZED_DEC_NUM` | Flex BASED; number; NumberLiteral |
| `UNSIZED_BIN_NUM` | Flex BASED; number; NumberLiteral |
| `UNSIZED_HEX_NUM` | Flex BASED; number; NumberLiteral |
| `sized_dec_num` | Flex BASED; number; NumberLiteral |
| `sized_bin_num` | Flex BASED; number; NumberLiteral |
| `sized_hex_num` | Flex BASED; number; NumberLiteral |
| `vector_id` | scalar_name strict_range |
| `index` | integer_expr |
| `rangex` | strict_range |
| `number` | number |
| `integer_expr` | integer_expr integer_arg (text/parentheses preserved; no evaluation) |
| `integer_expr_lvl1` | integer_expr integer_arg (text/parentheses preserved; no evaluation) |
| `integer_expr_lvl2` | integer_expr integer_arg (text/parentheses preserved; no evaluation) |
| `integer_expr_paren` | integer_expr integer_arg (text/parentheses preserved; no evaluation) |
| `integer_expr_arg` | integer_expr integer_arg (text/parentheses preserved; no evaluation) |
| `parameter_ref` | Flex PARAMREF; integer_arg |
| `unsized_number` | Flex BASED; number; NumberLiteral |
| `sized_number` | Flex BASED; number; NumberLiteral |
| `concat_number` | concat_number |
| `concat_number_list` | mux_codes |
| `hier_port` | signal_path signal_reference |
| `port_name` | scalar_name strict_range |
| `register_name` | scalar_name strict_range |
| `instance_name` | scalar_name |
| `namespace_name` | scalar_name |
| `module_name` | scalar_name |
| `reg_port_signal_id` | signal_reference |
| `signal` | signal |
| `unknown_signal` | data_signal |
| `reset_signal` | data_signal |
| `scan_signal` | data_signal |
| `data_signal` | data_signal |
| `clock_signal` | data_signal |
| `tck_signal` | data_signal |
| `tms_signal` | data_signal |
| `trst_signal` | data_signal |
| `shiftEn_signal` | data_signal |
| `captureEn_signal` | data_signal |
| `updateEn_signal` | data_signal |
| `concat_unknown_signal` | concat_data_signal |
| `concat_reset_signal` | concat_data_signal |
| `concat_scan_signal` | concat_data_signal |
| `concat_data_signal` | concat_data_signal |
| `concat_clock_signal` | concat_data_signal |
| `concat_tck_signal` | concat_data_signal |
| `concat_shiftEn_signal` | concat_data_signal |
| `concat_captureEn_signal` | concat_data_signal |
| `concat_updateEn_signal` | concat_data_signal |
| `concat_tms_signal` | concat_data_signal |
| `concat_trst_signal` | concat_data_signal |
| `icl_source` | file |
| `iclSource_items` | file |
| `nameSpace_def` | ns_stmt |
| `useNameSpace_def` | use_ns_stmt |
| `module_def` | module_def |
| `module_item` | module_item |
| `port_def` | checked_declaration |
| `scanInPort_def` | checked_declaration checked_declaration_item; CheckBlockItems |
| `scanInPort_name` | scalar_name strict_range |
| `scanOutPort_def` | checked_declaration checked_declaration_item; CheckBlockItems |
| `scanOutPort_name` | scalar_name strict_range |
| `scanOutPort_item` | checked_declaration checked_declaration_item; CheckBlockItems |
| `scanOutPort_source` | checked_declaration checked_declaration_item; CheckBlockItems |
| `scanOutPort_enable` | checked_declaration checked_declaration_item; CheckBlockItems |
| `shiftEnPort_def` | checked_declaration checked_declaration_item; CheckBlockItems |
| `shiftEnPort_name` | scalar_name strict_range |
| `captureEnPort_def` | checked_declaration checked_declaration_item; CheckBlockItems |
| `captureEnPort_name` | scalar_name strict_range |
| `updateEnPort_def` | checked_declaration checked_declaration_item; CheckBlockItems |
| `updateEnPort_name` | scalar_name strict_range |
| `dataInPort_def` | checked_declaration checked_declaration_item; CheckBlockItems |
| `dataInPort_name` | scalar_name strict_range |
| `dataInPort_item` | checked_declaration checked_declaration_item; CheckBlockItems |
| `dataInPort_refEnum` | checked_declaration checked_declaration_item; CheckBlockItems |
| `dataInPort_defaultLoadValue` | checked_declaration checked_declaration_item; CheckBlockItems |
| `dataOutPort_def` | checked_declaration checked_declaration_item; CheckBlockItems |
| `dataOutPort_name` | scalar_name strict_range |
| `dataOutPort_item` | checked_declaration checked_declaration_item; CheckBlockItems |
| `dataOutPort_source` | checked_declaration checked_declaration_item; CheckBlockItems |
| `dataOutPort_enable` | checked_declaration checked_declaration_item; CheckBlockItems |
| `dataOutPort_refEnum` | checked_declaration checked_declaration_item; CheckBlockItems |
| `toShiftEnPort_def` | checked_declaration checked_declaration_item; CheckBlockItems |
| `toShiftEnPort_name` | scalar_name strict_range |
| `toShiftEnPort_items` | checked_declaration checked_declaration_item; CheckBlockItems |
| `toShiftEnPort_source` | checked_declaration checked_declaration_item; CheckBlockItems |
| `toCaptureEnPort_def` | checked_declaration checked_declaration_item; CheckBlockItems |
| `toCaptureEnPort_name` | scalar_name strict_range |
| `toCaptureEnPort_items` | checked_declaration checked_declaration_item; CheckBlockItems |
| `toCaptureEnPort_source` | checked_declaration checked_declaration_item; CheckBlockItems |
| `toUpdateEnPort_def` | checked_declaration checked_declaration_item; CheckBlockItems |
| `toUpdateEnPort_name` | scalar_name strict_range |
| `toUpdateEnPort_items` | checked_declaration checked_declaration_item; CheckBlockItems |
| `toUpdateEnPort_source` | checked_declaration checked_declaration_item; CheckBlockItems |
| `selectPort_def` | checked_declaration checked_declaration_item; CheckBlockItems |
| `selectPort_name` | scalar_name strict_range |
| `toSelectPort_def` | checked_declaration checked_declaration_item; CheckBlockItems |
| `toSelectPort_name` | scalar_name strict_range |
| `toSelectPort_item` | checked_declaration checked_declaration_item; CheckBlockItems |
| `toSelectPort_source` | checked_declaration checked_declaration_item; CheckBlockItems |
| `resetPort_def` | checked_declaration checked_declaration_item; CheckBlockItems |
| `resetPort_name` | scalar_name strict_range |
| `resetPort_item` | checked_declaration checked_declaration_item; CheckBlockItems |
| `resetPort_polarity` | checked_declaration checked_declaration_item; CheckBlockItems |
| `toResetPort_def` | checked_declaration checked_declaration_item; CheckBlockItems |
| `toResetPort_name` | scalar_name strict_range |
| `toResetPort_item` | checked_declaration checked_declaration_item; CheckBlockItems |
| `toResetPort_source` | checked_declaration checked_declaration_item; CheckBlockItems |
| `toResetPort_polarity` | checked_declaration checked_declaration_item; CheckBlockItems |
| `tmsPort_def` | checked_declaration checked_declaration_item; CheckBlockItems |
| `tmsPort_name` | scalar_name strict_range |
| `toTmsPort_def` | checked_declaration checked_declaration_item; CheckBlockItems |
| `toTmsPort_name` | scalar_name strict_range |
| `toTmsPort_item` | checked_declaration checked_declaration_item; CheckBlockItems |
| `toTmsPort_source` | checked_declaration checked_declaration_item; CheckBlockItems |
| `toIRSelectPort_def` | checked_declaration checked_declaration_item; CheckBlockItems |
| `toIRSelectPort_name` | scalar_name strict_range |
| `tckPort_def` | checked_declaration checked_declaration_item; CheckBlockItems |
| `tckPort_name` | scalar_name strict_range |
| `toTckPort_def` | checked_declaration checked_declaration_item; CheckBlockItems |
| `toTckPort_name` | scalar_name strict_range |
| `clockPort_def` | checked_declaration checked_declaration_item; CheckBlockItems |
| `clockPort_name` | scalar_name strict_range |
| `clockPort_item` | checked_declaration checked_declaration_item; CheckBlockItems |
| `clockPort_diffPort` | checked_declaration checked_declaration_item; CheckBlockItems |
| `toClockPort_def` | checked_declaration checked_declaration_item; CheckBlockItems |
| `toClockPort_name` | scalar_name strict_range |
| `toClockPort_item` | checked_declaration checked_declaration_item; CheckBlockItems |
| `toClockPort_source` | checked_declaration checked_declaration_item; CheckBlockItems |
| `freqMultiplier_def` | frequency_keyword clock_factor |
| `freqDivider_def` | frequency_keyword clock_factor |
| `differentialInvOf_def` | checked_declaration_item |
| `period_def` | clock_period |
| `trstPort_def` | checked_declaration checked_declaration_item; CheckBlockItems |
| `trstPort_name` | scalar_name strict_range |
| `toTrstPort_def` | checked_declaration checked_declaration_item; CheckBlockItems |
| `toTrstPort_name` | scalar_name strict_range |
| `toTrstPort_item` | checked_declaration checked_declaration_item; CheckBlockItems |
| `toTrstPort_source` | checked_declaration checked_declaration_item; CheckBlockItems |
| `addressPort_def` | checked_declaration checked_declaration_item; CheckBlockItems |
| `addressPort_name` | scalar_name strict_range |
| `writeEnPort_def` | checked_declaration checked_declaration_item; CheckBlockItems |
| `writeEnPort_name` | scalar_name strict_range |
| `readEnPort_def` | checked_declaration checked_declaration_item; CheckBlockItems |
| `readEnPort_name` | scalar_name strict_range |
| `instance_def` | instance_declaration instance_property address_value |
| `instance_item` | instance_declaration instance_property address_value |
| `inputPort_connection` | instance_property |
| `allowBroadcast_def` | broadcast_names instance_property |
| `inputPort_name` | scalar_name strict_range |
| `inputPort_source` | concat_data_signal |
| `parameter_override` | parameter_override |
| `instance_addressValue` | instance_declaration instance_property address_value |
| `scanRegister_def` | checked_declaration checked_declaration_item; CheckBlockItems |
| `scanRegister_name` | checked_declaration checked_declaration_item; CheckBlockItems |
| `scanRegister_item` | checked_declaration checked_declaration_item; CheckBlockItems |
| `scanRegister_scanInSource` | checked_declaration checked_declaration_item; CheckBlockItems |
| `scanRegister_defaultLoadValue` | checked_declaration checked_declaration_item; CheckBlockItems |
| `scanRegister_captureSource` | checked_declaration checked_declaration_item; CheckBlockItems |
| `scanRegister_resetValue` | checked_declaration checked_declaration_item; CheckBlockItems |
| `scanRegister_refEnum` | checked_declaration checked_declaration_item; CheckBlockItems |
| `dataRegister_def` | data_register data_register_item callback_namespace callback_args; CheckBlockItems/CheckAddressContext |
| `dataRegister_name` | data_register data_register_item callback_namespace callback_args; CheckBlockItems/CheckAddressContext |
| `dataRegister_item` | data_register data_register_item callback_namespace callback_args; CheckBlockItems/CheckAddressContext |
| `dataRegister_type` | data_register data_register_item callback_namespace callback_args; CheckBlockItems/CheckAddressContext |
| `dataRegister_common` | data_register data_register_item callback_namespace callback_args; CheckBlockItems/CheckAddressContext |
| `dataRegister_resetValue` | data_register data_register_item callback_namespace callback_args; CheckBlockItems/CheckAddressContext |
| `dataRegister_defaultLoadValue` | data_register data_register_item callback_namespace callback_args; CheckBlockItems/CheckAddressContext |
| `dataRegister_refEnum` | data_register data_register_item callback_namespace callback_args; CheckBlockItems/CheckAddressContext |
| `dataRegister_selectable` | data_register data_register_item callback_namespace callback_args; CheckBlockItems/CheckAddressContext |
| `dataRegister_writeEnSource` | data_register data_register_item callback_namespace callback_args; CheckBlockItems/CheckAddressContext |
| `dataRegister_writeDataSource` | data_register data_register_item callback_namespace callback_args; CheckBlockItems/CheckAddressContext |
| `dataRegister_addressable` | data_register data_register_item callback_namespace callback_args; CheckBlockItems/CheckAddressContext |
| `dataRegister_addressValue` | data_register data_register_item callback_namespace callback_args; CheckBlockItems/CheckAddressContext |
| `dataRegister_readCallBack` | data_register data_register_item callback_namespace callback_args; CheckBlockItems/CheckAddressContext |
| `dataRegister_readCallBack_proc` | data_register data_register_item callback_namespace callback_args; CheckBlockItems/CheckAddressContext |
| `dataRegister_readDataSource` | data_register data_register_item callback_namespace callback_args; CheckBlockItems/CheckAddressContext |
| `dataRegister_writeCallBack` | data_register data_register_item callback_namespace callback_args; CheckBlockItems/CheckAddressContext |
| `iProc_namespace` | callback_namespace |
| `iProc_name` | callback_id |
| `iProc_args` | callback_arg |
| `sub_namespace` | callback_id |
| `ref_module_name` | callback_id |
| `logicSignal_def` | logic_body logic_expr/bit/compare/concat/unary/integer/atom |
| `logicSignal_name` | logic_body logic_expr/bit/compare/concat/unary/integer/atom |
| `logic_expr` | logic_body logic_expr/bit/compare/concat/unary/integer/atom |
| `logic_expr_lvl1` | logic_body logic_expr/bit/compare/concat/unary/integer/atom |
| `logic_expr_lvl2` | logic_body logic_expr/bit/compare/concat/unary/integer/atom |
| `logic_expr_lvl3` | logic_body logic_expr/bit/compare/concat/unary/integer/atom |
| `logic_expr_lvl4` | logic_body logic_expr/bit/compare/concat/unary/integer/atom |
| `logic_unary_expr` | logic_body logic_expr/bit/compare/concat/unary/integer/atom |
| `logic_expr_paren` | logic_body logic_expr/bit/compare/concat/unary/integer/atom |
| `logic_expr_arg` | logic_body logic_expr/bit/compare/concat/unary/integer/atom |
| `logic_expr_num_arg` | logic_body logic_expr/bit/compare/concat/unary/integer/atom |
| `scanMux_def` | strict_mux_keyword strict_mux_body mux_codes concat_data_signal |
| `scanMux_name` | strict_mux_keyword strict_mux_body mux_codes concat_data_signal |
| `scanMux_select` | strict_mux_keyword strict_mux_body mux_codes concat_data_signal |
| `scanMux_selection` | strict_mux_keyword strict_mux_body mux_codes concat_data_signal |
| `dataMux_def` | strict_mux_keyword strict_mux_body mux_codes concat_data_signal |
| `dataMux_name` | strict_mux_keyword strict_mux_body mux_codes concat_data_signal |
| `dataMux_select` | strict_mux_keyword strict_mux_body mux_codes concat_data_signal |
| `dataMux_selection` | strict_mux_keyword strict_mux_body mux_codes concat_data_signal |
| `clockMux_def` | strict_mux_keyword strict_mux_body mux_codes concat_data_signal |
| `clockMux_name` | strict_mux_keyword strict_mux_body mux_codes concat_data_signal |
| `clockMux_select` | strict_mux_keyword strict_mux_body mux_codes concat_data_signal |
| `clockMux_selection` | strict_mux_keyword strict_mux_body mux_codes concat_data_signal |
| `oneHotScanGroup_def` | one_hot_scan_body group_port |
| `oneHotScanGroup_name` | one_hot_scan_body group_port |
| `oneHotScanGroup_item` | one_hot_scan_body group_port |
| `oneHotDataGroup_def` | one_hot_data_body one_hot_data_item group_port |
| `oneHotDataGroup_name` | one_hot_data_body one_hot_data_item group_port |
| `oneHotDataGroup_item` | one_hot_data_body one_hot_data_item group_port |
| `oneHotDataGroup_portSource` | one_hot_data_body one_hot_data_item group_port |
| `scanInterface_def` | interface_body interface_item interface_property |
| `scanInterface_name` | interface_body interface_item interface_property |
| `scanInterface_item` | interface_body interface_item interface_property |
| `scanInterfacePort_def` | interface_body interface_item interface_property |
| `scanInterfaceChain_def` | chain chain_body interface_property; CheckChainName |
| `scanInterfaceChain_name` | chain chain_body interface_property; CheckChainName |
| `scanInterfaceChain_item` | chain chain_body interface_property; CheckChainName |
| `defaultLoad_def` | interface_property |
| `accessLink_def` | access_link |
| `AccessLinkGeneric_def` | access_link generic_link_type |
| `AccessLinkGeneric_block` | Flex GENERIC_LINK; generic_open |
| `AccessLinkGeneric_text` | Flex GENERIC_LINK |
| `accessLink_genericID` | generic_link_type |
| `accessLink1149_def` | access_link link_body |
| `accessLink_name` | scalar_name |
| `bsdlEntity_name` | scalar_name |
| `bsdl_instr_ref` | link_instruction |
| `bsdl_instr_name` | scalar_name |
| `bsdl_instr_selected_item` | link_selected_item |
| `accessLink1149_ActiveSignal_name` | link_active_item |
| `accessLink1149_ScanInterface_name` | link_interface_ref |
| `alias_def` | alias_signals alias_body alias_item; CheckBlockItems |
| `alias_name` | alias_signals alias_body alias_item; CheckBlockItems |
| `alias_item` | alias_signals alias_body alias_item; CheckBlockItems |
| `alias_iApplyEndState` | alias_signals alias_body alias_item; CheckBlockItems |
| `alias_refEnum` | alias_signals alias_body alias_item; CheckBlockItems |
| `concat_hier_data_signal` | alias_signals |
| `hier_data_signal` | alias_signal |
| `enum_def` | enum_body enum_item; CheckEnum/CheckEnumMember |
| `enum_name` | enum_body enum_item; CheckEnum/CheckEnumMember |
| `enum_item` | enum_body enum_item; CheckEnum/CheckEnumMember |
| `enum_symbol` | enum_body enum_item; CheckEnum/CheckEnumMember |
| `enum_value` | enum_body enum_item; CheckEnum/CheckEnumMember |
| `parameter_def` | parameter_override |
| `localParameter_def` | module_item |
| `parameter_name` | scalar_name |
| `parameter_value` | attribute_values |
| `concat_string` | attribute_values |
| `attribute_def` | strict_attribute |
| `attribute_name` | scalar_name |
| `attribute_value` | attribute_values |
| `STRING` | Flex STRING rule |
| `SPACE` | Flex whitespace rules |
| `ML_COMMENT` | Flex block comment rule |
| `SL_COMMENT` | Flex line comment rule |

## Adjudication of non-identical grammar

- IEEE normative clauses govern over this ANTLR rendition: keywords are valid scalar names; comments and strings follow §§6.3.4–9, including arbitrary escaped characters under the string prose.
- Integer/LogicSignal operator spellings and parentheses are preserved without evaluation; differences in parse associativity do not alter formatting or assert evaluated values.
- Generic AccessLink requires its balanced raw body in IEEE; Honza leaves its block fragment unused and its generic lexer rule overlaps the standard form. The local scanner preserves raw payload and parses the standard form separately.
- Module redefinition is legal (§6.4.5(a)); empty Module bodies appear in normative examples (§6.4.4). Empty documents are formatter/editor compatibility.
- Instance broadcast lists follow normative examples despite single-name BNF. Tessent address concatenation, mux Attributes, parameter clock factors and ToTCKPort Source are separately documented extensions.
- PortGroup, omitted ScanMux SelectedBy and ScanOutPort LaunchEdge are legacy LiU P1687 forms. Empty Alias bodies are user-corpus compatibility. Neither is claimed as new IEEE syntax.
- Reference acceptance alone does not check unique names, required context, exclusive properties, significant-bit truncation, parameter cycles/types or forbidden unknown end states. The per-case manifest cites why each reference-only negative remains rejected.

## Test-source inventory

All eight upstream `.icl` files have exact SHA256 matches locally, including the original Instruments file now retained as a negative. Its eight fixed-width overflows are corrected only in a derived `pass/ours/honza_instruments_corrected.icl`.

| Original input | Byte-identical local file | SHA256 |
|---|---|---|
| `tests/test_icls/Instruments.icl` | `tests/cases/fail/upstream/ijtag_Instruments.icl` | `738601f245f5572374896dcf5ba1110d0285219d497f45ab196a5d2d1b218bf6` |
| `tests/test_icls/benchmarks_conv/ICL/test_icl_syntax_5.icl` | `tests/cases/pass/upstream/ijtag_test_icl_syntax_5.icl` | `a0543c1dfcc98a237768920b2ec8bdc03ab8f3d4f8a0fc32b0ab5048cc1debec` |
| `tests/test_icls/benchmarks_conv/ICL/test_icl_syntax_6.icl` | `tests/cases/pass/upstream/ijtag_test_icl_syntax_6.icl` | `886c0c2e7160290bfdbde50aac9557bab3122466cdbca7220bd809b961272c2b` |
| `tests/test_icls/benchmarks_conv/ICL/test_icl_syntax_7.icl` | `tests/cases/pass/upstream/ijtag_test_icl_syntax_7.icl` | `03f906804d53ec6deb116f951aa4a3983f421d4f8975b17a13385e75898d26f4` |
| `tests/test_icls/scan_mux_001.icl` | `tests/cases/pass/upstream/ijtag_scan_mux_001.icl` | `b6a6c4dd6a2e6158deefee80d70a8477b4c43fae1c6e728bdbdac42e60fb47c6` |
| `tests/test_icls/scan_mux_002.icl` | `tests/cases/pass/upstream/ijtag_scan_mux_002.icl` | `4118abcb327ca079edc7208a475b7f93acc8b67734711a4b95878e964f59bb0f` |
| `tests/test_icls/test_icl_instance.icl` | `tests/cases/pass/upstream/ijtag_test_icl_instance.icl` | `44a29294b213fb12ea77049ee35536bbc5cbf3e0d370f90a23fe395af68d6435` |
| `tests/test_icls/test_icl_scan_register.icl` | `tests/cases/pass/upstream/ijtag_test_icl_scan_register.icl` | `434c7d3ce74ed834bdcf8b308da31c2b6da6ff9340e3adc032488e1469197048` |

All nine `tests/test_*.py` modules were inspected through their AST. None contains an embedded Module/ScanRegister source string; they load the ICL files/configured benchmark library. Their simulation, retargeting and numeric-object evaluation assertions are outside the formatter/LSP responsibility. `test_icl_number.py` has 33 literal constructor calls,30 distinct argument tuples: all30 are now separately represented under `tests/cases/upstream/lexical`, with source line and API-to-ICL normalization recorded. The original partial15-literal fixture remains as a representative aggregate. The upstream MIT notice is preserved at `tests/cases/upstream/LICENSE_Honza255`.

## Differential result

Command: `build/reference-venv/bin/python tests/antlr_diff.py --binary build/icl-format --ref-dir /tmp/icl-reference-honza-20261021`. Runtime: ANTLR4.7.2, Python3.14; a type-import alias only makes the generated `typing.io.TextIO` annotation importable, without altering grammar tables.

The adapter fills the token stream, counts lexer and parser errors, and requires EOF. Missing external dependencies exit77 (unrun), not0. Six direct adapter tests pass with the real reference. Final task8 run:2007 files,1625 agreements,253 local-only acceptances and129 reference-only acceptances (exit1 intentionally flags differences). All129 reference-only cases are local negatives with recorded grounds in `tests/cases/upstream/differential-adjudication.json`; none is a classified positive rejected locally. Local-only cases cover keyword identifiers, normative examples, supported extensions and the reference lexer/AccessLink gaps. The generated difference log is a local build artifact at `build/reference/differential-results.txt`, excluded from Git; the source-backed adjudication remains a maintained test expectation.


Hierarchical identifier whitespace and deep non-Alias paths are rejected under IEEE6.3.10.5, despite reference acceptance. Arbitrary string escapes follow §6.3.9 prose (the BNF only enumerates slash/quote); original spelling is preserved.

## Renewed audit after the MIT license change

Rechecked the pinned ICL grammar: all75 keyword/fixed-word spellings still have
explicit Flex rules; the remaining six multi-character words are time units in
TIME/clock_period (the single-character unit `s` is covered too). No PDL grammar
was used. The existing268-production matching inventory remains applicable;
no parser productions changed in this follow-up.

Real adapter tests pass6/6. The refreshed differential run over2035 inputs has
1642 agreements,258 local-only accepts and135 reference-only accepts (exit1
intentionally reports differences). Six additional reference-only inputs are
constructed negatives already added during the previous coverage pass: a newline
inside a hierarchical identifier (§6.3.10.5(a), printed67) and five LogicSignal
unknown operands outside a comparison numeric argument (§6.4.10(g), printed123).
All six were checked against those normative rules and added to the maintained
adjudication file. No classified positive is among the135 reference-only inputs.
New local-only cases exercise keyword identifiers and the previously documented
legacy PortGroup support. The raw log stays in ignored
`build/reference/renewed-differential-results.txt`.
