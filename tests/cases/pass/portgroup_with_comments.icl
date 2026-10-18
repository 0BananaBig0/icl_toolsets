Module m {
  // A comment inside a PortGroup list. A comment can appear wherever
  // whitespace can, and a formatter must not lose it, so these used to be a
  // syntax error. A `//` comment cannot stay inline — it would comment out the
  // closing brace — so the group goes onto several lines when it has one.
  PortGroup p {
    // leading
    a, b
  }
  PortGroup q {
    a, // between
    b, c // trailing
  }
  PortGroup r { /* inline */ d, e }
}
