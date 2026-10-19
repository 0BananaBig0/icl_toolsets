// The four Alias examples in IEEE 1687-2014 clause 6.5.2, verbatim. Each is
// given in the standard alone, referring to signals defined nowhere; the
// definitions below make each one a complete module, so a formatter has to
// cope with them as written rather than as paraphrase.
//
// The standard also states rules the examples do not show — for instance that
// the elements of an alias must name a DataInPort, DataOutPort, ScanRegister
// or DataRegister — and those are what the definitions here supply.

// Example 1: `Alias MyName1[2:0] = u1.a[2], u2.u3.b[1:0];`
Module AliasExample1 {
  Instance u1 Of Leaf1 { InputPort a = a_src; }
  Instance u2 Of Leaf2 { InputPort b = b_src; }
  DataInPort a_src;
  DataInPort b_src[1:0];
  Alias MyName1[2:0] = u1.a[2], u2.u3.b[1:0];
}

// Example 2: `Alias MyName2[1:0] = a[1], C { RefEnum T1; }`
Module AliasExample2 {
  DataInPort a[1:0];
  DataInPort C;
  Enum T1 { ZERO = 2'b00; ONE = 2'b01; }
  Alias MyName2[1:0] = a[1], C { RefEnum T1; }
}

// Example 3: `Alias RE = TDR1[63] { iApplyEndState 1'b0; }`
// The standard explains this one: it clears a read-trigger bit at the end of
// the iApply time frame, so the next transaction's rising edge is visible.
Module AliasExample3 {
  ScanRegister TDR1[63:0] { ScanInSource si; }
  ScanInPort si;
  Alias RE = TDR1[63] { iApplyEndState 1'b0; }
}

// Example 4: `Alias CNTL[42:0] = ADDR[7:0], DATA[31:0], CMD[2:0] {
//               AccessTogether; }`
// ADDR, DATA and CMD are DataInPorts; AccessTogether tells the retargeter to
// update all three in one shift.
Module AliasExample4 {
  DataInPort ADDR[7:0];
  DataInPort DATA[31:0];
  DataInPort CMD[2:0];
  Alias CNTL[42:0] = ADDR[7:0], DATA[31:0], CMD[2:0] { AccessTogether; }
}
