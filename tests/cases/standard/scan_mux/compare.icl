// IEEE 1687-2014 section 6.4.11; completed example
Module M {
DataInPort check_mismatch[1:0]; ScanInPort different; ScanInPort same;
ScanMux compare_out SelectedBy check_mismatch[1:0] { 1'b0,1'b1 | 1'b1,1'b0 : different; 1'b1,1'b1 | 1'b0,1'b0 : same; }
}
