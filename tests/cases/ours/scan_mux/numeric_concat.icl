// IEEE 1687-2014 section 6.4.11; constructed case
Module M {
ScanMux m SelectedBy s { ~1'b0, $N | (2 + $P), 1 : ~a, b[3:0]; }
}
