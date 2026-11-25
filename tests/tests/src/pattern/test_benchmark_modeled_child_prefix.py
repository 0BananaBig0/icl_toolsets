#!/usr/bin/env python3
"""Measured child-prefix reports require actual outputs and live getters."""
import importlib.util
import json
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[4]
SCRIPT = ROOT/'tests/src/pattern/benchmark_modeled_child_prefix.py'


def load():
    spec = importlib.util.spec_from_file_location('modeled_child_benchmark',SCRIPT)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


class ModeledChildBenchmark(unittest.TestCase):
    def test_parent_uses_native_child_request_identity_notes(self):
        benchmark = load()
        parent,expected,words,compares = benchmark.commands(2,8,parent=True)
        current,actual,current_words,current_compares = benchmark.commands(2,8)
        self.assertEqual((expected,words,compares),(actual,current_words,current_compares))
        self.assertEqual(parent,current)
        self.assertNotIn('modeled child raw history ',current)
        self.assertNotIn('modeled readback status clear ',parent)
        self.assertNotIn('modeled raw history omitted ',parent)
        self.assertEqual(len(expected),32)
        self.assertEqual(len(words),8)
        self.assertEqual(expected[:4],['0b10100101']*2+['0b00111100']*2)
        self.assertEqual(expected[6:8],['8','0'])

    def test_generated_rewrite_checks_values_and_replaces_each_clear_once(self):
        benchmark = load()
        _,expected,_,_ = benchmark.commands(2,8)
        data = 'iPDLLevel 0\n'
        for index,value in enumerate(expected):
            if index%8==6:
                data += 'iNote -comment "modeled readback status clear '+value+'"\n'
            data += 'iNote '+value+'\n'
        replay = benchmark.replay_observations(data,2,expected)
        self.assertEqual(replay.count('iGetReadData child'),8)
        self.assertEqual(replay.count('iGetMiscompares child'),8)
        self.assertEqual(replay.count('iGetStatus -clear'),4)
        self.assertNotIn('modeled readback status clear ',replay)
        with self.assertRaises(ValueError):
            benchmark.replay_observations(data.replace('iNote 8\n','iNote 7\n',1),2,expected)

    def test_zero_exit_without_products_fails_complete_matrix(self):
        with tempfile.TemporaryDirectory() as folder:
            result = subprocess.run([sys.executable,str(SCRIPT),'--binary','/bin/true',
                '--parent','/bin/true','--sanitized','/bin/true','--output',folder],
                capture_output=True,text=True,timeout=60)
            self.assertEqual(result.returncode,1,result.stdout+result.stderr)
            self.assertNotIn('Traceback',result.stderr)
            self.assertEqual(json.loads(result.stdout),dict(runs=216,expected_runs=216,
                                                            matrix_complete=True,all_verified=False))
            rows = json.loads((Path(folder)/'results.json').read_text())['runs']
            self.assertEqual(len(rows),216)
            self.assertTrue(all(x['exit']==0 and not x['actual_histories_verified'] for x in rows))

    def test_missing_sanitizer_cannot_produce_measurements(self):
        with tempfile.TemporaryDirectory() as folder:
            result = subprocess.run([sys.executable,str(SCRIPT),'--binary','/bin/true',
                '--parent','/bin/true','--output',folder],capture_output=True,text=True,timeout=15)
            self.assertNotEqual(result.returncode,0)
            self.assertIn('--sanitized',result.stderr)
            self.assertFalse((Path(folder)/'results.json').exists())


if __name__ == '__main__':
    unittest.main()
