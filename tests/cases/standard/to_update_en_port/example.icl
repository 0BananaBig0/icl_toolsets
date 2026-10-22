// IEEE 1687-2014 §6.4.6.10 printed 91–92/PDF 105–106; completed example.
Module Module_A { DataInPort upd_dis; UpdateEnPort UE; ToUpdateEnPort toUE; ToUpdateEnPort to_UE { Source UE_mux; } DataMux UE_mux SelectedBy upd_dis { 1'b0 : UE; 1'b1 : 1'b0; } }
