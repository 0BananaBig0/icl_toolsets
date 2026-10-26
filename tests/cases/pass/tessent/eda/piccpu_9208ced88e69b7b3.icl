//-------------------------------------------------
//  File created by: Tessent Shell
//          Version: 2026.2
//       Created on: Mon Oct 26 08:55:11 CST 2026
//-------------------------------------------------





// instanced as piccpu.piccpu_rtl1_tessent_mbist_bap_inst



// instanced as piccpu.piccpu_rtl1_tessent_sib_mbist_inst



// instanced as piccpu.piccpu_rtl1_tessent_sib_sri_inst



// instanced as piccpu.piccpu_rtl1_tessent_sib_sti_inst



// instanced as piccpu.piccpu_rtl1_tessent_tdr_sri_ctrl_inst



// instanced as piccpu.piccpu_rtl2_tessent_edt_lbist_c0_inst



// instanced as piccpu_rtl2_tessent_edt_lbist_c0.piccpu_rtl2_tessent_edt_c0_sib_decompressor_i
// instanced as piccpu_rtl2_tessent_edt_lbist_c0.piccpu_rtl2_tessent_edt_c0_sib_lbist_lp_static_control_i
// instanced as piccpu_rtl2_tessent_edt_lbist_c0.piccpu_rtl2_tessent_edt_c0_sib_lbist_lp_mask_shift_reg_i
// instanced as piccpu_rtl2_tessent_edt_lbist_c0.piccpu_rtl2_tessent_edt_c0_sib_chain_mask_i
// instanced as piccpu_rtl2_tessent_edt_lbist_c0.piccpu_rtl2_tessent_edt_c0_sib_misr_i



// instanced as piccpu.piccpu_rtl2_tessent_edt_lbist_c0_tdr_inst



// instanced as piccpu.piccpu_rtl2_tessent_lbist_inst



// instanced as piccpu_rtl2_tessent_lbist.piccpu_rtl2_tessent_lbist_sib_bist_registers_i
// instanced as piccpu_rtl2_tessent_lbist.piccpu_rtl2_tessent_lbist_sib_ncp_limits_sib_i
// instanced as piccpu_rtl2_tessent_lbist.piccpu_rtl2_tessent_lbist_sib_control_registers_i
// instanced as piccpu_rtl2_tessent_lbist.piccpu_rtl2_tessent_lbist_edt_sib_i



// instanced as piccpu.piccpu_rtl2_tessent_lbist_ncp_index_decoder_inst



// instanced as piccpu.piccpu_rtl2_tessent_occ_clk_inst
// instanced as piccpu.piccpu_rtl2_tessent_occ_ramclk_inst



// instanced as piccpu_rtl2_tessent_occ.tdr_sib



// instanced as piccpu.piccpu_rtl2_tessent_sib_edt_inst
// instanced as piccpu.piccpu_rtl2_tessent_sib_lbist_inst
// instanced as piccpu.piccpu_rtl2_tessent_sib_occ_inst
// instanced as piccpu.piccpu_rtl2_tessent_sib_sri_ctrl_inst



// instanced as piccpu.piccpu_rtl2_tessent_single_chain_mode_logic_inst



// instanced as piccpu_rtl2_tessent_single_chain_mode_logic.tdr_sib_i
// instanced as piccpu_rtl2_tessent_single_chain_mode_logic.single_chain_sib_i
// instanced as piccpu_rtl2_tessent_edt_internal_scan_registers.blk1_sib_i



// instanced as piccpu_rtl2_tessent_single_chain_mode_logic.piccpu_rtl2_tessent_edt_internal_scan_registers_i



// instanced as piccpu.piccpu_rtl2_tessent_tdr_sri_ctrl_inst



// instanced as piccpu.regs_piccpu_rtl1_tessent_mbist_c1_controller_inst



// instanced as piccpu.regs_picdram



// instanced as piccpu.regs_picdram_interface_inst

NameSpace ;
UseNameSpace ;
Module piccpu {
   // ICL module read from source on or near line 8 of file '/home/lianghuaxiao/workspaces/tessent_2026.2_usage_examples/tessent_example_hybrid_tk_lbist_flow_v2026.2/tsdb_outdir/dft_inserted_designs/piccpu_rtl2.dft_inserted_design/piccpu.icl'
   ClockPort clk {
      Attribute tessent_clock_domain_labels = "clk clk";
      Attribute tessent_clock_periods = "all 100.00ns";
   }
   CaptureEnPort ijtag_ce;
   ResetPort ijtag_reset {
      ActivePolarity 0;
   }
   ShiftEnPort ijtag_se;
   SelectPort ijtag_sel;
   ScanInPort ijtag_si;
   ScanOutPort ijtag_so {
      Source piccpu_rtl1_tessent_sib_sri_inst.ijtag_so;
   }
   TCKPort ijtag_tck;
   UpdateEnPort ijtag_ue;
   ClockPort ramclk {
      Attribute tessent_clock_domain_labels = "ramclk ramclk";
      Attribute tessent_clock_periods = "all 100.00ns";
   }
   ClockPort test_clock {
      Attribute function_modifier = "sync_tester_clock";
      Attribute forced_high_dft_signal_list = "ltest_en";
      Attribute connection_rule_option = "allowed_no_source";
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
       "{piccpu_rtl2_tessent_lbist_inst/scan_en_out}";
   Attribute icl_extraction_date = "Mon Oct 26 08:53:24 CST 2026";
   Attribute created_by_tessent_icl_extract = "true";
   Attribute tessent_design_id = "gate1";
   Attribute tessent_design_level = "physical_block";
   Attribute tessent_is_physical_module = "true";
   Instance piccpu_rtl1_tessent_mbist_bap_inst Of piccpu_rtl1_tessent_mbist_bap 
       {
      InputPort reset = piccpu_rtl1_tessent_sib_sti_inst.ijtag_to_reset;
      InputPort ijtag_select = piccpu_rtl1_tessent_sib_mbist_inst.ijtag_to_sel;
      InputPort si = piccpu_rtl1_tessent_sib_sti_inst.ijtag_to_si;
      InputPort capture_en = piccpu_rtl1_tessent_sib_sti_inst.ijtag_to_ce;
      InputPort shift_en = piccpu_rtl1_tessent_sib_sti_inst.ijtag_to_se;
      InputPort update_en = piccpu_rtl1_tessent_sib_sti_inst.ijtag_to_ue;
      InputPort tck = piccpu_rtl1_tessent_sib_sti_inst.ijtag_to_tck;
      InputPort memory_bypass_en = 
          piccpu_rtl1_tessent_sib_sti_inst.ltest_to_mem_bypass_en;
      InputPort mcp_bounding_en = 
          piccpu_rtl1_tessent_sib_sti_inst.ltest_to_mcp_bounding_en;
      InputPort ltest_en = piccpu_rtl1_tessent_sib_sti_inst.ltest_to_en;
      InputPort fromBist[0] = 
          regs_piccpu_rtl1_tessent_mbist_c1_controller_inst.MBISTPG_SO;
      InputPort MBISTPG_GO[0] = 
          regs_piccpu_rtl1_tessent_mbist_c1_controller_inst.MBISTPG_GO;
      InputPort MBISTPG_DONE[0] = 
          regs_piccpu_rtl1_tessent_mbist_c1_controller_inst.MBISTPG_DONE;
      Attribute tessent_design_instance = "piccpu_rtl1_tessent_mbist_bap_inst";
   }
   Instance piccpu_rtl1_tessent_sib_mbist_inst Of piccpu_rtl1_tessent_sib_3 {
      InputPort ijtag_reset = piccpu_rtl1_tessent_sib_sti_inst.ijtag_to_reset;
      InputPort ijtag_sel = piccpu_rtl1_tessent_sib_sti_inst.ijtag_to_sel;
      InputPort ijtag_si = piccpu_rtl1_tessent_sib_sti_inst.ijtag_to_si;
      InputPort ijtag_ce = piccpu_rtl1_tessent_sib_sti_inst.ijtag_to_ce;
      InputPort ijtag_se = piccpu_rtl1_tessent_sib_sti_inst.ijtag_to_se;
      InputPort ijtag_ue = piccpu_rtl1_tessent_sib_sti_inst.ijtag_to_ue;
      InputPort ijtag_tck = piccpu_rtl1_tessent_sib_sti_inst.ijtag_to_tck;
      InputPort ijtag_from_so = piccpu_rtl1_tessent_mbist_bap_inst.so;
      Attribute tessent_design_instance = "piccpu_rtl1_tessent_sib_mbist_inst";
   }
   Instance piccpu_rtl1_tessent_sib_sri_inst Of piccpu_rtl1_tessent_sib_2 {
      InputPort ijtag_reset = ijtag_reset;
      InputPort ijtag_sel = ijtag_sel;
      InputPort ijtag_si = piccpu_rtl1_tessent_sib_sti_inst.ijtag_so;
      InputPort ijtag_ce = ijtag_ce;
      InputPort ijtag_se = ijtag_se;
      InputPort ijtag_ue = ijtag_ue;
      InputPort ijtag_tck = ijtag_tck;
      InputPort ijtag_from_so = piccpu_rtl2_tessent_sib_sri_ctrl_inst.ijtag_so;
      Attribute tessent_design_instance = "piccpu_rtl1_tessent_sib_sri_inst";
   }
   Instance piccpu_rtl1_tessent_sib_sti_inst Of piccpu_rtl1_tessent_sib_1 {
      InputPort ijtag_reset = ijtag_reset;
      InputPort ijtag_sel = ijtag_sel;
      InputPort ijtag_si = ijtag_si;
      InputPort ijtag_ce = ijtag_ce;
      InputPort ijtag_se = ijtag_se;
      InputPort ijtag_ue = ijtag_ue;
      InputPort ijtag_tck = ijtag_tck;
      InputPort ijtag_from_so = piccpu_rtl1_tessent_sib_mbist_inst.ijtag_so;
      InputPort ltest_en = piccpu_rtl2_tessent_tdr_sri_ctrl_inst.ltest_en;
      InputPort ltest_occ_en = piccpu_rtl2_tessent_tdr_sri_ctrl_inst.tck_occ_en;
      InputPort ltest_static_clock_control_mode = 
          piccpu_rtl2_tessent_lbist_inst.lbist_en;
      InputPort ltest_clock_sequence[1] = 
          piccpu_rtl2_tessent_lbist_ncp_index_decoder_inst.occ1_clock_sequence[1];

      InputPort ltest_clock_sequence[0] = 
          piccpu_rtl2_tessent_lbist_ncp_index_decoder_inst.occ1_clock_sequence[0];

      InputPort ltest_mem_bypass_en = 
          piccpu_rtl2_tessent_tdr_sri_ctrl_inst.memory_bypass_en;
      InputPort ltest_mcp_bounding_en = 'b0;
      Attribute tessent_design_instance = "piccpu_rtl1_tessent_sib_sti_inst";
   }
   Instance piccpu_rtl1_tessent_tdr_sri_ctrl_inst Of 
       piccpu_rtl1_tessent_tdr_sri_ctrl {
      InputPort ijtag_reset = ijtag_reset;
      InputPort ijtag_sel = piccpu_rtl1_tessent_sib_sri_inst.ijtag_to_sel;
      InputPort ijtag_si = piccpu_rtl1_tessent_sib_sti_inst.ijtag_so;
      InputPort ijtag_ce = ijtag_ce;
      InputPort ijtag_se = ijtag_se;
      InputPort ijtag_ue = ijtag_ue;
      InputPort ijtag_tck = ijtag_tck;
      Attribute tessent_design_instance = 
          "piccpu_rtl1_tessent_tdr_sri_ctrl_inst";
   }
   Instance piccpu_rtl2_tessent_edt_lbist_c0_inst Of 
       piccpu_rtl2_tessent_edt_lbist_c0 {
      InputPort lbist_en = piccpu_rtl2_tessent_lbist_inst.lbist_en;
      InputPort ijtag_si = piccpu_rtl2_tessent_lbist_inst.to_edt_scan_in;
      InputPort edt_bypass = 
          piccpu_rtl2_tessent_edt_lbist_c0_tdr_inst.edt_bypass;
      InputPort edt_single_bypass_chain = 
          piccpu_rtl2_tessent_single_chain_mode_logic_inst.edt_single_bypass_chain_out;

      InputPort edt_low_power_shift_en = 
          piccpu_rtl2_tessent_edt_lbist_c0_tdr_inst.edt_low_power_shift_en;
      InputPort ccm_en = piccpu_rtl1_tessent_tdr_sri_ctrl_inst.HTKLB_CCM_EN;
      InputPort ijtag_tck = ijtag_tck;
      InputPort ijtag_reset = ijtag_reset;
      InputPort ijtag_sel = piccpu_rtl2_tessent_lbist_inst.edt_sib_en;
      InputPort ijtag_ce = ijtag_ce;
      InputPort ijtag_se = ijtag_se;
      InputPort ijtag_ue = ijtag_ue;
      Attribute tessent_design_instance = 
          "piccpu_rtl2_tessent_edt_lbist_c0_inst";
   }
   Instance piccpu_rtl2_tessent_edt_lbist_c0_tdr_inst Of 
       piccpu_rtl2_tessent_edt_lbist_c0_tdr {
      InputPort ijtag_reset = ijtag_reset;
      InputPort ijtag_sel = piccpu_rtl2_tessent_sib_edt_inst.ijtag_to_sel;
      InputPort ijtag_si = piccpu_rtl2_tessent_sib_lbist_inst.ijtag_so;
      InputPort ijtag_ce = ijtag_ce;
      InputPort ijtag_se = ijtag_se;
      InputPort ijtag_ue = ijtag_ue;
      InputPort ijtag_tck = ijtag_tck;
      Attribute tessent_design_instance = 
          "piccpu_rtl2_tessent_edt_lbist_c0_tdr_inst";
   }
   Instance piccpu_rtl2_tessent_lbist_inst Of piccpu_rtl2_tessent_lbist {
      InputPort ijtag_tck = ijtag_tck;
      InputPort test_clock = test_clock;
      InputPort shift_clock_src = clk;
      InputPort from_edt_scan_out = 
          piccpu_rtl2_tessent_edt_lbist_c0_inst.ijtag_so;
      InputPort ijtag_si = 
          piccpu_rtl2_tessent_single_chain_mode_logic_inst.ijtag_so;
      InputPort ccm_en = piccpu_rtl1_tessent_tdr_sri_ctrl_inst.HTKLB_CCM_EN;
      InputPort ijtag_reset = ijtag_reset;
      InputPort ijtag_sel = piccpu_rtl2_tessent_sib_lbist_inst.ijtag_to_sel;
      InputPort ijtag_ce = ijtag_ce;
      InputPort ijtag_se = ijtag_se;
      InputPort ijtag_ue = ijtag_ue;
      Attribute tessent_design_instance = "piccpu_rtl2_tessent_lbist_inst";
   }
   Instance piccpu_rtl2_tessent_lbist_ncp_index_decoder_inst Of 
       piccpu_rtl2_tessent_lbist_ncp_index_decoder {
      InputPort ncp_index[1] = piccpu_rtl2_tessent_lbist_inst.ncp[1];
      InputPort ncp_index[0] = piccpu_rtl2_tessent_lbist_inst.ncp[0];
      Attribute tessent_design_instance = 
          "piccpu_rtl2_tessent_lbist_ncp_index_decoder_inst";
   }
   Instance piccpu_rtl2_tessent_occ_clk_inst Of piccpu_rtl2_tessent_occ {
      InputPort fast_clock = clk;
      InputPort ijtag_tck = ijtag_tck;
      InputPort ijtag_si = piccpu_rtl2_tessent_sib_edt_inst.ijtag_so;
      InputPort ijtag_reset = ijtag_reset;
      InputPort ijtag_ce = ijtag_ce;
      InputPort ijtag_se = ijtag_se;
      InputPort ijtag_ue = ijtag_ue;
      InputPort ijtag_sel = piccpu_rtl2_tessent_sib_occ_inst.ijtag_to_sel;
      InputPort static_clock_control_mode = 
          piccpu_rtl2_tessent_lbist_inst.lbist_en;
      InputPort clock_sequence[2] = 
          piccpu_rtl2_tessent_lbist_ncp_index_decoder_inst.occ2_clock_sequence[2];

      InputPort clock_sequence[1] = 
          piccpu_rtl2_tessent_lbist_ncp_index_decoder_inst.occ2_clock_sequence[1];

      InputPort clock_sequence[0] = 
          piccpu_rtl2_tessent_lbist_ncp_index_decoder_inst.occ2_clock_sequence[0];

      Attribute tessent_design_instance = "piccpu_rtl2_tessent_occ_clk_inst";
   }
   Instance piccpu_rtl2_tessent_occ_ramclk_inst Of piccpu_rtl2_tessent_occ {
      InputPort fast_clock = ramclk;
      InputPort ijtag_tck = ijtag_tck;
      InputPort ijtag_si = piccpu_rtl2_tessent_occ_clk_inst.ijtag_so;
      InputPort ijtag_reset = ijtag_reset;
      InputPort ijtag_ce = ijtag_ce;
      InputPort ijtag_se = ijtag_se;
      InputPort ijtag_ue = ijtag_ue;
      InputPort ijtag_sel = piccpu_rtl2_tessent_sib_occ_inst.ijtag_to_sel;
      InputPort static_clock_control_mode = 
          piccpu_rtl2_tessent_lbist_inst.lbist_en;
      InputPort clock_sequence[2] = 
          piccpu_rtl2_tessent_lbist_ncp_index_decoder_inst.occ3_clock_sequence[2];

      InputPort clock_sequence[1] = 
          piccpu_rtl2_tessent_lbist_ncp_index_decoder_inst.occ3_clock_sequence[1];

      InputPort clock_sequence[0] = 
          piccpu_rtl2_tessent_lbist_ncp_index_decoder_inst.occ3_clock_sequence[0];

      Attribute tessent_design_instance = "piccpu_rtl2_tessent_occ_ramclk_inst";
   }
   Instance piccpu_rtl2_tessent_sib_edt_inst Of piccpu_rtl2_tessent_sib_1 {
      InputPort ijtag_reset = ijtag_reset;
      InputPort ijtag_sel = piccpu_rtl1_tessent_sib_sri_inst.ijtag_to_sel;
      InputPort ijtag_si = piccpu_rtl2_tessent_sib_lbist_inst.ijtag_so;
      InputPort ijtag_ce = ijtag_ce;
      InputPort ijtag_se = ijtag_se;
      InputPort ijtag_ue = ijtag_ue;
      InputPort ijtag_tck = ijtag_tck;
      InputPort ijtag_from_so = 
          piccpu_rtl2_tessent_edt_lbist_c0_tdr_inst.ijtag_so;
      Attribute tessent_design_instance = "piccpu_rtl2_tessent_sib_edt_inst";
   }
   Instance piccpu_rtl2_tessent_sib_lbist_inst Of piccpu_rtl2_tessent_sib_1 {
      InputPort ijtag_reset = ijtag_reset;
      InputPort ijtag_sel = piccpu_rtl1_tessent_sib_sri_inst.ijtag_to_sel;
      InputPort ijtag_si = piccpu_rtl1_tessent_tdr_sri_ctrl_inst.ijtag_so;
      InputPort ijtag_ce = ijtag_ce;
      InputPort ijtag_se = ijtag_se;
      InputPort ijtag_ue = ijtag_ue;
      InputPort ijtag_tck = ijtag_tck;
      InputPort ijtag_from_so = piccpu_rtl2_tessent_lbist_inst.ijtag_so;
      Attribute tessent_design_instance = "piccpu_rtl2_tessent_sib_lbist_inst";
   }
   Instance piccpu_rtl2_tessent_sib_occ_inst Of piccpu_rtl2_tessent_sib_1 {
      InputPort ijtag_reset = ijtag_reset;
      InputPort ijtag_sel = piccpu_rtl1_tessent_sib_sri_inst.ijtag_to_sel;
      InputPort ijtag_si = piccpu_rtl2_tessent_sib_edt_inst.ijtag_so;
      InputPort ijtag_ce = ijtag_ce;
      InputPort ijtag_se = ijtag_se;
      InputPort ijtag_ue = ijtag_ue;
      InputPort ijtag_tck = ijtag_tck;
      InputPort ijtag_from_so = piccpu_rtl2_tessent_occ_ramclk_inst.ijtag_so;
      Attribute tessent_design_instance = "piccpu_rtl2_tessent_sib_occ_inst";
   }
   Instance piccpu_rtl2_tessent_sib_sri_ctrl_inst Of piccpu_rtl2_tessent_sib_1 {
      InputPort ijtag_reset = ijtag_reset;
      InputPort ijtag_sel = piccpu_rtl1_tessent_sib_sri_inst.ijtag_to_sel;
      InputPort ijtag_si = piccpu_rtl2_tessent_sib_occ_inst.ijtag_so;
      InputPort ijtag_ce = ijtag_ce;
      InputPort ijtag_se = ijtag_se;
      InputPort ijtag_ue = ijtag_ue;
      InputPort ijtag_tck = ijtag_tck;
      InputPort ijtag_from_so = piccpu_rtl2_tessent_tdr_sri_ctrl_inst.ijtag_so;
      Attribute tessent_design_instance = 
          "piccpu_rtl2_tessent_sib_sri_ctrl_inst";
   }
   Instance piccpu_rtl2_tessent_single_chain_mode_logic_inst Of 
       piccpu_rtl2_tessent_single_chain_mode_logic {
      InputPort ijtag_si = piccpu_rtl1_tessent_tdr_sri_ctrl_inst.ijtag_so;
      InputPort ijtag_tck = ijtag_tck;
      InputPort ijtag_reset = ijtag_reset;
      InputPort ijtag_sel = piccpu_rtl2_tessent_sib_lbist_inst.ijtag_to_sel;
      InputPort ijtag_ce = ijtag_ce;
      InputPort ijtag_se = ijtag_se;
      InputPort ijtag_ue = ijtag_ue;
      InputPort edt_single_bypass_chain_in = 'b0;
      Attribute tessent_design_instance = 
          "piccpu_rtl2_tessent_single_chain_mode_logic_inst";
   }
   Instance piccpu_rtl2_tessent_tdr_sri_ctrl_inst Of 
       piccpu_rtl2_tessent_tdr_sri_ctrl {
      InputPort ijtag_reset = ijtag_reset;
      InputPort ijtag_sel = piccpu_rtl2_tessent_sib_sri_ctrl_inst.ijtag_to_sel;
      InputPort ijtag_si = piccpu_rtl2_tessent_sib_occ_inst.ijtag_so;
      InputPort ijtag_ce = ijtag_ce;
      InputPort ijtag_se = ijtag_se;
      InputPort ijtag_ue = ijtag_ue;
      InputPort ijtag_tck = ijtag_tck;
      Attribute tessent_design_instance = 
          "piccpu_rtl2_tessent_tdr_sri_ctrl_inst";
   }
   Instance regs_piccpu_rtl1_tessent_mbist_c1_controller_inst Of 
       piccpu_rtl1_tessent_mbist_c1_controller {
      InputPort BIST_CLK = piccpu_rtl2_tessent_occ_ramclk_inst.clock_out;
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
      InputPort TCK = piccpu_rtl1_tessent_sib_sti_inst.ijtag_to_tck;
      InputPort BIST_SI = piccpu_rtl1_tessent_mbist_bap_inst.toBist[0];
      InputPort BIST_HOLD = piccpu_rtl1_tessent_mbist_bap_inst.BIST_HOLD;
      InputPort MEM0_BIST_COLLAR_SO = regs_picdram_interface_inst.BIST_SO;
      Attribute tessent_design_instance = 
          "regs/piccpu_rtl1_tessent_mbist_c1_controller_inst";
   }
   Instance regs_picdram Of picdram {
      InputPort address[6] = regs_picdram_interface_inst.address[6];
      InputPort address[5] = regs_picdram_interface_inst.address[5];
      InputPort address[4] = regs_picdram_interface_inst.address[4];
      InputPort address[3] = regs_picdram_interface_inst.address[3];
      InputPort address[2] = regs_picdram_interface_inst.address[2];
      InputPort address[1] = regs_picdram_interface_inst.address[1];
      InputPort address[0] = regs_picdram_interface_inst.address[0];
      InputPort clk = piccpu_rtl2_tessent_occ_ramclk_inst.clock_out;
      Attribute tessent_design_instance = "regs/picdram";
   }
   Instance regs_picdram_interface_inst Of 
       piccpu_rtl1_tessent_mbist_c1_interface_m1 {
      InputPort BIST_CLK = piccpu_rtl2_tessent_occ_ramclk_inst.clock_out;
      InputPort BIST_COLLAR_EN = 
          regs_piccpu_rtl1_tessent_mbist_c1_controller_inst.BIST_COLLAR_EN0;
      InputPort BIST_ASYNC_RESETN = 
          piccpu_rtl1_tessent_mbist_bap_inst.BIST_ASYNC_RESET;
      InputPort BIST_SI = 
          regs_piccpu_rtl1_tessent_mbist_c1_controller_inst.MEM0_BIST_COLLAR_SI;
      InputPort BIST_SHIFT_COLLAR = 
          regs_piccpu_rtl1_tessent_mbist_c1_controller_inst.BIST_SHIFT_COLLAR;
      InputPort BIST_SETUP0 = piccpu_rtl1_tessent_mbist_bap_inst.BIST_SETUP[0];
      InputPort MEM_BYPASS_EN = 
          piccpu_rtl1_tessent_mbist_bap_inst.memory_bypass_to_en;
      InputPort MCP_BOUNDING_EN = 
          piccpu_rtl1_tessent_mbist_bap_inst.mcp_bounding_to_en;
      InputPort INCLUDE_MEM_RESULTS_REG = 
          piccpu_rtl1_tessent_mbist_bap_inst.INCLUDE_MEM_RESULTS_REG;
      Attribute tessent_design_instance = "regs/picdram_interface_inst";
   }
}
