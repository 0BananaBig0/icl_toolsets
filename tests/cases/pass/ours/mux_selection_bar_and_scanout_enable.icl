Module m {
  // The published grammar writes a mux selection as
  // `concat_number_list ':' concat_scan_signal ';'`, and concat_number_list
  // separates its alternatives with '|', not ','. Both are accepted; a
  // formatter must not collapse one into the other.
  ScanMux smux SelectedBy sel {
    1'b0 | 2'b01 : si_a;
    1'b1         : si_b, si_c;
  }
  DataMux dmux SelectedBy sel {
    1'b0 | 1 : d0;
  }
  ClockMux cmux SelectedBy sel {
    1'b0 | 1 : c0;
  }

  // ScanOutPort takes an Enable as well as a Source, which the plain port
  // block does not.
  ScanOutPort so {
    Source tdo;
    Enable en;
  }
}
