#!/usr/bin/env python3
"""Check maintained vendor PDL bytes, CLI stability and real stdio LSP parity."""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess
import sys
import tempfile

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from lsp_test import Client

ROOT = Path(__file__).resolve().parents[2]


def check(formatter, server, root):
    manifest = json.loads((root / 'sources.json').read_text())
    fixtures = {entry['path']: entry for entry in manifest['fixtures']}
    assert set(fixtures) == {p.relative_to(root).as_posix() for p in root.rglob('*.pdl')}
    assert len({entry['sha256'] for entry in fixtures.values()}) == len(fixtures)
    for origin in manifest['sources']:
        assert fixtures[origin['fixture']]['sha256'] == origin['sha256'], origin
    expected = json.loads((root / 'expected_diagnostics.json').read_text())
    assert set(expected) <= set(fixtures), 'expectation names missing fixtures'
    failures = []
    with tempfile.TemporaryDirectory() as directory:
        client = Client(server)
        try:
            reply = client.request('initialize', {'rootUri': Path(directory).as_uri()})
            assert reply and 'result' in reply, reply
            for name, entry in fixtures.items():
                path = root / name
                data = path.read_bytes()
                assert hashlib.sha256(data).hexdigest() == entry['sha256'], name
                assert len(data) == entry['bytes'], name
                run = subprocess.run([formatter, str(path)], capture_output=True, timeout=30)
                diagnostic = expected.get(name)
                if diagnostic:
                    suffix = f"line {diagnostic['line']}: {diagnostic['message']}"
                    if run.returncode != 2 or run.stdout or not run.stderr.decode().strip().endswith(suffix):
                        failures.append(f'{name}: CLI did not report the expected IEEE violation')
                elif run.returncode:
                    failures.append(f'{name}: CLI: {run.stderr.decode(errors="replace").strip()}')
                else:
                    stable = subprocess.run([formatter], input=run.stdout,
                                            capture_output=True, timeout=30)
                    if stable.returncode or stable.stdout != run.stdout:
                        failures.append(f'{name}: CLI formatting is not idempotent')
                uri = path.as_uri()
                client.notify('textDocument/didOpen', {'textDocument': dict(
                    uri=uri, languageId='pdl', version=1, text=data.decode('utf-8'))})
                diagnostics = client.wait(
                    lambda message: message.get('method') == 'textDocument/publishDiagnostics'
                    and message['params']['uri'] == uri, name)['params']['diagnostics']
                if diagnostic:
                    if (len(diagnostics) != 1 or diagnostics[0]['message'] != diagnostic['message']
                            or diagnostics[0]['range']['start']['line'] != diagnostic['line'] - 1):
                        failures.append(f'{name}: LSP did not report the expected IEEE violation')
                elif diagnostics:
                    failures.append(f'{name}: LSP: {diagnostics[0]["message"]}')
                reply = client.request('textDocument/formatting', {
                    'textDocument': {'uri': uri}, 'options': {'tabSize': 2, 'insertSpaces': True}})
                assert reply and 'result' in reply, reply
                edits = reply['result']
                if run.returncode == 0:
                    formatted = edits[0]['newText'].encode() if edits else data
                    if formatted != run.stdout:
                        failures.append(f'{name}: LSP formatting differs from CLI')
                elif edits:
                    failures.append(f'{name}: LSP formats a CLI-rejected document')
                client.notify('textDocument/didClose', {'textDocument': {'uri': uri}})
        finally:
            client.stop()
    for failure in failures:
        print(failure, file=sys.stderr)
    print(f"PDL corpus: {len(manifest['sources'])} original paths, {len(fixtures)} unique streams, "
          f'{len(fixtures) - len(expected)} positive, {len(expected)} expected negative, '
          f'{len(failures)} failures')
    return bool(failures)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--formatter', type=Path, default=ROOT / 'build/pdl-format')
    parser.add_argument('--server', type=Path, default=ROOT / 'build/pdl-lsp')
    parser.add_argument('--root', type=Path, default=ROOT / 'tests/cases/pdl_eda')
    args = parser.parse_args()
    return check(str(args.formatter.resolve()), str(args.server.resolve()), args.root.resolve())


if __name__ == '__main__':
    sys.exit(main())
