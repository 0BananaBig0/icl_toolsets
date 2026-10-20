// The one nesting the grammar does reject, and the reason is structural
// rather than a special case: module_def appears only in iclSource_items,
// so it is not reachable from attr_body at all.
Module outer {
  Module inner {
  }
}
