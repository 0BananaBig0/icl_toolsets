// Self-constructed IEEE 1687-2014 7.9.10(c)/7.9.11(c) operand regression.
Module EnumChild {
  Parameter ACTIVE = 4'b0101;
  Enum states {
    Active = $ACTIVE;
    Clear = 4'b0000;
    Mask = 4'bxx01;
    AllX = 4'bxxxx;
  }
  Enum inverted { Active = 4'b1010; }
  ScanInPort si;
  ScanRegister r[0:3] {
    ScanInSource si;
    CaptureSource r;
    RefEnum states;
  }
  Alias complement[3:0] = ~r { RefEnum inverted; }
  Alias unnamed[3:0] = r;
  ScanOutPort so { Source r[3]; }
}
Module EnumTop {
  ScanInPort si;
  Instance a Of EnumChild { InputPort si = si; }
  Instance b Of EnumChild {
    InputPort si = a.so;
    Parameter ACTIVE = 4'b1010;
  }
  ScanOutPort so { Source b.so; }
}
