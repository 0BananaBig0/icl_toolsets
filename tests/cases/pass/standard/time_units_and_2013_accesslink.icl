// Two gaps found by counting: every keyword literal in the published grammar
// was checked against the whole corpus, and five had zero occurrences.
//
// Four are time units. The scanner accepts all six as part of a number token —
// `10ns` is one literal, not a number followed by an identifier — but only
// `10ns` had ever appeared in a file here. The other five are `as`, `fs`,
// `ps`, `us` and `ms`.
//
// The fifth is STD_1149_1_2013, the 2013 revision of IEEE 1149.1 as an
// AccessLink target. Only the 2001 revision was exercised.
Module m {
  ToClockPort c1 { Period 10as; }
  ToClockPort c2 { Period 10fs; }
  ToClockPort c3 { Period 10ps; }
  ToClockPort c4 { Period 10ns; }
  ToClockPort c5 { Period 10us; }
  ToClockPort c6 { Period 10ms; }
  ToClockPort c7 {
    Period 10ns;
    FreqMultiplier 2;
    FreqDivider 3;
  }
  AccessLink t1 Of STD_1149_1_2001 {
    BSDL_Entity E;
    R { ScanPath { a; } }
  }
  AccessLink t2 Of STD_1149_1_2013 {
    BSDL_Entity E;
    R { ScanPath { a; } }
  }
}
