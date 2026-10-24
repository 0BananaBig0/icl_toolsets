# PDL symbol identity and namespace frame review

Authorized scope: reproduce and correct symbol-navigation defects found during
the code review, while removing redundant key serialization. Keep the existing
shared PDL AST and Tcl/ICL language separation.

1. Add failing LSP tests for distinct Tcl names containing newline characters
   and for namespace bodies entered from procedure scopes. Check definitions,
   references, completion, and unknown namespaces where applicable.
2. Replace delimiter-serialized procedure keys with tuple keys using the
   existing variable/member index design. Reset the variable owner when entering
   a namespace evaluation frame; preserve the parent context for substitutions.
3. Review the changed code independently, run focused dynamic/static tests,
   complete regression, style/artifact checks, and strict coverage. Investigate
   failures without weakening coverage thresholds. Record residual limitations
   and measured evidence in docs; do not commit, push, or install.

Performance scope: preserve indexed procedure lookup (logarithmic map lookup),
avoid concatenating composite keys, and run the existing 32,000-symbol scaling
regression. No speculative cache or claimed unmeasured speedup.

Completed: four regression cases, tuple indexing, namespace frame correction,
independent review and its completion-context follow-up, focused static checks,
full dynamic regression (exit 0), formatting/naming and artifact checks. Strict
coverage remains below the unchanged thresholds at 97.5% lines / 61.7% branches.
See `docs/review-symbol-identity.md` for findings and validation details.
