# Literal expressions, object navigation, and position indexing

Approved scope: fix the three reproduced findings at commit 5c064fc. Implement
inline in the existing workspace, preserving separate language grammars and
the shared formatter/LSP document pipeline. No commit, push, or installation.

1. Add failing PDL CLI/LSP tests for quoted/bare literal control expressions,
   with valid and dynamic controls. Add PDL-to-ICL navigation cases where enum
   members collide with registers/instances, plus genuine ambiguity controls.
2. Validate all literal expression words using the existing Tcl expression
   parser. Exclude EnumMember declarations from ordinary ICL object lookup.
3. Add shared-server long-line reference and Unicode-position tests. Reproduce
   the latency failure before introducing a shared per-document UTF-16 index.
   Store prefix counts every 256 bytes, bounding each lookup's rescanning while
   using about one size_t per 256 source bytes. Keep existing line indexes and
   exact byte-prefix behavior, including positions within multibyte characters.
4. Verify focused tests, Unicode boundaries and document replacement. Compare
   the same reference workload before/after: ICL 16,000 uses (~517 KB), PDL
   32,000 uses (~288 KB); target <2 seconds per query, measured separately from
   parsing, with no noisy relative-speed assertion in regression.
5. Obtain independent review, fix confirmed findings, build dynamic/static,
   run complete dynamic regression, focused static checks, style, cleanliness,
   and strict coverage. Investigate failures without weakening thresholds.

Baseline: equivalent single-line reference queries took about 3.28 s (ICL)
and 3.65 s (PDL), versus 0.06 s / 0.11 s multiline on WSL2, GCC 16.2.1 -O2.
The existing strict coverage gate is already failing; report its final result
separately from functional regression. Logs/generated artifacts stay ignored.

Independent review follow-up: a PDL vector suffix such as `1[0]` is dynamic in
a Tcl expression even though the existing PDL heuristic calls it literal.
Track that distinction and retain dynamic behavior for control flow and expr;
new CLI/LSP cases reproduced the false rejection before the correction. Also
add both shared text headers to unit-test prerequisites; dry-run dependency
tests reproduced stale unit builds for both headers before the Makefile fix.

Full-regression investigation: gramcov crashed on an existing enum fixture
because format_job.o retained the old ParsedDocument layout while
icl_semantics.o was rebuilt. Its recipes lacked depfiles. A dependency test
reproduced the missing header invalidation (holding generated headers fixed to
avoid misleading Make dry-run behavior). Enable gramcov depfiles and rebuild
legacy objects when the Makefile changes, then rerun the complete regression.

Completed: three fixes, independent-review corrections, dependency regression
repair, dynamic/static builds, focused static checks, and the complete dynamic
regression rerun (exit 0). Style and artifact checks pass. Strict coverage's
functional suites pass, but its unchanged 100%/90% gate remains below target.
Measurements and full results: `docs/review-expression-navigation-position.md`.
