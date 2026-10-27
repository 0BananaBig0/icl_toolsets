Module ResourceNetwork {
  ScanInPort si;
  ScanRegister gate[1:0] { ScanInSource si; ResetValue 2'b00; }
  Alias gate_alias[1:0] = gate;
  ScanRegister target[3:0] { ScanInSource gate[0]; ResetValue 4'b0; }
  ScanMux route SelectedBy gate[0] { 1'b0: gate[0]; 1'b1: target[0]; }
  ScanOutPort so { Source route; }
}
