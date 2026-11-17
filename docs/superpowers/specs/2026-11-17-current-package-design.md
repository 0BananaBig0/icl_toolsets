# Current package and installation qualification

Refresh the clean source/static archives and five-tool installation evidence at
master f75edb80472d6c892f5d01e71760b5f6a9f93589. Documentation only; no runtime,
test, Makefile, dependency, user installation, editor configuration or push.

Reuse existing dist/install/uninstall and SourceDistribution.check_installed_tools,
LSP checks and the current Task126 normal/SAN/raw coverage/remote51x783 results.
Verify every shipped tracked file against the clean commit, with only .gitignore
excluded and six generated extras. Verify all3994 runtime hashes, ten local and
ten remote product fingerprints, and all shipped non-documentation bytes against
the already-qualified remote source checkout, including generated parser/scanner
and .prebuilt. If these differ, stop evidence reuse and investigate the mismatch.

Fresh local and remote temporary installations must exercise all five versions,
PDL tools, original/flat/generated/reset generator replay, installed ICL LSP and
sentinel-preserving manifest uninstall for dynamic and transferred-static tools.
Remote installation consumes the previously built, hash-qualified current files
without rebuilding them. Record this reuse explicitly: no new remote source-
archive build, full make check, full generator rerun or vendor certificate claim.
Read-only pinned dependencies and authorized own remote/tmp outputs only.

Keep unchanged strict100%/90% failure and broader model/vendor gaps visible.
One independent final review, necessary correction pass, merge direct parent
master/delete branch and archive/sync. Four documentation files only.
