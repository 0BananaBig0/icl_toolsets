NameSpace ;
UseNameSpace ;
Module chip {
   ResetPort reset_n {
      ActivePolarity 0;
   }
   TCKPort TCK;
   ScanInPort TDI;
   ScanOutPort TDO {
      Source SIB_1.ijtag_so;
   }
   SelectPort sel;
   ShiftEnPort shift_en;
   UpdateEnPort update_en;
   CaptureEnPort capture_en;
   Instance SIB_1 Of SIB {
      InputPort ijtag_reset = reset_n;
      InputPort ijtag_si = TDI;
      InputPort ijtag_from_so = TDR_4.so, TDR_3.so, TDR_2.so, TDR_1.so;
      InputPort ijtag_sel = sel;
      InputPort ijtag_tck = TCK;
      InputPort ijtag_se = shift_en;
      InputPort ijtag_ce = capture_en;
      InputPort ijtag_ue = update_en;
   }
   Instance TDR_1 Of TDR {
      InputPort reset = reset_n;
      InputPort tck = TCK;
      InputPort si = SIB_1.ijtag_to_si;
      InputPort en = SIB_1.ijtag_to_sel[0];
      InputPort Temp = Sensor_1.Temp;
      InputPort se = shift_en;
      InputPort ce = capture_en;
      InputPort ue = update_en;
   }
   Instance TDR_2 Of TDR {
      InputPort reset = reset_n;
      InputPort tck = TCK;
      InputPort si = SIB_1.ijtag_to_si;
      InputPort en = SIB_1.ijtag_to_sel[1];
      InputPort Temp = Sensor_2.Temp;
      InputPort se = shift_en;
      InputPort ce = capture_en;
      InputPort ue = update_en;
   }
   Instance TDR_3 Of TDR {
      InputPort reset = reset_n;
      InputPort tck = TCK;
      InputPort si = SIB_1.ijtag_to_si;
      InputPort en = SIB_1.ijtag_to_sel[2];
      InputPort Temp = Sensor_3.Temp;
      InputPort se = shift_en;
      InputPort ce = capture_en;
      InputPort ue = update_en;
   }
   Instance TDR_4 Of TDR {
      InputPort reset = reset_n;
      InputPort tck = TCK;
      InputPort si = SIB_1.ijtag_to_si;
      InputPort en = SIB_1.ijtag_to_sel[3];
      InputPort Temp = Sensor_4.Temp;
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
   Instance Sensor_3 Of Sensor {
      InputPort tck = TCK;
      InputPort En = TDR_3.En;
   }
   Instance Sensor_4 Of Sensor {
      InputPort tck = TCK;
      InputPort En = TDR_4.En;
   }
}
