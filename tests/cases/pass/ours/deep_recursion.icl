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
  ScanInterface iface { Chain c { Port si; Attribute nested = "chain"; } }
}
