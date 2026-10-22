#!/usr/bin/env python3
"""Live syntax diagnostics against maintained expectations and editor lifecycle."""
import argparse
import json
from pathlib import Path
import sys
import unittest

TESTS = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(TESTS))
from lsp_test import Client

BINARY = 'build/icl-lsp'
FIXTURES = TESTS / 'cases/ours/diagnostics'


def fixture(name):
    return (FIXTURES / (name + '.icl')).read_bytes().decode()


def position(text, offset):
    prefix = text[:offset]
    return {'line': prefix.count('\n'),
            'character': len(prefix.rsplit('\n', 1)[-1].encode('utf-16-le')) // 2}


class Diagnostics(unittest.TestCase):
    def setUp(self):
        self.client = Client(BINARY)
        self.uri = 'file:///nonexistent/icl-diagnostics.icl'
        self.client.request('initialize', {'capabilities': {}})
        self.client.notify('initialized')

    def tearDown(self):
        self.client.stop()

    def receive(self):
        result = self.client.wait(lambda m: m.get('method') ==
                                  'textDocument/publishDiagnostics' and
                                  m['params']['uri'] == self.uri,
                                  'syntax diagnostics', timeout=2)
        self.assertIsNotNone(result)
        return result['params']

    def open(self, text, version=1):
        self.client.notify('textDocument/didOpen', {'textDocument': {
            'uri': self.uri, 'languageId': 'icl', 'version': version, 'text': text}})
        return self.receive()

    def change(self, text, version):
        self.client.notify('textDocument/didChange', {
            'textDocument': {'uri': self.uri, 'version': version},
            'contentChanges': [{'text': text}]})
        return self.receive()

    def test_precise_parser_lexer_unicode_and_nul_ranges(self):
        for name, token in [('parser', 'unexpected'), ('lexer', '@'),
                            ('unicode', '@'), ('nul', '\0')]:
            with self.subTest(name=name):
                self.uri = f'file:///nonexistent/diagnostic-{name}.icl'
                text = fixture(name)
                result = self.open(text)
                diagnostic, = result['diagnostics']
                offset = text.index(token)
                self.assertEqual(diagnostic['range'], {
                    'start': position(text, offset),
                    'end': position(text, offset + len(token))})
                self.assertEqual(diagnostic['severity'], 1)
                self.assertEqual(diagnostic['source'], 'icl-lsp')
                self.client.notify('textDocument/didClose', {'textDocument': {'uri': self.uri}})
                self.receive()

    def test_eof_range_is_at_document_end(self):
        for name in ['eof', 'generic_eof', 'generic_eof_newline']:
            with self.subTest(name=name):
                self.uri = f'file:///nonexistent/diagnostic-{name}.icl'
                text = fixture(name)
                diagnostic, = self.open(text)['diagnostics']
                end = position(text, len(text))
                self.assertEqual(diagnostic['range'], {'start': end, 'end': end})

    def test_semantic_range_excludes_crlf(self):
        text = fixture('semantic_crlf')
        diagnostic, = self.open(text)['diagnostics']
        line = diagnostic['range']['start']['line']
        self.assertEqual(diagnostic['range'], {
            'start': {'line': line, 'character': 0},
            'end': {'line': line, 'character': len(text.splitlines()[line])}})

    def test_versions_correction_close_and_reopen(self):
        self.assertEqual(self.open(fixture('lexer'), 4)['version'], 4)
        corrected = self.change(fixture('valid'), 5)
        self.assertEqual(corrected['version'], 5)
        self.assertEqual(corrected['diagnostics'], [])
        # A stale edit must neither replace the valid document nor publish errors.
        self.client.notify('textDocument/didChange', {
            'textDocument': {'uri': self.uri, 'version': 3},
            'contentChanges': [{'text': fixture('lexer')}]})
        reply = self.client.request('textDocument/formatting', {
            'textDocument': {'uri': self.uri}, 'options': {'tabSize': 2}})
        self.assertNotIn('error', reply)
        self.assertFalse(self.client.drain(timeout=0.01))
        self.client.notify('textDocument/didClose', {'textDocument': {'uri': self.uri}})
        closed = self.receive()
        self.assertEqual(closed['diagnostics'], [])
        self.assertNotIn('version', closed)
        reopened = self.open(fixture('parser'), 1)
        self.assertEqual(reopened['version'], 1)
        self.assertTrue(reopened['diagnostics'])

    def test_all_conformance_expectations_through_live_edits(self):
        self.open(fixture('valid'), 0)
        cases = json.loads((TESTS / 'cases/conformance.json').read_text())
        for version, case in enumerate(cases, 1):
            with self.subTest(path=case['path']):
                text = (TESTS / case['path']).read_text()
                result = self.change(text, version)
                self.assertEqual(not result['diagnostics'], case['accept'])
                self.assertEqual(result.get('version'), version)


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--binary', default=BINARY)
    args, remaining = parser.parse_known_args()
    BINARY = str(Path(args.binary).resolve())
    unittest.main(argv=[sys.argv[0], *remaining])
