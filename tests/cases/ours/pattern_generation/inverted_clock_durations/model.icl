Module Stage {
  ClockPort input;
  Alias inverse = ~input;
  ToClockPort out { Source inverse; FreqMultiplier 3; FreqDivider 2; }
}
Module Top {
  ClockPort root;
  ClockPort root_n { DifferentialInvOf root; }
  Instance child Of Stage { InputPort input = root; }
  ToClockPort out { Source ~child.out; FreqDivider 3; }
}
