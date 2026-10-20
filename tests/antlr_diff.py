#!/usr/bin/env python3
"""Differential test: our parser against the reference ANTLR parser.

The reference repository (Honza255/icl_parser) ships a *generated* ANTLR
parser — `iclParser.py` / `iclLexer.py` — so it can be run without invoking
ANTLR at all. That makes a real differential test possible: parse every
corpus file with both and compare accept/reject.

The script needs two things this repository does not vendor:

    pip install antlr4-python3-runtime==4.7.2
    ICL_REF_DIR=/path/to/icl_parser   (with src/icl_parser/*.py)

Without them it exits 0 and says so. It is a tool for when those are
available, not a gate — the repository's own checks must not depend on
another project being checked out.

Two findings came out of running it, and both are about the *reference*
being wrong rather than us:

1. `icl_source : iclSource_items+ ;` has **no EOF**. Anything after the last
   complete item is never examined, so `Module m { ... } }` and
   `Module m { ... } GARBAGE GARBAGE` both parse with zero errors. Their
   ACCEPT therefore does not certify the whole file.

2. `AccessLink Tap1 Of STD_1149_1_2001 { ... }` inside a module is rejected
   by their parser, because `accessLink1149_def` and `AccessLinkGeneric_def`
   overlap and ANTLR resolves the ambiguity toward the generic one. We accept
   it — and `Standard_Kernel.icl` and `Standard_MultiCoreAccessLink.icl`,
   both real benchmark files, depend on that.
"""

import argparse
import glob
import os
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(HERE)


def load_reference(ref_dir):
    sys.path.insert(0, os.path.join(ref_dir, 'src'))
    try:
        from antlr4 import CommonTokenStream, FileStream  # noqa: F401
        from antlr4.error.ErrorListener import ErrorListener
        from icl_parser.iclLexer import iclLexer
        from icl_parser.iclParser import iclParser
    except Exception as exc:
        return None, str(exc)

    class Collector(ErrorListener):
        def __init__(self):
            self.n = 0
            self.first = ''

        def syntaxError(self, r, o, line, col, msg, e):
            self.n += 1
            if not self.first:
                self.first = 'line %d:%d %s' % (line, col, msg)

        def reportAmbiguity(self, *a):
            pass

        def reportAttemptingFullContext(self, *a):
            pass

        def reportContextSensitivity(self, *a):
            pass

    def parse(path):
        inp = FileStream(path, encoding='utf-8')
        lex = iclLexer(inp)
        lex.removeErrorListeners()
        c1 = Collector()
        lex.addErrorListener(c1)
        parser = iclParser(CommonTokenStream(lex))
        parser.removeErrorListeners()
        c2 = Collector()
        parser.addErrorListener(c2)
        parser.icl_source()
        total = c1.n + c2.n
        return total == 0, (c1.first or c2.first)

    return parse, None


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--binary', default=os.path.join(ROOT, 'build', 'icl-format'))
    ap.add_argument('--ref-dir', default=os.environ.get('ICL_REF_DIR', ''))
    args = ap.parse_args()

    if not args.ref_dir:
        print()
        print('antlr diff: ICL_REF_DIR not set — skipped (needs the reference')
        print('            repository and antlr4-python3-runtime==4.7.2)')
        return 0
    parse, err = load_reference(args.ref_dir)
    if parse is None:
        print()
        print('antlr diff: reference parser unavailable (%s) — skipped' % err)
        return 0

    binary = os.path.abspath(args.binary)
    files = sorted(glob.glob(os.path.join(ROOT, 'tests', 'cases', '**', '*.icl'),
                             recursive=True))
    agree = over = under = 0
    print()
    for path in files:
        ours = subprocess.run([binary, path],
                              capture_output=True).returncode == 0
        theirs, why = parse(path)
        if ours == theirs:
            agree += 1
        elif ours:
            over += 1
            print('  we-only  %s  (%s)' % (os.path.relpath(path, ROOT), why[:60]))
        else:
            under += 1
            print('  GAP      %s  (they accept, we reject)' %
                  os.path.relpath(path, ROOT))
    print('antlr diff: %d files — agree %d, we-only %d, gaps %d'
          % (len(files), agree, over, under))
    print('  "we-only" is expected: we accept Origen `#include` and reject')
    print('  nothing the standard requires. "GAP" is the column that matters.')
    return 1 if under else 0


if __name__ == '__main__':
    sys.exit(main())
