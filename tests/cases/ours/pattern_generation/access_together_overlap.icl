// Self-constructed overlapping AccessTogether read obligations, IEEE 1687 6.5.2.
Module Network {
 ScanInPort si;
 ScanRegister gate { ScanInSource si; }
 ScanRegister a { ScanInSource gate; }
 ScanRegister b { ScanInSource a; }
 ScanRegister c { ScanInSource b; }
 ScanMux route SelectedBy gate { 1'b0: b; 1'b1: c; }
 ScanOutPort so { Source route; }
 Alias first[1:0] = a,b { AccessTogether; }
 Alias second[1:0] = b,c { AccessTogether; }
}
