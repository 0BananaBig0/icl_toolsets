// The two reference grammars disagree here, and this is the side we stay on.
//
// Honza255/icl_parser:
//   AccessLinkGeneric_def : 'AccessLink' SCALAR_ID 'Of' SCALAR_ID ;
//   fragment AccessLinkGeneric_block : ... // Not used in ANTLR parser
// — no body at all, and the block rule is a fragment its parser never uses.
//
// dtrodr/tree-sitter-icl:
//   access_generic_link_definition : 'AccessLink' id 'Of' id '{'
//       repeat($._access_generic_link_item) '}' ;
//   _access_generic_link_item : // Is this legal? seq('{',...)
//                               $.access_generic_link_text | $.string ;
// — a body of free text and strings, with the author unsure it is legal.
//
// A body is accepted here, but it has to contain ICL: `AccessLink g Of t
// { MyScanInterface; }` works. Unstructured text does not, because this
// formatter reproduces tokens and has no way to keep arbitrary prose aligned.
Module chip {
  ScanInterface si { Port a; }
  AccessLink link Of Generic {
    some free text here
  }
}
