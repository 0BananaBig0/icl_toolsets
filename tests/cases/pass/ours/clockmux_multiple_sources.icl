Module m {
  ClockMux cm SelectedBy sel {
    1'b0 : clk_a;
    1'b1 : clk_b;
  }
}
