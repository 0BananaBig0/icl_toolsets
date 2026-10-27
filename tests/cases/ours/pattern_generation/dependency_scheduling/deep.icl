Module DeepDependencies {
  ScanInPort si;
  ScanRegister gate0 { ScanInSource si; ResetValue 1'b0; }
  ScanRegister gate1 { ScanInSource gate0; ResetValue 1'b0; }
  ScanRegister gate2 { ScanInSource gate1; ResetValue 1'b0; }
  ScanRegister gate3 { ScanInSource gate2; ResetValue 1'b0; }
  ScanRegister gate4 { ScanInSource gate3; ResetValue 1'b0; }
  ScanRegister gate5 { ScanInSource gate4; ResetValue 1'b0; }
  ScanRegister gate6 { ScanInSource gate5; ResetValue 1'b0; }
  ScanRegister gate7 { ScanInSource gate6; ResetValue 1'b0; }
  ScanRegister gate8 { ScanInSource gate7; ResetValue 1'b0; }
  ScanRegister gate9 { ScanInSource gate8; ResetValue 1'b0; }
  ScanRegister gate10 { ScanInSource gate9; ResetValue 1'b0; }
  ScanRegister gate11 { ScanInSource gate10; ResetValue 1'b0; }
  ScanRegister gate12 { ScanInSource gate11; ResetValue 1'b0; }
  ScanRegister gate13 { ScanInSource gate12; ResetValue 1'b0; }
  ScanRegister gate14 { ScanInSource gate13; ResetValue 1'b0; }
  ScanRegister gate15 { ScanInSource gate14; ResetValue 1'b0; }
  ScanRegister gate16 { ScanInSource gate15; ResetValue 1'b0; }
  ScanRegister gate17 { ScanInSource gate16; ResetValue 1'b0; }
  ScanRegister gate18 { ScanInSource gate17; ResetValue 1'b0; }
  ScanRegister gate19 { ScanInSource gate18; ResetValue 1'b0; }
  ScanRegister gate20 { ScanInSource gate19; ResetValue 1'b0; }
  ScanRegister gate21 { ScanInSource gate20; ResetValue 1'b0; }
  ScanRegister gate22 { ScanInSource gate21; ResetValue 1'b0; }
  ScanRegister gate23 { ScanInSource gate22; ResetValue 1'b0; }
  ScanRegister gate24 { ScanInSource gate23; ResetValue 1'b0; }
  ScanRegister gate25 { ScanInSource gate24; ResetValue 1'b0; }
  ScanRegister gate26 { ScanInSource gate25; ResetValue 1'b0; }
  ScanRegister gate27 { ScanInSource gate26; ResetValue 1'b0; }
  ScanRegister gate28 { ScanInSource gate27; ResetValue 1'b0; }
  ScanRegister gate29 { ScanInSource gate28; ResetValue 1'b0; }
  ScanRegister gate30 { ScanInSource gate29; ResetValue 1'b0; }
  ScanRegister gate31 { ScanInSource gate30; ResetValue 1'b0; }
  ScanRegister gate32 { ScanInSource gate31; ResetValue 1'b0; }
  ScanRegister gate33 { ScanInSource gate32; ResetValue 1'b0; }
  ScanRegister gate34 { ScanInSource gate33; ResetValue 1'b0; }
  ScanRegister gate35 { ScanInSource gate34; ResetValue 1'b0; }
  ScanRegister gate36 { ScanInSource gate35; ResetValue 1'b0; }
  ScanRegister gate37 { ScanInSource gate36; ResetValue 1'b0; }
  ScanRegister gate38 { ScanInSource gate37; ResetValue 1'b0; }
  ScanRegister gate39 { ScanInSource gate38; ResetValue 1'b0; }
  ScanRegister target[7:0] { ScanInSource gate39; ResetValue 8'b0; }
  ScanMux path39 SelectedBy gate39 { 1'b0: gate39; 1'b1: target[0]; }
  ScanMux path38 SelectedBy gate38 { 1'b0: gate38; 1'b1: path39; }
  ScanMux path37 SelectedBy gate37 { 1'b0: gate37; 1'b1: path38; }
  ScanMux path36 SelectedBy gate36 { 1'b0: gate36; 1'b1: path37; }
  ScanMux path35 SelectedBy gate35 { 1'b0: gate35; 1'b1: path36; }
  ScanMux path34 SelectedBy gate34 { 1'b0: gate34; 1'b1: path35; }
  ScanMux path33 SelectedBy gate33 { 1'b0: gate33; 1'b1: path34; }
  ScanMux path32 SelectedBy gate32 { 1'b0: gate32; 1'b1: path33; }
  ScanMux path31 SelectedBy gate31 { 1'b0: gate31; 1'b1: path32; }
  ScanMux path30 SelectedBy gate30 { 1'b0: gate30; 1'b1: path31; }
  ScanMux path29 SelectedBy gate29 { 1'b0: gate29; 1'b1: path30; }
  ScanMux path28 SelectedBy gate28 { 1'b0: gate28; 1'b1: path29; }
  ScanMux path27 SelectedBy gate27 { 1'b0: gate27; 1'b1: path28; }
  ScanMux path26 SelectedBy gate26 { 1'b0: gate26; 1'b1: path27; }
  ScanMux path25 SelectedBy gate25 { 1'b0: gate25; 1'b1: path26; }
  ScanMux path24 SelectedBy gate24 { 1'b0: gate24; 1'b1: path25; }
  ScanMux path23 SelectedBy gate23 { 1'b0: gate23; 1'b1: path24; }
  ScanMux path22 SelectedBy gate22 { 1'b0: gate22; 1'b1: path23; }
  ScanMux path21 SelectedBy gate21 { 1'b0: gate21; 1'b1: path22; }
  ScanMux path20 SelectedBy gate20 { 1'b0: gate20; 1'b1: path21; }
  ScanMux path19 SelectedBy gate19 { 1'b0: gate19; 1'b1: path20; }
  ScanMux path18 SelectedBy gate18 { 1'b0: gate18; 1'b1: path19; }
  ScanMux path17 SelectedBy gate17 { 1'b0: gate17; 1'b1: path18; }
  ScanMux path16 SelectedBy gate16 { 1'b0: gate16; 1'b1: path17; }
  ScanMux path15 SelectedBy gate15 { 1'b0: gate15; 1'b1: path16; }
  ScanMux path14 SelectedBy gate14 { 1'b0: gate14; 1'b1: path15; }
  ScanMux path13 SelectedBy gate13 { 1'b0: gate13; 1'b1: path14; }
  ScanMux path12 SelectedBy gate12 { 1'b0: gate12; 1'b1: path13; }
  ScanMux path11 SelectedBy gate11 { 1'b0: gate11; 1'b1: path12; }
  ScanMux path10 SelectedBy gate10 { 1'b0: gate10; 1'b1: path11; }
  ScanMux path9 SelectedBy gate9 { 1'b0: gate9; 1'b1: path10; }
  ScanMux path8 SelectedBy gate8 { 1'b0: gate8; 1'b1: path9; }
  ScanMux path7 SelectedBy gate7 { 1'b0: gate7; 1'b1: path8; }
  ScanMux path6 SelectedBy gate6 { 1'b0: gate6; 1'b1: path7; }
  ScanMux path5 SelectedBy gate5 { 1'b0: gate5; 1'b1: path6; }
  ScanMux path4 SelectedBy gate4 { 1'b0: gate4; 1'b1: path5; }
  ScanMux path3 SelectedBy gate3 { 1'b0: gate3; 1'b1: path4; }
  ScanMux path2 SelectedBy gate2 { 1'b0: gate2; 1'b1: path3; }
  ScanMux path1 SelectedBy gate1 { 1'b0: gate1; 1'b1: path2; }
  ScanMux path0 SelectedBy gate0 { 1'b0: gate0; 1'b1: path1; }
  ScanOutPort so { Source path0; }
}
