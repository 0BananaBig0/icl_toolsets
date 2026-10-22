Module EdgeLogicNestedUnknownOutsideComparison {
  DataInPort enable;
  LogicSignal bad { enable && !(((1'bX))); }
}
