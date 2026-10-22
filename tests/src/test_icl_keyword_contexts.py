#!/usr/bin/env python3
"""IEEE 6.3.3/6.3.10 identifier permissions across independent syntax contexts."""
import argparse
import json
from pathlib import Path
import re
import subprocess
import sys

from test_icl_conformance import CONTENT
from test_icl_diagnostics import position

TESTS = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(TESTS))
from lsp_test import Client

SEEDS = TESTS / 'cases/ours/keyword_contexts'
INVALID_NAMES = ['_bus3', '5bitbus', 'bit-bus', 'A$suffix']


def combinations(seeds, names):
    for seed in seeds:
        text = seed.read_text()
        if 'ProbeName' not in text:
            raise ValueError(f'{seed}: missing identifier placeholder')
        for index, name in enumerate(names + INVALID_NAMES):
            # IEEE 6.4.9 permits a procedure followed by a parameter argument.
            # Here A$suffix is two legal tokens, not one malformed identifier.
            expected = name not in INVALID_NAMES or (
                seed.stem == 'callback_procedure' and name == 'A$suffix')
            yield seed.stem, index, name, expected, text.replace('ProbeName', name)


def formatting_matches(text, formatted, edits):
    expected = [] if text == formatted else [{
        'range': {'start': {'line': 0, 'character': 0},
                  'end': position(text, len(text))},
        'newText': formatted}]
    return edits == expected


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--binary', type=Path, required=True)
    parser.add_argument('--lsp', type=Path, required=True)
    parser.add_argument('--inventory', type=Path, required=True)
    parser.add_argument('--context')
    args = parser.parse_args()
    seeds = sorted(SEEDS.glob('*.icl'))
    if args.context:
        seeds = [s for s in seeds if s.stem == args.context]
    names = (SEEDS / 'names.txt').read_text().split()
    if not seeds or not names:
        parser.error('no matching contexts or names')
    if len(names) != len(set(names)) or not all(re.fullmatch(r'[A-Za-z][A-Za-z0-9_]*', n) for n in names):
        parser.error('valid names must be unique IEEE scalar identifiers')
    binary = str(args.binary.resolve())
    formatted_templates = {}
    for seed in seeds:
        run = subprocess.run([binary, str(seed)], capture_output=True, text=True,
                             timeout=10, check=True)
        formatted_templates[seed.stem] = run.stdout
    output = TESTS / 'generated/ours/keyword_contexts'
    cases = []
    expected_output = {}
    for context, index, name, expected, text in combinations(seeds, names):
        path = output / context / f'{index}.icl'
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(text)
        cases.append((path, name, expected, text))
        if expected:
            spelling = 'A $suffix' if name == 'A$suffix' else name
            expected_output[path] = formatted_templates[context].replace('ProbeName', spelling)
    failures = []
    for start in range(0, len(cases), 64):
        batch = cases[start:start + 64]
        run = subprocess.run([str(args.inventory.resolve()), '--audit',
                              *[str(p) for p, _, _, _ in batch]],
                             capture_output=True, text=True, timeout=30, check=True)
        results = json.loads(run.stdout)
        if len(results) != len(batch):
            raise RuntimeError('inventory result count differs from input count')
        for (path, name, expected, _), result in zip(batch, results):
            if result['ok'] != expected:
                failures.append(f'{path.relative_to(TESTS)} ({name}): acceptance != {expected}')
        positives = [(p, text) for p, _, accept, text in batch if accept]
        if not positives:
            continue
        run = subprocess.run([binary, *[str(p) for p, _ in positives]],
                             capture_output=True, text=True, timeout=30)
        source = '\n'.join(text for _, text in positives)
        expected_text = ''.join(expected_output[p] for p, _ in positives)
        if (run.returncode or run.stdout != expected_text or
                CONTENT.findall(source) != CONTENT.findall(run.stdout)):
            failures.append(f'CLI batch {start}: rejection or changed content: {run.stderr}')
            continue
        again = subprocess.run([binary], input=run.stdout, capture_output=True,
                               text=True, timeout=30)
        if again.returncode or again.stdout != run.stdout:
            failures.append(f'CLI batch {start}: reparse/idempotence failed: {again.stderr}')

    client = Client(str(args.lsp.resolve()))
    uri = 'file:///nonexistent/icl-keyword-contexts.icl'
    try:
        client.request('initialize', {'capabilities': {}})
        client.notify('initialized')
        for version, (path, name, expected, text) in enumerate(cases, 1):
            document = {'uri': uri, 'version': version}
            if version == 1:
                client.notify('textDocument/didOpen', {'textDocument': {
                    **document, 'languageId': 'icl', 'text': text}})
            else:
                client.notify('textDocument/didChange', {
                    'textDocument': document, 'contentChanges': [{'text': text}]})
            diagnostic = client.wait(lambda m: m.get('method') ==
                                     'textDocument/publishDiagnostics' and
                                     m['params']['uri'] == uri, 'keyword diagnostics')
            label = f'{path.relative_to(TESTS)} ({name})'
            if diagnostic is None:
                raise RuntimeError(f'{label}: no diagnostics notification')
            params = diagnostic['params']
            if (not params['diagnostics']) != expected or params.get('version') != version:
                failures.append(f'{label}: LSP acceptance/version mismatch: {params}')
            response = client.request('textDocument/formatting', {
                'textDocument': {'uri': uri}, 'options': {'tabSize': 2}})
            if response is None or 'result' not in response:
                raise RuntimeError(f'{label}: formatting response missing: {response}')
            edits = response['result']
            if not expected:
                if edits:
                    failures.append(f'{label}: invalid input produced formatting edits')
            elif not formatting_matches(text, expected_output[path], edits):
                failures.append(f'{label}: LSP formatting text/range differs from CLI')
    finally:
        client.stop()
    report = {'contexts': [p.stem for p in seeds], 'names': names,
              'invalid_names': INVALID_NAMES, 'cases': len(cases), 'failures': failures}
    (output / 'results.json').write_text(json.dumps(report, indent=2) + '\n')
    print(f'{len(cases)} identifier cases in {len(seeds)} contexts; '
          f'{len(failures)} failures (CLI, reparse, stability, live LSP)')
    for failure in failures[:20]:
        print('FAIL', failure)
    return bool(failures)


if __name__ == '__main__':
    raise SystemExit(main())
