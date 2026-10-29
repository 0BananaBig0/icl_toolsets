# Run with Tessent in an isolated test_icl_toolsets/test_resource_slices case.
# The caller sets ICL_TOOLSETS_CASE to the name of a copied PDL fixture.
set here [file dirname [info script]]
if {[catch {
  set case_name $env(ICL_TOOLSETS_CASE)
  if {$case_name ni {slice_high slice_low slice_release slice_alias slice_partial_write}} {
    error "invalid resource probe case"
  }
  set_context patterns -ijtag
  read_icl [file join $here slices.icl] -force -bsdl_file [file join $here model.bsdl]
  set_current_design SliceTop
  set_system_mode analysis
  open_pattern_set oracle -no_initial_ireset
  dofile [file join $here ${case_name}.pdl]
  close_pattern_set
  write_patterns [file join $here ${case_name}_result.pdl] -pdl -pattern_sets oracle
  puts "ICL_TOOLSETS_ORACLE_COMPLETE: $case_name"
} message]} {
  puts stderr "ICL_TOOLSETS_ORACLE_ERROR: $message"
  exit 1
}
exit 0
