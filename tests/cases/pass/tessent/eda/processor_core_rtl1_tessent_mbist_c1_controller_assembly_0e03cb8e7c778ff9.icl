//-------------------------------------------------
//  File created by: Tessent Shell
//          Version: 2026.2
//       Created on: Mon Oct 26 08:50:43 CST 2026
//-------------------------------------------------





// instanced as processor_core_rtl1_tessent_mbist_c1_controller_assembly.m1_inst
// instanced as processor_core_rtl1_tessent_mbist_c1_controller_assembly.m2_inst



// instanced as processor_core_rtl1_tessent_mbist_c1_controller_assembly.m1_interface_instance



// instanced as processor_core_rtl1_tessent_mbist_c1_controller_assembly.m2_interface_instance



// instanced as processor_core_rtl1_tessent_mbist_c1_controller_assembly.processor_core_rtl1_tessent_mbist_bap_inst



// instanced as processor_core_rtl1_tessent_mbist_c1_controller_assembly.processor_core_rtl1_tessent_mbist_c1_controller_inst

NameSpace ;
UseNameSpace ;
Module processor_core_rtl1_tessent_mbist_c1_controller_assembly {
   // Created by ICL extraction
   DataOutPort BIST_DONE {
      Source processor_core_rtl1_tessent_mbist_c1_controller_inst.MBISTPG_DONE;
      Attribute tessent_use_in_dft_specification = "false";
   }
   DataOutPort BIST_GO {
      Source processor_core_rtl1_tessent_mbist_c1_controller_inst.MBISTPG_GO;
      Attribute tessent_use_in_dft_specification = "false";
   }
   DataInPort LV_TM {
      DefaultLoadValue 1'b0;
      Attribute tessent_timing = "scan_reconfiguration";
      Attribute connection_rule_option = "allowed_tied_low";
      Attribute tessent_use_in_dft_specification = "false";
   }
   CaptureEnPort capture_en;
   ClockPort clk_dco_clk {
      Attribute tessent_clock_domain_labels = "clk_dco_clk clk_dco_clk";
      Attribute tessent_clock_periods = "all 3.00ns";
   }
   SelectPort ijtag_select;
   ResetPort reset {
      ActivePolarity 0;
   }
   ShiftEnPort shift_en;
   ScanInPort si;
   ScanOutPort so {
      Source processor_core_rtl1_tessent_mbist_bap_inst.so;
   }
   TCKPort tck;
   UpdateEnPort update_en;
   ScanInterface C0 {
      Port si;
      Port so;
      Port ijtag_select;
      Port shift_en;
      Port capture_en;
      Port update_en;
      Port reset;
      Port tck;
   }
   Attribute tessent_design_format = "verilog_2001";
   Attribute test_setup_procfile = "";
   Attribute forced_low_input_port_list = 
       "{MCP_BOUNDING_EN} {MEM_BYPASS_EN} {m1_inst_OEN} {m2_inst_OEN}";
   Attribute icl_extraction_date = "Mon Oct 26 08:50:42 CST 2026";
   Attribute created_by_tessent_icl_extract = "true";
   Attribute tessent_design_id = "rtl";
   Instance m1_inst Of SYNC_1RW_8Kx16 {
      InputPort A[12] = m1_interface_instance.A[12];
      InputPort A[11] = m1_interface_instance.A[11];
      InputPort A[10] = m1_interface_instance.A[10];
      InputPort A[9] = m1_interface_instance.A[9];
      InputPort A[8] = m1_interface_instance.A[8];
      InputPort A[7] = m1_interface_instance.A[7];
      InputPort A[6] = m1_interface_instance.A[6];
      InputPort A[5] = m1_interface_instance.A[5];
      InputPort A[4] = m1_interface_instance.A[4];
      InputPort A[3] = m1_interface_instance.A[3];
      InputPort A[2] = m1_interface_instance.A[2];
      InputPort A[1] = m1_interface_instance.A[1];
      InputPort A[0] = m1_interface_instance.A[0];
      InputPort CLK = clk_dco_clk;
      Attribute tessent_design_instance = "m1_inst";
   }
   Instance m1_interface_instance Of 
       processor_core_rtl1_tessent_mbist_c1_interface_m1 {
      InputPort BIST_CLK = clk_dco_clk;
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
      Attribute tessent_design_instance = "m1_interface_instance";
   }
   Instance m2_inst Of SYNC_1RW_8Kx16 {
      InputPort A[12] = m2_interface_instance.A[12];
      InputPort A[11] = m2_interface_instance.A[11];
      InputPort A[10] = m2_interface_instance.A[10];
      InputPort A[9] = m2_interface_instance.A[9];
      InputPort A[8] = m2_interface_instance.A[8];
      InputPort A[7] = m2_interface_instance.A[7];
      InputPort A[6] = m2_interface_instance.A[6];
      InputPort A[5] = m2_interface_instance.A[5];
      InputPort A[4] = m2_interface_instance.A[4];
      InputPort A[3] = m2_interface_instance.A[3];
      InputPort A[2] = m2_interface_instance.A[2];
      InputPort A[1] = m2_interface_instance.A[1];
      InputPort A[0] = m2_interface_instance.A[0];
      InputPort CLK = clk_dco_clk;
      Attribute tessent_design_instance = "m2_inst";
   }
   Instance m2_interface_instance Of 
       processor_core_rtl1_tessent_mbist_c1_interface_m2 {
      InputPort BIST_CLK = clk_dco_clk;
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
      Attribute tessent_design_instance = "m2_interface_instance";
   }
   Instance processor_core_rtl1_tessent_mbist_bap_inst Of 
       processor_core_rtl1_tessent_mbist_bap {
      InputPort reset = reset;
      InputPort ijtag_select = ijtag_select;
      InputPort si = si;
      InputPort capture_en = capture_en;
      InputPort shift_en = shift_en;
      InputPort update_en = update_en;
      InputPort tck = tck;
      InputPort memory_bypass_en = 'b0;
      InputPort mcp_bounding_en = 'b0;
      InputPort ltest_en = LV_TM;
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
      InputPort BIST_CLK = clk_dco_clk;
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
      InputPort TCK = tck;
      InputPort BIST_SI = processor_core_rtl1_tessent_mbist_bap_inst.toBist[0];
      InputPort BIST_HOLD = processor_core_rtl1_tessent_mbist_bap_inst.BIST_HOLD;

      InputPort MEM0_BIST_COLLAR_SO = m1_interface_instance.BIST_SO;
      InputPort MEM1_BIST_COLLAR_SO = m2_interface_instance.BIST_SO;
      Attribute tessent_design_instance = 
          "processor_core_rtl1_tessent_mbist_c1_controller_inst";
   }
}
