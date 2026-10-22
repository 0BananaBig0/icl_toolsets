// IEEE §6.4.8 example, supplied module and TDI.
Module Example { ScanInPort TDI; ScanRegister TDR[42:0] { ScanInSource TDI; } }
