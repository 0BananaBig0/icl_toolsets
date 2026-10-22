// IEEE §6.4.9 printed 119: supplied input declarations and wrapper.
Module M { DataInPort databus[31:0]; DataInPort block_sel; DataRegister Reg1[31:0] { WriteDataSource databus[31:0]; } DataRegister local_sel { WriteDataSource local_sel; WriteEnSource block_sel; } }
