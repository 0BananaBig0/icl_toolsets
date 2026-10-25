# Tessent patterns -ijtag: setup for loading/query, analysis for PDL/export.
set here [file dirname [info script]]
if {[catch {
  set_context patterns -ijtag
  read_icl [file join $here address_defaults.icl]
  read_icl [file join $here address_partial_data.icl]
  set top AddressDefaultsTop
  if {[info exists env(ICL_TOOLSETS_PROBE_TOP)]} {
    set top $env(ICL_TOOLSETS_PROBE_TOP)
  }
  set_current_design $top
  if {$top in {ExplicitDataTop SplitDefaultTop}} {
    set data_pins {child.upper[3] child.upper[0] child.lower[0] child.lower[3]}
  } else {
    set data_pins {child.data[7] child.data[4] child.data[3] child.data[0]}
  }
  foreach pin [concat {child.address[1] child.address[0]} $data_pins {child.we child.re}] {
    puts "ORACLE_FANIN $pin [get_name_list [get_icl_fanins $pin -stop_on pin]]"
  }
  set_system_mode analysis
  open_pattern_set address_defaults -no_initial_ireset
  iWrite child.r 0x96
  iApply
  close_pattern_set
  write_patterns [file join $here result.pdl] -pdl -pattern_sets address_defaults
  puts ICL_TOOLSETS_ORACLE_COMPLETE
} message]} {
  puts stderr "ICL_TOOLSETS_ORACLE_ERROR: $message"
  exit 1
}
exit 0
