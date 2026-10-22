NameSpace ;
UseNameSpace ;
Module tap_host_mux {

  DataInPort istc_active;

  TCKPort from_tck_buf;
  TMSPort from_tms_buf;
  TRSTPort from_trst_buf;
  ScanInPort from_tdi_buf;
  ScanOutPort to_tdo_buf {
    Source tdo_buf_gate;
    // Use this if the tdo_en is active low
    // Attribute forced_low_output_port_list = "to_tdo_en_buf";
    // Use this if the tdo_en is active high
    Attribute forced_high_output_port_list = "to_tdo_en_buf";
  }

  TCKPort from_tck_istc;
  TMSPort from_tms_istc;
  TRSTPort from_trst_istc;
  ScanInPort from_tdi_istc;
  ScanOutPort to_tdo_istc {
    Source tdo_istc_gate;
  }

  ToTCKPort to_tck_tap {
    Source from_tck_buf;
  }
  ToTMSPort to_tms_tap {
    Source from_tms_buf;
  }
  ToTRSTPort to_trst_tap {
    Source from_trst_buf;
  }
  ScanOutPort to_tdi_tap {
    Source tdi_mux;
  }
  ScanInPort from_tdo_tap;

  ScanMux tdi_mux SelectedBy istc_active {
    1'b0: from_tdi_buf;
    1'b1: from_tdi_istc;
  }
  ScanMux tdo_buf_gate SelectedBy istc_active {
    1'b0: from_tdo_tap;
  }
  ScanMux tdo_istc_gate SelectedBy istc_active {
    1'b1: from_tdo_tap;
  }
  ScanInterface tap_host {
    Port to_tck_tap;
    Port to_tms_tap;
    Port to_trst_tap;
    Port to_tdi_tap;
    Port from_tdo_tap;
  }
  ScanInterface istc_client {
    Port from_tck_istc;
    Port from_tms_istc;
    Port from_trst_istc;
    Port from_tdi_istc;
    Port to_tdo_istc;
  }

  ScanInterface buf_client {
    Port from_tck_buf;
    Port from_tms_buf;
    Port from_trst_buf;
    Port from_tdi_buf;
    Port to_tdo_buf;
  }

 
}
