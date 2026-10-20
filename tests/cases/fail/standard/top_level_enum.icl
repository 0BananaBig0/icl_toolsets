// An Enum outside a module is not ICL.
//
//   iclSource_items : nameSpace_def | useNameSpace_def | module_def
//
// enum_def is a module_item. This is worth pinning because the opposite was
// once asserted in tests/block_review.py — a comment there claimed enum_def
// was "also an iclSource item". The published grammar says otherwise, and so
// does this formatter.
Enum e {
  a = 1;
}
