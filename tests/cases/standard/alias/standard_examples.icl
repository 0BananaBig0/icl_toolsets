// IEEE 1687-2014 section 6.5.2 examples; missing wrappers, registers, ports and hierarchy supplied.
Module Leaf { DataInPort a[2:0]; DataInPort b[1:0]; }
Module Parent { Instance u3 Of Leaf; }
Module M {
 Instance u1 Of Leaf; Instance u2 Of Parent;
 DataInPort a[1:0]; DataInPort C; ScanInPort si;
 ScanRegister TDR1[63:0] { ScanInSource si; ResetValue 64'b0; }
 DataInPort ADDR[7:0]; DataInPort DATA[31:0]; DataInPort CMD[2:0];
 Enum T1 { ZERO = 2'b00; ONE = 2'b01; }
 Alias MyName1[2:0] = u1.a[2], u2.u3.b[1:0];
 Alias MyName2[1:0] = a[1], C { RefEnum T1; }
 Alias RE = TDR1[63] { iApplyEndState 1'b0; }
 Alias CNTL[42:0] = ADDR[7:0], DATA[31:0], CMD[2:0] { AccessTogether; }
}
