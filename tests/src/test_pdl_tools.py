#!/usr/bin/env python3
"""PDL executable boundaries: no evaluation, shared formatting and real LSP."""
import argparse
import json
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from lsp_test import Client

FORMATTER = None
SERVER = None
ROOT = Path(__file__).resolve().parents[2]
HEADER = 'iPDLLevel 1 -version STD_1687_2014\niProcsForModule Demo\n'
REVIEW_PROBES = json.loads((ROOT / 'tests/cases/ours/pdl/review_probes.json').read_text())


class Formatter(unittest.TestCase):
    def run_format(self, text, *args):
        return subprocess.run([FORMATTER, *args], input=text, text=True,
                              capture_output=True, timeout=5)

    def test_nested_scripts_are_formatted_but_data_is_preserved(self):
        source = HEADER + 'iProc run {} {\nset data {a\n  b}\nif {1} {\niWrite reg[7:0] 0xA5; iApply\n}\n}\n'
        run = self.run_format(source)
        self.assertEqual(run.returncode, 0, run.stderr)
        self.assertIn('  set data {a\n  b}', run.stdout)
        self.assertIn('  if {1} {\n    iWrite reg[7:0] 0xA5\n    iApply\n  }', run.stdout)
        self.assertEqual(self.run_format(run.stdout).stdout, run.stdout)

    def test_standard_and_constructed_fixtures_are_stable(self):
        for directory in ('standard/pdl', 'ours/pdl'):
            for path in (ROOT / 'tests/cases' / directory).glob('*.pdl'):
                with self.subTest(path=path):
                    run = self.run_format(path.read_text())
                    self.assertEqual(run.returncode, 0, run.stderr)
                    self.assertEqual(self.run_format(run.stdout).stdout, run.stdout)

    def test_no_source_or_substitution_is_executed(self):
        with tempfile.TemporaryDirectory() as directory:
            sentinel = Path(directory) / 'must-not-exist'
            source = HEADER + f'set result [exec touch {{{sentinel}}}]\n'
            run = self.run_format(source)
            self.assertEqual(run.returncode, 0, run.stderr)
            self.assertFalse(sentinel.exists())
            self.assertIn('[exec touch', run.stdout)

    def test_level_zero_rejects_tcl_and_level_one_commands(self):
        for body in ('set x 1', 'if {1} {iApply}', 'iGetStatus'):
            with self.subTest(body=body):
                source = HEADER.replace('Level 1', 'Level 0') + f'iProc run {{}} {{\n{body}\n}}\n'
                run = self.run_format(source)
                self.assertEqual(run.returncode, 2)
                self.assertEqual(run.stdout, '')
                self.assertIn('level-0', run.stderr)

    def test_invalid_nested_scripts_and_pdl_arguments(self):
        for body in ('iWrite reg', 'iApply -unknown', 'iPDLLevel 3',
                     'iProc run {} {', 'iProc run {} {set x "a"b}',
                     'if {1} {set x "a"b}', 'set x [iWrite reg]'):
            with self.subTest(body=body):
                run = self.run_format(HEADER + body + '\n')
                self.assertEqual(run.returncode, 2, run.stdout)
                self.assertEqual(run.stdout, '')

    def test_expression_and_switch_bodies_are_checked(self):
        for body in ('if {$value +} {iApply}',
                     'set x [expr {1 +}]',
                     'switch -- $value {0 {iWrite reg} default {iApply}}',
                     'switch -- $value 0 {iWrite reg} default {iApply}'):
            with self.subTest(body=body):
                self.assertEqual(self.run_format(HEADER + body + '\n').returncode, 2)

    def test_validates_options_and_default_argument_order(self):
        for body in ('iProcsForModule Demo -bad value',
                     'iProc run {{x 1} y} {iApply}',
                     'iScan access 8 -si', 'iClockOverride clk -bad 2',
                     'iRunLoop 5 -bad', 'iState reg 1 -bad',
                     'iGetReadData reg -bad'):
            with self.subTest(body=body):
                self.assertEqual(self.run_format(HEADER + body + '\n').returncode, 2)

    def test_pdl_numbers_are_distinct_from_icl_literals(self):
        for value in ('123', '0xAa', '0Xxx', '0b10xX', '0B01', 'Ready', '$value'):
            with self.subTest(value=value):
                self.assertEqual(self.run_format(HEADER + f'iWrite reg {value}\n').returncode, 0)
        for value in ("4'hF", '0b2', '0xG', '1_000', '-1', '0x', '{}'):
            with self.subTest(value=value):
                run = self.run_format(HEADER + f'iWrite reg {value}\n')
                self.assertEqual(run.returncode, 2, run.stdout)
                self.assertIn('PDL value', run.stderr)
        for command in ('iState reg 0xG', 'iRunLoop -1', 'iScan si 8 -si 0b2'):
            self.assertEqual(self.run_format(HEADER + command + '\n').returncode, 2)
        self.assertEqual(self.run_format(HEADER + 'iScan si 8 -si { 0b0011\n  xx01 }\n').returncode, 0)

    def test_comments_strings_continuations_and_unknown_commands(self):
        source = HEADER + '# top\niProc run {} {\n# nested\nset s "a; # literal\\ntext"\ncustom {opaque; data} \\\n  more\niWrite reg(7:0) 0xA5; # trailing\niApply\n}\n'
        run = self.run_format(source)
        self.assertEqual(run.returncode, 0, run.stderr)
        for text in ('# top', '# nested', '"a; # literal\\ntext"',
                     '{opaque; data}', '# trailing', 'reg(7:0)'):
            self.assertIn(text, run.stdout)
        self.assertEqual(self.run_format(run.stdout).stdout, run.stdout)

    def test_tcl_control_arguments_keep_their_meaning(self):
        scripts = (
            'catch {error boom} finally {opts}\nputs [info exists opts]\n',
            'catch {error boom} finally {iWrite reg}\nputs [info exists {iWrite reg}]\n',
            'switch -regexp -matchvar matches abc {abc} {puts yes}\n',
            'switch -regexp -indexvar indices abc {abc} {puts yes}\n',
            'switch -- x {x - y {puts yes} default {puts no}}\n',
            'try {error boom} on error {message options} {puts handled} finally {puts done}\n',
            'proc p {x x} {return $x}\nputs [p 1 2]\n',
            'if {0} {elseif} {puts ok}\n',
            'puts {*}{stdout hello}\n',
            'puts {*}{stdout {hello world}}\n',
            'puts {*}{stdout "hello world"}\n',
            'puts {*}{stdout hello\\ world}\n',
            'puts hello {*}{ }\n',
            '{*}{}\nputs ok\n',
            '{*}{puts hello}\n',
        )
        for script in scripts:
            with self.subTest(script=script):
                formatted = self.run_format(script)
                self.assertEqual(formatted.returncode, 0, formatted.stderr)
                original = subprocess.run(['tclsh'], input=script, text=True, capture_output=True, timeout=5)
                result = subprocess.run(['tclsh'], input=formatted.stdout, text=True, capture_output=True, timeout=5)
                self.assertEqual((result.returncode, result.stdout, result.stderr),
                                 (original.returncode, original.stdout, original.stderr))

    def test_scan_continuations(self):
        scan = HEADER + 'iScan si 8 -si {0b0000 ' + chr(92) + '\n0000}\n'
        run = self.run_format(scan)
        self.assertEqual(run.returncode, 0, run.stderr)
        self.assertIn(chr(92) + '\n0000', run.stdout)

    def test_retained_review_tcl_equivalence(self):
        for text in REVIEW_PROBES['equivalence']:
            with self.subTest(text=text):
                formatted = self.run_format(text)
                self.assertEqual(formatted.returncode, 0, formatted.stderr)
                self.assertEqual(self.run_format(formatted.stdout).stdout, formatted.stdout)
                before = subprocess.run(['tclsh'], input=text, text=True,
                                        capture_output=True, timeout=5)
                after = subprocess.run(['tclsh'], input=formatted.stdout, text=True,
                                       capture_output=True, timeout=5)
                self.assertEqual((after.returncode, after.stdout, after.stderr),
                                 (before.returncode, before.stdout, before.stderr))

    def test_level_zero_rejects_command_substitution(self):
        source = HEADER.replace('Level 1', 'Level 0') + 'iProc run {} {iWrite reg [iCall helper]}\n'
        run = self.run_format(source)
        self.assertEqual(run.returncode, 2, run.stdout)
        self.assertIn('level-0', run.stderr)

    def test_vendor_note_and_slm_clock_option_order(self):
        for command in ('iNote "TESSENT_PRAGMA marker"', 'iRunLoop -tck 4',
                        'iNote', 'iNote -bad "message"'):
            with self.subTest(command=command):
                run = self.run_format(HEADER + 'iProc run {} {' + command + '}\n')
                self.assertEqual(run.returncode, 0, run.stderr)
                self.assertIn(command, run.stdout)
        for command in ('iRunLoop -tck', 'iRunLoop -tck -1'):
            self.assertEqual(self.run_format(HEADER + command + '\n').returncode, 2)

    def test_tessent_apply_and_persistent_take(self):
        for command in ('iApply -end_in_pause', 'iTake', 'iTake reg -persistent',
                        'iTake -persistent', 'iTake reg',
                        'iWrite reg 1 -ignore_when_value_already_present on',
                        'iWrite reg 1 -ignore_when_value_already_present off',
                        'iWrite reg 1 -ignore_when_value_already_present auto'):
            with self.subTest(command=command):
                run = self.run_format(HEADER + command + '\n')
                self.assertEqual(run.returncode, 0, run.stderr)
                self.assertIn(command, run.stdout)
        for command in ('iApply -bad', 'iTake reg -bad', 'iTake reg extra',
                        'iTake reg -persistent extra', 'iTake -bad',
                        'iWrite reg 1 -ignore_when_value_already_present',
                        'iWrite reg 1 -ignore_when_value_already_present bad',
                        'iWrite reg 1 -bad on'):
            with self.subTest(command=command):
                self.assertEqual(self.run_format(HEADER + command + '\n').returncode, 2)

    def test_dynamic_module_binding_is_valid_but_missing_binding_is_not(self):
        source = ('foreach module {A B} {\niProcsForModule $module\n'
                  'iProc run {} {iApply}\n}\n')
        run = self.run_format(source)
        self.assertEqual(run.returncode, 0, run.stderr)
        self.assertIn('iProcsForModule $module', run.stdout)
        for source in ('iProc orphan {} {iApply}\n', 'iProcsForModule {}\niProc orphan {} {iApply}\n'):
            self.assertEqual(self.run_format(source).returncode, 2)

    def test_cli_modes_and_invalid_input_preservation(self):
        source = HEADER + 'iProc run {} { iApply }\n'
        formatted = self.run_format(source).stdout
        self.assertEqual(self.run_format(source, '--check').returncode, 1)
        self.assertEqual(self.run_format(formatted, '--check').returncode, 0)
        self.assertIn('    iApply', self.run_format(source, '--indent', '4').stdout)
        for flag in ('--help', '--version'):
            run = self.run_format('', flag)
            self.assertEqual(run.returncode, 0)
            self.assertIn('pdl-format', run.stdout)
        self.assertIn('iProc', self.run_format(source, '--ast').stdout)
        self.assertEqual(self.run_format(source, '--unknown').returncode, 2)
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / 'example.pdl'
            path.write_text(source)
            run = self.run_format('', '-w', str(path))
            self.assertEqual(run.returncode, 0, run.stderr)
            self.assertEqual(path.read_text(), formatted)
            stamp = path.stat().st_mtime_ns
            self.assertEqual(self.run_format('', '-w', str(path)).returncode, 0)
            self.assertEqual(path.stat().st_mtime_ns, stamp)
            path.write_text(HEADER + 'iProc broken {} {')
            self.assertEqual(self.run_format('', '-w', str(path)).returncode, 2)
            self.assertTrue(path.read_text().endswith('{'))


class LanguageServer(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.addCleanup(self.tmp.cleanup)
        self.root = Path(self.tmp.name)
        self.library = self.root / 'library.pdl'
        self.library.write_text(HEADER + 'iProc helper {value} {\niWrite reg $value\niApply\n}\n')
        self.uri = (self.root / 'main.pdl').as_uri()
        self.client = Client(SERVER)
        self.addCleanup(self.client.stop)
        self.capabilities = self.request('initialize', {'rootUri': self.root.as_uri()})['capabilities']

    def request(self, method, params):
        reply = self.client.request(method, params)
        self.assertIsNotNone(reply, method)
        self.assertNotIn('error', reply, reply)
        return reply['result']

    def open(self, text, version=1):
        self.client.notify('textDocument/didOpen', {'textDocument': dict(
            uri=self.uri, languageId='pdl', version=version, text=text)})
        result = self.client.wait(lambda m: m.get('method') == 'textDocument/publishDiagnostics'
                                  and m['params']['uri'] == self.uri, 'diagnostics')
        return result['params']

    def query(self, method, line, character, **extra):
        return self.request('textDocument/' + method, dict(
            textDocument={'uri': self.uri}, position=dict(line=line, character=character), **extra))

    def test_diagnostics_correction_and_shared_formatter(self):
        text = HEADER + 'iProc run {} {\n  iWrite reg\n}\n'
        result = self.open(text)
        self.assertEqual(result['version'], 1)
        self.assertEqual(result['diagnostics'][0]['source'], 'pdl-lsp')
        self.assertEqual(result['diagnostics'][0]['range']['start']['line'], 3)
        params = dict(textDocument={'uri': self.uri}, options={'tabSize': 4, 'insertSpaces': True})
        self.assertEqual(self.request('textDocument/formatting', params), [])
        text = text.replace('iWrite reg', 'iWrite reg 0xA5')
        self.assertEqual(self.open(text, 2)['diagnostics'], [])
        edits = self.request('textDocument/formatting', params)
        expected = subprocess.run([FORMATTER, '--indent', '4'], input=text,
                                  text=True, capture_output=True, check=True).stdout
        self.assertEqual(edits[0]['newText'], expected)

    def test_procedure_and_variable_navigation_completion_references(self):
        text = HEADER + 'iProc run {value} {\niCall helper $value\n}\n'
        self.assertEqual(self.open(text)['diagnostics'], [])
        for method in ('definition', 'declaration', 'implementation'):
            locations = self.query(method, 3, 8)
            self.assertEqual(locations[0]['uri'], self.library.as_uri())
            self.assertEqual(locations[0]['range']['start']['line'], 2)
        variable = self.query('definition', 3, 17)
        self.assertEqual(variable[0]['uri'], self.uri)
        self.assertEqual(variable[0]['range']['start']['line'], 2)
        labels = {item['label'] for item in self.query('completion', 3, 5)}
        self.assertTrue({'iWrite', 'iGetStatus', 'helper', 'run'} <= labels, labels)
        self.assertNotIn('ScanRegister', labels)
        refs = self.query('references', 3, 8, context={'includeDeclaration': True})
        self.assertEqual(len(refs), 2)
        self.assertIn('helper', str(self.query('hover', 3, 8)))
        symbols = self.request('textDocument/documentSymbol', {'textDocument': {'uri': self.uri}})
        self.assertIn('run', {s['name'] for s in symbols})

    def test_icl_binding_and_prefixed_object_navigation(self):
        icl = self.root / 'model.icl'
        icl.write_text('Module Leaf { DataInPort reg[7:0]; }\n'
                       'Module Demo { Instance child Of Leaf; }\n')
        self.client.notify('workspace/didChangeWatchedFiles', {
            'changes': [{'uri': icl.as_uri(), 'type': 1}]})
        text = HEADER + 'iProc run {} {\niPrefix child\niWrite reg[7:0] 0xA5\niApply\n}\n'
        self.assertEqual(self.open(text)['diagnostics'], [])
        target = self.query('definition', 4, 8)
        self.assertEqual(target[0]['uri'], icl.as_uri())
        self.assertEqual(target[0]['range']['start']['line'], 0)

    def test_namespace_binding_ambiguity_unicode_and_incomplete_completion(self):
        other = self.root / 'other.pdl'
        other.write_text(HEADER.replace('Module Demo', 'Module Other') +
                         'iProc helper {} {iApply}\n')
        self.client.notify('workspace/didChangeWatchedFiles', {
            'changes': [{'uri': other.as_uri(), 'type': 1}]})
        text = HEADER + '# 汉字😀\niProc run {} {\niCall helper 1\n}\n'
        self.assertEqual(self.open(text)['diagnostics'], [])
        self.assertEqual(self.query('definition', 4, 8)[0]['uri'], self.library.as_uri())
        self.open(HEADER + 'iProc run {value} {\niCall ', 2)
        labels = {item['label'] for item in self.query('completion', 3, 6)}
        self.assertIn('value', labels)
        self.assertIn('helper', labels)

    def test_qualified_tcl_procedures_and_nested_namespaces(self):
        text = HEADER + ('namespace eval outer {\n'
                         'namespace eval inner {\n'
                         'proc helper {} {iApply}\n'
                         'proc run {} {helper; ::outer::inner::helper}\n'
                         '}\n}\n'
                         'proc ::global_helper {} {iApply}\n'
                         'outer::inner::helper\n::global_helper\n')
        self.assertEqual(self.open(text)['diagnostics'], [])
        for line, column, target_line in ((5, 15, 4), (5, 30, 4), (9, 10, 4), (10, 5, 8)):
            with self.subTest(line=line):
                target = self.query('definition', line, column)
                self.assertTrue(target)
                self.assertEqual(target[0]['range']['start']['line'], target_line)

    def test_pdl_call_namespace_is_separate_from_definition_namespace(self):
        text = HEADER + ('iProc target {} {iApply}\n'
                         'iProcsForModule Demo -iProcNameSpace vendor\n'
                         'iProc target {} {iApply}\n'
                         'iProc caller {} {\n'
                         'iCall target\n'
                         'iUseProcNameSpace vendor\n'
                         'iCall target\n'
                         'iUseProcNameSpace -\n'
                         'iCall target\n}\n')
        self.assertEqual(self.open(text)['diagnostics'], [])
        for line, target_line in ((6, 2), (8, 4), (10, 2)):
            with self.subTest(line=line):
                target = self.query('definition', line, 8)
                self.assertTrue(target)
                self.assertEqual(target[0]['range']['start']['line'], target_line)

    def test_tcl_namespace_variables_do_not_collide(self):
        text = ('namespace eval a {\nset value 1\nputs $value\n}\n'
                'namespace eval b {\nset value 2\nputs $value\n}\n')
        self.assertEqual(self.open(text)['diagnostics'], [])
        for line, target_line in ((2, 1), (6, 5)):
            with self.subTest(line=line):
                target = self.query('definition', line, 8)
                self.assertTrue(target)
                self.assertEqual(target[0]['range']['start']['line'], target_line)

    def test_dynamic_bindings_do_not_invent_procedure_targets(self):
        text = ('foreach module {A B} {\niProcsForModule $module\n'
                'iProc target {} {iApply}\niProc caller {} {\niCall target\n}\n}\n')
        self.assertEqual(self.open(text)['diagnostics'], [])
        self.assertEqual(self.query('definition', 4, 8), [])

    def test_dynamic_namespace_and_prefix_do_not_invent_targets(self):
        for version, binding in enumerate((
                'iProcsForModule Demo -iProcNameSpace $ns',
                'iProcsForModule Demo $option ns'), start=1):
            with self.subTest(binding=binding):
                source = (binding + '\n'
                          'iProc target {} {iApply}\n'
                          'iProc caller {} {\niCall target\n}\n')
                self.assertEqual(self.open(source, version)['diagnostics'], [])
                self.assertEqual(self.query('definition', 3, 8), [])
        for version, (command, reset) in enumerate((
                ('iPrefix $path', 'iPrefix -'),
                ('iUseProcNameSpace $ns', 'iUseProcNameSpace -'),
                ('iPrefix {*}$args', 'iPrefix -'),
                ('iUseProcNameSpace {*}$args', 'iUseProcNameSpace -')), start=3):
            with self.subTest(command=command):
                source = HEADER + ('iProc target {} {iApply}\n'
                                   'iProc caller {} {\n' + command + '\n'
                                   'iCall target\n' + reset + '\niCall target\n}\n')
                self.assertEqual(self.open(source, version)['diagnostics'], [])
                self.assertEqual(self.query('definition', 5, 8), [])
                target = self.query('definition', 7, 8)
                self.assertTrue(target)
                self.assertEqual(target[0]['range']['start']['line'], 2)

    def test_help_and_version_do_not_wait_for_protocol_input(self):
        for option in ('-h', '--help', '--version', '--unknown'):
            with self.subTest(option=option), subprocess.Popen(
                    [SERVER, option], stdin=subprocess.PIPE, stdout=subprocess.PIPE,
                    stderr=subprocess.PIPE) as process:
                try:
                    process.wait(timeout=2)
                except subprocess.TimeoutExpired:
                    process.kill()
                    process.communicate()
                    self.fail('command-line option waited for stdin')
                stdout, stderr = process.communicate()
                self.assertEqual(process.returncode, 2 if option == '--unknown' else 0)
                self.assertIn(b'pdl-lsp', stderr if option == '--unknown' else stdout)

    def test_hierarchical_qualified_calls_and_unresolved_paths(self):
        icl = self.root / 'hierarchy.icl'
        icl.write_text('Module Leaf { DataInPort reg; }\n'
                       'Module Demo { Instance child Of Leaf; DataInPort plain; }\n')
        procedures = self.root / 'leaf.pdl'
        procedures.write_text('iProcsForModule Leaf -iProcNameSpace vendor\n'
                              'iProc action {} {iApply}\n')
        self.client.notify('workspace/didChangeWatchedFiles', {'changes': [
            {'uri': path.as_uri(), 'type': 1} for path in (icl, procedures)]})
        text = HEADER + ('iProc run {} {\n'
                         'iCall child.vendor::action\n'
                         'iCall missing.vendor::action\n'
                         'iCall plain.vendor::action\n'
                         'iCall child.vendor::absent\n'
                         'iRead plain.invalid\n}\n')
        self.assertEqual(self.open(text)['diagnostics'], [])
        target = self.query('definition', 3, 15)
        self.assertEqual(target[0]['uri'], procedures.as_uri())
        self.assertEqual(target[0]['range']['start']['line'], 1)
        for line in (4, 5, 6, 7):
            self.assertEqual(self.query('definition', line, 10), [])

    def test_tcl_fallback_and_missing_symbols(self):
        text = ('proc global {} {return ok}\nnamespace eval ns {\n'
                'proc caller {} {\nglobal\nmissing\nputs $absent\n}\n}\n# comment\n')
        self.assertEqual(self.open(text)['diagnostics'], [])
        target = self.query('definition', 3, 2)
        self.assertEqual(target[0]['range']['start']['line'], 0)
        for line, column in ((4, 2), (5, 8), (8, 4)):
            self.assertEqual(self.query('definition', line, column), [])

    def test_duplicate_tcl_formals_resolve_to_first_binding(self):
        self.assertEqual(self.open('proc p {x x} {return $x}\n')['diagnostics'], [])
        target = self.query('definition', 0, 22)
        self.assertEqual(target[0]['range']['start'], {'line': 0, 'character': 8})

    def test_formal_navigation_does_not_select_previous_default(self):
        for version, keyword in enumerate(('proc', 'iProc'), start=1):
            with self.subTest(keyword=keyword):
                text = HEADER + keyword + ' p {{first value} {value 1}} {return $value}\n'
                if keyword == 'iProc':
                    text = text.replace('return $value', 'iWrite reg $value')
                self.assertEqual(self.open(text, version)['diagnostics'], [])
                line = text.splitlines()[2]
                target = self.query('definition', 2, line.rfind('$value') + 1)
                self.assertEqual(target[0]['range']['start'],
                                 {'line': 2, 'character': line.index('{value') + 1})

    def test_nested_formal_name_ranges_and_escaped_names(self):
        for version, formal in enumerate(('{{value} 1}', '{"value" 1}',
                                         r'{v\x61lue value}'), start=1):
            text = 'proc p {' + formal + '} {return $value}\n'
            self.assertEqual(self.open(text, version)['diagnostics'], [])
            target = self.query('definition', 0, text.rfind('$value') + 1)
            if '\\' in formal:
                self.assertEqual(target, [])
            else:
                self.assertEqual(target[0]['range']['start'],
                                 {'line': 0, 'character': text.index('value')})

    def test_retained_review_diagnostics(self):
        for version, case in enumerate(REVIEW_PROBES['diagnostics'], start=1):
            with self.subTest(text=case['text']):
                diagnostics = self.open(case['text'], version)['diagnostics']
                self.assertEqual(not diagnostics, case['valid'])
                formatted = subprocess.run([FORMATTER], input=case['text'].encode(),
                                           capture_output=True, timeout=5)
                self.assertEqual(formatted.returncode, 0 if case['valid'] else 2)

    def test_retained_review_navigation(self):
        for version, case in enumerate(REVIEW_PROBES['navigation'], start=1):
            with self.subTest(text=case['text']):
                self.assertEqual(self.open(case['text'], version)['diagnostics'], [])
                target = self.query('definition', **case['query'])
                self.assertEqual(target[0]['range']['start'], case['target'])


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--formatter', default='build/pdl-format')
    parser.add_argument('--server', default='build/pdl-lsp')
    args, rest = parser.parse_known_args()
    FORMATTER = str(Path(args.formatter).resolve())
    SERVER = str(Path(args.server).resolve())
    unittest.main(argv=[sys.argv[0], *rest])
