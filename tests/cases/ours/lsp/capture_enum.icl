// IEEE 1687-2014 6.4.8(t): Enum takes precedence over a same-named signal.
Module Capture {
  ScanInPort si;
  Enum Modes { ON = 1'b1; }
  DataInPort ON;
  DataInPort input;
  Instance child Of lib::Leaf;
  ScanRegister state { ScanInSource si; CaptureSource ON; RefEnum Modes; }
  ScanRegister fallback { ScanInSource si; RefEnum Modes; CaptureSource input; }
  ScanRegister plain { ScanInSource si; CaptureSource ON; }
  ScanRegister indexed { ScanInSource si; RefEnum Modes; CaptureSource ON[0]; }
  ScanRegister concat[1:0] { ScanInSource si; RefEnum Modes; CaptureSource ON,input; }
  ScanRegister hierarchical { ScanInSource si; RefEnum Modes; CaptureSource child.output; }
}
