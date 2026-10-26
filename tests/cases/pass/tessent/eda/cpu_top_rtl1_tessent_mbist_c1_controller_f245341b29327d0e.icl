/*
----------------------------------------------------------------------------------
-                                                                                -
-  Unpublished work. Copyright 2026 Siemens                                      -
-                                                                                -
-  This material contains trade secrets or otherwise confidential                -
-  information owned by Siemens Industry Software Inc. or its affiliates         -
-  (collectively, SISW), or its licensors. Access to and use of this             -
-  information is strictly limited as set forth in the Customer's                -
-  applicable agreements with SISW.                                              -
-                                                                                -
----------------------------------------------------------------------------------
-  File created by: Tessent Shell                                                -
-          Version: 2026.2                                                       -
-       Created on: Mon Oct 26 08:54:25 CST 2026                                 -
----------------------------------------------------------------------------------


*/
// ============================================================================
// == Description     : ICL description for cpu_top_LVISION_MBISTPG_CTRL
// == Tool Name       : membistipCommonGenerate
// == Tool Version    : 2026.2      Thu May 21 21:09:45 GMT 2026
// ============================================================================

NameSpace ;
UseNameSpace ;
Module cpu_top_rtl1_tessent_mbist_c1_controller {
  ClockPort     BIST_CLK;
  DataInPort    MBISTPG_EN;
  DataInPort    LV_TM { Attribute connection_rule_option = "allowed_tied_low"; }
  DataInPort           MEM_BYPASS_EN {
    Attribute connection_rule_option = "allowed_tied_low";}
  DataInPort           MCP_BOUNDING_EN {
    Attribute connection_rule_option = "allowed_tied_low";}
  DataInPort    MBISTPG_ASYNC_RESETN {RefEnum AsyncResetN;}
  DataInPort    MBISTPG_DIAG_EN;
  DataInPort    BIST_SETUP2;
  DataInPort    BIST_SETUP[1:0];
  DataInPort    MBISTPG_REDUCED_ADDR_CNT_EN {RefEnum OnOff;}
  DataInPort    MBISTPG_MEM_RST {RefEnum OnOff;}
  DataInPort    MBISTPG_TESTDATA_SELECT {RefEnum OnOff;}
  DataInPort    FL_CNT_MODE[1:0];
  DataInPort    MBISTPG_ALGO_MODE[1:0];
  DataOutPort   MBISTPG_GO   {RefEnum PassFail;}
  DataOutPort   MBISTPG_DONE {RefEnum PassFail;}
  TCKPort       TCK;
  ScanInPort    BIST_SI;
  ScanOutPort   MBISTPG_SO {Source BIST_SO_INT;}
  ShiftEnPort   BIST_HOLD;
  ToShiftEnPort  BIST_SHIFT_COLLAR {Attribute connection_rule_option = "allowed_no_destination";}
  DataOutPort   BIST_COLLAR_EN0 {Attribute tessent_memory_alias = "m1";}
  ScanOutPort   MEM0_BIST_COLLAR_SI { Source MEM0_BIST_COLLAR_SI_INT;}
  ScanInPort    MEM0_BIST_COLLAR_SO;
  DataOutPort   BIST_COLLAR_EN1 {Attribute tessent_memory_alias = "m2";}
  ScanOutPort   MEM1_BIST_COLLAR_SI { Source MEM1_BIST_COLLAR_SI_INT;}
  ScanInPort    MEM1_BIST_COLLAR_SO;
  DataOutPort   BIST_COLLAR_EN2 {Attribute tessent_memory_alias = "m3";}
  ScanOutPort   MEM2_BIST_COLLAR_SI { Source MEM2_BIST_COLLAR_SI_INT;}
  ScanInPort    MEM2_BIST_COLLAR_SO;
  DataOutPort   BIST_COLLAR_EN3 {Attribute tessent_memory_alias = "m4";}
  ScanOutPort   MEM3_BIST_COLLAR_SI { Source MEM3_BIST_COLLAR_SI_INT;}
  ScanInPort    MEM3_BIST_COLLAR_SO;
  DataOutPort   BIST_COLLAR_EN4 {Attribute tessent_memory_alias = "m5";}
  ScanOutPort   MEM4_BIST_COLLAR_SI { Source MEM4_BIST_COLLAR_SI_INT;}
  ScanInPort    MEM4_BIST_COLLAR_SO;
  DataOutPort   BIST_COLLAR_EN5 {Attribute tessent_memory_alias = "m6";}
  ScanOutPort   MEM5_BIST_COLLAR_SI { Source MEM5_BIST_COLLAR_SI_INT;}
  ScanInPort    MEM5_BIST_COLLAR_SO;
  DataOutPort   BIST_COLLAR_EN6 {Attribute tessent_memory_alias = "m7";}
  ScanOutPort   MEM6_BIST_COLLAR_SI { Source MEM6_BIST_COLLAR_SI_INT;}
  ScanInPort    MEM6_BIST_COLLAR_SO;
  DataOutPort   BIST_COLLAR_EN7 {Attribute tessent_memory_alias = "m8";}
  ScanOutPort   MEM7_BIST_COLLAR_SI { Source MEM7_BIST_COLLAR_SI_INT;}
  ScanInPort    MEM7_BIST_COLLAR_SO;
  DataOutPort   BIST_COLLAR_EN8 {Attribute tessent_memory_alias = "m9";}
  ScanOutPort   MEM8_BIST_COLLAR_SI { Source MEM8_BIST_COLLAR_SI_INT;}
  ScanInPort    MEM8_BIST_COLLAR_SO;
  DataOutPort   BIST_COLLAR_EN9 {Attribute tessent_memory_alias = "m10";}
  ScanOutPort   MEM9_BIST_COLLAR_SI { Source MEM9_BIST_COLLAR_SI_INT;}
  ScanInPort    MEM9_BIST_COLLAR_SO;
  DataOutPort   BIST_COLLAR_EN10 {Attribute tessent_memory_alias = "m11";}
  ScanOutPort   MEM10_BIST_COLLAR_SI { Source MEM10_BIST_COLLAR_SI_INT;}
  ScanInPort    MEM10_BIST_COLLAR_SO;
  DataOutPort   BIST_COLLAR_EN11 {Attribute tessent_memory_alias = "m12";}
  ScanOutPort   MEM11_BIST_COLLAR_SI { Source MEM11_BIST_COLLAR_SI_INT;}
  ScanInPort    MEM11_BIST_COLLAR_SO;
  DataOutPort   BIST_COLLAR_EN12 {Attribute tessent_memory_alias = "m13";}
  ScanOutPort   MEM12_BIST_COLLAR_SI { Source MEM12_BIST_COLLAR_SI_INT;}
  ScanInPort    MEM12_BIST_COLLAR_SO;
  DataOutPort   BIST_COLLAR_EN13 {Attribute tessent_memory_alias = "m14";}
  ScanOutPort   MEM13_BIST_COLLAR_SI { Source MEM13_BIST_COLLAR_SI_INT;}
  ScanInPort    MEM13_BIST_COLLAR_SO;
  DataOutPort   BIST_COLLAR_EN14 {Attribute tessent_memory_alias = "m15";}
  ScanOutPort   MEM14_BIST_COLLAR_SI { Source MEM14_BIST_COLLAR_SI_INT;}
  ScanInPort    MEM14_BIST_COLLAR_SO;
  DataOutPort   BIST_COLLAR_EN15 {Attribute tessent_memory_alias = "m16";}
  ScanOutPort   MEM15_BIST_COLLAR_SI { Source MEM15_BIST_COLLAR_SI_INT;}
  ScanInPort    MEM15_BIST_COLLAR_SO;
  DataOutPort   BIST_COLLAR_EN16 {Attribute tessent_memory_alias = "m17";}
  ScanOutPort   MEM16_BIST_COLLAR_SI { Source MEM16_BIST_COLLAR_SI_INT;}
  ScanInPort    MEM16_BIST_COLLAR_SO;
  DataOutPort   BIST_COLLAR_EN17 {Attribute tessent_memory_alias = "m18";}
  ScanOutPort   MEM17_BIST_COLLAR_SI { Source MEM17_BIST_COLLAR_SI_INT;}
  ScanInPort    MEM17_BIST_COLLAR_SO;
  DataOutPort   BIST_COLLAR_EN18 {Attribute tessent_memory_alias = "m19";}
  ScanOutPort   MEM18_BIST_COLLAR_SI { Source MEM18_BIST_COLLAR_SI_INT;}
  ScanInPort    MEM18_BIST_COLLAR_SO;
  DataOutPort   BIST_COLLAR_EN19 {Attribute tessent_memory_alias = "m20";}
  ScanOutPort   MEM19_BIST_COLLAR_SI { Source MEM19_BIST_COLLAR_SI_INT;}
  ScanInPort    MEM19_BIST_COLLAR_SO;
  DataOutPort   BIST_COLLAR_EN20 {Attribute tessent_memory_alias = "m21";}
  ScanOutPort   MEM20_BIST_COLLAR_SI { Source MEM20_BIST_COLLAR_SI_INT;}
  ScanInPort    MEM20_BIST_COLLAR_SO;
  DataOutPort   BIST_COLLAR_EN21 {Attribute tessent_memory_alias = "m22";}
  ScanOutPort   MEM21_BIST_COLLAR_SI { Source MEM21_BIST_COLLAR_SI_INT;}
  ScanInPort    MEM21_BIST_COLLAR_SO;
  DataOutPort   BIST_COLLAR_EN22 {Attribute tessent_memory_alias = "m23";}
  ScanOutPort   MEM22_BIST_COLLAR_SI { Source MEM22_BIST_COLLAR_SI_INT;}
  ScanInPort    MEM22_BIST_COLLAR_SO;
  DataOutPort   BIST_COLLAR_EN23 {Attribute tessent_memory_alias = "m24";}
  ScanOutPort   MEM23_BIST_COLLAR_SI { Source MEM23_BIST_COLLAR_SI_INT;}
  ScanInPort    MEM23_BIST_COLLAR_SO;
  DataOutPort   BIST_COLLAR_EN24 {Attribute tessent_memory_alias = "m25";}
  ScanOutPort   MEM24_BIST_COLLAR_SI { Source MEM24_BIST_COLLAR_SI_INT;}
  ScanInPort    MEM24_BIST_COLLAR_SO;
  DataOutPort   BIST_COLLAR_EN25 {Attribute tessent_memory_alias = "m26";}
  ScanOutPort   MEM25_BIST_COLLAR_SI { Source MEM25_BIST_COLLAR_SI_INT;}
  ScanInPort    MEM25_BIST_COLLAR_SO;
  DataOutPort   BIST_COLLAR_EN26 {Attribute tessent_memory_alias = "m27";}
  ScanOutPort   MEM26_BIST_COLLAR_SI { Source MEM26_BIST_COLLAR_SI_INT;}
  ScanInPort    MEM26_BIST_COLLAR_SO;
  DataOutPort   BIST_COLLAR_EN27 {Attribute tessent_memory_alias = "m28";}
  ScanOutPort   MEM27_BIST_COLLAR_SI { Source MEM27_BIST_COLLAR_SI_INT;}
  ScanInPort    MEM27_BIST_COLLAR_SO;
  DataOutPort   BIST_COLLAR_EN28 {Attribute tessent_memory_alias = "m29";}
  ScanOutPort   MEM28_BIST_COLLAR_SI { Source MEM28_BIST_COLLAR_SI_INT;}
  ScanInPort    MEM28_BIST_COLLAR_SO;
  DataOutPort   BIST_COLLAR_EN29 {Attribute tessent_memory_alias = "m30";}
  ScanOutPort   MEM29_BIST_COLLAR_SI { Source MEM29_BIST_COLLAR_SI_INT;}
  ScanInPort    MEM29_BIST_COLLAR_SO;
  DataOutPort   BIST_COLLAR_EN30 {Attribute tessent_memory_alias = "m31";}
  ScanOutPort   MEM30_BIST_COLLAR_SI { Source MEM30_BIST_COLLAR_SI_INT;}
  ScanInPort    MEM30_BIST_COLLAR_SO;
  ScanInterface Client {
    Port BIST_SI;
    Port MBISTPG_SO;
    Port BIST_HOLD;
  }
  ScanInterface MEM0_INTERFACE {
    Port BIST_SHIFT_COLLAR;
    Port MEM0_BIST_COLLAR_SI;
    Port MEM0_BIST_COLLAR_SO;
  }
  ScanInterface MEM1_INTERFACE {
    Port BIST_SHIFT_COLLAR;
    Port MEM1_BIST_COLLAR_SI;
    Port MEM1_BIST_COLLAR_SO;
  }
  ScanInterface MEM2_INTERFACE {
    Port BIST_SHIFT_COLLAR;
    Port MEM2_BIST_COLLAR_SI;
    Port MEM2_BIST_COLLAR_SO;
  }
  ScanInterface MEM3_INTERFACE {
    Port BIST_SHIFT_COLLAR;
    Port MEM3_BIST_COLLAR_SI;
    Port MEM3_BIST_COLLAR_SO;
  }
  ScanInterface MEM4_INTERFACE {
    Port BIST_SHIFT_COLLAR;
    Port MEM4_BIST_COLLAR_SI;
    Port MEM4_BIST_COLLAR_SO;
  }
  ScanInterface MEM5_INTERFACE {
    Port BIST_SHIFT_COLLAR;
    Port MEM5_BIST_COLLAR_SI;
    Port MEM5_BIST_COLLAR_SO;
  }
  ScanInterface MEM6_INTERFACE {
    Port BIST_SHIFT_COLLAR;
    Port MEM6_BIST_COLLAR_SI;
    Port MEM6_BIST_COLLAR_SO;
  }
  ScanInterface MEM7_INTERFACE {
    Port BIST_SHIFT_COLLAR;
    Port MEM7_BIST_COLLAR_SI;
    Port MEM7_BIST_COLLAR_SO;
  }
  ScanInterface MEM8_INTERFACE {
    Port BIST_SHIFT_COLLAR;
    Port MEM8_BIST_COLLAR_SI;
    Port MEM8_BIST_COLLAR_SO;
  }
  ScanInterface MEM9_INTERFACE {
    Port BIST_SHIFT_COLLAR;
    Port MEM9_BIST_COLLAR_SI;
    Port MEM9_BIST_COLLAR_SO;
  }
  ScanInterface MEM10_INTERFACE {
    Port BIST_SHIFT_COLLAR;
    Port MEM10_BIST_COLLAR_SI;
    Port MEM10_BIST_COLLAR_SO;
  }
  ScanInterface MEM11_INTERFACE {
    Port BIST_SHIFT_COLLAR;
    Port MEM11_BIST_COLLAR_SI;
    Port MEM11_BIST_COLLAR_SO;
  }
  ScanInterface MEM12_INTERFACE {
    Port BIST_SHIFT_COLLAR;
    Port MEM12_BIST_COLLAR_SI;
    Port MEM12_BIST_COLLAR_SO;
  }
  ScanInterface MEM13_INTERFACE {
    Port BIST_SHIFT_COLLAR;
    Port MEM13_BIST_COLLAR_SI;
    Port MEM13_BIST_COLLAR_SO;
  }
  ScanInterface MEM14_INTERFACE {
    Port BIST_SHIFT_COLLAR;
    Port MEM14_BIST_COLLAR_SI;
    Port MEM14_BIST_COLLAR_SO;
  }
  ScanInterface MEM15_INTERFACE {
    Port BIST_SHIFT_COLLAR;
    Port MEM15_BIST_COLLAR_SI;
    Port MEM15_BIST_COLLAR_SO;
  }
  ScanInterface MEM16_INTERFACE {
    Port BIST_SHIFT_COLLAR;
    Port MEM16_BIST_COLLAR_SI;
    Port MEM16_BIST_COLLAR_SO;
  }
  ScanInterface MEM17_INTERFACE {
    Port BIST_SHIFT_COLLAR;
    Port MEM17_BIST_COLLAR_SI;
    Port MEM17_BIST_COLLAR_SO;
  }
  ScanInterface MEM18_INTERFACE {
    Port BIST_SHIFT_COLLAR;
    Port MEM18_BIST_COLLAR_SI;
    Port MEM18_BIST_COLLAR_SO;
  }
  ScanInterface MEM19_INTERFACE {
    Port BIST_SHIFT_COLLAR;
    Port MEM19_BIST_COLLAR_SI;
    Port MEM19_BIST_COLLAR_SO;
  }
  ScanInterface MEM20_INTERFACE {
    Port BIST_SHIFT_COLLAR;
    Port MEM20_BIST_COLLAR_SI;
    Port MEM20_BIST_COLLAR_SO;
  }
  ScanInterface MEM21_INTERFACE {
    Port BIST_SHIFT_COLLAR;
    Port MEM21_BIST_COLLAR_SI;
    Port MEM21_BIST_COLLAR_SO;
  }
  ScanInterface MEM22_INTERFACE {
    Port BIST_SHIFT_COLLAR;
    Port MEM22_BIST_COLLAR_SI;
    Port MEM22_BIST_COLLAR_SO;
  }
  ScanInterface MEM23_INTERFACE {
    Port BIST_SHIFT_COLLAR;
    Port MEM23_BIST_COLLAR_SI;
    Port MEM23_BIST_COLLAR_SO;
  }
  ScanInterface MEM24_INTERFACE {
    Port BIST_SHIFT_COLLAR;
    Port MEM24_BIST_COLLAR_SI;
    Port MEM24_BIST_COLLAR_SO;
  }
  ScanInterface MEM25_INTERFACE {
    Port BIST_SHIFT_COLLAR;
    Port MEM25_BIST_COLLAR_SI;
    Port MEM25_BIST_COLLAR_SO;
  }
  ScanInterface MEM26_INTERFACE {
    Port BIST_SHIFT_COLLAR;
    Port MEM26_BIST_COLLAR_SI;
    Port MEM26_BIST_COLLAR_SO;
  }
  ScanInterface MEM27_INTERFACE {
    Port BIST_SHIFT_COLLAR;
    Port MEM27_BIST_COLLAR_SI;
    Port MEM27_BIST_COLLAR_SO;
  }
  ScanInterface MEM28_INTERFACE {
    Port BIST_SHIFT_COLLAR;
    Port MEM28_BIST_COLLAR_SI;
    Port MEM28_BIST_COLLAR_SO;
  }
  ScanInterface MEM29_INTERFACE {
    Port BIST_SHIFT_COLLAR;
    Port MEM29_BIST_COLLAR_SI;
    Port MEM29_BIST_COLLAR_SO;
  }
  ScanInterface MEM30_INTERFACE {
    Port BIST_SHIFT_COLLAR;
    Port MEM30_BIST_COLLAR_SI;
    Port MEM30_BIST_COLLAR_SO;
  }
  Alias        SETUP_MODE[2:0] = BIST_SETUP2,BIST_SETUP[1:0] { RefEnum SetupModes; }
  Alias        RUN_MODE[2:0]   = BIST_SETUP2,BIST_SETUP[1:0] { RefEnum RunModes; }
  Enum         PassFail {
                    Pass = 1'b1;
                    Fail = 1'b0;
                    Ignore = 1'bx;
               }
  Enum         AsyncResetN {
                    On = 1'b0;
                    Off = 1'b1;
               }
  Enum         SetupModes {
                    Short = 3'b000;
                    Long  = 3'b001;
               }
  Enum         OnOff {
                    On  = 1'b1;
                    Off = 1'b0;
               }
  Enum         RunModes   {
                    HWDefault   = 3'b010;
                    RunTimeProg = 3'b011;
                    Idle        = 3'bx0x;
                    Off         = 3'bxxx;
               }
  LogicSignal  LONG_SETUP  { MBISTPG_EN,SETUP_MODE[2:0] == 1'b1,3'b001;}
  LogicSignal  SHORT_SETUP { MBISTPG_EN,SETUP_MODE[2:0] == 1'b1,3'b000;}
 
// [start] : LONG_SETUP / SHORT_SETUP chain registers {{{
  ScanRegister BIST_SI_Pipeline {
      ScanInSource   BIST_SI;
      ResetValue     1'b0;
  }
  ScanRegister DIAG_EN_REG[0:0] {
      ScanInSource   BIST_SI_Pipeline;
  }
  ScanRegister CMP_EN_MASK_EN[0:0] {
      ScanInSource   DIAG_EN_REG[0];
  }
  ScanRegister CMP_EN_PARITY[0:0] {
      CaptureSource   1'bx;
      ScanInSource   CMP_EN_MASK_EN[0];
  }
  ScanRegister MEM_SELECT_REG0[0:0] {
      ResetValue     1'd1;
      ScanInSource   CMP_EN_PARITY[0];
  }
  ScanRegister MEM_SELECT_REG1[0:0] {
      ResetValue     1'd1;
      ScanInSource   MEM_SELECT_REG0[0];
  }
  ScanRegister MEM_SELECT_REG2[0:0] {
      ResetValue     1'd1;
      ScanInSource   MEM_SELECT_REG1[0];
  }
  ScanRegister MEM_SELECT_REG3[0:0] {
      ResetValue     1'd1;
      ScanInSource   MEM_SELECT_REG2[0];
  }
  ScanRegister MEM_SELECT_REG4[0:0] {
      ResetValue     1'd1;
      ScanInSource   MEM_SELECT_REG3[0];
  }
  ScanRegister MEM_SELECT_REG5[0:0] {
      ResetValue     1'd1;
      ScanInSource   MEM_SELECT_REG4[0];
  }
  ScanRegister MEM_SELECT_REG6[0:0] {
      ResetValue     1'd1;
      ScanInSource   MEM_SELECT_REG5[0];
  }
  ScanRegister MEM_SELECT_REG7[0:0] {
      ResetValue     1'd1;
      ScanInSource   MEM_SELECT_REG6[0];
  }
  ScanRegister MEM_SELECT_REG8[0:0] {
      ResetValue     1'd1;
      ScanInSource   MEM_SELECT_REG7[0];
  }
  ScanRegister MEM_SELECT_REG9[0:0] {
      ResetValue     1'd1;
      ScanInSource   MEM_SELECT_REG8[0];
  }
  ScanRegister MEM_SELECT_REG10[0:0] {
      ResetValue     1'd1;
      ScanInSource   MEM_SELECT_REG9[0];
  }
  ScanRegister MEM_SELECT_REG11[0:0] {
      ResetValue     1'd1;
      ScanInSource   MEM_SELECT_REG10[0];
  }
  ScanRegister MEM_SELECT_REG12[0:0] {
      ResetValue     1'd1;
      ScanInSource   MEM_SELECT_REG11[0];
  }
  ScanRegister MEM_SELECT_REG13[0:0] {
      ResetValue     1'd1;
      ScanInSource   MEM_SELECT_REG12[0];
  }
  ScanRegister MEM_SELECT_REG14[0:0] {
      ResetValue     1'd1;
      ScanInSource   MEM_SELECT_REG13[0];
  }
  ScanRegister MEM_SELECT_REG15[0:0] {
      ResetValue     1'd1;
      ScanInSource   MEM_SELECT_REG14[0];
  }
  ScanRegister MEM_SELECT_REG16[0:0] {
      ResetValue     1'd1;
      ScanInSource   MEM_SELECT_REG15[0];
  }
  ScanRegister MEM_SELECT_REG17[0:0] {
      ResetValue     1'd1;
      ScanInSource   MEM_SELECT_REG16[0];
  }
  ScanRegister MEM_SELECT_REG18[0:0] {
      ResetValue     1'd1;
      ScanInSource   MEM_SELECT_REG17[0];
  }
  ScanRegister MEM_SELECT_REG19[0:0] {
      ResetValue     1'd1;
      ScanInSource   MEM_SELECT_REG18[0];
  }
  ScanRegister MEM_SELECT_REG20[0:0] {
      ResetValue     1'd1;
      ScanInSource   MEM_SELECT_REG19[0];
  }
  ScanRegister MEM_SELECT_REG21[0:0] {
      ResetValue     1'd1;
      ScanInSource   MEM_SELECT_REG20[0];
  }
  ScanRegister MEM_SELECT_REG22[0:0] {
      ResetValue     1'd1;
      ScanInSource   MEM_SELECT_REG21[0];
  }
  ScanRegister MEM_SELECT_REG23[0:0] {
      ResetValue     1'd1;
      ScanInSource   MEM_SELECT_REG22[0];
  }
  ScanRegister MEM_SELECT_REG24[0:0] {
      ResetValue     1'd1;
      ScanInSource   MEM_SELECT_REG23[0];
  }
  ScanRegister ALGO_SEL_CNT_REG[0:0] {
      ScanInSource   MEM_SELECT_REG24[0];
      RefEnum        OnOff;
  }
  ScanRegister SELECT_COMMON_OPSET_REG[0:0] {
      ScanInSource   ALGO_SEL_CNT_REG[0];
      RefEnum        OnOff;
  }
  ScanRegister SELECT_COMMON_DATA_PAT_REG[0:0] {
      ScanInSource   SELECT_COMMON_OPSET_REG[0];
      RefEnum        OnOff;
  }
  ScanRegister MICROCODE_EN_REG[0:0] {
      ScanInSource   SELECT_COMMON_DATA_PAT_REG[0];
  }
  ScanRegister INST_POINTER_REG_HW[0:4] {
      ScanInSource   MICROCODE_EN_REG[0];
  }
  Alias INST_POINTER_REG[4:0] = INST_POINTER_REG_HW[4:0];
  ScanRegister A_ADD_REG_X_HW[0:6] {
      ScanInSource   INST_POINTER_REG_HW[4];
  }
  Alias A_ADD_REG_X[6:0] = A_ADD_REG_X_HW[6:0];
  ScanRegister B_ADD_REG_X_HW[0:6] {
      ScanInSource   A_ADD_REG_X_HW[6];
  }
  Alias B_ADD_REG_X[6:0] = B_ADD_REG_X_HW[6:0];
  ScanRegister JCNT_HW[0:2] {
      ScanInSource   B_ADD_REG_X_HW[6];
  }
  Alias JCNT[2:0] = JCNT_HW[2:0];
  ScanRegister OPSET_SELECT_REG[0:0] {
      ScanInSource   JCNT_HW[2];
  }
  ScanRegister WDATA_REG_HW[0:1] {
      ScanInSource   OPSET_SELECT_REG[0];
  }
  Alias WDATA_REG[1:0] = WDATA_REG_HW[1:0];
  ScanRegister EDATA_REG_HW[0:1] {
      ScanInSource   WDATA_REG_HW[1];
  }
  Alias EDATA_REG[1:0] = EDATA_REG_HW[1:0];
  ScanRegister X_ADDR_BIT_SEL_REG[0:0] {
      ScanInSource   EDATA_REG_HW[1];
  }
  ScanRegister REPEATLOOP_A_CNTR_REG_HW[0:1] {
      ScanInSource   X_ADDR_BIT_SEL_REG[0];
  }
  Alias REPEATLOOP_A_CNTR_REG[1:0] = REPEATLOOP_A_CNTR_REG_HW[1:0];
  ScanRegister REPEATLOOP_B_CNTR_REG_HW[0:1] {
      ScanInSource   REPEATLOOP_A_CNTR_REG_HW[1];
  }
  Alias REPEATLOOP_B_CNTR_REG[1:0] = REPEATLOOP_B_CNTR_REG_HW[1:0];
  Alias MEM30_BIST_COLLAR_SI_INT = REPEATLOOP_B_CNTR_REG_HW[1];
  Alias MEM29_BIST_COLLAR_SI_INT = MEM30_BIST_COLLAR_SO;
  Alias MEM28_BIST_COLLAR_SI_INT = MEM29_BIST_COLLAR_SO;
  Alias MEM27_BIST_COLLAR_SI_INT = MEM28_BIST_COLLAR_SO;
  Alias MEM26_BIST_COLLAR_SI_INT = MEM27_BIST_COLLAR_SO;
  Alias MEM25_BIST_COLLAR_SI_INT = MEM26_BIST_COLLAR_SO;
  Alias MEM24_BIST_COLLAR_SI_INT = MEM25_BIST_COLLAR_SO;
  Alias MEM23_BIST_COLLAR_SI_INT = MEM24_BIST_COLLAR_SO;
  Alias MEM22_BIST_COLLAR_SI_INT = MEM23_BIST_COLLAR_SO;
  Alias MEM21_BIST_COLLAR_SI_INT = MEM22_BIST_COLLAR_SO;
  Alias MEM20_BIST_COLLAR_SI_INT = MEM21_BIST_COLLAR_SO;
  Alias MEM19_BIST_COLLAR_SI_INT = MEM20_BIST_COLLAR_SO;
  Alias MEM18_BIST_COLLAR_SI_INT = MEM19_BIST_COLLAR_SO;
  Alias MEM17_BIST_COLLAR_SI_INT = MEM18_BIST_COLLAR_SO;
  Alias MEM16_BIST_COLLAR_SI_INT = MEM17_BIST_COLLAR_SO;
  Alias MEM15_BIST_COLLAR_SI_INT = MEM16_BIST_COLLAR_SO;
  Alias MEM14_BIST_COLLAR_SI_INT = MEM15_BIST_COLLAR_SO;
  Alias MEM13_BIST_COLLAR_SI_INT = MEM14_BIST_COLLAR_SO;
  Alias MEM12_BIST_COLLAR_SI_INT = MEM13_BIST_COLLAR_SO;
  Alias MEM11_BIST_COLLAR_SI_INT = MEM12_BIST_COLLAR_SO;
  Alias MEM10_BIST_COLLAR_SI_INT = MEM11_BIST_COLLAR_SO;
  Alias MEM9_BIST_COLLAR_SI_INT = MEM10_BIST_COLLAR_SO;
  Alias MEM8_BIST_COLLAR_SI_INT = MEM9_BIST_COLLAR_SO;
  Alias MEM7_BIST_COLLAR_SI_INT = MEM8_BIST_COLLAR_SO;
  Alias MEM6_BIST_COLLAR_SI_INT = MEM7_BIST_COLLAR_SO;
  Alias MEM5_BIST_COLLAR_SI_INT = MEM6_BIST_COLLAR_SO;
  Alias MEM4_BIST_COLLAR_SI_INT = MEM5_BIST_COLLAR_SO;
  Alias MEM3_BIST_COLLAR_SI_INT = MEM4_BIST_COLLAR_SO;
  Alias MEM2_BIST_COLLAR_SI_INT = MEM3_BIST_COLLAR_SO;
  Alias MEM1_BIST_COLLAR_SI_INT = MEM2_BIST_COLLAR_SO;
  Alias MEM0_BIST_COLLAR_SI_INT = MEM1_BIST_COLLAR_SO;
  ScanRegister STOP_ON_ERROR_REG[0:0] {
      ScanInSource   MEM0_BIST_COLLAR_SO;
  }
  ScanRegister FREEZE_STOP_ERROR_REG[0:0] {
      ScanInSource   STOP_ON_ERROR_REG[0];
  }
  ScanRegister STOP_ERROR_CNT_REG_HW[0:11] {
      ScanInSource   FREEZE_STOP_ERROR_REG[0];
  }
  Alias STOP_ERROR_CNT_REG[11:0] = STOP_ERROR_CNT_REG_HW[11:0];
  ScanRegister STEP_COUNTER_FREEZE_REG[0:0] {
      ScanInSource   STOP_ERROR_CNT_REG_HW[11];
      RefEnum        OnOff;
  }
  ScanRegister STEP_COUNTER_REG[0:0] {
      ResetValue     1'd0;
      ScanInSource   STEP_COUNTER_FREEZE_REG[0];
  }
 ScanMux CONTROLLER_SETUP_CHAIN SelectedBy MBISTPG_EN,SETUP_MODE[2:0] {
     1'b1,3'b00x : STEP_COUNTER_REG[0];
 }
// [end]   : LONG_SETUP / SHORT_SETUP chain registers }}}
  ScanMux BIST_SO_OUT SelectedBy LV_TM,MBISTPG_ASYNC_RESETN {
    2'b01 : CONTROLLER_SETUP_CHAIN;
  }
  ScanRegister BIST_SO_Pipeline {
      ScanInSource   BIST_SO_OUT;
      ResetValue     1'b0;
  }
  Alias BIST_SO_INT = BIST_SO_Pipeline;
  Attribute     tessent_instrument_container           = "cpu_top_rtl1_mbist";
  Attribute     tessent_instrument_type                = "mentor::memory_bist";
  Attribute     tessent_instrument_subtype             = "controller";
  Attribute     tessent_signature                      = "32ed6d5712756fb65dc2dba62923a76a";
  Attribute     tessent_ignore_during_icl_verification = "on";
  Attribute     keep_active_during_scan_test           = "false";
  Attribute     tessent_use_in_dft_specification       = "false";
  Attribute     tessent_bist_clk_persistent_cell_output_list     = "tessent_persistent_cell_GATING_BIST_CLK/GCK";
}
