Module m {
  Instance i Of M {
    // Attribute names may be keywords: the scanner knows them, but inside a
    // block they are just names. These are the ones the corpus never used.
    Attribute Module = 1;
    Attribute Alias = 2;
    Attribute SelectedBy = 3;
    Attribute LogicSignal = 4;
    Attribute Connection = 5;
    Attribute PortGroup = 6;
    Attribute Of = 7;
    Attribute LocalParameter = 8;
    Attribute NameSpace = 9;
    Attribute UseNameSpace = 10;
    Attribute Attribute;
  }
}
