Module m {
  // A namespace may nest, so `a::b::c` is a legal module reference. It used
  // to be accepted in `Source a::b::c;` but rejected after `Of`, because the
  // two positions used different rules.
  Instance i Of a::b::c;
  Instance j Of a::b::c {
    InputPort p = 1;
  }
  Instance k Of X::x;
}
