// Merged from the 99 ICL files in the supplied list.
// Deduplicated by Module name; the first occurrence in that list is retained.
// 33 unique modules; 66 duplicate module occurrences omitted.
// Same-name definitions can differ across source variants.
// Modules with differing definition text: wrapped_pmm_ip

// Source: 1_pmm_ip_generation_with_1500_1687_and_apb_sim/PRJ/SLM/PMM/CORE/DFT/wrapped_pmm_ip.icl
Module wrapped_pmm_ip {
  ScanInPort     TDI;
  ScanOutPort    TDO { Source SIB1[0]; }
  ShiftEnPort    Shift;
  CaptureEnPort  Capture;
  UpdateEnPort   Update;
  SelectPort     Select;
  ResetPort      TRSTN { ActivePolarity 0; }
  TCKPort        TCK;
  ScanInterface client { 
    Port TDI; Port Capture; Port Shift; Port Update;
    Port Select; Port TRSTN; Port TCK; Port TDO;
  }
  ScanRegister SIB1[2:0] {
     ScanInSource SIB1mux; CaptureSource SIB1 ; ResetValue 3'b000;
     Attribute synopsys_access = "RW";
  }
  ScanRegister SIB_LOOP_SEL[3:0] {
     ScanInSource SIB_LOOP_SELmux; CaptureSource SIB_LOOP_SEL; ResetValue 4'b0000;
     Attribute synopsys_access = "RW";
  }
  ScanRegister SIB2_LOOP1[2:0] {
     ScanInSource SIB2_LOOP1mux; CaptureSource SIB2_LOOP1; ResetValue 3'b000;
     Attribute synopsys_access = "RW";
  }
  ScanRegister PM_MODE_TDR[5:0] { ScanInSource TDI; ResetValue 6'b0; Attribute synopsys_access = "RW";}
  ScanRegister FUNC_CLK_TDR[14:0] { ScanInSource TDI; ResetValue 15'b0; Attribute synopsys_access = "RW";}
  ScanRegister LOOP_BYPASS_TDR[0:0] { ScanInSource TDI; ResetValue 1'b1; Attribute synopsys_access = "RW";}
  ScanRegister SIB2_LOOP1_BYPASS_TDR[0:0] { ScanInSource TDI; ResetValue 1'b1; Attribute synopsys_access = "RW";}
  ScanRegister PATH_SEL_LOOP1[179:0] { ScanInSource TDI; ResetValue 180'b0; Attribute synopsys_access = "RW";}
  ScanRegister DELAY_SEL_LOOP1[299:0] { ScanInSource TDI; ResetValue 300'b0; Attribute synopsys_access = "RW";}
  ScanRegister COLLECT_LOOP1[119:0] { ScanInSource TDI; ResetValue 120'b0; Attribute synopsys_access = "RO";}
  ScanRegister CAPTURE_LOOP1[119:0] { ScanInSource TDI; ResetValue 120'b0; Attribute synopsys_access = "RO";}
  ScanMux SIB1mux SelectedBy SIB1 {
    3'b000 :  SIB_LOOP_SEL[0];
    3'b001 :  PM_MODE_TDR[0];
    3'b010 :  FUNC_CLK_TDR[0];
    3'b011 :  TDI;
    3'b100 :  TDI;
    3'b101 :  TDI;
    3'b110 :  TDI;
    3'b111 :  TDI;
 }
  ScanMux SIB_LOOP_SELmux SelectedBy SIB_LOOP_SEL {
    4'b0000 :  LOOP_BYPASS_TDR[0];
    4'b0001 :  SIB2_LOOP1[0];
    4'b0010 :  TDI;
    4'b0011 :  TDI;
    4'b0100 :  TDI;
    4'b0101 :  TDI;
    4'b0110 :  TDI;
    4'b0111 :  TDI;
    4'b1000 :  TDI;
    4'b1001 :  TDI;
    4'b1010 :  TDI;
    4'b1011 :  TDI;
    4'b1100 :  TDI;
    4'b1101 :  TDI;
    4'b1110 :  TDI;
    4'b1111 :  TDI;
 }
  ScanMux SIB2_LOOP1mux SelectedBy SIB2_LOOP1 {
    3'b000 :  SIB2_LOOP1_BYPASS_TDR[0] ;
    3'b001 :  PATH_SEL_LOOP1[0];
    3'b010 :  DELAY_SEL_LOOP1[0];
    3'b011 :  COLLECT_LOOP1[0];
    3'b100 :  CAPTURE_LOOP1[0];
    3'b101 :  TDI;
    3'b110 :  TDI;
    3'b111 :  TDI;
 }
}

// Source: all_out/1_pmm_ip_generation_with_1687_and_apb_sim/tessent_analysis_rtl_icl/icl/PMC.icl
// Generated; edit the PMM sources, not this file.
Module PMC {
    ShiftEnPort Shift_ijtag_se {
        Attribute tessent_design_rtl_ports = "Shift";
        Attribute tessent_design_gate_ports = "Shift";
    }
    CaptureEnPort Capture_ijtag_ce {
        Attribute tessent_design_rtl_ports = "Capture";
        Attribute tessent_design_gate_ports = "Capture";
    }
    UpdateEnPort Update_ijtag_ue {
        Attribute tessent_design_rtl_ports = "Update";
        Attribute tessent_design_gate_ports = "Update";
    }
    SelectPort Select_ijtag_sel {
        Attribute tessent_design_rtl_ports = "Select";
        Attribute tessent_design_gate_ports = "Select";
    }
    ResetPort TRSTN_ijtag_reset {
        Attribute tessent_design_rtl_ports = "TRSTN";
        Attribute tessent_design_gate_ports = "TRSTN";
        ActivePolarity 0;
    }
    TCKPort TCK_ijtag_tck {
        Attribute tessent_design_rtl_ports = "TCK";
        Attribute tessent_design_gate_ports = "TCK";
    }
    ScanInPort TDI_ijtag_si_1 {
        Attribute tessent_design_rtl_ports = "TDI";
        Attribute tessent_design_gate_ports = "TDI";
    }
    ScanOutPort TDO_ijtag_so {
        Attribute tessent_design_rtl_ports = "TDO";
        Attribute tessent_design_gate_ports = "TDO";
        Source path_monitor_controller_tai_inst.TDO_ijtag_so;
    }
    ScanInterface client {
        Port TDI_ijtag_si_1;
        Port TDO_ijtag_so;
        Port Shift_ijtag_se;
        Port Capture_ijtag_ce;
        Port Update_ijtag_ue;
        Port Select_ijtag_sel;
        Port TRSTN_ijtag_reset;
        Port TCK_ijtag_tck;
    }
    ScanInPort i_pmc2pmu_data_out_loop_ijtag_si_2[1:1] {
        Attribute tessent_design_rtl_ports = "{i_pmc2pmu_data_out_loop[1]}";
        Attribute tessent_design_gate_ports = "{i_pmc2pmu_data_out_loop[1]}";
    }
    ScanInPort i_pmc2pmu_capture_ff_tdo_ijtag_si_3[1:1] {
        Attribute tessent_design_rtl_ports = "{i_pmc2pmu_capture_ff_tdo[1]}";
        Attribute tessent_design_gate_ports = "{i_pmc2pmu_capture_ff_tdo[1]}";
    }
    ScanOutPort o_pmc2pmu_data_in_ijtag_to_si_1[1:1] {
        Attribute tessent_design_rtl_ports = "{o_pmc2pmu_data_in[1]}";
        Attribute tessent_design_gate_ports = "{o_pmc2pmu_data_in[1]}";
        Source path_monitor_controller_tai_inst.o_pmu_data_in_ijtag_to_si_1;
    }
    ScanOutPort o_pmc2pmu_capture_ff_tdi_ijtag_to_si_2[1:1] {
        Attribute tessent_design_rtl_ports = "{o_pmc2pmu_capture_ff_tdi[1]}";
        Attribute tessent_design_gate_ports = "{o_pmc2pmu_capture_ff_tdi[1]}";
        Source path_monitor_controller_tai_inst.o_pmu_capture_ff_tdi_ijtag_to_si_2;
    }
    ToSelectPort o_pmc2pmu_configure_path_sel_loop_ijtag_to_sel_1[1:1] {
        Attribute tessent_design_rtl_ports = "{o_pmc2pmu_configure_path_sel_loop[1]}";
        Attribute tessent_design_gate_ports = "{o_pmc2pmu_configure_path_sel_loop[1]}";
        Source path_monitor_controller_tai_inst.o_configure_path_sel_loop_ijtag_to_sel_1;
    }
    ToSelectPort o_pmc2pmu_configure_delay_sel_loop_ijtag_to_sel_2[1:1] {
        Attribute tessent_design_rtl_ports = "{o_pmc2pmu_configure_delay_sel_loop[1]}";
        Attribute tessent_design_gate_ports = "{o_pmc2pmu_configure_delay_sel_loop[1]}";
        Source path_monitor_controller_tai_inst.o_configure_delay_sel_loop_ijtag_to_sel_3;
    }
    ToSelectPort o_pmc2pmu_collect_status_loop_ijtag_to_sel_3[1:1] {
        Attribute tessent_design_rtl_ports = "{o_pmc2pmu_collect_status_loop[1]}";
        Attribute tessent_design_gate_ports = "{o_pmc2pmu_collect_status_loop[1]}";
        Source path_monitor_controller_tai_inst.o_pmu_collect_status_loop_ijtag_to_sel_2;
    }
    ToSelectPort o_pmc2pmu_capture_ff_shift_ijtag_to_sel_4[1:1] {
        Attribute tessent_design_rtl_ports = "{o_pmc2pmu_capture_ff_shift[1]}";
        Attribute tessent_design_gate_ports = "{o_pmc2pmu_capture_ff_shift[1]}";
        Source path_monitor_controller_tai_inst.o_pmu_capture_ff_shift_ijtag_to_sel_4;
    }
    Instance path_monitor_controller_tai_inst Of PATH_MONITOR_CONTROLLER_PATH_MONITOR_CONTROLLER_TAI {
        InputPort Shift_ijtag_se = Shift_ijtag_se;
        InputPort Capture_ijtag_ce = Capture_ijtag_ce;
        InputPort Update_ijtag_ue = Update_ijtag_ue;
        InputPort TRSTN_ijtag_reset = TRSTN_ijtag_reset;
        InputPort TCK_ijtag_tck = TCK_ijtag_tck;
        InputPort TDI_ijtag_si = TDI_ijtag_si_1;
        InputPort Select_ijtag_sel = Select_ijtag_sel;
        InputPort i_pmu_data_loop_ijtag_from_so_1 = i_pmc2pmu_data_out_loop_ijtag_si_2;
        InputPort i_pmu_capture_ff_tdo_ijtag_from_so_2 = i_pmc2pmu_capture_ff_tdo_ijtag_si_3;
    }
}

// Source: all_out/1_pmm_ip_generation_with_1687_and_apb_sim/tessent_analysis_rtl_icl/icl/FUNC_CLK_TDR.icl
// Generated; edit the PMM sources, not this file.
Module FUNC_CLK_TDR {
    ShiftEnPort Shift_ijtag_se;
    CaptureEnPort Capture_ijtag_ce;
    UpdateEnPort Update_ijtag_ue;
    SelectPort Select_ijtag_sel;
    ResetPort TRSTN_ijtag_reset {
        ActivePolarity 0;
    }
    TCKPort TCK_ijtag_tck;
    ScanInPort SI_ijtag_si;
    ScanOutPort SO_ijtag_so {
        Source Value[0];
    }
    ScanInterface client {
        Port SI_ijtag_si;
        Port SO_ijtag_so;
        Port Shift_ijtag_se;
        Port Capture_ijtag_ce;
        Port Update_ijtag_ue;
        Port Select_ijtag_sel;
        Port TRSTN_ijtag_reset;
        Port TCK_ijtag_tck;
    }
    ScanRegister Value[14:0] {
        ScanInSource SI_ijtag_si;
        CaptureSource Value;
        ResetValue 15'b0;
        Attribute synopsys_access = "RW";
    }
    DataOutPort func_clk_enable1_ijtag_do {
        Source Value[0];
    }
}

// Source: all_out/1_pmm_ip_generation_with_1687_and_apb_sim/tessent_analysis_rtl_icl/icl/PATH_MONITOR_UNIT_CFG1_MONITOR_SHADOW.icl
// Generated; edit the PMM sources, not this file.
Module PATH_MONITOR_UNIT_CFG1_MONITOR_SHADOW {
    ScanInPort capture_tdo_ijtag_si {
        Attribute tessent_design_rtl_ports = "capture_tdo";
        Attribute tessent_design_gate_ports = "capture_tdo";
    }
    SelectPort capture_shift_ijtag_sel {
        Attribute tessent_design_rtl_ports = "capture_shift";
        Attribute tessent_design_gate_ports = "capture_shift";
    }
    ScanOutPort I_capture_tdo_ijtag_so {
        Attribute tessent_design_rtl_ports = "I_capture_tdo";
        Attribute tessent_design_gate_ports = "I_capture_tdo";
        Source shadow_out;
    }
    ScanInterface client_shadow_out {
        Port capture_tdo_ijtag_si;
        Port I_capture_tdo_ijtag_so;
        Port capture_shift_ijtag_sel;
    }
    ScanRegister shadow_out {
        ScanInSource capture_tdo_ijtag_si;
        ResetValue 1'b0;
        Attribute synopsys_access = "RO";
    }
    DataOutPort o_shadow_out_ijtag_do {
        Attribute tessent_design_rtl_ports = "o_shadow_out";
        Attribute tessent_design_gate_ports = "o_shadow_out";
        Source shadow_out;
    }
}

// Source: all_out/1_pmm_ip_generation_with_1687_and_apb_sim/tessent_analysis_rtl_icl/icl/CAPTURE_LOOP.icl
// Generated; edit the PMM sources, not this file.
Module CAPTURE_LOOP {
    ShiftEnPort Shift_ijtag_se;
    CaptureEnPort Capture_ijtag_ce;
    UpdateEnPort Update_ijtag_ue;
    SelectPort Select_ijtag_sel;
    ResetPort TRSTN_ijtag_reset {
        ActivePolarity 0;
    }
    TCKPort TCK_ijtag_tck;
    ScanInPort SI_ijtag_si;
    ScanOutPort SO_ijtag_so {
        Source Shadow;
    }
    ScanInterface client {
        Port SI_ijtag_si;
        Port SO_ijtag_so;
        Port Shift_ijtag_se;
        Port Capture_ijtag_ce;
        Port Update_ijtag_ue;
        Port Select_ijtag_sel;
        Port TRSTN_ijtag_reset;
        Port TCK_ijtag_tck;
    }
    ScanRegister CaptureBit {
        ScanInSource SI_ijtag_si;
        ResetValue 1'b0;
        Attribute synopsys_access = "RO";
    }
    ScanRegister Shadow {
        ScanInSource CaptureBit;
        ResetValue 1'b0;
        Attribute synopsys_access = "RO";
    }
    DataOutPort capturebit_ijtag_do_1 {
        Source CaptureBit;
    }
    DataOutPort shadow_ijtag_do_2 {
        Source Shadow;
    }
}

// Source: all_out/1_pmm_ip_generation_with_1687_and_apb_sim/tessent_analysis_rtl_icl/icl/PATH_SEL_LOOP.icl
// Generated; edit the PMM sources, not this file.
Module PATH_SEL_LOOP {
    ShiftEnPort Shift_ijtag_se;
    CaptureEnPort Capture_ijtag_ce;
    UpdateEnPort Update_ijtag_ue;
    SelectPort Select_ijtag_sel;
    ResetPort TRSTN_ijtag_reset {
        ActivePolarity 0;
    }
    TCKPort TCK_ijtag_tck;
    ScanInPort SI_ijtag_si;
    ScanOutPort SO_ijtag_so {
        Source Value[0];
    }
    ScanInterface client {
        Port SI_ijtag_si;
        Port SO_ijtag_so;
        Port Shift_ijtag_se;
        Port Capture_ijtag_ce;
        Port Update_ijtag_ue;
        Port Select_ijtag_sel;
        Port TRSTN_ijtag_reset;
        Port TCK_ijtag_tck;
    }
    ScanRegister Value[2:0] {
        ScanInSource SI_ijtag_si;
        ResetValue 3'b0;
        Attribute synopsys_access = "RW";
    }
    DataOutPort path_sel_ijtag_do[2:0] {
        Source Value;
    }
}

// Source: all_out/1_pmm_ip_generation_with_1687_and_apb_sim/tessent_analysis_rtl_icl/icl/SIB2_LOOP1_BYPASS_TDR.icl
// Generated; edit the PMM sources, not this file.
Module SIB2_LOOP1_BYPASS_TDR {
    ShiftEnPort Shift_ijtag_se;
    CaptureEnPort Capture_ijtag_ce;
    UpdateEnPort Update_ijtag_ue;
    SelectPort Select_ijtag_sel;
    ResetPort TRSTN_ijtag_reset {
        ActivePolarity 0;
    }
    TCKPort TCK_ijtag_tck;
    ScanInPort SI_ijtag_si;
    ScanOutPort SO_ijtag_so {
        Source Value;
    }
    ScanInterface client {
        Port SI_ijtag_si;
        Port SO_ijtag_so;
        Port Shift_ijtag_se;
        Port Capture_ijtag_ce;
        Port Update_ijtag_ue;
        Port Select_ijtag_sel;
        Port TRSTN_ijtag_reset;
        Port TCK_ijtag_tck;
    }
    ScanRegister Value {
        ScanInSource SI_ijtag_si;
        CaptureSource Value;
        ResetValue 1'b1;
        Attribute synopsys_access = "RW";
    }
}

// Source: all_out/1_pmm_ip_generation_with_1687_and_apb_sim/tessent_analysis_rtl_icl/icl/PATH_MONITOR_UNIT_CFG1_PATH_SELECT_REG_MODULE.icl
// Generated; edit the PMM sources, not this file.
Module PATH_MONITOR_UNIT_CFG1_PATH_SELECT_REG_MODULE {
    ScanInPort i_pmu_data_in_ijtag_si {
        Attribute tessent_design_rtl_ports = "i_pmu_data_in";
        Attribute tessent_design_gate_ports = "i_pmu_data_in";
    }
    SelectPort i_configure_path_sel_ijtag_sel {
        Attribute tessent_design_rtl_ports = "i_configure_path_sel";
        Attribute tessent_design_gate_ports = "i_configure_path_sel";
    }
    ScanOutPort o_path_sel_so_ijtag_so {
        Attribute tessent_design_rtl_ports = "o_path_sel_so";
        Attribute tessent_design_gate_ports = "o_path_sel_so";
        Source path_sel_reg[0];
    }
    ScanInterface client_path_sel_reg {
        Port i_pmu_data_in_ijtag_si;
        Port o_path_sel_so_ijtag_so;
        Port i_configure_path_sel_ijtag_sel;
    }
    ScanRegister path_sel_reg[2:0] {
        ScanInSource i_pmu_data_in_ijtag_si;
        ResetValue 3'b0;
        Attribute synopsys_access = "RW";
    }
    DataOutPort o_path_select_ijtag_do[2:0] {
        Attribute tessent_design_rtl_ports = "{o_path_select[2]} {o_path_select[1]} {o_path_select[0]}";
        Attribute tessent_design_gate_ports = "{o_path_select[2]} {o_path_select[1]} {o_path_select[0]}";
        Source path_sel_reg;
    }
}

// Source: all_out/1_pmm_ip_generation_with_1687_and_apb_sim/tessent_analysis_rtl_icl/icl/SIB2_LOOP.icl
// Generated; edit the PMM sources, not this file.
Module SIB2_LOOP {
    ShiftEnPort Shift_ijtag_se;
    CaptureEnPort Capture_ijtag_ce;
    UpdateEnPort Update_ijtag_ue;
    SelectPort Select_ijtag_sel;
    ResetPort TRSTN_ijtag_reset {
        ActivePolarity 0;
    }
    TCKPort TCK_ijtag_tck;
    ScanInPort SI_ijtag_si;
    ScanOutPort SO_ijtag_so {
        Source Value[0];
    }
    ScanInterface client {
        Port SI_ijtag_si;
        Port SO_ijtag_so;
        Port Shift_ijtag_se;
        Port Capture_ijtag_ce;
        Port Update_ijtag_ue;
        Port Select_ijtag_sel;
        Port TRSTN_ijtag_reset;
        Port TCK_ijtag_tck;
    }
    ScanInPort B0_ijtag_from_so_1;
    LogicSignal Enable0 {
        Select_ijtag_sel && (Value == 3'd0);
    }
    ToSelectPort Select0_ijtag_to_sel_1 {
        Source Enable0;
    }
    ScanInPort B1_ijtag_from_so_2;
    LogicSignal Enable1 {
        Select_ijtag_sel && (Value == 3'd1);
    }
    ToSelectPort Select1_ijtag_to_sel_2 {
        Source Enable1;
    }
    ScanInPort B2_ijtag_from_so_3;
    LogicSignal Enable2 {
        Select_ijtag_sel && (Value == 3'd2);
    }
    ToSelectPort Select2_ijtag_to_sel_3 {
        Source Enable2;
    }
    ScanInPort B3_ijtag_from_so_4;
    LogicSignal Enable3 {
        Select_ijtag_sel && (Value == 3'd3);
    }
    ToSelectPort Select3_ijtag_to_sel_4 {
        Source Enable3;
    }
    ScanInPort B4_ijtag_from_so_5;
    LogicSignal Enable4 {
        Select_ijtag_sel && (Value == 3'd4);
    }
    ToSelectPort Select4_ijtag_to_sel_5 {
        Source Enable4;
    }
    ScanRegister Value[2:0] {
        ScanInSource mux;
        CaptureSource Value;
        ResetValue 3'b0;
    }
    DataOutPort Selection_ijtag_do[2:0] {
        Source Value;
    }
    ScanMux mux SelectedBy Value {
        3'd0 : B0_ijtag_from_so_1;
        3'd1 : B1_ijtag_from_so_2;
        3'd2 : B2_ijtag_from_so_3;
        3'd3 : B3_ijtag_from_so_4;
        3'd4 : B4_ijtag_from_so_5;
        3'd5 : SI_ijtag_si;
        3'd6 : SI_ijtag_si;
        3'd7 : SI_ijtag_si;
    }
}

// Source: all_out/1_pmm_ip_generation_with_1687_and_apb_sim/tessent_analysis_rtl_icl/icl/PATH_MONITOR_CONTROLLER_IEEE_1687_pmu_tdr.icl
// Generated; edit the PMM sources, not this file.
Module PATH_MONITOR_CONTROLLER_IEEE_1687_pmu_tdr {
    // Stable shift topology; capture/update holding and phase gating are abstracted.
    TCKPort i_tck_ijtag_tck {
        Attribute tessent_design_rtl_ports = "i_tck";
        Attribute tessent_design_gate_ports = "i_tck";
    }
    ResetPort i_resetn_ijtag_reset {
        Attribute tessent_design_rtl_ports = "i_resetn";
        Attribute tessent_design_gate_ports = "i_resetn";
        ActivePolarity 0;
    }
    ShiftEnPort i_pmu_tdr_sibShiftEn_ijtag_se {
        Attribute tessent_design_rtl_ports = "i_pmu_tdr_sibShiftEn";
        Attribute tessent_design_gate_ports = "i_pmu_tdr_sibShiftEn";
    }
    DataInPort i_pmu_tdr_sib_ijtag_di {
        Attribute tessent_design_rtl_ports = "i_pmu_tdr_sib";
        Attribute tessent_design_gate_ports = "i_pmu_tdr_sib";
    }
    CaptureEnPort i_pmu_tdr_sibCaptureEn_ijtag_ce {
        Attribute tessent_design_rtl_ports = "i_pmu_tdr_sibCaptureEn";
        Attribute tessent_design_gate_ports = "i_pmu_tdr_sibCaptureEn";
    }
    UpdateEnPort i_pmu_tdr_sibUpdateEn_ijtag_ue {
        Attribute tessent_design_rtl_ports = "i_pmu_tdr_sibUpdateEn";
        Attribute tessent_design_gate_ports = "i_pmu_tdr_sibUpdateEn";
    }
    ScanInPort i_pmu_tdr_scanin_ijtag_si {
        Attribute tessent_design_rtl_ports = "i_pmu_tdr_scanin";
        Attribute tessent_design_gate_ports = "i_pmu_tdr_scanin";
    }
    ScanOutPort o_pmu_tdr_scanout_ijtag_so {
        Attribute tessent_design_rtl_ports = "o_pmu_tdr_scanout";
        Attribute tessent_design_gate_ports = "o_pmu_tdr_scanout";
        Source i_pmu_tdr_data_out_ijtag_from_so;
    }
    ScanOutPort o_pmu_tdr_data_in_ijtag_to_si {
        Attribute tessent_design_rtl_ports = "o_pmu_tdr_data_in";
        Attribute tessent_design_gate_ports = "o_pmu_tdr_data_in";
        Source i_pmu_tdr_scanin_ijtag_si;
    }
    ToSelectPort o_pmu_shift_ijtag_to_sel {
        Attribute tessent_design_rtl_ports = "o_pmu_shift";
        Attribute tessent_design_gate_ports = "o_pmu_shift";
        Source i_pmu_tdr_sib_ijtag_di;
    }
    ScanInPort i_pmu_tdr_data_out_ijtag_from_so {
        Attribute tessent_design_rtl_ports = "i_pmu_tdr_data_out";
        Attribute tessent_design_gate_ports = "i_pmu_tdr_data_out";
    }
    ScanInterface client {
        Port i_pmu_tdr_scanin_ijtag_si;
        Port o_pmu_tdr_scanout_ijtag_so;
        Port i_pmu_tdr_sibShiftEn_ijtag_se;
        Port i_pmu_tdr_sibCaptureEn_ijtag_ce;
        Port i_pmu_tdr_sibUpdateEn_ijtag_ue;
        Port i_tck_ijtag_tck;
        Port i_resetn_ijtag_reset;
    }
    ScanInterface host {
        Port i_pmu_tdr_data_out_ijtag_from_so;
        Port o_pmu_shift_ijtag_to_sel;
    }
}

// Source: all_out/1_pmm_ip_generation_with_1687_and_apb_sim/tessent_analysis_rtl_icl/icl/PATH_MONITOR_CONTROLLER_TEST_MODE_DECODER.icl
// Generated; edit the PMM sources, not this file.
Module PATH_MONITOR_CONTROLLER_TEST_MODE_DECODER {
    DataInPort i_test_mode_ijtag_di[5:0] {
        Attribute tessent_design_rtl_ports = "{i_test_mode[5]} {i_test_mode[4]} {i_test_mode[3]} {i_test_mode[2]} {i_test_mode[1]} {i_test_mode[0]}";
        Attribute tessent_design_gate_ports = "{i_test_mode[5]} {i_test_mode[4]} {i_test_mode[3]} {i_test_mode[2]} {i_test_mode[1]} {i_test_mode[0]}";
    }
    DataOutPort o_pmu_testmode_ijtag_do_1[1:0] {
        Attribute tessent_design_rtl_ports = "{o_pmu_testmode[1]} {o_pmu_testmode[0]}";
        Attribute tessent_design_gate_ports = "{o_pmu_testmode[1]} {o_pmu_testmode[0]}";
        Source i_test_mode_ijtag_di[1:0];
    }
    LogicSignal o_pmu_clock_mux_en_value {
        (i_test_mode_ijtag_di[1] && i_test_mode_ijtag_di[0]) || (!i_test_mode_ijtag_di[1] && !i_test_mode_ijtag_di[0]);
    }
    DataOutPort o_pmu_clock_mux_en_ijtag_do_2 {
        Attribute tessent_design_rtl_ports = "o_pmu_clock_mux_en";
        Attribute tessent_design_gate_ports = "o_pmu_clock_mux_en";
        Source o_pmu_clock_mux_en_value;
    }
    DataOutPort o_clkdiv_cg_test_ijtag_do_3 {
        Attribute tessent_design_rtl_ports = "o_clkdiv_cg_test";
        Attribute tessent_design_gate_ports = "o_clkdiv_cg_test";
        Source i_test_mode_ijtag_di[5];
    }
    DataOutPort o_pmc_test_en_ijtag_do_4 {
        Attribute tessent_design_rtl_ports = "o_pmc_test_en";
        Attribute tessent_design_gate_ports = "o_pmc_test_en";
        Source i_test_mode_ijtag_di[4];
    }
    DataOutPort o_pmc_mode_tdr_override_ijtag_do_5 {
        Attribute tessent_design_rtl_ports = "o_pmc_mode_tdr_override";
        Attribute tessent_design_gate_ports = "o_pmc_mode_tdr_override";
        Source i_test_mode_ijtag_di[2];
    }
    DataOutPort debug_en_tdr_ijtag_do_6 {
        Attribute tessent_design_rtl_ports = "debug_en_tdr";
        Attribute tessent_design_gate_ports = "debug_en_tdr";
        Source i_test_mode_ijtag_di[3];
    }
}

// Source: all_out/1_pmm_ip_generation_with_1687_and_apb_sim/tessent_analysis_rtl_icl/icl/PATH_MONITOR_CONTROLLER_IEEE_1687_tdr_FIFTEEN_WIDTH.icl
// Generated; edit the PMM sources, not this file.
Module PATH_MONITOR_CONTROLLER_IEEE_1687_tdr_FIFTEEN_WIDTH {
    TCKPort i_tck_ijtag_tck {
        Attribute tessent_design_rtl_ports = "i_tck";
        Attribute tessent_design_gate_ports = "i_tck";
    }
    ResetPort i_resetn_ijtag_reset {
        Attribute tessent_design_rtl_ports = "i_resetn";
        Attribute tessent_design_gate_ports = "i_resetn";
        ActivePolarity 0;
    }
    ShiftEnPort i_tdr_sibShiftEn_ijtag_se {
        Attribute tessent_design_rtl_ports = "i_tdr_sibShiftEn";
        Attribute tessent_design_gate_ports = "i_tdr_sibShiftEn";
    }
    CaptureEnPort i_tdr_sibCaptureEn_ijtag_ce {
        Attribute tessent_design_rtl_ports = "i_tdr_sibCaptureEn";
        Attribute tessent_design_gate_ports = "i_tdr_sibCaptureEn";
    }
    UpdateEnPort i_tdr_sibUpdateEn_ijtag_ue {
        Attribute tessent_design_rtl_ports = "i_tdr_sibUpdateEn";
        Attribute tessent_design_gate_ports = "i_tdr_sibUpdateEn";
    }
    ScanInPort i_tdr_scanin_ijtag_si {
        Attribute tessent_design_rtl_ports = "i_tdr_scanin";
        Attribute tessent_design_gate_ports = "i_tdr_scanin";
    }
    ScanOutPort o_tdr_scanout_ijtag_so {
        Attribute tessent_design_rtl_ports = "o_tdr_scanout";
        Attribute tessent_design_gate_ports = "o_tdr_scanout";
        Source tdr_shift_cap[0];
    }
    ScanRegister tdr_shift_cap[14:0] {
        ScanInSource i_tdr_scanin_ijtag_si;
        CaptureSource tdr_shift_cap;
        ResetValue 15'b0;
    }
    DataOutPort o_tdr_update_ijtag_do[14:0] {
        Attribute tessent_design_rtl_ports = "{o_tdr_update[14]} {o_tdr_update[13]} {o_tdr_update[12]} {o_tdr_update[11]} {o_tdr_update[10]} {o_tdr_update[9]} {o_tdr_update[8]} {o_tdr_update[7]} {o_tdr_update[6]} {o_tdr_update[5]} {o_tdr_update[4]} {o_tdr_update[3]} {o_tdr_update[2]} {o_tdr_update[1]} {o_tdr_update[0]}";
        Attribute tessent_design_gate_ports = "{o_tdr_update[14]} {o_tdr_update[13]} {o_tdr_update[12]} {o_tdr_update[11]} {o_tdr_update[10]} {o_tdr_update[9]} {o_tdr_update[8]} {o_tdr_update[7]} {o_tdr_update[6]} {o_tdr_update[5]} {o_tdr_update[4]} {o_tdr_update[3]} {o_tdr_update[2]} {o_tdr_update[1]} {o_tdr_update[0]}";
        Source tdr_shift_cap;
    }
}

// Source: all_out/1_pmm_ip_generation_with_1687_and_apb_sim/tessent_analysis_rtl_icl/icl/PATH_MONITOR_CONTROLLER_PATH_MONITOR_CONTROLLER_TAI.icl
// Generated; edit the PMM sources, not this file.
Module PATH_MONITOR_CONTROLLER_PATH_MONITOR_CONTROLLER_TAI {
    TCKPort TCK_ijtag_tck {
        Attribute tessent_design_rtl_ports = "TCK";
        Attribute tessent_design_gate_ports = "TCK";
    }
    ResetPort TRSTN_ijtag_reset {
        Attribute tessent_design_rtl_ports = "TRSTN";
        Attribute tessent_design_gate_ports = "TRSTN";
        ActivePolarity 0;
    }
    ShiftEnPort Shift_ijtag_se {
        Attribute tessent_design_rtl_ports = "Shift";
        Attribute tessent_design_gate_ports = "Shift";
    }
    UpdateEnPort Update_ijtag_ue {
        Attribute tessent_design_rtl_ports = "Update";
        Attribute tessent_design_gate_ports = "Update";
    }
    CaptureEnPort Capture_ijtag_ce {
        Attribute tessent_design_rtl_ports = "Capture";
        Attribute tessent_design_gate_ports = "Capture";
    }
    SelectPort Select_ijtag_sel {
        Attribute tessent_design_rtl_ports = "Select";
        Attribute tessent_design_gate_ports = "Select";
    }
    ScanInPort TDI_ijtag_si {
        Attribute tessent_design_rtl_ports = "TDI";
        Attribute tessent_design_gate_ports = "TDI";
    }
    ScanOutPort TDO_ijtag_so {
        Attribute tessent_design_rtl_ports = "TDO";
        Attribute tessent_design_gate_ports = "TDO";
        Source IEEE_1687_module_inst.o_scanout_ijtag_so;
    }
    ScanInPort i_pmu_data_loop_ijtag_from_so_1[1:1] {
        Attribute tessent_design_rtl_ports = "{i_pmu_data_loop[1]}";
        Attribute tessent_design_gate_ports = "{i_pmu_data_loop[1]}";
    }
    ScanOutPort o_pmu_data_in_ijtag_to_si_1[1:1] {
        Attribute tessent_design_rtl_ports = "{o_pmu_data_in[1]}";
        Attribute tessent_design_gate_ports = "{o_pmu_data_in[1]}";
        Source tai_output_interface_inst.o_pmu_data_in_ijtag_to_si_1;
    }
    DataOutPort o_wrck_clk_mux_en_loop_ijtag_do_1[1:1] {
        Attribute tessent_design_rtl_ports = "{o_wrck_clk_mux_en_loop[1]}";
        Attribute tessent_design_gate_ports = "{o_wrck_clk_mux_en_loop[1]}";
        Source clk_cntrl_inst.o_wrck_clk_mux_en_ijtag_do_1;
    }
    ToSelectPort o_configure_path_sel_loop_ijtag_to_sel_1[1:1] {
        Attribute tessent_design_rtl_ports = "{o_configure_path_sel_loop[1]}";
        Attribute tessent_design_gate_ports = "{o_configure_path_sel_loop[1]}";
        Source tai_output_interface_inst.o_configure_path_sel_loop_ijtag_to_sel_1;
    }
    ToSelectPort o_pmu_collect_status_loop_ijtag_to_sel_2[1:1] {
        Attribute tessent_design_rtl_ports = "{o_pmu_collect_status_loop[1]}";
        Attribute tessent_design_gate_ports = "{o_pmu_collect_status_loop[1]}";
        Source tai_output_interface_inst.o_pmu_collect_status_loop_ijtag_to_sel_2;
    }
    ToSelectPort o_configure_delay_sel_loop_ijtag_to_sel_3[1:1] {
        Attribute tessent_design_rtl_ports = "{o_configure_delay_sel_loop[1]}";
        Attribute tessent_design_gate_ports = "{o_configure_delay_sel_loop[1]}";
        Source tai_output_interface_inst.o_configure_delay_sel_loop_ijtag_to_sel_3;
    }
    DataOutPort o_pmu_func_clk_gate_en_loop_ijtag_do_2[1:1] {
        Attribute tessent_design_rtl_ports = "{o_pmu_func_clk_gate_en_loop[1]}";
        Attribute tessent_design_gate_ports = "{o_pmu_func_clk_gate_en_loop[1]}";
        Source tai_output_interface_inst.o_pmu_func_clk_gate_en_loop_ijtag_do;
    }
    DataOutPort o_pmu_testmode_ijtag_do_3[1:0] {
        Attribute tessent_design_rtl_ports = "{o_pmu_testmode[1]} {o_pmu_testmode[0]}";
        Attribute tessent_design_gate_ports = "{o_pmu_testmode[1]} {o_pmu_testmode[0]}";
        Source test_mode_decoder_inst.o_pmu_testmode_ijtag_do_1;
    }
    DataOutPort o_pmc_test_en_ijtag_do_4 {
        Attribute tessent_design_rtl_ports = "o_pmc_test_en";
        Attribute tessent_design_gate_ports = "o_pmc_test_en";
        Source test_mode_decoder_inst.o_pmc_test_en_ijtag_do_4;
    }
    DataOutPort o_clkdiv_cg_test_ijtag_do_5 {
        Attribute tessent_design_rtl_ports = "o_clkdiv_cg_test";
        Attribute tessent_design_gate_ports = "o_clkdiv_cg_test";
        Source test_mode_decoder_inst.o_clkdiv_cg_test_ijtag_do_3;
    }
    DataOutPort o_pmc_mode_tdr_override_ijtag_do_6 {
        Attribute tessent_design_rtl_ports = "o_pmc_mode_tdr_override";
        Attribute tessent_design_gate_ports = "o_pmc_mode_tdr_override";
        Source test_mode_decoder_inst.o_pmc_mode_tdr_override_ijtag_do_5;
    }
    ToSelectPort o_pmu_capture_ff_shift_ijtag_to_sel_4[1:1] {
        Attribute tessent_design_rtl_ports = "{o_pmu_capture_ff_shift[1]}";
        Attribute tessent_design_gate_ports = "{o_pmu_capture_ff_shift[1]}";
        Source tai_output_interface_inst.o_pmu_capture_ff_shift_ijtag_to_sel_4;
    }
    ScanOutPort o_pmu_capture_ff_tdi_ijtag_to_si_2[1:1] {
        Attribute tessent_design_rtl_ports = "{o_pmu_capture_ff_tdi[1]}";
        Attribute tessent_design_gate_ports = "{o_pmu_capture_ff_tdi[1]}";
        Source tai_output_interface_inst.o_pmu_capture_ff_tdi_ijtag_to_si_2;
    }
    ScanInPort i_pmu_capture_ff_tdo_ijtag_from_so_2[1:1] {
        Attribute tessent_design_rtl_ports = "{i_pmu_capture_ff_tdo[1]}";
        Attribute tessent_design_gate_ports = "{i_pmu_capture_ff_tdo[1]}";
    }
    Instance tai_output_interface_inst Of PATH_MONITOR_CONTROLLER_TAI_OUTPUT_INTERFACE {
        InputPort i_clk_ijtag_tck = TCK_ijtag_tck;
        InputPort i_rstn_ijtag_reset = TRSTN_ijtag_reset;
        InputPort i_configure_path_sel_ijtag_di_4 = IEEE_1687_module_inst.o_configure_path_sel_pmu_ijtag_to_sel_3;
        InputPort i_configure_delay_sel_ijtag_di_2 = IEEE_1687_module_inst.o_configure_delay_sel_pmu_ijtag_to_sel_2;
        InputPort i_pmu_collect_status_ijtag_di_3 = IEEE_1687_module_inst.o_pmu_collect_status_ijtag_to_sel_1;
        InputPort i_pmu_capture_ff_shift_ijtag_di_5 = IEEE_1687_module_inst.o_pmu_capture_ff_shift_ijtag_to_sel_4;
        InputPort i_pmu_func_clk_gate_en_ijtag_di_1 = IEEE_1687_module_inst.o_pmu_func_clk_gate_en_ijtag_do_2;
        InputPort i_pmu_capture_ff_tdi_ijtag_si_1 = IEEE_1687_module_inst.o_pmu_capture_ff_tdi_ijtag_to_si_4;
        InputPort pmu_status_in_ijtag_si_4 = IEEE_1687_module_inst.o_pmu_status_ijtag_to_si_3;
        InputPort i_path_select_ijtag_si_3 = IEEE_1687_module_inst.o_path_select_ijtag_to_si_2;
        InputPort i_delay_select_ijtag_si_2 = IEEE_1687_module_inst.o_delay_select_ijtag_to_si_1;
    }
    Instance IEEE_1687_module_inst Of PATH_MONITOR_CONTROLLER_IEEE_1687_module {
        InputPort i_scanin_ijtag_si = TDI_ijtag_si;
        InputPort i_resetn_ijtag_reset = TRSTN_ijtag_reset;
        InputPort i_tck_ijtag_tck_1 = TCK_ijtag_tck;
        InputPort i_captureEn_ijtag_ce = Capture_ijtag_ce;
        InputPort i_shiftEn_ijtag_se = Shift_ijtag_se;
        InputPort i_updateEn_ijtag_ue = Update_ijtag_ue;
        InputPort i_select_ijtag_sel = Select_ijtag_sel;
        InputPort i_wrck_gated_mutex_ijtag_tck_2 = TCK_ijtag_tck;
        InputPort i_pmu_data_out_ijtag_from_so_1 = i_pmu_data_loop_ijtag_from_so_1;
        InputPort i_pmu_capture_ff_data_out_ijtag_from_so_2 = i_pmu_capture_ff_tdo_ijtag_from_so_2;
    }
    Instance clk_cntrl_inst Of PATH_MONITOR_CONTROLLER_clk_cntrl {
        InputPort i_pmu_clock_mux_en_ijtag_di_1 = test_mode_decoder_inst.o_pmu_clock_mux_en_ijtag_do_2;
        InputPort i_pmu_mode_ijtag_di_2 = test_mode_decoder_inst.o_pmu_testmode_ijtag_do_1;
        InputPort i_resetn_ijtag_reset = TRSTN_ijtag_reset;
        InputPort i_testmode_sibSelect_ijtag_di_3 = IEEE_1687_module_inst.o_testmode_sibSelect_ijtag_do_3;
        InputPort i_sib_loop_bypass_tdr_sibSelect_ijtag_di_4 = IEEE_1687_module_inst.o_sib_loop_bypass_tdr_sibSelect_ijtag_do_4;
        InputPort i_pmu_path_select_tdr_sibSelect_ijtag_di_5 = IEEE_1687_module_inst.o_pmu_path_select_tdr_sibSelect_ijtag_do_5;
        InputPort i_pmu_path_select_tdr_sib_ijtag_di_6 = IEEE_1687_module_inst.o_pmu_path_select_tdr_sib_ijtag_do_6;
        InputPort i_pmu_delay_select_tdr_sibSelect_ijtag_di_7 = IEEE_1687_module_inst.o_pmu_delay_select_tdr_sibSelect_ijtag_do_7;
        InputPort i_pmu_delay_select_tdr_sib_ijtag_di_8 = IEEE_1687_module_inst.o_pmu_delay_select_tdr_sib_ijtag_do_8;
        InputPort i_pmu_collect_status_tdr_sibSelect_ijtag_di_9 = IEEE_1687_module_inst.o_pmu_collect_status_tdr_sibSelect_ijtag_do_9;
        InputPort i_pmu_collect_status_tdr_sib_ijtag_di_10 = IEEE_1687_module_inst.o_pmu_collect_status_tdr_sib_ijtag_do_11;
        InputPort i_pmu_func_clk_gate_en_tdr_sibSelect_ijtag_di_11 = IEEE_1687_module_inst.o_pmu_func_clk_gate_en_tdr_sibSelect_ijtag_do_12;
        InputPort i_sib2_bypass_tdr_sibSelect_ijtag_di_12 = IEEE_1687_module_inst.o_sib2_bypass_tdr_sibSelect_ijtag_do_10;
        InputPort i_pmu_capture_ff_tdr_sibSelect_ijtag_di_13 = IEEE_1687_module_inst.o_pmu_capture_ff_tdr_sibSelect_ijtag_do_13;
        InputPort i_pmu_capture_ff_tdr_sib_ijtag_di_14 = IEEE_1687_module_inst.o_pmu_capture_ff_tdr_sib_ijtag_do_14;
        InputPort i_wrck_gated_mutex_ijtag_tck = TCK_ijtag_tck;
    }
    Instance test_mode_decoder_inst Of PATH_MONITOR_CONTROLLER_TEST_MODE_DECODER {
        InputPort i_test_mode_ijtag_di = IEEE_1687_module_inst.o_test_mode_1687_ijtag_do_1;
    }
    ScanInterface client {
        Port TDI_ijtag_si;
        Port TDO_ijtag_so;
        Port Select_ijtag_sel;
        Port Shift_ijtag_se;
        Port Capture_ijtag_ce;
        Port Update_ijtag_ue;
        Port TRSTN_ijtag_reset;
        Port TCK_ijtag_tck;
    }
    ScanInterface host_o_configure_path_sel_loop1 {
        Port i_pmu_data_loop_ijtag_from_so_1[1];
        Port o_configure_path_sel_loop_ijtag_to_sel_1[1];
    }
    ScanInterface host_o_configure_delay_sel_loop1 {
        Port i_pmu_data_loop_ijtag_from_so_1[1];
        Port o_configure_delay_sel_loop_ijtag_to_sel_3[1];
    }
    ScanInterface host_o_pmu_collect_status_loop1 {
        Port i_pmu_data_loop_ijtag_from_so_1[1];
        Port o_pmu_collect_status_loop_ijtag_to_sel_2[1];
    }
    ScanInterface host_o_pmu_capture_ff_shift1 {
        Port i_pmu_capture_ff_tdo_ijtag_from_so_2[1];
        Port o_pmu_capture_ff_shift_ijtag_to_sel_4[1];
    }
}

// Source: all_out/1_pmm_ip_generation_with_1687_and_apb_sim/tessent_analysis_rtl_icl/icl/PATH_MONITOR_CONTROLLER_IEEE_1687_tdr_SIX_WIDTH.icl
// Generated; edit the PMM sources, not this file.
Module PATH_MONITOR_CONTROLLER_IEEE_1687_tdr_SIX_WIDTH {
    TCKPort i_tck_ijtag_tck {
        Attribute tessent_design_rtl_ports = "i_tck";
        Attribute tessent_design_gate_ports = "i_tck";
    }
    ResetPort i_resetn_ijtag_reset {
        Attribute tessent_design_rtl_ports = "i_resetn";
        Attribute tessent_design_gate_ports = "i_resetn";
        ActivePolarity 0;
    }
    ShiftEnPort i_tdr_sibShiftEn_ijtag_se {
        Attribute tessent_design_rtl_ports = "i_tdr_sibShiftEn";
        Attribute tessent_design_gate_ports = "i_tdr_sibShiftEn";
    }
    CaptureEnPort i_tdr_sibCaptureEn_ijtag_ce {
        Attribute tessent_design_rtl_ports = "i_tdr_sibCaptureEn";
        Attribute tessent_design_gate_ports = "i_tdr_sibCaptureEn";
    }
    UpdateEnPort i_tdr_sibUpdateEn_ijtag_ue {
        Attribute tessent_design_rtl_ports = "i_tdr_sibUpdateEn";
        Attribute tessent_design_gate_ports = "i_tdr_sibUpdateEn";
    }
    ScanInPort i_tdr_scanin_ijtag_si {
        Attribute tessent_design_rtl_ports = "i_tdr_scanin";
        Attribute tessent_design_gate_ports = "i_tdr_scanin";
    }
    ScanOutPort o_tdr_scanout_ijtag_so {
        Attribute tessent_design_rtl_ports = "o_tdr_scanout";
        Attribute tessent_design_gate_ports = "o_tdr_scanout";
        Source tdr_shift_cap[0];
    }
    ScanRegister tdr_shift_cap[5:0] {
        ScanInSource i_tdr_scanin_ijtag_si;
        CaptureSource tdr_shift_cap;
        ResetValue 6'b0;
    }
    DataOutPort o_tdr_update_ijtag_do[5:0] {
        Attribute tessent_design_rtl_ports = "{o_tdr_update[5]} {o_tdr_update[4]} {o_tdr_update[3]} {o_tdr_update[2]} {o_tdr_update[1]} {o_tdr_update[0]}";
        Attribute tessent_design_gate_ports = "{o_tdr_update[5]} {o_tdr_update[4]} {o_tdr_update[3]} {o_tdr_update[2]} {o_tdr_update[1]} {o_tdr_update[0]}";
        Source tdr_shift_cap;
    }
}

// Source: all_out/1_pmm_ip_generation_with_1687_and_apb_sim/tessent_analysis_rtl_icl/icl/PATH_MONITOR_CONTROLLER_IEEE_1687_one_bit_tdr.icl
// Generated; edit the PMM sources, not this file.
Module PATH_MONITOR_CONTROLLER_IEEE_1687_one_bit_tdr {
    TCKPort i_tck_ijtag_tck {
        Attribute tessent_design_rtl_ports = "i_tck";
        Attribute tessent_design_gate_ports = "i_tck";
    }
    ResetPort i_resetn_ijtag_reset {
        Attribute tessent_design_rtl_ports = "i_resetn";
        Attribute tessent_design_gate_ports = "i_resetn";
        ActivePolarity 0;
    }
    ShiftEnPort i_tdr_sibShiftEn_ijtag_se {
        Attribute tessent_design_rtl_ports = "i_tdr_sibShiftEn";
        Attribute tessent_design_gate_ports = "i_tdr_sibShiftEn";
    }
    CaptureEnPort i_tdr_sibCaptureEn_ijtag_ce {
        Attribute tessent_design_rtl_ports = "i_tdr_sibCaptureEn";
        Attribute tessent_design_gate_ports = "i_tdr_sibCaptureEn";
    }
    UpdateEnPort i_tdr_sibUpdateEn_ijtag_ue {
        Attribute tessent_design_rtl_ports = "i_tdr_sibUpdateEn";
        Attribute tessent_design_gate_ports = "i_tdr_sibUpdateEn";
    }
    ScanInPort i_tdr_scanin_ijtag_si {
        Attribute tessent_design_rtl_ports = "i_tdr_scanin";
        Attribute tessent_design_gate_ports = "i_tdr_scanin";
    }
    ScanOutPort o_tdr_scanout_ijtag_so {
        Attribute tessent_design_rtl_ports = "o_tdr_scanout";
        Attribute tessent_design_gate_ports = "o_tdr_scanout";
        Source tdr_shift_cap;
    }
    ScanRegister tdr_shift_cap {
        ScanInSource i_tdr_scanin_ijtag_si;
        CaptureSource tdr_shift_cap;
        ResetValue 1'b1;
    }
    DataOutPort o_tdr_update_ijtag_do {
        Attribute tessent_design_rtl_ports = "o_tdr_update";
        Attribute tessent_design_gate_ports = "o_tdr_update";
        Source tdr_shift_cap;
    }
}

// Source: all_out/1_pmm_ip_generation_with_1687_and_apb_sim/tessent_analysis_rtl_icl/icl/PATH_MONITOR_CONTROLLER_clk_cntrl.icl
// Generated; edit the PMM sources, not this file.
Module PATH_MONITOR_CONTROLLER_clk_cntrl {
    // Sources describe settled controls; holding edges and clock routing are abstracted.
    DataInPort i_pmu_clock_mux_en_ijtag_di_1 {
        Attribute tessent_design_rtl_ports = "i_pmu_clock_mux_en";
        Attribute tessent_design_gate_ports = "i_pmu_clock_mux_en";
    }
    DataInPort i_pmu_mode_ijtag_di_2[1:0] {
        Attribute tessent_design_rtl_ports = "{i_pmu_mode[1]} {i_pmu_mode[0]}";
        Attribute tessent_design_gate_ports = "{i_pmu_mode[1]} {i_pmu_mode[0]}";
    }
    ResetPort i_resetn_ijtag_reset {
        Attribute tessent_design_rtl_ports = "i_resetn";
        Attribute tessent_design_gate_ports = "i_resetn";
        ActivePolarity 0;
    }
    DataInPort i_testmode_sibSelect_ijtag_di_3 {
        Attribute tessent_design_rtl_ports = "i_testmode_sibSelect";
        Attribute tessent_design_gate_ports = "i_testmode_sibSelect";
    }
    DataInPort i_sib_loop_bypass_tdr_sibSelect_ijtag_di_4 {
        Attribute tessent_design_rtl_ports = "i_sib_loop_bypass_tdr_sibSelect";
        Attribute tessent_design_gate_ports = "i_sib_loop_bypass_tdr_sibSelect";
    }
    DataInPort i_pmu_path_select_tdr_sibSelect_ijtag_di_5[1:1] {
        Attribute tessent_design_rtl_ports = "{i_pmu_path_select_tdr_sibSelect[1]}";
        Attribute tessent_design_gate_ports = "{i_pmu_path_select_tdr_sibSelect[1]}";
    }
    DataInPort i_pmu_path_select_tdr_sib_ijtag_di_6[1:1] {
        Attribute tessent_design_rtl_ports = "{i_pmu_path_select_tdr_sib[1]}";
        Attribute tessent_design_gate_ports = "{i_pmu_path_select_tdr_sib[1]}";
    }
    DataInPort i_pmu_delay_select_tdr_sibSelect_ijtag_di_7[1:1] {
        Attribute tessent_design_rtl_ports = "{i_pmu_delay_select_tdr_sibSelect[1]}";
        Attribute tessent_design_gate_ports = "{i_pmu_delay_select_tdr_sibSelect[1]}";
    }
    DataInPort i_pmu_delay_select_tdr_sib_ijtag_di_8[1:1] {
        Attribute tessent_design_rtl_ports = "{i_pmu_delay_select_tdr_sib[1]}";
        Attribute tessent_design_gate_ports = "{i_pmu_delay_select_tdr_sib[1]}";
    }
    DataInPort i_pmu_collect_status_tdr_sibSelect_ijtag_di_9[1:1] {
        Attribute tessent_design_rtl_ports = "{i_pmu_collect_status_tdr_sibSelect[1]}";
        Attribute tessent_design_gate_ports = "{i_pmu_collect_status_tdr_sibSelect[1]}";
    }
    DataInPort i_pmu_collect_status_tdr_sib_ijtag_di_10[1:1] {
        Attribute tessent_design_rtl_ports = "{i_pmu_collect_status_tdr_sib[1]}";
        Attribute tessent_design_gate_ports = "{i_pmu_collect_status_tdr_sib[1]}";
    }
    DataInPort i_pmu_func_clk_gate_en_tdr_sibSelect_ijtag_di_11 {
        Attribute tessent_design_rtl_ports = "i_pmu_func_clk_gate_en_tdr_sibSelect";
        Attribute tessent_design_gate_ports = "i_pmu_func_clk_gate_en_tdr_sibSelect";
    }
    DataInPort i_sib2_bypass_tdr_sibSelect_ijtag_di_12[1:1] {
        Attribute tessent_design_rtl_ports = "{i_sib2_bypass_tdr_sibSelect[1]}";
        Attribute tessent_design_gate_ports = "{i_sib2_bypass_tdr_sibSelect[1]}";
    }
    DataInPort i_pmu_capture_ff_tdr_sibSelect_ijtag_di_13[1:1] {
        Attribute tessent_design_rtl_ports = "{i_pmu_capture_ff_tdr_sibSelect[1]}";
        Attribute tessent_design_gate_ports = "{i_pmu_capture_ff_tdr_sibSelect[1]}";
    }
    DataInPort i_pmu_capture_ff_tdr_sib_ijtag_di_14[1:1] {
        Attribute tessent_design_rtl_ports = "{i_pmu_capture_ff_tdr_sib[1]}";
        Attribute tessent_design_gate_ports = "{i_pmu_capture_ff_tdr_sib[1]}";
    }
    LogicSignal o_wrck_clk_mux_en_value {
        ((((i_pmu_mode_ijtag_di_2==2'b00)||(i_pmu_mode_ijtag_di_2==2'b11))&&(i_pmu_collect_status_tdr_sib_ijtag_di_10[1]||i_pmu_delay_select_tdr_sib_ijtag_di_8[1]||i_pmu_path_select_tdr_sib_ijtag_di_6[1]||i_pmu_capture_ff_tdr_sib_ijtag_di_14[1]))||(!((i_pmu_mode_ijtag_di_2==2'b00)||(i_pmu_mode_ijtag_di_2==2'b11))&&((i_pmu_collect_status_tdr_sib_ijtag_di_10[1]||i_pmu_capture_ff_tdr_sibSelect_ijtag_di_13[1]))))&&i_pmu_clock_mux_en_ijtag_di_1;
    }
    DataOutPort o_wrck_clk_mux_en_ijtag_do_1[1:1] {
        Attribute tessent_design_rtl_ports = "{o_wrck_clk_mux_en[1]}";
        Attribute tessent_design_gate_ports = "{o_wrck_clk_mux_en[1]}";
        Source o_wrck_clk_mux_en_value;
    }
    LogicSignal o_data_clamp_enable_value {
        (i_pmu_delay_select_tdr_sibSelect_ijtag_di_7[1])||(i_pmu_path_select_tdr_sibSelect_ijtag_di_5[1]);
    }
    DataOutPort o_data_clamp_enable_ijtag_do_2 {
        Attribute tessent_design_rtl_ports = "o_data_clamp_enable";
        Attribute tessent_design_gate_ports = "o_data_clamp_enable";
        Source o_data_clamp_enable_value;
    }
    LogicSignal o_mutex_clk_gate_en_value {
        i_sib_loop_bypass_tdr_sibSelect_ijtag_di_4||(i_pmu_path_select_tdr_sibSelect_ijtag_di_5[1])||i_testmode_sibSelect_ijtag_di_3||(i_pmu_delay_select_tdr_sibSelect_ijtag_di_7[1])||(i_pmu_collect_status_tdr_sibSelect_ijtag_di_9[1])||i_pmu_func_clk_gate_en_tdr_sibSelect_ijtag_di_11||(i_sib2_bypass_tdr_sibSelect_ijtag_di_12[1])||(i_pmu_capture_ff_tdr_sibSelect_ijtag_di_13[1]);
    }
    DataOutPort o_mutex_clk_gate_en_ijtag_do_3 {
        Attribute tessent_design_rtl_ports = "o_mutex_clk_gate_en";
        Attribute tessent_design_gate_ports = "o_mutex_clk_gate_en";
        Source o_mutex_clk_gate_en_value;
    }
    TCKPort i_wrck_gated_mutex_ijtag_tck {
        Attribute tessent_design_rtl_ports = "i_wrck_gated_mutex";
        Attribute tessent_design_gate_ports = "i_wrck_gated_mutex";
    }
}

// Source: all_out/1_pmm_ip_generation_with_1687_and_apb_sim/tessent_analysis_rtl_icl/icl/PATH_MONITOR_CONTROLLER_pmm_tai_sib_block.icl
// Generated; edit the PMM sources, not this file.
Module PATH_MONITOR_CONTROLLER_pmm_tai_sib_block {
    ScanMux sib2_muxout_1_ SelectedBy sib2_inst_1.sibSelect_reg_ijtag_do_2[2:0] {
        3'd0 : i_sib2_bypass_tdr_scanout_ijtag_from_so_6[1];
        3'd1 : i_pmu_path_select_tdr_scanout_ijtag_from_so_3[1];
        3'd2 : i_pmu_delay_select_tdr_scanout_ijtag_from_so_4[1];
        3'd3 : i_pmu_collect_status_tdr_scanout_ijtag_from_so_5[1];
        3'd4 : i_pmu_capture_ff_tdr_scanout_ijtag_from_so_7[1];
        3'd5 : i_scanin_ijtag_si;
        3'd6 : i_scanin_ijtag_si;
        3'd7 : i_scanin_ijtag_si;
    }
    ScanMux sib1_muxout SelectedBy sib1_inst.sibSelect_reg_ijtag_do_2[2:0] {
        3'd0 : loop_sel_inst.TDO_ijtag_so;
        3'd1 : i_testmode_tdr_scanout_ijtag_from_so_1;
        3'd2 : i_pmu_func_clk_gate_en_tdr_scanout_ijtag_from_so_2;
        3'd3 : i_scanin_ijtag_si;
        3'd4 : i_scanin_ijtag_si;
        3'd5 : i_scanin_ijtag_si;
        3'd6 : i_scanin_ijtag_si;
        3'd7 : i_scanin_ijtag_si;
    }
    ScanMux loop_return SelectedBy loop_sel_inst.sibSelect_reg_ijtag_do_2 {
        4'd0 : i_sib_loop_bypass_tdr_scanout_ijtag_from_so_8;
        4'd1 : sib2_inst_1.TDO_ijtag_so;
        4'd2 : i_scanin_ijtag_si;
        4'd3 : i_scanin_ijtag_si;
        4'd4 : i_scanin_ijtag_si;
        4'd5 : i_scanin_ijtag_si;
        4'd6 : i_scanin_ijtag_si;
        4'd7 : i_scanin_ijtag_si;
        4'd8 : i_scanin_ijtag_si;
        4'd9 : i_scanin_ijtag_si;
        4'd10 : i_scanin_ijtag_si;
        4'd11 : i_scanin_ijtag_si;
        4'd12 : i_scanin_ijtag_si;
        4'd13 : i_scanin_ijtag_si;
        4'd14 : i_scanin_ijtag_si;
        4'd15 : i_scanin_ijtag_si;
    }
    ScanInPort i_scanin_ijtag_si {
        Attribute tessent_design_rtl_ports = "i_scanin";
        Attribute tessent_design_gate_ports = "i_scanin";
    }
    ResetPort i_resetn_ijtag_reset {
        Attribute tessent_design_rtl_ports = "i_resetn";
        Attribute tessent_design_gate_ports = "i_resetn";
        ActivePolarity 0;
    }
    TCKPort i_tck_ijtag_tck {
        Attribute tessent_design_rtl_ports = "i_tck";
        Attribute tessent_design_gate_ports = "i_tck";
    }
    CaptureEnPort i_captureEn_ijtag_ce {
        Attribute tessent_design_rtl_ports = "i_captureEn";
        Attribute tessent_design_gate_ports = "i_captureEn";
    }
    ShiftEnPort i_shiftEn_ijtag_se {
        Attribute tessent_design_rtl_ports = "i_shiftEn";
        Attribute tessent_design_gate_ports = "i_shiftEn";
    }
    UpdateEnPort i_updateEn_ijtag_ue {
        Attribute tessent_design_rtl_ports = "i_updateEn";
        Attribute tessent_design_gate_ports = "i_updateEn";
    }
    SelectPort i_select_ijtag_sel {
        Attribute tessent_design_rtl_ports = "i_select";
        Attribute tessent_design_gate_ports = "i_select";
    }
    ScanOutPort o_testmode_tdr_scanin_ijtag_to_si_1 {
        Attribute tessent_design_rtl_ports = "o_testmode_tdr_scanin";
        Attribute tessent_design_gate_ports = "o_testmode_tdr_scanin";
        Source i_scanin_ijtag_si;
    }
    ScanInPort i_testmode_tdr_scanout_ijtag_from_so_1 {
        Attribute tessent_design_rtl_ports = "i_testmode_tdr_scanout";
        Attribute tessent_design_gate_ports = "i_testmode_tdr_scanout";
    }
    ToCaptureEnPort o_testmode_sibCaptureEn_ijtag_to_ce_1 {
        Attribute tessent_design_rtl_ports = "o_testmode_sibCaptureEn";
        Attribute tessent_design_gate_ports = "o_testmode_sibCaptureEn";
    }
    ToShiftEnPort o_testmode_sibShiftEn_ijtag_to_se_1 {
        Attribute tessent_design_rtl_ports = "o_testmode_sibShiftEn";
        Attribute tessent_design_gate_ports = "o_testmode_sibShiftEn";
    }
    ToUpdateEnPort o_testmode_sibUpdateEn_ijtag_to_ue_1 {
        Attribute tessent_design_rtl_ports = "o_testmode_sibUpdateEn";
        Attribute tessent_design_gate_ports = "o_testmode_sibUpdateEn";
    }
    LogicSignal o_testmode_sibSelect_value {
        (sib1_inst.sibSelect_reg_ijtag_do_2 == 3'b001);
    }
    DataOutPort o_testmode_sibSelect_ijtag_do_1 {
        Attribute tessent_design_rtl_ports = "o_testmode_sibSelect";
        Attribute tessent_design_gate_ports = "o_testmode_sibSelect";
        Source o_testmode_sibSelect_value;
    }
    ScanOutPort o_scanout_ijtag_so {
        Attribute tessent_design_rtl_ports = "o_scanout";
        Attribute tessent_design_gate_ports = "o_scanout";
        Source sib1_inst.TDO_ijtag_so;
    }
    ScanOutPort o_pmu_func_clk_gate_en_tdr_scanin_ijtag_to_si_2 {
        Attribute tessent_design_rtl_ports = "o_pmu_func_clk_gate_en_tdr_scanin";
        Attribute tessent_design_gate_ports = "o_pmu_func_clk_gate_en_tdr_scanin";
        Source i_scanin_ijtag_si;
    }
    ScanInPort i_pmu_func_clk_gate_en_tdr_scanout_ijtag_from_so_2 {
        Attribute tessent_design_rtl_ports = "i_pmu_func_clk_gate_en_tdr_scanout";
        Attribute tessent_design_gate_ports = "i_pmu_func_clk_gate_en_tdr_scanout";
    }
    ToCaptureEnPort o_pmu_func_clk_gate_en_tdr_sibCaptureEn_ijtag_to_ce_2 {
        Attribute tessent_design_rtl_ports = "o_pmu_func_clk_gate_en_tdr_sibCaptureEn";
        Attribute tessent_design_gate_ports = "o_pmu_func_clk_gate_en_tdr_sibCaptureEn";
    }
    ToShiftEnPort o_pmu_func_clk_gate_en_tdr_sibShiftEn_ijtag_to_se_2 {
        Attribute tessent_design_rtl_ports = "o_pmu_func_clk_gate_en_tdr_sibShiftEn";
        Attribute tessent_design_gate_ports = "o_pmu_func_clk_gate_en_tdr_sibShiftEn";
    }
    ToUpdateEnPort o_pmu_func_clk_gate_en_tdr_sibUpdateEn_ijtag_to_ue_2 {
        Attribute tessent_design_rtl_ports = "o_pmu_func_clk_gate_en_tdr_sibUpdateEn";
        Attribute tessent_design_gate_ports = "o_pmu_func_clk_gate_en_tdr_sibUpdateEn";
    }
    LogicSignal o_pmu_func_clk_gate_en_tdr_sibSelect_value {
        (sib1_inst.sibSelect_reg_ijtag_do_2 == 3'b010);
    }
    DataOutPort o_pmu_func_clk_gate_en_tdr_sibSelect_ijtag_do_2 {
        Attribute tessent_design_rtl_ports = "o_pmu_func_clk_gate_en_tdr_sibSelect";
        Attribute tessent_design_gate_ports = "o_pmu_func_clk_gate_en_tdr_sibSelect";
        Source o_pmu_func_clk_gate_en_tdr_sibSelect_value;
    }
    ScanOutPort o_pmu_path_select_tdr_scanin_ijtag_to_si_3[1] {
        Attribute tessent_design_rtl_ports = "{o_pmu_path_select_tdr_scanin[1]}";
        Attribute tessent_design_gate_ports = "{o_pmu_path_select_tdr_scanin[1]}";
        Source i_scanin_ijtag_si;
    }
    ScanInPort i_pmu_path_select_tdr_scanout_ijtag_from_so_3[1:1] {
        Attribute tessent_design_rtl_ports = "{i_pmu_path_select_tdr_scanout[1]}";
        Attribute tessent_design_gate_ports = "{i_pmu_path_select_tdr_scanout[1]}";
    }
    ToCaptureEnPort o_pmu_path_select_tdr_sibCaptureEn_ijtag_to_ce_3[1:1] {
        Attribute tessent_design_rtl_ports = "{o_pmu_path_select_tdr_sibCaptureEn[1]}";
        Attribute tessent_design_gate_ports = "{o_pmu_path_select_tdr_sibCaptureEn[1]}";
    }
    ToShiftEnPort o_pmu_path_select_tdr_sibShiftEn_ijtag_to_se_3[1:1] {
        Attribute tessent_design_rtl_ports = "{o_pmu_path_select_tdr_sibShiftEn[1]}";
        Attribute tessent_design_gate_ports = "{o_pmu_path_select_tdr_sibShiftEn[1]}";
    }
    ToUpdateEnPort o_pmu_path_select_tdr_sibUpdateEn_ijtag_to_ue_3[1:1] {
        Attribute tessent_design_rtl_ports = "{o_pmu_path_select_tdr_sibUpdateEn[1]}";
        Attribute tessent_design_gate_ports = "{o_pmu_path_select_tdr_sibUpdateEn[1]}";
    }
    LogicSignal o_pmu_path_select_tdr_sibSelect_1__value {
        (sib2_inst_1.sibSelect_reg_ijtag_do_2 == 3'b001);
    }
    DataOutPort o_pmu_path_select_tdr_sibSelect_ijtag_do_3[1] {
        Attribute tessent_design_rtl_ports = "{o_pmu_path_select_tdr_sibSelect[1]}";
        Attribute tessent_design_gate_ports = "{o_pmu_path_select_tdr_sibSelect[1]}";
        Source o_pmu_path_select_tdr_sibSelect_1__value;
    }
    LogicSignal o_pmu_path_select_tdr_sib_1__value {
        (sib2_inst_1.TDR_ijtag_do_1 == 3'b001);
    }
    DataOutPort o_pmu_path_select_tdr_sib_ijtag_do_4[1] {
        Attribute tessent_design_rtl_ports = "{o_pmu_path_select_tdr_sib[1]}";
        Attribute tessent_design_gate_ports = "{o_pmu_path_select_tdr_sib[1]}";
        Source o_pmu_path_select_tdr_sib_1__value;
    }
    ScanOutPort o_pmu_delay_select_tdr_scanin_ijtag_to_si_4[1] {
        Attribute tessent_design_rtl_ports = "{o_pmu_delay_select_tdr_scanin[1]}";
        Attribute tessent_design_gate_ports = "{o_pmu_delay_select_tdr_scanin[1]}";
        Source i_scanin_ijtag_si;
    }
    ScanInPort i_pmu_delay_select_tdr_scanout_ijtag_from_so_4[1:1] {
        Attribute tessent_design_rtl_ports = "{i_pmu_delay_select_tdr_scanout[1]}";
        Attribute tessent_design_gate_ports = "{i_pmu_delay_select_tdr_scanout[1]}";
    }
    ToCaptureEnPort o_pmu_delay_select_tdr_sibCaptureEn_ijtag_to_ce_4[1:1] {
        Attribute tessent_design_rtl_ports = "{o_pmu_delay_select_tdr_sibCaptureEn[1]}";
        Attribute tessent_design_gate_ports = "{o_pmu_delay_select_tdr_sibCaptureEn[1]}";
    }
    ToShiftEnPort o_pmu_delay_select_tdr_sibShiftEn_ijtag_to_se_4[1:1] {
        Attribute tessent_design_rtl_ports = "{o_pmu_delay_select_tdr_sibShiftEn[1]}";
        Attribute tessent_design_gate_ports = "{o_pmu_delay_select_tdr_sibShiftEn[1]}";
    }
    ToUpdateEnPort o_pmu_delay_select_tdr_sibUpdateEn_ijtag_to_ue_4[1:1] {
        Attribute tessent_design_rtl_ports = "{o_pmu_delay_select_tdr_sibUpdateEn[1]}";
        Attribute tessent_design_gate_ports = "{o_pmu_delay_select_tdr_sibUpdateEn[1]}";
    }
    LogicSignal o_pmu_delay_select_tdr_sibSelect_1__value {
        (sib2_inst_1.sibSelect_reg_ijtag_do_2 == 3'b010);
    }
    DataOutPort o_pmu_delay_select_tdr_sibSelect_ijtag_do_5[1] {
        Attribute tessent_design_rtl_ports = "{o_pmu_delay_select_tdr_sibSelect[1]}";
        Attribute tessent_design_gate_ports = "{o_pmu_delay_select_tdr_sibSelect[1]}";
        Source o_pmu_delay_select_tdr_sibSelect_1__value;
    }
    LogicSignal o_pmu_delay_select_tdr_sib_1__value {
        (sib2_inst_1.TDR_ijtag_do_1 == 3'b010);
    }
    DataOutPort o_pmu_delay_select_tdr_sib_ijtag_do_6[1] {
        Attribute tessent_design_rtl_ports = "{o_pmu_delay_select_tdr_sib[1]}";
        Attribute tessent_design_gate_ports = "{o_pmu_delay_select_tdr_sib[1]}";
        Source o_pmu_delay_select_tdr_sib_1__value;
    }
    ScanOutPort o_pmu_collect_status_tdr_scanin_ijtag_to_si_5[1] {
        Attribute tessent_design_rtl_ports = "{o_pmu_collect_status_tdr_scanin[1]}";
        Attribute tessent_design_gate_ports = "{o_pmu_collect_status_tdr_scanin[1]}";
        Source i_scanin_ijtag_si;
    }
    ScanInPort i_pmu_collect_status_tdr_scanout_ijtag_from_so_5[1:1] {
        Attribute tessent_design_rtl_ports = "{i_pmu_collect_status_tdr_scanout[1]}";
        Attribute tessent_design_gate_ports = "{i_pmu_collect_status_tdr_scanout[1]}";
    }
    ToCaptureEnPort o_pmu_collect_status_tdr_sibCaptureEn_ijtag_to_ce_5[1:1] {
        Attribute tessent_design_rtl_ports = "{o_pmu_collect_status_tdr_sibCaptureEn[1]}";
        Attribute tessent_design_gate_ports = "{o_pmu_collect_status_tdr_sibCaptureEn[1]}";
    }
    ToShiftEnPort o_pmu_collect_status_tdr_sibShiftEn_ijtag_to_se_5[1:1] {
        Attribute tessent_design_rtl_ports = "{o_pmu_collect_status_tdr_sibShiftEn[1]}";
        Attribute tessent_design_gate_ports = "{o_pmu_collect_status_tdr_sibShiftEn[1]}";
    }
    ToUpdateEnPort o_pmu_collect_status_tdr_sibUpdateEn_ijtag_to_ue_5[1:1] {
        Attribute tessent_design_rtl_ports = "{o_pmu_collect_status_tdr_sibUpdateEn[1]}";
        Attribute tessent_design_gate_ports = "{o_pmu_collect_status_tdr_sibUpdateEn[1]}";
    }
    LogicSignal o_pmu_collect_status_tdr_sibSelect_1__value {
        (sib2_inst_1.sibSelect_reg_ijtag_do_2 == 3'b011);
    }
    DataOutPort o_pmu_collect_status_tdr_sibSelect_ijtag_do_7[1] {
        Attribute tessent_design_rtl_ports = "{o_pmu_collect_status_tdr_sibSelect[1]}";
        Attribute tessent_design_gate_ports = "{o_pmu_collect_status_tdr_sibSelect[1]}";
        Source o_pmu_collect_status_tdr_sibSelect_1__value;
    }
    LogicSignal o_pmu_collect_status_tdr_sib_1__value {
        (sib2_inst_1.TDR_ijtag_do_1 == 3'b011);
    }
    DataOutPort o_pmu_collect_status_tdr_sib_ijtag_do_8[1] {
        Attribute tessent_design_rtl_ports = "{o_pmu_collect_status_tdr_sib[1]}";
        Attribute tessent_design_gate_ports = "{o_pmu_collect_status_tdr_sib[1]}";
        Source o_pmu_collect_status_tdr_sib_1__value;
    }
    ScanOutPort o_sib2_bypass_tdr_scanin_ijtag_to_si_6[1] {
        Attribute tessent_design_rtl_ports = "{o_sib2_bypass_tdr_scanin[1]}";
        Attribute tessent_design_gate_ports = "{o_sib2_bypass_tdr_scanin[1]}";
        Source i_scanin_ijtag_si;
    }
    ScanInPort i_sib2_bypass_tdr_scanout_ijtag_from_so_6[1:1] {
        Attribute tessent_design_rtl_ports = "{i_sib2_bypass_tdr_scanout[1]}";
        Attribute tessent_design_gate_ports = "{i_sib2_bypass_tdr_scanout[1]}";
    }
    ToCaptureEnPort o_sib2_bypass_tdr_sibCaptureEn_ijtag_to_ce_6[1:1] {
        Attribute tessent_design_rtl_ports = "{o_sib2_bypass_tdr_sibCaptureEn[1]}";
        Attribute tessent_design_gate_ports = "{o_sib2_bypass_tdr_sibCaptureEn[1]}";
    }
    ToShiftEnPort o_sib2_bypass_tdr_sibShiftEn_ijtag_to_se_6[1:1] {
        Attribute tessent_design_rtl_ports = "{o_sib2_bypass_tdr_sibShiftEn[1]}";
        Attribute tessent_design_gate_ports = "{o_sib2_bypass_tdr_sibShiftEn[1]}";
    }
    ToUpdateEnPort o_sib2_bypass_tdr_sibUpdateEn_ijtag_to_ue_6[1:1] {
        Attribute tessent_design_rtl_ports = "{o_sib2_bypass_tdr_sibUpdateEn[1]}";
        Attribute tessent_design_gate_ports = "{o_sib2_bypass_tdr_sibUpdateEn[1]}";
    }
    LogicSignal o_sib2_bypass_tdr_sibSelect_1__value {
        (sib2_inst_1.sibSelect_reg_ijtag_do_2 == 3'b000) && (loop_sel_inst.sibSelect_reg_ijtag_do_2 == 4'd1);
    }
    DataOutPort o_sib2_bypass_tdr_sibSelect_ijtag_do_9[1] {
        Attribute tessent_design_rtl_ports = "{o_sib2_bypass_tdr_sibSelect[1]}";
        Attribute tessent_design_gate_ports = "{o_sib2_bypass_tdr_sibSelect[1]}";
        Source o_sib2_bypass_tdr_sibSelect_1__value;
    }
    ScanInPort i_pmu_capture_ff_tdr_scanout_ijtag_from_so_7[1:1] {
        Attribute tessent_design_rtl_ports = "{i_pmu_capture_ff_tdr_scanout[1]}";
        Attribute tessent_design_gate_ports = "{i_pmu_capture_ff_tdr_scanout[1]}";
    }
    ScanOutPort o_pmu_capture_ff_tdr_scanin_ijtag_to_si_7[1] {
        Attribute tessent_design_rtl_ports = "{o_pmu_capture_ff_tdr_scanin[1]}";
        Attribute tessent_design_gate_ports = "{o_pmu_capture_ff_tdr_scanin[1]}";
        Source i_scanin_ijtag_si;
    }
    ToCaptureEnPort o_pmu_capture_ff_tdr_sibCaptureEn_ijtag_to_ce_7[1:1] {
        Attribute tessent_design_rtl_ports = "{o_pmu_capture_ff_tdr_sibCaptureEn[1]}";
        Attribute tessent_design_gate_ports = "{o_pmu_capture_ff_tdr_sibCaptureEn[1]}";
    }
    ToShiftEnPort o_pmu_capture_ff_tdr_sibShiftEn_ijtag_to_se_7[1:1] {
        Attribute tessent_design_rtl_ports = "{o_pmu_capture_ff_tdr_sibShiftEn[1]}";
        Attribute tessent_design_gate_ports = "{o_pmu_capture_ff_tdr_sibShiftEn[1]}";
    }
    ToUpdateEnPort o_pmu_capture_ff_tdr_sibUpdateEn_ijtag_to_ue_7[1:1] {
        Attribute tessent_design_rtl_ports = "{o_pmu_capture_ff_tdr_sibUpdateEn[1]}";
        Attribute tessent_design_gate_ports = "{o_pmu_capture_ff_tdr_sibUpdateEn[1]}";
    }
    LogicSignal o_pmu_capture_ff_tdr_sibSelect_1__value {
        (sib2_inst_1.sibSelect_reg_ijtag_do_2 == 3'b100);
    }
    DataOutPort o_pmu_capture_ff_tdr_sibSelect_ijtag_do_10[1] {
        Attribute tessent_design_rtl_ports = "{o_pmu_capture_ff_tdr_sibSelect[1]}";
        Attribute tessent_design_gate_ports = "{o_pmu_capture_ff_tdr_sibSelect[1]}";
        Source o_pmu_capture_ff_tdr_sibSelect_1__value;
    }
    LogicSignal o_pmu_capture_ff_tdr_sib_1__value {
        (sib2_inst_1.TDR_ijtag_do_1 == 3'b100);
    }
    DataOutPort o_pmu_capture_ff_tdr_sib_ijtag_do_11[1] {
        Attribute tessent_design_rtl_ports = "{o_pmu_capture_ff_tdr_sib[1]}";
        Attribute tessent_design_gate_ports = "{o_pmu_capture_ff_tdr_sib[1]}";
        Source o_pmu_capture_ff_tdr_sib_1__value;
    }
    ScanOutPort o_sib_loop_bypass_tdr_scanin_ijtag_to_si_8 {
        Attribute tessent_design_rtl_ports = "o_sib_loop_bypass_tdr_scanin";
        Attribute tessent_design_gate_ports = "o_sib_loop_bypass_tdr_scanin";
        Source i_scanin_ijtag_si;
    }
    ScanInPort i_sib_loop_bypass_tdr_scanout_ijtag_from_so_8 {
        Attribute tessent_design_rtl_ports = "i_sib_loop_bypass_tdr_scanout";
        Attribute tessent_design_gate_ports = "i_sib_loop_bypass_tdr_scanout";
    }
    ToCaptureEnPort o_sib_loop_bypass_tdr_sibCaptureEn_ijtag_to_ce_8 {
        Attribute tessent_design_rtl_ports = "o_sib_loop_bypass_tdr_sibCaptureEn";
        Attribute tessent_design_gate_ports = "o_sib_loop_bypass_tdr_sibCaptureEn";
    }
    ToShiftEnPort o_sib_loop_bypass_tdr_sibShiftEn_ijtag_to_se_8 {
        Attribute tessent_design_rtl_ports = "o_sib_loop_bypass_tdr_sibShiftEn";
        Attribute tessent_design_gate_ports = "o_sib_loop_bypass_tdr_sibShiftEn";
    }
    ToUpdateEnPort o_sib_loop_bypass_tdr_sibUpdateEn_ijtag_to_ue_8 {
        Attribute tessent_design_rtl_ports = "o_sib_loop_bypass_tdr_sibUpdateEn";
        Attribute tessent_design_gate_ports = "o_sib_loop_bypass_tdr_sibUpdateEn";
    }
    LogicSignal o_sib_loop_bypass_tdr_sibSelect_value {
        (loop_sel_inst.sibSelect_reg_ijtag_do_2 == 4'b0000) && (sib1_inst.sibSelect_reg_ijtag_do_2 == 3'b000);
    }
    DataOutPort o_sib_loop_bypass_tdr_sibSelect_ijtag_do_12 {
        Attribute tessent_design_rtl_ports = "o_sib_loop_bypass_tdr_sibSelect";
        Attribute tessent_design_gate_ports = "o_sib_loop_bypass_tdr_sibSelect";
        Source o_sib_loop_bypass_tdr_sibSelect_value;
    }
    LogicSignal sib2_inst_1_Select_value {
        i_select_ijtag_sel&&(loop_sel_inst.sibSelect_reg_ijtag_do_2== 4'd1);
    }
    Instance sib2_inst_1 Of PATH_MONITOR_CONTROLLER_PMM_1687_SIB_REG_3 {
        InputPort TDI_ijtag_si = sib2_muxout_1_;
        InputPort TRSTN_ijtag_reset = i_resetn_ijtag_reset;
        InputPort clk_ijtag_tck = i_tck_ijtag_tck;
        InputPort Select_ijtag_sel = sib2_inst_1_Select_value;
        InputPort ShiftEn_ijtag_se = i_shiftEn_ijtag_se;
        InputPort CaptureEn_ijtag_ce = i_captureEn_ijtag_ce;
        InputPort UpdateEn_ijtag_ue = i_updateEn_ijtag_ue;
    }
    LogicSignal loop_sel_inst_Select_value {
        i_select_ijtag_sel&&(sib1_inst.sibSelect_reg_ijtag_do_2==3'b000);
    }
    Instance loop_sel_inst Of PATH_MONITOR_CONTROLLER_PMM_1687_SIB_REG_4 {
        InputPort TDI_ijtag_si = loop_return;
        InputPort TRSTN_ijtag_reset = i_resetn_ijtag_reset;
        InputPort clk_ijtag_tck = i_tck_ijtag_tck;
        InputPort Select_ijtag_sel = loop_sel_inst_Select_value;
        InputPort ShiftEn_ijtag_se = i_shiftEn_ijtag_se;
        InputPort CaptureEn_ijtag_ce = i_captureEn_ijtag_ce;
        InputPort UpdateEn_ijtag_ue = i_updateEn_ijtag_ue;
    }
    Instance sib1_inst Of PATH_MONITOR_CONTROLLER_PMM_1687_SIB_REG_3 {
        InputPort TDI_ijtag_si = sib1_muxout;
        InputPort TRSTN_ijtag_reset = i_resetn_ijtag_reset;
        InputPort clk_ijtag_tck = i_tck_ijtag_tck;
        InputPort Select_ijtag_sel = i_select_ijtag_sel;
        InputPort ShiftEn_ijtag_se = i_shiftEn_ijtag_se;
        InputPort CaptureEn_ijtag_ce = i_captureEn_ijtag_ce;
        InputPort UpdateEn_ijtag_ue = i_updateEn_ijtag_ue;
    }
    ScanInterface client {
        Port i_scanin_ijtag_si;
        Port o_scanout_ijtag_so;
        Port i_select_ijtag_sel;
        Port i_shiftEn_ijtag_se;
        Port i_captureEn_ijtag_ce;
        Port i_updateEn_ijtag_ue;
        Port i_resetn_ijtag_reset;
        Port i_tck_ijtag_tck;
    }
    ScanInterface host_o_testmode_sibShiftEn {
        Port o_testmode_tdr_scanin_ijtag_to_si_1;
        Port i_testmode_tdr_scanout_ijtag_from_so_1;
        Port o_testmode_sibShiftEn_ijtag_to_se_1;
        Port o_testmode_sibCaptureEn_ijtag_to_ce_1;
        Port o_testmode_sibUpdateEn_ijtag_to_ue_1;
    }
    ScanInterface host_o_pmu_func_clk_gate_en_tdr_sibShiftEn {
        Port o_pmu_func_clk_gate_en_tdr_scanin_ijtag_to_si_2;
        Port i_pmu_func_clk_gate_en_tdr_scanout_ijtag_from_so_2;
        Port o_pmu_func_clk_gate_en_tdr_sibShiftEn_ijtag_to_se_2;
        Port o_pmu_func_clk_gate_en_tdr_sibCaptureEn_ijtag_to_ce_2;
        Port o_pmu_func_clk_gate_en_tdr_sibUpdateEn_ijtag_to_ue_2;
    }
    ScanInterface host_o_pmu_path_select_tdr_sibShiftEn1 {
        Port i_pmu_path_select_tdr_scanout_ijtag_from_so_3[1];
        Port o_pmu_path_select_tdr_sibShiftEn_ijtag_to_se_3[1];
        Port o_pmu_path_select_tdr_sibCaptureEn_ijtag_to_ce_3[1];
        Port o_pmu_path_select_tdr_sibUpdateEn_ijtag_to_ue_3[1];
    }
    ScanInterface host_o_pmu_delay_select_tdr_sibShiftEn1 {
        Port i_pmu_delay_select_tdr_scanout_ijtag_from_so_4[1];
        Port o_pmu_delay_select_tdr_sibShiftEn_ijtag_to_se_4[1];
        Port o_pmu_delay_select_tdr_sibCaptureEn_ijtag_to_ce_4[1];
        Port o_pmu_delay_select_tdr_sibUpdateEn_ijtag_to_ue_4[1];
    }
    ScanInterface host_o_pmu_collect_status_tdr_sibShiftEn1 {
        Port i_pmu_collect_status_tdr_scanout_ijtag_from_so_5[1];
        Port o_pmu_collect_status_tdr_sibShiftEn_ijtag_to_se_5[1];
        Port o_pmu_collect_status_tdr_sibCaptureEn_ijtag_to_ce_5[1];
        Port o_pmu_collect_status_tdr_sibUpdateEn_ijtag_to_ue_5[1];
    }
    ScanInterface host_o_sib2_bypass_tdr_sibShiftEn1 {
        Port o_sib2_bypass_tdr_scanin_ijtag_to_si_6[1];
        Port i_sib2_bypass_tdr_scanout_ijtag_from_so_6[1];
        Port o_sib2_bypass_tdr_sibShiftEn_ijtag_to_se_6[1];
        Port o_sib2_bypass_tdr_sibCaptureEn_ijtag_to_ce_6[1];
        Port o_sib2_bypass_tdr_sibUpdateEn_ijtag_to_ue_6[1];
    }
    ScanInterface host_o_pmu_capture_ff_tdr_sibShiftEn1 {
        Port i_pmu_capture_ff_tdr_scanout_ijtag_from_so_7[1];
        Port o_pmu_capture_ff_tdr_sibShiftEn_ijtag_to_se_7[1];
        Port o_pmu_capture_ff_tdr_sibCaptureEn_ijtag_to_ce_7[1];
        Port o_pmu_capture_ff_tdr_sibUpdateEn_ijtag_to_ue_7[1];
    }
    ScanInterface host_o_sib_loop_bypass_tdr_sibShiftEn {
        Port o_sib_loop_bypass_tdr_scanin_ijtag_to_si_8;
        Port i_sib_loop_bypass_tdr_scanout_ijtag_from_so_8;
        Port o_sib_loop_bypass_tdr_sibShiftEn_ijtag_to_se_8;
        Port o_sib_loop_bypass_tdr_sibCaptureEn_ijtag_to_ce_8;
        Port o_sib_loop_bypass_tdr_sibUpdateEn_ijtag_to_ue_8;
    }
}

// Source: all_out/1_pmm_ip_generation_with_1687_and_apb_sim/tessent_analysis_rtl_icl/icl/PMU_BUNDLE.icl
// Generated; edit the PMM sources, not this file.
Module PMU_BUNDLE {
    SelectPort pmc2pmu_configure_path_sel_ijtag_sel_1[1:1] {
        Attribute tessent_design_rtl_ports = "{pmc2pmu_configure_path_sel[1]}";
        Attribute tessent_design_gate_ports = "{pmc2pmu_configure_path_sel[1]}";
    }
    SelectPort pmc2pmu_configure_delay_sel_ijtag_sel_2[1:1] {
        Attribute tessent_design_rtl_ports = "{pmc2pmu_configure_delay_sel[1]}";
        Attribute tessent_design_gate_ports = "{pmc2pmu_configure_delay_sel[1]}";
    }
    SelectPort pmc2pmu_collect_status_ijtag_sel_3[1:1] {
        Attribute tessent_design_rtl_ports = "{pmc2pmu_collect_status[1]}";
        Attribute tessent_design_gate_ports = "{pmc2pmu_collect_status[1]}";
    }
    SelectPort pmc2pmu_capture_ff_shift_ijtag_sel_4[1:1] {
        Attribute tessent_design_rtl_ports = "{pmc2pmu_capture_ff_shift[1]}";
        Attribute tessent_design_gate_ports = "{pmc2pmu_capture_ff_shift[1]}";
    }
    ScanInPort pmc2pmu_data_in_ijtag_si_1[1:1] {
        Attribute tessent_design_rtl_ports = "{pmc2pmu_data_in[1]}";
        Attribute tessent_design_gate_ports = "{pmc2pmu_data_in[1]}";
    }
    ScanOutPort pmu2pmc_data_out_ijtag_so_1[1:1] {
        Attribute tessent_design_rtl_ports = "{pmu2pmc_data_out[1]}";
        Attribute tessent_design_gate_ports = "{pmu2pmc_data_out[1]}";
        Source path_monitor_unit5_loop1.o_pmu_data_out_ijtag_so_1;
    }
    ScanInPort pmc2pmu_capture_ff_tdi_ijtag_si_2[1:1] {
        Attribute tessent_design_rtl_ports = "{pmc2pmu_capture_ff_tdi[1]}";
        Attribute tessent_design_gate_ports = "{pmc2pmu_capture_ff_tdi[1]}";
    }
    ScanOutPort pmu2pmc_capture_ff_tdo_ijtag_so_2[1:1] {
        Attribute tessent_design_rtl_ports = "{pmu2pmc_capture_ff_tdo[1]}";
        Attribute tessent_design_gate_ports = "{pmu2pmc_capture_ff_tdo[1]}";
        Source path_monitor_unit5_loop1.I_capture_tdo_ijtag_so_2;
    }
    ScanInterface client_pmc2pmu_configure_path_sel1 {
        Port pmc2pmu_data_in_ijtag_si_1[1];
        Port pmu2pmc_data_out_ijtag_so_1[1];
        Port pmc2pmu_configure_path_sel_ijtag_sel_1[1];
    }
    ScanInterface client_pmc2pmu_configure_delay_sel1 {
        Port pmc2pmu_data_in_ijtag_si_1[1];
        Port pmu2pmc_data_out_ijtag_so_1[1];
        Port pmc2pmu_configure_delay_sel_ijtag_sel_2[1];
    }
    ScanInterface client_pmc2pmu_collect_status1 {
        Port pmc2pmu_data_in_ijtag_si_1[1];
        Port pmu2pmc_data_out_ijtag_so_1[1];
        Port pmc2pmu_collect_status_ijtag_sel_3[1];
    }
    ScanInterface client_pmc2pmu_capture_ff_shift1 {
        Port pmc2pmu_capture_ff_tdi_ijtag_si_2[1];
        Port pmu2pmc_capture_ff_tdo_ijtag_so_2[1];
        Port pmc2pmu_capture_ff_shift_ijtag_sel_4[1];
    }
    Instance path_monitor_unit0_loop1 Of PMU {
        InputPort i_pmu_configure_path_sel_ijtag_sel_1 = pmc2pmu_configure_path_sel_ijtag_sel_1[1];
        InputPort i_pmu_configure_delay_sel_ijtag_sel_2 = pmc2pmu_configure_delay_sel_ijtag_sel_2[1];
        InputPort i_pmu_status_collect_ijtag_sel_3 = pmc2pmu_collect_status_ijtag_sel_3[1];
        InputPort I_capture_shift_ijtag_sel_4 = pmc2pmu_capture_ff_shift_ijtag_sel_4[1];
        InputPort i_pmu_data_in_ijtag_si_1 = pmc2pmu_data_in_ijtag_si_1[1];
        InputPort I_capture_tdi_ijtag_si_2 = pmc2pmu_capture_ff_tdi_ijtag_si_2[1];
    }
    Instance path_monitor_unit1_loop1 Of PMU {
        InputPort i_pmu_configure_path_sel_ijtag_sel_1 = pmc2pmu_configure_path_sel_ijtag_sel_1[1];
        InputPort i_pmu_configure_delay_sel_ijtag_sel_2 = pmc2pmu_configure_delay_sel_ijtag_sel_2[1];
        InputPort i_pmu_status_collect_ijtag_sel_3 = pmc2pmu_collect_status_ijtag_sel_3[1];
        InputPort I_capture_shift_ijtag_sel_4 = pmc2pmu_capture_ff_shift_ijtag_sel_4[1];
        InputPort i_pmu_data_in_ijtag_si_1 = path_monitor_unit0_loop1.o_pmu_data_out_ijtag_so_1;
        InputPort I_capture_tdi_ijtag_si_2 = path_monitor_unit0_loop1.I_capture_tdo_ijtag_so_2;
    }
    Instance path_monitor_unit2_loop1 Of PMU {
        InputPort i_pmu_configure_path_sel_ijtag_sel_1 = pmc2pmu_configure_path_sel_ijtag_sel_1[1];
        InputPort i_pmu_configure_delay_sel_ijtag_sel_2 = pmc2pmu_configure_delay_sel_ijtag_sel_2[1];
        InputPort i_pmu_status_collect_ijtag_sel_3 = pmc2pmu_collect_status_ijtag_sel_3[1];
        InputPort I_capture_shift_ijtag_sel_4 = pmc2pmu_capture_ff_shift_ijtag_sel_4[1];
        InputPort i_pmu_data_in_ijtag_si_1 = path_monitor_unit1_loop1.o_pmu_data_out_ijtag_so_1;
        InputPort I_capture_tdi_ijtag_si_2 = path_monitor_unit1_loop1.I_capture_tdo_ijtag_so_2;
    }
    Instance path_monitor_unit3_loop1 Of PMU {
        InputPort i_pmu_configure_path_sel_ijtag_sel_1 = pmc2pmu_configure_path_sel_ijtag_sel_1[1];
        InputPort i_pmu_configure_delay_sel_ijtag_sel_2 = pmc2pmu_configure_delay_sel_ijtag_sel_2[1];
        InputPort i_pmu_status_collect_ijtag_sel_3 = pmc2pmu_collect_status_ijtag_sel_3[1];
        InputPort I_capture_shift_ijtag_sel_4 = pmc2pmu_capture_ff_shift_ijtag_sel_4[1];
        InputPort i_pmu_data_in_ijtag_si_1 = path_monitor_unit2_loop1.o_pmu_data_out_ijtag_so_1;
        InputPort I_capture_tdi_ijtag_si_2 = path_monitor_unit2_loop1.I_capture_tdo_ijtag_so_2;
    }
    Instance path_monitor_unit4_loop1 Of PMU {
        InputPort i_pmu_configure_path_sel_ijtag_sel_1 = pmc2pmu_configure_path_sel_ijtag_sel_1[1];
        InputPort i_pmu_configure_delay_sel_ijtag_sel_2 = pmc2pmu_configure_delay_sel_ijtag_sel_2[1];
        InputPort i_pmu_status_collect_ijtag_sel_3 = pmc2pmu_collect_status_ijtag_sel_3[1];
        InputPort I_capture_shift_ijtag_sel_4 = pmc2pmu_capture_ff_shift_ijtag_sel_4[1];
        InputPort i_pmu_data_in_ijtag_si_1 = path_monitor_unit3_loop1.o_pmu_data_out_ijtag_so_1;
        InputPort I_capture_tdi_ijtag_si_2 = path_monitor_unit3_loop1.I_capture_tdo_ijtag_so_2;
    }
    Instance path_monitor_unit5_loop1 Of PMU {
        InputPort i_pmu_configure_path_sel_ijtag_sel_1 = pmc2pmu_configure_path_sel_ijtag_sel_1[1];
        InputPort i_pmu_configure_delay_sel_ijtag_sel_2 = pmc2pmu_configure_delay_sel_ijtag_sel_2[1];
        InputPort i_pmu_status_collect_ijtag_sel_3 = pmc2pmu_collect_status_ijtag_sel_3[1];
        InputPort I_capture_shift_ijtag_sel_4 = pmc2pmu_capture_ff_shift_ijtag_sel_4[1];
        InputPort i_pmu_data_in_ijtag_si_1 = path_monitor_unit4_loop1.o_pmu_data_out_ijtag_so_1;
        InputPort I_capture_tdi_ijtag_si_2 = path_monitor_unit4_loop1.I_capture_tdo_ijtag_so_2;
    }
}

// Source: all_out/1_pmm_ip_generation_with_1687_and_apb_sim/tessent_analysis_rtl_icl/icl/SIB_LOOP_SEL.icl
// Generated; edit the PMM sources, not this file.
Module SIB_LOOP_SEL {
    ShiftEnPort Shift_ijtag_se;
    CaptureEnPort Capture_ijtag_ce;
    UpdateEnPort Update_ijtag_ue;
    SelectPort Select_ijtag_sel;
    ResetPort TRSTN_ijtag_reset {
        ActivePolarity 0;
    }
    TCKPort TCK_ijtag_tck;
    ScanInPort SI_ijtag_si;
    ScanOutPort SO_ijtag_so {
        Source Value[0];
    }
    ScanInterface client {
        Port SI_ijtag_si;
        Port SO_ijtag_so;
        Port Shift_ijtag_se;
        Port Capture_ijtag_ce;
        Port Update_ijtag_ue;
        Port Select_ijtag_sel;
        Port TRSTN_ijtag_reset;
        Port TCK_ijtag_tck;
    }
    ScanInPort B0_ijtag_from_so_1;
    LogicSignal Enable0 {
        Select_ijtag_sel && (Value == 4'd0);
    }
    ToSelectPort Select0_ijtag_to_sel_1 {
        Source Enable0;
    }
    ScanInPort B1_ijtag_from_so_2;
    LogicSignal Enable1 {
        Select_ijtag_sel && (Value == 4'd1);
    }
    ToSelectPort Select1_ijtag_to_sel_2 {
        Source Enable1;
    }
    ScanRegister Value[3:0] {
        ScanInSource mux;
        CaptureSource Value;
        ResetValue 4'b0;
    }
    DataOutPort Selection_ijtag_do[3:0] {
        Source Value;
    }
    ScanMux mux SelectedBy Value {
        4'd0 : B0_ijtag_from_so_1;
        4'd1 : B1_ijtag_from_so_2;
        4'd2 : SI_ijtag_si;
        4'd3 : SI_ijtag_si;
        4'd4 : SI_ijtag_si;
        4'd5 : SI_ijtag_si;
        4'd6 : SI_ijtag_si;
        4'd7 : SI_ijtag_si;
        4'd8 : SI_ijtag_si;
        4'd9 : SI_ijtag_si;
        4'd10 : SI_ijtag_si;
        4'd11 : SI_ijtag_si;
        4'd12 : SI_ijtag_si;
        4'd13 : SI_ijtag_si;
        4'd14 : SI_ijtag_si;
        4'd15 : SI_ijtag_si;
    }
}

// Source: all_out/1_pmm_ip_generation_with_1687_and_apb_sim/tessent_analysis_rtl_icl/icl/PATH_MONITOR_CONTROLLER_IEEE_1687_module.icl
// Generated; edit the PMM sources, not this file.
Module PATH_MONITOR_CONTROLLER_IEEE_1687_module {
    ScanInPort i_scanin_ijtag_si {
        Attribute tessent_design_rtl_ports = "i_scanin";
        Attribute tessent_design_gate_ports = "i_scanin";
    }
    ResetPort i_resetn_ijtag_reset {
        Attribute tessent_design_rtl_ports = "i_resetn";
        Attribute tessent_design_gate_ports = "i_resetn";
        ActivePolarity 0;
    }
    TCKPort i_tck_ijtag_tck_1 {
        Attribute tessent_design_rtl_ports = "i_tck";
        Attribute tessent_design_gate_ports = "i_tck";
    }
    CaptureEnPort i_captureEn_ijtag_ce {
        Attribute tessent_design_rtl_ports = "i_captureEn";
        Attribute tessent_design_gate_ports = "i_captureEn";
    }
    ShiftEnPort i_shiftEn_ijtag_se {
        Attribute tessent_design_rtl_ports = "i_shiftEn";
        Attribute tessent_design_gate_ports = "i_shiftEn";
    }
    UpdateEnPort i_updateEn_ijtag_ue {
        Attribute tessent_design_rtl_ports = "i_updateEn";
        Attribute tessent_design_gate_ports = "i_updateEn";
    }
    SelectPort i_select_ijtag_sel {
        Attribute tessent_design_rtl_ports = "i_select";
        Attribute tessent_design_gate_ports = "i_select";
    }
    ScanOutPort o_scanout_ijtag_so {
        Attribute tessent_design_rtl_ports = "o_scanout";
        Attribute tessent_design_gate_ports = "o_scanout";
        Source pmm_tai_sib_inst.o_scanout_ijtag_so;
    }
    DataOutPort o_test_mode_1687_ijtag_do_1[5:0] {
        Attribute tessent_design_rtl_ports = "{o_test_mode_1687[5]} {o_test_mode_1687[4]} {o_test_mode_1687[3]} {o_test_mode_1687[2]} {o_test_mode_1687[1]} {o_test_mode_1687[0]}";
        Attribute tessent_design_gate_ports = "{o_test_mode_1687[5]} {o_test_mode_1687[4]} {o_test_mode_1687[3]} {o_test_mode_1687[2]} {o_test_mode_1687[1]} {o_test_mode_1687[0]}";
        Source IEEE_1687_dr_inst.o_test_mode_ijtag_do_1;
    }
    ScanOutPort o_delay_select_ijtag_to_si_1[1:1] {
        Attribute tessent_design_rtl_ports = "{o_delay_select[1]}";
        Attribute tessent_design_gate_ports = "{o_delay_select[1]}";
        Source IEEE_1687_dr_inst.o_delay_select_ijtag_to_si_2;
    }
    ToSelectPort o_pmu_collect_status_ijtag_to_sel_1[1:1] {
        Attribute tessent_design_rtl_ports = "{o_pmu_collect_status[1]}";
        Attribute tessent_design_gate_ports = "{o_pmu_collect_status[1]}";
        Source IEEE_1687_dr_inst.o_pmu_collect_status_ijtag_to_sel_3;
    }
    ToSelectPort o_configure_delay_sel_pmu_ijtag_to_sel_2[1:1] {
        Attribute tessent_design_rtl_ports = "{o_configure_delay_sel_pmu[1]}";
        Attribute tessent_design_gate_ports = "{o_configure_delay_sel_pmu[1]}";
        Source IEEE_1687_dr_inst.o_configure_delay_select_pmu_ijtag_to_sel_2;
    }
    ToSelectPort o_configure_path_sel_pmu_ijtag_to_sel_3[1:1] {
        Attribute tessent_design_rtl_ports = "{o_configure_path_sel_pmu[1]}";
        Attribute tessent_design_gate_ports = "{o_configure_path_sel_pmu[1]}";
        Source IEEE_1687_dr_inst.o_configure_path_sel_pmu_ijtag_to_sel_1;
    }
    ScanOutPort o_path_select_ijtag_to_si_2[1:1] {
        Attribute tessent_design_rtl_ports = "{o_path_select[1]}";
        Attribute tessent_design_gate_ports = "{o_path_select[1]}";
        Source IEEE_1687_dr_inst.o_path_select_ijtag_to_si_1;
    }
    DataOutPort o_pmu_func_clk_gate_en_ijtag_do_2[1:1] {
        Attribute tessent_design_rtl_ports = "{o_pmu_func_clk_gate_en[1]}";
        Attribute tessent_design_gate_ports = "{o_pmu_func_clk_gate_en[1]}";
        Source IEEE_1687_dr_inst.o_pmu_func_clk_gate_en_ijtag_do_2;
    }
    DataOutPort o_testmode_sibSelect_ijtag_do_3 {
        Attribute tessent_design_rtl_ports = "o_testmode_sibSelect";
        Attribute tessent_design_gate_ports = "o_testmode_sibSelect";
        Source pmm_tai_sib_inst.o_testmode_sibSelect_ijtag_do_1;
    }
    DataOutPort o_sib_loop_bypass_tdr_sibSelect_ijtag_do_4 {
        Attribute tessent_design_rtl_ports = "o_sib_loop_bypass_tdr_sibSelect";
        Attribute tessent_design_gate_ports = "o_sib_loop_bypass_tdr_sibSelect";
        Source pmm_tai_sib_inst.o_sib_loop_bypass_tdr_sibSelect_ijtag_do_12;
    }
    DataOutPort o_pmu_path_select_tdr_sibSelect_ijtag_do_5[1:1] {
        Attribute tessent_design_rtl_ports = "{o_pmu_path_select_tdr_sibSelect[1]}";
        Attribute tessent_design_gate_ports = "{o_pmu_path_select_tdr_sibSelect[1]}";
        Source pmm_tai_sib_inst.o_pmu_path_select_tdr_sibSelect_ijtag_do_3;
    }
    DataOutPort o_pmu_path_select_tdr_sib_ijtag_do_6[1:1] {
        Attribute tessent_design_rtl_ports = "{o_pmu_path_select_tdr_sib[1]}";
        Attribute tessent_design_gate_ports = "{o_pmu_path_select_tdr_sib[1]}";
        Source pmm_tai_sib_inst.o_pmu_path_select_tdr_sib_ijtag_do_4;
    }
    DataOutPort o_pmu_delay_select_tdr_sibSelect_ijtag_do_7[1:1] {
        Attribute tessent_design_rtl_ports = "{o_pmu_delay_select_tdr_sibSelect[1]}";
        Attribute tessent_design_gate_ports = "{o_pmu_delay_select_tdr_sibSelect[1]}";
        Source pmm_tai_sib_inst.o_pmu_delay_select_tdr_sibSelect_ijtag_do_5;
    }
    DataOutPort o_pmu_delay_select_tdr_sib_ijtag_do_8[1:1] {
        Attribute tessent_design_rtl_ports = "{o_pmu_delay_select_tdr_sib[1]}";
        Attribute tessent_design_gate_ports = "{o_pmu_delay_select_tdr_sib[1]}";
        Source pmm_tai_sib_inst.o_pmu_delay_select_tdr_sib_ijtag_do_6;
    }
    DataOutPort o_pmu_collect_status_tdr_sibSelect_ijtag_do_9[1:1] {
        Attribute tessent_design_rtl_ports = "{o_pmu_collect_status_tdr_sibSelect[1]}";
        Attribute tessent_design_gate_ports = "{o_pmu_collect_status_tdr_sibSelect[1]}";
        Source pmm_tai_sib_inst.o_pmu_collect_status_tdr_sibSelect_ijtag_do_7;
    }
    DataOutPort o_sib2_bypass_tdr_sibSelect_ijtag_do_10[1:1] {
        Attribute tessent_design_rtl_ports = "{o_sib2_bypass_tdr_sibSelect[1]}";
        Attribute tessent_design_gate_ports = "{o_sib2_bypass_tdr_sibSelect[1]}";
        Source pmm_tai_sib_inst.o_sib2_bypass_tdr_sibSelect_ijtag_do_9;
    }
    DataOutPort o_pmu_collect_status_tdr_sib_ijtag_do_11[1:1] {
        Attribute tessent_design_rtl_ports = "{o_pmu_collect_status_tdr_sib[1]}";
        Attribute tessent_design_gate_ports = "{o_pmu_collect_status_tdr_sib[1]}";
        Source pmm_tai_sib_inst.o_pmu_collect_status_tdr_sib_ijtag_do_8;
    }
    DataOutPort o_pmu_func_clk_gate_en_tdr_sibSelect_ijtag_do_12 {
        Attribute tessent_design_rtl_ports = "o_pmu_func_clk_gate_en_tdr_sibSelect";
        Attribute tessent_design_gate_ports = "o_pmu_func_clk_gate_en_tdr_sibSelect";
        Source pmm_tai_sib_inst.o_pmu_func_clk_gate_en_tdr_sibSelect_ijtag_do_2;
    }
    ScanInPort i_pmu_data_out_ijtag_from_so_1[1:1] {
        Attribute tessent_design_rtl_ports = "{i_pmu_data_out[1]}";
        Attribute tessent_design_gate_ports = "{i_pmu_data_out[1]}";
    }
    ScanOutPort o_pmu_status_ijtag_to_si_3[1:1] {
        Attribute tessent_design_rtl_ports = "{o_pmu_status[1]}";
        Attribute tessent_design_gate_ports = "{o_pmu_status[1]}";
        Source IEEE_1687_dr_inst.o_pmu_status_ijtag_to_si_3;
    }
    TCKPort i_wrck_gated_mutex_ijtag_tck_2 {
        Attribute tessent_design_rtl_ports = "i_wrck_gated_mutex";
        Attribute tessent_design_gate_ports = "i_wrck_gated_mutex";
    }
    DataOutPort o_pmu_capture_ff_tdr_sibSelect_ijtag_do_13[1:1] {
        Attribute tessent_design_rtl_ports = "{o_pmu_capture_ff_tdr_sibSelect[1]}";
        Attribute tessent_design_gate_ports = "{o_pmu_capture_ff_tdr_sibSelect[1]}";
        Source pmm_tai_sib_inst.o_pmu_capture_ff_tdr_sibSelect_ijtag_do_10;
    }
    ToSelectPort o_pmu_capture_ff_shift_ijtag_to_sel_4[1:1] {
        Attribute tessent_design_rtl_ports = "{o_pmu_capture_ff_shift[1]}";
        Attribute tessent_design_gate_ports = "{o_pmu_capture_ff_shift[1]}";
        Source IEEE_1687_dr_inst.o_pmu_capture_ff_shift_ijtag_to_sel_4;
    }
    DataOutPort o_pmu_capture_ff_tdr_sib_ijtag_do_14[1:1] {
        Attribute tessent_design_rtl_ports = "{o_pmu_capture_ff_tdr_sib[1]}";
        Attribute tessent_design_gate_ports = "{o_pmu_capture_ff_tdr_sib[1]}";
        Source pmm_tai_sib_inst.o_pmu_capture_ff_tdr_sib_ijtag_do_11;
    }
    ScanOutPort o_pmu_capture_ff_tdi_ijtag_to_si_4[1:1] {
        Attribute tessent_design_rtl_ports = "{o_pmu_capture_ff_tdi[1]}";
        Attribute tessent_design_gate_ports = "{o_pmu_capture_ff_tdi[1]}";
        Source IEEE_1687_dr_inst.o_pmu_capture_ff_tdr_data_in_ijtag_to_si_4;
    }
    ScanInPort i_pmu_capture_ff_data_out_ijtag_from_so_2[1:1] {
        Attribute tessent_design_rtl_ports = "{i_pmu_capture_ff_data_out[1]}";
        Attribute tessent_design_gate_ports = "{i_pmu_capture_ff_data_out[1]}";
    }
    Instance pmm_tai_sib_inst Of PATH_MONITOR_CONTROLLER_pmm_tai_sib_block {
        InputPort i_scanin_ijtag_si = i_scanin_ijtag_si;
        InputPort i_resetn_ijtag_reset = i_resetn_ijtag_reset;
        InputPort i_tck_ijtag_tck = i_tck_ijtag_tck_1;
        InputPort i_captureEn_ijtag_ce = i_captureEn_ijtag_ce;
        InputPort i_shiftEn_ijtag_se = i_shiftEn_ijtag_se;
        InputPort i_updateEn_ijtag_ue = i_updateEn_ijtag_ue;
        InputPort i_select_ijtag_sel = i_select_ijtag_sel;
        InputPort i_testmode_tdr_scanout_ijtag_from_so_1 = IEEE_1687_dr_inst.o_tm_tdr_scanout_ijtag_so_1;
        InputPort i_pmu_func_clk_gate_en_tdr_scanout_ijtag_from_so_2 = IEEE_1687_dr_inst.o_pmu_func_clk_gate_en_tdr_scanout_ijtag_so_2;
        InputPort i_pmu_path_select_tdr_scanout_ijtag_from_so_3 = IEEE_1687_dr_inst.o_pmu_path_select_tdr_scanout_ijtag_so_5;
        InputPort i_pmu_delay_select_tdr_scanout_ijtag_from_so_4 = IEEE_1687_dr_inst.o_pmu_delay_select_tdr_scanout_ijtag_so_6;
        InputPort i_pmu_collect_status_tdr_scanout_ijtag_from_so_5 = IEEE_1687_dr_inst.o_pmu_collect_status_tdr_scanout_ijtag_so_7;
        InputPort i_sib2_bypass_tdr_scanout_ijtag_from_so_6 = IEEE_1687_dr_inst.o_sib2_bypass_tdr_scanout_ijtag_so_4;
        InputPort i_pmu_capture_ff_tdr_scanout_ijtag_from_so_7 = IEEE_1687_dr_inst.o_pmu_capture_tdr_scanout_ijtag_so_8;
        InputPort i_sib_loop_bypass_tdr_scanout_ijtag_from_so_8 = IEEE_1687_dr_inst.o_sib_loop_bypass_tdr_scanout_ijtag_so_3;
    }
    Instance IEEE_1687_dr_inst Of PATH_MONITOR_CONTROLLER_IEEE_1687_dr {
        InputPort i_tck_ijtag_tck = i_wrck_gated_mutex_ijtag_tck_2;
        InputPort i_Resetn_ijtag_reset = i_resetn_ijtag_reset;
        InputPort i_tm_sibShiftEn_ijtag_se_1 = pmm_tai_sib_inst.o_testmode_sibShiftEn_ijtag_to_se_1;
        InputPort i_tm_sibCaptureEn_ijtag_ce_1 = pmm_tai_sib_inst.o_testmode_sibCaptureEn_ijtag_to_ce_1;
        InputPort i_tm_sibUpdateEn_ijtag_ue_1 = pmm_tai_sib_inst.o_testmode_sibUpdateEn_ijtag_to_ue_1;
        InputPort i_tm_tdr_scanin_ijtag_si_1 = pmm_tai_sib_inst.o_testmode_tdr_scanin_ijtag_to_si_1;
        InputPort i_pmu_func_clk_gate_en_tdr_sibShiftEn_ijtag_se_2 = pmm_tai_sib_inst.o_pmu_func_clk_gate_en_tdr_sibShiftEn_ijtag_to_se_2;
        InputPort i_pmu_func_clk_gate_en_tdr_sibCaptureEn_ijtag_ce_2 = pmm_tai_sib_inst.o_pmu_func_clk_gate_en_tdr_sibCaptureEn_ijtag_to_ce_2;
        InputPort i_pmu_func_clk_gate_en_tdr_sibUpdateEn_ijtag_ue_2 = pmm_tai_sib_inst.o_pmu_func_clk_gate_en_tdr_sibUpdateEn_ijtag_to_ue_2;
        InputPort i_pmu_func_clk_gate_en_tdr_scanin_ijtag_si_2 = pmm_tai_sib_inst.o_pmu_func_clk_gate_en_tdr_scanin_ijtag_to_si_2;
        InputPort i_sib_loop_bypass_tdr_sibShiftEn_ijtag_se_3 = pmm_tai_sib_inst.o_sib_loop_bypass_tdr_sibShiftEn_ijtag_to_se_8;
        InputPort i_sib_loop_bypass_tdr_sibCaptureEn_ijtag_ce_3 = pmm_tai_sib_inst.o_sib_loop_bypass_tdr_sibCaptureEn_ijtag_to_ce_8;
        InputPort i_sib_loop_bypass_tdr_sibUpdateEn_ijtag_ue_3 = pmm_tai_sib_inst.o_sib_loop_bypass_tdr_sibUpdateEn_ijtag_to_ue_8;
        InputPort i_sib_loop_bypass_tdr_scanin_ijtag_si_3 = pmm_tai_sib_inst.o_sib_loop_bypass_tdr_scanin_ijtag_to_si_8;
        InputPort i_sib2_bypass_tdr_sibShiftEn_ijtag_se_4 = pmm_tai_sib_inst.o_sib2_bypass_tdr_sibShiftEn_ijtag_to_se_6;
        InputPort i_sib2_bypass_tdr_sibCaptureEn_ijtag_ce_4 = pmm_tai_sib_inst.o_sib2_bypass_tdr_sibCaptureEn_ijtag_to_ce_6;
        InputPort i_sib2_bypass_tdr_sibUpdateEn_ijtag_ue_4 = pmm_tai_sib_inst.o_sib2_bypass_tdr_sibUpdateEn_ijtag_to_ue_6;
        InputPort i_sib2_bypass_tdr_scanin_ijtag_si_4 = pmm_tai_sib_inst.o_sib2_bypass_tdr_scanin_ijtag_to_si_6;
        InputPort i_pmu_path_select_tdr_sibShiftEn_ijtag_se_5 = pmm_tai_sib_inst.o_pmu_path_select_tdr_sibShiftEn_ijtag_to_se_3;
        InputPort i_pmu_path_select_tdr_sib_ijtag_di_1 = pmm_tai_sib_inst.o_pmu_path_select_tdr_sibSelect_ijtag_do_3;
        InputPort i_pmu_path_select_tdr_sibCaptureEn_ijtag_ce_5 = pmm_tai_sib_inst.o_pmu_path_select_tdr_sibCaptureEn_ijtag_to_ce_3;
        InputPort i_pmu_path_select_tdr_sibUpdateEn_ijtag_ue_5 = pmm_tai_sib_inst.o_pmu_path_select_tdr_sibUpdateEn_ijtag_to_ue_3;
        InputPort i_pmu_path_select_tdr_scanin_ijtag_si_5 = pmm_tai_sib_inst.o_pmu_path_select_tdr_scanin_ijtag_to_si_3;
        InputPort i_pmu_data_out_ijtag_from_so_1 = i_pmu_data_out_ijtag_from_so_1;
        InputPort i_pmu_delay_select_tdr_sibShiftEn_ijtag_se_6 = pmm_tai_sib_inst.o_pmu_delay_select_tdr_sibShiftEn_ijtag_to_se_4;
        InputPort i_pmu_delay_select_tdr_sib_ijtag_di_2 = pmm_tai_sib_inst.o_pmu_delay_select_tdr_sibSelect_ijtag_do_5;
        InputPort i_pmu_delay_select_tdr_sibCaptureEn_ijtag_ce_6 = pmm_tai_sib_inst.o_pmu_delay_select_tdr_sibCaptureEn_ijtag_to_ce_4;
        InputPort i_pmu_delay_select_tdr_sibUpdateEn_ijtag_ue_6 = pmm_tai_sib_inst.o_pmu_delay_select_tdr_sibUpdateEn_ijtag_to_ue_4;
        InputPort i_pmu_delay_select_tdr_scanin_ijtag_si_6 = pmm_tai_sib_inst.o_pmu_delay_select_tdr_scanin_ijtag_to_si_4;
        InputPort i_pmu_collect_status_tdr_sibShiftEn_ijtag_se_7 = pmm_tai_sib_inst.o_pmu_collect_status_tdr_sibShiftEn_ijtag_to_se_5;
        InputPort i_pmu_collect_status_tdr_sib_ijtag_di_3 = pmm_tai_sib_inst.o_pmu_collect_status_tdr_sibSelect_ijtag_do_7;
        InputPort i_pmu_collect_status_tdr_sibCaptureEn_ijtag_ce_7 = pmm_tai_sib_inst.o_pmu_collect_status_tdr_sibCaptureEn_ijtag_to_ce_5;
        InputPort i_pmu_collect_status_tdr_sibUpdateEn_ijtag_ue_7 = pmm_tai_sib_inst.o_pmu_collect_status_tdr_sibUpdateEn_ijtag_to_ue_5;
        InputPort i_pmu_collect_status_tdr_scanin_ijtag_si_7 = pmm_tai_sib_inst.o_pmu_collect_status_tdr_scanin_ijtag_to_si_5;
        InputPort i_pmu_capture_ff_tdr_sibShiftEn_ijtag_se_8 = pmm_tai_sib_inst.o_pmu_capture_ff_tdr_sibShiftEn_ijtag_to_se_7;
        InputPort i_pmu_capture_ff_tdr_sib_ijtag_di_4 = pmm_tai_sib_inst.o_pmu_capture_ff_tdr_sibSelect_ijtag_do_10;
        InputPort i_pmu_capture_ff_tdr_sibCaptureEn_ijtag_ce_8 = pmm_tai_sib_inst.o_pmu_capture_ff_tdr_sibCaptureEn_ijtag_to_ce_7;
        InputPort i_pmu_capture_ff_tdr_sibUpdateEn_ijtag_ue_8 = pmm_tai_sib_inst.o_pmu_capture_ff_tdr_sibUpdateEn_ijtag_to_ue_7;
        InputPort i_pmu_capture_ff_tdr_scanin_ijtag_si_8 = pmm_tai_sib_inst.o_pmu_capture_ff_tdr_scanin_ijtag_to_si_7;
        InputPort i_pmu_capture_ff_data_out_ijtag_from_so_2 = i_pmu_capture_ff_data_out_ijtag_from_so_2;
    }
    ScanInterface client {
        Port i_scanin_ijtag_si;
        Port o_scanout_ijtag_so;
        Port i_select_ijtag_sel;
        Port i_shiftEn_ijtag_se;
        Port i_captureEn_ijtag_ce;
        Port i_updateEn_ijtag_ue;
        Port i_resetn_ijtag_reset;
        Port i_tck_ijtag_tck_1;
    }
    ScanInterface host_o_configure_path_sel_pmu1 {
        Port i_pmu_data_out_ijtag_from_so_1[1];
        Port o_configure_path_sel_pmu_ijtag_to_sel_3[1];
    }
    ScanInterface host_o_configure_delay_sel_pmu1 {
        Port i_pmu_data_out_ijtag_from_so_1[1];
        Port o_configure_delay_sel_pmu_ijtag_to_sel_2[1];
    }
    ScanInterface host_o_pmu_collect_status1 {
        Port i_pmu_data_out_ijtag_from_so_1[1];
        Port o_pmu_collect_status_ijtag_to_sel_1[1];
    }
    ScanInterface host_o_pmu_capture_ff_shift1 {
        Port i_pmu_capture_ff_data_out_ijtag_from_so_2[1];
        Port o_pmu_capture_ff_shift_ijtag_to_sel_4[1];
    }
}

// Source: all_out/1_pmm_ip_generation_with_1687_and_apb_sim/tessent_analysis_rtl_icl/icl/LOOP_BYPASS_TDR.icl
// Generated; edit the PMM sources, not this file.
Module LOOP_BYPASS_TDR {
    ShiftEnPort Shift_ijtag_se;
    CaptureEnPort Capture_ijtag_ce;
    UpdateEnPort Update_ijtag_ue;
    SelectPort Select_ijtag_sel;
    ResetPort TRSTN_ijtag_reset {
        ActivePolarity 0;
    }
    TCKPort TCK_ijtag_tck;
    ScanInPort SI_ijtag_si;
    ScanOutPort SO_ijtag_so {
        Source Value;
    }
    ScanInterface client {
        Port SI_ijtag_si;
        Port SO_ijtag_so;
        Port Shift_ijtag_se;
        Port Capture_ijtag_ce;
        Port Update_ijtag_ue;
        Port Select_ijtag_sel;
        Port TRSTN_ijtag_reset;
        Port TCK_ijtag_tck;
    }
    ScanRegister Value {
        ScanInSource SI_ijtag_si;
        CaptureSource Value;
        ResetValue 1'b1;
        Attribute synopsys_access = "RW";
    }
}

// Source: all_out/1_pmm_ip_generation_with_1687_and_apb_sim/tessent_analysis_rtl_icl/icl/DELAY_SEL_LOOP.icl
// Generated; edit the PMM sources, not this file.
Module DELAY_SEL_LOOP {
    ShiftEnPort Shift_ijtag_se;
    CaptureEnPort Capture_ijtag_ce;
    UpdateEnPort Update_ijtag_ue;
    SelectPort Select_ijtag_sel;
    ResetPort TRSTN_ijtag_reset {
        ActivePolarity 0;
    }
    TCKPort TCK_ijtag_tck;
    ScanInPort SI_ijtag_si;
    ScanOutPort SO_ijtag_so {
        Source Value[0];
    }
    ScanInterface client {
        Port SI_ijtag_si;
        Port SO_ijtag_so;
        Port Shift_ijtag_se;
        Port Capture_ijtag_ce;
        Port Update_ijtag_ue;
        Port Select_ijtag_sel;
        Port TRSTN_ijtag_reset;
        Port TCK_ijtag_tck;
    }
    ScanRegister Value[4:0] {
        ScanInSource SI_ijtag_si;
        ResetValue 5'b0;
        Attribute synopsys_access = "RW";
    }
    DataOutPort delay_sel_ijtag_do[4:0] {
        Source Value;
    }
}

// Source: all_out/1_pmm_ip_generation_with_1687_and_apb_sim/tessent_analysis_rtl_icl/icl/PATH_MONITOR_UNIT_CFG1_TOGGLE_DETECTOR.icl
// Generated; edit the PMM sources, not this file.
Module PATH_MONITOR_UNIT_CFG1_TOGGLE_DETECTOR {
    ScanInPort status_out_ijtag_si {
        Attribute tessent_design_rtl_ports = "status_out";
        Attribute tessent_design_gate_ports = "status_out";
    }
    SelectPort collect_ijtag_sel {
        Attribute tessent_design_rtl_ports = "collect";
        Attribute tessent_design_gate_ports = "collect";
    }
    ScanOutPort toggle_out_ijtag_so {
        Attribute tessent_design_rtl_ports = "toggle_out";
        Attribute tessent_design_gate_ports = "toggle_out";
        Source in_toggle_out;
    }
    ScanInterface client_in_toggle_out {
        Port status_out_ijtag_si;
        Port toggle_out_ijtag_so;
        Port collect_ijtag_sel;
    }
    ScanRegister in_toggle_out {
        ScanInSource status_out_ijtag_si;
        ResetValue 1'b0;
        Attribute synopsys_access = "RO";
    }
}

// Source: all_out/1_pmm_ip_generation_with_1687_and_apb_sim/tessent_analysis_rtl_icl/icl/PATH_MONITOR_UNIT_CFG1_DELAY_SEL_CAPTURE_FF.icl
// Generated; edit the PMM sources, not this file.
Module PATH_MONITOR_UNIT_CFG1_DELAY_SEL_CAPTURE_FF {
    ScanInPort i_pmu_data_in_ijtag_si_1 {
        Attribute tessent_design_rtl_ports = "i_pmu_data_in";
        Attribute tessent_design_gate_ports = "i_pmu_data_in";
    }
    SelectPort i_configure_delay_sel_ijtag_sel_1 {
        Attribute tessent_design_rtl_ports = "i_configure_delay_sel";
        Attribute tessent_design_gate_ports = "i_configure_delay_sel";
    }
    ScanOutPort delay_select_ijtag_so_1 {
        Attribute tessent_design_rtl_ports = "delay_select";
        Attribute tessent_design_gate_ports = "delay_select";
        Source o_delay_select[0];
    }
    ScanInterface client_o_delay_select {
        Port i_pmu_data_in_ijtag_si_1;
        Port delay_select_ijtag_so_1;
        Port i_configure_delay_sel_ijtag_sel_1;
    }
    ScanRegister o_delay_select[4:0] {
        ScanInSource i_pmu_data_in_ijtag_si_1;
        ResetValue 5'b0;
        Attribute synopsys_access = "RW";
    }
    ScanInPort i_capture_data_ijtag_si_2 {
        Attribute tessent_design_rtl_ports = "i_capture_data";
        Attribute tessent_design_gate_ports = "i_capture_data";
    }
    SelectPort i_capture_shift_ijtag_sel_2 {
        Attribute tessent_design_rtl_ports = "i_capture_shift";
        Attribute tessent_design_gate_ports = "i_capture_shift";
    }
    ScanOutPort o_capture_tdo_ijtag_so_2 {
        Attribute tessent_design_rtl_ports = "o_capture_tdo";
        Attribute tessent_design_gate_ports = "o_capture_tdo";
        Source capture_data_reg;
    }
    ScanInterface client_capture_data_reg {
        Port i_capture_data_ijtag_si_2;
        Port o_capture_tdo_ijtag_so_2;
        Port i_capture_shift_ijtag_sel_2;
    }
    ScanRegister capture_data_reg {
        ScanInSource i_capture_data_ijtag_si_2;
        ResetValue 1'b0;
        Attribute synopsys_access = "RO";
    }
    DataOutPort delay_select_mux_ijtag_do_1[4:0] {
        Attribute tessent_design_rtl_ports = "{delay_select_mux[4]} {delay_select_mux[3]} {delay_select_mux[2]} {delay_select_mux[1]} {delay_select_mux[0]}";
        Attribute tessent_design_gate_ports = "{delay_select_mux[4]} {delay_select_mux[3]} {delay_select_mux[2]} {delay_select_mux[1]} {delay_select_mux[0]}";
        Source o_delay_select;
    }
    DataOutPort o_capture_data_ijtag_do_2 {
        Attribute tessent_design_rtl_ports = "o_capture_data";
        Attribute tessent_design_gate_ports = "o_capture_data";
        Source capture_data_reg;
    }
}

// Source: all_out/1_pmm_ip_generation_with_1687_and_apb_sim/tessent_analysis_rtl_icl/icl/PATH_MONITOR_CONTROLLER_PMM_1687_SIB_REG_4.icl
// Generated; edit the PMM sources, not this file.
Module PATH_MONITOR_CONTROLLER_PMM_1687_SIB_REG_4 {
    TCKPort clk_ijtag_tck {
        Attribute tessent_design_rtl_ports = "clk";
        Attribute tessent_design_gate_ports = "clk";
    }
    ResetPort TRSTN_ijtag_reset {
        Attribute tessent_design_rtl_ports = "TRSTN";
        Attribute tessent_design_gate_ports = "TRSTN";
        ActivePolarity 0;
    }
    SelectPort Select_ijtag_sel {
        Attribute tessent_design_rtl_ports = "Select";
        Attribute tessent_design_gate_ports = "Select";
    }
    ShiftEnPort ShiftEn_ijtag_se {
        Attribute tessent_design_rtl_ports = "ShiftEn";
        Attribute tessent_design_gate_ports = "ShiftEn";
    }
    CaptureEnPort CaptureEn_ijtag_ce {
        Attribute tessent_design_rtl_ports = "CaptureEn";
        Attribute tessent_design_gate_ports = "CaptureEn";
    }
    UpdateEnPort UpdateEn_ijtag_ue {
        Attribute tessent_design_rtl_ports = "UpdateEn";
        Attribute tessent_design_gate_ports = "UpdateEn";
    }
    ScanInPort TDI_ijtag_si {
        Attribute tessent_design_rtl_ports = "TDI";
        Attribute tessent_design_gate_ports = "TDI";
    }
    ScanOutPort TDO_ijtag_so {
        Attribute tessent_design_rtl_ports = "TDO";
        Attribute tessent_design_gate_ports = "TDO";
        Source TDR_shift[0];
    }
    ScanRegister TDR_shift[3:0] {
        ScanInSource TDI_ijtag_si;
        CaptureSource TDR_shift;
        ResetValue 4'b0;
    }
    DataOutPort TDR_ijtag_do_1[3:0] {
        Attribute tessent_design_rtl_ports = "{TDR[3]} {TDR[2]} {TDR[1]} {TDR[0]}";
        Attribute tessent_design_gate_ports = "{TDR[3]} {TDR[2]} {TDR[1]} {TDR[0]}";
        Source TDR_shift;
    }
    DataOutPort sibSelect_reg_ijtag_do_2[3:0] {
        Attribute tessent_design_rtl_ports = "{sibSelect_reg[3]} {sibSelect_reg[2]} {sibSelect_reg[1]} {sibSelect_reg[0]}";
        Attribute tessent_design_gate_ports = "{sibSelect_reg[3]} {sibSelect_reg[2]} {sibSelect_reg[1]} {sibSelect_reg[0]}";
        Source TDR_shift;
    }
}

// Source: all_out/1_pmm_ip_generation_with_1687_and_apb_sim/tessent_analysis_rtl_icl/icl/PM_MODE_TDR.icl
// Generated; edit the PMM sources, not this file.
Module PM_MODE_TDR {
    ShiftEnPort Shift_ijtag_se;
    CaptureEnPort Capture_ijtag_ce;
    UpdateEnPort Update_ijtag_ue;
    SelectPort Select_ijtag_sel;
    ResetPort TRSTN_ijtag_reset {
        ActivePolarity 0;
    }
    TCKPort TCK_ijtag_tck;
    ScanInPort SI_ijtag_si;
    ScanOutPort SO_ijtag_so {
        Source Mode[0];
    }
    ScanInterface client {
        Port SI_ijtag_si;
        Port SO_ijtag_so;
        Port Shift_ijtag_se;
        Port Capture_ijtag_ce;
        Port Update_ijtag_ue;
        Port Select_ijtag_sel;
        Port TRSTN_ijtag_reset;
        Port TCK_ijtag_tck;
    }
    ScanRegister ClockDividerTestEnable {
        ScanInSource SI_ijtag_si;
        CaptureSource ClockDividerTestEnable;
        ResetValue 1'b0;
        Attribute synopsys_access = "RW";
    }
    ScanRegister TestEnable {
        ScanInSource ClockDividerTestEnable;
        CaptureSource TestEnable;
        ResetValue 1'b0;
        Attribute synopsys_access = "RW";
    }
    ScanRegister DebugEnable {
        ScanInSource TestEnable;
        CaptureSource DebugEnable;
        ResetValue 1'b0;
        Attribute synopsys_access = "RW";
    }
    ScanRegister TAISelect {
        ScanInSource DebugEnable;
        CaptureSource TAISelect;
        ResetValue 1'b0;
        Attribute synopsys_access = "RW";
    }
    ScanRegister Mode[1:0] {
        ScanInSource TAISelect;
        CaptureSource Mode;
        ResetValue 2'b0;
        Attribute synopsys_access = "RW";
    }
    DataOutPort clock_divider_test_enable_ijtag_do_1 {
        Source ClockDividerTestEnable;
    }
    DataOutPort test_enable_ijtag_do_2 {
        Source TestEnable;
    }
    DataOutPort debug_enable_ijtag_do_3 {
        Source DebugEnable;
    }
    DataOutPort tai_select_ijtag_do_4 {
        Source TAISelect;
    }
    DataOutPort pmu_mode_ijtag_do_5[1:0] {
        Source Mode;
    }
}

// Source: all_out/1_pmm_ip_generation_with_1687_and_apb_sim/tessent_analysis_rtl_icl/icl/PATH_MONITOR_CONTROLLER_PMM_1687_SIB_REG_3.icl
// Generated; edit the PMM sources, not this file.
Module PATH_MONITOR_CONTROLLER_PMM_1687_SIB_REG_3 {
    TCKPort clk_ijtag_tck {
        Attribute tessent_design_rtl_ports = "clk";
        Attribute tessent_design_gate_ports = "clk";
    }
    ResetPort TRSTN_ijtag_reset {
        Attribute tessent_design_rtl_ports = "TRSTN";
        Attribute tessent_design_gate_ports = "TRSTN";
        ActivePolarity 0;
    }
    SelectPort Select_ijtag_sel {
        Attribute tessent_design_rtl_ports = "Select";
        Attribute tessent_design_gate_ports = "Select";
    }
    ShiftEnPort ShiftEn_ijtag_se {
        Attribute tessent_design_rtl_ports = "ShiftEn";
        Attribute tessent_design_gate_ports = "ShiftEn";
    }
    CaptureEnPort CaptureEn_ijtag_ce {
        Attribute tessent_design_rtl_ports = "CaptureEn";
        Attribute tessent_design_gate_ports = "CaptureEn";
    }
    UpdateEnPort UpdateEn_ijtag_ue {
        Attribute tessent_design_rtl_ports = "UpdateEn";
        Attribute tessent_design_gate_ports = "UpdateEn";
    }
    ScanInPort TDI_ijtag_si {
        Attribute tessent_design_rtl_ports = "TDI";
        Attribute tessent_design_gate_ports = "TDI";
    }
    ScanOutPort TDO_ijtag_so {
        Attribute tessent_design_rtl_ports = "TDO";
        Attribute tessent_design_gate_ports = "TDO";
        Source TDR_shift[0];
    }
    ScanRegister TDR_shift[2:0] {
        ScanInSource TDI_ijtag_si;
        CaptureSource TDR_shift;
        ResetValue 3'b0;
    }
    DataOutPort TDR_ijtag_do_1[2:0] {
        Attribute tessent_design_rtl_ports = "{TDR[2]} {TDR[1]} {TDR[0]}";
        Attribute tessent_design_gate_ports = "{TDR[2]} {TDR[1]} {TDR[0]}";
        Source TDR_shift;
    }
    DataOutPort sibSelect_reg_ijtag_do_2[2:0] {
        Attribute tessent_design_rtl_ports = "{sibSelect_reg[2]} {sibSelect_reg[1]} {sibSelect_reg[0]}";
        Attribute tessent_design_gate_ports = "{sibSelect_reg[2]} {sibSelect_reg[1]} {sibSelect_reg[0]}";
        Source TDR_shift;
    }
}

// Source: all_out/1_pmm_ip_generation_with_1687_and_apb_sim/tessent_analysis_rtl_icl/icl/SIB1.icl
// Generated; edit the PMM sources, not this file.
Module SIB1 {
    ShiftEnPort Shift_ijtag_se;
    CaptureEnPort Capture_ijtag_ce;
    UpdateEnPort Update_ijtag_ue;
    SelectPort Select_ijtag_sel;
    ResetPort TRSTN_ijtag_reset {
        ActivePolarity 0;
    }
    TCKPort TCK_ijtag_tck;
    ScanInPort SI_ijtag_si;
    ScanOutPort SO_ijtag_so {
        Source Value[0];
    }
    ScanInterface client {
        Port SI_ijtag_si;
        Port SO_ijtag_so;
        Port Shift_ijtag_se;
        Port Capture_ijtag_ce;
        Port Update_ijtag_ue;
        Port Select_ijtag_sel;
        Port TRSTN_ijtag_reset;
        Port TCK_ijtag_tck;
    }
    ScanInPort B0_ijtag_from_so_1;
    LogicSignal Enable0 {
        Select_ijtag_sel && (Value == 3'd0);
    }
    ToSelectPort Select0_ijtag_to_sel_1 {
        Source Enable0;
    }
    ScanInPort B1_ijtag_from_so_2;
    LogicSignal Enable1 {
        Select_ijtag_sel && (Value == 3'd1);
    }
    ToSelectPort Select1_ijtag_to_sel_2 {
        Source Enable1;
    }
    ScanInPort B2_ijtag_from_so_3;
    LogicSignal Enable2 {
        Select_ijtag_sel && (Value == 3'd2);
    }
    ToSelectPort Select2_ijtag_to_sel_3 {
        Source Enable2;
    }
    ScanRegister Value[2:0] {
        ScanInSource mux;
        CaptureSource Value;
        ResetValue 3'b0;
    }
    DataOutPort Selection_ijtag_do[2:0] {
        Source Value;
    }
    ScanMux mux SelectedBy Value {
        3'd0 : B0_ijtag_from_so_1;
        3'd1 : B1_ijtag_from_so_2;
        3'd2 : B2_ijtag_from_so_3;
        3'd3 : SI_ijtag_si;
        3'd4 : SI_ijtag_si;
        3'd5 : SI_ijtag_si;
        3'd6 : SI_ijtag_si;
        3'd7 : SI_ijtag_si;
    }
}

// Source: all_out/1_pmm_ip_generation_with_1687_and_apb_sim/tessent_analysis_rtl_icl/icl/PATH_MONITOR_UNIT_CFG1_PMU_STATUS_DETECTOR.icl
// Generated; edit the PMM sources, not this file.
Module PATH_MONITOR_UNIT_CFG1_PMU_STATUS_DETECTOR {
    ScanInPort i_pmu_data_ijtag_si {
        Attribute tessent_design_rtl_ports = "i_pmu_data";
        Attribute tessent_design_gate_ports = "i_pmu_data";
    }
    SelectPort collect_ijtag_sel {
        Attribute tessent_design_rtl_ports = "collect";
        Attribute tessent_design_gate_ports = "collect";
    }
    ScanOutPort o_pmu_status_ijtag_so {
        Attribute tessent_design_rtl_ports = "o_pmu_status";
        Attribute tessent_design_gate_ports = "o_pmu_status";
        Source pmu_status_reg;
    }
    ScanInterface client_pmu_status_reg {
        Port i_pmu_data_ijtag_si;
        Port o_pmu_status_ijtag_so;
        Port collect_ijtag_sel;
    }
    ScanRegister pmu_status_reg {
        ScanInSource i_pmu_data_ijtag_si;
        ResetValue 1'b0;
        Attribute synopsys_access = "RO";
    }
}

// Source: all_out/1_pmm_ip_generation_with_1687_and_apb_sim/tessent_analysis_rtl_icl/icl/PMU.icl
// Generated; edit the PMM sources, not this file.
Module PMU {
    ScanInPort i_pmu_data_in_ijtag_si_1 {
        Attribute tessent_design_rtl_ports = "i_pmu_data_in";
        Attribute tessent_design_gate_ports = "i_pmu_data_in";
    }
    ScanInPort I_capture_tdi_ijtag_si_2 {
        Attribute tessent_design_rtl_ports = "I_capture_tdi";
        Attribute tessent_design_gate_ports = "I_capture_tdi";
    }
    SelectPort i_pmu_configure_path_sel_ijtag_sel_1 {
        Attribute tessent_design_rtl_ports = "i_pmu_configure_path_sel";
        Attribute tessent_design_gate_ports = "i_pmu_configure_path_sel";
    }
    SelectPort i_pmu_configure_delay_sel_ijtag_sel_2 {
        Attribute tessent_design_rtl_ports = "i_pmu_configure_delay_sel";
        Attribute tessent_design_gate_ports = "i_pmu_configure_delay_sel";
    }
    SelectPort i_pmu_status_collect_ijtag_sel_3 {
        Attribute tessent_design_rtl_ports = "i_pmu_status_collect";
        Attribute tessent_design_gate_ports = "i_pmu_status_collect";
    }
    SelectPort I_capture_shift_ijtag_sel_4 {
        Attribute tessent_design_rtl_ports = "I_capture_shift";
        Attribute tessent_design_gate_ports = "I_capture_shift";
    }
    ScanOutPort o_pmu_data_out_ijtag_so_1 {
        Attribute tessent_design_rtl_ports = "o_pmu_data_out";
        Attribute tessent_design_gate_ports = "o_pmu_data_out";
        Source data_return;
    }
    ScanOutPort I_capture_tdo_ijtag_so_2 {
        Attribute tessent_design_rtl_ports = "I_capture_tdo";
        Attribute tessent_design_gate_ports = "I_capture_tdo";
        Source monitor_shadow_inst.I_capture_tdo_ijtag_so;
    }
    LogicSignal configure {
        i_pmu_configure_path_sel_ijtag_sel_1 || i_pmu_configure_delay_sel_ijtag_sel_2;
    }
    ScanMux configure_return SelectedBy i_pmu_configure_delay_sel_ijtag_sel_2 {
        1'b0 : path_sel_reg_inst.o_path_sel_so_ijtag_so;
        1'b1 : delay_selector_capture_ff.delay_select_ijtag_so_1;
    }
    ScanMux data_return SelectedBy configure {
        1'b0 : toggle_detector_inst.toggle_out_ijtag_so;
        1'b1 : configure_return;
    }
    ScanInterface PATH {
        Port i_pmu_data_in_ijtag_si_1;
        Port o_pmu_data_out_ijtag_so_1;
        Port i_pmu_configure_path_sel_ijtag_sel_1;
    }
    ScanInterface DELAY {
        Port i_pmu_data_in_ijtag_si_1;
        Port o_pmu_data_out_ijtag_so_1;
        Port i_pmu_configure_delay_sel_ijtag_sel_2;
    }
    ScanInterface COLLECT {
        Port i_pmu_data_in_ijtag_si_1;
        Port o_pmu_data_out_ijtag_so_1;
        Port i_pmu_status_collect_ijtag_sel_3;
    }
    ScanInterface CAPTURE {
        Port I_capture_tdi_ijtag_si_2;
        Port I_capture_tdo_ijtag_so_2;
        Port I_capture_shift_ijtag_sel_4;
    }
    Instance path_sel_reg_inst Of PATH_MONITOR_UNIT_CFG1_PATH_SELECT_REG_MODULE {
        InputPort i_pmu_data_in_ijtag_si = i_pmu_data_in_ijtag_si_1;
        InputPort i_configure_path_sel_ijtag_sel = i_pmu_configure_path_sel_ijtag_sel_1;
    }
    Instance pmu_status_detector_inst Of PATH_MONITOR_UNIT_CFG1_PMU_STATUS_DETECTOR {
        InputPort i_pmu_data_ijtag_si = i_pmu_data_in_ijtag_si_1;
        InputPort collect_ijtag_sel = i_pmu_status_collect_ijtag_sel_3;
    }
    Instance toggle_detector_inst Of PATH_MONITOR_UNIT_CFG1_TOGGLE_DETECTOR {
        InputPort status_out_ijtag_si = pmu_status_detector_inst.o_pmu_status_ijtag_so;
        InputPort collect_ijtag_sel = i_pmu_status_collect_ijtag_sel_3;
    }
    Instance monitor_shadow_inst Of PATH_MONITOR_UNIT_CFG1_MONITOR_SHADOW {
        InputPort capture_tdo_ijtag_si = delay_selector_capture_ff.o_capture_tdo_ijtag_so_2;
        InputPort capture_shift_ijtag_sel = I_capture_shift_ijtag_sel_4;
    }
    Instance delay_selector_capture_ff Of PATH_MONITOR_UNIT_CFG1_DELAY_SEL_CAPTURE_FF {
        InputPort i_pmu_data_in_ijtag_si_1 = i_pmu_data_in_ijtag_si_1;
        InputPort i_configure_delay_sel_ijtag_sel_1 = i_pmu_configure_delay_sel_ijtag_sel_2;
        InputPort i_capture_data_ijtag_si_2 = I_capture_tdi_ijtag_si_2;
        InputPort i_capture_shift_ijtag_sel_2 = I_capture_shift_ijtag_sel_4;
    }
}

// Source: all_out/1_pmm_ip_generation_with_1687_and_apb_sim/tessent_analysis_rtl_icl/icl/PATH_MONITOR_CONTROLLER_TAI_OUTPUT_INTERFACE.icl
// Generated; edit the PMM sources, not this file.
Module PATH_MONITOR_CONTROLLER_TAI_OUTPUT_INTERFACE {
    // Falling-edge holding copies: Sources describe settled values, not cycle timing.
    ScanMux capture_serial_1 SelectedBy i_pmu_capture_ff_shift_ijtag_di_5[1] {
        1'b0 : 1'b0;
        1'b1 : i_pmu_capture_ff_tdi_ijtag_si_1[1];
    }
    ScanMux i_delay_select_serial_1 SelectedBy i_configure_delay_sel_ijtag_di_2[1] {
        1'b0 : 1'b0;
        1'b1 : i_delay_select_ijtag_si_2[1];
    }
    ScanMux i_path_select_serial_1 SelectedBy i_configure_path_sel_ijtag_di_4[1] {
        1'b0 : i_delay_select_serial_1;
        1'b1 : i_path_select_ijtag_si_3[1];
    }
    ScanMux pmu_status_in_serial_1 SelectedBy i_pmu_collect_status_ijtag_di_3[1] {
        1'b0 : i_path_select_serial_1;
        1'b1 : pmu_status_in_ijtag_si_4[1];
    }
    TCKPort i_clk_ijtag_tck {
        Attribute tessent_design_rtl_ports = "i_clk";
        Attribute tessent_design_gate_ports = "i_clk";
    }
    ResetPort i_rstn_ijtag_reset {
        Attribute tessent_design_rtl_ports = "i_rstn";
        Attribute tessent_design_gate_ports = "i_rstn";
        ActivePolarity 0;
    }
    DataInPort i_pmu_func_clk_gate_en_ijtag_di_1[1:1] {
        Attribute tessent_design_rtl_ports = "{i_pmu_func_clk_gate_en[1]}";
        Attribute tessent_design_gate_ports = "{i_pmu_func_clk_gate_en[1]}";
    }
    DataInPort i_configure_delay_sel_ijtag_di_2[1:1] {
        Attribute tessent_design_rtl_ports = "{i_configure_delay_sel[1]}";
        Attribute tessent_design_gate_ports = "{i_configure_delay_sel[1]}";
    }
    DataInPort i_pmu_collect_status_ijtag_di_3[1:1] {
        Attribute tessent_design_rtl_ports = "{i_pmu_collect_status[1]}";
        Attribute tessent_design_gate_ports = "{i_pmu_collect_status[1]}";
    }
    ScanInPort i_pmu_capture_ff_tdi_ijtag_si_1[1:1] {
        Attribute tessent_design_rtl_ports = "{i_pmu_capture_ff_tdi[1]}";
        Attribute tessent_design_gate_ports = "{i_pmu_capture_ff_tdi[1]}";
    }
    DataInPort i_configure_path_sel_ijtag_di_4[1:1] {
        Attribute tessent_design_rtl_ports = "{i_configure_path_sel[1]}";
        Attribute tessent_design_gate_ports = "{i_configure_path_sel[1]}";
    }
    ScanInPort i_delay_select_ijtag_si_2[1:1] {
        Attribute tessent_design_rtl_ports = "{i_delay_select[1]}";
        Attribute tessent_design_gate_ports = "{i_delay_select[1]}";
    }
    ScanInPort i_path_select_ijtag_si_3[1:1] {
        Attribute tessent_design_rtl_ports = "{i_path_select[1]}";
        Attribute tessent_design_gate_ports = "{i_path_select[1]}";
    }
    ScanInPort pmu_status_in_ijtag_si_4[1:1] {
        Attribute tessent_design_rtl_ports = "{pmu_status_in[1]}";
        Attribute tessent_design_gate_ports = "{pmu_status_in[1]}";
    }
    DataInPort i_pmu_capture_ff_shift_ijtag_di_5[1:1] {
        Attribute tessent_design_rtl_ports = "{i_pmu_capture_ff_shift[1]}";
        Attribute tessent_design_gate_ports = "{i_pmu_capture_ff_shift[1]}";
    }
    ToSelectPort o_configure_path_sel_loop_ijtag_to_sel_1[1:1] {
        Attribute tessent_design_rtl_ports = "{o_configure_path_sel_loop[1]}";
        Attribute tessent_design_gate_ports = "{o_configure_path_sel_loop[1]}";
        Source i_configure_path_sel_ijtag_di_4;
    }
    ToSelectPort o_pmu_collect_status_loop_ijtag_to_sel_2[1:1] {
        Attribute tessent_design_rtl_ports = "{o_pmu_collect_status_loop[1]}";
        Attribute tessent_design_gate_ports = "{o_pmu_collect_status_loop[1]}";
        Source i_pmu_collect_status_ijtag_di_3;
    }
    ToSelectPort o_configure_delay_sel_loop_ijtag_to_sel_3[1:1] {
        Attribute tessent_design_rtl_ports = "{o_configure_delay_sel_loop[1]}";
        Attribute tessent_design_gate_ports = "{o_configure_delay_sel_loop[1]}";
        Source i_configure_delay_sel_ijtag_di_2;
    }
    DataOutPort o_pmu_func_clk_gate_en_loop_ijtag_do[1:1] {
        Attribute tessent_design_rtl_ports = "{o_pmu_func_clk_gate_en_loop[1]}";
        Attribute tessent_design_gate_ports = "{o_pmu_func_clk_gate_en_loop[1]}";
        Source i_pmu_func_clk_gate_en_ijtag_di_1;
    }
    ScanOutPort o_pmu_data_in_ijtag_to_si_1[1:1] {
        Attribute tessent_design_rtl_ports = "{o_pmu_data_in[1]}";
        Attribute tessent_design_gate_ports = "{o_pmu_data_in[1]}";
        Source pmu_status_in_serial_1;
    }
    ToSelectPort o_pmu_capture_ff_shift_ijtag_to_sel_4[1:1] {
        Attribute tessent_design_rtl_ports = "{o_pmu_capture_ff_shift[1]}";
        Attribute tessent_design_gate_ports = "{o_pmu_capture_ff_shift[1]}";
        Source i_pmu_capture_ff_shift_ijtag_di_5;
    }
    ScanOutPort o_pmu_capture_ff_tdi_ijtag_to_si_2[1:1] {
        Attribute tessent_design_rtl_ports = "{o_pmu_capture_ff_tdi[1]}";
        Attribute tessent_design_gate_ports = "{o_pmu_capture_ff_tdi[1]}";
        Source capture_serial_1;
    }
    ScanInterface host_capture1 {
        Port o_pmu_capture_ff_tdi_ijtag_to_si_2[1];
        Port o_pmu_capture_ff_shift_ijtag_to_sel_4[1];
    }
    ScanInterface host_o_configure_path_sel_loop1 {
        Port o_pmu_data_in_ijtag_to_si_1[1];
        Port o_configure_path_sel_loop_ijtag_to_sel_1[1];
    }
    ScanInterface host_o_configure_delay_sel_loop1 {
        Port o_pmu_data_in_ijtag_to_si_1[1];
        Port o_configure_delay_sel_loop_ijtag_to_sel_3[1];
    }
    ScanInterface host_o_pmu_collect_status_loop1 {
        Port o_pmu_data_in_ijtag_to_si_1[1];
        Port o_pmu_collect_status_loop_ijtag_to_sel_2[1];
    }
}

// Source: all_out/1_pmm_ip_generation_with_1687_and_apb_sim/tessent_analysis_rtl_icl/icl/COLLECT_LOOP.icl
// Generated; edit the PMM sources, not this file.
Module COLLECT_LOOP {
    ShiftEnPort Shift_ijtag_se;
    CaptureEnPort Capture_ijtag_ce;
    UpdateEnPort Update_ijtag_ue;
    SelectPort Select_ijtag_sel;
    ResetPort TRSTN_ijtag_reset {
        ActivePolarity 0;
    }
    TCKPort TCK_ijtag_tck;
    ScanInPort SI_ijtag_si;
    ScanOutPort SO_ijtag_so {
        Source Toggle;
    }
    ScanInterface client {
        Port SI_ijtag_si;
        Port SO_ijtag_so;
        Port Shift_ijtag_se;
        Port Capture_ijtag_ce;
        Port Update_ijtag_ue;
        Port Select_ijtag_sel;
        Port TRSTN_ijtag_reset;
        Port TCK_ijtag_tck;
    }
    ScanRegister Status {
        ScanInSource SI_ijtag_si;
        ResetValue 1'b0;
        Attribute synopsys_access = "RO";
    }
    ScanRegister Toggle {
        ScanInSource Status;
        ResetValue 1'b0;
        Attribute synopsys_access = "RO";
    }
    DataOutPort status_ijtag_do_1 {
        Source Status;
    }
    DataOutPort toggle_ijtag_do_2 {
        Source Toggle;
    }
}

// Source: all_out/1_pmm_ip_generation_with_1687_and_apb_sim/tessent_analysis_rtl_icl/icl/PATH_MONITOR_CONTROLLER_IEEE_1687_dr.icl
// Generated; edit the PMM sources, not this file.
Module PATH_MONITOR_CONTROLLER_IEEE_1687_dr {
    TCKPort i_tck_ijtag_tck {
        Attribute tessent_design_rtl_ports = "i_tck";
        Attribute tessent_design_gate_ports = "i_tck";
    }
    ResetPort i_Resetn_ijtag_reset {
        Attribute tessent_design_rtl_ports = "i_Resetn";
        Attribute tessent_design_gate_ports = "i_Resetn";
        ActivePolarity 0;
    }
    CaptureEnPort i_tm_sibCaptureEn_ijtag_ce_1 {
        Attribute tessent_design_rtl_ports = "i_tm_sibCaptureEn";
        Attribute tessent_design_gate_ports = "i_tm_sibCaptureEn";
    }
    UpdateEnPort i_tm_sibUpdateEn_ijtag_ue_1 {
        Attribute tessent_design_rtl_ports = "i_tm_sibUpdateEn";
        Attribute tessent_design_gate_ports = "i_tm_sibUpdateEn";
    }
    ShiftEnPort i_tm_sibShiftEn_ijtag_se_1 {
        Attribute tessent_design_rtl_ports = "i_tm_sibShiftEn";
        Attribute tessent_design_gate_ports = "i_tm_sibShiftEn";
    }
    ScanInPort i_tm_tdr_scanin_ijtag_si_1 {
        Attribute tessent_design_rtl_ports = "i_tm_tdr_scanin";
        Attribute tessent_design_gate_ports = "i_tm_tdr_scanin";
    }
    ScanOutPort o_tm_tdr_scanout_ijtag_so_1 {
        Attribute tessent_design_rtl_ports = "o_tm_tdr_scanout";
        Attribute tessent_design_gate_ports = "o_tm_tdr_scanout";
        Source IEEE_1687_tm_tdr_inst.o_tdr_scanout_ijtag_so;
    }
    DataOutPort o_test_mode_ijtag_do_1[5:0] {
        Attribute tessent_design_rtl_ports = "{o_test_mode[5]} {o_test_mode[4]} {o_test_mode[3]} {o_test_mode[2]} {o_test_mode[1]} {o_test_mode[0]}";
        Attribute tessent_design_gate_ports = "{o_test_mode[5]} {o_test_mode[4]} {o_test_mode[3]} {o_test_mode[2]} {o_test_mode[1]} {o_test_mode[0]}";
        Source IEEE_1687_tm_tdr_inst.o_tdr_update_ijtag_do;
    }
    ShiftEnPort i_pmu_func_clk_gate_en_tdr_sibShiftEn_ijtag_se_2 {
        Attribute tessent_design_rtl_ports = "i_pmu_func_clk_gate_en_tdr_sibShiftEn";
        Attribute tessent_design_gate_ports = "i_pmu_func_clk_gate_en_tdr_sibShiftEn";
    }
    CaptureEnPort i_pmu_func_clk_gate_en_tdr_sibCaptureEn_ijtag_ce_2 {
        Attribute tessent_design_rtl_ports = "i_pmu_func_clk_gate_en_tdr_sibCaptureEn";
        Attribute tessent_design_gate_ports = "i_pmu_func_clk_gate_en_tdr_sibCaptureEn";
    }
    UpdateEnPort i_pmu_func_clk_gate_en_tdr_sibUpdateEn_ijtag_ue_2 {
        Attribute tessent_design_rtl_ports = "i_pmu_func_clk_gate_en_tdr_sibUpdateEn";
        Attribute tessent_design_gate_ports = "i_pmu_func_clk_gate_en_tdr_sibUpdateEn";
    }
    ScanInPort i_pmu_func_clk_gate_en_tdr_scanin_ijtag_si_2 {
        Attribute tessent_design_rtl_ports = "i_pmu_func_clk_gate_en_tdr_scanin";
        Attribute tessent_design_gate_ports = "i_pmu_func_clk_gate_en_tdr_scanin";
    }
    ScanOutPort o_pmu_func_clk_gate_en_tdr_scanout_ijtag_so_2 {
        Attribute tessent_design_rtl_ports = "o_pmu_func_clk_gate_en_tdr_scanout";
        Attribute tessent_design_gate_ports = "o_pmu_func_clk_gate_en_tdr_scanout";
        Source IEEE_1687_pmu_func_clk_gate_en_tdr_inst.o_tdr_scanout_ijtag_so;
    }
    DataOutPort o_pmu_func_clk_gate_en_ijtag_do_2[1:1] {
        Attribute tessent_design_rtl_ports = "{o_pmu_func_clk_gate_en[1]}";
        Attribute tessent_design_gate_ports = "{o_pmu_func_clk_gate_en[1]}";
        Source IEEE_1687_pmu_func_clk_gate_en_tdr_inst.o_tdr_update_ijtag_do[0:0];
    }
    ShiftEnPort i_sib_loop_bypass_tdr_sibShiftEn_ijtag_se_3 {
        Attribute tessent_design_rtl_ports = "i_sib_loop_bypass_tdr_sibShiftEn";
        Attribute tessent_design_gate_ports = "i_sib_loop_bypass_tdr_sibShiftEn";
    }
    CaptureEnPort i_sib_loop_bypass_tdr_sibCaptureEn_ijtag_ce_3 {
        Attribute tessent_design_rtl_ports = "i_sib_loop_bypass_tdr_sibCaptureEn";
        Attribute tessent_design_gate_ports = "i_sib_loop_bypass_tdr_sibCaptureEn";
    }
    UpdateEnPort i_sib_loop_bypass_tdr_sibUpdateEn_ijtag_ue_3 {
        Attribute tessent_design_rtl_ports = "i_sib_loop_bypass_tdr_sibUpdateEn";
        Attribute tessent_design_gate_ports = "i_sib_loop_bypass_tdr_sibUpdateEn";
    }
    ScanInPort i_sib_loop_bypass_tdr_scanin_ijtag_si_3 {
        Attribute tessent_design_rtl_ports = "i_sib_loop_bypass_tdr_scanin";
        Attribute tessent_design_gate_ports = "i_sib_loop_bypass_tdr_scanin";
    }
    ScanOutPort o_sib_loop_bypass_tdr_scanout_ijtag_so_3 {
        Attribute tessent_design_rtl_ports = "o_sib_loop_bypass_tdr_scanout";
        Attribute tessent_design_gate_ports = "o_sib_loop_bypass_tdr_scanout";
        Source IEEE_1687_pmu_sib_loop_bypass_tdr_inst.o_tdr_scanout_ijtag_so;
    }
    ShiftEnPort i_sib2_bypass_tdr_sibShiftEn_ijtag_se_4[1:1] {
        Attribute tessent_design_rtl_ports = "{i_sib2_bypass_tdr_sibShiftEn[1]}";
        Attribute tessent_design_gate_ports = "{i_sib2_bypass_tdr_sibShiftEn[1]}";
    }
    CaptureEnPort i_sib2_bypass_tdr_sibCaptureEn_ijtag_ce_4[1:1] {
        Attribute tessent_design_rtl_ports = "{i_sib2_bypass_tdr_sibCaptureEn[1]}";
        Attribute tessent_design_gate_ports = "{i_sib2_bypass_tdr_sibCaptureEn[1]}";
    }
    UpdateEnPort i_sib2_bypass_tdr_sibUpdateEn_ijtag_ue_4[1:1] {
        Attribute tessent_design_rtl_ports = "{i_sib2_bypass_tdr_sibUpdateEn[1]}";
        Attribute tessent_design_gate_ports = "{i_sib2_bypass_tdr_sibUpdateEn[1]}";
    }
    ScanInPort i_sib2_bypass_tdr_scanin_ijtag_si_4[1:1] {
        Attribute tessent_design_rtl_ports = "{i_sib2_bypass_tdr_scanin[1]}";
        Attribute tessent_design_gate_ports = "{i_sib2_bypass_tdr_scanin[1]}";
    }
    ScanOutPort o_sib2_bypass_tdr_scanout_ijtag_so_4[1:1] {
        Attribute tessent_design_rtl_ports = "{o_sib2_bypass_tdr_scanout[1]}";
        Attribute tessent_design_gate_ports = "{o_sib2_bypass_tdr_scanout[1]}";
        Source IEEE_1687_sib2_bypass_tdr_inst_1.o_tdr_scanout_ijtag_so;
    }
    ShiftEnPort i_pmu_path_select_tdr_sibShiftEn_ijtag_se_5[1:1] {
        Attribute tessent_design_rtl_ports = "{i_pmu_path_select_tdr_sibShiftEn[1]}";
        Attribute tessent_design_gate_ports = "{i_pmu_path_select_tdr_sibShiftEn[1]}";
    }
    DataInPort i_pmu_path_select_tdr_sib_ijtag_di_1[1:1] {
        Attribute tessent_design_rtl_ports = "{i_pmu_path_select_tdr_sib[1]}";
        Attribute tessent_design_gate_ports = "{i_pmu_path_select_tdr_sib[1]}";
    }
    CaptureEnPort i_pmu_path_select_tdr_sibCaptureEn_ijtag_ce_5[1:1] {
        Attribute tessent_design_rtl_ports = "{i_pmu_path_select_tdr_sibCaptureEn[1]}";
        Attribute tessent_design_gate_ports = "{i_pmu_path_select_tdr_sibCaptureEn[1]}";
    }
    UpdateEnPort i_pmu_path_select_tdr_sibUpdateEn_ijtag_ue_5[1:1] {
        Attribute tessent_design_rtl_ports = "{i_pmu_path_select_tdr_sibUpdateEn[1]}";
        Attribute tessent_design_gate_ports = "{i_pmu_path_select_tdr_sibUpdateEn[1]}";
    }
    ScanInPort i_pmu_path_select_tdr_scanin_ijtag_si_5[1:1] {
        Attribute tessent_design_rtl_ports = "{i_pmu_path_select_tdr_scanin[1]}";
        Attribute tessent_design_gate_ports = "{i_pmu_path_select_tdr_scanin[1]}";
    }
    ScanOutPort o_pmu_path_select_tdr_scanout_ijtag_so_5[1:1] {
        Attribute tessent_design_rtl_ports = "{o_pmu_path_select_tdr_scanout[1]}";
        Attribute tessent_design_gate_ports = "{o_pmu_path_select_tdr_scanout[1]}";
        Source IEEE_1687_pmu_path_select_tdr_inst_1.o_pmu_tdr_scanout_ijtag_so;
    }
    ToSelectPort o_configure_path_sel_pmu_ijtag_to_sel_1[1:1] {
        Attribute tessent_design_rtl_ports = "{o_configure_path_sel_pmu[1]}";
        Attribute tessent_design_gate_ports = "{o_configure_path_sel_pmu[1]}";
        Source IEEE_1687_pmu_path_select_tdr_inst_1.o_pmu_shift_ijtag_to_sel;
    }
    ScanOutPort o_path_select_ijtag_to_si_1[1:1] {
        Attribute tessent_design_rtl_ports = "{o_path_select[1]}";
        Attribute tessent_design_gate_ports = "{o_path_select[1]}";
        Source IEEE_1687_pmu_path_select_tdr_inst_1.o_pmu_tdr_data_in_ijtag_to_si;
    }
    ShiftEnPort i_pmu_delay_select_tdr_sibShiftEn_ijtag_se_6[1:1] {
        Attribute tessent_design_rtl_ports = "{i_pmu_delay_select_tdr_sibShiftEn[1]}";
        Attribute tessent_design_gate_ports = "{i_pmu_delay_select_tdr_sibShiftEn[1]}";
    }
    DataInPort i_pmu_delay_select_tdr_sib_ijtag_di_2[1:1] {
        Attribute tessent_design_rtl_ports = "{i_pmu_delay_select_tdr_sib[1]}";
        Attribute tessent_design_gate_ports = "{i_pmu_delay_select_tdr_sib[1]}";
    }
    CaptureEnPort i_pmu_delay_select_tdr_sibCaptureEn_ijtag_ce_6[1:1] {
        Attribute tessent_design_rtl_ports = "{i_pmu_delay_select_tdr_sibCaptureEn[1]}";
        Attribute tessent_design_gate_ports = "{i_pmu_delay_select_tdr_sibCaptureEn[1]}";
    }
    UpdateEnPort i_pmu_delay_select_tdr_sibUpdateEn_ijtag_ue_6[1:1] {
        Attribute tessent_design_rtl_ports = "{i_pmu_delay_select_tdr_sibUpdateEn[1]}";
        Attribute tessent_design_gate_ports = "{i_pmu_delay_select_tdr_sibUpdateEn[1]}";
    }
    ScanInPort i_pmu_delay_select_tdr_scanin_ijtag_si_6[1:1] {
        Attribute tessent_design_rtl_ports = "{i_pmu_delay_select_tdr_scanin[1]}";
        Attribute tessent_design_gate_ports = "{i_pmu_delay_select_tdr_scanin[1]}";
    }
    ScanOutPort o_pmu_delay_select_tdr_scanout_ijtag_so_6[1:1] {
        Attribute tessent_design_rtl_ports = "{o_pmu_delay_select_tdr_scanout[1]}";
        Attribute tessent_design_gate_ports = "{o_pmu_delay_select_tdr_scanout[1]}";
        Source IEEE_1687_pmu_delay_select_tdr_inst_1.o_pmu_tdr_scanout_ijtag_so;
    }
    ScanOutPort o_delay_select_ijtag_to_si_2[1:1] {
        Attribute tessent_design_rtl_ports = "{o_delay_select[1]}";
        Attribute tessent_design_gate_ports = "{o_delay_select[1]}";
        Source IEEE_1687_pmu_delay_select_tdr_inst_1.o_pmu_tdr_data_in_ijtag_to_si;
    }
    ToSelectPort o_configure_delay_select_pmu_ijtag_to_sel_2[1:1] {
        Attribute tessent_design_rtl_ports = "{o_configure_delay_select_pmu[1]}";
        Attribute tessent_design_gate_ports = "{o_configure_delay_select_pmu[1]}";
        Source IEEE_1687_pmu_delay_select_tdr_inst_1.o_pmu_shift_ijtag_to_sel;
    }
    ShiftEnPort i_pmu_collect_status_tdr_sibShiftEn_ijtag_se_7[1:1] {
        Attribute tessent_design_rtl_ports = "{i_pmu_collect_status_tdr_sibShiftEn[1]}";
        Attribute tessent_design_gate_ports = "{i_pmu_collect_status_tdr_sibShiftEn[1]}";
    }
    DataInPort i_pmu_collect_status_tdr_sib_ijtag_di_3[1:1] {
        Attribute tessent_design_rtl_ports = "{i_pmu_collect_status_tdr_sib[1]}";
        Attribute tessent_design_gate_ports = "{i_pmu_collect_status_tdr_sib[1]}";
    }
    CaptureEnPort i_pmu_collect_status_tdr_sibCaptureEn_ijtag_ce_7[1:1] {
        Attribute tessent_design_rtl_ports = "{i_pmu_collect_status_tdr_sibCaptureEn[1]}";
        Attribute tessent_design_gate_ports = "{i_pmu_collect_status_tdr_sibCaptureEn[1]}";
    }
    UpdateEnPort i_pmu_collect_status_tdr_sibUpdateEn_ijtag_ue_7[1:1] {
        Attribute tessent_design_rtl_ports = "{i_pmu_collect_status_tdr_sibUpdateEn[1]}";
        Attribute tessent_design_gate_ports = "{i_pmu_collect_status_tdr_sibUpdateEn[1]}";
    }
    ScanInPort i_pmu_collect_status_tdr_scanin_ijtag_si_7[1:1] {
        Attribute tessent_design_rtl_ports = "{i_pmu_collect_status_tdr_scanin[1]}";
        Attribute tessent_design_gate_ports = "{i_pmu_collect_status_tdr_scanin[1]}";
    }
    ScanOutPort o_pmu_collect_status_tdr_scanout_ijtag_so_7[1:1] {
        Attribute tessent_design_rtl_ports = "{o_pmu_collect_status_tdr_scanout[1]}";
        Attribute tessent_design_gate_ports = "{o_pmu_collect_status_tdr_scanout[1]}";
        Source IEEE_1687_pmu_collect_status_tdr_inst_1.o_pmu_tdr_scanout_ijtag_so;
    }
    ToSelectPort o_pmu_collect_status_ijtag_to_sel_3[1:1] {
        Attribute tessent_design_rtl_ports = "{o_pmu_collect_status[1]}";
        Attribute tessent_design_gate_ports = "{o_pmu_collect_status[1]}";
        Source IEEE_1687_pmu_collect_status_tdr_inst_1.o_pmu_shift_ijtag_to_sel;
    }
    ScanInPort i_pmu_data_out_ijtag_from_so_1[1:1] {
        Attribute tessent_design_rtl_ports = "{i_pmu_data_out[1]}";
        Attribute tessent_design_gate_ports = "{i_pmu_data_out[1]}";
    }
    ScanOutPort o_pmu_status_ijtag_to_si_3[1:1] {
        Attribute tessent_design_rtl_ports = "{o_pmu_status[1]}";
        Attribute tessent_design_gate_ports = "{o_pmu_status[1]}";
        Source IEEE_1687_pmu_collect_status_tdr_inst_1.o_pmu_tdr_data_in_ijtag_to_si;
    }
    DataInPort i_pmu_capture_ff_tdr_sib_ijtag_di_4[1:1] {
        Attribute tessent_design_rtl_ports = "{i_pmu_capture_ff_tdr_sib[1]}";
        Attribute tessent_design_gate_ports = "{i_pmu_capture_ff_tdr_sib[1]}";
    }
    ShiftEnPort i_pmu_capture_ff_tdr_sibShiftEn_ijtag_se_8[1:1] {
        Attribute tessent_design_rtl_ports = "{i_pmu_capture_ff_tdr_sibShiftEn[1]}";
        Attribute tessent_design_gate_ports = "{i_pmu_capture_ff_tdr_sibShiftEn[1]}";
    }
    CaptureEnPort i_pmu_capture_ff_tdr_sibCaptureEn_ijtag_ce_8[1:1] {
        Attribute tessent_design_rtl_ports = "{i_pmu_capture_ff_tdr_sibCaptureEn[1]}";
        Attribute tessent_design_gate_ports = "{i_pmu_capture_ff_tdr_sibCaptureEn[1]}";
    }
    UpdateEnPort i_pmu_capture_ff_tdr_sibUpdateEn_ijtag_ue_8[1:1] {
        Attribute tessent_design_rtl_ports = "{i_pmu_capture_ff_tdr_sibUpdateEn[1]}";
        Attribute tessent_design_gate_ports = "{i_pmu_capture_ff_tdr_sibUpdateEn[1]}";
    }
    ScanInPort i_pmu_capture_ff_tdr_scanin_ijtag_si_8[1:1] {
        Attribute tessent_design_rtl_ports = "{i_pmu_capture_ff_tdr_scanin[1]}";
        Attribute tessent_design_gate_ports = "{i_pmu_capture_ff_tdr_scanin[1]}";
    }
    ScanOutPort o_pmu_capture_tdr_scanout_ijtag_so_8[1:1] {
        Attribute tessent_design_rtl_ports = "{o_pmu_capture_tdr_scanout[1]}";
        Attribute tessent_design_gate_ports = "{o_pmu_capture_tdr_scanout[1]}";
        Source IEEE_1687_pmu_capture_ff_tdr_inst_1.o_pmu_tdr_scanout_ijtag_so;
    }
    ScanOutPort o_pmu_capture_ff_tdr_data_in_ijtag_to_si_4[1:1] {
        Attribute tessent_design_rtl_ports = "{o_pmu_capture_ff_tdr_data_in[1]}";
        Attribute tessent_design_gate_ports = "{o_pmu_capture_ff_tdr_data_in[1]}";
        Source IEEE_1687_pmu_capture_ff_tdr_inst_1.o_pmu_tdr_data_in_ijtag_to_si;
    }
    ScanInPort i_pmu_capture_ff_data_out_ijtag_from_so_2[1:1] {
        Attribute tessent_design_rtl_ports = "{i_pmu_capture_ff_data_out[1]}";
        Attribute tessent_design_gate_ports = "{i_pmu_capture_ff_data_out[1]}";
    }
    ToSelectPort o_pmu_capture_ff_shift_ijtag_to_sel_4[1:1] {
        Attribute tessent_design_rtl_ports = "{o_pmu_capture_ff_shift[1]}";
        Attribute tessent_design_gate_ports = "{o_pmu_capture_ff_shift[1]}";
        Source IEEE_1687_pmu_capture_ff_tdr_inst_1.o_pmu_shift_ijtag_to_sel;
    }
    Instance IEEE_1687_tm_tdr_inst Of PATH_MONITOR_CONTROLLER_IEEE_1687_tdr_SIX_WIDTH {
        InputPort i_tck_ijtag_tck = i_tck_ijtag_tck;
        InputPort i_resetn_ijtag_reset = i_Resetn_ijtag_reset;
        InputPort i_tdr_sibShiftEn_ijtag_se = i_tm_sibShiftEn_ijtag_se_1;
        InputPort i_tdr_sibCaptureEn_ijtag_ce = i_tm_sibCaptureEn_ijtag_ce_1;
        InputPort i_tdr_sibUpdateEn_ijtag_ue = i_tm_sibUpdateEn_ijtag_ue_1;
        InputPort i_tdr_scanin_ijtag_si = i_tm_tdr_scanin_ijtag_si_1;
    }
    Instance IEEE_1687_pmu_func_clk_gate_en_tdr_inst Of PATH_MONITOR_CONTROLLER_IEEE_1687_tdr_FIFTEEN_WIDTH {
        InputPort i_tck_ijtag_tck = i_tck_ijtag_tck;
        InputPort i_resetn_ijtag_reset = i_Resetn_ijtag_reset;
        InputPort i_tdr_sibShiftEn_ijtag_se = i_pmu_func_clk_gate_en_tdr_sibShiftEn_ijtag_se_2;
        InputPort i_tdr_sibCaptureEn_ijtag_ce = i_pmu_func_clk_gate_en_tdr_sibCaptureEn_ijtag_ce_2;
        InputPort i_tdr_sibUpdateEn_ijtag_ue = i_pmu_func_clk_gate_en_tdr_sibUpdateEn_ijtag_ue_2;
        InputPort i_tdr_scanin_ijtag_si = i_pmu_func_clk_gate_en_tdr_scanin_ijtag_si_2;
    }
    Instance IEEE_1687_pmu_sib_loop_bypass_tdr_inst Of PATH_MONITOR_CONTROLLER_IEEE_1687_one_bit_tdr {
        InputPort i_tck_ijtag_tck = i_tck_ijtag_tck;
        InputPort i_resetn_ijtag_reset = i_Resetn_ijtag_reset;
        InputPort i_tdr_sibShiftEn_ijtag_se = i_sib_loop_bypass_tdr_sibShiftEn_ijtag_se_3;
        InputPort i_tdr_sibCaptureEn_ijtag_ce = i_sib_loop_bypass_tdr_sibCaptureEn_ijtag_ce_3;
        InputPort i_tdr_sibUpdateEn_ijtag_ue = i_sib_loop_bypass_tdr_sibUpdateEn_ijtag_ue_3;
        InputPort i_tdr_scanin_ijtag_si = i_sib_loop_bypass_tdr_scanin_ijtag_si_3;
    }
    Instance IEEE_1687_sib2_bypass_tdr_inst_1 Of PATH_MONITOR_CONTROLLER_IEEE_1687_one_bit_tdr {
        InputPort i_tck_ijtag_tck = i_tck_ijtag_tck;
        InputPort i_resetn_ijtag_reset = i_Resetn_ijtag_reset;
        InputPort i_tdr_sibShiftEn_ijtag_se = i_sib2_bypass_tdr_sibShiftEn_ijtag_se_4[1];
        InputPort i_tdr_sibCaptureEn_ijtag_ce = i_sib2_bypass_tdr_sibCaptureEn_ijtag_ce_4[1];
        InputPort i_tdr_sibUpdateEn_ijtag_ue = i_sib2_bypass_tdr_sibUpdateEn_ijtag_ue_4[1];
        InputPort i_tdr_scanin_ijtag_si = i_sib2_bypass_tdr_scanin_ijtag_si_4[1];
    }
    Instance IEEE_1687_pmu_path_select_tdr_inst_1 Of PATH_MONITOR_CONTROLLER_IEEE_1687_pmu_tdr {
        InputPort i_tck_ijtag_tck = i_tck_ijtag_tck;
        InputPort i_resetn_ijtag_reset = i_Resetn_ijtag_reset;
        InputPort i_pmu_tdr_sibShiftEn_ijtag_se = i_pmu_path_select_tdr_sibShiftEn_ijtag_se_5[1];
        InputPort i_pmu_tdr_sib_ijtag_di = i_pmu_path_select_tdr_sib_ijtag_di_1[1];
        InputPort i_pmu_tdr_sibCaptureEn_ijtag_ce = i_pmu_path_select_tdr_sibCaptureEn_ijtag_ce_5[1];
        InputPort i_pmu_tdr_sibUpdateEn_ijtag_ue = i_pmu_path_select_tdr_sibUpdateEn_ijtag_ue_5[1];
        InputPort i_pmu_tdr_scanin_ijtag_si = i_pmu_path_select_tdr_scanin_ijtag_si_5[1];
        InputPort i_pmu_tdr_data_out_ijtag_from_so = i_pmu_data_out_ijtag_from_so_1[1];
    }
    Instance IEEE_1687_pmu_delay_select_tdr_inst_1 Of PATH_MONITOR_CONTROLLER_IEEE_1687_pmu_tdr {
        InputPort i_tck_ijtag_tck = i_tck_ijtag_tck;
        InputPort i_resetn_ijtag_reset = i_Resetn_ijtag_reset;
        InputPort i_pmu_tdr_sibShiftEn_ijtag_se = i_pmu_delay_select_tdr_sibShiftEn_ijtag_se_6[1];
        InputPort i_pmu_tdr_sib_ijtag_di = i_pmu_delay_select_tdr_sib_ijtag_di_2[1];
        InputPort i_pmu_tdr_sibCaptureEn_ijtag_ce = i_pmu_delay_select_tdr_sibCaptureEn_ijtag_ce_6[1];
        InputPort i_pmu_tdr_sibUpdateEn_ijtag_ue = i_pmu_delay_select_tdr_sibUpdateEn_ijtag_ue_6[1];
        InputPort i_pmu_tdr_scanin_ijtag_si = i_pmu_delay_select_tdr_scanin_ijtag_si_6[1];
        InputPort i_pmu_tdr_data_out_ijtag_from_so = i_pmu_data_out_ijtag_from_so_1[1];
    }
    Instance IEEE_1687_pmu_collect_status_tdr_inst_1 Of PATH_MONITOR_CONTROLLER_IEEE_1687_pmu_tdr {
        InputPort i_tck_ijtag_tck = i_tck_ijtag_tck;
        InputPort i_resetn_ijtag_reset = i_Resetn_ijtag_reset;
        InputPort i_pmu_tdr_sibShiftEn_ijtag_se = i_pmu_collect_status_tdr_sibShiftEn_ijtag_se_7[1];
        InputPort i_pmu_tdr_sib_ijtag_di = i_pmu_collect_status_tdr_sib_ijtag_di_3[1];
        InputPort i_pmu_tdr_sibCaptureEn_ijtag_ce = i_pmu_collect_status_tdr_sibCaptureEn_ijtag_ce_7[1];
        InputPort i_pmu_tdr_sibUpdateEn_ijtag_ue = i_pmu_collect_status_tdr_sibUpdateEn_ijtag_ue_7[1];
        InputPort i_pmu_tdr_scanin_ijtag_si = i_pmu_collect_status_tdr_scanin_ijtag_si_7[1];
        InputPort i_pmu_tdr_data_out_ijtag_from_so = i_pmu_data_out_ijtag_from_so_1[1];
    }
    Instance IEEE_1687_pmu_capture_ff_tdr_inst_1 Of PATH_MONITOR_CONTROLLER_IEEE_1687_pmu_tdr {
        InputPort i_tck_ijtag_tck = i_tck_ijtag_tck;
        InputPort i_resetn_ijtag_reset = i_Resetn_ijtag_reset;
        InputPort i_pmu_tdr_sibShiftEn_ijtag_se = i_pmu_capture_ff_tdr_sibShiftEn_ijtag_se_8[1];
        InputPort i_pmu_tdr_sib_ijtag_di = i_pmu_capture_ff_tdr_sib_ijtag_di_4[1];
        InputPort i_pmu_tdr_sibCaptureEn_ijtag_ce = i_pmu_capture_ff_tdr_sibCaptureEn_ijtag_ce_8[1];
        InputPort i_pmu_tdr_sibUpdateEn_ijtag_ue = i_pmu_capture_ff_tdr_sibUpdateEn_ijtag_ue_8[1];
        InputPort i_pmu_tdr_scanin_ijtag_si = i_pmu_capture_ff_tdr_scanin_ijtag_si_8[1];
        InputPort i_pmu_tdr_data_out_ijtag_from_so = i_pmu_capture_ff_data_out_ijtag_from_so_2[1];
    }
    ScanInterface client_IEEE_1687_tm_tdr_inst {
        Port i_tm_tdr_scanin_ijtag_si_1;
        Port o_tm_tdr_scanout_ijtag_so_1;
        Port i_tm_sibShiftEn_ijtag_se_1;
        Port i_tm_sibCaptureEn_ijtag_ce_1;
        Port i_tm_sibUpdateEn_ijtag_ue_1;
        Port i_tck_ijtag_tck;
        Port i_Resetn_ijtag_reset;
    }
    ScanInterface client_IEEE_1687_pmu_func_clk_gate_en_tdr_inst {
        Port i_pmu_func_clk_gate_en_tdr_scanin_ijtag_si_2;
        Port o_pmu_func_clk_gate_en_tdr_scanout_ijtag_so_2;
        Port i_pmu_func_clk_gate_en_tdr_sibShiftEn_ijtag_se_2;
        Port i_pmu_func_clk_gate_en_tdr_sibCaptureEn_ijtag_ce_2;
        Port i_pmu_func_clk_gate_en_tdr_sibUpdateEn_ijtag_ue_2;
        Port i_tck_ijtag_tck;
        Port i_Resetn_ijtag_reset;
    }
    ScanInterface client_IEEE_1687_pmu_sib_loop_bypass_tdr_inst {
        Port i_sib_loop_bypass_tdr_scanin_ijtag_si_3;
        Port o_sib_loop_bypass_tdr_scanout_ijtag_so_3;
        Port i_sib_loop_bypass_tdr_sibShiftEn_ijtag_se_3;
        Port i_sib_loop_bypass_tdr_sibCaptureEn_ijtag_ce_3;
        Port i_sib_loop_bypass_tdr_sibUpdateEn_ijtag_ue_3;
        Port i_tck_ijtag_tck;
        Port i_Resetn_ijtag_reset;
    }
    ScanInterface client_IEEE_1687_sib2_bypass_tdr_inst_1 {
        Port i_sib2_bypass_tdr_scanin_ijtag_si_4[1];
        Port o_sib2_bypass_tdr_scanout_ijtag_so_4[1];
        Port i_sib2_bypass_tdr_sibShiftEn_ijtag_se_4[1];
        Port i_sib2_bypass_tdr_sibCaptureEn_ijtag_ce_4[1];
        Port i_sib2_bypass_tdr_sibUpdateEn_ijtag_ue_4[1];
        Port i_tck_ijtag_tck;
        Port i_Resetn_ijtag_reset;
    }
    ScanInterface client_IEEE_1687_pmu_path_select_tdr_inst_1 {
        Port i_pmu_path_select_tdr_scanin_ijtag_si_5[1];
        Port o_pmu_path_select_tdr_scanout_ijtag_so_5[1];
        Port i_pmu_path_select_tdr_sibShiftEn_ijtag_se_5[1];
        Port i_pmu_path_select_tdr_sibCaptureEn_ijtag_ce_5[1];
        Port i_pmu_path_select_tdr_sibUpdateEn_ijtag_ue_5[1];
        Port i_tck_ijtag_tck;
        Port i_Resetn_ijtag_reset;
    }
    ScanInterface host_IEEE_1687_pmu_path_select_tdr_inst_1 {
        Port i_pmu_data_out_ijtag_from_so_1[1];
        Port o_configure_path_sel_pmu_ijtag_to_sel_1[1];
    }
    ScanInterface client_IEEE_1687_pmu_delay_select_tdr_inst_1 {
        Port i_pmu_delay_select_tdr_scanin_ijtag_si_6[1];
        Port o_pmu_delay_select_tdr_scanout_ijtag_so_6[1];
        Port i_pmu_delay_select_tdr_sibShiftEn_ijtag_se_6[1];
        Port i_pmu_delay_select_tdr_sibCaptureEn_ijtag_ce_6[1];
        Port i_pmu_delay_select_tdr_sibUpdateEn_ijtag_ue_6[1];
        Port i_tck_ijtag_tck;
        Port i_Resetn_ijtag_reset;
    }
    ScanInterface host_IEEE_1687_pmu_delay_select_tdr_inst_1 {
        Port i_pmu_data_out_ijtag_from_so_1[1];
        Port o_configure_delay_select_pmu_ijtag_to_sel_2[1];
    }
    ScanInterface client_IEEE_1687_pmu_collect_status_tdr_inst_1 {
        Port i_pmu_collect_status_tdr_scanin_ijtag_si_7[1];
        Port o_pmu_collect_status_tdr_scanout_ijtag_so_7[1];
        Port i_pmu_collect_status_tdr_sibShiftEn_ijtag_se_7[1];
        Port i_pmu_collect_status_tdr_sibCaptureEn_ijtag_ce_7[1];
        Port i_pmu_collect_status_tdr_sibUpdateEn_ijtag_ue_7[1];
        Port i_tck_ijtag_tck;
        Port i_Resetn_ijtag_reset;
    }
    ScanInterface host_IEEE_1687_pmu_collect_status_tdr_inst_1 {
        Port i_pmu_data_out_ijtag_from_so_1[1];
        Port o_pmu_collect_status_ijtag_to_sel_3[1];
    }
    ScanInterface client_IEEE_1687_pmu_capture_ff_tdr_inst_1 {
        Port i_pmu_capture_ff_tdr_scanin_ijtag_si_8[1];
        Port o_pmu_capture_tdr_scanout_ijtag_so_8[1];
        Port i_pmu_capture_ff_tdr_sibShiftEn_ijtag_se_8[1];
        Port i_pmu_capture_ff_tdr_sibCaptureEn_ijtag_ce_8[1];
        Port i_pmu_capture_ff_tdr_sibUpdateEn_ijtag_ue_8[1];
        Port i_tck_ijtag_tck;
        Port i_Resetn_ijtag_reset;
    }
    ScanInterface host_IEEE_1687_pmu_capture_ff_tdr_inst_1 {
        Port i_pmu_capture_ff_data_out_ijtag_from_so_2[1];
        Port o_pmu_capture_ff_shift_ijtag_to_sel_4[1];
    }
}

