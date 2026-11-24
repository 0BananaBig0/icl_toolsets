#!/usr/bin/env python3
"""The native decimal recorder requires all four measured modes."""
from pathlib import Path
import json
import subprocess
import sys
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[4]
BENCHMARK = ROOT/'tests/src/pattern/benchmark_wide_decimal_constants.py'
BINARY = Path(sys.argv.pop(1)).resolve() if len(sys.argv) > 1 else ROOT/'build/pattern_expression_test'


class DecimalBenchmark(unittest.TestCase):
    def run_recorder(self, sanitized):
        with tempfile.TemporaryDirectory(prefix='decimal-benchmark-') as folder:
            command = [sys.executable, str(BENCHMARK), '--binary', str(BINARY),
                       '--parent', str(BINARY), '--output', folder]
            if sanitized:
                command += ['--sanitized', str(BINARY)]
            result = subprocess.run(command, cwd=ROOT, capture_output=True, text=True, timeout=60)
            report = Path(folder)/'results.json'
            return result, json.loads(report.read_text()) if report.exists() else None

    def test_missing_sanitizer_cannot_report_completion(self):
        result, report = self.run_recorder(False)
        self.assertNotEqual(result.returncode, 0)
        self.assertIn('--sanitized', result.stderr)
        self.assertIsNone(report)

    def test_complete_matrix_checks_all_twelve_processes(self):
        result, report = self.run_recorder(True)
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual(json.loads(result.stdout), dict(runs=12, matrix_complete=True, all_values_verified=True))
        self.assertEqual({(row['owner'], row['mode'], row['repeat']) for row in report['runs']},
                         {(owner, mode, repeat) for owner, mode in
                          [('parent', 'hex'), ('current', 'hex'), ('current', 'decimal'),
                           ('sanitized', 'decimal')] for repeat in range(3)})
        self.assertTrue(all(row['values_verified'] and row['checked_values'] == 10
                            and row['within_bounds'] for row in report['runs']))


if __name__ == '__main__':
    unittest.main()
