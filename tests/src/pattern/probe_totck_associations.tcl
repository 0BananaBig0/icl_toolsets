# Copyright 2026 The ICL Toolsets Authors.
# SPDX-License-Identifier: MIT
# Tessent patterns -ijtag / setup; query commands permit all modes.
set here [file dirname [info script]]
if {[catch {
  set_context patterns -ijtag
  read_icl [file join $here model.icl]
  set_current_design Top
} message]} {
  puts stderr "ICL_TOOLSETS_TCK_LOAD_ERROR: $message"
  exit 1
}
puts ICL_TOOLSETS_TCK_MODEL_LOADED
if {[catch {
  set outputs [get_icl_ports out*]
  puts [list ICL_TOOLSETS_TCK_OUTPUTS [get_name_list $outputs]]
  foreach target [get_name_list $outputs] {
    puts [list ICL_TOOLSETS_TCK_FANIN $target \
              [get_name_list [get_icl_fanins [list $target]]]]
    puts [list ICL_TOOLSETS_TCK_MODULE_FANIN $target \
              [get_name_list [get_icl_fanins_in_module \
                              [get_icl_ports [list $target]]]]]
  }
} message]} {
  puts stderr "ICL_TOOLSETS_TCK_QUERY_ERROR: $message"
  exit 2
}
exit 0
