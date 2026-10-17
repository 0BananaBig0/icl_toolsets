Module M1 {
  DataInPort In1;
  DataInPort In2;
  DataOutPort Out1 {
    Source I1.B;
  }
  DataOutPort Out2 {
    Source I2.Z;
  }
  Instance I1 Of M2 {
    InputPort A = In2;
    InputPort C = I2.Y;
  }
  Instance I12 Of M3 {
    InputPort X = I1.B;
    InputPort W = In1;
  }
}
