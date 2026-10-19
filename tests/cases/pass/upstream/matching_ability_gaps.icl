// Two constructs the reference grammar (Honza255/icl_parser, icl.g4) admits
// that this formatter could not match at all. Both were found by walking the
// g4 rules one at a time and asking what input each can match.

// 1. `signal : (number | reg_port_signal_id | hier_port)` and
//    `number : unsized_number | sized_number | integer_expr`
//    — so an integer expression is a legal signal. `Source 1 + 1;` was
//    rejected: binary operators were allowed after `=` but not inside an
//    attribute value.
//
//    Fixing it meant narrowing the left side of an assignment from `value` to
//    `postfix`. With `value` on both sides the parser could not tell, at
//    `name postfix` with an operator ahead, whether it was reducing the left
//    side of an assignment or the first operand of an expression — a
//    reduce/reduce conflict. Only a name, a range or a hierarchical name ever
//    appears on the left, so the narrowing costs nothing.
Module chip {
  ScanOutPort so_arith {
    Source 1 + 1;
  }
  ScanOutPort so_param {
    Source $W + 1;
  }

  // 2. `oneHotDataGroup_item : instance_def | dataRegister_def |
  //     oneHotDataGroup_portSource`, and instance_def has two bodies:
  //     (';' | ('{' instance_item* '}')).
  //    The semicolon form worked at module level and failed inside a block —
  //    the shared attribute list referenced only the braced rule.
  OneHotDataGroup group {
    Instance inst_no_body Of Instrument;
    Instance inst_qualified Of ns::Instrument;
    Instance inst_with_body Of Instrument {
      InputPort din = top.din[7:0];
    }
    Port data_a;
    DataRegister dr {
      AddressValue 3;
    }
  }
}
