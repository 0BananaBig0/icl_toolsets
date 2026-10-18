Module m {
  ScanMux c1 SelectedBy MBISTPG_EN, SETUP_MODE[2:0] {
    1'b1, 3'b00x : STEP_COUNTER_REG[0];
  }
  ScanMux c2 SelectedBy config_sib_shift_reg, enable_shift_reg {
    Attribute exclude_codes_from_icl_verify = "{2'b01}";
    2'b10 : packet_width_shift_reg;
  }
}
