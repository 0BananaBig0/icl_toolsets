Module Child { ScanInPort si; ScanInterface access { Port si; } }
Module Host {
  Instance child Of Child;
  AccessLink tap Of STD_1149_1_2001 {
    BSDLEntity host;
    ijtag_en { ScanInterface { child.access; } }
  }
}
