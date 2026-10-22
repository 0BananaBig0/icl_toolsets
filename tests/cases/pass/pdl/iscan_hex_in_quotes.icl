// Accepted, and must stay accepted — see the note below.
//
// `"0x1F"` is a STRING here, and a string's contents are not lexed as
// numbers. PDL's iScan hex form is also spelled `"0x..."`, so the two look
// alike; the difference is that this one is inside an ICL string, where any
// bytes at all are legal.
//
// This is therefore the one PDL-looking spelling that is ICL, and it lives
// in pass/pdl so that the resemblance is written down next to the cases that
// do fail.
Module m {
  ScanRegister sr {
    Attribute text = "0x1F";
  }
}
