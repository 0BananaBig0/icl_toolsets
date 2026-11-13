Module DataResetProbeCell {
 ScanInPort si; ShiftEnPort se; ResetPort rst { ActivePolarity 0; }
 ScanRegister value[3:0] { ScanInSource si; CaptureSource value; ResetValue 0; }
 ScanOutPort so { Source value[0]; }
}
Module DataResetProbe {
 ScanInPort si; ShiftEnPort se; CaptureEnPort ce; UpdateEnPort ue; TCKPort ck; ResetPort rst { ActivePolarity 0; } DataInPort data; DataInPort enable;
 DataRegister release { WriteDataSource data; WriteEnSource enable; ResetValue 0; }
 DataMux reset_gate SelectedBy release { 1'b0: 1'b0; 1'b1: rst; }
 Instance c Of DataResetProbeCell { InputPort si = si; InputPort se = se; InputPort rst = reset_gate; }
 ScanOutPort so { Source c.so; }
 ScanInterface bus { Port si; Port so; Port se; Port ce; Port ue; Port ck; }
}
