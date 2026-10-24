#!/usr/bin/env python3
"""Shared LSP transport, document-sync and workspace filesystem regressions."""
import argparse
import json
import os
from pathlib import Path
import resource
import subprocess
import sys
import tempfile
import time
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from lsp_test import Client

BINARIES = {}


def limits():
    resource.setrlimit(resource.RLIMIT_CORE, (0, 0))


class WorkspaceReads(unittest.TestCase):
    def test_reference_positions_scale_on_long_unicode_lines(self):
        for language, binary in BINARIES.items():
            with self.subTest(language=language):
                client = Client(binary)
                try:
                    client.request('initialize', {})
                    uri = 'file:///long-line.' + language
                    count = 16000 if language == 'icl' else 32000
                    prefix = ('Module M { DataInPort p; ' if language == 'icl'
                              else 'set p 1; ')
                    declaration = prefix.index(' p') + 1
                    chunks = [prefix]
                    expected = [declaration]
                    position = len(prefix)
                    for i in range(count):
                        chunk = (f'/*é汉😀*/ DataOutPort q{i} {{ Source p; }} ' if language == 'icl'
                                 else 'puts {é汉😀}; puts $p; ')
                        use = chunk.index(' p;') + 1 if language == 'icl' else chunk.index('$p') + 1
                        expected.append(position + len(chunk[:use].encode('utf-16-le')) // 2)
                        position += len(chunk.encode('utf-16-le')) // 2
                        chunks.append(chunk)
                    chunks.append('}\n' if language == 'icl' else '\n')
                    source = ''.join(chunks)
                    client.notify('textDocument/didOpen', {'textDocument': dict(
                        uri=uri, languageId=language, version=1, text=source)})
                    diagnostic = client.wait(lambda m: m.get('method') ==
                                             'textDocument/publishDiagnostics', 'diagnostics')
                    self.assertEqual(diagnostic['params']['diagnostics'], [])
                    start = time.monotonic()
                    result = client.request('textDocument/references', {
                        'textDocument': {'uri': uri},
                        'position': dict(line=0, character=declaration),
                        'context': {'includeDeclaration': True}})['result']
                    seconds = time.monotonic() - start
                    self.assertLess(seconds, 2.0, f'{language}: {len(source.encode())} bytes, {seconds}s')
                    self.assertEqual([r['range']['start']['character'] for r in result], expected)
                    self.assertTrue(all(r['range']['start']['line'] == 0 and
                                        r['range']['end'] == dict(line=0, character=e + 1)
                                        for r, e in zip(result, expected)))
                    # A replacement document must not retain the old checkpoints.
                    replacement = ('Module M { /*😀*/ DataInPort p; DataOutPort q { Source p; } }\n'
                                   if language == 'icl' else 'set label {😀}; set p 1; puts $p;\n')
                    use = replacement.rindex(' p;') + 1 if language == 'icl' else replacement.index('$p') + 1
                    declaration = replacement.index(' p ') + 1 if language == 'pdl' else replacement.index(' p;') + 1
                    client.notify('textDocument/didChange', {
                        'textDocument': dict(uri=uri, version=2),
                        'contentChanges': [{'text': replacement}]})
                    result = client.request('textDocument/definition', {
                        'textDocument': {'uri': uri}, 'position': dict(
                            line=0, character=len(replacement[:use].encode('utf-16-le')) // 2)})['result']
                    self.assertEqual(result[0]['range']['start'], dict(
                        line=0, character=len(replacement[:declaration].encode('utf-16-le')) // 2))
                finally:
                    client.stop()

    def test_invalid_utf8_disk_text_never_reaches_protocol(self):
        invalid = (b'\xe9', b'\x80', b'\xc0\xaf', b'\xe0\x80\x80',
                   b'\xed\xa0\x80', b'\xf0\x80\x80\x80',
                   b'\xf4\x90\x80\x80', b'\xf5\x80\x80\x80')
        for language, binary in BINARIES.items():
            for value in invalid:
                with self.subTest(language=language, value=value), tempfile.TemporaryDirectory() as tmp:
                    root = Path(tmp)
                    path = root / ('invalid.' + language)
                    path.write_bytes(b'Module Bad { Parameter P = "caf' + value + b'"; }\n'
                                     if language == 'icl' else b'proc caf' + value + b' {} {}\n')
                    messages = [
                        dict(id=1, method='initialize', params={'rootUri': root.as_uri()}),
                        dict(id=2, method='workspace/symbol', params={'query': ''}),
                        dict(id=3, method='textDocument/hover', params={
                            'textDocument': {'uri': path.as_uri()},
                            'position': {'line': 0, 'character': 23}}),
                        dict(id=4, method='shutdown', params={}),
                        dict(method='exit', params={}),
                    ]
                    wire = b''
                    for message in messages:
                        body = json.dumps(dict(jsonrpc='2.0', **message)).encode()
                        wire += b'Content-Length: %d\r\n\r\n' % len(body) + body
                    run = subprocess.run([binary], input=wire, capture_output=True, timeout=10)
                    self.assertEqual(run.returncode, 0, run.stderr)
                    replies = {}
                    output = run.stdout
                    while output:
                        header, output = output.split(b'\r\n\r\n', 1)
                        length = int(header.split(b':', 1)[1])
                        reply = json.loads(output[:length].decode('utf-8'))
                        output = output[length:]
                        if 'id' in reply:
                            replies[reply['id']] = reply
                    self.assertEqual(replies[2]['result'], [])
                    self.assertIsNone(replies[3]['result'])
                    self.assertIn(b'UTF-8', run.stderr)

    def test_equivalent_uri_delete_removes_disk_alias(self):
        for language, binary in BINARIES.items():
            for symlink in (False, True):
                with self.subTest(language=language, symlink=symlink), tempfile.TemporaryDirectory() as tmp:
                    root = Path(tmp)
                    workspace = root / 'workspace'
                    workspace.mkdir()
                    path = workspace / ('caf\u00e9.' + language)
                    source = 'Module Stale {}\n' if language == 'icl' else 'proc Stale {} {}\n'
                    if symlink:
                        target = root / ('external.' + language)
                        target.write_text(source)
                        path.symlink_to(target)
                    else:
                        path.write_text(source)
                    client = Client(binary)
                    try:
                        client.request('initialize', {'rootUri': workspace.as_uri()})
                        query = lambda: client.request('workspace/symbol', {'query': 'Stale'})['result']
                        self.assertEqual(len(query()), 1)
                        path.unlink()
                        uri = path.as_uri().replace('%C3%A9', '%c3%a9').replace('caf', '%63af')
                        client.notify('workspace/didChangeWatchedFiles', {
                            'changes': [dict(uri=uri, type=3)]})
                        self.assertEqual(query(), [])
                    finally:
                        client.stop()

    def test_encoding_watcher_recovery_preserves_open_overlay(self):
        for language, binary in BINARIES.items():
            with self.subTest(language=language), tempfile.TemporaryDirectory() as tmp:
                root = Path(tmp)
                path = root / ('encoding.' + language)
                source = ('Module Good { Parameter P = "caf\u00e9 \u6c49 \U0001f600"; }\n'
                          if language == 'icl' else 'proc Good {} {set x "caf\u00e9 \u6c49 \U0001f600"}\n')
                path.write_text(source)
                client = Client(binary)
                try:
                    client.request('initialize', {'rootUri': root.as_uri()})
                    query = lambda word: client.request('workspace/symbol', {'query': word})['result']
                    self.assertEqual(len(query('Good')), 1)
                    path.write_bytes(source.encode() + b'# truncated: \xf0\x90')
                    client.notify('workspace/didChangeWatchedFiles', {
                        'changes': [dict(uri=path.as_uri(), type=2)]})
                    self.assertEqual(query('Good'), [])
                    client.notify('textDocument/didOpen', {'textDocument': dict(
                        uri=path.as_uri(), languageId=language, version=1,
                        text=source.replace('Good', 'Overlay'))})
                    client.notify('workspace/didChangeWatchedFiles', {
                        'changes': [dict(uri=path.as_uri(), type=2)]})
                    self.assertEqual(len(query('Overlay')), 1)
                    client.notify('textDocument/didClose', {'textDocument': {'uri': path.as_uri()}})
                    self.assertEqual(query('Overlay'), [])
                    path.write_text(source)
                    client.notify('workspace/didChangeWatchedFiles', {
                        'changes': [dict(uri=path.as_uri(), type=2)]})
                    self.assertEqual(len(query('Good')), 1)
                finally:
                    client.stop()

    @unittest.skipUnless(Path('/proc/self/io').exists(), 'requires Linux process I/O counters')
    def test_close_reads_ordinary_file_once(self):
        for language, binary in BINARIES.items():
            for invalid in (False, True):
                with self.subTest(language=language, invalid=invalid), tempfile.TemporaryDirectory() as tmp:
                    path = Path(tmp) / ('large.' + language)
                    source = ('Module Large {\n' + ''.join(f'DataInPort p{i};\n' for i in range(16000)) + '}\n'
                              if language == 'icl' else ''.join(f'set v{i} {i}\n' for i in range(16000)))
                    disk = source.encode() + (b'\xe9' if invalid else b'')
                    path.write_bytes(disk)
                    client = Client(binary)
                    try:
                        client.request('initialize', {'rootUri': Path(tmp).as_uri()})
                        client.notify('textDocument/didOpen', {'textDocument': dict(
                            uri=path.as_uri(), languageId=language, version=1, text=source)})
                        barrier = lambda: client.request('workspace/symbol', {'query': 'ZZmissing'})
                        barrier()
                        def read_bytes():
                            counters = dict(line.split(': ') for line in Path(f'/proc/{client.proc.pid}/io').read_text().splitlines())
                            return int(counters['rchar'])
                        before = read_bytes()
                        client.notify('textDocument/didClose', {'textDocument': {'uri': path.as_uri()}})
                        barrier()
                        count = read_bytes() - before
                        self.assertGreaterEqual(count, len(disk))
                        self.assertLess(count, len(disk) + 16384, f'duplicate disk reads: {count}')
                    finally:
                        client.stop()

    def test_watcher_preserves_scan_exclusions(self):
        for language, binary in BINARIES.items():
            for directory in ('build', '.generated'):
                with self.subTest(language=language, directory=directory), tempfile.TemporaryDirectory() as tmp:
                    root = Path(tmp)
                    source = 'Module Child {}\n' if language == 'icl' else 'proc child {} {}\n'
                    library = root / ('library.' + language)
                    library.write_text(source)
                    excluded = root / directory / library.name
                    excluded.parent.mkdir()
                    excluded.write_text(source)
                    client = Client(binary)
                    try:
                        client.request('initialize', {'rootUri': root.as_uri()})
                        query = lambda: client.request('workspace/symbol', {'query': 'Child' if language == 'icl' else 'child'})['result']
                        self.assertEqual(len(query()), 1)
                        for event in (1, 2):
                            client.notify('workspace/didChangeWatchedFiles', {
                                'changes': [dict(uri=excluded.as_uri(), type=event)]})
                            self.assertEqual(len(query()), 1)
                        # Exclusion is automatic-index policy, not an editor ban.
                        client.notify('textDocument/didOpen', {'textDocument': dict(
                            uri=excluded.as_uri(), languageId=language, version=1, text=source)})
                        self.assertEqual(len(query()), 2)
                        client.notify('textDocument/didClose', {'textDocument': {'uri': excluded.as_uri()}})
                        self.assertEqual(len(query()), 1)
                    finally:
                        client.stop()

    def test_symlink_identity_and_unsaved_overlays(self):
        for language, binary in BINARIES.items():
            with self.subTest(language=language), tempfile.TemporaryDirectory() as tmp:
                root = Path(tmp)
                library = root / ('library.' + language)
                alias = root / ('alias.' + language)
                source = 'Module Child {}\n' if language == 'icl' else 'proc child {} {}\n'
                name = 'Child' if language == 'icl' else 'child'
                library.write_text(source)
                alias.symlink_to(library)
                client = Client(binary)
                try:
                    client.request('initialize', {'rootUri': root.as_uri()})
                    query = lambda word: client.request('workspace/symbol', {'query': word})['result']
                    self.assertEqual(len(query(name)), 1)
                    for path, version, replacement in ((alias, 10, 'Edited'), (library, 1, 'Second')):
                        client.notify('textDocument/didOpen', {'textDocument': dict(
                            uri=path.as_uri(), languageId=language, version=version,
                            text=source.replace(name, replacement))})
                        symbols = query(replacement)
                        self.assertEqual(len(symbols), 1)
                        self.assertEqual(symbols[0]['location']['uri'], path.as_uri())
                    client.notify('workspace/didChangeWatchedFiles', {
                        'changes': [dict(uri=alias.as_uri(), type=2)]})
                    self.assertEqual(len(query('Second')), 1)
                    for path, expected in ((alias, 'Edited'), (library, 'Second')):
                        symbols = client.request('textDocument/documentSymbol', {
                            'textDocument': {'uri': path.as_uri()}})['result']
                        self.assertEqual([s['name'] for s in symbols], [expected])
                        self.assertEqual(symbols[0]['location']['uri'], path.as_uri())
                    client.notify('textDocument/didClose', {'textDocument': {'uri': library.as_uri()}})
                    symbols = query('Edited')
                    self.assertEqual(len(symbols), 1)
                    self.assertEqual(symbols[0]['location']['uri'], alias.as_uri())
                    client.notify('textDocument/didClose', {'textDocument': {'uri': alias.as_uri()}})
                    self.assertEqual(len(query(name)), 1)
                finally:
                    client.stop()

    def test_watcher_skips_symlink_directories_but_explicit_roots_work(self):
        for language, binary in BINARIES.items():
            with self.subTest(language=language), tempfile.TemporaryDirectory() as tmp:
                root = Path(tmp)
                workspace = root / 'workspace'
                workspace.mkdir()
                external = root / 'external'
                external.mkdir()
                link = workspace / 'linked'
                link.symlink_to(external, target_is_directory=True)
                source = 'Module External {}\n' if language == 'icl' else 'proc External {} {}\n'
                path = link / ('library.' + language)
                path.write_text(source)
                client = Client(binary)
                try:
                    client.request('initialize', {'rootUri': workspace.as_uri()})
                    query = lambda: client.request('workspace/symbol', {'query': 'External'})['result']
                    self.assertEqual(query(), [])
                    client.notify('workspace/didChangeWatchedFiles', {
                        'changes': [dict(uri=path.as_uri(), type=2)]})
                    self.assertEqual(query(), [])
                    client.notify('textDocument/didOpen', {'textDocument': dict(
                        uri=path.as_uri(), languageId=language, version=1, text=source)})
                    self.assertEqual(len(query()), 1)
                    client.notify('textDocument/didClose', {'textDocument': {'uri': path.as_uri()}})
                    self.assertEqual(query(), [])
                finally:
                    client.stop()
                client = Client(binary)
                try:
                    client.request('initialize', {'workspaceFolders': [
                        dict(uri=workspace.as_uri(), name='parent'),
                        dict(uri=link.as_uri(), name='explicit')]})
                    self.assertEqual(len(client.request('workspace/symbol', {'query': 'External'})['result']), 1)
                    path.write_text(source.replace('External', 'Updated'))
                    client.notify('workspace/didChangeWatchedFiles', {
                        'changes': [dict(uri=path.as_uri(), type=2)]})
                    self.assertEqual(len(client.request('workspace/symbol', {'query': 'Updated'})['result']), 1)
                finally:
                    client.stop()

    def test_client_responses_do_not_receive_responses(self):
        for language, binary in BINARIES.items():
            with self.subTest(language=language):
                client = Client(binary)
                try:
                    client.request('initialize', {'capabilities': {'workspace': {
                        'didChangeWatchedFiles': {'dynamicRegistration': True}}}})
                    client.notify('initialized', {})
                    registration = client.wait(
                        lambda m: m.get('method') == 'client/registerCapability',
                        'watcher registration')
                    self.assertIsNotNone(registration)
                    for payload in ({'result': None}, {'error': {
                            'code': -32601, 'message': 'registration unavailable'}}):
                        client.send(dict(jsonrpc='2.0', id=registration['id'], **payload))
                        # A following request is a barrier: no timing-based sleep.
                        reply = client.request('workspace/symbol', {'query': ''})
                        self.assertIsNotNone(reply)
                        self.assertEqual(client.drain(), [])
                finally:
                    client.stop()

    def test_symlink_retarget_and_target_deletion_remove_stale_symbols(self):
        for language, binary in BINARIES.items():
            with self.subTest(language=language), tempfile.TemporaryDirectory() as tmp:
                root = Path(tmp)
                workspace = root / 'workspace'
                workspace.mkdir()
                source = 'Module Old {}\n' if language == 'icl' else 'proc Old {} {}\n'
                old = root / ('old.' + language)
                new = root / ('new.' + language)
                old.write_text(source)
                new.write_text(source.replace('Old', 'New'))
                alias = workspace / ('alias.' + language)
                alias.symlink_to(old)
                client = Client(binary)
                try:
                    client.request('initialize', {'rootUri': workspace.as_uri()})
                    query = lambda: [s['name'] for s in client.request('workspace/symbol', {'query': ''})['result']]
                    self.assertEqual(query(), ['Old'])
                    alias.unlink()
                    alias.symlink_to(new)
                    client.notify('workspace/didChangeWatchedFiles', {
                        'changes': [dict(uri=alias.as_uri(), type=2)]})
                    self.assertEqual(query(), ['New'])
                    second = workspace / ('second.' + language)
                    second.symlink_to(new)
                    client.notify('workspace/didChangeWatchedFiles', {
                        'changes': [dict(uri=second.as_uri(), type=1)]})
                    self.assertEqual(query(), ['New'])
                    for opened in (alias, second):
                        client.notify('textDocument/didOpen', {'textDocument': dict(
                            uri=opened.as_uri(), languageId=language, version=1,
                            text=source.replace('Old', 'Buffered'))})
                    alias.unlink()
                    alias.symlink_to(old)
                    # Closing an inactive alias must refresh its retarget even
                    # when the client's watcher has not delivered an event.
                    client.notify('textDocument/didClose', {'textDocument': {'uri': alias.as_uri()}})
                    self.assertEqual(sorted(query()), ['Buffered', 'Old'])
                    client.notify('textDocument/didClose', {'textDocument': {'uri': second.as_uri()}})
                    self.assertEqual(sorted(query()), ['New', 'Old'])
                    new.unlink()
                    client.notify('workspace/didChangeWatchedFiles', {
                        'changes': [dict(uri=second.as_uri(), type=2)]})
                    self.assertEqual(query(), ['Old'])
                finally:
                    client.stop()

    def test_ranged_changes_do_not_replace_full_document(self):
        for language, binary in BINARIES.items():
            with self.subTest(language=language), tempfile.TemporaryDirectory() as tmp:
                client = Client(binary)
                try:
                    client.request('initialize', {})
                    uri = (Path(tmp) / ('buffer.' + language)).as_uri()
                    source = ('Module Original {}\n' if language == 'icl'
                              else 'proc Original {} {}\n')
                    client.notify('textDocument/didOpen', {'textDocument': dict(
                        uri=uri, languageId=language, version=1, text=source)})
                    client.notify('textDocument/didChange', {
                        'textDocument': dict(uri=uri, version=2),
                        'contentChanges': [{'range': {
                            'start': dict(line=0, character=0),
                            'end': dict(line=0, character=1)}, 'text': ''}]})
                    reply = client.request('workspace/symbol', {'query': 'Original'})
                    self.assertIsNotNone(reply)
                    self.assertEqual([s['name'] for s in reply['result']], ['Original'])
                    # An ignored change must not advance the stored version.
                    client.notify('textDocument/didChange', {
                        'textDocument': dict(uri=uri, version=2),
                        'contentChanges': [{'text': source.replace('Original', 'Updated')}]})
                    reply = client.request('workspace/symbol', {'query': 'Updated'})
                    self.assertIsNotNone(reply)
                    self.assertEqual([s['name'] for s in reply['result']], ['Updated'])
                finally:
                    client.stop()

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
