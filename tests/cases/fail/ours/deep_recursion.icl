Module m {
  LogicSignal deep_parens {
    (((((a && b) || (c == d)))));
  }
  ScanRegister r {
    ScanInSource a[0];
    CaptureSource b[($i)];
    ResetValue ($x + 1);
  }
  ScanRegister s[($a + 1):($b - 1)] {
    ScanInSource t[$MSB];
  }
  Reg outer {
    Field l1 {
      Field l2 {
        Field l3 {
          Width 1;
        }
      }
    }
  }
}
