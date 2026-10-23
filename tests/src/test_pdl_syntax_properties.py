#!/usr/bin/env python3
"""Independent PDL production properties and bounded Tcl syntax differential tests."""
import argparse
import json
from pathlib import Path
import subprocess
import sys
import tempfile

TESTS = Path(__file__).resolve().parents[1]
ROOT = TESTS.parent
sys.path.insert(0, str(TESTS))
from lsp_test import Client


def stream(body, level=0, top=False):
    header = f'iPDLLevel {level} -version STD_1687_2014\niProcsForModule Demo\n'
    return header + (body + '\n' if top else 'iProc test {bit data reg} {\n' + body + '\n}\n')


def pdl_cases():
    reference = json.loads((TESTS / 'cases/standard/pdl/syntax_properties.json').read_text())
    seeds = [(c['name'], stream(c['text'], c['level'], c['stream']), True)
             for c in reference['commands']]
    for case in reference['commands']:
        for key in ('invalid', 'also_valid'):
            if key in case:
                seeds.append((case['name'] + '-' + key,
                              stream(case[key], case['level'], case['stream']),
                              key == 'also_valid'))
    for group, template in [('numbers', 'iWrite reg {}'), ('objects', 'iRead {}')]:
        for validity in ('valid', 'invalid'):
            for index, value in enumerate(reference[group][validity]):
                seeds.append((f'{group}-{validity}-{index}', stream(template.format(value)),
                              validity == 'valid'))
    for name, text, valid in seeds:
        for layout, transformed in (
                ('lf', text), ('crlf', text.replace('\n', '\r\n')),
                ('tabs', text.replace(' ', '\t')),
                ('continuations', text.replace(' ', ' \\\n '))):
            yield f'pdl-{name}-{layout}', transformed, valid


def nested_cases():
    for depth in (1, 2, 8, 32):
        for valid in (True, False):
            body = 'iWrite reg 1' if valid else 'iWrite reg'
            for _ in range(depth):
                body = 'if {1} {\n' + body + '\n}'
            yield f'nesting-{depth}-{valid}', stream(body, level=1), valid


def tcl_cases():
    cases = json.loads((TESTS / 'cases/ours/pdl/tcl_syntax_oracle.json').read_text())['cases']
    # Execute only this bounded, maintained Tcl corpus, never imported PDL.
    script = []
    for index, case in enumerate(cases):
        encoded = case['text'].encode().hex()
        script.extend([
            'set child [interp create -safe]',
            'interp limit $child command -value 10000',
            f'set input [encoding convertfrom utf-8 [binary decode hex {encoded}]]',
            'set code [catch {interp eval $child $input} result]',
            f'puts "{index} $code [binary encode hex [encoding convertto utf-8 $result]]"',
            'interp delete $child',
        ])
    result = subprocess.run(['tclsh'], input='\n'.join(script), text=True,
                            capture_output=True, timeout=10, check=True)
    if result.stderr:
        raise RuntimeError('Tcl oracle failed: ' + result.stderr)
    lines = result.stdout.splitlines()
    if len(lines) != len(cases):
        raise RuntimeError('Tcl oracle did not return one result per input')
    for index, (case, line) in enumerate(zip(cases, lines)):
        fields = line.split(' ', 2)
        if int(fields[0]) != index or fields[1] not in ('0', '1'):
            raise RuntimeError('unexpected Tcl oracle result: ' + line)
        message = bytes.fromhex(fields[2]).decode() if fields[2] else ''
        if 'limit exceeded' in message:
            raise RuntimeError('Tcl oracle exceeded its budget: ' + case['name'])
        yield 'tcl-' + case['name'], case['text'], fields[1] == '0'


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--formatter', type=Path, default=ROOT / 'build/pdl-format')
    parser.add_argument('--server', type=Path, default=ROOT / 'build/pdl-lsp')
    parser.add_argument('--only', choices=('pdl', 'tcl', 'nesting'))
    parser.add_argument('--report', type=Path, default=ROOT / 'build/pdl-syntax-report.json')
    args = parser.parse_args()
    cases = []
    for group, factory in [('pdl', pdl_cases), ('tcl', tcl_cases), ('nesting', nested_cases)]:
        if args.only in (None, group):
            cases.extend(factory())
    generated = TESTS / 'generated/pdl_syntax'
    generated.mkdir(parents=True, exist_ok=True)
    failures = []
    with tempfile.TemporaryDirectory() as directory:
        client = Client(str(args.server.resolve()))
        try:
            reply = client.request('initialize', {'rootUri': Path(directory).as_uri()})
            assert reply and 'result' in reply, reply
            for name, text, valid in cases:
                path = generated / (name + '.pdl')
                path.write_bytes(text.encode())
                run = subprocess.run([str(args.formatter.resolve()), str(path)],
                                     capture_output=True, timeout=5)
                reasons = []
                if run.returncode != (0 if valid else 2):
                    reasons.append(f'CLI expected valid={valid}, exit={run.returncode}: '
                                   + run.stderr.decode(errors='replace'))
                if not valid and run.stdout:
                    reasons.append('invalid source produced formatted output')
                if valid and run.returncode == 0:
                    again = subprocess.run([str(args.formatter.resolve())], input=run.stdout,
                                           capture_output=True, timeout=5)
                    if again.returncode or again.stdout != run.stdout:
                        reasons.append('formatting is not idempotent')
                uri = path.as_uri()
                client.notify('textDocument/didOpen', {'textDocument': dict(
                    uri=uri, languageId='pdl', version=1, text=text)})
                diagnostics = client.wait(lambda m: m.get('method') == 'textDocument/publishDiagnostics'
                    and m['params']['uri'] == uri, name)['params']['diagnostics']
                if bool(diagnostics) == valid:
                    reasons.append('LSP acceptance disagrees with reference')
                reply = client.request('textDocument/formatting', dict(
                    textDocument={'uri': uri}, options={'tabSize': 2, 'insertSpaces': True}))
                assert reply and 'result' in reply, reply
                edits = reply['result']
                if not valid and edits:
                    reasons.append('invalid source received LSP edits')
                elif valid and run.returncode == 0:
                    output = edits[0]['newText'] if edits else text
                    if output.encode() != run.stdout:
                        reasons.append('CLI/LSP formatting differs')
                client.notify('textDocument/didClose', {'textDocument': {'uri': uri}})
                if reasons:
                    failures.append(dict(case=name, path=str(path.relative_to(ROOT)),
                                         valid=valid, errors=reasons))
        finally:
            client.stop()
    args.report.parent.mkdir(parents=True, exist_ok=True)
    args.report.write_text(json.dumps(dict(cases=len(cases), failures=failures), indent=2) + '\n')
    for failure in failures:
        print(f"FAIL {failure['case']}: {'; '.join(failure['errors'])}")
    print(f'PDL syntax properties: {len(cases)} cases, {len(failures)} failures')
    return bool(failures)


if __name__ == '__main__':
    sys.exit(main())
