# treesitter/ — dtrodr/tree-sitter-icl

20 test cases extracted from `corpus/{basics,declarations,ports}.txt` of an
independent ICL grammar written for tree-sitter.

Those files use tree-sitter's own test format: a name, an input, `---`, and the
expected parse tree as an S-expression. The inputs are whole `source_file`s, so
each one is a complete input for this formatter too. They were extracted
mechanically, not retyped.

One case was **not** carried over: `Attributes declarations`, which puts
`Attribute foo;` at top level. That grammar's own comment on its `source_file`
rule says it is "Illegal by the spec but makes the testing much simpler", and
IEEE 1687's `iclSource_items` is `nameSpace_def | useNameSpace_def |
module_def`. Rejecting it is correct, so it lives in
`fail/treesitter/top_level_attribute.icl` instead.

Also worth noting from that grammar: its keyword set matches ours on every
reserved word. The only literal keywords it has that we do not are the seven
time units (`as fs ps ns us ms s`), which our scanner folds into the number
token instead — `Period 10ns` is one token, not a number followed by an
identifier.
