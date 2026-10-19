Module m {
  ScanMux mx SelectedBy s {
    // A comment inside a mux body, and an assignment there too.
    foo = 1;
    3'b000 : si;
  }
  LogicSignal ls {
    // A comment inside a logic body, and an assignment there too.
    foo = 1;
    a && b;
  }
  Connection {
    // A comment inside a connection body.
    a -> b;
  }
}
