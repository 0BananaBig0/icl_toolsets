Module FeedbackResetCell {
 ScanInPort si; ShiftEnPort se; ResetPort rst { ActivePolarity 0; }
 ScanRegister value[3:0] { ScanInSource si; CaptureSource value; ResetValue 0; }
 ScanOutPort so { Source value[0]; } DataOutPort q[3:0] { Source value; }
}
Module FeedbackReset {
 ScanInPort si; ShiftEnPort se; CaptureEnPort ce; UpdateEnPort ue; TCKPort ck; ResetPort rst { ActivePolarity 0; }
 DataMux reset_gate SelectedBy c.q[0] { 1'b0: rst; 1'b1: 1'b0; }
 Instance c Of FeedbackResetCell { InputPort si = si; InputPort se = se; InputPort rst = reset_gate; }
 ScanOutPort so { Source c.so; }
 ScanInterface bus { Port si; Port so; Port se; Port ce; Port ue; Port ck; }
}
