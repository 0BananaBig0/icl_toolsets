Module DataResetCell {
 ScanInPort si; ResetPort rst { ActivePolarity 0; }
 ScanRegister value[3:0] { ScanInSource si; CaptureSource value; ResetValue 0; }
 ScanOutPort so { Source value[0]; }
}
Module DataResetNetwork {
 ScanInPort si; DataInPort data; DataInPort enable;
 DataRegister release { WriteDataSource data; WriteEnSource enable; ResetValue 0; }
 Instance c Of DataResetCell { InputPort si = si; InputPort rst = release; }
 ScanOutPort so { Source c.so; }
}
