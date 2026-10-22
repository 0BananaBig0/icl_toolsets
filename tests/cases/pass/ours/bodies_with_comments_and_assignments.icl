Module m {
  ScanMux mx SelectedBy s {
    // A comment inside a mux body, and an assignment there too.
    Attribute foo = 1;
    3'b000 : si;
  }
  LogicSignal ls {
    // A comment before the single LogicSignal expression.
    a && b;
  }
}
