set here [file dirname [info script]]
if {[catch {
  set_context patterns -ijtag
  read_icl [file join $here model.icl]
  set_current_design Top
  puts ICL_TOOLSETS_DATAMUX_MODEL_LOADED
} message]} {
  puts stderr "ICL_TOOLSETS_DATAMUX_LOAD_ERROR: $message"
  exit 1
}
exit 0
