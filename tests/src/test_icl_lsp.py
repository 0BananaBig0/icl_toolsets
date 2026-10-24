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
        self.assertEqual(self.open(self.uri, self.main, 2)['params']['diagnostics'], [])

    def test_port_group_references_with_comments(self):
        self.use_fixture('port_groups.icl')
        for name in ('Source', 'input'):
            for occurrence in (1, 2):
                with self.subTest(name=name, occurrence=occurrence):
                    self.expect_location(self.query('definition', name, occurrence),
                                         self.uri, self.main, 'DataInPort ' + name, offset=11)
            refs = self.query('references', name, context={'includeDeclaration': True})
            self.assertEqual(len(refs), 3, refs)

    def test_capture_enum_precedence_and_signal_fallback(self):
        self.use_fixture('capture_enum.icl')
        self.expect_location(self.query('definition', 'CaptureSource ON', offset=14),
                             self.uri, self.main, "ON = 1'b1", length=2)
        self.expect_location(self.query('definition', 'CaptureSource input', offset=14),
                             self.uri, self.main, 'DataInPort input', offset=11)
        for occurrence in (1, 2, 3):
            self.expect_location(self.query('definition', 'CaptureSource ON', occurrence, offset=14),
                                 self.uri, self.main, 'DataInPort ON', offset=11)
        self.expect_location(self.query('definition', 'child.output', offset=7),
                             self.lib_uri, self.lib, 'DataOutPort output', offset=12)
        self.assertEqual(len(self.query('references', "ON = 1'b1", context={'includeDeclaration': True})), 2)

    def test_enum_context_resolves_same_spelling(self):
        self.use_fixture('enums.icl')
        self.expect_location(self.query('definition', 'DefaultLoadValue ON', offset=18), self.uri, self.main, "ON = 1'b1", length=2)
        self.expect_location(self.query('definition', 'other == ON', offset=10), self.uri, self.main, "ON = 2'b11", length=2)

    def test_enum_comparison_operand_context(self):
        for version, expression in enumerate(('mode == (ON)', 'mode[$IDX] == ON',
                           '(mode[$IDX]) != ((ON))', '((mode[0])) == ON',
                           '(child.mode[$IDX]) == (ON)',
                           '~mode == ON', '!(mode[$IDX]) == (ON)',
                           'mode[$IDX:0] == /* right */ (ON)',
                           '(mode == (ON)) && (mode[$IDX] != ON)'), 2):
            with self.subTest(expression=expression):
                self.main = ("Module M { Parameter IDX = 0; "
                             "Enum E { OFF = 1'b0; ON = 1'b1; } "
                             "DataInPort mode[1:0] { RefEnum E; } DataInPort ON; "
                             "Instance child Of M; "
                             "LogicSignal enabled { " + expression + '; } }')
                self.assertEqual(self.open(self.uri, self.main, version)['params']['diagnostics'], [])
                for occurrence in range(expression.count('ON')):
                    self.expect_location(self.query('definition', 'ON', occurrence + 2),
                                         self.uri, self.main, "ON = 1'b1", length=2)
                if '$IDX' in expression:
                    self.expect_location(self.query('definition', '$IDX', offset=1),
                                         self.uri, self.main, 'Parameter IDX', offset=10, length=3)

    def test_each_hierarchical_path_component(self):
        self.use_fixture('hierarchy.icl')
        for offset, declaration, name_offset, length in (
                (0, 'Instance mid', 9, 3),
                (4, 'Instance leaf', 9, 4),
                (9, 'DataOutPort out', 12, 3)):
            with self.subTest(component=declaration):
                self.expect_location(self.query('definition', 'mid.leaf.out', offset=offset),
                                     self.uri, self.main, declaration, offset=name_offset, length=length)
        for offset, names in ((4, {'leaf'}), (9, {'Source', 'out'})):
            with self.subTest(completion=offset):
                self.assertEqual({i['label'] for i in self.query('completion', 'mid.leaf.out', offset=offset)}, names)

    def test_deep_alias_and_keyword_name(self):
        self.use_fixture('hierarchy.icl')
        self.expect_location(self.query('definition', 'mid.leaf.out', offset=10), self.uri, self.main, 'DataOutPort out', offset=12, length=3)
        self.expect_location(self.query('definition', 'Source Source', offset=8), self.uri, self.main, 'DataInPort Source', offset=11, length=6)

    def test_access_link_scan_interface_reference(self):
        self.use_fixture('interface.icl')
        self.expect_location(self.query('definition', 'child.access', offset=8), self.uri, self.main, 'ScanInterface access', offset=14, length=6)

    def test_review_parameter_in_numeric_context(self):
        self.use_fixture('review.icl')
        for occurrence in range(3):
            with self.subTest(occurrence=occurrence):
                self.expect_location(self.query('definition', '$V', occurrence, offset=1), self.uri, self.main, 'Parameter V', offset=10, length=1)

    def test_review_contextual_keyword_identifiers(self):
        self.use_fixture('review.icl')
        for use, declaration, offset in [
            ('Module;', 'DataInPort Module', 11),
            ('Of;', 'DataInPort Of', 11),
            ('SelectedBy;', 'DataInPort SelectedBy', 11),
        ]:
            with self.subTest(use=use):
                self.expect_location(self.query('definition', use, occurrence=1), self.uri, self.main, declaration, offset=offset)
        for name in ['Module', 'NameSpace', 'UseNameSpace']:
            with self.subTest(enum=name):
                self.expect_location(self.query('definition', 'DefaultLoadValue ' + name, offset=17), self.uri, self.main, name + ' =')
        self.expect_location(self.query('definition', '::Leaf', offset=3), self.uri, self.main, 'Module Leaf', offset=7)
        symbols = self.result('textDocument/documentSymbol', {'textDocument': {'uri': self.uri}})
        self.assertNotIn(';', {s['name'] for s in symbols})

    def test_review_broadcast_uses_instance_scope(self):
        self.use_fixture('review.icl')
        for name in ['iface', 'second']:
            with self.subTest(name=name):
                self.expect_location(self.query('definition', name + (';' if name == 'second' else ',')), self.uri, self.main, 'ScanInterface ' + name, offset=14)

    def test_review_active_signals_references(self):
        self.use_fixture('review.icl')
        for name, declaration, offset in [('reset', 'ResetPort reset', 10), ('Module', 'DataInPort Module', 11)]:
            with self.subTest(name=name):
                occurrence = 1 if name == 'reset' else 3
                self.expect_location(self.query('definition', name + ';', occurrence), self.uri, self.main, declaration, offset=offset)
                self.assertIsNotNone(self.query('hover', name + ';', occurrence))

    def test_keyword_signal_completion_is_not_module_context(self):
        self.use_fixture('completion_keywords.icl')
        for prefix in ('Source Of ', 'Source Instance, Of ', '{ Instance & Of '):
            with self.subTest(prefix=prefix):
                labels = {i['label'] for i in self.query('completion', prefix, offset=len(prefix))}
                self.assertIn('local_signal', labels)
                self.assertNotIn('GlobalTarget', labels)

    def test_keyword_namespace_completion(self):
        self.use_fixture('completion_keywords.icl')
        for needle, offset, expected in [('Of Of::KeywordTarget', 7, {'KeywordTarget'}),
                                          ('Of ::GlobalTarget', 5, {'Completion', 'GlobalTarget'})]:
            with self.subTest(needle=needle):
                self.assertEqual({i['label'] for i in self.query('completion', needle, offset=offset)},
                                 expected)
        end = self.main.index('Of Of::KeywordTarget') + 7
        self.main = self.main[:end]
        self.open(self.uri, self.main, 3)
        self.assertEqual({i['label'] for i in self.query('completion', 'Of Of::', offset=7)}, {'KeywordTarget'})

    def test_incomplete_namespace_outside_instance_keeps_scope(self):
        self.main = (CASES / 'incomplete_namespace.icl').read_text()
        self.open(self.uri, self.main, 2)
        labels = {i['label'] for i in self.query('completion', '::', offset=2)}
        self.assertIn('local_signal', labels)
        self.assertIn('Instance', labels)

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

    def test_unicode_cursor_uses_utf16(self):
        self.main = self.main.replace('Source readback', '/* é汉😀 */ Source readback')
        self.open(self.uri, self.main, 2)
        self.expect_location(self.query('definition', 'Source readback', offset=9), self.uri, self.main, 'Alias readback', offset=6, length=8)

    def test_percent_encoded_file_uri(self):
        old = self.root/'library.icl'
        renamed = self.root/'library space.icl'
        old.rename(renamed)
        self.client.notify('workspace/didChangeWatchedFiles', {'changes': [
            {'uri': self.lib_uri, 'type': 3}, {'uri': renamed.as_uri(), 'type': 1}]})
        self.expect_location(self.query('definition', 'Of Leaf', offset=4), renamed.as_uri(), self.lib, 'Module Leaf', offset=7, length=4)

    def test_missing_and_ambiguous_definitions_are_not_guessed(self):
        self.main = self.main.replace('Source readback', 'Source missing')
        self.open(self.uri, self.main, 2)
        self.assertEqual(self.query('definition', 'Source missing', offset=9), [])
        self.assertIsNone(self.query('hover', 'Source missing', offset=9))
        duplicate = self.root/'duplicate.icl'
        duplicate.write_text(self.lib)
        self.client.notify('workspace/didChangeWatchedFiles', {'changes': [{'uri': duplicate.as_uri(), 'type': 1}]})
        self.assertEqual(self.query('definition', 'Of Leaf', offset=4), [])

    def test_stale_change_is_ignored(self):
        self.client.notify('textDocument/didChange', {'textDocument': {'uri': self.uri, 'version': 0}, 'contentChanges': [{'text': ''}]})
        self.expect_location(self.query('definition', 'Source readback', offset=9), self.uri, self.main, 'Alias readback', offset=6)

    def test_top_level_completion_and_nonmodule_implementation(self):
        self.assertEqual(self.query('implementation', 'DataInPort input', offset=12), [])
        self.main = ''
        self.open(self.uri, self.main, 2)
        items = self.result('textDocument/completion', {'textDocument': {'uri': self.uri}, 'position': {'line': 0, 'character': 0}})
        self.assertIn('Module', {i['label'] for i in items})

    def test_enum_members_do_not_leak_from_other_enums(self):
        self.use_fixture('enums.icl')
        self.main = self.main.replace('Enum First', "Enum Third { MISSING = 1'b0; } Enum First").replace('DefaultLoadValue ON', 'DefaultLoadValue MISSING', 1)
        self.open(self.uri, self.main, 3)
        self.assertEqual(self.query('definition', 'DefaultLoadValue MISSING', offset=17), [])

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

    def test_incomplete_deep_alias_completion(self):
        self.use_fixture('hierarchy.icl')
        self.main = self.main[:self.main.index('Alias result')] + 'Alias result = mid.leaf.'
        self.open(self.uri, self.main, 3)
        items = self.query('completion', 'mid.leaf.', offset=9)
        self.assertIn('out', {i['label'] for i in items})

    def test_workspace_discovers_encoded_uri(self):
        path = self.root/'library space.icl'
        (self.root/'library.icl').rename(path)
        self.client.stop()
        self.client = Client(BINARY)
        self.result('initialize', {'rootUri': self.root.as_uri()})
        self.open(self.uri, self.main)
        self.expect_location(self.query('definition', 'Of Leaf', offset=4), path.as_uri(), self.lib, 'Module Leaf', offset=7)

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
