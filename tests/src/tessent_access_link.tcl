# Run in an isolated directory with the access_link fixtures copied alongside.
set here [file dirname [info script]]
if {[catch {
  set_context patterns -ijtag
  read_icl [file join $here simple.icl] -bsdl_file [file join $here simple.bsdl]
  set_current_design AccessTop
  set_system_mode analysis
  open_pattern_set oracle -no_initial_ireset
  dofile [file join $here write.pdl]
  close_pattern_set
  write_patterns [file join $here result.pdl] -pdl -pattern_sets oracle
  write_icl [file join $here expanded.icl]
  puts "ICL_TOOLSETS_ACCESS_LINK_COMPLETE"
} message]} {
  puts stderr "ICL_TOOLSETS_ACCESS_LINK_ERROR: $message"
  exit 1
}
exit 0
