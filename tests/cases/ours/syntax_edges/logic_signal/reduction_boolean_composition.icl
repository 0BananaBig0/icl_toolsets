Module EdgeLogicReductionBooleanComposition {
  DataInPort a[3:0];
  DataInPort b[3:0];
  LogicSignal result {
    ((&a) && (|b)) || ((^a) & !(^b)) ^ ((a[0] | b[0]) & ~a[1]);
  }
}
