#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""Mutation fuzzing: the tool must never crash, whatever it is given.

The regression corpus proves the tool does the right thing for inputs we
thought of. Fuzzing asks the other question — what it does for inputs nobody
thought of — and the answer required is narrow but absolute:

  * exit 0 (accepted) or 2 (rejected with a line number);
  * never a signal, never an abort, never a non-zero code that is neither;
  * if accepted, the two invariants still hold: idempotent, and no character
    lost.

That is the contract parsers are held to (clang and rustc both fuzz their
front ends on exactly this basis), and it is what makes "rejected with a line
number" trustworthy: a crash would also be a rejection, just an unusable one.

Mutations are derived from real corpus files rather than generated from
scratch, so every mutant starts from something the grammar accepts — that is
where the interesting boundaries are. The seed is fixed, so a failure is
reproducible and can be added to `tests/cases/` as a regression case.

    python3 tests/fuzz.py --binary build/icl-format
    python3 tests/fuzz.py --binary build/icl-format --iterations 5000
"""

import argparse
import os
import random
import re
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
CASES = os.path.join(HERE, 'cases', 'pass')

TOKENS = ['{', '}', ';', ',', '[', ']', ':', '=', '::', "'b1", '~', '//',
          '/*', '*/', 'Module', 'Instance', 'Of', 'SelectedBy', '$P', '"s"']


def strip_ws(text):
    return ''.join(text.split())


def sources():
    """Small corpus files: a mutant of a 25k-line file tells you little."""
    out = []
    for root, dirs, files in os.walk(CASES):
        dirs.sort()
        for name in sorted(files):
            if not name.endswith('.icl'):
                continue
            path = os.path.join(root, name)
            if os.path.getsize(path) < 4000:
                out.append(path)
    return out


def mutate(text, rng):
    op = rng.randrange(6)
    if not text:
        return text
    if op == 0:                                    # delete a character
        i = rng.randrange(len(text))
        return text[:i] + text[i + 1:]
    if op == 1:                                    # duplicate a line
        lines = text.split('\n')
        if not lines:
            return text
        i = rng.randrange(len(lines))
        return '\n'.join(lines[:i] + [lines[i]] + lines[i:])
    if op == 2:                                    # insert a token
        i = rng.randrange(len(text) + 1)
        return text[:i] + rng.choice(TOKENS) + text[i:]
    if op == 3:                                    # swap two characters
        if len(text) < 2:
            return text
        i = rng.randrange(len(text) - 1)
        return text[:i] + text[i + 1] + text[i] + text[i + 2:]
    if op == 4:                                    # truncate
        return text[:rng.randrange(len(text))]
    return text.replace(';', '', 1)                # drop a terminator


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--binary', default='build/icl-format')
    parser.add_argument('--iterations', type=int, default=600)
    parser.add_argument('--seed', type=int, default=20241019)
    args = parser.parse_args()

    if not os.path.exists(args.binary):
        print('error: no binary at %s' % args.binary, file=sys.stderr)
        return 2

    rng = random.Random(args.seed)
    pool = sources()
    if not pool:
        print('error: no corpus files under %s' % CASES, file=sys.stderr)
        return 2

    accepted = rejected = 0
    failures = []
    for n in range(args.iterations):
        src = open(rng.choice(pool), 'r', encoding='utf-8',
                   errors='replace').read()
        mutant = mutate(src, rng)
        result = subprocess.run([args.binary], input=mutant.encode('utf-8'),
                                capture_output=True)
        rc = result.returncode
        if rc not in (0, 2):
            failures.append('iteration %d: exit %d (signal %d)'
                            % (n, rc, -rc if rc < 0 else 0))
            open(os.path.join('/tmp', 'fuzz_fail_%d.icl' % n), 'w').write(
                mutant)
            continue
        if rc == 2:
            rejected += 1
            # A rejection must say where. Otherwise it is a refusal, not an
            # error message, and the user has nothing to act on.
            err = result.stderr.decode('utf-8', 'replace')
            if 'line' not in err:
                failures.append('iteration %d: rejected with no line number: %r'
                                % (n, err[:70]))
            continue
        accepted += 1
        out = result.stdout.decode('utf-8', 'replace')
        if strip_ws(mutant) != strip_ws(out):
            failures.append('iteration %d: accepted but lost characters' % n)
            continue
        second = subprocess.run([args.binary], input=out.encode('utf-8'),
                                capture_output=True)
        if second.returncode != 0 or second.stdout.decode('utf-8', 'replace') != out:
            failures.append('iteration %d: accepted but not idempotent' % n)

    print('fuzz: %d mutants — %d accepted, %d rejected, %d failures'
          % (args.iterations, accepted, rejected, len(failures)))
    for line in failures[:10]:
        print('  [FAIL] ' + line)
    if failures:
        print('  (mutants written to /tmp/fuzz_fail_*.icl)')
        return 1
    print('No crashes; every mutant was either formatted or cleanly rejected.')
    return 0


if __name__ == '__main__':
    sys.exit(main())
