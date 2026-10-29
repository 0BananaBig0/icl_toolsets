Module OneHotBranch {
  ScanInPort si;
  DataInPort enabled;
  ScanRegister data[3:0] { ScanInSource si; ResetValue 4'b0; }
  ScanOutPort so { Source data[0]; Enable enabled; }
}
Module OneHotReturns {
  ScanInPort si;
  DataInPort a_enabled;
  DataInPort b_enabled;
  Instance a Of OneHotBranch {
    InputPort si = si;
    InputPort enabled = a_enabled;
  }
  Instance b Of OneHotBranch {
    InputPort si = si;
    InputPort enabled = b_enabled;
  }
  OneHotScanGroup returns { Port a.so; Port b.so; }
  ScanOutPort so { Source returns; }
}
Module OneHotTop {
  ScanInPort si;
  LogicSignal a_enabled { select == 2'b01; }
  LogicSignal b_enabled { select == 2'b10; }
  LogicSignal bypass_enabled { select == 2'b00; }
  Instance branches Of OneHotReturns {
    InputPort si = si;
    InputPort a_enabled = a_enabled;
    InputPort b_enabled = b_enabled;
  }
  Instance bypass Of OneHotBypass {
    InputPort si = si;
    InputPort enabled = bypass_enabled;
  }
  OneHotScanGroup route { Port branches.so; Port bypass.so; }
  ScanRegister select[1:0] { ScanInSource route; ResetValue 2'b00; }
  ScanOutPort so { Source select[0]; }
}
Module OneHotBypass {
  ScanInPort si;
  DataInPort enabled;
  ScanOutPort so { Source si; Enable enabled; }
}
