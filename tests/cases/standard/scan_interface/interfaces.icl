// IEEE 1687-2014 section 6.4.16 examples; missing module and port declarations supplied.
Module ClientExample { SelectPort SEL; ScanInPort SI; ScanOutPort SO;
 ScanInterface Client { Port SEL; Port SI; Port SO; }
}
Module HostExample { ToSelectPort ToSel { Source 1; } ScanInPort FromSO;
 ScanInterface Host { Port ToSel; Port FromSO; }
}
Module ParallelExample { SelectPort Sel; ShiftEnPort SE; ScanInPort SI1A; ScanInPort SI1B; ScanOutPort SO1A; ScanOutPort SO1B;
 ScanInterface Par_client2 {
  Port Sel; Port SE;
  Chain c1 { Port SI1A; Port SO1A; DefaultLoadValue 8'b11011010; }
  Chain c2 { Port SI1B; Port SO1B; DefaultLoadValue 6'b00100101; }
 }
}
Module TapExample { TMSPort tms; ScanInPort tdi; ScanOutPort tdo;
 ScanInterface TAP { Port tms; Port tdi; Port tdo; }
}
