Module m {
  LogicSignal logic_0 { reg8.SR[1:0] == 2'b10 || C == 2'b11; }
  LogicSignal logic_1 { |(~(reg8.SR[0]), reg8.SR[1]); }
  LogicSignal logic_3 { reg8.SR[1] & ~reg8.SR[0] | reg8.SR[1] & reg8.SR[0]; }
  LogicSignal logic_5 { (reg8.SR[1:0] & ~reg8.SR[1:0]) == 3; }
}
