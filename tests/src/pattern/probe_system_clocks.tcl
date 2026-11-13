# Run beside model.icl, source.pdl and ours.pdl in an isolated probe directory.
# The physical root uses an explicit 100ns period for this probe.
set here [file dirname [info script]]
if {[catch {
  set_context patterns -ijtag
  read_icl [file join $here model.icl]
  set_current_design Top
  add_clocks root -period 100ns
  set_system_mode analysis
  foreach {label input} {source source.pdl normalized ours.pdl} {
    open_pattern_set $label -no_initial_ireset
    dofile [file join $here $input]
    close_pattern_set
    write_patterns [file join $here $label-result.pdl] -pdl -pattern_sets $label
  }
  puts ICL_TOOLSETS_CLOCK_PROBE_COMPLETE
} message]} {
  puts stderr "ICL_TOOLSETS_CLOCK_PROBE_ERROR: $message"
  exit 1
}
exit 0
