Module DataResetCell {
 ScanInPort si; ResetPort rst { ActivePolarity 0; }
 ScanRegister value[3:0] { ScanInSource si; CaptureSource value; ResetValue 0; }
 ScanOutPort so { Source value[0]; }
}
Module DataResetNetwork {
 ScanInPort si; DataInPort data; DataInPort enable; ResetPort global { ActivePolarity 0; }
 DataRegister release { WriteDataSource data; WriteEnSource enable; ResetValue 0; }
 DataMux reset_gate SelectedBy release { 1'b0: 1'b0; 1'b1: global; }
 Instance c Of DataResetCell { InputPort si = si; InputPort rst = reset_gate; }
 ScanOutPort so { Source c.so; }
}
