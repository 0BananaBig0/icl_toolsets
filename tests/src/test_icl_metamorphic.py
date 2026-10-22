#!/usr/bin/env python3
"""IEEE 1687 metamorphic acceptance checks with a nested-comment rejection check.

This oracle does not read our grammar or lexer. Unlike crash fuzzing, rejecting
a valid transformation is a failure. Generated inputs stay in tests/generated/ours.
"""
import argparse
import json
from pathlib import Path
import re
import subprocess

TESTS = Path(__file__).resolve().parents[1]
PARTS = re.compile(r'//[^\n]*|/\*.*?\*/|"(?:\\.|[^"\\])*"|[ \t\r\n]+', re.DOTALL)


def variants(text):
    gaps = [m for m in PARTS.finditer(text) if m[0][0] in ' \t\r\n']
    # Keep newline termination of existing // comments, and never edit strings.
    for index, gap in enumerate(gaps):
        for kind, insertion in [('block', '/* audit */ '),
                                ('line', '// audit\n')]:
            transformed = text[:gap.end()] + insertion + text[gap.end():]
            yield f'{kind}_{index}', transformed, True
    for kind, whitespace in [('crlf', '\r\n'), ('tabs', '\n\t')]:
        yield kind, PARTS.sub(
            lambda m: whitespace if m[0][0] in ' \t\r\n' else m[0], text), True
    indexes = re.compile(
        r'//[^\n]*|/\*.*?\*/|"(?:\\.|[^"\\])*"|\[([0-9]+)(?::([0-9]+))?\]',
        re.DOTALL)
    for depth in [1, 4, 16]:
        def wrap(match):
            if match[1] is None:
                return match[0]
            values = [match[1]] + ([match[2]] if match[2] is not None else [])
            wrapped = ':'.join('(' * depth + v + ')' * depth for v in values)
            return '[' + wrapped + ']'
        nested = indexes.sub(wrap, text)
        if nested != text:
            yield f'index_depth_{depth}', nested, True
    if gaps:
        end = gaps[0].end()
        invalid = text[:end] + '/* outer /* nested */ */ ' + text[end:]
        yield 'invalid_nested_comment', invalid, False


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--inventory', type=Path, required=True)
    parser.add_argument('--block')
    args = parser.parse_args()
    cases = json.loads((TESTS / 'cases/conformance.json').read_text())
    if args.block and args.block not in {case['block'] for case in cases}:
        parser.error(f'unknown syntax block: {args.block}')
    output = TESTS / 'generated/ours/metamorphic'
    output.mkdir(parents=True, exist_ok=True)
    pending = []
    failures = []
    counts = {}
    skipped = []
    seeds = {}

    def check_batch():
        if not pending:
            return
        run = subprocess.run([str(args.inventory.resolve()), '--audit',
                              *[str(p) for p, _ in pending]], capture_output=True,
                             text=True, timeout=60)
        if run.returncode:
            raise RuntimeError(run.stderr)
        results = json.loads(run.stdout)
        if len(results) != len(pending):
            raise RuntimeError('inventory returned the wrong number of results')
        failures.extend(str(p.relative_to(TESTS))
                        for (p, expected), r in zip(pending, results)
                        if r['ok'] != expected)
        pending.clear()

    for seed_index, case in enumerate(cases):
        if not case['accept'] or (args.block and case['block'] != args.block):
            continue
        source = TESTS / case['path']
        text = source.read_text()
        # Opaque vendor payloads have their own lexical rules. Large EDA files
        # are covered by corpus validation; avoid quadratic generated storage.
        if (len(text) > 4096 or 'generic' in source.name.lower() or
                case['block'] == 'eda-corpus' or '#include' in text):
            skipped.append(case['path'])
            continue
        seeds[str(seed_index)] = case
        directory = output / case['block'] / str(seed_index)
        directory.mkdir(parents=True, exist_ok=True)
        for name, variant, expected in variants(text):
            path = directory / (name + '.icl')
            path.write_text(variant)
            pending.append((path, expected))
            counts[case['block']] = counts.get(case['block'], 0) + 1
            if len(pending) == 128:
                check_batch()
    check_batch()
    report = {'counts': counts, 'seeds': seeds, 'skipped': skipped,
              'failures': failures}
    if not counts:
        parser.error('no eligible seeds for the selected syntax block')
    (output / 'results.json').write_text(json.dumps(report, indent=2) + '\n')
    print(f'{sum(counts.values())} variants across {len(counts)} blocks; '
          f'{len(skipped)} seeds excluded; {len(failures)} unexpected results')
    for path in failures[:20]:
        print('FAIL', path)
    return bool(failures)


if __name__ == '__main__':
    raise SystemExit(main())
