#!/usr/bin/env python3
"""Reports which rules of the hand-written grammar and scanner the corpus reaches.

The generated C++ from bison and flex is machine-written; a missing line there
tells nobody anything. A rule in src/icl_parser.yy or src/icl_lexer.l that no
test ever reaches is different: it is a construct nobody has checked, or a
rule that is dead and should be deleted.

How it works, and why it is not instrumented by hand:

  * `bison -t` prints "Reducing stack by rule N (line L):" for every
    reduction, and `flex -d` prints "--accepting rule at line N" for every
    token. Both trace the line in the *source* .y/.l file.
  * The set of lines that are rules at all comes from the generated tables
    themselves — yyrline[] in the parser and yy_rule_linenum[] in the
    scanner — so this report cannot drift from the grammar it measures.

Usage: grammar_coverage.py --binary BIN --parser GEN.cc --lexer GEN.cc
Exits non-zero when a rule is never exercised.
"""

import argparse
import os
import re
import subprocess
import sys

REDUCE = re.compile(r'Reducing stack by rule \d+ \(line (\d+)\):')
ACCEPT = re.compile(r'--accepting rule at line (\d+)')


def rule_lines(path, array_name):
    """Reads a generated `int array[] = { ... };` and returns its numbers.

    bison writes yyrline[] (one entry per grammar rule, holding the line of
    that rule in the .yy) and flex writes yy_rule_linenum[] (the same idea for
    scanner rules). Parsing them here means the expected set always comes from
    the generator rather than from a second parser of the .y/.l syntax, which
    would be one more thing to keep correct.
    """
    text = open(path, encoding='utf-8', errors='replace').read()
    # bison emits the table as a class static, `IclParser::yyrline_[]`, hence
    # the optional trailing underscore.
    match = re.search(re.escape(array_name) +
                      r'_?\s*(?:\[[^\]]*\])?\s*=\s*\{(.*?)\}', text, re.S)
    if not match:
        print('error: %s not found in %s' % (array_name, path), file=sys.stderr)
        return []
    # Line 0 is not a rule: it is the `$accept` pseudo-rule in yyrline[] and
    # flex's "no rule matched" slot in yy_rule_linenum[]. Neither is something
    # a test can reach, so both are dropped rather than reported as missing.
    return [int(n) for n in re.findall(r'-?\d+', match.group(1)) if int(n) > 0]


def corpus_files():
    root = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'cases')
    names = []
    for sub in ('pass', 'fail'):
        directory = os.path.join(root, sub)
        if not os.path.isdir(directory):
            continue
        names += [os.path.join(directory, n)
                  for n in sorted(os.listdir(directory)) if n.endswith('.icl')]
    return names


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--binary', required=True)
    parser.add_argument('--parser', required=True)
    parser.add_argument('--lexer', required=True)
    parser.add_argument('--cases', nargs='*')
    parser.add_argument('--no-fail', action='store_true',
                        help='report only, always exit 0')
    args = parser.parse_args()

    files = args.cases if args.cases else corpus_files()
    if not files:
        print('error: no .icl cases found', file=sys.stderr)
        return 2

    grammar_seen, scanner_seen = set(), set()
    for path in files:
        with open(path, 'rb') as handle:
            data = handle.read()
        # No path argument: that is how the CLI reads stdin.
        result = subprocess.run([args.binary],
                                input=data,
                                stdout=subprocess.DEVNULL,
                                stderr=subprocess.PIPE)
        trace = result.stderr.decode('utf-8', 'replace')
        grammar_seen |= {int(n) for n in REDUCE.findall(trace)}
        scanner_seen |= {int(n) for n in ACCEPT.findall(trace)}

    grammar_all = set(rule_lines(args.parser, 'yyrline'))
    scanner_all = set(rule_lines(args.lexer, 'yy_rule_linenum'))

    def report(label, source, seen, expected):
        missing = sorted(expected - seen)
        total = len(expected)
        covered = len(expected & seen)
        print('%-10s %s: %d/%d rules reached (%.1f%%)'
              % (label, source, covered, total,
                 100.0 * covered / total if total else 0.0))
        return missing

    print('Corpus: %d files' % len(files))
    g_missing = report('grammar', 'src/icl_parser.yy', grammar_seen, grammar_all)
    s_missing = report('scanner', 'src/icl_lexer.l', scanner_seen, scanner_all)

    ok = True
    for label, source, missing in (('grammar', 'src/icl_parser.yy', g_missing),
                                   ('scanner', 'src/icl_lexer.l', s_missing)):
        if not missing:
            continue
        ok = False
        print('\n%s rules never exercised (%s):' % (label, source))
        lines = open(os.path.join(os.path.dirname(os.path.abspath(__file__)),
                                  '..', source), encoding='utf-8').read().split('\n')
        for number in missing:
            text = lines[number - 1].strip() if 0 < number <= len(lines) else '?'
            print('  line %-4d %s' % (number, text[:78]))

    if ok:
        print('\nEvery grammar and scanner rule is exercised by the corpus.')
        return 0
    if args.no_fail:
        return 0
    print('\nFAIL: unreached rules above', file=sys.stderr)
    return 1


if __name__ == '__main__':
    sys.exit(main())
