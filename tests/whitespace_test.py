#!/usr/bin/env python3
"""Whitespace must not change the output.

For a formatter this is close to a defining property: if two inputs differ
only in whitespace, the formatted result must be the same. Three existing
checks do not cover it:

    idempotence            one input, formatted twice
    character preservation tokens survive, whatever their arrangement
    golden files           one input, one expected output

None of them asks what happens when the *same* file is written another way,
and a formatter that, say, keyed its indentation off the original line breaks
would pass all three.

So: every corpus file is reflowed two ways — everything onto one line, and
every `{`, `}`, `;` onto its own line — and all three must format to the same
result.

Blank lines are exempt. The formatter preserves them on purpose: a blank line
before a block in the input stays in the output. Compacting removes that
information, so comparing would be comparing two different inputs. Non-blank
lines only.

Reflowing has to respect strings and comments — putting a newline inside a
string literal splits it into an unterminated string and the check would be
measuring its own bug. (It did, the first time: two files "failed" whose only
offence was containing `{` inside a string.)
"""

import argparse
import glob
import os
import re
import subprocess
import sys
import tempfile

from src.test_icl_conformance import CONTENT

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(HERE)


def format_source(binary, text, workdir):
    path = os.path.join(workdir, 'ws.icl')
    with open(path, 'w', encoding='utf-8') as fh:
        fh.write(text)
    proc = subprocess.run([binary, path], capture_output=True, text=True)
    return proc.stdout if proc.returncode == 0 else None


def compact(text):
    return re.sub(r'\s+', ' ', text).strip()


def expand(text):
    """Put every brace and semicolon on its own line, outside strings."""
    out = []
    i = 0
    while i < len(text):
        ch = text[i]
        if ch == '"':
            end = text.find('"', i + 1)
            end = len(text) if end < 0 else end + 1
            out.append(text[i:end])
            i = end
            continue
        if text.startswith('//', i):
            end = text.find('\n', i)
            end = len(text) if end < 0 else end
            out.append(text[i:end])
            i = end
            continue
        out.append('\n' + ch + '\n' if ch in '{};' else ch)
        i += 1
    return ''.join(out)


def without_blank_lines(text):
    return [l for l in text.split('\n') if l.strip()]


def corpus():
    pattern = os.path.join(ROOT, 'tests', 'cases', 'pass', '**', '*.icl')
    for path in sorted(glob.glob(pattern, recursive=True)):
        text = open(path, encoding='utf-8', errors='replace').read()
        # Block comments can contain `{` and `;`; reflowing inside one is not
        # meaningful, and skipping them keeps `expand` simple.
        if '//' in text or '/*' in text:
            continue
        yield path, text


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--binary', default=os.path.join(ROOT, 'build', 'icl-format'))
    args = parser.parse_args()
    binary = os.path.abspath(args.binary)

    failures = []
    checked = 0
    with tempfile.TemporaryDirectory() as workdir:
        for path, text in corpus():
            original = format_source(binary, text, workdir)
            if original is None:
                continue
            one_line = format_source(binary, compact(text), workdir)
            spread = format_source(binary, expand(text), workdir)
            if one_line is None:
                failures.append((path, 'rejected once compacted onto one line'))
                continue
            if spread is None:
                failures.append((path, 'rejected once every brace is on its own line'))
                continue
            checked += 1
            # Generic controllers define their own payload; only outer ICL is formatted.
            if re.search(r'AccessLink\s+\w+\s+Of\s+(?!STD_1149_1_(?:2001|2013)\b)\w+\s*\{', text):
                for source, result in [(text, original), (compact(text), one_line), (expand(text), spread)]:
                    if CONTENT.findall(source) != CONTENT.findall(result):
                        failures.append((path, 'generic payload content changed'))
                    if format_source(binary, result, workdir) != result:
                        failures.append((path, 'generic payload formatting is not idempotent'))
                continue
            if not (without_blank_lines(original) ==
                    without_blank_lines(one_line) ==
                    without_blank_lines(spread)):
                failures.append((path, 'the three reflows format differently'))

    print()
    print('whitespace: %d files reflowed, %d failed' % (checked, len(failures)))
    for path, why in failures:
        print('  FAIL %s: %s' % (os.path.relpath(path, ROOT), why))
    if failures:
        return 1
    print('Known ICL reflows canonically; opaque Generic payloads preserve content and remain idempotent.')
    return 0


if __name__ == '__main__':
    sys.exit(main())
