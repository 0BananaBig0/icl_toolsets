// Rejected: `1.5` is PDL's DVALUE. ICL numbers have no decimal point.
//
//   PDL : DVALUE : POS_INT '.' POS_INT ;
//   ICL : pos_int : '0' | '1' | POS_INT ;
Module m {
  ScanRegister sr {
    ResetValue 1.5;
  }
}
