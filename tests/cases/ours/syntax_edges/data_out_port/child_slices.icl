Module EdgeOutputSliceLeaf {
  DataInPort in[7:0];
  DataOutPort out[7:0] { Source in; }
}
Module EdgeOutputSliceParent {
  Parameter WIDTH = 8;
  DataInPort input[7:0];
  DataInPort enable;
  Instance child Of EdgeOutputSliceLeaf { InputPort in = input; }
  DataOutPort reversed[7:0] {
    Source child.out[($WIDTH - 1):4], ~child.out[3:0];
    Enable ~enable;
  }
}
