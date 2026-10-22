UseNameSpace lib;
Module Top {
  Parameter Width = 8;
  DataInPort input[$Width-1:0];
  Instance child Of Leaf { InputPort input = input; Parameter Width = $Width; }
  Alias readback = child.output;
  DataOutPort output { Source readback; }
  Enum Mode { OFF = 1'b0; ON = 1'b1; }
  DataInPort mode { RefEnum Mode; }
  LogicSignal enabled { mode == ON; }
}
Module Other { DataInPort input; DataOutPort out { Source input; } }
