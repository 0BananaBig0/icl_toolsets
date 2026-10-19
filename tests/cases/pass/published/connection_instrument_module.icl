// A minimal instrument module from a Chinese ICL/PDL tutorial, completed.
// It is here because it uses `Connection` with `->` edges, a construct no
// file of ours exercised.
//
// The tutorial writes `DataIn sc;` / `DataOut so;` at module level. That is
// not a module item in the published grammar — module level admits only
// attribute_def, which begins with the keyword Attribute — so the standard
// spellings are used here, and the tutorial's own spellings are exercised
// where they are legal, inside a block.
Module my_instr {
  Attribute name = "my_instr";
  ScanInterface si { Port sc; }
  DataInPort sc;
  DataOutPort so;
  Reg reg_ctrl { Width 8; }
  Connection {
    sc -> reg_ctrl.si;
    reg_ctrl.so -> so;
  }
}
// The same shape with the tutorial's spellings inside a block, where an
// attribute name may be any identifier.
Module my_instr_block {
  ScanInterface si { Port sc; }
  DataInPort sc;
  DataOutPort so;
  Reg reg_ctrl {
    DataIn sc;
    DataOut so;
    Width 8;
  }
  Connection {
    sc -> reg_ctrl.si;
    reg_ctrl.so -> so;
  }
}
