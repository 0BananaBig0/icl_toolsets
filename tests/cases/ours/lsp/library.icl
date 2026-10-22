NameSpace lib;
Module Leaf {
  DataInPort input[7:0];
  DataOutPort output[7:0] { Source input; }
  Parameter Width = 8;
}
NameSpace other;
Module Leaf { DataInPort different; }
