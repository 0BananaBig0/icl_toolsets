#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""Golden-file tests: the formatted output is compared byte for byte.

The regression corpus checks two invariants — that formatting is idempotent
and that no character is lost. Both are necessary and neither is sufficient.
A formatter that emitted every token on one line, or that dropped all
indentation, would satisfy both: it is idempotent, and stripping whitespace
from input and output would still match. What the invariants cannot see is
whether the output *looks right*.

So a small set of inputs has its expected output stored alongside it, and
this compares exactly. This is what gofmt, clang-format and prettier all do,
and it is deliberately brittle: a change in formatting behaviour is supposed
to break these.

    python3 tests/golden_test.py --binary build/icl-format
    python3 tests/golden_test.py --update      # re-record every expectation

--update exists because the expectations are output, not intent: they are
re-recorded after an intentional change. It is not a way to make a failure go
away — the diff that --update produces is the thing to review.
"""

import argparse
import os
import subprocess
import sys

HERE = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'golden')


def cases():
    out = []
    for name in sorted(os.listdir(HERE)):
        if name.endswith('.icl'):
            out.append(name[:-len('.icl')])
    return out


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--binary', default='build/icl-format')
    parser.add_argument('--update', action='store_true',
                        help='re-record the expected output for every case')
    args = parser.parse_args()

    if not os.path.exists(args.binary):
        print('error: no binary at %s; run make first' % args.binary,
              file=sys.stderr)
        return 2

    passed = 0
    failed = []
    for name in cases():
        src = os.path.join(HERE, name + '.icl')
        exp = os.path.join(HERE, name + '.expected')
        result = subprocess.run([args.binary, src], capture_output=True)
        if result.returncode != 0:
            failed.append('%s: exit %d %s'
                          % (name, result.returncode,
                             result.stderr.decode('utf-8', 'replace')[:70]))
            continue
        actual = result.stdout
        if args.update:
            with open(exp, 'wb') as f:
                f.write(actual)
            passed += 1
            continue
        if not os.path.exists(exp):
            failed.append('%s: no .expected file; run with --update' % name)
            continue
        with open(exp, 'rb') as f:
            expected = f.read()
        if actual == expected:
            passed += 1
            continue
        # A readable diff: the first line that differs, with the two versions.
        a = actual.decode('utf-8', 'replace').split('\n')
        e = expected.decode('utf-8', 'replace').split('\n')
        where = next((i for i in range(max(len(a), len(e)))
                      if i >= len(a) or i >= len(e) or a[i] != e[i]), 0)
        failed.append('%s: differs at line %d\n       got %r\n  expected %r'
                      % (name, where + 1,
                         a[where] if where < len(a) else '<missing>',
                         e[where] if where < len(e) else '<missing>'))

    print('golden: PASS=%d  FAIL=%d' % (passed, len(failed)))
    for line in failed:
        print('  [FAIL] ' + line)
    if failed:
        return 1
    print('All golden files match.')
    return 0


if __name__ == '__main__':
    sys.exit(main())
