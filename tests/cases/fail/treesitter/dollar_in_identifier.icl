// Two things dtrodr/tree-sitter-icl accepts that IEEE 1687 does not, kept
// rejected here on purpose.

// Its identifier regexes are:
//   enum_name           : /[a-zA-Z][a-zA-Z0-9_$]*/
//   parameter_reference : /\$[a-zA-Z][a-zA-Z0-9_$]*/
//
// and the file says where they came from:
//
//   // from: https://github.com/tree-sitter/tree-sitter-c/blob/master/grammar.js
//
// That line is on parameter_reference. The regexes are copied from the C
// grammar, and `$` is a C leftover — it is not an ICL identifier character.
// The published ANTLR grammar spells both of these without it:
// `enum_name : SCALAR_ID` and `parameter_ref : '$'(SCALAR_ID)`, and SCALAR_ID
// is letters, digits and underscore.
//
// So this is not two implementations disagreeing where one has to be chosen.
// One of them is quoting a rule from another language.
Enum E$x {
  a = 1;
}
