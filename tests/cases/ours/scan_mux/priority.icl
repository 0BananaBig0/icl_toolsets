// IEEE 1687-2014 section 6.4.11; constructed case
Module M {
ScanMux m SelectedBy s { 2'bxx : a; 2'b10 : b; }
}
