Module m {
ScanMux mx SelectedBy s {
1'b0 : a;
1'b1 : b, c;
2'b10 | 2'b11 : d;
}
}
