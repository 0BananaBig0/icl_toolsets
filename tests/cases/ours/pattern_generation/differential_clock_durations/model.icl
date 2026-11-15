Module Ratio {
  ClockPort input;
  ToClockPort positive { Source input; FreqMultiplier 3; FreqDivider 2; }
  ToClockPort negative { DifferentialInvOf positive; }
}
Module Osc {
  ToClockPort positive { Period 10ns; }
  ToClockPort negative { DifferentialInvOf positive; }
}
Module Top {
  ClockPort root;
  ClockPort root_n { DifferentialInvOf root; }
  Instance child Of Ratio { InputPort input = root; }
  Instance osc Of Osc;
}
