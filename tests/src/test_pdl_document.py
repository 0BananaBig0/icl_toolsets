#!/usr/bin/env python3
"""Bounded PDL reference lookup regression and repeatable scaling benchmark."""
import argparse
import json
from pathlib import Path
import sys
import time

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from lsp_test import Client


def object_references(server, benchmark):
    results = []
    for size in ((2000, 4000, 8000, 16000, 32000) if benchmark else (32000,)):
        client = Client(str(server.resolve()))
        try:
            client.request('initialize', {})
            sources = {
                'file:///objects.icl': 'Module M {\n' + ''.join(
                    f'DataInPort r{i};\n' for i in range(size)) + '}\n',
                'file:///references.pdl': 'iProcsForModule M\n' + ''.join(
                    f'iRead r{i}\n' for i in range(size)),
            }
            for uri, source in sources.items():
                client.notify('textDocument/didOpen', {'textDocument': dict(
                    uri=uri, languageId=uri.rsplit('.', 1)[1], version=1, text=source)})
                diagnostics = client.wait(
                    lambda m: m.get('method') == 'textDocument/publishDiagnostics'
                    and m['params']['uri'] == uri, 'diagnostics')
                assert diagnostics and not diagnostics['params']['diagnostics'], diagnostics
            query = dict(textDocument={'uri': 'file:///references.pdl'},
                         position=dict(line=1, character=7),
                         context={'includeDeclaration': False})
            times = []
            for _ in range(3 if benchmark else 1):
                start = time.monotonic()
                reply = client.request('textDocument/references', query)
                times.append(time.monotonic() - start)
                assert reply and [r['range']['start']['line'] for r in reply['result']] == [1], reply
            result = dict(objects=size, bytes=sum(len(s) for s in sources.values()), seconds=times)
            results.append(result)
            if not benchmark:
                assert times[0] < 2.0, result
            # Reindexing must remove old members and retain genuine ambiguity.
            for version, text in ((2, 'Module M { DataInPort other; }\n'),
                                  (3, 'Module M {}\nModule M {}\n')):
                client.notify('textDocument/didChange', {
                    'textDocument': dict(uri='file:///objects.icl', version=version),
                    'contentChanges': [{'text': text}]})
                reply = client.request('textDocument/references', query)
                assert reply and reply['result'] == [], reply
        finally:
            client.stop()
    return results


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--server', required=True, type=Path)
    parser.add_argument('--benchmark', action='store_true')
    args = parser.parse_args()
    results = []
    for size in ((2000, 4000, 8000) if args.benchmark else (32000,)):
        client = Client(str(args.server.resolve()))
        try:
            client.request('initialize', {})
            source = ''.join(f'set v{i} {i}\n' for i in range(size))
            source += ''.join(f'puts $v{i}\n' for i in range(size))
            uri = 'file:///pdl-reference-scaling.pdl'
            client.notify('textDocument/didOpen', {'textDocument': dict(
                uri=uri, languageId='pdl', version=1, text=source)})
            diagnostic = client.wait(
                lambda m: m.get('method') == 'textDocument/publishDiagnostics',
                'diagnostics')
            assert diagnostic and not diagnostic['params']['diagnostics'], diagnostic
            times = []
            for _ in range(3 if args.benchmark else 1):
                start = time.monotonic()
                result = client.request('textDocument/references', dict(
                    textDocument={'uri': uri}, position=dict(line=0, character=4),
                    context={'includeDeclaration': True}))
                times.append(time.monotonic() - start)
                assert result and 'result' in result, result
                assert [ref['range']['start']['line'] for ref in result['result']] == [0, size], result
            results.append(dict(variables=size, bytes=len(source), seconds=times))
            # A 0.9 MiB document must not block the single-threaded server for
            # seconds. This catches repeated full declaration scans, with ample
            # headroom over indexed lookup; benchmark mode only records timings.
            if not args.benchmark:
                assert times[0] < 2.0, results[-1]
        finally:
            client.stop()
    results.extend(object_references(args.server, args.benchmark))
    print(json.dumps(results, indent=2))


if __name__ == '__main__':
    main()
