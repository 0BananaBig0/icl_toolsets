Module Example { ScanInPort SI; ScanRegister RegA[3:0] { ScanInSource SI; } ScanOutPort SO1 { Source RegA[0]; Attribute LaunchEdge = "Rising"; } }
