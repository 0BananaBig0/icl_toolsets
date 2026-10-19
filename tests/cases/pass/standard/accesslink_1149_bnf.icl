// The accessLink1149_def BNF and its two examples, from IEEE 1687-2014.
//
//   accessLink1149_def : 'AccessLink' accessLink_name 'Of'
//       ('STD_1149_1_2001' | 'STD_1149_1_2013')
//       '{' 'BSDLEntity' bsdlEntity_name ';' bsdl_instr_ref+ '}' ;
//   bsdl_instr_ref : bsdl_instr_name '{' bsdl_instr_selected_item+ '}' ;
//   bsdl_instr_selected_item : 'ScanInterface' '{' name ';' + '}'
//       ('ActiveSignals' '{' name ';' + '}') ;
//
// Note the spelling: the standard writes **BSDLEntity**, with no underscore.
// Real files (the IJTAG benchmark set) use that spelling, not `BSDL_Entity`,
// so both are accepted — see scanworks_accesslink_nested.icl for the other.
//
// The rule text also pins three keywords that the scanner did not have:
// STD_1149_1_2001, STD_1149_1_2013 and ActiveSignals.
Module chip2542 {
  ScanInterface MyScanInterface { Port a; }

  // Example 1 from the standard: IEEE 1149.1 access.
  AccessLink dot1 Of STD_1149_1_2001 {
    BSDLEntity chip2542;
    ijtag_en {                          // Name of instruction
      ScanInterface {                   // defines the ScanInterface selected
        InstPath.MyScanInterface;
      }
    }
  }

  // The same, with the 2013 revision and the ActiveSignals item that the
  // BNF lists as the second bsdl_instr_selected_item.
  AccessLink dot2 Of STD_1149_1_2013 {
    BSDLEntity chip2542;
    ijtag_en {
      ScanInterface { InstPath.MyScanInterface; }
      ActiveSignals { signal_name1; signal_name2; }
    }
  }

  // Example 2 from the standard: generic access. The link type is an
  // ordinary identifier (accessLink_genericID : SCALAR_ID), not a keyword.
  AccessLink mylink Of Generic { MyScanInterface; }
}
