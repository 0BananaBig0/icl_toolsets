// From dtrodr/tree-sitter-icl, corpus/declarations.txt:
//
//   Attributes declarations
//   Attribute foo;
//   Attribute foo = 1;
//   ...
//
// That grammar's own source_file rule says why this must be rejected here:
//
//   source_file: $ => repeat(choice(
//       $._icl_source_items,
//       // Illegal by the spec but makes the testing much simpler
//       $._module_statement,
//   )),
//
// "_module_statement" at top level is admitted by that author for testing
// convenience and is marked illegal by him. IEEE 1687's iclSource_items is
// nameSpace_def | useNameSpace_def | module_def — no attribute. So this file
// must be rejected, and rejecting it is not a gap.
Attribute foo;
Attribute foo = 1;
Attribute foo = "bar";
Attribute foo = 1, 2;
Attribute foo = "bar", "baz";
