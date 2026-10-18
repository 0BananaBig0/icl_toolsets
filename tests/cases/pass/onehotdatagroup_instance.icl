Module m {
  // An instance inside a one-hot data group: the published grammar lists
  // `instance_def` among `oneHotDataGroup_item`, not only at module level.
  //
  // Accepting it needed `of_block_kw` to stop being a bare attribute name:
  // `Instance` was otherwise ambiguous between "an attribute called Instance"
  // and "an instance header", a genuine shift/reduce conflict.
  OneHotDataGroup g {
    Instance i Of M {
      InputPort a = b;
    }
    Port d;
  }
}
