#!/usr/bin/env python3
"""ICL and PDL must stay separate languages.

IEEE 1687 defines both, and upstream they sit in the same repository
(`src/icl_parser/icl.g4` and `src/pdl_parser/pdl.g4`), which makes them easy
to conflate here. They have different lexers, different keyword sets and
different number spellings.

This test asserts the separation holds, and does so from a list written down
here rather than from the network, so it keeps working offline. Like
`tree_clean_test.py`, that duplication is the point: if the two sides ever
drift, this fails and names the keyword.

Checked:

1. No PDL-only keyword is reserved by our scanner. Reserving one would break
   a legal ICL file using that word as an identifier, and would mean PDL had
   been modelled here after all.
2. Every PDL-only keyword still parses as a plain attribute name — the other
   half of the same claim.
3. The PDL-only lexical forms are rejected: `0x`/`0b` literals and `#`
   comments.
"""

import argparse
import os
import re
import subprocess
import sys
import tempfile

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(HERE)
LEXER = os.path.join(ROOT, 'src', 'icl_lexer.l')

# Literals that appear in pdl.g4 and not in icl.g4. Taken from the two
# grammars at Honza255/icl_parser commit 333f8bc; `POS_INT` and
# `STD_1687_2014` are lexer/standard names rather than keywords, but they
# come from the same diff and are kept for completeness.
PDL_ONLY_KEYWORDS = [
    'iApply', 'iCall', 'iClock', 'iClockOverride', 'iGetMiscompares',
    'iGetReadData', 'iGetStatus', 'iMerge', 'iNote', 'iOverrideScanInterface',
    'iPDLLevel', 'iPrefix', 'iProc', 'iProcsForModule', 'iRead', 'iRelease',
    'iReset', 'iRunLoop', 'iScan', 'iSetFail', 'iState', 'iTake',
    'iUseProcNameSpace', 'iWrite', 'off', 'on', 'STD_1687_2014', 'POS_INT',
]

# Spelled the same in both languages, so NOT expected to be rejected:
#   SCALAR_ID, $name (ARGUMENT_REF), and the seven time units
#   (s ms us ns ps fs as) — plus the hex/binary digit characters.
SHARED = ['s', 'ms', 'us', 'ns', 'ps', 'fs', 'as']


def reserved_keywords():
    """Keywords the scanner returns as their own token.

    A rule line looks like:

        ScanInPort          { g_location.step(); return ...make_SCANINPORT...; }

    Matching on `make_` excludes other brace-opening lines in the file, such
    as the anonymous `namespace {`.
    """
    out = set()
    with open(LEXER, encoding='utf-8') as handle:
        for line in handle:
            m = re.match(r'^([A-Za-z][A-Za-z0-9_]*)\s+\{', line)
            if m and 'make_' in line:
                out.add(m.group(1))
    return out


def accepts(binary, text):
    with tempfile.NamedTemporaryFile('w', suffix='.icl', delete=False) as fh:
        fh.write(text)
        path = fh.name
    try:
        return subprocess.run([binary, path], capture_output=True,
                              text=True).returncode == 0
    finally:
        os.unlink(path)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--binary', default=os.path.join(ROOT, 'build',
                                                         'icl-format'))
    args = parser.parse_args()
    if not os.path.exists(args.binary):
        print('pdl separation: %s not built — skipped' % args.binary)
        return 0
    if not os.path.exists(LEXER):
        print('pdl separation: %s missing' % LEXER)
        return 1

    failures = []
    ours = reserved_keywords()

    # 1. no PDL-only keyword is reserved here
    clash = sorted(k for k in PDL_ONLY_KEYWORDS if k in ours)
    if clash:
        failures.append('reserved here but PDL-only: %s' % ', '.join(clash))

    # 2. ...and each still works as an ordinary attribute name
    for kw in PDL_ONLY_KEYWORDS:
        if not accepts(args.binary,
                       'Module m { ScanRegister sr { %s 1; } }' % kw):
            failures.append('%s: not usable as an attribute name' % kw)

    # 3. the PDL-only lexical forms are rejected
    for form, why in [
        ('ResetValue 0x10;', '0x literal'),
        ('ResetValue 0b101;', '0b literal'),
        ('# comment', '# comment'),
    ]:
        if accepts(args.binary, 'Module m { ScanRegister sr { %s } }' % form):
            failures.append('%s: accepted, should be rejected' % why)

    # A shared spelling must NOT be rejected — the guard against overreach.
    for unit in SHARED:
        if not accepts(args.binary,
                       'Module m { ToClockPort c { Period 10%s; } }' % unit):
            failures.append('time unit %s rejected, but it is ICL too' % unit)

    if failures:
        print('pdl separation: %d failure(s)' % len(failures))
        for f in failures:
            print('  FAIL %s' % f)
        return 1
    print('pdl separation: %d PDL-only keywords stay identifiers; '
          '0x/0b/# rejected; %d shared spellings kept'
          % (len(PDL_ONLY_KEYWORDS), len(SHARED)))
    return 0


if __name__ == '__main__':
    sys.exit(main())
