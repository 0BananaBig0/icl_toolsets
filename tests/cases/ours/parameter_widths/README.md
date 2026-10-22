# Local parameter-width validation

Constructed from IEEE1687-2014 §6.3.8(j,o), printed63–64/PDF77–78,
and §6.5.4, printed138–140/PDF152–154. The two completed normative
`SIZE=2` examples live separately in `standard/lexical/parameter_size_*.icl`.

These fixtures check numeric literal defaults and aliases, including forward
references, all three bases, underscores/comments, zero widths, partial hex
nibbles, nested uses, keyword names, module isolation and very large numbers.
Instance override references use the enclosing module defaults; the override's
left-hand name does not introduce a new lexical binding in its enclosing module.
The live diagnostic fixture/correction pair is under `ours/diagnostics/`.

Cases named `*_deferred`, `unknown_width`, `unresolved_width`,
`attribute_not_parameter` and `large_width` protect against false diagnostics:
compound or context-dependent values are not treated as literal magnitudes,
Attribute declarations are not Parameter declarations, and bounded metadata
must not wrap large values into an incorrect small width. They test syntax and
conservative local validation, not full parameter elaboration or valid hardware.
For example, concatenating 4'h4 and 4'h4 yields 68, not4; inversion of 8'd8
produces247, not8. Valid values that fit these widths must remain accepted.

All expectations/provenance are in `tests/cases/conformance.json`. Run:

```sh
python3 tests/src/test_icl_conformance.py --binary build/icl-format --block parameter
python3 tests/src/test_icl_diagnostics.py --binary build/icl-lsp
```

## Explicit unresolved semantic case

`alias_arithmetic_unchecked.icl` is the independent review's known limitation
probe: B=4+4 and W=$B imply width8, so `$W'h100` should be rejected by full
elaboration. The current bounded check accepts it because arithmetic is deferred.
It is intentionally **not** a positive conformance classification. It remains
in the trace corpus as evidence of the boundary, not a claim of IEEE validity.
