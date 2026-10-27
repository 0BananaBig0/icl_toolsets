#!/usr/bin/env python3
"""Verify static pattern comparison reuses the PDL tree without executing Tcl."""
from pathlib import Path
import json
import os
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[2]
BINARY = Path(os.environ.get('PDL_TRACE', ROOT / 'build/pdl_pattern_trace')).resolve()


class Trace(unittest.TestCase):
    def run_trace(self, source, entry=''):
        with tempfile.TemporaryDirectory() as temp:
            path = Path(temp) / 'input.pdl'
            path.write_text(source)
            return subprocess.run([str(BINARY), str(path), entry], capture_output=True,
                                  text=True, timeout=10)

    def test_wrapper_and_main_have_identical_operations(self):
        source = 'iProcsForModule M\niWrite r[0] 0b1\niApply\n'
        wrapped = ('iProcsForModule M\niProc oracle {} {\n# vendor comment\n'
                   'iWrite r[0] 0b1; iApply\n}\n')
        a, b = self.run_trace(source), self.run_trace(wrapped, 'oracle')
        self.assertEqual(a.returncode, 0, a.stderr)
        self.assertEqual(b.returncode, 0, b.stderr)
        self.assertEqual(json.loads(a.stdout), json.loads(b.stdout))
        self.assertEqual(json.loads(a.stdout)['operations'],
                         [['iWrite', 'r[0]', '0b1'], ['iApply']])

    def test_operation_order_and_values_are_not_discarded(self):
        a = self.run_trace('iWrite r 1\niApply\niWrite r 0\niApply')
        b = self.run_trace('iWrite r 0\niApply\niWrite r 1\niApply')
        self.assertEqual(a.returncode, 0, a.stderr)
        self.assertEqual(b.returncode, 0, b.stderr)
        self.assertNotEqual(json.loads(a.stdout), json.loads(b.stdout))

    def test_state_assumptions_are_retained_in_strict_comparison(self):
        source = 'iState gate[0] 0b1 -LastWrittenValue\niWrite r 5\niApply\n'
        actual = self.run_trace(source)
        wrapped = self.run_trace('iProcsForModule M\niProc oracle {} {\n' + source + '}\n', 'oracle')
        self.assertEqual(actual.returncode, 0, actual.stderr)
        self.assertEqual(wrapped.returncode, 0, wrapped.stderr)
        expected = [['iState', 'gate[0]', '0b1', '-LastWrittenValue'],
                    ['iWrite', 'r', '5'], ['iApply']]
        self.assertEqual(json.loads(actual.stdout)['operations'], expected)
        self.assertEqual(json.loads(wrapped.stdout)['operations'], expected)
        for changed in (source.replace('0b1', '0b0'), 'iWrite r 5\niApply\n'):
            result = self.run_trace(changed)
            self.assertEqual(result.returncode, 0, result.stderr)
            self.assertNotEqual(json.loads(result.stdout)['operations'], expected)

    def test_dynamic_commands_ambiguous_entries_and_missing_entry_rejected(self):
        cases = [('iWrite r [exec echo bad]\niApply', ''),
                 ('iWrite r[0][exec echo bad] 1\niApply', ''),
                 (r'iWrite r \x31' + '\niApply', ''),
                 ('iWrite r $value\niApply', ''),
                 ('iState r $value\niApply', ''),
                 ('iCall run', ''),
                 ('iProcsForModule M\niProc oracle {} {iApply}\niApply', 'oracle'),
                 ('iProcsForModule M\niProc other {} {iApply}', 'oracle'),
                 ('iProcsForModule M\niProc oracle {value} {iApply}', 'oracle'),
                 ('iProcsForModule M\niProc oracle {} {iApply}\n'
                  'iProc oracle {} {iApply}', 'oracle')]
        for source, entry in cases:
            with self.subTest(source=source):
                run = self.run_trace(source, entry)
                self.assertNotEqual(run.returncode, 0)
                self.assertEqual(run.stdout, '')


if __name__ == '__main__':
    unittest.main()
