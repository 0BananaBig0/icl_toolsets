#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""Golden-file and two-pass stability tests: outputs are compared byte for byte.

The regression corpus checks two invariants — that formatting is idempotent
and that no character is lost. Both are necessary and neither is sufficient.
A formatter that emitted every token on one line, or that dropped all
indentation, would satisfy both: it is idempotent, and stripping whitespace
from input and output would still match. What the invariants cannot see is
whether the output *looks right*.

So a small set of inputs has its expected output stored alongside it, and
this compares exactly. This is what gofmt, clang-format and prettier all do,
and it is deliberately brittle: a change in formatting behaviour is supposed
to break these.

    python3 tests/golden_test.py --binary build/icl-format
    python3 tests/golden_test.py --language pdl --case 'nested*'
    python3 tests/golden_test.py --language pdl --server build/pdl-lsp
    python3 tests/golden_test.py --update      # re-record every expectation

--update exists because the expectations are output, not intent: they are
re-recorded after an intentional change. It is not a way to make a failure go
away — the diff that --update produces is the thing to review.

Every case must also satisfy F(F(input)) == F(input). With --server, original
and formatted documents must parse in the LSP, and formatting must agree with
the CLI and leave the second document unchanged. Explicit updates are deferred
until all selected cases pass these checks; ordinary runs never write baselines.
"""

import argparse
import difflib
import fnmatch
import math
import os
from pathlib import Path
import subprocess
import sys

from lsp_test import Client

HERE = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'golden')
PDL_HERE = os.path.join(os.path.dirname(os.path.abspath(__file__)),
                        'cases', 'ours', 'pdl', 'golden')


def cases(language='icl'):
    out = []
    root = HERE if language == 'icl' else PDL_HERE
    for name in sorted(os.listdir(root)):
        if name.endswith('.' + language):
            out.append(name[:-(len(language) + 1)])
    return out


def check_lsp(client, path, language, source, formatted):
    uri = Path(path).resolve().as_uri()
    expected = formatted.decode('utf-8')
    try:
        for version, data in enumerate((source, formatted), 1):
            text = data.decode('utf-8')
            document = {'uri': uri, 'version': version}
            if version == 1:
                client.notify('textDocument/didOpen', {'textDocument': dict(
                    document, languageId=language, text=text)})
            else:
                client.notify('textDocument/didChange', {
                    'textDocument': document, 'contentChanges': [{'text': text}]})
            diagnostic = client.wait(
                lambda m: m.get('method') == 'textDocument/publishDiagnostics'
                and m['params'].get('uri') == uri
                and m['params'].get('version') == version,
                'golden diagnostics: ' + path)
            if not diagnostic or diagnostic.get('params', {}).get('diagnostics') != []:
                raise ValueError('LSP pass %d: missing or nonempty diagnostics' % version)
            reply = client.request('textDocument/formatting', {
                'textDocument': {'uri': uri},
                'options': {'tabSize': 2, 'insertSpaces': True}})
            # Both servers promise one whole-document edit, or no edit at a fixed point.
            end = {'line': text.count('\n'),
                   'character': len(text.rsplit('\n', 1)[-1].removesuffix('\r')
                                    .encode('utf-16-le')) // 2}
            edits = [] if text == expected else [{
                'range': {'start': {'line': 0, 'character': 0}, 'end': end},
                'newText': expected}]
            if not reply or 'error' in reply or reply.get('result') != edits:
                raise ValueError('LSP pass %d: formatting differs from CLI or is not stable' %
                                 version)
    finally:
        client.notify('textDocument/didClose', {'textDocument': {'uri': uri}})


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--language', choices=('icl', 'pdl'), default='icl')
    parser.add_argument('--binary', help='default: build/<language>-format')
    parser.add_argument('--server', help='also check this LSP server against both CLI passes')
    parser.add_argument('--case', default='*', help='case-name glob (quote wildcard patterns)')
    parser.add_argument('--timeout', type=float, default=10,
                        help='wall-clock seconds per formatter invocation (default: 10)')
    parser.add_argument('--update', action='store_true',
                        help='re-record the expected output for every case')
    args = parser.parse_args()
    if not math.isfinite(args.timeout) or args.timeout <= 0:
        parser.error('--timeout must be finite and greater than zero')
    args.binary = args.binary or 'build/' + args.language + '-format'

    if not os.path.exists(args.binary):
        print('error: no binary at %s; run make first' % args.binary,
              file=sys.stderr)
        return 2

    root = HERE if args.language == 'icl' else PDL_HERE
    try:
        selected = [name for name in cases(args.language) if fnmatch.fnmatchcase(name, args.case)]
    except OSError as error:
        print('error discovering golden cases: ' + str(error), file=sys.stderr)
        return 2
    if not selected:
        print('error: no golden cases match ' + args.case, file=sys.stderr)
        return 2

    client = None
    try:
        if args.server:
            client = Client(os.path.abspath(args.server))
            reply = client.request('initialize', {})
            if not reply or 'error' in reply or not isinstance(reply.get('result'), dict):
                print('error: LSP initialization failed', file=sys.stderr)
                return 2
            client.notify('initialized')
        return run_cases(args, root, selected, client)
    except OSError as error:
        print('error: ' + str(error), file=sys.stderr)
        return 2
    finally:
        if client is not None:
            client.stop()


def run_cases(args, root, selected, client):
    passed = 0
    failed = []
    updates = {}
    for name in selected:
        src = os.path.join(root, name + '.' + args.language)
        exp = os.path.join(root, name + '.expected')
        try:
            with open(src, 'rb') as handle:
                source = handle.read()
            outputs = []
            for mode, command, options in (
                    ('file', [args.binary, src], {}),
                    ('stdin', [args.binary], {'input': source})):
                result = subprocess.run(command, capture_output=True,
                                        timeout=args.timeout, **options)
                if result.returncode != 0:
                    failed.append('%s [%s]: exit %d %s' % (
                        name, mode, result.returncode,
                        result.stderr.decode('utf-8', 'replace')[:200]))
                    break
                outputs.append(result.stdout)
            if len(outputs) != 2:
                continue
            if outputs[0] != outputs[1]:
                failed.append(name + ': file/stdin outputs differ')
                continue
            actual = outputs[0]
            mode = 'second pass'
            result = subprocess.run([args.binary], input=actual, capture_output=True,
                                    timeout=args.timeout)
            if result.returncode or result.stdout != actual:
                failed.append(name + ': formatting is not idempotent (second pass exit %d)' %
                              result.returncode)
                continue
            if client is not None:
                check_lsp(client, src, args.language, source, actual)
            if args.update:
                updates[exp] = actual
                passed += 1
                continue
            with open(exp, 'rb') as handle:
                expected = handle.read()
        except subprocess.TimeoutExpired:
            failed.append('%s [%s]: timed out after %g seconds' % (name, mode, args.timeout))
            continue
        except (OSError, ValueError) as error:
            failed.append(name + ': ' + str(error))
            continue
        if actual == expected:
            passed += 1
            continue
        # Byte comparison decides the result; the diff is only explanatory.
        difference = ''.join(difflib.unified_diff(
            expected.decode('utf-8', 'backslashreplace').splitlines(keepends=True),
            actual.decode('utf-8', 'backslashreplace').splitlines(keepends=True),
            fromfile=exp, tofile=name + '.actual'))
        failed.append(name + ': output differs\n' + difference)

    if not failed:
        for path, output in updates.items():
            try:
                with open(path, 'wb') as handle:
                    handle.write(output)
            except OSError as error:
                failed.append(path + ': ' + str(error))
    print('%s golden: PASS=%d  FAIL=%d' % (args.language, passed, len(failed)))
    for line in failed:
        print('  [FAIL] ' + line)
    if failed:
        return 1
    print('Golden files updated.' if args.update else 'All golden files match.')
    return 0


if __name__ == '__main__':
    sys.exit(main())
