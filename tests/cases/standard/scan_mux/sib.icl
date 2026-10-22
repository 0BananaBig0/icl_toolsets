// IEEE 1687-2014 section 6.4.11; completed example
Module M {
DataInPort SIBREG; ScanInPort base[0]; ScanInPort aux[0];
ScanMux SIB_out SelectedBy SIBREG { 1'b0 : base[0]; 1'b1 : aux[0]; }
}
