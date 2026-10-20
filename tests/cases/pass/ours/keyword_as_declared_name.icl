// ICL keywords are not reserved: a declared name may be one.
//
// This is stated in a comment at the top of the reference grammar:
//
//   // ANTLR4 has all keywords reserved. Since ICL keywords are not
//   // reserved, an additional rule will be required to properly handle.
//
// Inside a block that was already true here — `Attribute Source;` and
// `Module 1;` inside an Instance body both parse (keyword_as_attribute_name).
// But the *declaration* positions took a bare identifier, so every one of
// these was a syntax error until now:
//
//   ScanInPort Source;        Module Enum { ... }
//   ScanRegister Source { }   Enum Source { ... }
//   ScanMux Port SelectedBy   Alias Port = ...;
//   Parameter Source = 1;     Attribute Source = 1;
//
// A declared name is now `decl_name : IDENT | any_keyword` in those
// positions — the same rule `attr_name` already used, so the two agree.
//
// Two positions are deliberately NOT changed, and both were measured rather
// than assumed:
//
// 1. An Instance name, in `Instance <name> Of <module>`. Allowing a keyword
//    there is a real shift/reduce conflict: `Of` is itself a keyword, so
//    `Instance Of Of M;` has two parses.
//
// 2. A keyword as a *value* — `Source Enum;`, `SelectedBy Source;`. Adding
//    `any_keyword` to `primary` costs one shift/reduce and one
//    reduce/reduce conflict, because a keyword at the head of a statement is
//    also a block or attribute name.
//
// So a signal still has to have a non-keyword name even though a port does
// not. That is a real asymmetry, and it is the price of a grammar that
// cannot be made conflict-free both ways. Every value below is `si`, `so`
// or `sel` for this reason.
Module Source {
  ScanInPort Source;
  ScanOutPort Port {
    Source so;
  }
  DataInPort Enum[3:0];
  ScanRegister Source {
    ScanInSource si;
  }
  Enum Source {
    a = 1;
  }
  Alias Port = si;
  Parameter Source = 1;
  Attribute Source = 1;
  Attribute Port;
  ScanMux Port SelectedBy sel {
    1'b0 : si;
  }
  LogicSignal Port {
    si & so;
  }
  OneHotScanGroup Port {
    Port si;
  }
  PortGroup Port { Source, Port }
  AllowBroadcastOnScanInterface Port;
}
