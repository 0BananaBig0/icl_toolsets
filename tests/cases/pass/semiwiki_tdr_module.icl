// The instrument module from the SemiWiki article on IEEE P1687 / Tessent
// IJTAG, completed into a whole module.
//
// One line needs a note. The article prints `CaptureEnport CE;` with a
// lower-case p, but the keyword is CaptureEnPort; the line above is spelled
// correctly. Left as printed, `CaptureEnport CE;` is rejected with a line
// number, and that is the right answer: at module level the published grammar
// admits only `attribute_def`, which begins with the keyword Attribute. An
// arbitrary `name value;` statement is not an ICL module item. (Inside a
// block, where an attribute name may be any keyword or identifier, the same
// line is accepted — the asymmetry is deliberate and is listed under Known
// limitations.)
Module TDR {
  ScanInPort SI;
  ScanOutPort SO { Source R[0]; }
  SelectPort EN;
  ShiftEnPort SE;
  UpdateEnPort UE;
  CaptureEnPort CE;
  TCKPort TCK;
  ScanRegister R[7:0] {
    ScanInSource SI;
  }
}
// The article's PDL follows the module, naming it with `iProcsForModule`.
// That keyword belongs to PDL, not ICL, and is not part of this file.
