Module m {
  ScanInterface Par_client2 {
    Port Sel;
    Port SE;
    Chain c1 {
      Port SI1A;
      Port SO1A;
      DefaultLoadValue 8'b11011010;
    }
    Chain c2 {
      Port SI1B;
      Port SO1B;
      DefaultLoadValue 6'b00100101;
    }
  }
}
