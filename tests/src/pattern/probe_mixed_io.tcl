# Self-constructed scan/parallel event-order probe for Tessent patterns -ijtag.
# Copy the two mixed_io fixtures beside this script; run in a separate case dir.
set here [file dirname [info script]]
if {[catch {
  set case_name $env(ICL_TOOLSETS_CASE)
  switch -- $case_name {
    primary_data { set instrument MixedPrimaryData; set input {data[3:0]}; set port data }
    scan_data { set instrument MixedScanData; set input enable; set port enable }
    default { error "invalid mixed-I/O probe case" }
  }
  set top ${instrument}Probe
  set handle [open [file join $here ${case_name}.icl] r]
  set source [read $handle]
  close $handle
  set source [string map {{ScanInPort si;} {ScanInPort si;
  SelectPort select; CaptureEnPort capture; ShiftEnPort shift;
  UpdateEnPort update; TCKPort clock;
  ScanInterface client {
    Port si; Port so; Port select; Port capture; Port shift; Port update; Port clock;
  }}} $source]
  append source "\nModule $top \{\n  DataInPort $input;\n"
  append source "  Instance core Of $instrument \{ InputPort $port = $port; \}\n"
  append source {  DataOutPort observed[3:0] { Source core.observed; }
  AccessLink tap Of STD_1149_1_2001 {
    BSDLEntity AccessProbe;
    IJTAG_ON { ScanInterface { core.client; } }
    IJTAG_OFF { ScanInterface { core.client; } }
  }
}
  append source "\}\n"
  set model [file join [pwd] mixed_model.icl]
  set handle [open $model w]
  puts $handle $source
  close $handle
  set_context patterns -ijtag
  read_icl $model -force -bsdl_file [file join $here model.bsdl]
  set_current_design $top
  set_system_mode analysis
  open_pattern_set oracle -no_initial_ireset
  iReset
  iWrite core.value 5
  iApply
  iWrite core.value 9
  iRead core.value 5
  iApply
  close_pattern_set
  write_patterns [file join [pwd] result.pdl] -pdl -pattern_sets oracle -replace
  write_patterns [file join [pwd] result.v] -verilog -pattern_sets oracle -replace
  puts "ICL_TOOLSETS_ORACLE_COMPLETE: $case_name"
} message]} {
  puts stderr "ICL_TOOLSETS_ORACLE_ERROR: $message"
  exit 1
}
exit 0
