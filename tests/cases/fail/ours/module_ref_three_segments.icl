// IEEE 1687-2014 section 6.4.7 allows at most one namespace qualifier.
Module m {
  Instance i Of a::b::c;
  Instance j Of a::b::c {
    InputPort p = 1;
  }
  Instance k Of X::x;
}
