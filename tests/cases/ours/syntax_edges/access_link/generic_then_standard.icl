Module EdgeAccessLinkGenericThenStandard {
  ScanInPort si;
  ScanOutPort so { Source si; }
  ScanInterface scan { Port si; Port so; }
  AccessLink custom Of Generic {
    outer { inner { "escaped quote \" and } {" } }
    /* } ignored brace */
    // { ignored brace
    / raw + @ vendor tokens ;
  }
  AccessLink tap Of STD_1149_1_2013 {
    BSDLEntity Chip;
    IJTAG { ScanInterface { scan; } }
  }
  Attribute after = "normal lexer again";
}
