// From Honza255/icl_parser: the commit message on its README is
// "chainScanData can have multiple data for multiple SI -> SO ports".
//
// That sentence is about its own Python data structure, not about ICL syntax:
// chainScanData is a class in src/icl_common.py holding one (tdi_port,
// tdo_port, in_data, exp_data) tuple, and stepScanData holds a dict of them
// keyed by chain name — so one step can drive several scan chains.
//
// The ICL side of that idea is: a ScanInterface may carry more than one
// Chain, and each Chain names its own Port set. That is what
// scanInterfaceChain_def says:
//
//   scanInterfaceChain_def : 'Chain' scanInterfaceChain_name '{'
//                            scanInterfaceChain_item+ '}' ;
//
// so the shapes below are the ones that sentence implies, written out.
Module m {
  ScanInterface si {
    // One chain, one SI/SO pair.
    Chain default_chain {
      Port SI_0;
      Port SO_0;
    }
    // A second pair in the same interface: this is the "multiple SI -> SO"
    // case, and it is why Chain exists as a named block at all — without a
    // name there would be no way to tell the pairs apart.
    Chain second_chain {
      Port TDI_1;
      Port TDO_1;
      Port SEL_1;
    }
  }

  // The unnamed form: a single chain needs no Chain block, and its ports are
  // written directly in the interface.
  ScanInterface si_single {
    Port SI;
    Port SO;
  }
}
