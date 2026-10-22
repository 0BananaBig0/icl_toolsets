Module EdgeInterfaceNestedChain {
  ScanInPort si;
  ScanInterface scan {
    Chain outer { Port si; Chain inner { Port si; } }
  }
}
