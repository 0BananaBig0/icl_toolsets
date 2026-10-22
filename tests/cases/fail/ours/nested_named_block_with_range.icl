Module m {
  Reg reg_ctrl {
    Field f[3:0] {
      Width 4;
    }
    Field g {
      Width 2;
    }
  }
  ScanInterface si {
    Chain c {
      Port p;
      DefaultLoadValue 8'b1010;
    }
  }
}
