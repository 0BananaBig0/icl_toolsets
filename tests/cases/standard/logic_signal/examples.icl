// IEEE §6.4.10 examples; supplied inputs and separate wrappers for repeated ctl name.
Module A { DataInPort e1[1:0]; DataInPort e2[1:0]; LogicSignal ctl { e1 == 2'b10 || e2 == 2'b11; } }
Module B { DataInPort e1[1:0]; DataInPort e2[1:0]; LogicSignal ctl { e1[1] & ~e1[0] | e2[1] & e2[0]; } }
