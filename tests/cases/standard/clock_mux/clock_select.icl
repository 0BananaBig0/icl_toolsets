// IEEE 1687-2014 section 6.4.13 example, with missing declarations completed.
Module M {
DataInPort md[1:0]; ClockPort sck1; ClockPort sck2; ClockPort sck3;
ClockMux ck SelectedBy md[1:0] { 2'b01 : sck1; 2'b10 : sck2; 2'bxx : sck3; }
}
