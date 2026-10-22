// IEEE 1687-2014 section 6.4.17 examples; enclosing module and referenced child supplied.
Module Child { ScanInPort si; ScanOutPort so; SelectPort en;
 ScanInterface MyScanInterface { Port si; Port so; Port en; }
}
Module Top { Instance InstPath Of Child;
 AccessLink dot1 Of STD_1149_1_2001 { BSDLEntity chip2542; ijtag_en { ScanInterface { InstPath.MyScanInterface; } } }
 AccessLink mylink Of Generic { MyScanInterface; }
}
