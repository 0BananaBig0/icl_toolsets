// Leaf and ResetStage come from nested.icl. This variant has a real TAP
// interface so Tessent can export patterns for the reset-release sequence.
Module ResetAccessNetwork {
  ScanInPort si;
  SelectPort sel;
  CaptureEnPort capture;
  ShiftEnPort shift;
  UpdateEnPort update;
  TCKPort tck;
  ResetPort rst;
  ScanRegister gate { ScanInSource si; ResetValue 1'b0; }
  ScanRegister root { ScanInSource gate; ResetValue 1'b0; }
  DataMux branch_reset SelectedBy root { 1'b0: 1'b1; 1'b1: rst; }
  Instance branch Of ResetStage { InputPort si = root; InputPort rst = branch_reset; }
  Instance a Of Leaf { InputPort si = branch.so; InputPort rst = branch.reset_out; }
  Instance b Of Leaf { InputPort si = a.so; InputPort rst = branch.reset_out; }
  ScanMux route SelectedBy gate { 1'b0: branch.so; 1'b1: b.so; }
  ScanOutPort so { Source route; }
  ScanInterface client {
    Port si; Port so; Port sel; Port capture; Port shift; Port update; Port tck;
  }
}
Module ResetAccessTop {
  TRSTPort TRST;
  Instance core Of ResetAccessNetwork { InputPort rst = ~TRST; }
  AccessLink tap Of STD_1149_1_2001 {
    BSDLEntity AccessProbe;
    IJTAG_ON { ScanInterface { core.client; } }
    IJTAG_OFF { ScanInterface { core.client; } }
  }
}
