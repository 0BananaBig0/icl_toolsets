//-------------------------------------------------
//  File created by: Tessent Shell
//          Version: 2026.2
//       Created on: Thu Aug 27 11:28:14 CST 2026
//-------------------------------------------------





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
Module processor_core {
   // ICL module read from source on or near line 8 of file '/home/lianghuaxiao/tessent_2026.2_usage_examples/tessent_example_hierarchical_flow_with_ssn_v2026.2/wrapped_cores/processor_core/tsdb_outdir/dft_inserted_designs/processor_core_rtl2.dft_inserted_design/processor_core.icl'
   ClockPort dco_clk {
      Attribute tessent_clock_domain_labels = "dco_clk dco_clk";
      Attribute tessent_clock_periods = "all 3.00ns";
   }
   CaptureEnPort ijtag_ce;
   ResetPort ijtag_reset {
      ActivePolarity 0;
   }
   ShiftEnPort ijtag_se;
   SelectPort ijtag_sel;
   ScanInPort ijtag_si;
   ScanOutPort ijtag_so {
      Source processor_core_rtl2_tessent_sib_sri_inst.ijtag_so;
   }
   TCKPort ijtag_tck;
   UpdateEnPort ijtag_ue;
   ClockPort ssn_bus_clock {
      Attribute function_modifier = "tessent_ssn_clock";
      Attribute forced_high_dft_signal_list = "ssn_en";
      Attribute tessent_ssn_datapath_ids = "1";
   }
   DataInPort ssn_bus_data_in[1] {
      Attribute function_modifier = "tessent_ssn_bus_data";
      Attribute forced_high_dft_signal_list = "ssn_en";
      Attribute tessent_ssn_datapath_ids = "1";
      Attribute tessent_ssn_datapath_bus_index = 1;
      Attribute tessent_ssn_clocks = "ssn_bus_clock";
      Attribute tessent_ssn_data_phases = "1 1";
   }
   DataInPort ssn_bus_data_in[0] {
      Attribute function_modifier = "tessent_ssn_bus_data";
      Attribute forced_high_dft_signal_list = "ssn_en";
      Attribute tessent_ssn_datapath_ids = "1";
      Attribute tessent_ssn_datapath_bus_index = 0;
      Attribute tessent_ssn_clocks = "ssn_bus_clock";
      Attribute tessent_ssn_data_phases = "1 1";
   }
   DataOutPort ssn_bus_data_out[1] {
      Source processor_core_rtl2_tessent_ssn_pipe_2_inst.bus_data_out[1];
      Attribute function_modifier = "tessent_ssn_bus_data";
      Attribute forced_high_dft_signal_list = "ssn_en";
      Attribute tessent_ssn_datapath_ids = "1";
      Attribute tessent_ssn_datapath_bus_index = 1;
      Attribute tessent_ssn_clocks = "ssn_bus_clock";
      Attribute tessent_ssn_data_phases = "1 1";
   }
   DataOutPort ssn_bus_data_out[0] {
      Source processor_core_rtl2_tessent_ssn_pipe_2_inst.bus_data_out[0];
      Attribute function_modifier = "tessent_ssn_bus_data";
      Attribute forced_high_dft_signal_list = "ssn_en";
      Attribute tessent_ssn_datapath_ids = "1";
      Attribute tessent_ssn_datapath_bus_index = 0;
      Attribute tessent_ssn_clocks = "ssn_bus_clock";
      Attribute tessent_ssn_data_phases = "1 1";
   }
   ScanInterface ijtag {
      Port ijtag_ce;
      Port ijtag_reset;
      Port ijtag_se;
      Port ijtag_sel;
      Port ijtag_si;
      Port ijtag_so;
      Port ijtag_tck;
      Port ijtag_ue;
   }
   Attribute tessent_design_format = "verilog_2001";
   Attribute test_setup_procfile = "";
   Attribute forced_low_internal_input_port_list = 
       "{processor_core_rtl2_tessent_ssn_scan_host_1_inst/scan_en}";
   Attribute icl_extraction_date = "Thu Aug 27 11:22:50 CST 2026";
   Attribute created_by_tessent_icl_extract = "true";
   Attribute tessent_design_id = "gate";
   Attribute tessent_design_level = "physical_block";
   Attribute tessent_is_physical_module = "true";
   Instance DATA_MEM_1 Of SYNC_1RW_8Kx16 {
      InputPort A[12] = DATA_MEM_1_interface_inst.A[12];
      InputPort A[11] = DATA_MEM_1_interface_inst.A[11];
      InputPort A[10] = DATA_MEM_1_interface_inst.A[10];
      InputPort A[9] = DATA_MEM_1_interface_inst.A[9];
      InputPort A[8] = DATA_MEM_1_interface_inst.A[8];
      InputPort A[7] = DATA_MEM_1_interface_inst.A[7];
      InputPort A[6] = DATA_MEM_1_interface_inst.A[6];
      InputPort A[5] = DATA_MEM_1_interface_inst.A[5];
      InputPort A[4] = DATA_MEM_1_interface_inst.A[4];
      InputPort A[3] = DATA_MEM_1_interface_inst.A[3];
      InputPort A[2] = DATA_MEM_1_interface_inst.A[2];
      InputPort A[1] = DATA_MEM_1_interface_inst.A[1];
      InputPort A[0] = DATA_MEM_1_interface_inst.A[0];
      InputPort CLK = processor_core_rtl2_tessent_occ_dco_clk_inst.clock_out;
      Attribute tessent_design_instance = "DATA_MEM_1";
   }
   Instance DATA_MEM_1_interface_inst Of 
       processor_core_rtl1_tessent_mbist_c1_interface_m1 {
      InputPort BIST_CLK = 
          processor_core_rtl2_tessent_occ_dco_clk_inst.clock_out;
      InputPort BIST_COLLAR_EN = 
          processor_core_rtl1_tessent_mbist_c1_controller_inst.BIST_COLLAR_EN0;
      InputPort BIST_ASYNC_RESETN = 
          processor_core_rtl1_tessent_mbist_bap_inst.BIST_ASYNC_RESET;
      InputPort BIST_SI = 
          processor_core_rtl1_tessent_mbist_c1_controller_inst.MEM0_BIST_COLLAR_SI;

      InputPort BIST_SHIFT_COLLAR = 
          processor_core_rtl1_tessent_mbist_c1_controller_inst.BIST_SHIFT_COLLAR;

      InputPort BIST_SETUP0 = 
          processor_core_rtl1_tessent_mbist_bap_inst.BIST_SETUP[0];
      InputPort MEM_BYPASS_EN = 
          processor_core_rtl1_tessent_mbist_bap_inst.memory_bypass_to_en;
      InputPort MCP_BOUNDING_EN = 
          processor_core_rtl1_tessent_mbist_bap_inst.mcp_bounding_to_en;
      InputPort INCLUDE_MEM_RESULTS_REG = 
          processor_core_rtl1_tessent_mbist_bap_inst.INCLUDE_MEM_RESULTS_REG;
      Attribute tessent_design_instance = "DATA_MEM_1_interface_inst";
   }
   Instance PROG_MEM_2 Of SYNC_1RW_8Kx16 {
      InputPort A[12] = PROG_MEM_2_interface_inst.A[12];
      InputPort A[11] = PROG_MEM_2_interface_inst.A[11];
      InputPort A[10] = PROG_MEM_2_interface_inst.A[10];
      InputPort A[9] = PROG_MEM_2_interface_inst.A[9];
      InputPort A[8] = PROG_MEM_2_interface_inst.A[8];
      InputPort A[7] = PROG_MEM_2_interface_inst.A[7];
      InputPort A[6] = PROG_MEM_2_interface_inst.A[6];
      InputPort A[5] = PROG_MEM_2_interface_inst.A[5];
      InputPort A[4] = PROG_MEM_2_interface_inst.A[4];
      InputPort A[3] = PROG_MEM_2_interface_inst.A[3];
      InputPort A[2] = PROG_MEM_2_interface_inst.A[2];
      InputPort A[1] = PROG_MEM_2_interface_inst.A[1];
      InputPort A[0] = PROG_MEM_2_interface_inst.A[0];
      InputPort CLK = processor_core_rtl2_tessent_occ_dco_clk_inst.clock_out;
      Attribute tessent_design_instance = "PROG_MEM_2";
   }
   Instance PROG_MEM_2_interface_inst Of 
       processor_core_rtl1_tessent_mbist_c1_interface_m2 {
      InputPort BIST_CLK = 
          processor_core_rtl2_tessent_occ_dco_clk_inst.clock_out;
      InputPort BIST_COLLAR_EN = 
          processor_core_rtl1_tessent_mbist_c1_controller_inst.BIST_COLLAR_EN1;
      InputPort BIST_ASYNC_RESETN = 
          processor_core_rtl1_tessent_mbist_bap_inst.BIST_ASYNC_RESET;
      InputPort BIST_SI = 
          processor_core_rtl1_tessent_mbist_c1_controller_inst.MEM1_BIST_COLLAR_SI;

      InputPort BIST_SHIFT_COLLAR = 
          processor_core_rtl1_tessent_mbist_c1_controller_inst.BIST_SHIFT_COLLAR;

      InputPort BIST_SETUP0 = 
          processor_core_rtl1_tessent_mbist_bap_inst.BIST_SETUP[0];
      InputPort MEM_BYPASS_EN = 
          processor_core_rtl1_tessent_mbist_bap_inst.memory_bypass_to_en;
      InputPort MCP_BOUNDING_EN = 
          processor_core_rtl1_tessent_mbist_bap_inst.mcp_bounding_to_en;
      InputPort INCLUDE_MEM_RESULTS_REG = 
          processor_core_rtl1_tessent_mbist_bap_inst.INCLUDE_MEM_RESULTS_REG;
      Attribute tessent_design_instance = "PROG_MEM_2_interface_inst";
   }
   Instance processor_core_rtl1_tessent_mbist_bap_inst Of 
       processor_core_rtl1_tessent_mbist_bap {
      InputPort reset = processor_core_rtl1_tessent_sib_sti_inst.ijtag_to_reset;
      InputPort ijtag_select = 
          processor_core_rtl1_tessent_sib_mbist_inst.ijtag_to_sel;
      InputPort si = processor_core_rtl1_tessent_sib_sti_inst.ijtag_to_si;
      InputPort capture_en = 
          processor_core_rtl1_tessent_sib_sti_inst.ijtag_to_ce;
      InputPort shift_en = processor_core_rtl1_tessent_sib_sti_inst.ijtag_to_se;
      InputPort update_en = processor_core_rtl1_tessent_sib_sti_inst.ijtag_to_ue;

      InputPort tck = processor_core_rtl1_tessent_sib_sti_inst.ijtag_to_tck;
      InputPort memory_bypass_en = 
          processor_core_rtl1_tessent_sib_sti_inst.ltest_to_mem_bypass_en;
      InputPort mcp_bounding_en = 
          processor_core_rtl1_tessent_sib_sti_inst.ltest_to_mcp_bounding_en;
      InputPort ltest_en = processor_core_rtl1_tessent_sib_sti_inst.ltest_to_en;
      InputPort fromBist[0] = 
          processor_core_rtl1_tessent_mbist_c1_controller_inst.MBISTPG_SO;
      InputPort MBISTPG_GO[0] = 
          processor_core_rtl1_tessent_mbist_c1_controller_inst.MBISTPG_GO;
      InputPort MBISTPG_DONE[0] = 
          processor_core_rtl1_tessent_mbist_c1_controller_inst.MBISTPG_DONE;
      Attribute tessent_design_instance = 
          "processor_core_rtl1_tessent_mbist_bap_inst";
   }
   Instance processor_core_rtl1_tessent_mbist_c1_controller_inst Of 
       processor_core_rtl1_tessent_mbist_c1_controller {
      InputPort BIST_CLK = 
          processor_core_rtl2_tessent_occ_dco_clk_inst.clock_out;
      InputPort MBISTPG_EN = 
          processor_core_rtl1_tessent_mbist_bap_inst.bistEn[0];
      InputPort LV_TM = processor_core_rtl1_tessent_mbist_bap_inst.ltest_to_en;
      InputPort MEM_BYPASS_EN = 
          processor_core_rtl1_tessent_mbist_bap_inst.memory_bypass_to_en;
      InputPort MCP_BOUNDING_EN = 
          processor_core_rtl1_tessent_mbist_bap_inst.mcp_bounding_to_en;
      InputPort MBISTPG_ASYNC_RESETN = 
          processor_core_rtl1_tessent_mbist_bap_inst.BIST_ASYNC_RESET;
      InputPort MBISTPG_DIAG_EN = 
          processor_core_rtl1_tessent_mbist_bap_inst.BIST_DIAG_EN;
      InputPort BIST_SETUP2 = 
          processor_core_rtl1_tessent_mbist_bap_inst.BIST_SETUP[2];
      InputPort BIST_SETUP[1] = 
          processor_core_rtl1_tessent_mbist_bap_inst.BIST_SETUP[1];
      InputPort BIST_SETUP[0] = 
          processor_core_rtl1_tessent_mbist_bap_inst.BIST_SETUP[0];
      InputPort MBISTPG_REDUCED_ADDR_CNT_EN = 
          processor_core_rtl1_tessent_mbist_bap_inst.REDUCED_ADDRESS_COUNT;
      InputPort MBISTPG_MEM_RST = 
          processor_core_rtl1_tessent_mbist_bap_inst.ENABLE_MEM_RESET;
      InputPort MBISTPG_TESTDATA_SELECT = 
          processor_core_rtl1_tessent_mbist_bap_inst.BIST_SELECT_TEST_DATA;
      InputPort FL_CNT_MODE[1] = 
          processor_core_rtl1_tessent_mbist_bap_inst.FL_CNT_MODE1;
      InputPort FL_CNT_MODE[0] = 
          processor_core_rtl1_tessent_mbist_bap_inst.FL_CNT_MODE0;
      InputPort MBISTPG_ALGO_MODE[1] = 
          processor_core_rtl1_tessent_mbist_bap_inst.BIST_ALGO_MODE1;
      InputPort MBISTPG_ALGO_MODE[0] = 
          processor_core_rtl1_tessent_mbist_bap_inst.BIST_ALGO_MODE0;
      InputPort TCK = processor_core_rtl1_tessent_sib_sti_inst.ijtag_to_tck;
      InputPort BIST_SI = processor_core_rtl1_tessent_mbist_bap_inst.toBist[0];
      InputPort BIST_HOLD = processor_core_rtl1_tessent_mbist_bap_inst.BIST_HOLD;

      InputPort MEM0_BIST_COLLAR_SO = DATA_MEM_1_interface_inst.BIST_SO;
      InputPort MEM1_BIST_COLLAR_SO = PROG_MEM_2_interface_inst.BIST_SO;
      Attribute tessent_design_instance = 
          "processor_core_rtl1_tessent_mbist_c1_controller_inst";
   }
   Instance processor_core_rtl1_tessent_sib_mbist_inst Of 
       processor_core_rtl1_tessent_sib_2 {
      InputPort ijtag_reset = 
          processor_core_rtl1_tessent_sib_sti_inst.ijtag_to_reset;
      InputPort ijtag_sel = 
          processor_core_rtl1_tessent_sib_sti_inst.ijtag_to_sel;
      InputPort ijtag_si = processor_core_rtl1_tessent_sib_sti_inst.ijtag_to_si;
      InputPort ijtag_ce = processor_core_rtl1_tessent_sib_sti_inst.ijtag_to_ce;
      InputPort ijtag_se = processor_core_rtl1_tessent_sib_sti_inst.ijtag_to_se;
      InputPort ijtag_ue = processor_core_rtl1_tessent_sib_sti_inst.ijtag_to_ue;
      InputPort ijtag_tck = 
          processor_core_rtl1_tessent_sib_sti_inst.ijtag_to_tck;
      InputPort ijtag_from_so = processor_core_rtl1_tessent_mbist_bap_inst.so;
      Attribute tessent_design_instance = 
          "processor_core_rtl1_tessent_sib_mbist_inst";
   }
   Instance processor_core_rtl1_tessent_sib_sti_inst Of 
       processor_core_rtl1_tessent_sib_1 {
      InputPort ijtag_reset = ijtag_reset;
      InputPort ijtag_sel = ijtag_sel;
      InputPort ijtag_si = ijtag_si;
      InputPort ijtag_ce = ijtag_ce;
      InputPort ijtag_se = ijtag_se;
      InputPort ijtag_ue = ijtag_ue;
      InputPort ijtag_tck = ijtag_tck;
      InputPort ijtag_from_so = 
          processor_core_rtl1_tessent_sib_mbist_inst.ijtag_so;
      InputPort ltest_en = 
          processor_core_rtl2_tessent_tdr_sri_ctrl_inst.ltest_en;
      InputPort ltest_occ_en = 
          processor_core_rtl2_tessent_tdr_sri_ctrl_inst.tck_occ_en;
      InputPort ltest_static_clock_control_mode = 'b0;
      InputPort ltest_clock_sequence[1] = 'b0;
      InputPort ltest_clock_sequence[0] = 'b0;
      InputPort ltest_mem_bypass_en = 
          processor_core_rtl2_tessent_tdr_sri_ctrl_inst.memory_bypass_en;
      InputPort ltest_mcp_bounding_en = 'b0;
      Attribute tessent_design_instance = 
          "processor_core_rtl1_tessent_sib_sti_inst";
   }
   Instance processor_core_rtl2_tessent_edt_c1_ext_inst Of 
       processor_core_rtl2_tessent_edt_c1_ext {
      InputPort edt_bypass = 
          processor_core_rtl2_tessent_edt_c1_ext_tdr_inst.edt_bypass;
      Attribute tessent_design_instance = 
          "processor_core_rtl2_tessent_edt_c1_ext_inst";
   }
   Instance processor_core_rtl2_tessent_edt_c1_ext_tdr_inst Of 
       processor_core_rtl2_tessent_edt_c1_ext_tdr {
      InputPort ijtag_reset = ijtag_reset;
      InputPort ijtag_sel = 
          processor_core_rtl2_tessent_sib_edt_inst.ijtag_to_sel;
      InputPort ijtag_si = 
          processor_core_rtl2_tessent_edt_c1_int_tdr_inst.ijtag_so;
      InputPort ijtag_ce = ijtag_ce;
      InputPort ijtag_se = ijtag_se;
      InputPort ijtag_ue = ijtag_ue;
      InputPort ijtag_tck = ijtag_tck;
      Attribute tessent_design_instance = 
          "processor_core_rtl2_tessent_edt_c1_ext_tdr_inst";
   }
   Instance processor_core_rtl2_tessent_edt_c1_int_inst Of 
       processor_core_rtl2_tessent_edt_c1_int {
      InputPort edt_bypass = 
          processor_core_rtl2_tessent_edt_c1_int_tdr_inst.edt_bypass;
      Attribute tessent_design_instance = 
          "processor_core_rtl2_tessent_edt_c1_int_inst";
   }
   Instance processor_core_rtl2_tessent_edt_c1_int_tdr_inst Of 
       processor_core_rtl2_tessent_edt_c1_int_tdr {
      InputPort ijtag_reset = ijtag_reset;
      InputPort ijtag_sel = 
          processor_core_rtl2_tessent_sib_edt_inst.ijtag_to_sel;
      InputPort ijtag_si = processor_core_rtl2_tessent_sib_occ_inst.ijtag_so;
      InputPort ijtag_ce = ijtag_ce;
      InputPort ijtag_se = ijtag_se;
      InputPort ijtag_ue = ijtag_ue;
      InputPort ijtag_tck = ijtag_tck;
      Attribute tessent_design_instance = 
          "processor_core_rtl2_tessent_edt_c1_int_tdr_inst";
   }
   Instance processor_core_rtl2_tessent_occ_dco_clk_inst Of 
       processor_core_rtl2_tessent_occ {
      InputPort fast_clock = dco_clk;
      InputPort ijtag_tck = ijtag_tck;
      InputPort ijtag_si = processor_core_rtl2_tessent_sib_ssn_inst.ijtag_so;
      InputPort ijtag_reset = ijtag_reset;
      InputPort ijtag_ce = ijtag_ce;
      InputPort ijtag_se = ijtag_se;
      InputPort ijtag_ue = ijtag_ue;
      InputPort ijtag_sel = 
          processor_core_rtl2_tessent_sib_occ_inst.ijtag_to_sel;
      Attribute tessent_design_instance = 
          "processor_core_rtl2_tessent_occ_dco_clk_inst";
   }
   Instance processor_core_rtl2_tessent_sib_edt_inst Of 
       processor_core_rtl2_tessent_sib_2 {
      InputPort ijtag_reset = ijtag_reset;
      InputPort ijtag_sel = 
          processor_core_rtl2_tessent_sib_sri_inst.ijtag_to_sel;
      InputPort ijtag_si = processor_core_rtl2_tessent_sib_occ_inst.ijtag_so;
      InputPort ijtag_ce = ijtag_ce;
      InputPort ijtag_se = ijtag_se;
      InputPort ijtag_ue = ijtag_ue;
      InputPort ijtag_tck = ijtag_tck;
      InputPort ijtag_from_so = 
          processor_core_rtl2_tessent_edt_c1_ext_tdr_inst.ijtag_so;
      Attribute tessent_design_instance = 
          "processor_core_rtl2_tessent_sib_edt_inst";
   }
   Instance processor_core_rtl2_tessent_sib_occ_inst Of 
       processor_core_rtl2_tessent_sib_2 {
      InputPort ijtag_reset = ijtag_reset;
      InputPort ijtag_sel = 
          processor_core_rtl2_tessent_sib_sri_inst.ijtag_to_sel;
      InputPort ijtag_si = processor_core_rtl2_tessent_sib_ssn_inst.ijtag_so;
      InputPort ijtag_ce = ijtag_ce;
      InputPort ijtag_se = ijtag_se;
      InputPort ijtag_ue = ijtag_ue;
      InputPort ijtag_tck = ijtag_tck;
      InputPort ijtag_from_so = 
          processor_core_rtl2_tessent_occ_dco_clk_inst.ijtag_so;
      Attribute tessent_design_instance = 
          "processor_core_rtl2_tessent_sib_occ_inst";
   }
   Instance processor_core_rtl2_tessent_sib_sri_ctrl_inst Of 
       processor_core_rtl2_tessent_sib_2 {
      InputPort ijtag_reset = ijtag_reset;
      InputPort ijtag_sel = 
          processor_core_rtl2_tessent_sib_sri_inst.ijtag_to_sel;
      InputPort ijtag_si = processor_core_rtl2_tessent_sib_edt_inst.ijtag_so;
      InputPort ijtag_ce = ijtag_ce;
      InputPort ijtag_se = ijtag_se;
      InputPort ijtag_ue = ijtag_ue;
      InputPort ijtag_tck = ijtag_tck;
      InputPort ijtag_from_so = 
          processor_core_rtl2_tessent_tdr_sri_ctrl_inst.ijtag_so;
      Attribute tessent_design_instance = 
          "processor_core_rtl2_tessent_sib_sri_ctrl_inst";
   }
   Instance processor_core_rtl2_tessent_sib_sri_inst Of 
       processor_core_rtl2_tessent_sib_1 {
      InputPort ijtag_reset = ijtag_reset;
      InputPort ijtag_sel = ijtag_sel;
      InputPort ijtag_si = processor_core_rtl1_tessent_sib_sti_inst.ijtag_so;
      InputPort ijtag_ce = ijtag_ce;
      InputPort ijtag_se = ijtag_se;
      InputPort ijtag_ue = ijtag_ue;
      InputPort ijtag_tck = ijtag_tck;
      InputPort ijtag_from_so = 
          processor_core_rtl2_tessent_sib_sri_ctrl_inst.ijtag_so;
      Attribute tessent_design_instance = 
          "processor_core_rtl2_tessent_sib_sri_inst";
   }
   Instance processor_core_rtl2_tessent_sib_ssn_inst Of 
       processor_core_rtl2_tessent_sib_2 {
      InputPort ijtag_reset = ijtag_reset;
      InputPort ijtag_sel = 
          processor_core_rtl2_tessent_sib_sri_inst.ijtag_to_sel;
      InputPort ijtag_si = processor_core_rtl1_tessent_sib_sti_inst.ijtag_so;
      InputPort ijtag_ce = ijtag_ce;
      InputPort ijtag_se = ijtag_se;
      InputPort ijtag_ue = ijtag_ue;
      InputPort ijtag_tck = ijtag_tck;
      InputPort ijtag_from_so = 
          processor_core_rtl2_tessent_ssn_scan_host_1_inst.ijtag_so;
      Attribute tessent_design_instance = 
          "processor_core_rtl2_tessent_sib_ssn_inst";
   }
   Instance processor_core_rtl2_tessent_ssn_pipe_1_inst Of 
       processor_core_rtl2_tessent_ssn_pipe_w2_1 {
      InputPort ijtag_reset = ijtag_reset;
      InputPort bus_clock = ssn_bus_clock;
      InputPort bus_data_in[1] = ssn_bus_data_in[1];
      InputPort bus_data_in[0] = ssn_bus_data_in[0];
      Attribute tessent_design_instance = 
          "processor_core_rtl2_tessent_ssn_pipe_1_inst";
   }
   Instance processor_core_rtl2_tessent_ssn_pipe_2_inst Of 
       processor_core_rtl2_tessent_ssn_pipe_w2_1 {
      InputPort ijtag_reset = ijtag_reset;
      InputPort bus_clock = ssn_bus_clock;
      InputPort bus_data_in[1] = 
          processor_core_rtl2_tessent_ssn_scan_host_1_inst.bus_data_out[1];
      InputPort bus_data_in[0] = 
          processor_core_rtl2_tessent_ssn_scan_host_1_inst.bus_data_out[0];
      Attribute tessent_design_instance = 
          "processor_core_rtl2_tessent_ssn_pipe_2_inst";
   }
   Instance processor_core_rtl2_tessent_ssn_scan_host_1_inst Of 
       processor_core_rtl2_tessent_ssn_scan_host_1 {
      InputPort bus_clock = ssn_bus_clock;
      InputPort bus_data_in[1] = 
          processor_core_rtl2_tessent_ssn_pipe_1_inst.bus_data_out[1];
      InputPort bus_data_in[0] = 
          processor_core_rtl2_tessent_ssn_pipe_1_inst.bus_data_out[0];
      InputPort ijtag_reset = ijtag_reset;
      InputPort ijtag_tck = ijtag_tck;
      InputPort ijtag_sel = 
          processor_core_rtl2_tessent_sib_ssn_inst.ijtag_to_sel;
      InputPort ijtag_ce = ijtag_ce;
      InputPort ijtag_se = ijtag_se;
      InputPort ijtag_ue = ijtag_ue;
      InputPort ijtag_si = processor_core_rtl1_tessent_sib_sti_inst.ijtag_so;
      Attribute tessent_design_instance = 
          "processor_core_rtl2_tessent_ssn_scan_host_1_inst";
   }
   Instance processor_core_rtl2_tessent_tdr_sri_ctrl_inst Of 
       processor_core_rtl2_tessent_tdr_sri_ctrl {
      InputPort ijtag_reset = ijtag_reset;
      InputPort ijtag_sel = 
          processor_core_rtl2_tessent_sib_sri_ctrl_inst.ijtag_to_sel;
      InputPort ijtag_si = processor_core_rtl2_tessent_sib_edt_inst.ijtag_so;
      InputPort ijtag_ce = ijtag_ce;
      InputPort ijtag_se = ijtag_se;
      InputPort ijtag_ue = ijtag_ue;
      InputPort ijtag_tck = ijtag_tck;
      Attribute tessent_design_instance = 
          "processor_core_rtl2_tessent_tdr_sri_ctrl_inst";
   }
}
