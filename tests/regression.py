#!/usr/bin/env python3
"""Regression corpus runner.

Every case is a single file under `tests/cases/`:

    pass/   valid ICL.   Must be accepted (exit 0), idempotent, and
                         character-preserving: stripping all whitespace from
                         input and output must give the same string.
    fail/   invalid ICL. Must be rejected with exit 2.

Adding a case is dropping a file into a directory. No test code changes. The
corpus *is* the regression suite, so a change that breaks an old case is a
decision, not a nuisance:

    the code is wrong -> fix the code
    the case is wrong -> fix or delete the case, and say which in the commit

Either way the failure is investigated, never skipped.

Usage:
    python3 tests/regression.py [--binary build/icl-format]

Exit code 0 when every case passes, 1 otherwise.
"""

import argparse
import os
import tempfile
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
CASES = os.path.join(HERE, 'cases')


def strip_ws(text):
    return ''.join(text.split())


def cases(folder):
    """Sorted (name, text) for every `.icl` file under `folder`, recursively.

    Recursive, not just the direct children: the benchmark networks live in
    `pass/benchmarks/`, and a runner that read only the top level would have
    reported them as passing without ever opening them. Names are relative to
    `folder` so a failure in a subdirectory is identifiable.
    """
    path = os.path.join(CASES, folder)
    if not os.path.isdir(path):
        return []
    out = []
    for root, dirs, files in os.walk(path):
        dirs.sort()
        for name in sorted(files):
            if not name.endswith('.icl'):
                continue
            full = os.path.join(root, name)
            rel = os.path.relpath(full, path)
            with open(full, 'r', encoding='utf-8') as f:
                out.append((rel, f.read()))
    return sorted(out)


def run(binary, path):
    return subprocess.run([binary, path], capture_output=True)


def run_as_file(binary, text):
    """Run the tool on a real file, not just on stdin.

    Every case used to go through stdin only, so a defect that appeared solely
    when reading a named file — the BOM was one: stripped from a file, rejected
    from stdin — was invisible to the whole corpus. Each case is now run both
    ways, and both must agree.
    """
    with tempfile.NamedTemporaryFile('w', suffix='.icl', delete=False,
                                     encoding='utf-8') as f:
        f.write(text)
        path = f.name
    try:
        result = subprocess.run([binary, path], capture_output=True)
    finally:
        os.unlink(path)
    result.stdout = result.stdout.decode('utf-8', 'replace')
    result.stderr = result.stderr.decode('utf-8', 'replace')
    return result


def run_stdin(binary, text):
    # Bytes, not text: the corpus deliberately includes a BOM and non-UTF-8
    # byte sequences, and decoding them here would crash the runner instead of
    # testing the tool.
    result = subprocess.run([binary], input=text.encode('utf-8'),
                            capture_output=True)
    result.stdout = result.stdout.decode('utf-8', 'replace')
    result.stderr = result.stderr.decode('utf-8', 'replace')
    return result


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--binary', default='build/icl-format')
    args = parser.parse_args()

    if not os.path.exists(args.binary):
        print('missing binary: %s' % args.binary)
        return 1

    passed = 0
    failed = []

    def check(name, ok, detail=''):
        nonlocal passed
        if ok:
            passed += 1
        else:
            failed.append('%s  %s' % (name, detail))
            print('  [FAIL] %s  %s' % (name, detail))

    print('=' * 72)
    print('FAIL: invalid ICL must be rejected (exit 2)')
    print('=' * 72)
    for name, text in cases('fail'):
        for label, result in (('stdin', run_stdin(args.binary, text)),
                              ('file', run_as_file(args.binary, text))):
            check('%s [%s]' % (name, label), result.returncode == 2,
                  'rc=%d output=%r' % (result.returncode, result.stdout[:60]))

    print()
    print('=' * 72)
    print('PASS: valid ICL must format cleanly')
    print('=' * 72)
    for name, text in cases('pass'):
        for label, result in (('stdin', run_stdin(args.binary, text)),
                              ('file', run_as_file(args.binary, text))):
            tag = '%s [%s]' % (name, label)
            if not check(tag, result.returncode == 0,
                         'rc=%d err=%s' % (result.returncode,
                                           result.stderr.strip()[:70])):
                continue
            output = result.stdout
            if strip_ws(text) != strip_ws(output):
                check(tag + ' | no char lost', False,
                      'input=%r output=%r' % (strip_ws(text)[:40],
                                              strip_ws(output)[:40]))
                continue
            second = run_stdin(args.binary, output)
            if second.returncode != 0 or second.stdout != output:
                check(tag + ' | idempotent', False,
                      'second run differs (rc=%d)' % second.returncode)
                continue
            check(tag + ' | clean', True)

    print()
    print('TOTAL: PASS=%d  FAIL=%d' % (passed, len(failed)))
    if failed:
        print('%d failing case(s):' % len(failed))
        for line in failed:
            print('  ' + line)
        return 1
    print('All regression cases passed.')
    return 0


if __name__ == '__main__':
    sys.exit(main())
