# Runtime and public corpus review

Baseline: `cab5af6`. Continue the authorized five-component review in the
current checkout. Preserve separate ICL/PDL trees and each language's shared
formatter/LSP pipeline. No commit or push is requested for this review.

## Tasks

- [x] Review protocol, formatting and Vim boundaries; reproduce confirmed
  defects before changing shared code.
- [x] Fetch additional public ICL/PDL inputs, pin provenance, check duplication,
  and integrate them into CLI/LSP/editor regression without executing PDL.
- [x] Apply minimal fixes, run focused checks, and obtain an independent review.
- [x] Run the complete dynamic regression, static focused checks and style
  checks; document actual results and remaining limitations.

## Scope and checks

Use the existing corpus sizes and runtime baseline (about 170 seconds for the
editor corpus). Avoid new per-file subprocess loops or repeated indexing in
production paths. Protocol fixes should add only constant work per message.
No performance improvement is claimed without measurement. Generated reports
and downloaded reference checkouts stay under ignored `build/`.

The LSP protocol test file is already run by `check-dynamic` and coverage.
Existing fixture runners and editor corpus discovery should be reused. Tests
must verify both servers and continued correct behavior after malformed input.
Known strict coverage shortfall from the preceding review is not a regression
pass and its thresholds must not be weakened.

## Findings and evidence

Both suspected protocol defects reproduced in both servers (four failing
subtests); the shared fix passes. Added one unique public ICL file containing
five modules and two PDL fixtures (original negative and completed positive).
Independent code and corpus reviews found no actionable issues.

The first dynamic run exposed an ambiguous new editor-test anchor, not a
production-code failure. After narrowing that anchor, the complete editor suite
passed. Static focused checks and style/naming checks passed. Coverage functional
tests passed, but the unchanged strict gate fails at 97.5% lines / 61.4% branches.
The final complete dynamic rerun passed with exit 0, including both editors'
whole corpus and 334/334 grammar plus 119/119 scanner rules. Details and commands are in
[the review report](../../runtime-corpus-review.md); logs stay in ignored
`build/review-public/`.
