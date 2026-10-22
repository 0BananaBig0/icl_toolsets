Module EdgeDataMuxConcatMissingCode {
  DataInPort select[1:0];
  DataInPort a;
  DataMux route SelectedBy select { 1'b0, 1'b1 | /* missing */ : a; }
}
