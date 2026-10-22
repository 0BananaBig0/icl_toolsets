Module EdgeLogicNestedBasedArithmetic {
  DataInPort a[3:0];
  LogicSignal bad { a == ((4'h1) + (2 * 3)); }
}
