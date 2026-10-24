# Encoding, qualified Tcl, and workspace event fixes

Approved scope: fix the four confirmed review findings without changing CLI
encoding policy or ICL/PDL language separation.

1. Add failing shared-server tests for invalid UTF-8 disk files, equivalent URI
   deletion events (including symlinks), and duplicate close reads. Add PDL CLI
   and LSP tests for global Tcl builtin qualification and nested diagnostics.
2. Reuse JSON's UTF-8 validation at the LSP disk boundary; keep JSON behavior.
   Normalize only supported global Tcl builtins for recognition, preserving
   source spelling, user procedure lookup, and PDL level-0 restrictions.
3. Normalize disk-event URI encoding without resolving away symlink identity;
   reload each required identity once on close, retaining overlay semantics.
4. Run focused tests, inspect measured read volume, obtain independent review,
   then build dynamic/static tools and run full regression, style, cleanliness,
   and strict coverage. Investigate failures without weakening checks.

Performance acceptance: one ordinary close reads one file, with a bounded
protocol-I/O allowance. Existing symlink retarget/overlay regressions must pass.
Generated artifacts and logs stay ignored. No commit, push, or installation is
part of this request. Record results, including any existing coverage shortfall.

Completed: all four fixes, failing-then-passing focused tests, independent
review and its additional invalid-encoding close-read correction, dynamic and
static builds, full dynamic regression, static checks, and style checks.
Strict coverage remains below its existing thresholds; see
`docs/review-encoding-qualified-tcl.md` for measurements and verification.
