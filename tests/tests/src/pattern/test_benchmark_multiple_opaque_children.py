#!/usr/bin/env python3
"""Zero-exit non-generators cannot qualify the opaque replay matrix."""
from pathlib import Path
import json
import subprocess
import sys
import tempfile
import unittest
from unittest import mock

ROOT = Path(__file__).resolve().parents[4]
BENCHMARK = ROOT/'tests/src/pattern/benchmark_multiple_opaque_children.py'


class MultipleOpaqueBenchmark(unittest.TestCase):
    def test_multichild_uses_opaque_budget_and_retains_single_wide_control(self):
        import contextlib
        import importlib.util
        import io
        spec = importlib.util.spec_from_file_location('opaque_recorder', BENCHMARK)
        benchmark = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(benchmark)
        original_run = subprocess.run
        def measured(command, **kwargs):
            process = original_run(command, **kwargs)
            # Inject only the measurement boundary; no generator output is invented.
            if command[0]=='/usr/bin/time':
                Path(command[command.index('-o')+1]).write_text('0.001 307200\n')
            return process
        with tempfile.TemporaryDirectory(prefix='multiple-opaque-budget-') as folder:
            arguments = [str(BENCHMARK), '--binary', '/bin/true', '--parent', '/bin/true',
                         '--sanitized', '/bin/true', '--output', folder]
            with mock.patch.object(sys, 'argv', arguments), mock.patch.object(
                    subprocess, 'run', side_effect=measured), contextlib.redirect_stdout(io.StringIO()):
                self.assertEqual(benchmark.main(), 1)
            rows = json.loads((Path(folder)/'results.json').read_text())['runs']
            controls = [row for row in rows if row['children']==1 and row['width']==1024]
            multiple = [row for row in rows if row['children']>1 and row['width']==1024]
            self.assertTrue(controls and multiple)
            self.assertTrue(all(not row['within_bounds'] for row in controls))
            self.assertTrue(all(row['within_bounds'] for row in multiple))

    def test_zero_exit_without_outputs_is_a_complete_failed_matrix(self):
        with tempfile.TemporaryDirectory(prefix='multiple-opaque-report-') as folder:
            result = subprocess.run([sys.executable, str(BENCHMARK), '--binary', '/bin/true',
                '--parent', '/bin/true', '--sanitized', '/bin/true', '--output', folder],
                cwd=ROOT, capture_output=True, text=True, timeout=60)
            self.assertNotEqual(result.returncode, 0)
            self.assertNotIn('Traceback', result.stderr)
            self.assertEqual(json.loads(result.stdout), dict(runs=183, expected_runs=183,
                                                            matrix_complete=True, all_verified=False))
            rows = json.loads((Path(folder)/'results.json').read_text())['runs']
            self.assertEqual(len(rows), 183)
            self.assertTrue(all(row['exit']==0 and not row['actual_histories_verified'] for row in rows))

    def test_missing_sanitizer_does_not_measure(self):
        with tempfile.TemporaryDirectory(prefix='multiple-opaque-missing-') as folder:
            result = subprocess.run([sys.executable, str(BENCHMARK), '--binary', '/bin/true',
                '--parent', '/bin/true', '--output', folder],
                cwd=ROOT, capture_output=True, text=True, timeout=15)
            self.assertNotEqual(result.returncode, 0)
            self.assertIn('--sanitized', result.stderr)
            self.assertFalse((Path(folder)/'results.json').exists())


if __name__ == '__main__':
    unittest.main()
