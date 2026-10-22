Module EdgeLogicNestedRhsSignal {
  DataInPort a[3:0];
  DataInPort b[3:0];
  LogicSignal bad { a == (((b[3:0]))); }
}
