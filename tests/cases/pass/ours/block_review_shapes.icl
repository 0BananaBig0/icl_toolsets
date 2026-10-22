// Shapes checked block by block against the published grammar, that no file
// here exercised. Collected so a regression names the shape instead of
// pointing at whichever large file happened to contain it.
Module block_review {
  // DataOutPort: one Enable data_signal, optionally inverted (IEEE §6.4.6.7).
  DataOutPort do_all {
    Attribute a = 1;
    Source t;
    Enable ~e;
    RefEnum E;
  }

  // The control ports with bodies, and reset polarity.
  ShiftEnPort se { Attribute x = 1; }
  CaptureEnPort ce { Attribute x = 1; }
  UpdateEnPort ue { Attribute x = 1; }
  SelectPort sp { Attribute x = 1; }
  ResetPort rp { ActivePolarity 1; }

  // The "to" ports, each of which takes a Source.
  ToTCKPort a { Source s; }
  ToTMSPort b { Source s; }
  ToTRSTPort c { Source s; }
  ToShiftEnPort d { Attribute x = 1; }
  ToCaptureEnPort e { Attribute x = 1; }
  ToUpdateEnPort f { Attribute x = 1; }
  ToSelectPort g { Source s; }
  ToResetPort h { Source s; ActivePolarity 0; }
  ToIRSelectPort i;
  AddressPort j;
  WriteEnPort k;
  ReadEnPort l;

  // DataRegister: all four flavours in one module.
  DataRegister dr_write { WriteEnSource e; WriteDataSource d; }
  OneHotDataGroup addressed { DataRegister dr_addr { AddressValue 'h0; } }
  DataRegister dr_read { ReadDataSource r; }
  DataRegister dr_cb { WriteCallBack m p <D>; }

  // One-hot groups: one with a port list, one containing a whole register.
  OneHotScanGroup ohs[3:0] { Port a, b; }
  OneHotDataGroup ohd { DataRegister dr { AddressValue 0; } Port a; }

  // A scan interface with a chain, and one with several ports.
  ScanInterface si_chain { Chain c { Port a; DefaultLoadValue 1; } }
  ScanInterface si_ports { Port a; Port b; Port c; }

  // LogicSignal: reduction operators and a parenthesised list.
  LogicSignal l_red { |(~(a), b); }
  LogicSignal l_par { (a, b); }

  // Instance: every item the grammar lists.
  Instance inst Of M {
    InputPort a = b;
    InputPort c = d;
    Parameter P = 1;
    Attribute x = 1;
  }

  // A concatenation mixing every element form.
  Alias al = a, ~b, c[1:0];
}
