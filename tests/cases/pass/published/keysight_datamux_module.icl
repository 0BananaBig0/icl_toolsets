Module Module_Mux {
  DataInPort DataIn;
  DataInPort MuxSel;
  DataOutPort DataOut {
    Source Data_Mux;
  }
  DataMux Data_mux SelectedBy MuxSel {
    1'b0 : DataIn;
    1'b1 : 1'b0;
  }
}
