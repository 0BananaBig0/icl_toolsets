# Where the test cases come from

Every file under `tests/cases/` sits in a directory naming where it came from. `pass/` files must format cleanly, idempotently and without losing a character; `fail/` files must be rejected with exit code 2. Both are run through stdin *and* through a real file.


## pass/standard — IEEE 1687-2014 — the standard itself

Clause and appendix text: the four Alias examples of §6.5.2, the spellings from Appendix E, the accessLink1149_def BNF with its two examples, the time units, and the 2013-revision AccessLink target.


| file | constructs it exercises |
|---|---|
| `accesslink_1149_bnf.icl` | Module, ScanInterface, AccessLink, Port, BSDL_Entity, BSDLEntity, ActiveSignals, STD_1149_1_2001, STD_1149_1_2013 |
| `allow_broadcast_on_scaninterface.icl` | Module, ScanInterface, Port, AllowBroadcastOnScanInterface |
| `standard_alias_examples.icl` | Module, Instance, InputPort, ScanInPort, DataInPort, DataOutPort, ScanRegister, DataRegister, Alias… |
| `standard_regex_samples.icl` | NameSpace, UseNameSpace, LocalParameter, Parameter, Attribute, Module, Instance, InputPort, DataInPort… |
| `std_tap_names_as_attribute_names.icl` | Module, Instance, AccessLink, STD_1149_1_2001, STD_1149_1_2013 |
| `time_units_and_2013_accesslink.icl` | Module, ToClockPort, AccessLink, Period, BSDL_Entity, STD_1149_1_2001, STD_1149_1_2013 |

6 files.

## pass/tessent — Tessent — Siemens EDA

Real tool output supplied by the user (Tessent Shell 2024.2, and two merged files), plus fragments from the Tessent IJTAG User's Manual completed into whole files.


| file | constructs it exercises |
|---|---|
| `tessent_chip_top.icl` | Attribute, Module, Instance, InputPort, ScanInPort, ScanOutPort, DataInPort, DataOutPort, ShiftEnPort… |
| `tessent_connection_rule_option.icl` | Attribute, Module, Instance, InputPort, ScanInPort, ScanOutPort, DataInPort, DataOutPort, ScanRegister… |
| `tessent_merged_icl.icl` | Attribute, Module, Instance, InputPort, ScanInPort, ScanOutPort, DataInPort, DataOutPort, ShiftEnPort… |
| `tessent_merged_icl_1.icl` | Attribute, Module, Instance, InputPort, ScanInPort, ScanOutPort, DataInPort, DataOutPort, ShiftEnPort… |
| `tessent_parameterized_module.icl` | Parameter, Module, Instance, InputPort, DataInPort, DataOutPort, ScanRegister, Source, ScanInSource |
| `tessent_style.icl` | Parameter, Attribute, Module, Instance, InputPort, ScanInPort, ScanOutPort, DataInPort, DataOutPort… |
| `wrapped_pmm_ip.icl` | Attribute, Module, ScanInPort, ScanOutPort, ShiftEnPort, CaptureEnPort, UpdateEnPort, SelectPort, ResetPort… |

7 files.

## pass/benchmark — IJTAG-Ecosystem/Benchmarks — A Suite of IEEE 1687 Benchmark Networks

26 files, five categories (Basic, Classic, ITC02, Standard, Advanced) plus Instruments / NetworkStructs / ScanCell. Verified byte-identical to the upstream repository by content hash.


| file | constructs it exercises |
|---|---|
| `Advanced_FlexScan.icl` | Attribute, Module, Instance, InputPort, ScanInPort, ScanOutPort, ShiftEnPort, CaptureEnPort, UpdateEnPort… |
| `Advanced_TrapOrFlap.icl` | Parameter, Attribute, Module, Instance, InputPort, ScanInPort, ScanOutPort, ShiftEnPort, CaptureEnPort… |
| `Basic_BasicSCB.icl` | Parameter, Attribute, Module, Instance, InputPort, ScanInPort, ScanOutPort, ShiftEnPort, CaptureEnPort… |
| `Basic_Mingle.icl` | Parameter, Attribute, Module, Instance, InputPort, ScanInPort, ScanOutPort, ShiftEnPort, CaptureEnPort… |
| `Basic_TreeBalanced.icl` | Parameter, Attribute, Module, Instance, InputPort, ScanInPort, ScanOutPort, DataInPort, ShiftEnPort… |
| `Basic_TreeFlat.icl` | Parameter, Attribute, Module, Instance, InputPort, ScanInPort, ScanOutPort, ShiftEnPort, CaptureEnPort… |
| `Basic_TreeFlat_Ex.icl` | Parameter, Attribute, Module, Instance, InputPort, ScanInPort, ScanOutPort, DataInPort, ShiftEnPort… |
| `Basic_TreeUnbalanced.icl` | Parameter, Attribute, Module, Instance, InputPort, ScanInPort, ScanOutPort, DataInPort, ShiftEnPort… |
| `Classic_a586710.icl` | NameSpace, UseNameSpace, Parameter, Attribute, Module, Instance, InputPort, ScanInPort, ScanOutPort… |
| `Classic_p22810.icl` | NameSpace, UseNameSpace, Parameter, Attribute, Module, Instance, InputPort, ScanInPort, ScanOutPort… |
| `Classic_p34392.icl` | NameSpace, UseNameSpace, Parameter, Attribute, Module, Instance, InputPort, ScanInPort, ScanOutPort… |
| `Classic_p93791.icl` | NameSpace, UseNameSpace, Parameter, Attribute, Module, Instance, InputPort, ScanInPort, ScanOutPort… |
| `Classic_q12710.icl` | NameSpace, UseNameSpace, Parameter, Attribute, Module, Instance, InputPort, ScanInPort, ScanOutPort… |
| `Classic_t512505.icl` | NameSpace, UseNameSpace, Parameter, Attribute, Module, Instance, InputPort, ScanInPort, ScanOutPort… |
| `ITC02_A586710.icl` | NameSpace, UseNameSpace, Parameter, Attribute, Module, Instance, InputPort, ScanInPort, ScanOutPort… |
| `ITC02_EmptyModule.icl` | Parameter, Attribute, Module, Instance, InputPort, ScanInPort, ScanOutPort, ShiftEnPort, CaptureEnPort… |
| `ITC02_G1023.icl` | NameSpace, UseNameSpace, Parameter, Attribute, Module, Instance, InputPort, ScanInPort, ScanOutPort… |
| `ITC02_H953.icl` | NameSpace, UseNameSpace, Parameter, Attribute, Module, Instance, InputPort, ScanInPort, ScanOutPort… |
| `Instruments.icl` | Parameter, Attribute, Module, Instance, InputPort, ScanInPort, ScanOutPort, DataInPort, DataOutPort… |
| `NetworkStructs.icl` | Parameter, Attribute, Module, ScanInPort, ScanOutPort, DataOutPort, ShiftEnPort, CaptureEnPort, UpdateEnPort… |
| `ScanCell.icl` | Parameter, Attribute, Module, Instance, InputPort, ScanInPort, ScanOutPort, ShiftEnPort, CaptureEnPort… |
| `Standard_CAD.icl` | Attribute, Module, Instance, InputPort, ScanInPort, ScanOutPort, DataInPort, DataOutPort, ShiftEnPort… |
| `Standard_Kernel.icl` | Parameter, Attribute, Module, Instance, InputPort, ScanInPort, ScanOutPort, DataInPort, DataOutPort… |
| `Standard_MultiCoreAccessLink.icl` | Attribute, Module, Instance, InputPort, ScanInPort, ScanOutPort, DataInPort, ShiftEnPort, CaptureEnPort… |
| `Standard_MultiTAP.icl` | Parameter, Attribute, Module, Instance, InputPort, ScanInPort, ScanOutPort, DataInPort, DataOutPort… |
| `Standard_SOC_DAP.icl` | Attribute, Module, Instance, InputPort, ScanInPort, ScanOutPort, ShiftEnPort, CaptureEnPort, UpdateEnPort… |

26 files.

## pass/upstream — Honza255/icl_parser — an ICL parser built on an ANTLR grammar

Its eight test ICL files, plus the number literal spellings that its test_icl_number.py feeds to IclNumber, transcribed into how they look in a file. Instruments.icl carries code from IEEE 1687-2014 Appendix E.


| file | constructs it exercises |
|---|---|
| `ijtag_Instruments.icl` | NameSpace, UseNameSpace, LocalParameter, Parameter, Attribute, Module, Instance, InputPort, ScanInPort… |
| `ijtag_scan_mux_001.icl` | Module, ScanInPort, ScanOutPort, DataInPort, ScanMux, SelectedBy, Source |
| `ijtag_scan_mux_002.icl` | Parameter, Attribute, Module, Instance, InputPort, ScanInPort, ScanOutPort, ShiftEnPort, CaptureEnPort… |
| `ijtag_test_icl_instance.icl` | Module, Instance, InputPort, ScanInPort, ScanOutPort, DataInPort, TCKPort, ScanRegister, Source… |
| `ijtag_test_icl_scan_register.icl` | Module, ScanInPort, ScanOutPort, ShiftEnPort, CaptureEnPort, UpdateEnPort, SelectPort, ResetPort, TCKPort… |
| `ijtag_test_icl_syntax_5.icl` | Parameter, Module, ScanInPort, ScanOutPort, ShiftEnPort, CaptureEnPort, UpdateEnPort, SelectPort, ResetPort… |
| `ijtag_test_icl_syntax_6.icl` | Parameter, Module, ScanInPort, ScanOutPort, ShiftEnPort, CaptureEnPort, UpdateEnPort, SelectPort, ResetPort… |
| `ijtag_test_icl_syntax_7.icl` | Module, Instance, InputPort, ScanInPort, ScanOutPort, TCKPort, ToShiftEnPort, ToCaptureEnPort, ToUpdateEnPort… |
| `number_literals_from_upstream.icl` | Attribute, Module, ScanRegister, ScanInSource, CaptureSource, ResetValue, DefaultLoadValue |

9 files.

## pass/p2654 — bradfordvt/P2654Simulations — IEEE P2654 modelling work

Three files under icl/standards/s1687: SReg.icl, sib_mux_pre.icl and sib_mux_post.icl. Verified byte-identical to the upstream repository by content hash.


| file | constructs it exercises |
|---|---|
| `SReg.icl` | Attribute, Module, ScanInPort, ScanOutPort, DataInPort, DataOutPort, ShiftEnPort, CaptureEnPort, UpdateEnPort… |
| `sib_mux_post.icl` | Attribute, Module, ScanInPort, ScanOutPort, ShiftEnPort, CaptureEnPort, UpdateEnPort, SelectPort, ResetPort… |
| `sib_mux_pre.icl` | Attribute, Module, ScanInPort, ScanOutPort, ShiftEnPort, CaptureEnPort, UpdateEnPort, SelectPort, ResetPort… |

3 files.

## pass/published — Papers, application notes and articles

ICL printed in a publication: the ASSET InterTech ICL/PDL article, Keysight's "Silicon Test to Board Test" application note, ScanWorks AccessLink examples, the LiU P1687 paper Listing 8, and a Chinese ICL/PDL tutorial.


| file | constructs it exercises |
|---|---|
| `asset_ijtag_demo_network.icl` | Module, Instance, InputPort, ScanInPort, ScanOutPort, DataInPort, DataOutPort, ScanRegister, ScanMux… |
| `connection_instrument_module.icl` | Attribute, Module, DataInPort, DataOutPort, Reg, ScanInterface, Connection, Port |
| `keysight_datamux_module.icl` | Module, DataInPort, DataOutPort, DataMux, SelectedBy, Source |
| `keysight_gpio.icl` | Attribute, Module, ScanInPort, ScanOutPort, DataInPort, DataOutPort, ShiftEnPort, CaptureEnPort, UpdateEnPort… |
| `liu_sib_and_chip_icl.icl` | Module, Instance, InputPort, ScanInPort, ScanOutPort, SelectPort, ToSelectPort, ScanRegister, ScanMux… |
| `published_sib_and_chip.icl` | Module, Instance, InputPort, ScanInPort, ScanOutPort, SelectPort, ToSelectPort, ScanRegister, ScanMux… |
| `real.icl` | Module, Instance, InputPort, ScanInPort, ScanOutPort, SelectPort, ToSelectPort, ScanRegister, ScanMux… |
| `scanworks_accesslink_nested.icl` | Module, Instance, InputPort, ScanInPort, ScanOutPort, DataOutPort, ScanInterface, AccessLink, Port… |
| `semiwiki_tdr_module.icl` | Attribute, Module, ScanInPort, ScanOutPort, ShiftEnPort, CaptureEnPort, UpdateEnPort, SelectPort, TCKPort… |

9 files.

## pass/ours — Written here

Hand-written to pin a specific behaviour: a construct, a comment position, a recursion depth, a literal spelling. Most were added alongside a fix, so the commit that introduced them names the bug.


| file | constructs it exercises |
|---|---|
| `accesslink_nested.icl` | Module, AccessLink, BSDL_Entity, ActiveSignals |
| `alias_bit_range.icl` | Module, Alias |
| `alias_concat_list.icl` | Module, Alias, RefEnum, AccessTogether, iApplyEndState |
| `alias_nested_block.icl` | Attribute, Module, Alias |
| `alias_without_block.icl` | Module, Alias |
| `all_13_port_kinds.icl` | Module, ScanInPort, ScanOutPort, DataInPort, DataOutPort, ShiftEnPort, CaptureEnPort, UpdateEnPort, SelectPort… |
| `all_port_functions.icl` | Module, ScanInPort, ScanOutPort, DataInPort, DataOutPort, ShiftEnPort, CaptureEnPort, UpdateEnPort, SelectPort… |
| `arithmetic_expression.icl` | Attribute, Module |
| `attr_with_bit_range.icl` | Module, Instance, InputPort, OutputPort |
| `attribute_comma_value_list.icl` | Attribute, Module, ScanRegister |
| `attribute_hierarchical.icl` | Attribute, Module |
| `attribute_numeric_value.icl` | Attribute, Module |
| `bare_hierarchical_name_statement.icl` | Module, ScanRegister, Source |
| `bare_use_namespace.icl` | NameSpace, UseNameSpace, Module |
| `benchmark_derived_spellings.icl` | Parameter, Module, ScanInPort, ScanOutPort, DataInPort, DataOutPort, DataRegister, ScanMux, DataMux… |
| `block_review_shapes.icl` | Parameter, Attribute, Module, Instance, InputPort, OutputPort, DataOutPort, ShiftEnPort, CaptureEnPort… |
| `bodies_with_comments_and_assignments.icl` | Module, ScanMux, LogicSignal, Connection, SelectedBy |
| `bom_stripped_not_kept.icl` | Module, ScanInPort |
| `client_tap_ports.icl` | Module, TCKPort |
| `clock_and_callback.icl` | Module, Instance, ClockPort, ToClockPort, DataRegister, ReadDataSource, AddressValue, ReadCallBack, WriteCallBack… |
| `clockmux.icl` | Module, ClockMux, SelectedBy |
| `clockmux_multiple_sources.icl` | Module, ClockMux, SelectedBy |
| `clockport.icl` | Module, ClockPort |
| `comment_nesting.icl` | Module, ScanRegister |
| `comment_opens_with_star.icl` | Module, ScanInPort |
| `comment_tab_indented.icl` | Module, ScanInPort |
| `comment_with_blank_line.icl` | Module, ScanInPort |
| `comparison_expression.icl` | Attribute, Module |
| `computed_range_bound_negative.icl` | Attribute, Module |
| `computed_range_bounds.icl` | Attribute, Module, ScanRegister, ScanInSource |
| `connection_arrows.icl` | Module, Connection |
| `crlf_line_endings.icl` | Module, ScanInPort, ScanRegister, ScanInSource |
| `datamux.icl` | Module, DataMux, SelectedBy |
| `dataregister_block.icl` | Module, DataRegister |
| `dataregister_statement.icl` | Module, DataRegister |
| `deep_recursion.icl` | Module, ScanRegister, Reg, Field, LogicSignal, Source, CaptureSource, ResetValue |
| `empty_file.icl` | — |
| `empty_module.icl` | Module |
| `enum.icl` | Module, Enum |
| `extra_ports.icl` | Module, ToIRSelectPort, AddressPort, WriteEnPort, ReadEnPort |
| `host_side_ports.icl` | Module, ToTCKPort, ToTMSPort, ToTRSTPort, ToShiftEnPort, ToCaptureEnPort, ToUpdateEnPort, ToResetPort |
| `indexed_expressions.icl` | Module, ScanRegister, ScanInSource, CaptureSource |
| `instance_of_no_body.icl` | Module, Instance, AccessLink, STD_1149_1_2001 |
| `instance_port_mapping.icl` | Module, Instance, InputPort |
| `instrument_with_refenum.icl` | Module, DataInPort, DataOutPort, ClockPort, Alias, Enum, RefEnum |
| `integer_expr_in_range.icl` | Parameter, Module, DataInPort, Reg, RefEnum, Source, ResetValue, DefaultLoadValue |
| `inverted_signals_in_concat.icl` | Module, DataOutPort, ScanRegister, Alias, Source, Enable, ScanInSource, CaptureSource, ResetValue |
| `iproc_namespace_leading_colons.icl` | Module, Instance, InputPort, DataRegister, ReadCallBack, WriteCallBack |
| `keyword_as_attribute_name.icl` | NameSpace, UseNameSpace, LocalParameter, Attribute, Module, Instance, PortGroup, LogicSignal, Alias… |
| `keyword_attr_clockport.icl` | Module, Instance, ClockPort |
| `keyword_attr_datamux.icl` | Module, Instance, DataMux |
| `keyword_attr_dataregister.icl` | Module, Instance, DataRegister |
| `keyword_attr_onehotscangroup.icl` | Module, Instance, OneHotScanGroup |
| `keyword_block_inside_register.icl` | Module, Reg, Field, LogicSignal, ResetValue |
| `literal_sized_by_parameter.icl` | Parameter, Module, DataInPort, Reg, CaptureSource, ResetValue, DefaultLoadValue |
| `logicsignal.icl` | Module, LogicSignal, Source |
| `logicsignal_boolean_expressions.icl` | Module, LogicSignal |
| `logicsignal_nested.icl` | Module, LogicSignal |
| `logicsignal_paren_list.icl` | Module, LogicSignal |
| `module_ref_global_namespace.icl` | Module, Instance, InputPort |
| `module_ref_three_segments.icl` | Module, Instance, InputPort, Source |
| `multi_instance_netlist.icl` | Module, Instance, InputPort, DataInPort, DataOutPort, Source |
| `multiple_blank_lines.icl` | Module, ScanInPort, ScanOutPort |
| `mux_comma_selectors.icl` | Attribute, Module, ScanMux, SelectedBy |
| `mux_selection_bar_and_scanout_enable.icl` | Module, ScanOutPort, ScanMux, DataMux, ClockMux, SelectedBy, Source, Enable |
| `mux_with_range.icl` | Module, ScanMux, SelectedBy |
| `namespace_and_localparam.icl` | NameSpace, UseNameSpace, LocalParameter, Parameter, Attribute, Module, Instance, InputPort, DataInPort |
| `negative_literal.icl` | Attribute, Module |
| `nested_named_block_with_range.icl` | Module, Reg, Field, ScanInterface, Chain, Port, DefaultLoadValue |
| `onehotdatagroup.icl` | Module, OneHotDataGroup, Port |
| `onehotdatagroup_instance.icl` | Module, Instance, InputPort, OneHotDataGroup, Port |
| `onehotscangroup.icl` | Module, OneHotScanGroup, Port |
| `onehotscaninterface.icl` | Module, Port |
| `only_a_comment.icl` | — |
| `param_bus_ranged_assign.icl` | Parameter, Module, Instance, InputPort, DataInPort |
| `parameter_declared.icl` | Parameter, Module |
| `parameter_in_index.icl` | Module, DataOutPort, Source |
| `parameter_in_range.icl` | Module, DataInPort |
| `parameter_overridden.icl` | Parameter, Module, Instance |
| `parameterised_bus.icl` | Parameter, Module, Instance, InputPort, DataInPort, DataOutPort, Source |
| `parenthesised.icl` | Attribute, Module |
| `portgroup_comma_list.icl` | Module, PortGroup |
| `portgroup_with_comments.icl` | Module, PortGroup |
| `ports_with_bit_ranges.icl` | Module, DataInPort, DataOutPort |
| `ragged_star_comment.icl` | Module, ScanInPort |
| `ranged_names.icl` | Module, ScanMux, DataMux, ClockMux, LogicSignal, OneHotScanGroup, OneHotDataGroup, SelectedBy, Port |
| `reg_and_field.icl` | Attribute, Module, Reg, Field, WriteEnSource |
| `scaninterface.icl` | Module, ScanInterface, Port |
| `scaninterface_chain.icl` | Module, ScanInterface, Chain, Port, DefaultLoadValue |
| `scanmux_hierarchical_sel.icl` | Module, ScanMux, SelectedBy |
| `scanmux_table.icl` | Module, ScanMux, SelectedBy |
| `scanregister_range.icl` | Module, ScanRegister, ScanInSource |
| `separated_decimal.icl` | Attribute, Module |
| `several_modules.icl` | Module |
| `shift_and_logic.icl` | Attribute, Module |
| `single_port.icl` | Module, ScanInPort |
| `sized_literals.icl` | Module, ScanRegister, ResetValue |
| `toclockport.icl` | Module, ToClockPort |
| `unary_attribute_value.icl` | Module, Reg, Source, CaptureSource, ResetValue |
| `unary_complement.icl` | Attribute, Module |
| `unsized_binary_literal.icl` | Module, Instance, InputPort |
| `unsized_hex_literal.icl` | Attribute, Module |
| `wrapped_statement_no_blank_line.icl` | Module, ScanRegister, ScanInSource, CaptureSource |

103 files.

## fail/standard — rules stated by the standard or the published grammar

| file | what it must reject |
|---|---|
| `chain_outside_scaninterface.icl` | Module | Chain | Port |
| `module_level_bare_attribute.icl` | Attribute | Module |

## fail/ours — written here

| file | what it must reject |
|---|---|
| `accesslink_block_incomplete.icl` | Module | AccessLink | BSDL_Entity |
| `connection_without_semicolon.icl` | Module | Connection |
| `extra_closing_brace.icl` | Module | ScanInPort |
| `illegal_character.icl` | Module | ScanInPort |
| `instance_without_of.icl` | Module | Instance |
| `misspelled_keyword_modul.icl` | ScanInPort |
| `module_level_identifier_attribute.icl` | Module |
| `module_without_a_name.icl` | Module | ScanInPort |
| `parameter_without.icl` | Parameter | Module |
| `port_at_top_level.icl` | ScanInPort |
| `portgroup_without_list.icl` | Module | PortGroup |
| `scanmux_without_select_reg.icl` | Module | ScanMux |
| `statement_without_semicolon.icl` | Module | ScanInPort |
| `unclosed_brace.icl` | Module | ScanInPort |
| `unknown_construct_foobar.icl` | Module |

## Adding a case

Drop a `.icl` file into the directory matching its origin. Nothing else needs editing: the runner walks subdirectories.

