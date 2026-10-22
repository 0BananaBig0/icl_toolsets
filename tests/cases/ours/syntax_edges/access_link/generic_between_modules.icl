Module EdgeAccessLinkGenericFirst {
  AccessLink first Of Generic { "/* not a comment */" { / / } }
}
Module EdgeAccessLinkGenericSecond {
  Parameter WIDTH = 8;
  AccessLink second Of Generic { "// } not a delimiter" { { ; } } }
  DataInPort input[($WIDTH - 1):0];
}
