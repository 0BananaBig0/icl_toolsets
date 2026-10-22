Module ex1 { Instance ex2_inst Of ex2; }
NameSpace ex1ns;
Module ex1 { Instance ex2_inst Of ex2; UseNameSpace; Instance ex2_inst_2 Of ex2; Instance ex2 Of ex2ns::ex2; }
Module ex2 {}
Module ex3 { Instance ex2_inst Of ex2; }
