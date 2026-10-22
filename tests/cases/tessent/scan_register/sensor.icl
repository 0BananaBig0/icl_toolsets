// Tessent v2023.4 ScanRegister Example 2: spaces normalized; sensor stray semicolon before port body removed.
Module sensor {
	TCKPort        tck;
	TRSTPort       rst;
	ScanInPort     si;
	ScanOutPort    so       {  Source  S_reg[0]; }
	CaptureEnPort  capture_en;
	ShiftEnPort    shift_en;
	UpdateEnPort   update_en;
	SelectPort     sel_en;
	DataInPort     sdata_in [2:0];
	DataInPort     sens_from;
	DataInPort     cntrl [1:0];
ScanRegister S_reg [5:0] {
	ResetValue     6'b000000;
	CaptureSource  sdata_in, sens_from, cntrl ;
	ScanInSource   si;
  }
} 
