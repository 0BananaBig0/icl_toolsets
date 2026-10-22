#!/usr/bin/env python3
"""Measure open/format/symbol request latency through the real stdio transport."""
import argparse
import json
from pathlib import Path
import statistics
import sys
import tempfile
import time
sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from lsp_test import Client


def main():
    ap = argparse.ArgumentParser(); ap.add_argument('--binary', type=Path, required=True); ap.add_argument('--output', type=Path, required=True)
    args = ap.parse_args()
    root = Path(__file__).resolve().parents[2]
    files = [root/'tests/cases/pass/tessent/tessent_merged_icl.icl', root/'tests/generated/ours/performance/merged_x16.icl']
    results = []
    for path in files:
        text = path.read_text(); uri = path.as_uri()
        with tempfile.TemporaryDirectory(prefix='icl-lsp-perf-') as tmp:
            client = Client(str(args.binary.resolve()))
            try:
                client.request('initialize', {'rootUri': Path(tmp).as_uri()})
                start = time.perf_counter()
                client.notify('textDocument/didOpen', {'textDocument': {'uri': uri, 'text': text, 'languageId': 'icl', 'version': 1}})
                diagnostic = client.wait(lambda m: m.get('method') == 'textDocument/publishDiagnostics', 'diagnostic', timeout=30)
                assert diagnostic and not diagnostic['params']['diagnostics'], diagnostic
                row = dict(path=str(path.relative_to(root)), bytes=path.stat().st_size, open_seconds=time.perf_counter()-start)
                for method in ['formatting', 'documentSymbol']:
                    timings = []
                    for _ in range(5):
                        start = time.perf_counter()
                        reply = client.request('textDocument/'+method, {'textDocument': {'uri': uri}, 'options': {'tabSize': 2}})
                        assert reply and 'result' in reply, reply
                        timings.append(time.perf_counter()-start)
                    row[method+'_median_seconds'] = statistics.median(timings)
                results.append(row)
            finally: client.stop()
    args.output.write_text(json.dumps(results, indent=2)+'\n')
    print(json.dumps(results, indent=2))

if __name__ == '__main__': main()
