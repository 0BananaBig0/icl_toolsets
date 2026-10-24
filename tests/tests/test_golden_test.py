#!/usr/bin/env python3
"""Golden expectations must detect bad formatting and fail closed on tool errors."""
import contextlib
import io
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest
from unittest.mock import Mock, patch

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import golden_test


class GoldenRunner(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.addCleanup(self.tmp.cleanup)
        self.root = Path(self.tmp.name)
        self.source = b'iProc sample {} { iApply }\n'
        self.expected = b'iProc sample {} {\n  iApply\n}\n'
        self.output = io.StringIO()

    def fixture(self, name='sample', language='pdl'):
        (self.root / (name + '.' + language)).write_bytes(self.source)
        path = self.root / (name + '.expected')
        path.write_bytes(self.expected)
        return path

    def run_main(self, *args, binary=None):
        argv = ['golden_test.py', '--binary', binary or sys.executable, *args]
        with patch.object(golden_test, 'HERE', str(self.root)), \
                patch.object(golden_test, 'PDL_HERE', str(self.root), create=True), \
                patch.object(sys, 'argv', argv), \
                contextlib.redirect_stdout(self.output), \
                contextlib.redirect_stderr(self.output):
            try:
                return golden_test.main()
            except SystemExit as error:
                return error.code

    def result(self, output=None, code=0):
        return subprocess.CompletedProcess([], code, self.expected if output is None else output,
                                           b'error detail' if code else b'')

    def lsp_client(self):
        client = Mock()
        client.wait.return_value = {'params': {'diagnostics': []}}
        client.request.side_effect = [
            {'result': {}},
            {'result': [{'range': {'start': {'line': 0, 'character': 0},
                                  'end': {'line': 1, 'character': 0}},
                         'newText': self.expected.decode()}]},
            {'result': []},
        ]
        return client

    def test_lsp_checks_original_and_formatted_documents(self):
        for language in ('icl', 'pdl'):
            with self.subTest(language=language):
                self.fixture(language=language)
                client = self.lsp_client()
                with patch.object(golden_test, 'Client', return_value=client, create=True), \
                        patch.object(golden_test.subprocess, 'run', return_value=self.result()):
                    self.assertEqual(self.run_main('--language', language, '--server', 'server'),
                                     0, self.output.getvalue())
                calls = client.notify.call_args_list
                opened = next(c.args[1] for c in calls if c.args[0] == 'textDocument/didOpen')
                changed = next(c.args[1] for c in calls if c.args[0] == 'textDocument/didChange')
                self.assertEqual(opened['textDocument']['text'], self.source.decode())
                self.assertEqual(opened['textDocument']['languageId'], language)
                self.assertEqual(changed['contentChanges'], [{'text': self.expected.decode()}])
                self.assertEqual(changed['textDocument']['version'], 2)
                self.assertEqual(client.wait.call_count, 2)
                client.stop.assert_called_once()

    def test_lsp_range_uses_utf16_units(self):
        self.source = '# 😀'.encode()
        self.fixture(language='icl')
        client = self.lsp_client()
        client.request.side_effect = [
            {'result': {}},
            {'result': [{'range': {'start': {'line': 0, 'character': 0},
                                  'end': {'line': 0, 'character': 4}},
                         'newText': self.expected.decode()}]},
            {'result': []},
        ]
        with patch.object(golden_test, 'Client', return_value=client, create=True), \
                patch.object(golden_test.subprocess, 'run', return_value=self.result()):
            self.assertEqual(self.run_main('--server', 'server'), 0, self.output.getvalue())

    def test_lsp_failures_prevent_baseline_updates_and_stop_server(self):
        expected = self.fixture(language='icl')
        for failure in ('initialize', 'diagnostics', 'second diagnostics',
                        'first timeout', 'first pass', 'second pass', 'range', 'text'):
            with self.subTest(failure=failure):
                client = self.lsp_client()
                responses = list(client.request.side_effect)
                if failure == 'initialize':
                    responses[0] = None
                elif failure == 'diagnostics':
                    client.wait.return_value = {'params': {'diagnostics': [{'message': 'bad'}]}}
                elif failure == 'second diagnostics':
                    client.wait.side_effect = [client.wait.return_value, None]
                elif failure == 'first timeout':
                    responses[1] = None
                elif failure == 'first pass':
                    responses[1] = {'error': {'code': -32603}}
                elif failure == 'second pass':
                    responses[2] = responses[1]
                elif failure == 'range':
                    responses[1]['result'][0]['range']['end']['line'] = 99
                else:
                    responses[1]['result'][0]['newText'] = 'wrong output'
                client.request.side_effect = responses
                with patch.object(golden_test, 'Client', return_value=client, create=True), \
                        patch.object(golden_test.subprocess, 'run', return_value=self.result()):
                    self.assertNotEqual(self.run_main('--server', 'server', '--update'), 0)
                self.assertEqual(expected.read_bytes(), self.expected)
                client.stop.assert_called_once()

    def test_missing_lsp_executable_is_an_error(self):
        expected = self.fixture(language='icl')
        self.assertEqual(self.run_main('--server', str(self.root / 'missing'), '--update'), 2)
        self.assertEqual(expected.read_bytes(), self.expected)

    def test_both_languages_use_file_and_stdin(self):
        for language in ('icl', 'pdl'):
            with self.subTest(language=language):
                self.fixture(language=language)
                with patch.object(golden_test.subprocess, 'run', return_value=self.result()) as run:
                    self.assertEqual(self.run_main('--language', language), 0, self.output.getvalue())
                self.assertEqual(run.call_count, 3)
                self.assertEqual(run.call_args_list[0].args[0][-1], str(self.root / ('sample.' + language)))
                self.assertEqual(run.call_args_list[1].kwargs['input'], self.source)
                self.assertEqual(run.call_args_list[2].kwargs['input'], self.expected)
                self.assertTrue(all(call.kwargs['timeout'] == 10 for call in run.call_args_list))

    def test_case_filter_runs_only_selected_case(self):
        self.fixture('selected')
        self.fixture('unselected')
        with patch.object(golden_test.subprocess, 'run', return_value=self.result()) as run:
            self.assertEqual(self.run_main('--language', 'pdl', '--case', 'select*'), 0)
        self.assertEqual(run.call_count, 3)

    def test_empty_discovery_is_an_error(self):
        self.assertEqual(self.run_main(), 2)

    def test_unmatched_filter_is_an_error(self):
        self.fixture()
        self.assertEqual(self.run_main('--language', 'pdl', '--case', 'missing'), 2)
        self.assertIn('no golden cases', self.output.getvalue())

    def test_wrong_indentation_is_not_accepted_or_recorded(self):
        expected = self.fixture(language='icl')
        with patch.object(golden_test.subprocess, 'run', return_value=self.result(self.source)):
            self.assertEqual(self.run_main(), 1)
        self.assertEqual(expected.read_bytes(), self.expected)
        self.assertIn('---', self.output.getvalue())
        self.assertIn('+++', self.output.getvalue())

    def test_missing_expectation_is_not_created(self):
        expected = self.fixture(language='icl')
        expected.unlink()
        with patch.object(golden_test.subprocess, 'run', return_value=self.result()):
            self.assertEqual(self.run_main(), 1)
        self.assertFalse(expected.exists())

    def test_update_requires_stdin_file_agreement(self):
        expected = self.fixture(language='icl')
        with patch.object(golden_test.subprocess, 'run', side_effect=[
                self.result(self.source), self.result(b'different\n')]):
            self.assertEqual(self.run_main('--update'), 1)
        self.assertEqual(expected.read_bytes(), self.expected)

    def test_explicit_update_records_matching_output(self):
        expected = self.fixture(language='icl')
        with patch.object(golden_test.subprocess, 'run', return_value=self.result(self.source)):
            self.assertEqual(self.run_main('--update'), 0)
        self.assertEqual(expected.read_bytes(), self.source)

    def test_second_pass_must_match_first_even_when_first_matches_golden(self):
        for language in ('icl', 'pdl'):
            for update in (False, True):
                with self.subTest(language=language, update=update):
                    expected = self.fixture(language=language)
                    with patch.object(golden_test.subprocess, 'run', side_effect=[
                            self.result(), self.result(), self.result(b'changed again\n')]):
                        flags = ['--language', language] + (['--update'] if update else [])
                        self.assertEqual(self.run_main(*flags), 1)
                    self.assertEqual(expected.read_bytes(), self.expected)
        self.assertIn('idempotent', self.output.getvalue())

    def test_second_pass_errors_do_not_record_baselines(self):
        expected = self.fixture(language='icl')
        for failure in (self.result(code=2), self.result(code=-11),
                        subprocess.TimeoutExpired('formatter', 10), OSError('cannot run')):
            with self.subTest(failure=failure), patch.object(
                    golden_test.subprocess, 'run', side_effect=[
                        self.result(), self.result(), failure]):
                self.assertEqual(self.run_main('--update'), 1)
            self.assertEqual(expected.read_bytes(), self.expected)

    def test_update_waits_until_all_selected_cases_pass(self):
        first = self.fixture('a', language='icl')
        second = self.fixture('b', language='icl')
        with patch.object(golden_test.subprocess, 'run', side_effect=[
                self.result(self.source), self.result(self.source), self.result(self.source),
                self.result(code=2)]):
            self.assertEqual(self.run_main('--update'), 1)
        self.assertEqual(first.read_bytes(), self.expected)
        self.assertEqual(second.read_bytes(), self.expected)

    def test_baseline_write_error_is_reported(self):
        expected = self.fixture(language='icl')
        expected.unlink()
        expected.mkdir()
        with patch.object(golden_test.subprocess, 'run', return_value=self.result()):
            self.assertEqual(self.run_main('--update'), 1)
        self.assertIn('sample.expected', self.output.getvalue())

    def test_exit_and_signal_are_failures_even_with_expected_stdout(self):
        expected = self.fixture(language='icl')
        for code in (2, -11):
            with self.subTest(code=code), patch.object(
                    golden_test.subprocess, 'run', return_value=self.result(code=code)):
                self.assertEqual(self.run_main('--update'), 1)
        self.assertEqual(expected.read_bytes(), self.expected)

    def test_timeout_kills_child_and_preserves_expectation(self):
        expected = self.fixture(language='icl')
        tool = self.root / 'slow_formatter'
        tool.write_text('#!' + sys.executable + '\nimport time\ntime.sleep(30)\n')
        tool.chmod(0o755)
        self.assertEqual(self.run_main('--timeout', '0.05', '--update', binary=str(tool)), 1)
        self.assertIn('timed out', self.output.getvalue())
        self.assertEqual(expected.read_bytes(), self.expected)

    def test_invalid_timeouts_are_rejected(self):
        for value in ('0', '-1', 'nan', 'inf'):
            with self.subTest(value=value):
                self.assertEqual(self.run_main('--timeout', value), 2)


if __name__ == '__main__':
    unittest.main()
