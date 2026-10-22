// IEEE 1687-2014 section 6.4.12 example, with missing declarations completed.
Module M {
DataInPort select_bit; DataInPort status[7:0]; DataInPort command[7:0];
DataMux C_S_REG SelectedBy select_bit { 1'b0 : status[7:0]; 1'b1 : command[7:0]; }
}
