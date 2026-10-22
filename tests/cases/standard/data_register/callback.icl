// IEEE §6.4.9 printed 119–120: quoted string parameter, supplied u1 module.
Module Producer { DataOutPort dout[7:0]; }
Module M3 { Parameter callback_namespace = "M3"; Instance u1 Of Producer; DataRegister R1 { ReadDataSource u1.dout[7:0]; ReadCallBack $callback_namespace read_proc <D> <R>; } DataRegister W1 { WriteCallBack $callback_namespace write_proc <D> <R>; } }
