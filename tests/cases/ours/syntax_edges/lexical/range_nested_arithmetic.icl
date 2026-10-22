Module EdgeRangeNestedArithmetic {
  Parameter WIDTH = 16;
  Parameter BASE = 2;
  DataInPort descending[((($WIDTH / 2) * 3) - (5 % 3)):($BASE - 1)];
  DataInPort ascending[($BASE - 1):(($WIDTH / 2) + 1)];
  DataInPort single[((($WIDTH - 1)))];
}
