# Literal parameter-width diagnostics

Scope: close the recorded `$WIDTH'H1_FF`, WIDTH -> BITS -> 8 diagnostic gap
in both frontends. Reuse the shared AST/driver and existing Kahn dependency
walk. User authorized autonomous fixes; execute inline in the existing clean
icl-conformance worktree, then independent review and Git delivery.

IEEE1687-2014 §6.3.8(j,o), printed63–64/PDF77–78, explicitly shows
`Parameter SIZE = 2; $SIZE'hF` as an error. Literal numeric defaults and scalar
alias chains can be resolved locally without an instance elaborator. Arithmetic,
inversion, concatenations and external/instance-dependent widths stay deferred.
Do not mis-evaluate arithmetic using the parser's formatting-only reductions.

- [x] Promote the existing expected failure to an ordinary diagnostic test;
  classify it as a negative. Add positive/negative fixtures for bases, forward
  aliases, underscores/comments, zero, boundary bits, module isolation, nested
  blocks and unresolved/compound defaults. Run red against the current build.
- [x] Preserve optional known unsigned literal values in ParsedValue. Propagate
  direct aliases through the existing linear dependency order. Record deferred
  sized literals per module and check resolved nonzero widths with FitsWidth.
  Keep unknown/too-large/compound values unknown, avoiding wraparound and false
  diagnostics. Both frontends continue using the same parser and AST.
- [x] Run focused conformance/live diagnostics, full dynamic/style checks,
  grammar reachability and handwritten coverage; retain unchanged strict gates.
  Measure a long forward-alias chain and the existing representative benchmark.
- [x] Independent review; document exactly which widths resolve and which
  remain deferred. Install/verify both tools. Git delivery follows the completed
  checks under standing commit/push authorization.

Expected cost: O(V+E) default resolution, linear bounded literal conversion.
Existing decimal overflow checking retains its limb-multiplication cost
(O(D * min(D, W/32 + 1)) for D digits and W bits); binary/hex checks are linear.
No recursive dependency evaluation, new grammar, new arbitrary-precision
allocation or per-literal parser process.
