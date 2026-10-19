// allowBroadcast_def, from the published grammar:
//   'AllowBroadcastOnScanInterface' scanInterface_name ';'
// Marks a scan interface as accepting a broadcast write. It was rejected
// outright — the keyword was not in the scanner, so it arrived as an
// identifier and then failed as a module item.
//
// Note the name is a single SCALAR_ID, not a list: scanInterface_name has no
// comma form, so `AllowBroadcastOnScanInterface si_a, si_b;` is not ICL.
Module m {
  ScanInterface si_a { Port a; }
  ScanInterface si_b { Port b; }
  AllowBroadcastOnScanInterface si_a;
}
