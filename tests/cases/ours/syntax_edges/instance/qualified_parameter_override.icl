NameSpace EdgeFixtureLibrary;
Module EdgeQualifiedLeaf {
  Parameter WIDTH = 4;
  DataInPort input[($WIDTH - 1):0];
  DataOutPort output[($WIDTH - 1):0] { Source input; }
}
NameSpace EdgeFixtureUsers;
Module EdgeQualifiedParent {
  DataInPort data[7:0];
  UseNameSpace EdgeFixtureLibrary;
  Instance child Of EdgeFixtureLibrary /* qualifier */ :: EdgeQualifiedLeaf {
    Parameter WIDTH = (2 * (1 + 3));
    InputPort input = data[7:4], ~data[3:0];
  }
  DataOutPort output[7:0] { Source child.output; }
}
