// IEEE 1687-2014 §6.4.6.9 Figure 42: removed ellipses, normalized apostrophes.
Module Module_A {
 DataInPort cap_dis;
 CaptureEnPort CE;
 ToCaptureEnPort to_CE { Source CE_mux; }
 DataMux CE_mux SelectedBy cap_dis { 1'b0 : CE; 1'b1 : 1'b0; }
}
