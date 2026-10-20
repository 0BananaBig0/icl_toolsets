// `size` is not an integer expression:
//
//   size : pos_int | '$' SCALAR_ID ;
//
// so a parenthesised expression cannot be a width. `(1+1)'d3` must stay
// rejected, while both $W'd5 and 8'd5 — the two forms size does allow —
// are accepted elsewhere in the corpus.
Module m { Parameter p = (1+1)'d3; }
