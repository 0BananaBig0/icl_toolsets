Module Leaf {
  ScanInPort si;
  ScanInterface iface { Port si; }
  ScanInterface second { Port si; }
}
Module Review {
  Parameter V = 1;
  ScanRegister R { ResetValue $V; }
  DataInPort p { DefaultLoadValue $V; }
  LogicSignal compare { p == $V; }
  DataInPort Module;
  DataInPort Of;
  DataInPort SelectedBy;
  LogicSignal keyword { Module; }
  Alias aliasOf = Of;
  Alias aliasSelected = SelectedBy;
  Enum E { Module = 1; NameSpace = 0; UseNameSpace = 1; }
  DataInPort mode { RefEnum E; DefaultLoadValue Module; }
  DataInPort space { RefEnum E; DefaultLoadValue NameSpace; }
  DataInPort useSpace { RefEnum E; DefaultLoadValue UseNameSpace; }
  ScanInPort ti;
  ScanInterface iface { Port ti; }
  ScanInterface second { Port ti; }
  Instance i Of ::Leaf { AllowBroadcastOnScanInterface iface, second; }
  ResetPort reset;
  AccessLink tap Of STD_1149_1_2001 {
    BSDLEntity entity;
    Module { ActiveSignals { reset; Module; } }
  }
}
