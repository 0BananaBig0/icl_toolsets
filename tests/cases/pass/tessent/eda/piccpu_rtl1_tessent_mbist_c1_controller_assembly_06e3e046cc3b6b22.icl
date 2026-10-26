//-------------------------------------------------
//  File created by: Tessent Shell
//          Version: 2026.2
//       Created on: Mon Oct 26 08:52:05 CST 2026
//-------------------------------------------------





// instanced as piccpu_rtl1_tessent_mbist_c1_controller_assembly.m1_inst



// instanced as piccpu_rtl1_tessent_mbist_c1_controller_assembly.m1_interface_instance



// instanced as piccpu_rtl1_tessent_mbist_c1_controller_assembly.piccpu_rtl1_tessent_mbist_bap_inst



// instanced as piccpu_rtl1_tessent_mbist_c1_controller_assembly.piccpu_rtl1_tessent_mbist_c1_controller_inst

NameSpace ;
UseNameSpace ;
Module piccpu_rtl1_tessent_mbist_c1_controller_assembly {
   // Created by ICL extraction
   DataOutPort BIST_DONE {
      Source piccpu_rtl1_tessent_mbist_c1_controller_inst.MBISTPG_DONE;
      Attribute tessent_use_in_dft_specification = "false";
   }
   DataOutPort BIST_GO {
      Source piccpu_rtl1_tessent_mbist_c1_controller_inst.MBISTPG_GO;
      Attribute tessent_use_in_dft_specification = "false";
   }
   DataInPort LV_TM {
      DefaultLoadValue 1'b0;
      Attribute tessent_timing = "scan_reconfiguration";
      Attribute connection_rule_option = "allowed_tied_low";
      Attribute tessent_use_in_dft_specification = "false";
   }
   CaptureEnPort capture_en;
   ClockPort clk_ramclk {
      Attribute tessent_clock_domain_labels = "clk_ramclk clk_ramclk";
      Attribute tessent_clock_periods = "all 100.00ns";
   }
   SelectPort ijtag_select;
   ResetPort reset {
      ActivePolarity 0;
   }
   ShiftEnPort shift_en;
   ScanInPort si;
   ScanOutPort so {
      Source piccpu_rtl1_tessent_mbist_bap_inst.so;
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
   Attribute forced_low_input_port_list = "{MCP_BOUNDING_EN} {MEM_BYPASS_EN}";
   Attribute icl_extraction_date = "Mon Oct 26 08:52:05 CST 2026";
   Attribute created_by_tessent_icl_extract = "true";
   Attribute tessent_design_id = "rtl";
   Instance m1_inst Of picdram {
      InputPort address[6] = m1_interface_instance.address[6];
      InputPort address[5] = m1_interface_instance.address[5];
      InputPort address[4] = m1_interface_instance.address[4];
      InputPort address[3] = m1_interface_instance.address[3];
      InputPort address[2] = m1_interface_instance.address[2];
      InputPort address[1] = m1_interface_instance.address[1];
      InputPort address[0] = m1_interface_instance.address[0];
      InputPort clk = clk_ramclk;
      Attribute tessent_design_instance = "m1_inst";
   }
   Instance m1_interface_instance Of piccpu_rtl1_tessent_mbist_c1_interface_m1 {
      InputPort BIST_CLK = clk_ramclk;
      InputPort BIST_COLLAR_EN = 
          piccpu_rtl1_tessent_mbist_c1_controller_inst.BIST_COLLAR_EN0;
      InputPort BIST_ASYNC_RESETN = 
          piccpu_rtl1_tessent_mbist_bap_inst.BIST_ASYNC_RESET;
      InputPort BIST_SI = 
          piccpu_rtl1_tessent_mbist_c1_controller_inst.MEM0_BIST_COLLAR_SI;
      InputPort BIST_SHIFT_COLLAR = 
          piccpu_rtl1_tessent_mbist_c1_controller_inst.BIST_SHIFT_COLLAR;
      InputPort BIST_SETUP0 = piccpu_rtl1_tessent_mbist_bap_inst.BIST_SETUP[0];
      InputPort MEM_BYPASS_EN = 
          piccpu_rtl1_tessent_mbist_bap_inst.memory_bypass_to_en;
      InputPort MCP_BOUNDING_EN = 
          piccpu_rtl1_tessent_mbist_bap_inst.mcp_bounding_to_en;
      InputPort INCLUDE_MEM_RESULTS_REG = 
          piccpu_rtl1_tessent_mbist_bap_inst.INCLUDE_MEM_RESULTS_REG;
      Attribute tessent_design_instance = "m1_interface_instance";
   }
   Instance piccpu_rtl1_tessent_mbist_bap_inst Of piccpu_rtl1_tessent_mbist_bap 
       {
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
          piccpu_rtl1_tessent_mbist_c1_controller_inst.MBISTPG_SO;
      InputPort MBISTPG_GO[0] = 
          piccpu_rtl1_tessent_mbist_c1_controller_inst.MBISTPG_GO;
      InputPort MBISTPG_DONE[0] = 
          piccpu_rtl1_tessent_mbist_c1_controller_inst.MBISTPG_DONE;
      Attribute tessent_design_instance = "piccpu_rtl1_tessent_mbist_bap_inst";
   }
   Instance piccpu_rtl1_tessent_mbist_c1_controller_inst Of 
       piccpu_rtl1_tessent_mbist_c1_controller {
      InputPort BIST_CLK = clk_ramclk;
      InputPort MBISTPG_EN = piccpu_rtl1_tessent_mbist_bap_inst.bistEn[0];
      InputPort LV_TM = piccpu_rtl1_tessent_mbist_bap_inst.ltest_to_en;
      InputPort MEM_BYPASS_EN = 
          piccpu_rtl1_tessent_mbist_bap_inst.memory_bypass_to_en;
      InputPort MCP_BOUNDING_EN = 
          piccpu_rtl1_tessent_mbist_bap_inst.mcp_bounding_to_en;
      InputPort MBISTPG_ASYNC_RESETN = 
          piccpu_rtl1_tessent_mbist_bap_inst.BIST_ASYNC_RESET;
      InputPort MBISTPG_DIAG_EN = 
          piccpu_rtl1_tessent_mbist_bap_inst.BIST_DIAG_EN;
      InputPort BIST_SETUP2 = piccpu_rtl1_tessent_mbist_bap_inst.BIST_SETUP[2];
      InputPort BIST_SETUP[1] = piccpu_rtl1_tessent_mbist_bap_inst.BIST_SETUP[1];

      InputPort BIST_SETUP[0] = piccpu_rtl1_tessent_mbist_bap_inst.BIST_SETUP[0];

      InputPort MBISTPG_REDUCED_ADDR_CNT_EN = 
          piccpu_rtl1_tessent_mbist_bap_inst.REDUCED_ADDRESS_COUNT;
      InputPort MBISTPG_MEM_RST = 
          piccpu_rtl1_tessent_mbist_bap_inst.ENABLE_MEM_RESET;
      InputPort MBISTPG_TESTDATA_SELECT = 
          piccpu_rtl1_tessent_mbist_bap_inst.BIST_SELECT_TEST_DATA;
      InputPort FL_CNT_MODE[1] = piccpu_rtl1_tessent_mbist_bap_inst.FL_CNT_MODE1;

      InputPort FL_CNT_MODE[0] = piccpu_rtl1_tessent_mbist_bap_inst.FL_CNT_MODE0;

      InputPort MBISTPG_ALGO_MODE[1] = 
          piccpu_rtl1_tessent_mbist_bap_inst.BIST_ALGO_MODE1;
      InputPort MBISTPG_ALGO_MODE[0] = 
          piccpu_rtl1_tessent_mbist_bap_inst.BIST_ALGO_MODE0;
      InputPort TCK = tck;
      InputPort BIST_SI = piccpu_rtl1_tessent_mbist_bap_inst.toBist[0];
      InputPort BIST_HOLD = piccpu_rtl1_tessent_mbist_bap_inst.BIST_HOLD;
      InputPort MEM0_BIST_COLLAR_SO = m1_interface_instance.BIST_SO;
      Attribute tessent_design_instance = 
          "piccpu_rtl1_tessent_mbist_c1_controller_inst";
   }
}
