Module m {
  ScanMux compare_out_1[1:0] SelectedBy lol[1], lol[0] {
    1'b0,1'b1 | 1'b1,1'b0 : SI[0];
  }
  ScanMux mx[1:0] SelectedBy s {
    1'b0 : a;
  }
}
