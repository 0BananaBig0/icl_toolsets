#!/usr/bin/env python3
"""Workspace filesystem errors must not terminate either language server."""
import argparse
import json
import os
from pathlib import Path
import resource
import subprocess
import sys
import tempfile
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from lsp_test import Client

BINARIES = {}


def limits():
    resource.setrlimit(resource.RLIMIT_CORE, (0, 0))


class WorkspaceReads(unittest.TestCase):
    def test_replaced_files_drop_stale_symbols(self):
        for language, binary in BINARIES.items():
            for kind in ('directory', 'fifo'):
                with self.subTest(language=language, kind=kind), tempfile.TemporaryDirectory() as tmp:
                    path = Path(tmp) / ('replace.' + language)
                    path.write_text('Module Stale {}\n' if language == 'icl'
                                    else 'proc Stale {} {}\n')
                    client = Client(binary)
                    try:
                        client.request('initialize', {})
                        change = {'changes': [dict(uri=path.as_uri(), type=2)]}
                        client.notify('workspace/didChangeWatchedFiles', change)
                        before = client.request('workspace/symbol', {'query': 'Stale'})
                        self.assertIsNotNone(before)
                        self.assertTrue(before['result'], before)
                        path.unlink()
                        if kind == 'directory':
                            path.mkdir()
                        else:
                            os.mkfifo(path)
                        client.notify('workspace/didChangeWatchedFiles', change)
                        after = client.request('workspace/symbol', {'query': 'Stale'})
                        self.assertIsNotNone(after)
                        self.assertEqual(after['result'], [])
                    finally:
                        client.stop()

    def test_nonregular_watched_files_are_ignored(self):
        for language, binary in BINARIES.items():
            for kind in ('directory', 'fifo'):
                with self.subTest(language=language, kind=kind), tempfile.TemporaryDirectory() as tmp:
                    path = Path(tmp) / ('unreadable.' + language)
                    if kind == 'directory':
                        path.mkdir()
                    else:
                        os.mkfifo(path)
                    messages = [
                        dict(jsonrpc='2.0', id=1, method='initialize', params={}),
                        dict(jsonrpc='2.0', method='workspace/didChangeWatchedFiles', params={
                            'changes': [dict(uri=path.as_uri(), type=2)]}),
                        dict(jsonrpc='2.0', id=2, method='workspace/symbol', params={'query': ''}),
                        dict(jsonrpc='2.0', id=3, method='shutdown', params={}),
                        dict(jsonrpc='2.0', method='exit', params={}),
                    ]
                    wire = b''
                    for message in messages:
                        body = json.dumps(message).encode()
                        wire += b'Content-Length: %d\r\n\r\n' % len(body) + body
                    try:
                        run = subprocess.run([binary], input=wire, capture_output=True,
                                             timeout=3, preexec_fn=limits)
                    except subprocess.TimeoutExpired:
                        self.fail('language server blocked reading a nonregular file')
                    self.assertEqual(run.returncode, 0, run.stderr)
                    replies = []
                    remaining = run.stdout
                    while remaining:
                        header, remaining = remaining.split(b'\r\n\r\n', 1)
                        size = int(header.split(b':', 1)[1])
                        replies.append(json.loads(remaining[:size]))
                        remaining = remaining[size:]
                    self.assertEqual([r for r in replies if r.get('id') == 2],
                                     [dict(jsonrpc='2.0', id=2, result=[])])


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--icl', required=True)
    parser.add_argument('--pdl', required=True)
    args, rest = parser.parse_known_args()
    BINARIES = {'icl': str(Path(args.icl).resolve()), 'pdl': str(Path(args.pdl).resolve())}
    unittest.main(argv=['test_lsp_server.py'] + rest)
