Module m {
  LogicSignal deep_parens {
    (((((a && b) || (c == d)))));
  }
  ScanRegister r {
    Source a[0][1];
    CaptureSource b[(i)];
    ResetValue c[(x + 1)];
  }
  ScanRegister s[a[0]:(b - 1)] {
    Source t[$MSB:0];
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

