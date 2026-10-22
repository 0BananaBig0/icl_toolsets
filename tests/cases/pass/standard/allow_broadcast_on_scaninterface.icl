// IEEE 1687 section6.4.7: broadcast belongs to an Instance body.
Module m {
  ScanInterface si_a { Port a; }
  ScanInterface si_b { Port b; }
  Instance broadcast_reference Of target { AllowBroadcastOnScanInterface si_a; }
}
