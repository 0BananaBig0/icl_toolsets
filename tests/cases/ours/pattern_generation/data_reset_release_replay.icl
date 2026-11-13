Module Child {
 ScanInPort si; ResetPort rst { ActivePolarity 0; }
 ScanRegister value[3:0] { ScanInSource si; CaptureSource value; ResetValue 0; }
 ScanOutPort so { Source value[0]; }
}
Module DataCell {
 ResetPort rst { ActivePolarity 0; } DataInPort data; DataInPort enable;
 DataRegister release { WriteDataSource data; WriteEnSource enable; ResetValue 0; }
 DataOutPort q { Source release; }
}
Module DataResetReleaseScan {
 ScanInPort si; ScanRegister data { ScanInSource si; ResetValue 0; } ScanRegister enable { ScanInSource data; ResetValue 0; }
 DataRegister release { WriteDataSource data; WriteEnSource enable; ResetValue 0; }
 Instance c Of Child { InputPort si = enable; InputPort rst = release; }
 ScanOutPort so { Source c.so; }
}
Module DataResetReleaseMixed {
 ScanInPort si; DataInPort data; DataInPort enable;
 ScanRegister gate { ScanInSource si; CaptureSource gate; ResetValue 0; }
 Instance d Of DataCell { InputPort rst=gate; InputPort data=data; InputPort enable=enable; }
 Instance c Of Child { InputPort si = gate; InputPort rst = d.q; }
 ScanOutPort so { Source c.so; }
}
