# Self-constructed ownership/reset probe. Setup precedes patterns -ijtag analysis.
# Copy with resources/slices.icl and access_link/simple.bsdl as model.bsdl.
set here [file dirname [info script]]
if {[catch {
  set case_name $env(ICL_TOOLSETS_CASE)
  if {![regexp {^(high|low)_(zero|one)_(async|sync)$} $case_name -> field value mode]} {
    error "invalid reset reservation probe"
  }
  set index [expr {$field eq "high" ? 1 : 0}]
  set value [expr {$value eq "one" ? 1 : 0}]
  set target [format {core.gate[%d]} $index]
  set_context patterns -ijtag
  read_icl [file join $here slices.icl] -force -bsdl_file [file join $here model.bsdl]
  set_current_design SliceTop
  set_system_mode analysis
  open_pattern_set oracle -no_initial_ireset
  iReset
  iTake $target
  iWrite $target $value
  iApply
  if {$mode eq "sync"} { iReset -sync } else { iReset }
  puts "ICL_TOOLSETS_RESET_COMPLETE: $case_name"
  if {[info exists env(ICL_TOOLSETS_REWRITE)] && $env(ICL_TOOLSETS_REWRITE)} {
    iWrite $target $value
    iApply
  }
  iWrite core.target 5
  iApply
  close_pattern_set
  write_patterns [file join $here result.pdl] -pdl -pattern_sets oracle
  puts "ICL_TOOLSETS_ORACLE_COMPLETE: $case_name"
} message]} {
  puts stderr "ICL_TOOLSETS_ORACLE_ERROR: $message"
  exit 1
}
exit 0
