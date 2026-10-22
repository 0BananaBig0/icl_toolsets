Module Leaf { DataInPort pinA; } Module Mid { Instance U2 Of Leaf; } Module M { DataInPort pinA; Instance U2 Of Leaf; Instance U1 Of Mid; Alias a = pinA; Alias b = U2.pinA; Alias c = U1.U2.pinA; }
