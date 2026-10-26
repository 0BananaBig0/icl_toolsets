//-------------------------------------------------
//  File created by: Tessent Shell
//          Version: 2026.2
//       Created on: Mon Oct 26 09:09:16 CST 2026
//-------------------------------------------------





// instanced as chip_top.add_clock_inst_0



// instanced as chip_top.chip_top_rtl1_tessent_bscan_interface_I



// instanced as chip_top.chip_top_rtl1_tessent_sib_pb1_inst
// instanced as chip_top.chip_top_rtl1_tessent_sib_pb2_inst
// instanced as chip_top.chip_top_rtl1_tessent_sib_pb3_inst
// instanced as chip_top.chip_top_rtl1_tessent_sib_sri_ctrl_inst



// instanced as chip_top.chip_top_rtl1_tessent_sib_sri_inst



// instanced as chip_top.chip_top_rtl1_tessent_tap_main_inst



// instanced as chip_top_rtl1_tessent_tap_main.fsm



// instanced as chip_top.chip_top_rtl1_tessent_tdr_sri_ctrl_inst



//tessent_pragma child_block_beginning

// instanced as chip_top.GPS_1
// instanced as chip_top.GPS_2



// instanced as gps_baseband.gps_baseband_rtl1_tessent_edt_c1_ext_inst



// instanced as gps_baseband.gps_baseband_rtl1_tessent_edt_c1_ext_tdr_inst



// instanced as gps_baseband.gps_baseband_rtl1_tessent_edt_c1_int_inst



// instanced as gps_baseband.gps_baseband_rtl1_tessent_edt_c1_int_tdr_inst



// instanced as gps_baseband.gps_baseband_rtl1_tessent_occ_clk_inst



// instanced as gps_baseband_rtl1_tessent_occ.tdr_sib



// instanced as gps_baseband.gps_baseband_rtl1_tessent_sib_edt_inst
// instanced as gps_baseband.gps_baseband_rtl1_tessent_sib_occ_inst
// instanced as gps_baseband.gps_baseband_rtl1_tessent_sib_sri_ctrl_inst
// instanced as gps_baseband.gps_baseband_rtl1_tessent_sib_ssn_inst



// instanced as gps_baseband.gps_baseband_rtl1_tessent_sib_sri_inst



// instanced as gps_baseband.gps_baseband_rtl1_tessent_ssn_pipe_1_inst
// instanced as gps_baseband.gps_baseband_rtl1_tessent_ssn_pipe_2_inst



// instanced as gps_baseband.gps_baseband_rtl1_tessent_ssn_scan_host_1_inst



// instanced as gps_baseband_rtl1_tessent_ssn_scan_host_1.inner



// instanced as gps_baseband.gps_baseband_rtl1_tessent_tdr_sri_ctrl_inst



// instanced as chip_top.PROCESSOR_1



// instanced as processor_core.DATA_MEM_1
// instanced as processor_core.PROG_MEM_2



// instanced as processor_core.DATA_MEM_1_interface_inst



// instanced as processor_core.PROG_MEM_2_interface_inst



// instanced as processor_core.processor_core_rtl1_tessent_mbist_bap_inst



// instanced as processor_core.processor_core_rtl1_tessent_mbist_c1_controller_inst



// instanced as processor_core.processor_core_rtl1_tessent_sib_mbist_inst



// instanced as processor_core.processor_core_rtl1_tessent_sib_sti_inst



// instanced as processor_core.processor_core_rtl2_tessent_edt_c1_ext_inst



// instanced as processor_core.processor_core_rtl2_tessent_edt_c1_ext_tdr_inst



// instanced as processor_core.processor_core_rtl2_tessent_edt_c1_int_inst



// instanced as processor_core.processor_core_rtl2_tessent_edt_c1_int_tdr_inst



// instanced as processor_core.processor_core_rtl2_tessent_occ_dco_clk_inst



// instanced as processor_core_rtl2_tessent_occ.tdr_sib



// instanced as processor_core.processor_core_rtl2_tessent_sib_edt_inst
// instanced as processor_core.processor_core_rtl2_tessent_sib_occ_inst
// instanced as processor_core.processor_core_rtl2_tessent_sib_sri_ctrl_inst
// instanced as processor_core.processor_core_rtl2_tessent_sib_ssn_inst



// instanced as processor_core.processor_core_rtl2_tessent_sib_sri_inst



// instanced as processor_core.processor_core_rtl2_tessent_ssn_pipe_1_inst
// instanced as processor_core.processor_core_rtl2_tessent_ssn_pipe_2_inst



// instanced as processor_core.processor_core_rtl2_tessent_ssn_scan_host_1_inst



// instanced as processor_core_rtl2_tessent_ssn_scan_host_1.inner



// instanced as processor_core.processor_core_rtl2_tessent_tdr_sri_ctrl_inst

NameSpace ;
UseNameSpace ;
Module chip_top {
   // Created by ICL extraction
   ClockPort INCLK {
      Attribute tessent_clock_domain_labels = "INCLK INCLK";
      Attribute tessent_clock_periods = "all 10.00ns";
   }
   ClockPort REF_CLK {
      Attribute tessent_clock_domain_labels = "REF_CLK REF_CLK";
      Attribute tessent_clock_periods = "all 48.00ns";
   }
   TCKPort TCK;
   ScanInPort TDI {
      Attribute tessent_use_in_dft_specification = "false";
   }
   ScanOutPort TDO {
      Source chip_top_rtl1_tessent_tap_main_inst.tdo;
      Attribute forced_low_dft_signal_list = "tms_disable";
   }
   TMSPort TMS {
      Attribute forced_low_dft_signal_list = "tms_disable";
   }
   TRSTPort TRST {
      Attribute connection_rule_option = "allowed_tied_high";
   }
   ScanInterface tap {
      Port TCK;
      Port TDI;
      Port TDO;
      Port TMS;
      Port TRST;
   }
   Attribute tessent_design_format = "verilog_2001";
   Attribute tessent_incomplete_features = "ssn";
   Attribute test_setup_procfile = "";
   Attribute icl_extraction_date = "Mon Oct 26 09:09:16 CST 2026";
   Attribute created_by_tessent_icl_extract = "true";
   Attribute tessent_design_id = "rtl1";
   Attribute tessent_design_level = "chip";
   Attribute tessent_is_physical_module = "true";
   Instance GPS_1 Of gps_baseband {
      InputPort clk = add_clock_inst_0.ClockOut;
      InputPort ijtag_ce = chip_top_rtl1_tessent_tap_main_inst.capture_dr_en;
      InputPort ijtag_reset = 
          chip_top_rtl1_tessent_tap_main_inst.test_logic_reset;
      InputPort ijtag_se = chip_top_rtl1_tessent_tap_main_inst.shift_dr_en;
      InputPort ijtag_sel = chip_top_rtl1_tessent_sib_pb1_inst.ijtag_to_sel;
      InputPort ijtag_si = chip_top_rtl1_tessent_sib_pb2_inst.ijtag_so;
      InputPort ijtag_tck = TCK;
      InputPort ijtag_ue = chip_top_rtl1_tessent_tap_main_inst.update_dr_en;
      InputPort ssn_bus_data_in[1] = 'bx;
      InputPort ssn_bus_data_in[0] = 'bx;
      Attribute tessent_design_instance = "GPS_1";
   }
   Instance GPS_2 Of gps_baseband {
      InputPort clk = add_clock_inst_0.ClockOut;
      InputPort ijtag_ce = chip_top_rtl1_tessent_tap_main_inst.capture_dr_en;
      InputPort ijtag_reset = 
          chip_top_rtl1_tessent_tap_main_inst.test_logic_reset;
      InputPort ijtag_se = chip_top_rtl1_tessent_tap_main_inst.shift_dr_en;
      InputPort ijtag_sel = chip_top_rtl1_tessent_sib_pb2_inst.ijtag_to_sel;
      InputPort ijtag_si = chip_top_rtl1_tessent_sib_pb3_inst.ijtag_so;
      InputPort ijtag_tck = TCK;
      InputPort ijtag_ue = chip_top_rtl1_tessent_tap_main_inst.update_dr_en;
      InputPort ssn_bus_data_in[1] = 'bx;
      InputPort ssn_bus_data_in[0] = 'bx;
      Attribute tessent_design_instance = "GPS_2";
   }
   Instance PROCESSOR_1 Of processor_core {
      InputPort dco_clk = add_clock_inst_0.ClockOut;
      InputPort ijtag_ce = chip_top_rtl1_tessent_tap_main_inst.capture_dr_en;
      InputPort ijtag_reset = 
          chip_top_rtl1_tessent_tap_main_inst.test_logic_reset;
      InputPort ijtag_se = chip_top_rtl1_tessent_tap_main_inst.shift_dr_en;
      InputPort ijtag_sel = chip_top_rtl1_tessent_sib_pb3_inst.ijtag_to_sel;
      InputPort ijtag_si = chip_top_rtl1_tessent_sib_sri_ctrl_inst.ijtag_so;
      InputPort ijtag_tck = TCK;
      InputPort ijtag_ue = chip_top_rtl1_tessent_tap_main_inst.update_dr_en;
      InputPort ssn_bus_data_in[1] = 'bx;
      InputPort ssn_bus_data_in[0] = 'bx;
      Attribute tessent_design_instance = "PROCESSOR_1";
   }
   Instance add_clock_inst_0 Of chip_top_add_clock_0 {
      InputPort ClockIn = REF_CLK;
      Attribute tessent_clock_domain_labels = "ck0 {PLL_1/pll_clock_0}";
   }
   Instance chip_top_rtl1_tessent_bscan_interface_I Of 
       chip_top_rtl1_tessent_bscan_interface {
      InputPort scan_in = TDI;
      InputPort from_bscan_scan_out = 'bx;
      InputPort ijtag_capture_en = 
          chip_top_rtl1_tessent_tap_main_inst.capture_dr_en;
      InputPort ijtag_shift_en = chip_top_rtl1_tessent_tap_main_inst.shift_dr_en;

      InputPort ijtag_update_en = 
          chip_top_rtl1_tessent_tap_main_inst.update_dr_en;
      InputPort bscan_select = 
          chip_top_rtl1_tessent_tap_main_inst.host_bscan_to_sel;
      InputPort ijtag_tck = TCK;
      InputPort force_disable = 
          chip_top_rtl1_tessent_tap_main_inst.force_disable;
      InputPort select_jtag_input = 
          chip_top_rtl1_tessent_tap_main_inst.select_jtag_input;
      InputPort select_jtag_output = 
          chip_top_rtl1_tessent_tap_main_inst.select_jtag_output;
      InputPort ltest_en = 'b0;
      InputPort occ_en = 'b0;
      InputPort static_clock_control_mode = 'b0;
      InputPort ltest_clock_sequence[1] = 'b0;
      InputPort ltest_clock_sequence[0] = 'b0;
      Attribute tessent_design_instance = 
          "chip_top_rtl1_tessent_bscan_interface_I";
   }
   Instance chip_top_rtl1_tessent_sib_pb1_inst Of chip_top_rtl1_tessent_sib_2 {
      InputPort ijtag_reset = 
          chip_top_rtl1_tessent_tap_main_inst.test_logic_reset;
      InputPort ijtag_sel = chip_top_rtl1_tessent_sib_sri_inst.ijtag_to_sel;
      InputPort ijtag_si = chip_top_rtl1_tessent_sib_pb2_inst.ijtag_so;
      InputPort ijtag_ce = chip_top_rtl1_tessent_tap_main_inst.capture_dr_en;
      InputPort ijtag_se = chip_top_rtl1_tessent_tap_main_inst.shift_dr_en;
      InputPort ijtag_ue = chip_top_rtl1_tessent_tap_main_inst.update_dr_en;
      InputPort ijtag_tck = TCK;
      InputPort ijtag_from_so = GPS_1.ijtag_so;
      Attribute tessent_design_instance = "chip_top_rtl1_tessent_sib_pb1_inst";
   }
   Instance chip_top_rtl1_tessent_sib_pb2_inst Of chip_top_rtl1_tessent_sib_2 {
      InputPort ijtag_reset = 
          chip_top_rtl1_tessent_tap_main_inst.test_logic_reset;
      InputPort ijtag_sel = chip_top_rtl1_tessent_sib_sri_inst.ijtag_to_sel;
      InputPort ijtag_si = chip_top_rtl1_tessent_sib_pb3_inst.ijtag_so;
      InputPort ijtag_ce = chip_top_rtl1_tessent_tap_main_inst.capture_dr_en;
      InputPort ijtag_se = chip_top_rtl1_tessent_tap_main_inst.shift_dr_en;
      InputPort ijtag_ue = chip_top_rtl1_tessent_tap_main_inst.update_dr_en;
      InputPort ijtag_tck = TCK;
      InputPort ijtag_from_so = GPS_2.ijtag_so;
      Attribute tessent_design_instance = "chip_top_rtl1_tessent_sib_pb2_inst";
   }
   Instance chip_top_rtl1_tessent_sib_pb3_inst Of chip_top_rtl1_tessent_sib_2 {
      InputPort ijtag_reset = 
          chip_top_rtl1_tessent_tap_main_inst.test_logic_reset;
      InputPort ijtag_sel = chip_top_rtl1_tessent_sib_sri_inst.ijtag_to_sel;
      InputPort ijtag_si = chip_top_rtl1_tessent_sib_sri_ctrl_inst.ijtag_so;
      InputPort ijtag_ce = chip_top_rtl1_tessent_tap_main_inst.capture_dr_en;
      InputPort ijtag_se = chip_top_rtl1_tessent_tap_main_inst.shift_dr_en;
      InputPort ijtag_ue = chip_top_rtl1_tessent_tap_main_inst.update_dr_en;
      InputPort ijtag_tck = TCK;
      InputPort ijtag_from_so = PROCESSOR_1.ijtag_so;
      Attribute tessent_design_instance = "chip_top_rtl1_tessent_sib_pb3_inst";
   }
   Instance chip_top_rtl1_tessent_sib_sri_ctrl_inst Of 
       chip_top_rtl1_tessent_sib_2 {
      InputPort ijtag_reset = 
          chip_top_rtl1_tessent_tap_main_inst.test_logic_reset;
      InputPort ijtag_sel = chip_top_rtl1_tessent_sib_sri_inst.ijtag_to_sel;
      InputPort ijtag_si = TDI;
      InputPort ijtag_ce = chip_top_rtl1_tessent_tap_main_inst.capture_dr_en;
      InputPort ijtag_se = chip_top_rtl1_tessent_tap_main_inst.shift_dr_en;
      InputPort ijtag_ue = chip_top_rtl1_tessent_tap_main_inst.update_dr_en;
      InputPort ijtag_tck = TCK;
      InputPort ijtag_from_so = chip_top_rtl1_tessent_tdr_sri_ctrl_inst.ijtag_so;

      Attribute tessent_design_instance = 
          "chip_top_rtl1_tessent_sib_sri_ctrl_inst";
   }
   Instance chip_top_rtl1_tessent_sib_sri_inst Of chip_top_rtl1_tessent_sib_1 {
      InputPort ijtag_reset = 
          chip_top_rtl1_tessent_tap_main_inst.test_logic_reset;
      InputPort ijtag_sel = chip_top_rtl1_tessent_tap_main_inst.host_1_to_sel;
      InputPort ijtag_si = TDI;
      InputPort ijtag_ce = chip_top_rtl1_tessent_tap_main_inst.capture_dr_en;
      InputPort ijtag_se = chip_top_rtl1_tessent_tap_main_inst.shift_dr_en;
      InputPort ijtag_ue = chip_top_rtl1_tessent_tap_main_inst.update_dr_en;
      InputPort ijtag_tck = TCK;
      InputPort ijtag_from_so = chip_top_rtl1_tessent_sib_pb1_inst.ijtag_so;
      Attribute tessent_design_instance = "chip_top_rtl1_tessent_sib_sri_inst";
   }
   Instance chip_top_rtl1_tessent_tap_main_inst Of 
       chip_top_rtl1_tessent_tap_main {
      InputPort tck = TCK;
      InputPort tdi = TDI;
      InputPort tms = TMS;
      InputPort trst = TRST;
      InputPort host_1_from_so = chip_top_rtl1_tessent_sib_sri_inst.ijtag_so;
      InputPort host_bscan_from_so = 
          chip_top_rtl1_tessent_bscan_interface_I.scan_out;
      Attribute tessent_design_instance = "chip_top_rtl1_tessent_tap_main_inst";
   }
   Instance chip_top_rtl1_tessent_tdr_sri_ctrl_inst Of 
       chip_top_rtl1_tessent_tdr_sri_ctrl {
      InputPort ijtag_reset = 
          chip_top_rtl1_tessent_tap_main_inst.test_logic_reset;
      InputPort ijtag_sel = chip_top_rtl1_tessent_sib_sri_ctrl_inst.ijtag_to_sel;

      InputPort ijtag_si = TDI;
      InputPort ijtag_ce = chip_top_rtl1_tessent_tap_main_inst.capture_dr_en;
      InputPort ijtag_se = chip_top_rtl1_tessent_tap_main_inst.shift_dr_en;
      InputPort ijtag_ue = chip_top_rtl1_tessent_tap_main_inst.update_dr_en;
      InputPort ijtag_tck = TCK;
      Attribute tessent_design_instance = 
          "chip_top_rtl1_tessent_tdr_sri_ctrl_inst";
   }
}
