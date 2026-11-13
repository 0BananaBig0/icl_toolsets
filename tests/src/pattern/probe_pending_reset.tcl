# Self-constructed queued-action reset probe, patterns -ijtag / analysis.
# Copy with access_link/simple.icl and simple.bsdl; keep each run's outputs.
set here [file dirname [info script]]
if {[catch {
  set_context patterns -ijtag
  read_icl [file join $here simple.icl] -force -bsdl_file [file join $here simple.bsdl]
  set_current_design AccessTop
  set_system_mode analysis
  foreach kind {write read both} {
    foreach mode {async sync} {
      set name ${kind}_${mode}
      open_pattern_set $name -no_initial_ireset
      iReset
      iWrite core.r 90
      iApply
      if {$kind in {write both}} { iWrite core.r 165 }
      if {$kind in {read both}} { iRead core.r 90 }
      if {$mode eq "sync"} { iReset -sync } else { iReset }
      iApply
      close_pattern_set
      write_patterns [file join $here ${name}.pdl] -pdl -pattern_sets $name
      puts "ICL_TOOLSETS_ORACLE_COMPLETE: $name"
    }
  }
} message]} {
  puts stderr "ICL_TOOLSETS_ORACLE_ERROR: $message"
  exit 1
}
exit 0
