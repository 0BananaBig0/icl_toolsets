# Self-constructed clock-only duration probe; stage model.icl and input.pdl.
set here [file dirname [info script]]
if {[catch {
  set_context patterns -ijtag
  read_icl [file join $here model.icl]
  set_current_design Top
  add_clocks root -period 100ns
  add_clocks root_n -period 100ns
  add_input_constraints -equivalent root -invert root_n
  set_system_mode analysis
  open_pattern_set duration -no_initial_ireset
  dofile [file join $here input.pdl]
  close_pattern_set
  write_patterns [file join $here result.pdl] -pdl -pattern_sets duration
  puts ICL_TOOLSETS_CLOCK_PROBE_COMPLETE
} message]} {
  puts stderr "ICL_TOOLSETS_CLOCK_PROBE_ERROR: $message"
  exit 1
}
exit 0
