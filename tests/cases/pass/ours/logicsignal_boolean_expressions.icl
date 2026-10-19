Module m {
  LogicSignal a_and_b {
    streaming_through_ijtag_en_shift_reg && enable_shift_reg;
  }
  LogicSignal parenthesised {
    (i_test_mode_ijtag_di[1] && i_test_mode_ijtag_di[0]) || (!i_test_mode_ijtag_di[1] && !i_test_mode_ijtag_di[0]);
  }
  LogicSignal concat_compare {
    enable, sib == 2'b11;
  }
  LogicSignal mixed_and {
    Select_ijtag_sel && (Value == 3'd0);
  }
}
