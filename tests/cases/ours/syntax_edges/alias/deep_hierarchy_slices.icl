Module EdgeAliasLeaf { DataInPort bits[7:0]; }
Module EdgeAliasBranch { Instance leaf Of EdgeAliasLeaf; }
Module EdgeAliasRoot {
  Instance branch Of EdgeAliasBranch;
  Alias reversed[7:0] = ~branch.leaf.bits[0:3], branch.leaf.bits[(3 + 4):4];
}
