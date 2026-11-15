Module Child {
  ScanInPort si; SelectPort sel; CaptureEnPort ce;
  ShiftEnPort se; UpdateEnPort ue; TCKPort ck;
  ScanRegister value[3:0] { ScanInSource si; ResetValue 4'b0; }
  ScanOutPort so { Source value[0]; }
  ScanInterface client {
    Port si; Port so; Port sel; Port ce; Port se; Port ue; Port ck;
  }
}
Module ClientTop {
  ScanInPort si; SelectPort sel; CaptureEnPort ce;
  ShiftEnPort se; UpdateEnPort ue; TCKPort ck;
  DataInPort enable; DataInPort update_enable; DataMux update_gate SelectedBy update_enable { 1'b0: 1'b0; 1'b1: ue; }
  Instance child Of Child {
    InputPort si = si; InputPort sel = sel;
    InputPort ce = ce; InputPort se = se; InputPort ue = update_gate; InputPort ck = ck;
  }
  ScanOutPort so { Source child.so; }
  ScanInterface client {
    Port si; Port so; Port sel; Port ce; Port se; Port ue; Port ck;
  }
}
