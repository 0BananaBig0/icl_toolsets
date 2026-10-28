# DataRegister probes

These are self-constructed complete models based on IEEE 1687-2014 §6.4.9,
printed pp.118–121. `selectable.icl` covers primary data/write-enable control;
`addressed.icl` covers two registers sharing address/data/enable buses and an
observable OneHotDataGroup. Their PDL exercises writes followed by comparisons.

`address_defaults.pdl` uses the existing Tessent-derived
`tests/cases/tessent/pattern_generation/address_defaults.icl` fixture: child offset
4 plus register address 1, and the measured parent-upper-byte data mapping.
No duplicate ICL copy is added. The vendor manifest references these same sources.

The CLI suite also constructs inversion, mux, partial-read, end-state, unknown
control and destructive-read variants in its temporary directory. All input
text is retained in `tests/src/pattern/test_data_register_scheduling.py`.
