Module ConnectionHierarchyLeaf {
  ScanInPort si;
  DataInPort data[3:0];
  ResetPort rst;
  ScanRegister value[3:0] {
    ScanInSource si;
    CaptureSource value;
    ResetValue 0;
  }
  ScanOutPort so { Source value[0]; }
  DataOutPort q[3:0] { Source value; }
  ToResetPort reset_out { Source rst; }
  LogicSignal flag { data[0] == 1'b1; }
  Alias local[3:0] = value;
}

Module ConnectionHierarchyMiddle {
  ScanInPort si;
  ResetPort rst;
  Instance leaf Of ConnectionHierarchyLeaf {
    InputPort si = si;
    InputPort data = 4'b0;
    InputPort rst = rst;
  }
  ScanOutPort so { Source leaf.so; }
  DataOutPort q[3:0] { Source leaf.q; }
  ToResetPort reset_out { Source leaf.reset_out; }
}

Module ConnectionHierarchy {
  ScanInPort si;
  ResetPort rst;
  Instance child Of ConnectionHierarchyLeaf {
    InputPort si = si;
    InputPort data = 4'b0;
    InputPort rst = middle.reset_out;
  }
  Instance middle Of ConnectionHierarchyMiddle {
    InputPort si = child.so;
    InputPort rst = rst;
  }
  ScanOutPort so { Source middle.so; }
  DataOutPort observed[3:0] { Source middle.q; }
  DataOutPort reversed[1:0] { Source ~child.q[1:0]; }
  Alias deep[3:0] = middle.leaf.value;
  Alias deep_port[3:0] = middle.leaf.q;
}
