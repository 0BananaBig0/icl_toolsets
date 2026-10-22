#!/usr/bin/env python3
"""Real protocol tests for shared-AST semantic language services."""
import argparse
import shutil
import sys
import tempfile
from pathlib import Path
import unittest
sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from lsp_test import Client

CASES = Path(__file__).resolve().parents[1] / 'cases/ours/lsp'
BINARY = None


def position(text, needle, occurrence=0, offset=0):
    start = -1
    for _ in range(occurrence + 1):
        start = text.index(needle, start + 1)
    start += offset
    line = text.count('\n', 0, start)
    column = len(text[text.rfind('\n', 0, start) + 1:start].encode('utf-16-le')) // 2
    return dict(line=line, character=column)


class LanguageServices(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory(prefix='icl-lsp-')
        self.root = Path(self.tmp.name)
        for src in (CASES / 'main.icl', CASES / 'library.icl'):
            shutil.copy(src, self.root)
        self.main = (self.root / 'main.icl').read_text()
        self.lib = (self.root / 'library.icl').read_text()
        self.uri = (self.root / 'main.icl').as_uri()
        self.lib_uri = (self.root / 'library.icl').as_uri()
        self.client = Client(BINARY)
        self.capabilities = self.result('initialize', {'rootUri': self.root.as_uri()})['capabilities']
        self.open(self.uri, self.main)

    def tearDown(self):
        self.client.stop()
        self.tmp.cleanup()

    def result(self, method, params):
        reply = self.client.request(method, params)
        self.assertIsNotNone(reply, method)
        self.assertNotIn('error', reply, reply)
        return reply['result']

    def open(self, uri, text, version=1):
        self.client.notify('textDocument/didOpen', {'textDocument': dict(uri=uri, text=text, languageId='icl', version=version)})
        return self.client.wait(lambda m: m.get('method') == 'textDocument/publishDiagnostics' and m['params']['uri'] == uri, 'diagnostics')

    def query(self, method, needle, occurrence=0, offset=0, **extra):
        return self.result('textDocument/' + method, dict(textDocument={'uri': self.uri}, position=position(self.main, needle, occurrence, offset), **extra))

    def expect_location(self, result, uri, text, needle, occurrence=0, offset=0, length=None):
        self.assertEqual(len(result), 1, result)
        self.assertEqual(result[0]['uri'], uri)
        start = position(text, needle, occurrence, offset)
        self.assertEqual(result[0]['range']['start'], start)
        if length is not None:
            self.assertEqual(result[0]['range']['end'], dict(line=start['line'], character=start['character'] + length))

    def test_capabilities(self):
        for key in ['completionProvider', 'definitionProvider', 'declarationProvider', 'implementationProvider', 'hoverProvider', 'referencesProvider', 'documentSymbolProvider', 'workspaceSymbolProvider']:
            self.assertTrue(self.capabilities.get(key), key)
        self.assertFalse(self.capabilities.get('renameProvider'))

    def test_local_alias_definition(self):
        self.expect_location(self.query('definition', 'Source readback', offset=8), self.uri, self.main, 'Alias readback', offset=6, length=8)

    def test_parameter_reference_definition(self):
        self.expect_location(self.query('definition', '$Width', offset=2), self.uri, self.main, 'Parameter Width', offset=10, length=5)

    def test_unopened_module_definition(self):
        self.expect_location(self.query('definition', 'Of Leaf', offset=4), self.lib_uri, self.lib, 'Module Leaf', offset=7, length=4)

    def test_instance_member_definition(self):
        self.expect_location(self.query('definition', 'child.output', offset=8), self.lib_uri, self.lib, 'DataOutPort output', offset=12, length=6)

    def test_instance_override_lhs_and_rhs(self):
        self.expect_location(self.query('definition', 'InputPort input', offset=11), self.lib_uri, self.lib, 'DataInPort input', offset=11, length=5)
        self.expect_location(self.query('definition', 'input = input', offset=9), self.uri, self.main, 'DataInPort input', offset=11, length=5)

    def test_implementation_means_icl_module(self):
        result = self.query('implementation', 'child.output', offset=2)
        self.expect_location(result, self.lib_uri, self.lib, 'Module Leaf')
        self.assertEqual(result[0]['range']['end']['line'], 5)

    def test_completion_in_scope(self):
        items = self.query('completion', 'Alias readback', offset=0)
        labels = {i['label'] for i in items}
        self.assertTrue({'ScanRegister', 'DataOutPort', 'input', 'Width', 'child', 'readback'} <= labels, labels)
        self.assertNotIn('different', labels)

    def test_member_completion(self):
        items = self.query('completion', 'child.output', offset=6)
        self.assertEqual({i['label'] for i in items}, {'input', 'output', 'Width'})

    def test_scoped_references(self):
        refs = self.query('references', 'DataInPort input', offset=12, context={'includeDeclaration': True})
        self.assertEqual(len(refs), 2, refs)
        self.assertTrue(all(x['uri'] == self.uri for x in refs))
        self.assertEqual({x['range']['start']['line'] for x in refs}, {3, 4})

    def test_hover_and_symbols(self):
        hover = self.query('hover', 'Source readback', offset=10)
        self.assertIn('Alias', str(hover))
        self.assertIn('readback', str(hover))
        symbols = self.result('textDocument/documentSymbol', {'textDocument': {'uri': self.uri}})
        self.assertTrue({'Top', 'Other', 'readback', 'Mode', 'ON'} <= {s['name'] for s in symbols}, symbols)
        symbols = self.result('workspace/symbol', {'query': 'Leaf'})
        self.assertEqual(len(symbols), 2)

    def test_namespace_qualification(self):
        self.main = self.main.replace('Of Leaf', 'Of other::Leaf')
        self.open(self.uri, self.main, 2)
        self.expect_location(self.query('declaration', 'other::Leaf', offset=8), self.lib_uri, self.lib, 'Module Leaf', occurrence=1, offset=7, length=4)

    def test_unicode_position_and_updated_document(self):
        self.main = self.main.replace('Alias readback', '/* 汉字😀 */ Alias readback')
        self.open(self.uri, self.main, 2)
        self.expect_location(self.query('definition', 'Source readback', offset=9), self.uri, self.main, 'Alias readback', offset=6, length=8)
        changed = self.main.replace('readback', 'renamed')
        self.client.notify('textDocument/didChange', {'textDocument': {'uri': self.uri, 'version': 3}, 'contentChanges': [{'text': changed}]})
        self.main = changed
        self.expect_location(self.query('definition', 'Source renamed', offset=9), self.uri, self.main, 'Alias renamed', offset=6, length=7)

    def test_incomplete_current_buffer(self):
        self.main = 'Module Incomplete { DataInPort first; DataOutPort second { Source '
        self.open(self.uri, self.main, 2)
        labels = {i['label'] for i in self.query('completion', 'Source ', offset=7)}
        self.assertIn('first', labels)
        self.assertNotIn('readback', labels)
        self.assertEqual(self.result('textDocument/formatting', {'textDocument': {'uri': self.uri}, 'options': {'tabSize': 2}}), [])

    def use_fixture(self, name):
        self.main = (CASES / name).read_text()
        self.open(self.uri, self.main, 2)

    def test_enum_context_resolves_same_spelling(self):
        self.use_fixture('enums.icl')
        self.expect_location(self.query('definition', 'DefaultLoadValue ON', offset=18), self.uri, self.main, "ON = 1'b1", length=2)
        self.expect_location(self.query('definition', 'other == ON', offset=10), self.uri, self.main, "ON = 2'b11", length=2)

    def test_deep_alias_and_keyword_name(self):
        self.use_fixture('hierarchy.icl')
        self.expect_location(self.query('definition', 'mid.leaf.out', offset=10), self.uri, self.main, 'DataOutPort out', offset=12, length=3)
        self.expect_location(self.query('definition', 'Source Source', offset=8), self.uri, self.main, 'DataInPort Source', offset=11, length=6)

    def test_access_link_scan_interface_reference(self):
        self.use_fixture('interface.icl')
        self.expect_location(self.query('definition', 'child.access', offset=8), self.uri, self.main, 'ScanInterface access', offset=14, length=6)

    def test_module_type_completion(self):
        items = self.query('completion', 'Of Leaf', offset=3)
        self.assertIn('Leaf', {i['label'] for i in items})

    def test_comment_and_string_are_not_references(self):
        self.main = self.main.replace('Alias readback', 'Attribute note = "input"; /* input */ Alias readback')
        self.open(self.uri, self.main, 2)
        self.assertEqual(self.query('definition', '"input"', offset=3), [])
        self.assertEqual(self.query('definition', '/* input */', offset=5), [])

    def test_parameter_literal_width_reference(self):
        self.main = self.main.replace('Alias readback', "Attribute bits = $Width 'hff; Alias readback")
        self.open(self.uri, self.main, 2)
        self.expect_location(self.query('definition', "$Width 'hff", offset=2), self.uri, self.main, 'Parameter Width', offset=10, length=5)

    def test_file_watcher_registration(self):
        self.result('initialize', {'capabilities': {'workspace': {'didChangeWatchedFiles': {'dynamicRegistration': True}}}})
        self.client.notify('initialized', {})
        registration = self.client.wait(lambda m: m.get('method') == 'client/registerCapability', 'watch registration', timeout=1)
        self.assertIsNotNone(registration)
        self.assertEqual(registration['params']['registrations'][0]['registerOptions']['watchers'][0]['globPattern'], '**/*.icl')
        self.client.send({'jsonrpc': '2.0', 'id': registration['id'], 'result': None})

    def test_stale_change_is_ignored(self):
        self.client.notify('textDocument/didChange', {'textDocument': {'uri': self.uri, 'version': 0}, 'contentChanges': [{'text': ''}]})
        self.expect_location(self.query('definition', 'Source readback', offset=9), self.uri, self.main, 'Alias readback', offset=6)

    def test_references_exclude_declaration(self):
        refs = self.query('references', 'Alias readback', offset=7, context={'includeDeclaration': False})
        self.assertEqual(len(refs), 1)
        self.assertEqual(refs[0]['range']['start'], position(self.main, 'Source readback', offset=7))

    def test_utf16_whole_document_edit(self):
        self.main = 'Module M{Attribute note="汉字😀";}'
        self.open(self.uri, self.main, 2)
        edits = self.result('textDocument/formatting', {'textDocument': {'uri': self.uri}, 'options': {'tabSize': 2}})
        self.assertEqual(edits[0]['range']['end'], {'line': 0, 'character': len(self.main.encode('utf-16-le')) // 2})

    def test_incomplete_member_completion(self):
        self.main = self.main[:self.main.index('Alias readback')] + 'Alias readback = child.'
        self.open(self.uri, self.main, 2)
        items = self.query('completion', 'child.', offset=6)
        self.assertEqual({i['label'] for i in items}, {'input', 'output', 'Width'})

    def test_disk_change_delete_and_close(self):
        path = self.root/'library.icl'
        path.write_text(self.lib.replace('output', 'new_output'))
        self.client.notify('workspace/didChangeWatchedFiles', {'changes': [{'uri': self.lib_uri, 'type': 2}]})
        self.assertEqual(self.query('definition', 'child.output', offset=8), [])
        path.unlink()
        self.client.notify('workspace/didChangeWatchedFiles', {'changes': [{'uri': self.lib_uri, 'type': 3}]})
        self.assertEqual(self.query('definition', 'Of Leaf', offset=4), [])
        self.client.notify('textDocument/didClose', {'textDocument': {'uri': self.uri}})
        self.assertEqual(self.result('textDocument/formatting', {'textDocument': {'uri': self.uri}}), [])


if __name__ == '__main__':
    ap = argparse.ArgumentParser()
    ap.add_argument('--binary', required=True)
    args, rest = ap.parse_known_args()
    BINARY = str(Path(args.binary).resolve())
    unittest.main(argv=[sys.argv[0]] + rest)
