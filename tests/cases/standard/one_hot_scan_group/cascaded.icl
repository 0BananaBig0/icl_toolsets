// IEEE 1687-2014 section 6.4.14 example; ellipses removed, M3 register/control supplied.
Module M1 {
 ScanOutPort SO { Source M1; }
 OneHotScanGroup M1 { Port child1.SO; Port child2.SO; Port child3.SO; }
 Instance child1 Of M2; Instance child2 Of M2; Instance child3 Of M2;
}
Module M2 {
 ScanOutPort SO { Source M1; }
 OneHotScanGroup M1 { Port child1.SO; Port child2.SO; Port child3.SO; }
 Instance child1 Of M3; Instance child2 Of M3; Instance child3 Of M3;
}
Module M3 {
 ScanInPort SI; DataInPort En1;
 ScanRegister R1 { ScanInSource SI; }
 ScanOutPort SO { Source R1[0]; Enable En1; }
}
