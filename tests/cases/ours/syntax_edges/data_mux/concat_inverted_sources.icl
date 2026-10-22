Module EdgeDataMuxConcatInvertedSources {
  DataInPort select[1:0];
  DataInPort a[3:0];
  DataInPort b[3:0];
  DataMux route[3:0] SelectedBy select {
    1'b0, 1'b1 | 1'b1, 1'b0 : ~a[(1 + 2):2], b[1:0];
    2'b00 | 2'b11 : b[3:2], ~a[1:0];
  }
  DataOutPort out[3:0] { Source route; }
}
