// Rejected: an iScan literal. PDL only — `ISCAN_BIN_NUMBER` and
// `ISCAN_HEX_NUMBER` have no counterpart in icl.g4.
Module m {
  ScanRegister sr {
    ResetValue {0b1010};
  }
}
