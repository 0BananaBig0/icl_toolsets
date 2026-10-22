// IEEE 1687-2014 section 6.4.15: complete module types/ports and disambiguate duplicated A/B names.
Module M1 {
 DataInPort MSBS[3:0]; DataInPort LSBS[3:0];
 AddressPort addr[7:0]; WriteEnPort we; ReadEnPort re;
 OneHotDataGroup M1[7:0] {
  Port M2_I1.A; Port M2_I1.B;
  Instance IA Of Mod1 { AddressValue 8'h00; }
  Instance IB Of Mod1 { AddressValue 8'h10; }
  DataRegister A[7:0] { AddressValue 8'h20; }
  DataRegister B[7:0] { AddressValue 8'h21; }
 }
 Instance M2_I1 Of M2;
}
Module M2 {
 DataInPort Reg1[7:0]; DataInPort En1;
 DataOutPort A[7:0] { Source Reg1; Enable En1; }
 DataOutPort B[7:0] { Source Reg1; Enable En1; }
}
Module Mod1 {
 DataInPort data[7:0]; AddressPort addr[7:0]; WriteEnPort we; ReadEnPort re;
 DataOutPort out[7:0] { Source regs; }
 OneHotDataGroup regs[7:0] { DataRegister R[7:0] { AddressValue 0; } }
}
