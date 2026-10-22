// Constructs taken from the regular expressions in
// dtrodr/tree-sitter-icl's grammar.js, one per regex, and checked against
// this formatter. Each line below corresponds to a rule in that grammar.

Module m {
  // string : seq('"', repeat(choice($.string_content, $.escape_sequence)), '"')
  // escape_sequence : seq('\\', choice('\\', '"'))  — only these two escapes
  Attribute esc_backslash = "x\\y";
  Attribute esc_quote = "x\"y";
  Attribute esc_empty = "";

  // time : token(seq(/[0-9][0-9_]*/, optional(choice('s','ms','us','ns','ps','fs','as'))))
  // — the unit is part of the number token, and it is optional.
  ToClockPort c {
    Period 10;
  }
  ToClockPort separated {
    Period 1_0ns;
  }

  // size : choice($.positive_integer, $.parameter_reference)
  // parameter_reference : /\$[a-zA-Z][a-zA-Z0-9_$]*/
  Parameter sized = $W'd5;
  Parameter named = $abc_1;

  // index : $.integer_expression — recursively parenthesized arithmetic
  ScanRegister sr[($W + (2 * 3))];
  ScanRegister sr2[$W - 1:0];

  // concat_number_list : prec.left(seq($.concat_number, repeat(seq('|', $.concat_number))))
  // — `|`, not `,`, is the published spelling; both are accepted here.
  ScanMux mx SelectedBy s {
    1'b0 | 1'b1 : a, b;
  }

  // concat_hier_data_signal : '~'? hier_data_signal (',' '~'? hier_data_signal)*
  Alias x = ~a, b, ~c;

  // i_proc_namespace : optional(scope) name optional(sub_scope)
  DataRegister dr {
    WriteCallBack a::b::c <D>;
    WriteCallBack $a::b::$c <D>;
  }

  // port_enable : seq('Enable', $.signal, ';')  — one signal; a list is
  // rejected per IEEE §6.4.6.7; see classified data_out_port negative cases.
  DataOutPort d {
    Enable a;
  }
}
