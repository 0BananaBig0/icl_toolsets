#!/usr/bin/env python3
"""Zero-exit non-generators cannot qualify the opaque replay matrix."""
from pathlib import Path
import importlib.util
import json
import subprocess
import sys
import tempfile
import unittest
from unittest import mock

ROOT = Path(__file__).resolve().parents[4]
BENCHMARK = ROOT/'tests/src/pattern/benchmark_multiple_opaque_children.py'

def load_benchmark():
    spec = importlib.util.spec_from_file_location('opaque_recorder', BENCHMARK)
    benchmark = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(benchmark)
    return benchmark


class MultipleOpaqueBenchmark(unittest.TestCase):
    def test_multichild_uses_opaque_budget_and_retains_single_wide_control(self):
        import contextlib
        import io
        benchmark = load_benchmark()
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
            self.assertEqual(json.loads(result.stdout), dict(runs=216, expected_runs=216,
                                                            matrix_complete=True, all_verified=False))
            rows = json.loads((Path(folder)/'results.json').read_text())['runs']
            self.assertEqual(len(rows), 216)
            self.assertTrue(all(row['exit']==0 and not row['actual_histories_verified'] for row in rows))

    def test_missing_sanitizer_does_not_measure(self):
        with tempfile.TemporaryDirectory(prefix='multiple-opaque-missing-') as folder:
            result = subprocess.run([sys.executable, str(BENCHMARK), '--binary', '/bin/true',
                '--parent', '/bin/true', '--output', folder],
                cwd=ROOT, capture_output=True, text=True, timeout=15)
            self.assertNotEqual(result.returncode, 0)
            self.assertIn('--sanitized', result.stderr)
            self.assertFalse((Path(folder)/'results.json').exists())

    def test_generated_form_issues_each_named_child_and_root_query(self):
        benchmark = load_benchmark()
        values = ['0bxxxxxxxx','0bxxxxxxxx','0b101xxxxxxxxxxxxxxxx10011',
                  '0b000000000000000000000000','0','0']*4
        data = 'iPDLLevel 0\niNote {modeled register/PIO readback; no live device}\n'
        data += ''.join('iNote '+value+'\n' for value in values)
        rewritten = benchmark.replay_observations(data, 2, values)
        queries = ['iNote [iGetReadData child0.local -bin]',
                   'iNote [iGetReadData child1.local -bin]',
                   'iNote [iGetReadData client -bin]',
                   'iNote [iGetMiscompares client -bin]',
                   'iNote [iGetStatus -clear]', 'iNote [iGetStatus]']*4
        self.assertEqual([line for line in rewritten.splitlines() if line.startswith('iNote [')], queries)
        self.assertTrue(rewritten.startswith('iPDLLevel 1\n'))
        with self.assertRaises(ValueError):
            benchmark.replay_observations(data.replace('iNote 0\n', 'iNote 1\n', 1), 2, values)

    def test_parent_comments_do_not_duplicate_native_child_identities(self):
        benchmark = load_benchmark()
        text, _, _, _ = benchmark.commands([8,16], parent=True)
        self.assertNotIn('modeled opaque child raw history ',text)
        self.assertNotIn('modeled raw history omitted ',text)
        self.assertEqual(sum(line.startswith('iScan ') for line in text.splitlines()), 12)
        self.assertEqual(text.splitlines().count('iApply'), 4)

    def test_parent_clear_comments_do_not_duplicate_native_events(self):
        benchmark = load_benchmark()
        text, _, _, _ = benchmark.commands([8,16], parent=True)
        marker = 'iNote -comment "modeled readback status clear 0"'
        self.assertNotIn(marker,text)
        current, _, _, _ = benchmark.commands([8,16])
        self.assertNotIn('modeled readback status clear ',current)

    def test_live_clear_getter_replaces_its_recorded_event_once(self):
        benchmark = load_benchmark()
        values = ['0bxxxxxxxx','0b101xxxxxxxx10011','0b'+'0'*16,'0','0']*4
        values[3] = '5'
        marker = 'iNote -comment "modeled readback status clear 0"'
        data = 'iPDLLevel 0\n'
        for position,value in enumerate(values):
            if position%5==3:
                data += 'iNote -comment "modeled readback status clear '+value+'"\n'
            data += 'iNote '+value+'\n'
        rewritten = benchmark.replay_observations(data,1,values)
        self.assertNotIn('modeled readback status clear ',rewritten)
        self.assertEqual(rewritten.count('iGetStatus -clear'),4)
        with self.assertRaises(ValueError):
            benchmark.replay_observations(data.replace('status clear 5','status clear 6',1),1,values)
        parent,_,_,_ = benchmark.commands([8],parent=True)
        self.assertEqual(parent.count(marker),0)
        current,_,_,_ = benchmark.commands([8])
        self.assertNotIn('modeled readback status clear ',current)


if __name__ == '__main__':
    unittest.main()
