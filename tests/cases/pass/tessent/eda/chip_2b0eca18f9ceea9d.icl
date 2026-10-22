NameSpace ;
UseNameSpace ;
Module chip {
   TCKPort TCK;
   ScanInPort TDI;
   ScanOutPort TDO {
      Source SIB_2.ijtag_so;
   }
   SelectPort sel;
   ShiftEnPort shift_en;
   UpdateEnPort update_en;
   CaptureEnPort capture_en;
   Instance SIB_1 Of SIB {
      InputPort ijtag_si = TDI;
      InputPort ijtag_from_so = TDR_1.so;
      InputPort ijtag_sel = sel;
      InputPort ijtag_tck = TCK;
      InputPort ijtag_se = shift_en;
      InputPort ijtag_ce = capture_en;
      InputPort ijtag_ue = update_en;
   }
   Instance SIB_2 Of SIB {
      InputPort ijtag_si = SIB_1.ijtag_so;
      InputPort ijtag_from_so = TDR_2.so;
      InputPort ijtag_sel = sel;
      InputPort ijtag_tck = TCK;
      InputPort ijtag_se = shift_en;
      InputPort ijtag_ce = capture_en;
      InputPort ijtag_ue = update_en;
   }
   Instance TDR_1 Of TDR {
      InputPort tck = TCK;
      InputPort si = TDI;
      InputPort en = SIB_1.ijtag_to_sel;
      InputPort Temp = Sensor_1.Temp;
      InputPort se = shift_en;
      InputPort ce = capture_en;
      InputPort ue = update_en;
   }
   Instance TDR_2 Of TDR {
      InputPort tck = TCK;
      InputPort si = SIB_1.ijtag_so;
      InputPort en = SIB_2.ijtag_to_sel;
      InputPort Temp = Sensor_2.Temp;
      InputPort se = shift_en;
      InputPort ce = capture_en;
      InputPort ue = update_en;
   }
   Instance Sensor_1 Of Sensor {
      InputPort tck = TCK;
      InputPort En = TDR_1.En;
   }
   Instance Sensor_2 Of Sensor {
      InputPort tck = TCK;
      InputPort En = TDR_2.En;
   }
}
