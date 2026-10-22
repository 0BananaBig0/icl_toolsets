NameSpace ;
UseNameSpace ;
Module SIB {
   ResetPort ijtag_reset {
      ActivePolarity 0;
      Attribute connection_rule_option = "allowed_no_source";
   }
   SelectPort ijtag_sel;
   ScanInPort ijtag_si;
   CaptureEnPort ijtag_ce;
   ShiftEnPort ijtag_se;
   UpdateEnPort ijtag_ue;
   TCKPort ijtag_tck;
   ScanOutPort ijtag_so {
      Source sib[0];
   }
   ScanOutPort ijtag_to_si {
      Source ijtag_si;
   }
   ToSelectPort ijtag_to_sel[3:0] {
      Source sel_3, sel_2, sel_1, sel_0;
   }
   ScanInPort ijtag_from_so[3:0];
   ScanInterface client {
      Port ijtag_si;
      Port ijtag_so;
      Port ijtag_sel;
      Port ijtag_tck;
      Port ijtag_reset;
      Port ijtag_se;
      Port ijtag_ce;
      Port ijtag_ue;
   }
   ScanInterface host_0 {
      Port ijtag_to_si;
      Port ijtag_from_so[0];
      Port ijtag_to_sel[0];
   }
   ScanInterface host_1 {
      Port ijtag_to_si;
      Port ijtag_from_so[1];
      Port ijtag_to_sel[1];
   }
   ScanInterface host_2 {
      Port ijtag_to_si;
      Port ijtag_from_so[2];
      Port ijtag_to_sel[2];
   }
   ScanInterface host_3 {
      Port ijtag_to_si;
      Port ijtag_from_so[3];
      Port ijtag_to_sel[3];
   }
   LogicSignal sel_0 {
      ijtag_sel && (sib == 2'b00);
   }
   LogicSignal sel_1 {
      ijtag_sel && (sib == 2'b01);
   }
   LogicSignal sel_2 {
      ijtag_sel && (sib == 2'b10);
   }
   LogicSignal sel_3 {
      ijtag_sel && (sib == 2'b11);
   }
   ScanRegister sib[1:0] {
      ScanInSource scan_in_mux;
      CaptureSource 2'b00;
      ResetValue 2'b00;
   }
   // All four codes select a TDR; there is no bypass code.
   ScanMux scan_in_mux SelectedBy sib {
      2'b00 : ijtag_from_so[0];
      2'b01 : ijtag_from_so[1];
      2'b10 : ijtag_from_so[2];
      2'b11 : ijtag_from_so[3];
   }
}
