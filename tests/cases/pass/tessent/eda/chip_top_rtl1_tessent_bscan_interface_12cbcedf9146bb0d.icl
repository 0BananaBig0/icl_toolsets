//--------------------------------------------------------------------------
//
//  Unpublished work. Copyright 2026 Siemens
//
//  This material contains trade secrets or otherwise confidential 
//  information owned by Siemens Industry Software Inc. or its affiliates 
//  (collectively, SISW), or its licensors. Access to and use of this 
//  information is strictly limited as set forth in the Customer's 
//  applicable agreements with SISW.
//
//--------------------------------------------------------------------------
//  File created by: Tessent Shell
//          Version: 2026.2
//       Created on: Mon Oct 26 09:09:11 CST 2026
//--------------------------------------------------------------------------


NameSpace ;
UseNameSpace ;
Module chip_top_rtl1_tessent_bscan_interface {

  ScanOutPort        scan_out {
    Source           BScanReg[0];
    Attribute        connection_rule_option = "allowed_no_destination";
    Attribute        tessent_use_in_dft_specification = "false";
  }
  ScanInPort         scan_in {
    Attribute        connection_rule_option = "allowed_no_source";
    Attribute        tessent_use_in_dft_specification = "false";
  }
  ScanInPort         from_bscan_scan_out {
    Attribute        connection_rule_option = "allowed_no_source";
    Attribute        tessent_use_in_dft_specification = "false";
  }
  CaptureEnPort      ijtag_capture_en {
    Attribute        connection_rule_option = "allowed_no_source";
    Attribute        tessent_use_in_dft_specification = "false";
  }
  ShiftEnPort        ijtag_shift_en {
    Attribute        connection_rule_option = "allowed_no_source";
    Attribute        tessent_use_in_dft_specification = "false";
  }
  UpdateEnPort       ijtag_update_en {
    Attribute        connection_rule_option = "allowed_no_source";
    Attribute        tessent_use_in_dft_specification = "false";
  }
  SelectPort         bscan_select {
    Attribute        tessent_bscan_function = "select";
    Attribute        tessent_bscan_clock_port = "ijtag_tck";
    Attribute        tessent_use_in_dft_specification = "false";
  }

  TCKPort            ijtag_tck;
  DataInPort         force_disable {
    Attribute        connection_rule_option = "allowed_tied_low";
    Attribute        tessent_bscan_function = "force_disable";
    Attribute        tessent_bscan_clock_port = "ijtag_tck";
    Attribute        tessent_use_in_dft_specification = "false";
  }
  DataInPort         select_jtag_input {
    Attribute        connection_rule_option = "allowed_tied_low";
    Attribute        tessent_bscan_function = "select_jtag_input";
    Attribute        tessent_bscan_clock_port = "ijtag_tck";
    Attribute        tessent_use_in_dft_specification = "false";
  }
  DataInPort         select_jtag_output {
    Attribute        connection_rule_option = "allowed_tied_low";
    Attribute        tessent_bscan_function = "select_jtag_output";
    Attribute        tessent_bscan_clock_port = "ijtag_tck";
    Attribute        tessent_use_in_dft_specification = "false";
  }
  DataInPort         ltest_en {
    Attribute        explicit_iwrite_only = 1'b1;
    Attribute        connection_rule_option = "allowed_tied_low";
  }
  DataInPort         occ_en {
    Attribute        explicit_iwrite_only = 1'b1;
    Attribute        connection_rule_option = "allowed_tied_low";
  }
  DataInPort         static_clock_control_mode {
    Attribute        connection_rule_option = "allowed_tied_low";
  }
  DataInPort         ltest_clock_sequence[1:0] {
    Attribute        connection_rule_option = "allowed_no_source";
    Attribute        tessent_use_in_dft_specification = "false";
    Attribute        function_modifier = "tessent_clock_sequence";
  }

  ScanInterface tessent_bscan {
    Attribute        tessent_disable_constraints_if_unused = "true";
    Port             scan_out;
    Port             scan_in;
    Port             ijtag_capture_en;
    Port             ijtag_shift_en;
    Port             ijtag_update_en;
    Port             ijtag_tck;
    Port             bscan_select;
  }

  ScanRegister       BScanReg[89:0] {
    ScanInSource     scan_in;
    Attribute        tessent_ignore_during_icl_verification = "on";
    Attribute        tessent_use_icl_override_wrapper = "false";
  }

  Attribute          tessent_instrument_container     = "chip_top_rtl1_bscan.instrument";
  Attribute          tessent_instrument_type          = "mentor::jtag_bscan";
  Attribute          tessent_instrument_subtype       = "bscan_interface";
  Attribute          tessent_design_id                = "rtl1";
  Attribute          tessent_signature                = "56c704cc96df5462bbcb43a0927ed072";
  Attribute          tessent_use_in_dft_specification = "false";
  Attribute          tessent_boundary_scan_reg        = "BScanReg";
  Attribute          forced_low_output_port_list      = "{to_bscan_force_disable} {to_bscan_select_jtag_input} {to_bscan_select_jtag_output}";
  Attribute          tessent_bscan_clock_persistent_buffer_output_list =
                                                        "{tessent_persistent_cell_to_bscan_tck_buf/Y} ",
                                                        "{tessent_persistent_cell_capture_shift_clock_gater_inst/GCK} ",
                                                        "{tessent_persistent_cell_update_clock_gater_inst/GCK}";
  Attribute          tessent_bscan_control_persistent_buffer_output_list =
                                                        "{tessent_persistent_cell_ijtag_shift_en_buf/Y} ",
                                                        "{tessent_persistent_cell_ijtag_update_en_buf/Y} ",
                                                        "{tessent_persistent_cell_ijtag_capture_en_buf/Y} ",
                                                        "{tessent_persistent_cell_force_disable_buf/Y}";
  Attribute          tessent_bscan_force_high_persistent_buffer_output_list =
                                                        "{tessent_persistent_cell_bscan_select_buf/Y} ",
                                                        "{tessent_persistent_cell_select_jtag_output_buf/Y}";
  Attribute          tessent_bscan_force_low_persistent_buffer_output_list =
                                                        "{tessent_persistent_cell_select_jtag_input_buf/Y} ",
                                                        "{tessent_persistent_cell_ltest_en_buf/Y}";
  Attribute          tessent_bscan_dft_signal_buffer_output_list =
                                                        "{tessent_persistent_cell_force_disable_buf/Y} ",
                                                        "{tessent_persistent_cell_ltest_en_buf/Y} ",
                                                        "{tessent_persistent_cell_output_pad_disable_buf/Y} ",
                                                        "{tessent_persistent_cell_bscan_clamp_enable_buf/Y} ",
                                                        "{tessent_persistent_cell_int_ltest_en_buf/Y}";
}
