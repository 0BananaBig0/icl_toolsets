Module EdgeScanMuxConcatAlternativeComments {
  DataInPort select[1:0];
  ScanInPort main;
  ScanInPort auxiliary;
  ScanMux route SelectedBy select[1], select[0] {
    1'b0, /* low */ 1'b1 | 1'b1, 1'b0 : auxiliary;
    2'b00 /* alternate */ | // preserve priority
      2'b11 : main;
  }
  ScanOutPort out { Source route; }
}
