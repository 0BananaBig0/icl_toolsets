Module Example { Enum enum1 { zero = 1'b0; } DataInPort DI; DataInPort CNTL[7:0]; DataInPort ABC { RefEnum enum1; } DataInPort D[7:0] { DefaultLoadValue 8'b01011010; } }
