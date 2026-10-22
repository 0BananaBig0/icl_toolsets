Module Enums {
  Enum First { OFF = 1'b0; ON = 1'b1; }
  Enum Second { OFF = 2'b00; ON = 2'b11; }
  DataInPort mode { RefEnum First; DefaultLoadValue ON; }
  DataInPort other[1:0] { RefEnum Second; DefaultLoadValue ON; }
  LogicSignal enabled { mode == ON; }
  LogicSignal other_enabled { other == ON; }
}
