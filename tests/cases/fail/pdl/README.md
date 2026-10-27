# PDL cases — rejected on purpose

ICL and PDL are two languages. Both are part of IEEE 1687, and both live in
the same repository upstream (`src/icl_parser/icl.g4` and
`src/pdl_parser/pdl.g4`), which makes them easy to confuse here.

They do not share a lexer:

| | ICL | PDL |
|---|---|---|
| line comment | `//` `/* */` | `#` |
| hex literal | `8'h10`, `'h10` | `0x10` |
| binary literal | `4'b0011`, `'b0011` | `0b101` |
| decimal | `10` | `10`, `1.5`, `1.5e-3` |
| statement end | `;` | `;` **or newline** |
| iScan literal | — | `{0b1010}`, `"0x1F"` |
| line continuation | — | `\` at end of line |

`ARGUMENT_REF` (`$name`), `SCALAR_ID` and the seven time units
(`s ms us ns ps fs as`) are spelled the same in both, so those are *not*
here — they are ICL too.

Every file in this directory is something PDL allows and ICL does not, so
every one of them must be rejected. The point of the directory is not that
these are exotic; it is that a PDL file handed to this tool by mistake
should be reported as the wrong language rather than parsed as a broken ICL
one.

Two of the rejections are lexer rules written for exactly this, in
`src/icl/icl_lexer.l` — `0x`/`0b` and `#`. Before they existed, `0x10` was cut
into NUMBER("0") and IDENT("x10"), and the space-separated value rule
joined them back as `0 x10`: the formatter changed the text of a file while
every invariant still held, because character conservation cannot see a
space that was not there before.
