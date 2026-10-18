Module m {
  // `::` with no namespace in front of it: the global namespace. The published
  // grammar allows it after `Of` — instance_def writes the qualifier as
  // `(namespace_name? '::')?`, namespace optional.
  Instance i Of ::M;
  Instance j Of ::M {
    InputPort p = 1;
  }
  Instance k Of ns::M;
}
