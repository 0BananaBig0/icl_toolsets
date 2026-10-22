#!/usr/bin/env python3
"""Source-backed grammar acceptance, content preservation and idempotence."""
import argparse
import json
from pathlib import Path
import re
import subprocess
import sys

TESTS = Path(__file__).resolve().parents[1]
# Keep string and comment contents exact; ignore layout whitespace elsewhere.
CONTENT = re.compile(r'//[^\n]*|/\*.*?\*/|"(?:\\.|[^"\\])*"|[^\s]', re.DOTALL)

def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--binary', type=Path, required=True)
    parser.add_argument('--block')
    args = parser.parse_args()
    binary = args.binary.resolve()
    cases = json.loads((TESTS / 'cases/conformance.json').read_text())
    selected = [c for c in cases if not args.block or c['block'] == args.block]
    if not selected:
        parser.error('no matching cases')
    failures = []
    for case in selected:
        text = (TESTS / case['path']).read_text()
        try:
            result = subprocess.run([str(binary)], input=text, capture_output=True, text=True, timeout=10)
            if case['accept']:
                assert result.returncode == 0, result.stderr
                assert CONTENT.findall(text) == CONTENT.findall(result.stdout), 'content changed'
                again = subprocess.run([str(binary)], input=result.stdout, capture_output=True, text=True, timeout=10)
                assert again.returncode == 0, again.stderr
                assert again.stdout == result.stdout, 'not idempotent'
            else:
                assert result.returncode == 2, f'expected syntax error (2), got {result.returncode}'
                assert not result.stdout, 'rejected input produced output'
                assert result.stderr, 'missing diagnostic'
        except (AssertionError, subprocess.TimeoutExpired) as error:
            failures.append(case['path'])
            print(f"FAIL {case['path']}: {error}")
    print(f'{len(selected) - len(failures)}/{len(selected)} conformance cases passed')
    return bool(failures)

if __name__ == '__main__':
    sys.exit(main())
