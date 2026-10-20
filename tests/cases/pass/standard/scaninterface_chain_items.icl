// IEEE 1687, the ScanInterface block, checked item by item.
//
//   scanInterface_def      : 'ScanInterface' scanInterface_name '{'
//                            scanInterface_item+ '}' ;
//   scanInterface_item     : attribute_def
//                          | scanInterfacePort_def
//                          | defaultLoad_def
//                          | scanInterfaceChain_def ;
//   scanInterfaceChain_def : 'Chain' scanInterfaceChain_name '{'
//                            scanInterfaceChain_item+ '}' ;
//   scanInterfaceChain_item: attribute_def
//                          | scanInterfacePort_def
//                          | defaultLoad_def ;
//
// The three items are the same list in both places; only Chain is missing
// from the inner one, so a Chain cannot nest inside a Chain.
Module m {
  ScanInterface si {
    Port SI;
    Attribute a = 1;
    DefaultLoadValue 1'b0;
    Chain c {
      Port SO;
      Attribute b = 2;
      DefaultLoadValue 1'b1;
    }
  }
}
