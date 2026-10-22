Module Leaf { DataInPort Source; DataOutPort out { Source Source; } }
Module Mid { Instance leaf Of Leaf; }
Module Main {
  Parameter Width = 8;
  Instance mid Of Mid;
  Alias result = mid.leaf.out;
}
