Module m {
  // `LogicSignal` is a module-level construct: the published grammar's
  // scanRegister_item does not include it. Inside a register, then, it is
  // read as an attribute name followed by a nested block — an attribute name
  // may be any keyword, and `name { ... }` is a block.
  //
  // That is deliberate: a formatter that rejected it would refuse a file it
  // cannot tell from a legal one, and the two spellings are identical here.
  // The alternative — checking attribute names per enclosing block — is the
  // upgrade noted in the ponytail ledger.
  Reg r {
    LogicSignal l {
      a;
    }
    Field f[3:0] {
      ResetValue 1'b0;
    }
  }
}
