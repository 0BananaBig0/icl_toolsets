Module ResetCallbackScanCell {
 Parameter W=1; Parameter R=1; ScanInPort si; ResetPort rst;
 ScanRegister value[$W-1:0] { ScanInSource si; CaptureSource value; ResetValue $R; }
 ScanOutPort so { Source value[0]; } DataOutPort q[$W-1:0] { Source value; }
}
Module ResetCallbackDataCell {
 ResetPort rst; DataRegister value { WriteDataSource 1'b1; WriteEnSource 1'b1; ResetValue 0; }
 DataOutPort q { Source value; }
}
Module DataResetCallbacks {
 ScanInPort si;
 Instance a Of ResetCallbackScanCell { InputPort si=si; InputPort rst=z; }
 Instance d Of ResetCallbackDataCell { InputPort rst=a.q; }
 DataRegister z { WriteDataSource 1'b1; WriteEnSource 1'b1; ResetValue 0; }
 LogicSignal rb { z & d.q; }
 Instance b Of ResetCallbackScanCell { Parameter W=4; Parameter R=0; InputPort si=a.so; InputPort rst=rb; }
 ScanOutPort so { Source b.so; }
}
