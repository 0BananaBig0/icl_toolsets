#!/usr/bin/env python3
"""Bounded PDL reference lookup regression and repeatable scaling benchmark."""
import argparse
import json
from pathlib import Path
import sys
import time

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from lsp_test import Client


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
    print(json.dumps(results, indent=2))


if __name__ == '__main__':
    main()
