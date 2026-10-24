# Review cleanup and boundary fixes

Scope authorized by the review/optimization request: fix reproduced formatter
indent parsing and Tcl namespace navigation defects, and remove an avoidable
full-source copy in the shared LSP open/change path. Retain language separation,
existing APIs, source spelling, and full-sync/version/overlay behavior.

1. Add failing shared CLI tests for malformed/overflowing indentation and
   valid signed/zero-padded inputs. Use checked integer parsing.
2. Add PDL navigation tests for equivalent repeated Tcl namespace separators,
   variables, references, and single colons inside names. Normalize Tcl names
   before qualification; keep IEEE PDL names separate.
3. Move the owned text from Open into the parser instead of copying it. Verify
   large open/change/formatting and stale versions. Measure peak RSS and latency
   on the same 4 MiB input before/after; do not impose noisy memory thresholds.
4. Run focused checks, independent review, dynamic/static builds, complete
   regression, style, artifact checks, and strict coverage. Investigate failures
   and retain existing coverage thresholds. No commit/push or user installation.

Baseline (WSL2, GCC 16.2.1 -O2, three fresh servers per language): 4,194,319-byte
comment-dominated files used peak RSS 33,464–33,780 KiB for ICL and
26,196–26,400 KiB for PDL. End-to-end open/barrier time was 0.059–0.068 s and
0.035–0.038 s respectively. The goal is to remove redundant ownership copies;
timing noise is not evidence of a speedup.

Completed: failing-then-passing bug tests, 4 MiB ownership/version tests,
independent review, dynamic/static builds, full dynamic regression (exit 0),
static checks, style and artifact checks. Peak RSS decreased by approximately
4 MiB per server on the measured input. The existing strict coverage gate still
fails at 97.5% lines / 61.6% branches; thresholds were not weakened. Details:
`docs/review-cli-namespace-copy.md`.
