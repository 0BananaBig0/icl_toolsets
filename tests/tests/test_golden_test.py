#!/usr/bin/env python3
"""Golden expectations must detect bad formatting and fail closed on tool errors."""
import contextlib
import io
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest
from unittest.mock import patch

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

    def test_both_languages_use_file_and_stdin(self):
        for language in ('icl', 'pdl'):
            with self.subTest(language=language):
                self.fixture(language=language)
                with patch.object(golden_test.subprocess, 'run', return_value=self.result()) as run:
                    self.assertEqual(self.run_main('--language', language), 0, self.output.getvalue())
                self.assertEqual(run.call_count, 2)
                self.assertEqual(run.call_args_list[0].args[0][-1], str(self.root / ('sample.' + language)))
                self.assertEqual(run.call_args_list[1].kwargs['input'], self.source)
                self.assertTrue(all(call.kwargs['timeout'] == 10 for call in run.call_args_list))

    def test_case_filter_runs_only_selected_case(self):
        self.fixture('selected')
        self.fixture('unselected')
        with patch.object(golden_test.subprocess, 'run', return_value=self.result()) as run:
            self.assertEqual(self.run_main('--language', 'pdl', '--case', 'select*'), 0)
        self.assertEqual(run.call_count, 2)

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
