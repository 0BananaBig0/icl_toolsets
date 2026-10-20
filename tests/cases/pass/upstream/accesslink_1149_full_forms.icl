// AccessLink shapes built from Honza255/icl_parser's grammar, which spells
// accessLink1149_def out in full:
//
//   accessLink1149_def : 'AccessLink' accessLink_name 'Of'
//       ('STD_1149_1_2001' | 'STD_1149_1_2013')
//       '{' 'BSDLEntity' bsdlEntity_name ';' bsdl_instr_ref+ '}' ;
//   bsdl_instr_ref : bsdl_instr_name '{' bsdl_instr_selected_item+ '}' ;
//   bsdl_instr_selected_item
//       : 'ScanInterface' '{' (accessLink1149_ScanInterface_name ';')+ '}'
//       | 'ActiveSignals' '{' (accessLink1149_ActiveSignal_name ';')+ '}' ;
//
// The `+` on bsdl_instr_ref is what the case in standard/ does not exercise:
// that one has a single instruction per link. Several instructions in one
// link is the common shape in a real chip-level file, and each instruction
// may carry either item, or both.
//
// Note also that AccessLink is NOT a top-level item here. iclSource_items is
// `nameSpace_def | useNameSpace_def | module_def` in that grammar too, so a
// link belongs inside a module — writing one at file scope is rejected, and
// that is not a gap.
Module chip {
  ScanInterface si { Port a; }
  ScanInterface si2 { Port b; }

  AccessLink tap Of STD_1149_1_2013 {
    BSDLEntity chip_bsdl;

    // One instruction per item.
    ijtag_en {
      ScanInterface { si; }
    }
    mbist_en {
      ActiveSignals { sig_a; }
    }
    // Both items in one instruction.
    scan_en {
      ScanInterface { si2; }
      ActiveSignals { sig_b; }
    }
    // The selected-item bodies are lists, not single names.
    debug_en {
      ScanInterface { si; si2; }
      ActiveSignals { sig_a; sig_b; sig_c; }
    }
  }

  // accessLink1149_ScanInterface_name is instance_name ('.' scanInterface_name)?,
  // so a hierarchical reference is spelled with a dot.
  AccessLink tapped Of STD_1149_1_2001 {
    BSDLEntity chip_bsdl;
    hier_en {
      ScanInterface { inst_a.si; inst_b.si2; }
    }
  }

  // accessLink1149_ActiveSignal_name is reg_port_signal_id, so it may be
  // indexed or ranged.
  AccessLink indexed Of STD_1149_1_2001 {
    BSDLEntity chip_bsdl;
    idx_en {
      ActiveSignals { bus[3:0]; one_bit[1]; }
    }
  }
}
