// The two standard TAP module names, used as attribute names.
//
// No real file does this — `STD_1149_1_2001` and `STD_1149_1_2013` appear only
// after `Of`, as the target of an AccessLink, and that position has its own
// rule (`std_tap_kw`). This file exists because *every other* keyword may be
// an attribute name, and making these two an exception is how the two lists
// first drifted apart: a keyword accepted in one place and rejected in another,
// which is exactly the bug `any_keyword` was introduced to make impossible.
//
// So they are treated uniformly, and this pins the uniform treatment.
Module m {
  Instance i Of M {
    STD_1149_1_2001 = 1'b0;
    STD_1149_1_2013 = 1'b1;
  }
}
