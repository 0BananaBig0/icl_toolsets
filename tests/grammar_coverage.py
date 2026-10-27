#!/usr/bin/env python3
"""Reports which rules of the hand-written grammar and scanner the corpus reaches.

The generated C++ from bison and flex is machine-written; a missing line there
tells nobody anything. A rule in src/icl/icl_parser.yy or src/icl/icl_lexer.l that no
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
import json
import os
import re
import subprocess
import sys
import tempfile
from pathlib import Path

REDUCE = re.compile(r'Reducing stack by rule (\d+) \(line (\d+)\):')
ACCEPT = re.compile(r'--accepting rule at line (\d+)')


def rule_lines(path, array_name):
    """Reads a generated `int array[] = { ... };` and returns its numbers.

    bison writes yyrline[] (one entry per grammar rule, holding the line of
    that rule in the .yy) and flex writes yy_rule_linenum[] (the same idea for
    scanner rules). Parsing them here means the expected set always comes from
    the generator rather than from a second parser of the .y/.l syntax, which
    would be one more thing to keep correct.
    """
    text = Path(path).read_text(encoding='utf-8', errors='replace')
    match = re.search(re.escape(array_name) +
                      r'_?\s*(?:\[[^\]]*\])?\s*=\s*\{(.*?)\}', text, re.S)
    if not match:
        raise ValueError('%s not found in %s' % (array_name, path))
    values = [int(n) for n in re.findall(r'-?\d+', match.group(1))]
    if array_name == 'yyrline':
        # Slot0 is unused, slot1 is $accept. Bison prints table index minus1.
        rules = {slot - 1: line for slot, line in enumerate(values) if slot > 1 and line > 0}
    else:
        rules = {line: line for line in values if line > 0}
    if not rules:
        raise ValueError('%s has no source rules in %s' % (array_name, path))
    return rules


def corpus_files():
    """Every `.icl` case, recursively.

    Recursive for the same reason the regression runner is: the corpus is now
    sorted into per-source subdirectories, and a flat listing would find
    nothing and report success on an empty set.
    """
    root = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'cases')
    names = []
    for sub in ('pass', 'fail', 'standard', 'tessent', 'upstream', 'ours', 'published'):
        directory = os.path.join(root, sub)
        if not os.path.isdir(directory):
            continue
        for dirpath, dirnames, filenames in os.walk(directory):
            dirnames.sort()
            names += [os.path.join(dirpath, n)
                      for n in sorted(filenames) if n.endswith('.icl')]
    return names


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--binary', required=True)
    parser.add_argument('--parser', required=True)
    parser.add_argument('--lexer', required=True)
    parser.add_argument('--cases', nargs='+')
    tests = Path(__file__).resolve().parent
    parser.add_argument('--manifest', type=Path, default=tests / 'cases/conformance.json')
    parser.add_argument('--report', type=Path, help='write per-block trace evidence as JSON')
    parser.add_argument('--no-fail', action='store_true',
                        help='allow unreached rules; process and expectation errors still fail')
    args = parser.parse_args()

    files = args.cases if args.cases else corpus_files()
    if not files:
        print('error: no .icl cases found', file=sys.stderr)
        return 2

    try:
        cases = json.loads(args.manifest.read_text())
        expected = {}
        for case in cases:
            expected.setdefault((tests / case['path']).resolve(), []).append(case)
        order = {block: i for i, block in enumerate(dict.fromkeys(c['block'] for c in cases))}
    except (OSError, ValueError, KeyError) as error:
        print('error:', error, file=sys.stderr)
        return 2
    # One trace execution per file, in the normative manifest's review order.
    files = sorted(files, key=lambda path: min(
        (order[c['block']] for c in expected.get(Path(path).resolve(), [])),
        default=len(order)))
    grammar_seen, scanner_seen, blocks = set(), set(), {}
    for path in files:
        try:
            data = Path(path).read_bytes()
            # No path argument: that is how the CLI reads stdin.
            # The 673KiB benchmark emits 143MB of trace in ~45s locally.
            # Spool to disk to bound memory; allow headroom for slower CI hosts.
            with tempfile.TemporaryFile() as trace:
                result = subprocess.run([args.binary], input=data, timeout=180,
                                        stdout=subprocess.DEVNULL, stderr=trace)
                trace.seek(0)
                productions, scanner_lines = set(), set()
                for raw_line in trace:
                    line = raw_line.decode('utf-8', 'replace')
                    productions.update(int(rule) for rule, _ in REDUCE.findall(line))
                    scanner_lines.update(int(n) for n in ACCEPT.findall(line))
        except (OSError, subprocess.TimeoutExpired) as error:
            print(f'error: {path}: {error}', file=sys.stderr)
            return 2
        classifications = expected.get(Path(path).resolve(), [])
        if result.returncode not in (0, 2):
            print(f'error: {path}: formatter exited {result.returncode}', file=sys.stderr)
            return 2
        if any((result.returncode == 0) != case['accept'] for case in classifications):
            print(f'error: {path}: acceptance disagrees with conformance manifest', file=sys.stderr)
            return 1
        grammar_seen |= productions
        scanner_seen |= scanner_lines
        for name in dict.fromkeys(c['block'] for c in classifications) or ['unclassified']:
            block = blocks.setdefault(name, {
                'accepted': 0, 'rejected': 0, 'productions': set(), 'scanner_lines': set()})
            block['accepted' if result.returncode == 0 else 'rejected'] += 1
            block['productions'] |= productions
            block['scanner_lines'] |= scanner_lines

    try:
        grammar_lines = rule_lines(args.parser, 'yyrline')
        scanner_lines = rule_lines(args.lexer, 'yy_rule_linenum')
    except (OSError, ValueError) as error:
        print('error:', error, file=sys.stderr)
        return 2
    grammar_all, scanner_all = set(grammar_lines), set(scanner_lines)

    def report(label, source, seen, expected):
        missing = sorted(expected - seen)
        total = len(expected)
        covered = len(expected & seen)
        print('%-10s %s: %d/%d rules reached (%.1f%%)'
              % (label, source, covered, total,
                 100.0 * covered / total if total else 0.0))
        return missing

    print('Corpus: %d files' % len(files))
    g_missing = report('grammar', 'src/icl/icl_parser.yy', grammar_seen, grammar_all)
    s_missing = report('scanner', 'src/icl/icl_lexer.l', scanner_seen, scanner_all)

    if args.report:
        for block in blocks.values():
            block['productions'] = sorted(block['productions'])
            block['scanner_lines'] = sorted(block['scanner_lines'])
        args.report.parent.mkdir(parents=True, exist_ok=True)
        args.report.write_text(json.dumps({
            'files': len(files), 'blocks': blocks,
            'missing_productions': g_missing, 'missing_scanner_lines': s_missing,
        }, indent=2) + '\n')

    ok = True
    for label, source, missing in (('grammar', 'src/icl/icl_parser.yy', g_missing),
                                   ('scanner', 'src/icl/icl_lexer.l', s_missing)):
        if not missing:
            continue
        ok = False
        print('\n%s rules never exercised (%s):' % (label, source))
        lines = open(os.path.join(os.path.dirname(os.path.abspath(__file__)),
                                  '..', source), encoding='utf-8').read().split('\n')
        mapping = grammar_lines if label == 'grammar' else scanner_lines
        for rule in missing:
            number = mapping[rule]
            text = lines[number - 1].strip() if 0 < number <= len(lines) else '?'
            print('  rule %-4d line %-4d %s' % (rule, number, text[:78]))

    if ok:
        print('\nEvery grammar and scanner rule is exercised by the corpus.')
        return 0
    if args.no_fail:
        return 0
    print('\nFAIL: unreached rules above', file=sys.stderr)
    return 1


if __name__ == '__main__':
    sys.exit(main())
