Module m {
  Alias MyName1[2:0] = u1.a[2], u2.u3.b[1:0];
  Alias CNTL[42:0] = ADDR[7:0], ~DATA[31:0], CMD[2:0] { AccessTogether; }
  Alias RE = TDR1[63] { iApplyEndState 1'b0; }
  Alias MyName2[1:0] = a[1], C { RefEnum T1; }
}
