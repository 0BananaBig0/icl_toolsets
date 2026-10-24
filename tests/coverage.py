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
from pathlib import Path
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
           'icl_semantics.cc', 'icl_parser.yy', 'icl_lexer.l',
           'icl_ast.h', 'icl_driver.h', 'icl_semantics.h', 'format_tree.h', 'json.h',
           'pdl_document.cc', 'pdl_document.h', 'pdl_format_main.cc',
           'pdl_lsp_main.cc', 'format_cli.h', 'lsp_server.h', 'text_input.h',
           'utf8.h']


LINE = re.compile(r'^\s*([0-9]+\*?|#####|=====|-):\s*(\d+):')


def line_coverage(path):
    seen = {}
    for line in Path(path).read_text().splitlines():
        match = LINE.match(line)
        if not match or match[1] == '-' or int(match[2]) == 0:
            continue
        count = match[1].rstrip('*')
        number = int(match[2])
        seen[number] = seen.get(number, False) or (count.isdigit() and int(count) > 0)
    missing = sorted(number for number, hit in seen.items() if not hit)
    return sum(seen.values()), len(seen), missing


def branch_coverage(path):
    """Count never-executed and exception edges, including template instances."""
    current, function, seen = None, '', {}
    for line in Path(path).read_text().splitlines():
        if line.startswith('function '):
            function = line.split(' called ', 1)[0]
        match = LINE.match(line)
        if match:
            current = int(match[2])
            continue
        branch = re.search(r'branch\s+(\d+)\s+(?:taken\s+(\d+)|never executed)', line)
        if branch and current is not None:
            key = (function, current, int(branch[1]))
            seen[key] = max(seen.get(key, 0), int(branch[2] or 0))
    return sum(count > 0 for count in seen.values()), len(seen), sorted({key[1] for key, count in seen.items() if not count})


def source_line(name, number):
    """The |number|-th line of a measured source file, trimmed.

    The .gcov files already carry the source text, but reading it from the
    original keeps this independent of gcov's formatting and of whether the
    file was emitted at all.
    """
    root = Path(__file__).resolve().parents[1]
    for directory in (root / 'src', root):
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

    missing = [name for name in SOURCES if not Path(name + '.gcov').exists()]
    if missing:
        print('error: missing source reports: ' + ', '.join(missing), file=sys.stderr)
        return 2
    tl = cl = tb = cb = 0
    rows = []
    for name in SOURCES:
        path = name + '.gcov'
        c, t, ul = line_coverage(path)
        b, bt, ub = branch_coverage(path)
        cl += c; tl += t; cb += b; tb += bt
        rows.append((name, c, t, b, bt, ul, ub))

    if not tl:
        print('error: no executable source lines in reports', file=sys.stderr)
        return 2
    print('Branch totals include compiler-emitted exception and cleanup edges.')
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
