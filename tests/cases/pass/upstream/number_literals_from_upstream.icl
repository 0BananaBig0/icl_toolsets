// Every literal spelling that tests/test_icl_number.py in Honza255/icl_parser
// feeds to IclNumber and expects to be accepted.
//
// That test calls IclNumber(text, base, width) directly, so it supplies the
// base as an argument rather than in the text. Here the same values appear as
// they would in a file, with the base written in the literal — `'bxx` for
// ("xx", "bin"), `'d0235` or a bare `0235` for ("0235", "dec"), and so on.
Module number_literals {
  ScanRegister r {
    // ("xx", "bin") — unknown digits, unsized.
    ResetValue 'bxx;
    // ("'b11_00", "bin") — underscores between binary digits.
    DefaultLoadValue 'b11_00;
    // ("'B11", "bin") — an upper-case base.
    CaptureSource 'B11;
    // ("xx01_x11", "bin") — unknown digits mixed with a separator.
    ScanInSource 'bxx01_x11;
    // ("'d13", "dec") and ("1_2_3", "dec") and ("0235", "dec").
    Attribute a = 'd13;
    Attribute b = 1_2_3;
    Attribute c = 0235;
    // ("'HE", "hex") — an upper-case hex base, unsized.
    Attribute d = 'HE;
    // ("'d 134", "dec") and ("'h 23ff", "hex") — a space after the base.
    Attribute e = 'd 134;
    Attribute f = 'h 23ff;
    // ("'b0101", "bin", 4) and ("'D 3", "dec", 5) — sized, with a space.
    Attribute g = 4'b0101;
    Attribute h = 5'D 3;
    // ("'b0_1x", "bin", 3) — a trailing unknown digit.
    Attribute i = 3'b0_1x;
    // ("'hx", "hex", 7) — an x-extended hex literal.
    Attribute j = 7'hx;
    // ("'h3", "hex", 2).
    Attribute k = 2'h3;
  }
}
