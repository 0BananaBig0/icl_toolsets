Module EdgeLogicNestedIntegerRhs {
  Parameter WIDTH = 8;
  DataInPort a[3:0];
  LogicSignal match { a == ((($WIDTH * 2) - (9 / 3)) % 16); }
}
