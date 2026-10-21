#!/usr/bin/env python3
"""Reports line and branch coverage of the C++ sources.

Both numbers matter and neither implies the other: a line can be executed
with only one of its branches taken, and a branch can be exercised on a line
gcov counts as covered for other reasons. Run with --strict to fail when
either falls below --min-line / --min-branch.

Usage: coverage.py [--min-line PCT] [--min-branch PCT] [--strict]
Assumes the tree was already built with --coverage and the test suites run.
"""

import argparse
import glob
import os
import re
import subprocess
import sys

# The two hand-written grammar files are here as well, and are the reason this
# list is not just "the .cc files": gcov honours the #line directives that
# bison and flex emit, so `icl_parser.yy.gcov` and `icl_lexer.l.gcov` report
# against the .yy/.l themselves — prologue, semantic actions and scanner
# actions, which is exactly the hand-written logic the brief asks to be
# covered. The generated .cc files are deliberately NOT listed: their other
# lines are bison's tables and flex's DFA, and a table row that no test
# reaches says nothing about this code.
SOURCES = ['icl_driver.cc', 'format_tree.cc', 'format_job.cc', 'json.cc',
           'icl_format_main.cc', 'icl_lsp_main.cc',
           'icl_parser.yy', 'icl_lexer.l']


def line_coverage(path):
    total, covered, uncovered = 0, 0, []
    for line in open(path):
        m = re.match(r'^\s*(#####|[0-9]+):\s*(\d+):', line)
        if not m:
            continue
        total += 1
        if m.group(1) == '#####':
            uncovered.append(int(m.group(2)))
        else:
            covered += 1
    return covered, total, uncovered


def branch_coverage(path):
    """Counts each branch point once; a branch is 'taken' if ever non-zero."""
    current, seen = None, {}
    for line in open(path):
        m = re.match(r'^\s*[0-9#*-]+:\s*(\d+):', line)
        if m:
            current = int(m.group(1))
            continue
        if 'branch' not in line:
            continue
        b = re.search(r'branch\s+(\d+)\s+(?:taken\s+(\d+))?(?:\s*\((\w+)\))?',
                      line)
        if not b or current is None:
            continue
        # 'throw' paths are unreachable: this code base builds with -fno-exceptions.
        if b.group(3) == 'throw':
            continue
        count = b.group(2)
        if count is None:      # bare "(fallthrough)" with no counter
            continue
        key = (current, b.group(1))
        seen[key] = max(seen.get(key, 0), int(count))
    taken = sum(1 for v in seen.values() if v > 0)
    untaken = sorted({k[0] for k, v in seen.items() if v == 0})
    return taken, len(seen), untaken


def source_line(name, number):
    """The |number|-th line of a measured source file, trimmed.

    The .gcov files already carry the source text, but reading it from the
    original keeps this independent of gcov's formatting and of whether the
    file was emitted at all.
    """
    for directory in ('src', '.'):
        path = os.path.join(directory, name)
        if os.path.exists(path):
            with open(path, encoding='utf-8', errors='replace') as handle:
                lines = handle.read().split('\n')
            if 1 <= number <= len(lines):
                return lines[number - 1].strip()[:96]
    return '<source not found>'


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--min-line', type=float, default=0.0)
    parser.add_argument('--min-branch', type=float, default=0.0)
    parser.add_argument('--strict', action='store_true')
    args = parser.parse_args()

    if not glob.glob('*.gcov'):
        print('error: no .gcov files; run gcov first', file=sys.stderr)
        return 2

    tl = cl = tb = cb = 0
    rows = []
    for name in SOURCES:
        path = name + '.gcov'
        if not os.path.exists(path):
            continue
        c, t, ul = line_coverage(path)
        b, bt, ub = branch_coverage(path)
        cl += c; tl += t; cb += b; tb += bt
        rows.append((name, c, t, b, bt, ul, ub))

    print('%-22s %14s %16s' % ('file', 'lines', 'branches'))
    for name, c, t, b, bt, ul, ub in rows:
        print('%-22s %5d/%-5d %5.1f%% %5d/%-5d %6.1f%%' %
              (name, c, t, 100 * c / t if t else 0,
               b, bt, 100 * b / bt if bt else 0))
    lp = 100 * cl / tl if tl else 0
    bp = 100 * cb / tb if tb else 0
    print('-' * 72)
    print('%-22s %5d/%-5d %5.1f%% %5d/%-5d %6.1f%%' %
          ('TOTAL', cl, tl, lp, cb, tb, bp))

    # Never-taken branches are printed with their source line, not just their
    # number. A bare line number cannot be judged: the set below is a mix of
    # three different things — branches no test reaches yet (a real gap),
    # branches that no input can reach (dead code, to be deleted), and
    # guards that are unreachable on purpose because they defend against a
    # caller contract rather than against input. Those want opposite actions,
    # and a percentage does not distinguish them. Printing the code lets a
    # reader tell which is which without opening the file.
    for name, c, t, b, bt, ul, ub in rows:
        if ul or ub:
            print('\n%s: uncovered lines %s' % (name, ul[:24]))
            if not ub:
                continue
            print('%s: %d never-taken branch(es):' % (name, len(ub)))
            for line in ub[:24]:
                print('    %4d  %s' % (line, source_line(name, line)))

    bad = []
    if lp < args.min_line:
        bad.append('line coverage %.1f%% < %.1f%%' % (lp, args.min_line))
    if bp < args.min_branch:
        bad.append('branch coverage %.1f%% < %.1f%%' % (bp, args.min_branch))
    if bad and args.strict:
        print('\nFAIL: ' + '; '.join(bad), file=sys.stderr)
        return 1
    return 0


if __name__ == '__main__':
    sys.exit(main())
