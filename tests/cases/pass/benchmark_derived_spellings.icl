// Spellings taken from the IJTAG-Ecosystem/Benchmarks suite, the standard
// IEEE 1687 benchmark networks. Each appears in a large generated file there;
// collected here in one small file so a failure names the construct rather
// than a 25k-line file.
Module m {
  Parameter size = 8;
  DataInPort DI[7:0];
  DataOutPort DO[1:0];
  ScanInPort SI;
  ScanOutPort SO;

  // From Classic/Kernel: a parameter expression as the width, and a data
  // register with its write path.
  DataRegister dr[$size - 1:0] {
    WriteDataSource DI[$size - 1:0];
    WriteEnSource enable;
  }

  // From Classic/Kernel: RefEnum, a symbolic reset value and an address.
  DataRegister R_id {
    RefEnum Identification;
    ResetValue CORE_ID;
    AddressValue 'h0000;
  }

  // From Standard/Kernel: a one-hot group containing a whole data register.
  OneHotDataGroup M_Core[31:0] {
    DataRegister R_id2 { RefEnum I; ResetValue C; AddressValue 'h0000; }
  }

  // From Standard/Kernel: two selection signals, and indexed selectors.
  DataMux dataMux SelectedBy IdSelectSignal, ReadSignal {
    2'b01 : M1[31:0];
    2'b10 : ID_sys_reg;
  }
  ClockMux CLK_MUX_A SelectedBy SR[0] {
    0: CLK_A;
    1: CLK_B;
  }

  // From Classic: a bare integer as a mux selection, with no base.
  ScanMux mx SelectedBy s {
    0 : a;
    1 : b;
  }

  // From Classic: a concatenation written with no space after the comma, and
  // a parameter expression as an index.
  Alias mode[3:0] = DI[6:5],DI[3:2] { AccessTogether; }
  Alias enable = DI[$size - 1] { iApplyEndState 1'b0; }
  Alias data[2:0] = DI[4],DI[1:0];

  // From Advanced/FlexScan: the `&` form, with and without a leading `~`.
  LogicSignal sel_WI_1 {
    SREG0.DO[0] & SIB_22.toSEL;
  }
  LogicSignal sel_WI_2 {
    ~SREG0.DO[0] & SIB_22.toSEL;
  }

  // From NetworkStructs: three ports in one scan interface.
  ScanInterface scan_client {
    Port SI;
    Port SO;
    Port SEL;
  }
}
