Module Groups {
  DataInPort Source;
  DataInPort input;
  PortGroup inline_group {Source, input}
  PortGroup commented_group {
    // Names remain references when comments change the AST layout.
    Source, input
  }
}
