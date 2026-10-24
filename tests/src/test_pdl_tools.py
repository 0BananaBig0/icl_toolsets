#!/usr/bin/env python3
"""PDL executable boundaries: no evaluation, shared formatting and real LSP."""
import argparse
import json
import resource
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


def parser_limits():
    resource.setrlimit(resource.RLIMIT_CORE, (0, 0))
    resource.setrlimit(resource.RLIMIT_STACK, (8 * 1024 * 1024,) * 2)
    resource.setrlimit(resource.RLIMIT_AS, (512 * 1024 * 1024,) * 2)
    resource.setrlimit(resource.RLIMIT_CPU, (5, 5))


class Formatter(unittest.TestCase):
    def run_format(self, text, *args):
        return subprocess.run([FORMATTER, *args], input=text, text=True,
                              capture_output=True, timeout=5)

    def test_deep_substitutions_fail_safely_in_cli_and_lsp(self):
        deep = '[' * 30000 + 'list' + ']' * 30000
        for source in ('set x ' + deep + '\n',
                       'set x "' + deep + '"\n',
                       'proc p {} {set x ' + deep + '}\n',
                       'expr {' + deep + '}\n',
                       'set x ' + '$a(' * 30000 + 'x' + ')' * 30000 + '\n'):
            with self.subTest(prefix=source[:20]):
                run = subprocess.run([FORMATTER], input=source, text=True,
                                     capture_output=True, timeout=10, preexec_fn=parser_limits)
                self.assertEqual(run.returncode, 2, run.stderr)
                self.assertIn('nesting', run.stderr)
                messages = [
                    dict(jsonrpc='2.0', id=1, method='initialize', params={}),
                    dict(jsonrpc='2.0', method='textDocument/didOpen', params={
                        'textDocument': dict(uri='file:///tmp/deep.pdl',
                                             languageId='pdl', version=1, text=source)}),
                    dict(jsonrpc='2.0', id=2, method='shutdown', params={}),
                    dict(jsonrpc='2.0', method='exit', params={}),
                ]
                wire = b''
                for message in messages:
                    body = json.dumps(message).encode()
                    wire += b'Content-Length: %d\r\n\r\n' % len(body) + body
                run = subprocess.run([SERVER], input=wire, capture_output=True,
                                     timeout=10, preexec_fn=parser_limits)
                self.assertEqual(run.returncode, 0, run.stderr)
                self.assertIn(b'nesting', run.stdout)
                self.assertIn(b'publishDiagnostics', run.stdout)

    def test_substitution_depth_boundary(self):
        for depth in (128, 129):
            nested = '[' * depth + 'list' + ']' * depth
            for source in ('set x ' + nested + '\n',
                           'set x "' + nested + '"\n',
                           'expr {' + nested + '}\n',
                           'set x ' + '$a(' * depth + 'x' + ')' * depth + '\n'):
                with self.subTest(depth=depth, prefix=source[:20]):
                    run = self.run_format(source)
                    self.assertEqual(run.returncode, 0 if depth == 128 else 2,
                                     run.stderr)
                    if depth == 129:
                        self.assertIn('nesting', run.stderr)

    def test_iterative_expression_operators_remain_supported(self):
        for expression in ('(' * 30000 + '1' + ')' * 30000,
                           '!' * 30000 + '1'):
            run = subprocess.run([FORMATTER], input='expr {' + expression + '}\n',
                                 text=True, capture_output=True, timeout=10,
                                 preexec_fn=parser_limits)
            self.assertEqual(run.returncode, 0, run.stderr)

    def test_literal_brackets_do_not_consume_substitution_depth(self):
        brackets = '[' * 30000 + ']' * 30000
        for source in ('set x {' + brackets + '}\n',
                       '# ' + brackets + '\nset x 1\n',
                       'set x "' + brackets.replace('[', r'\[') + '"\n',
                       'set x ${' + brackets + '}\n',
                       'set x [list {' + brackets + '}]\n',
                       'set x "[list {' + brackets + '}]"\n',
                       'set x [# ' + brackets + '\nlist ok]\n',
                       'list {*}{' + brackets + '}\n',
                       'list \\\n{' + brackets + '}\n'):
            with self.subTest(prefix=source[:20]):
                run = self.run_format(source)
                self.assertEqual(run.returncode, 0, run.stderr)

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
    def test_equivalent_tcl_namespace_spellings_resolve(self):
        source = ('namespace eval n {\nvariable value 1\nproc target {} {}\n'
                  'proc a:b {} {}\n}\n'
                  '::n::::target\n:::n::target\n'
                  'puts $::n::::value\n::n::::a:b\n'
                  'namespace eval ::::n {target; puts $value}\n')
        self.assertEqual(self.open(source)['diagnostics'], [])
        for line, word, target in ((5, 'target', 2), (6, 'target', 2),
                                   (7, 'value', 1), (8, 'a:b', 3),
                                   (9, 'target', 2), (9, 'value', 1)):
            with self.subTest(line=line, word=word):
                column = source.splitlines()[line].index(word)
                result = self.query('definition', line, column)
                self.assertEqual(len(result), 1)
                self.assertEqual(result[0]['range']['start']['line'], target)
        references = self.query('references', 2, 6, context={'includeDeclaration': True})
        self.assertEqual(sorted(r['range']['start']['line'] for r in references), [2, 5, 6, 9])
        # This normalization is Tcl-specific; PDL namespace grammar stays strict.
        self.assertTrue(self.open('iPDLLevel 0\niProcsForModule n::::M\n', 2)['diagnostics'])

    def test_literal_control_expressions_are_checked_in_cli_and_lsp(self):
        version = 0
        for expression in ('{1 +}', '"1 +"', '1+', '"1 +\\\n  "'):
            for command in ('if %s {}', 'while %s {}', 'for {} %s {} {}',
                            '::if %s {}', 'if 0 {} elseif %s {}'):
                source = command % expression + '\n'
                version += 1
                with self.subTest(source=source):
                    run = subprocess.run([FORMATTER], input=source, text=True,
                                         capture_output=True, timeout=5)
                    self.assertEqual(run.returncode, 2, run.stderr)
                    self.assertIn('invalid Tcl expression', run.stderr)
                    diagnostics = self.open(source, version)['diagnostics']
                    self.assertEqual(len(diagnostics), 1)
                    self.assertIn('invalid Tcl expression', diagnostics[0]['message'])
        for expression in ('{1 + 2}', '"1 + 2"', '1+2', '$condition',
                           '"$lhs + $rhs"', '"1 +\\\n  2"'):
            source = 'if ' + expression + ' {iApply}\n'
            version += 1
            with self.subTest(source=source):
                run = subprocess.run([FORMATTER], input=source, text=True,
                                     capture_output=True, timeout=5)
                self.assertEqual(run.returncode, 0, run.stderr)
                self.assertEqual(self.open(source, version)['diagnostics'], [])
        for expression in ('1[0]', '"1[0]"'):
            for command in ('if %s {}', 'while %s {break}',
                            'for {} %s {} {break}', 'expr %s', 'expr %s + 2'):
                source = 'proc 0 {} {return 1}\n' + command % expression + '\n'
                version += 1
                with self.subTest(source=source):
                    run = subprocess.run([FORMATTER], input=source, text=True,
                                         capture_output=True, timeout=5)
                    self.assertEqual(run.returncode, 0, run.stderr)
                    self.assertEqual(self.open(source, version)['diagnostics'], [])

    def test_enum_members_do_not_shadow_icl_object_navigation(self):
        model = self.root / 'enum_objects.icl'
        model.write_text("Module Leaf {\n Enum E { status = 1'b0; only_enum = 1'b1; }\n"
                         ' ScanInPort si;\n'
                         ' ScanRegister status[0] { ScanInSource si; }\n}\n'
                         "Module Top { Enum E { child = 1'b0; } Instance child Of Leaf; }\n")
        self.client.notify('workspace/didChangeWatchedFiles', {'changes': [
            dict(uri=model.as_uri(), type=1)]})
        source = ('iProcsForModule Leaf\niRead status\niRead only_enum\n'
                  'iProcsForModule Top\niRead child.status\n')
        self.assertEqual(self.open(source)['diagnostics'], [])
        for line, column in ((1, 7), (4, 16)):
            with self.subTest(line=line):
                result = self.query('definition', line, column)
                self.assertEqual(len(result), 1)
                self.assertEqual(result[0]['uri'], model.as_uri())
                self.assertEqual(result[0]['range']['start']['line'], 3)
        self.assertEqual(self.query('definition', 2, 9), [])
        model.write_text('Module Leaf { DataInPort status; DataInPort status; }\n')
        self.client.notify('workspace/didChangeWatchedFiles', {'changes': [
            dict(uri=model.as_uri(), type=2)]})
        self.assertEqual(self.query('definition', 1, 7), [])

    def test_global_tcl_builtins_check_nested_syntax(self):
        cases = ('::if {1 +} {}', '::while {1 +} {}',
                 '::for {} {1 +} {} {}', '::expr {1 +}', '::expr 1 +',
                 '::proc p {} {iWrite reg}', '::::proc p {} {::if {1 +} {}}',
                 '::namespace eval n {::proc p {} {iWrite reg}}',
                 '::catch {iWrite reg}', '::try {iWrite reg}',
                 '::foreach x {1} {iWrite reg}', '::lmap x {1} {iWrite reg}',
                 '::switch x {x {iWrite reg}}', '::set',
                 'iPDLLevel 0\n::iWrite reg 1', 'iPDLLevel 0\n::proc p {} {}')
        for version, source in enumerate(cases, 1):
            with self.subTest(source=source):
                run = subprocess.run([FORMATTER], input=source + '\n', text=True,
                                     capture_output=True, timeout=5)
                self.assertEqual(run.returncode, 2, run.stderr)
                self.assertTrue(self.open(source + '\n', version)['diagnostics'])

    def test_global_tcl_spelling_and_navigation_are_preserved(self):
        source = ('::namespace eval n {\n::proc helper {arg} {\n'
                  '::set local $arg\nreturn $local\n}\n}\n'
                  '::n::helper 1\n'
                  '::proc ::custom::if {x y} {}\n::custom::if {1 +} {}\n')
        self.assertEqual(self.open(source)['diagnostics'], [])
        self.assertEqual(self.query('definition', 6, 7)[0]['range']['start']['line'], 1)
        self.assertEqual(self.query('definition', 3, 9)[0]['range']['start']['line'], 2)
        self.assertEqual(self.query('definition', 8, 11)[0]['range']['start']['line'], 7)
        first = subprocess.run([FORMATTER], input=source, text=True,
                               capture_output=True, timeout=5)
        self.assertEqual(first.returncode, 0, first.stderr)
        self.assertIn('::proc helper', first.stdout)
        self.assertIn('::custom::if {1 +} {}', first.stdout)
        second = subprocess.run([FORMATTER], input=first.stdout, text=True,
                                capture_output=True, timeout=5)
        self.assertEqual(second.returncode, 0, second.stderr)
        self.assertEqual(second.stdout, first.stdout)

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

    def test_imported_icl_document_cannot_be_formatted_as_pdl(self):
        self.uri = (self.root / 'model.icl').as_uri()
        text = 'Module Model { DataInPort data; }\n'
        self.open(text)
        edits = self.request('textDocument/formatting', dict(
            textDocument={'uri': self.uri}, options={'tabSize': 2, 'insertSpaces': True}))
        self.assertEqual(edits, [])
        symbols = self.request('textDocument/documentSymbol', {'textDocument': {'uri': self.uri}})
        self.assertIn('Model', {s['name'] for s in symbols})

    def test_array_assignments_and_reads_share_the_array_identity(self):
        source = ('proc p {key} {\nset a(first) 1\nset a(second) 2\n'
                  'puts $a(first)\nset a($key) 3\nputs $a($key)\n}\n'
                  'proc q {} {set a(first) 9; puts $a(first)}\n')
        self.assertEqual(self.open(source)['diagnostics'], [])
        for line, column in ((1, 4), (2, 4), (3, 6), (4, 4), (5, 6)):
            with self.subTest(line=line):
                definitions = self.query('definition', line, column)
                self.assertEqual(len(definitions), 1, definitions)
                self.assertEqual(definitions[0]['range']['start'], dict(line=1, character=4))
                refs = self.query('references', line, column,
                                  context={'includeDeclaration': True})
                self.assertEqual(sorted(r['range']['start']['line'] for r in refs),
                                 [1, 2, 3, 4, 5])
        key = self.query('definition', 5, 9)
        self.assertEqual(key[0]['range']['start'], dict(line=0, character=8))

    def test_namespace_variable_binding_is_shared_with_procedures(self):
        source = ('namespace eval A {\nvariable x 1\nproc p {} {\n'
                  'variable x\nset x 2\nputs $x\n}\n'
                  'proc q {} {set x 3; puts $x}\n}\n'
                  'namespace eval B {variable x 9}\n')
        self.assertEqual(self.open(source)['diagnostics'], [])
        for line, column in ((1, 9), (3, 9), (4, 4), (5, 6)):
            with self.subTest(line=line):
                definitions = self.query('definition', line, column)
                self.assertEqual(len(definitions), 1, definitions)
                self.assertEqual(definitions[0]['range']['start'], dict(line=1, character=9))
                refs = self.query('references', line, column,
                                  context={'includeDeclaration': True})
                self.assertEqual(sorted(r['range']['start']['line'] for r in refs), [1, 3, 4, 5])
        completion = {s['label'] for s in self.query('completion', 5, 6)}
        self.assertIn('x', completion)

    def test_completion_retains_namespace_and_module_context(self):
        source = ('proc global_only {} {}\nnamespace eval A {\n'
                  'proc local_only {} {}\nset a 1\nlocal_only\n}\n'
                  'namespace eval B {set hidden 1}\nglobal_only\n'
                  'iProcsForModule Demo\niProc run {} {}\niCall run\n')
        self.assertEqual(self.open(source)['diagnostics'], [])
        local = {s['label'] for s in self.query('completion', 4, 3)}
        self.assertIn('local_only', local)
        self.assertIn('a', local)
        self.assertNotIn('hidden', local)
        global_scope = {s['label'] for s in self.query('completion', 7, 3)}
        self.assertIn('global_only', global_scope)
        self.assertNotIn('local_only', global_scope)
        self.assertNotIn('a', global_scope)
        pdl = {s['label'] for s in self.query('completion', 10, 8)}
        self.assertIn('run', pdl)
        self.assertIn('helper', pdl)

    def test_qualified_arrays_and_multi_variable_namespace_bindings(self):
        source = ('namespace eval A {\nvariable x 1 y 2\nset arr(k) 3\n'
                  'proc p {} {\nvariable x y_value y\n'
                  'puts $x\nputs $y\nset ::A::arr(other) 4\n'
                  'puts $::A::arr(k)\n}\nputs ${arr(k)}\n}\n')
        self.assertEqual(self.open(source)['diagnostics'], [])
        for line, column, target_line, target_column in (
                (5, 6, 1, 9), (6, 6, 1, 13), (7, 9, 2, 4),
                (8, 11, 2, 4), (10, 7, 2, 4)):
            with self.subTest(line=line):
                result = self.query('definition', line, column)
                self.assertEqual(len(result), 1, result)
                self.assertEqual(result[0]['range']['start'],
                                 dict(line=target_line, character=target_column))

    def test_completion_keeps_pdl_call_namespaces_separate_from_tcl(self):
        source = ('iProcsForModule Demo -iProcNameSpace selected\n'
                  'iProc special {} {}\niUseProcNameSpace selected\niCall special\n'
                  'iUseProcNameSpace -\niCall helper\n'
                  'namespace eval selected {\nproc tcl_local {} {}\ntcl_local\n}\n')
        self.assertEqual(self.open(source)['diagnostics'], [])
        selected = {s['label'] for s in self.query('completion', 3, 8)}
        self.assertIn('special', selected)
        self.assertNotIn('tcl_local', selected)
        default = {s['label'] for s in self.query('completion', 5, 8)}
        self.assertIn('helper', default)
        self.assertNotIn('special', default)
        tcl_scope = {s['label'] for s in self.query('completion', 8, 3)}
        self.assertIn('tcl_local', tcl_scope)
        self.assertNotIn('special', tcl_scope)

    def test_unknown_tcl_namespace_does_not_invent_global_symbols(self):
        source = ('namespace eval $ns {\nproc hidden {arg} {puts $arg}\n'
                  'set hidden_var 1\nproc ::visible {} {}\n}\n'
                  'hidden\nputs $hidden_var\n::visible\n')
        self.assertEqual(self.open(source)['diagnostics'], [])
        self.assertEqual(self.query('definition', 5, 2), [])
        self.assertEqual(self.query('definition', 6, 8), [])
        self.assertEqual(self.query('definition', 7, 4)[0]['range']['start']['line'], 3)
        labels = {s['label'] for s in self.query('completion', 5, 2)}
        self.assertNotIn('hidden', labels)
        self.assertNotIn('hidden_var', labels)
        self.assertIn('visible', labels)
        # Local arguments remain known even when their procedure's namespace is not.
        self.assertEqual(self.query('definition', 1, 25)[0]['range']['start']['line'], 1)

    def test_relative_qualified_tcl_command_falls_back_to_global(self):
        source = ('namespace eval lib {proc helper {} {}}\n'
                  'namespace eval outer {\nlib::helper\n'
                  '::lib::helper\n}\n')
        self.assertEqual(self.open(source)['diagnostics'], [])
        for line in (2, 3):
            with self.subTest(line=line):
                result = self.query('definition', line, 5)
                self.assertEqual(len(result), 1)
                self.assertEqual(result[0]['range']['start']['line'], 0)

    def test_global_bindings_support_qualified_names_and_arrays(self):
        source = ('set x 1\nnamespace eval A {set arr(k) 2}\n'
                  'namespace eval inner {\nproc p {} {\n'
                  'global x A::arr\nputs $x\nputs $arr(k)\n'
                  'set x 3\n}\n}\n')
        self.assertEqual(self.open(source)['diagnostics'], [])
        for line, column, target in ((5, 6, 0), (6, 7, 1), (7, 4, 0)):
            with self.subTest(line=line):
                result = self.query('definition', line, column)
                self.assertEqual(len(result), 1)
                self.assertEqual(result[0]['range']['start']['line'], target)
        labels = {s['label'] for s in self.query('completion', 5, 6)}
        self.assertTrue({'x', 'arr'} <= labels)
        refs = self.query('references', 0, 4, context={'includeDeclaration': False})
        self.assertTrue({5, 7} <= {r['range']['start']['line'] for r in refs})

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

    def test_repeated_assignments_share_one_scoped_variable(self):
        source = ('proc p {} {set x 1; set x 2; puts $x}\n'
                  'proc q {} {set x 3; puts $x}\n'
                  'proc a {x} {set x 4; puts $x}\n'
                  'namespace eval A {set x 1; set x 2; puts $x}\n'
                  'namespace eval B {set x 1; puts $x}\n')
        self.assertEqual(self.open(source)['diagnostics'], [])
        for line, text in enumerate(source.splitlines()):
            positions = [i for i, char in enumerate(text) if char == 'x']
            for position in positions:
                with self.subTest(line=line, position=position):
                    refs = self.query('references', line, position,
                                      context={'includeDeclaration': True})
                    self.assertEqual(sorted((ref['range']['start']['line'],
                                             ref['range']['start']['character'])
                                            for ref in refs),
                                     [(line, column) for column in positions])
                    refs = self.query('references', line, position,
                                      context={'includeDeclaration': False})
                    self.assertEqual(sorted(ref['range']['start']['character']
                                            for ref in refs), positions[1:])
                    definition = self.query('definition', line, position)
                    self.assertEqual(definition[0]['range']['start'],
                                     dict(line=line, character=positions[0]))

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

    def test_completion_does_not_leak_empty_procedure_arguments(self):
        for version, command in enumerate(('proc', 'iProc'), 1):
            with self.subTest(command=command):
                self.assertEqual(self.open(HEADER + command + ' run {value} {}\n\n',
                                           version)['diagnostics'], [])
                labels = {item['label'] for item in self.query('completion', 3, 0)}
                self.assertNotIn('value', labels)

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
