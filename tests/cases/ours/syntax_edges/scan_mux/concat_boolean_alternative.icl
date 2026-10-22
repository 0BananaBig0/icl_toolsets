Module EdgeScanMuxConcatBooleanAlternative {
  DataInPort select[1:0];
  ScanInPort main;
  ScanMux route SelectedBy select {
    1'b0, 1'b1 || 1'b1, 1'b0 : main;
  }
}
