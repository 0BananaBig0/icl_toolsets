# Language boundary follow-up

Baseline: `605012d`. Continue the authorized five-component audit in the current
checkout. Reuse the preceding combined audit's IEEE/Honza inventory; do not
repeat source collection or change language separation. No commit/push requested.

## Tasks

- [x] Revalidate ICL blocks sequentially (Enum then DataOutPort, then manifest
  order), checking uncovered authored action logic and existing nested fixtures.
- [x] Revalidate PDL blocks in manifest order. For the reproduced iScan prefix
  defect, add source-classified negative and valid-control fixtures, observe
  CLI/LSP failures, fix the shared validator and pass the block before moving on.
- [x] Check downstream Vim lexical boundaries and both shared formatter/LSP
  paths. Review dead code and scanning costs; avoid speculative refactoring.
- [x] Obtain one independent integrated review, then run dynamic regression,
  focused static checks, strict coverage and installation checks. Print combined
  coc.nvim configuration; record actual limits without weakening coverage gates.

## Evidence and decisions

- IEEE 1687-2014 §7.7 (printed154/PDF168) requires a contiguous `0b`/`0x`
  iScan prefix. §7.8 also permits uppercase prefixes. §7.9.12 (169–170/183–184)
  supplies examples with whitespace after the prefix and between digits.
- Baseline accepts `{0 b1010}`. Root cause: check_number(scan=true) strips
  whitespace before checking whether the value began with a valid prefix.
- Keep linear-time scanning and bounded nesting for the existing roughly 1 MiB
  editing workload. No new dependency or Tcl execution is needed.
- Preserve previous acceptance of leading whitespace in multiline scan data;
  only reject whitespace *inside* the two-character numeric prefix.
- Known strict coverage baseline is 97.4% lines / 61.4% branches; do not treat
  334/334 grammar rules or 119/119 scanner rules as full action-branch coverage.

## Execution ledger

- Four malformed-prefix fixtures failed in CLI and LSP before the fix. Prefix
  validation resolved three; the quoted continuation remained incorrectly
  accepted because it was classified as dynamic. Allowing only text and
  continuation tokens in a constant word resolved the remaining failure.
- Ruling: include constant-continuation classification in this fix because the
  original regression demonstrated the bypass. Other escapes and substitutions
  remain conservative; source bytes/ranges are preserved. The connected parser,
  formatter and LSP regression suites test the expanded impact.
- ICL 46 categories / 2,055 cases and PDL 22 / 197 passed sequentially. All 46
  PDL tool test methods passed. Google style was corrected with clang-format;
  style/naming then passed. Full dynamic regression passed with the real ANTLR
  adapter enabled, and the independent final review found no actionable issue.
- Coverage exposed four previously covered legacy continuation-error lines.
  Two LF/CRLF malformed-interpolation fixtures cover them; the branch is retained
  for raw level-0 input. Final PDL counts: 199 cases, 24 in iScan. Dynamic block,
  complete static and installed block suites, and coverage functional checks
  passed. All 720 final PDL files passed both editor corpus drivers; the full
  regression already checked 38,487 unchanged ICL files per editor.
- All four static outputs built. Shared static/installed CLI and protocol tests
  passed, as did PDL tool and large-reference-query checks. Dynamic tools were
  installed into /home/banana/.local/bin and help/version exits verified.
- Strict coverage is still unmet: 97.4% lines / 61.5% branches, with unchanged
  100%/90% thresholds. Checked tasks mean the required checks were executed,
  not that the coverage gate passed. See the updated combined audit report.
