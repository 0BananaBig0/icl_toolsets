// Self-constructed hierarchy/resource cases for IEEE 6.4.7(v) data defaults.
Module DataBusLeaf {
  Parameter W = 8;
  DataInPort data[$W-1:0];
  DataOutPort out[$W-1:0] { Source data; }
}
Module DataBusMiddle {
  Parameter W = 8;
  DataInPort data[0:$W-1];
  DataOutPort out[$W-1:0] { Source group; }
  OneHotDataGroup group[$W-1:0] {
    Instance leaf Of DataBusLeaf { Parameter W = $W; AddressValue 0; }
  }
}
Module NestedDataTop {
  Parameter W = 8;
  DataInPort data[$W-1:0];
  DataOutPort out[$W-1:0] { Source group; }
  OneHotDataGroup group[$W-1:0] {
    Instance a Of DataBusMiddle { Parameter W = $W; AddressValue 0; }
    Instance b Of DataBusMiddle { Parameter W = $W; AddressValue 1; }
    Instance c Of DataBusMiddle { Parameter W = $W; AddressValue 2; }
  }
  Instance unaddressed Of DataBusLeaf { Parameter W = $W; }
}
Module DataBudgetTop {
  Instance child Of NestedDataTop { Parameter W = 1048576; }
}
Module MissingDataTop {
  OneHotDataGroup group[7:0] {
    Instance child Of DataBusLeaf { AddressValue 0; }
  }
}
