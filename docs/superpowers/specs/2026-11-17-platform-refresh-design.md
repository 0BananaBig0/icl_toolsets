# Five-tool platform qualification refresh

Refresh the stale473bc9e packaging/platform checkpoint through masterf018bb2.
This task changes qualification documentation only. Reuse Task116's exact local
full1820.716870s/editor189.933s/SAN32/current77certificates and remote66 evidence
while implementation/tests remain unchanged. No new product behavior, dependency,
Tessent certificate or general model/strict coverage completion claim.

Reuse the established source-distribution and installation checks. Package clean
Root at f018bb2 with generated parser/scanner; verify shipped tracked files match
that checkout and identify every expected generated extra/excluded file. Transfer
archive and all five locally built static ELFs with hashes to a fresh authorized
lianghuaxiao /tmp/icl-platform-f018bb2-20261117. Build dynamic without Bison/Flex
or a static Tcl archive using existing read-only pinned Make/GCC/Z3 dependencies.
Run native targets, all current dynamic and transferred-static generator CLI
suites, installed five-tool version/PDL/LSP/pattern/reset replay, ELF/library
checks and manifest uninstall preserving an unrelated sentinel. Record actual
suite/test counts, environment, archives/input/binary hashes and exit statuses.
No remote static source-build or full remote make check is claimed.

Use old commands and standard tar/hashlib/subprocess helpers. Rebuilding an
unrelated entire local suite or adding a second qualification framework has no
new evidence value. Source-archive build/install and remote candidate tests are
fresh because packaging and platform behavior are the acceptance target. Heavy
builds/tests remain serial per machine, bounded build concurrency2. Record
independent final evidence review, then docs-only commit/merge/delete; no push.
