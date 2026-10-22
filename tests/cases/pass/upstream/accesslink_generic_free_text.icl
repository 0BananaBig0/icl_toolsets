// IEEE 1687-2014 section 6.4.17 requires a Generic body and permits free text.
// Honza leaves its block lexer fragment unused; the normative grammar governs.
Module chip {
  ScanInterface si { Port a; }
  AccessLink link Of Generic {
    some free text here
  }
}
