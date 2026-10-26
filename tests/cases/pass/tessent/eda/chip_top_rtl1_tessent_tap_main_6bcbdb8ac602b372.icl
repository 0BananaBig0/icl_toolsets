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
//       Created on: Mon Oct 26 09:09:07 CST 2026
//--------------------------------------------------------------------------


NameSpace ;
UseNameSpace ;
Module chip_top_rtl1_tessent_tap_main {
  TCKPort         tck;
  ScanInPort      tdi;
  ScanOutPort     tdo    { Source IRMux;
    Attribute forced_high_output_port_list = "tdo_en"; 
    Attribute forced_low_dft_signal_list = "tms_disable";
  }
  DataOutPort     tdo_en {
    Attribute associated_scan_port_list = "tdo";
    Attribute connection_rule_option = "allowed_no_destination";
    Attribute function_modifier = "tdo_enable_active_high";
  }
  TMSPort         tms    {
    Attribute forced_low_dft_signal_list = "tms_disable";
  }
  TRSTPort        trst   {
    Attribute connection_rule_option = "allowed_tied_high";
  }
  ToCaptureEnPort capture_dr_en;
  ToShiftEnPort   shift_dr_en;
  ToUpdateEnPort  update_dr_en;
  ToResetPort     test_logic_reset  { ActivePolarity 0; }
  ToSelectPort    host_1_to_sel { Source host_1_to_sel_int;
    Attribute connection_rule_option = "allowed_no_destination";   
  }
  LogicSignal     host_1_to_sel_int { instruction == HOSTIJTAG_1;    }
  ScanInPort      host_1_from_so {
    Attribute connection_rule_option = "allowed_no_source";
  }
  ScanInPort      host_bscan_from_so {
    Attribute connection_rule_option         = "allowed_no_source";
    Attribute tessent_bscan_pipeline_stages  = "0";
  }
  ToSelectPort    host_bscan_to_sel { Source bscan_select_int;
    Attribute connection_rule_option = "allowed_no_destination";
    Attribute tessent_bscan_function  = "select";
  }
  LogicSignal bscan_select_int { 
    (instruction == EXTEST) ||
    (instruction == INTEST) ||
    (instruction == EXTEST_PULSE) ||
    (instruction == EXTEST_TRAIN) ||
    (instruction == SAMPLE) ||
    (instruction == PRELOAD) ;
  }
  DataOutPort     force_disable { Source force_disable_int; 
    Attribute connection_rule_option = "allowed_no_destination";
    Attribute tessent_bscan_function  = "force_disable";
  }
  LogicSignal force_disable_int { instruction == HIGHZ; } 
  DataOutPort     select_jtag_input { Source select_jtag_input_int;
    Attribute connection_rule_option = "allowed_no_destination";
    Attribute tessent_bscan_function = "select_jtag_input";
  }
  LogicSignal select_jtag_input_int { instruction == INTEST; } 
  DataOutPort     select_jtag_output { Source select_jtag_output_int;
    Attribute connection_rule_option = "allowed_no_destination";
    Attribute tessent_bscan_function = "select_jtag_output";
  }
  LogicSignal select_jtag_output_int { 
    (instruction == EXTEST) ||
    (instruction == EXTEST_PULSE) ||
    (instruction == EXTEST_TRAIN) ||
    (instruction == CLAMP) ||
    (instruction == HIGHZ) ;
  }
  DataOutPort     extest_pulse { Source ext_test_pulse_int;
    Attribute connection_rule_option = "allowed_no_destination";
    Attribute tessent_bscan_function = "extest_pulse";
  }
  LogicSignal ext_test_pulse_int { instruction == EXTEST_PULSE; } 
  DataOutPort     extest_train { Source ext_test_train_int;
    Attribute connection_rule_option = "allowed_no_destination";
    Attribute tessent_bscan_function = "extest_train";
  }
  LogicSignal ext_test_train_int { instruction == EXTEST_TRAIN; } 
  DataOutPort fsm_state[3:0]{ 
    Attribute connection_rule_option = "allowed_no_destination";
    Attribute function_modifier = "tap_fsm_state";
    RefEnum   state_encoding;
  }
 
  Enum state_encoding {
    test_logic_reset  = 4'b1111;
    run_test_idle     = 4'b1110;
    select_dr         = 4'b0111;
    capture_dr        = 4'b0110;
    shift_dr          = 4'b0010;
    exit1_dr          = 4'b0001;
    pause_dr          = 4'b0011;
    exit2_dr          = 4'b0000;
    update_dr         = 4'b0101;
    select_ir         = 4'b0100;
    capture_ir        = 4'b1100;
    shift_ir          = 4'b1010;
    exit1_ir          = 4'b1001;
    pause_ir          = 4'b1011;
    exit2_ir          = 4'b1000;
    update_ir         = 4'b1101;
  }
 
  ScanInterface tap_client { 
    Port tdi; 
    Port tdo; 
    Port tms;
    Port tck;
    Port trst;
  }
  ScanInterface host_ijtag_1 { 
    Port host_1_from_so; 
    Port host_1_to_sel; 
  }
 
  ScanInterface host_bscan {
    Port host_bscan_to_sel;
    Port host_bscan_from_so;
    Port capture_dr_en;
    Port shift_dr_en;
    Port update_dr_en;
    Port test_logic_reset;
    Attribute          tessent_is_bscan_host = "true";
  }
  Instance fsm Of chip_top_rtl1_tessent_tap_main_fsm  {
    InputPort tck  = tck;
    InputPort tms  = tms;
    InputPort trst = trst;
  }
  ScanRegister instruction[3:0] {
    CaptureSource  4'b0001;
    ResetValue     4'b1111;
    ScanInSource   tdi;
    RefEnum        instruction_opcodes;
  }
  Enum instruction_opcodes {
    BYPASS            = 4'b1111;
    HOSTIJTAG_1       = 4'b0000;
    CLAMP             = 4'b0001;
    EXTEST            = 4'b0010;
    EXTEST_PULSE      = 4'b0011;
    EXTEST_TRAIN      = 4'b0100;
    INTEST            = 4'b0101;
    SAMPLE            = 4'b0110;
    PRELOAD           = 4'b0110;
    HIGHZ             = 4'b0111;
  }
 
  ScanRegister bypass {
    CaptureSource  1'b0;
    ScanInSource   tdi;
  }
  ScanMux IRMux SelectedBy fsm.irSel {
    1'b0 : DRMux;
    1'b1 : instruction[0];
  }
  ScanMux DRMux SelectedBy instruction {
    4'b1111           : bypass;
    4'b0000           : host_1_from_so;
    4'b0001           : bypass;
    4'b0010           : host_bscan_from_so;
    4'b0011           : host_bscan_from_so;
    4'b0100           : host_bscan_from_so;
    4'b0101           : host_bscan_from_so;
    4'b0110           : host_bscan_from_so;
    4'b0111           : bypass;
    'bX               : bypass;
  }
  Attribute            keep_active_during_scan_test = "true";
  Attribute            tessent_instruction_reg          = "instruction";
  Attribute            tessent_bypass_reg               = "bypass";
  Attribute            tessent_instrument_container     = "chip_top_rtl1_ijtag"; 
  Attribute            tessent_use_in_dft_specification = "false";
  Attribute            tessent_instrument_type          = "mentor::ijtag_node";
  Attribute            tessent_instrument_subtype       = "tap_controller";
  Attribute            tessent_signature                = "774f0efdef9d5b074af5f536d5077f57";
}
