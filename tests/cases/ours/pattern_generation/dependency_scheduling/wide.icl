Module WideDependencies {
  ScanInPort si;
  ScanRegister gate0 { ScanInSource si; ResetValue 1'b0; }
  ScanRegister target0[7:0] { ScanInSource gate0; ResetValue 8'b0; }
  ScanMux path0 SelectedBy gate0 { 1'b0: gate0; 1'b1: target0[0]; }
  ScanRegister gate1 { ScanInSource path0; ResetValue 1'b0; }
  ScanRegister target1[7:0] { ScanInSource gate1; ResetValue 8'b0; }
  ScanMux path1 SelectedBy gate1 { 1'b0: gate1; 1'b1: target1[0]; }
  ScanRegister gate2 { ScanInSource path1; ResetValue 1'b0; }
  ScanRegister target2[7:0] { ScanInSource gate2; ResetValue 8'b0; }
  ScanMux path2 SelectedBy gate2 { 1'b0: gate2; 1'b1: target2[0]; }
  ScanRegister gate3 { ScanInSource path2; ResetValue 1'b0; }
  ScanRegister target3[7:0] { ScanInSource gate3; ResetValue 8'b0; }
  ScanMux path3 SelectedBy gate3 { 1'b0: gate3; 1'b1: target3[0]; }
  ScanRegister gate4 { ScanInSource path3; ResetValue 1'b0; }
  ScanRegister target4[7:0] { ScanInSource gate4; ResetValue 8'b0; }
  ScanMux path4 SelectedBy gate4 { 1'b0: gate4; 1'b1: target4[0]; }
  ScanRegister gate5 { ScanInSource path4; ResetValue 1'b0; }
  ScanRegister target5[7:0] { ScanInSource gate5; ResetValue 8'b0; }
  ScanMux path5 SelectedBy gate5 { 1'b0: gate5; 1'b1: target5[0]; }
  ScanRegister gate6 { ScanInSource path5; ResetValue 1'b0; }
  ScanRegister target6[7:0] { ScanInSource gate6; ResetValue 8'b0; }
  ScanMux path6 SelectedBy gate6 { 1'b0: gate6; 1'b1: target6[0]; }
  ScanRegister gate7 { ScanInSource path6; ResetValue 1'b0; }
  ScanRegister target7[7:0] { ScanInSource gate7; ResetValue 8'b0; }
  ScanMux path7 SelectedBy gate7 { 1'b0: gate7; 1'b1: target7[0]; }
  ScanRegister gate8 { ScanInSource path7; ResetValue 1'b0; }
  ScanRegister target8[7:0] { ScanInSource gate8; ResetValue 8'b0; }
  ScanMux path8 SelectedBy gate8 { 1'b0: gate8; 1'b1: target8[0]; }
  ScanRegister gate9 { ScanInSource path8; ResetValue 1'b0; }
  ScanRegister target9[7:0] { ScanInSource gate9; ResetValue 8'b0; }
  ScanMux path9 SelectedBy gate9 { 1'b0: gate9; 1'b1: target9[0]; }
  ScanRegister gate10 { ScanInSource path9; ResetValue 1'b0; }
  ScanRegister target10[7:0] { ScanInSource gate10; ResetValue 8'b0; }
  ScanMux path10 SelectedBy gate10 { 1'b0: gate10; 1'b1: target10[0]; }
  ScanRegister gate11 { ScanInSource path10; ResetValue 1'b0; }
  ScanRegister target11[7:0] { ScanInSource gate11; ResetValue 8'b0; }
  ScanMux path11 SelectedBy gate11 { 1'b0: gate11; 1'b1: target11[0]; }
  ScanRegister gate12 { ScanInSource path11; ResetValue 1'b0; }
  ScanRegister target12[7:0] { ScanInSource gate12; ResetValue 8'b0; }
  ScanMux path12 SelectedBy gate12 { 1'b0: gate12; 1'b1: target12[0]; }
  ScanRegister gate13 { ScanInSource path12; ResetValue 1'b0; }
  ScanRegister target13[7:0] { ScanInSource gate13; ResetValue 8'b0; }
  ScanMux path13 SelectedBy gate13 { 1'b0: gate13; 1'b1: target13[0]; }
  ScanRegister gate14 { ScanInSource path13; ResetValue 1'b0; }
  ScanRegister target14[7:0] { ScanInSource gate14; ResetValue 8'b0; }
  ScanMux path14 SelectedBy gate14 { 1'b0: gate14; 1'b1: target14[0]; }
  ScanRegister gate15 { ScanInSource path14; ResetValue 1'b0; }
  ScanRegister target15[7:0] { ScanInSource gate15; ResetValue 8'b0; }
  ScanMux path15 SelectedBy gate15 { 1'b0: gate15; 1'b1: target15[0]; }
  ScanRegister gate16 { ScanInSource path15; ResetValue 1'b0; }
  ScanRegister target16[7:0] { ScanInSource gate16; ResetValue 8'b0; }
  ScanMux path16 SelectedBy gate16 { 1'b0: gate16; 1'b1: target16[0]; }
  ScanRegister gate17 { ScanInSource path16; ResetValue 1'b0; }
  ScanRegister target17[7:0] { ScanInSource gate17; ResetValue 8'b0; }
  ScanMux path17 SelectedBy gate17 { 1'b0: gate17; 1'b1: target17[0]; }
  ScanRegister gate18 { ScanInSource path17; ResetValue 1'b0; }
  ScanRegister target18[7:0] { ScanInSource gate18; ResetValue 8'b0; }
  ScanMux path18 SelectedBy gate18 { 1'b0: gate18; 1'b1: target18[0]; }
  ScanRegister gate19 { ScanInSource path18; ResetValue 1'b0; }
  ScanRegister target19[7:0] { ScanInSource gate19; ResetValue 8'b0; }
  ScanMux path19 SelectedBy gate19 { 1'b0: gate19; 1'b1: target19[0]; }
  ScanRegister gate20 { ScanInSource path19; ResetValue 1'b0; }
  ScanRegister target20[7:0] { ScanInSource gate20; ResetValue 8'b0; }
  ScanMux path20 SelectedBy gate20 { 1'b0: gate20; 1'b1: target20[0]; }
  ScanRegister gate21 { ScanInSource path20; ResetValue 1'b0; }
  ScanRegister target21[7:0] { ScanInSource gate21; ResetValue 8'b0; }
  ScanMux path21 SelectedBy gate21 { 1'b0: gate21; 1'b1: target21[0]; }
  ScanRegister gate22 { ScanInSource path21; ResetValue 1'b0; }
  ScanRegister target22[7:0] { ScanInSource gate22; ResetValue 8'b0; }
  ScanMux path22 SelectedBy gate22 { 1'b0: gate22; 1'b1: target22[0]; }
  ScanRegister gate23 { ScanInSource path22; ResetValue 1'b0; }
  ScanRegister target23[7:0] { ScanInSource gate23; ResetValue 8'b0; }
  ScanMux path23 SelectedBy gate23 { 1'b0: gate23; 1'b1: target23[0]; }
  ScanRegister gate24 { ScanInSource path23; ResetValue 1'b0; }
  ScanRegister target24[7:0] { ScanInSource gate24; ResetValue 8'b0; }
  ScanMux path24 SelectedBy gate24 { 1'b0: gate24; 1'b1: target24[0]; }
  ScanRegister gate25 { ScanInSource path24; ResetValue 1'b0; }
  ScanRegister target25[7:0] { ScanInSource gate25; ResetValue 8'b0; }
  ScanMux path25 SelectedBy gate25 { 1'b0: gate25; 1'b1: target25[0]; }
  ScanRegister gate26 { ScanInSource path25; ResetValue 1'b0; }
  ScanRegister target26[7:0] { ScanInSource gate26; ResetValue 8'b0; }
  ScanMux path26 SelectedBy gate26 { 1'b0: gate26; 1'b1: target26[0]; }
  ScanRegister gate27 { ScanInSource path26; ResetValue 1'b0; }
  ScanRegister target27[7:0] { ScanInSource gate27; ResetValue 8'b0; }
  ScanMux path27 SelectedBy gate27 { 1'b0: gate27; 1'b1: target27[0]; }
  ScanRegister gate28 { ScanInSource path27; ResetValue 1'b0; }
  ScanRegister target28[7:0] { ScanInSource gate28; ResetValue 8'b0; }
  ScanMux path28 SelectedBy gate28 { 1'b0: gate28; 1'b1: target28[0]; }
  ScanRegister gate29 { ScanInSource path28; ResetValue 1'b0; }
  ScanRegister target29[7:0] { ScanInSource gate29; ResetValue 8'b0; }
  ScanMux path29 SelectedBy gate29 { 1'b0: gate29; 1'b1: target29[0]; }
  ScanRegister gate30 { ScanInSource path29; ResetValue 1'b0; }
  ScanRegister target30[7:0] { ScanInSource gate30; ResetValue 8'b0; }
  ScanMux path30 SelectedBy gate30 { 1'b0: gate30; 1'b1: target30[0]; }
  ScanRegister gate31 { ScanInSource path30; ResetValue 1'b0; }
  ScanRegister target31[7:0] { ScanInSource gate31; ResetValue 8'b0; }
  ScanMux path31 SelectedBy gate31 { 1'b0: gate31; 1'b1: target31[0]; }
  ScanRegister gate32 { ScanInSource path31; ResetValue 1'b0; }
  ScanRegister target32[7:0] { ScanInSource gate32; ResetValue 8'b0; }
  ScanMux path32 SelectedBy gate32 { 1'b0: gate32; 1'b1: target32[0]; }
  ScanRegister gate33 { ScanInSource path32; ResetValue 1'b0; }
  ScanRegister target33[7:0] { ScanInSource gate33; ResetValue 8'b0; }
  ScanMux path33 SelectedBy gate33 { 1'b0: gate33; 1'b1: target33[0]; }
  ScanRegister gate34 { ScanInSource path33; ResetValue 1'b0; }
  ScanRegister target34[7:0] { ScanInSource gate34; ResetValue 8'b0; }
  ScanMux path34 SelectedBy gate34 { 1'b0: gate34; 1'b1: target34[0]; }
  ScanRegister gate35 { ScanInSource path34; ResetValue 1'b0; }
  ScanRegister target35[7:0] { ScanInSource gate35; ResetValue 8'b0; }
  ScanMux path35 SelectedBy gate35 { 1'b0: gate35; 1'b1: target35[0]; }
  ScanRegister gate36 { ScanInSource path35; ResetValue 1'b0; }
  ScanRegister target36[7:0] { ScanInSource gate36; ResetValue 8'b0; }
  ScanMux path36 SelectedBy gate36 { 1'b0: gate36; 1'b1: target36[0]; }
  ScanRegister gate37 { ScanInSource path36; ResetValue 1'b0; }
  ScanRegister target37[7:0] { ScanInSource gate37; ResetValue 8'b0; }
  ScanMux path37 SelectedBy gate37 { 1'b0: gate37; 1'b1: target37[0]; }
  ScanRegister gate38 { ScanInSource path37; ResetValue 1'b0; }
  ScanRegister target38[7:0] { ScanInSource gate38; ResetValue 8'b0; }
  ScanMux path38 SelectedBy gate38 { 1'b0: gate38; 1'b1: target38[0]; }
  ScanRegister gate39 { ScanInSource path38; ResetValue 1'b0; }
  ScanRegister target39[7:0] { ScanInSource gate39; ResetValue 8'b0; }
  ScanMux path39 SelectedBy gate39 { 1'b0: gate39; 1'b1: target39[0]; }
  ScanRegister gate40 { ScanInSource path39; ResetValue 1'b0; }
  ScanRegister target40[7:0] { ScanInSource gate40; ResetValue 8'b0; }
  ScanMux path40 SelectedBy gate40 { 1'b0: gate40; 1'b1: target40[0]; }
  ScanRegister gate41 { ScanInSource path40; ResetValue 1'b0; }
  ScanRegister target41[7:0] { ScanInSource gate41; ResetValue 8'b0; }
  ScanMux path41 SelectedBy gate41 { 1'b0: gate41; 1'b1: target41[0]; }
  ScanRegister gate42 { ScanInSource path41; ResetValue 1'b0; }
  ScanRegister target42[7:0] { ScanInSource gate42; ResetValue 8'b0; }
  ScanMux path42 SelectedBy gate42 { 1'b0: gate42; 1'b1: target42[0]; }
  ScanRegister gate43 { ScanInSource path42; ResetValue 1'b0; }
  ScanRegister target43[7:0] { ScanInSource gate43; ResetValue 8'b0; }
  ScanMux path43 SelectedBy gate43 { 1'b0: gate43; 1'b1: target43[0]; }
  ScanRegister gate44 { ScanInSource path43; ResetValue 1'b0; }
  ScanRegister target44[7:0] { ScanInSource gate44; ResetValue 8'b0; }
  ScanMux path44 SelectedBy gate44 { 1'b0: gate44; 1'b1: target44[0]; }
  ScanRegister gate45 { ScanInSource path44; ResetValue 1'b0; }
  ScanRegister target45[7:0] { ScanInSource gate45; ResetValue 8'b0; }
  ScanMux path45 SelectedBy gate45 { 1'b0: gate45; 1'b1: target45[0]; }
  ScanRegister gate46 { ScanInSource path45; ResetValue 1'b0; }
  ScanRegister target46[7:0] { ScanInSource gate46; ResetValue 8'b0; }
  ScanMux path46 SelectedBy gate46 { 1'b0: gate46; 1'b1: target46[0]; }
  ScanRegister gate47 { ScanInSource path46; ResetValue 1'b0; }
  ScanRegister target47[7:0] { ScanInSource gate47; ResetValue 8'b0; }
  ScanMux path47 SelectedBy gate47 { 1'b0: gate47; 1'b1: target47[0]; }
  ScanRegister gate48 { ScanInSource path47; ResetValue 1'b0; }
  ScanRegister target48[7:0] { ScanInSource gate48; ResetValue 8'b0; }
  ScanMux path48 SelectedBy gate48 { 1'b0: gate48; 1'b1: target48[0]; }
  ScanRegister gate49 { ScanInSource path48; ResetValue 1'b0; }
  ScanRegister target49[7:0] { ScanInSource gate49; ResetValue 8'b0; }
  ScanMux path49 SelectedBy gate49 { 1'b0: gate49; 1'b1: target49[0]; }
  ScanRegister gate50 { ScanInSource path49; ResetValue 1'b0; }
  ScanRegister target50[7:0] { ScanInSource gate50; ResetValue 8'b0; }
  ScanMux path50 SelectedBy gate50 { 1'b0: gate50; 1'b1: target50[0]; }
  ScanRegister gate51 { ScanInSource path50; ResetValue 1'b0; }
  ScanRegister target51[7:0] { ScanInSource gate51; ResetValue 8'b0; }
  ScanMux path51 SelectedBy gate51 { 1'b0: gate51; 1'b1: target51[0]; }
  ScanRegister gate52 { ScanInSource path51; ResetValue 1'b0; }
  ScanRegister target52[7:0] { ScanInSource gate52; ResetValue 8'b0; }
  ScanMux path52 SelectedBy gate52 { 1'b0: gate52; 1'b1: target52[0]; }
  ScanRegister gate53 { ScanInSource path52; ResetValue 1'b0; }
  ScanRegister target53[7:0] { ScanInSource gate53; ResetValue 8'b0; }
  ScanMux path53 SelectedBy gate53 { 1'b0: gate53; 1'b1: target53[0]; }
  ScanRegister gate54 { ScanInSource path53; ResetValue 1'b0; }
  ScanRegister target54[7:0] { ScanInSource gate54; ResetValue 8'b0; }
  ScanMux path54 SelectedBy gate54 { 1'b0: gate54; 1'b1: target54[0]; }
  ScanRegister gate55 { ScanInSource path54; ResetValue 1'b0; }
  ScanRegister target55[7:0] { ScanInSource gate55; ResetValue 8'b0; }
  ScanMux path55 SelectedBy gate55 { 1'b0: gate55; 1'b1: target55[0]; }
  ScanRegister gate56 { ScanInSource path55; ResetValue 1'b0; }
  ScanRegister target56[7:0] { ScanInSource gate56; ResetValue 8'b0; }
  ScanMux path56 SelectedBy gate56 { 1'b0: gate56; 1'b1: target56[0]; }
  ScanRegister gate57 { ScanInSource path56; ResetValue 1'b0; }
  ScanRegister target57[7:0] { ScanInSource gate57; ResetValue 8'b0; }
  ScanMux path57 SelectedBy gate57 { 1'b0: gate57; 1'b1: target57[0]; }
  ScanRegister gate58 { ScanInSource path57; ResetValue 1'b0; }
  ScanRegister target58[7:0] { ScanInSource gate58; ResetValue 8'b0; }
  ScanMux path58 SelectedBy gate58 { 1'b0: gate58; 1'b1: target58[0]; }
  ScanRegister gate59 { ScanInSource path58; ResetValue 1'b0; }
  ScanRegister target59[7:0] { ScanInSource gate59; ResetValue 8'b0; }
  ScanMux path59 SelectedBy gate59 { 1'b0: gate59; 1'b1: target59[0]; }
  ScanRegister gate60 { ScanInSource path59; ResetValue 1'b0; }
  ScanRegister target60[7:0] { ScanInSource gate60; ResetValue 8'b0; }
  ScanMux path60 SelectedBy gate60 { 1'b0: gate60; 1'b1: target60[0]; }
  ScanRegister gate61 { ScanInSource path60; ResetValue 1'b0; }
  ScanRegister target61[7:0] { ScanInSource gate61; ResetValue 8'b0; }
  ScanMux path61 SelectedBy gate61 { 1'b0: gate61; 1'b1: target61[0]; }
  ScanRegister gate62 { ScanInSource path61; ResetValue 1'b0; }
  ScanRegister target62[7:0] { ScanInSource gate62; ResetValue 8'b0; }
  ScanMux path62 SelectedBy gate62 { 1'b0: gate62; 1'b1: target62[0]; }
  ScanRegister gate63 { ScanInSource path62; ResetValue 1'b0; }
  ScanRegister target63[7:0] { ScanInSource gate63; ResetValue 8'b0; }
  ScanMux path63 SelectedBy gate63 { 1'b0: gate63; 1'b1: target63[0]; }
  ScanOutPort so { Source path63; }
}
