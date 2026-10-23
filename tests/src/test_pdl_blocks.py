#!/usr/bin/env python3
"""Independent, source-attributed IEEE PDL block checks at CLI/LSP boundaries."""
import argparse
import json
from pathlib import Path
import subprocess
import sys
import tempfile

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from lsp_test import Client

ROOT = Path(__file__).resolve().parents[2]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--formatter', type=Path, default=ROOT / 'build/pdl-format')
    parser.add_argument('--server', type=Path, default=ROOT / 'build/pdl-lsp')
    parser.add_argument('--block', action='append')
    args = parser.parse_args()
    cases = json.loads((ROOT / 'tests/cases/pdl_blocks.json').read_text())
    assert len({case['path'] for case in cases}) == len(cases), 'duplicate case paths'
    if args.block:
        assert set(args.block) <= {case['block'] for case in cases}, args.block
        cases = [case for case in cases if case['block'] in args.block]
    failures = []
    with tempfile.TemporaryDirectory() as directory:
        client = Client(str(args.server.resolve()))
        try:
            assert client.request('initialize', {'rootUri': Path(directory).as_uri()})
            for case in cases:
                path = ROOT / 'tests/cases' / case['path']
                source = path.read_bytes().decode('utf-8')
                run = subprocess.run([str(args.formatter.resolve()), str(path)],
                                     capture_output=True, timeout=10)
                output = run.stdout.decode('utf-8')
                valid = case['valid']
                if run.returncode != (0 if valid else 2):
                    failures.append(f"{case['block']} {case['path']}: CLI expected valid={valid}: {run.stderr.decode('utf-8', errors='replace')}")
                elif valid:
                    for spelling in case.get('preserve', []):
                        if spelling not in output:
                            failures.append(f"{case['path']}: changed preserved word {spelling}")
                    stable = subprocess.run([str(args.formatter.resolve())], input=run.stdout,
                                            capture_output=True, timeout=10)
                    if stable.returncode or stable.stdout != run.stdout:
                        failures.append(f"{case['path']}: non-idempotent formatting")
                elif run.stdout:
                    failures.append(f"{case['path']}: invalid input produced output")
                uri = path.as_uri()
                client.notify('textDocument/didOpen', {'textDocument': dict(
                    uri=uri, languageId='pdl', version=1, text=source)})
                diagnostics = client.wait(lambda m: m.get('method') == 'textDocument/publishDiagnostics'
                    and m['params']['uri'] == uri, case['path'])['params']['diagnostics']
                if bool(diagnostics) == valid:
                    failures.append(f"{case['block']} {case['path']}: wrong LSP diagnostics {diagnostics}")
                expected = case.get('diagnostic')
                if expected:
                    if expected['message'] not in run.stderr.decode('utf-8', errors='replace'):
                        failures.append(f"{case['path']}: wrong CLI diagnostic {run.stderr!r}")
                    if (not diagnostics or diagnostics[0]['message'] != expected['message']
                            or diagnostics[0]['range']['start'] != expected['start']):
                        failures.append(f"{case['path']}: expected diagnostic {expected}, got {diagnostics}")
                reply = client.request('textDocument/formatting', {'textDocument': {'uri': uri},
                    'options': {'tabSize': 2, 'insertSpaces': True}})
                assert reply and 'result' in reply, reply
                edits = reply['result']
                if valid and run.returncode == 0:
                    formatted = edits[0]['newText'] if edits else source
                    if formatted != output:
                        failures.append(f"{case['path']}: CLI/LSP formatting mismatch")
                elif not valid and edits:
                    failures.append(f"{case['path']}: invalid input received formatting edits")
                client.notify('textDocument/didClose', {'textDocument': {'uri': uri}})
        finally:
            client.stop()
    for failure in failures:
        print(failure, file=sys.stderr)
    print(f'PDL block review: {len(cases)} cases, {len(failures)} failures')
    return bool(failures)


if __name__ == '__main__':
    sys.exit(main())
