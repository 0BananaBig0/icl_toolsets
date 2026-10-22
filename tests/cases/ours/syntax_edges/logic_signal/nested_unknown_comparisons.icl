Module EdgeLogicNestedUnknownComparisons {
  DataInPort a[3:0];
  DataInPort b[3:0];
  LogicSignal match {
    ((a == ((4'b1x0X))) && !(b != (4'hX))) || (a == (2'b01, 2'bxx));
  }
}
