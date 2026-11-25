#!/usr/bin/env python3
"""Dynamic opaque measurements require real unknown returns and exact controls."""
import importlib.util
import json
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[4]
SCRIPT = ROOT/'tests/src/pattern/benchmark_dynamic_opaque_routes.py'


def load():
    spec = importlib.util.spec_from_file_location('dynamic_opaque_benchmark',SCRIPT)
    module = importlib.util.module_from_spec(spec)
    sys.path.insert(0,str(SCRIPT.parent))
    try:
        spec.loader.exec_module(module)
    finally:
        sys.path.pop(0)
    return module


class DynamicOpaqueBenchmark(unittest.TestCase):
    def test_parent_is_an_unknown_capture_twin_without_injected_observations(self):
        benchmark = load()
        parent,expected,words,compares = benchmark.commands(2,8,parent=True)
        current,actual,current_words,current_compares = benchmark.commands(2,8)
        self.assertEqual((parent,expected,words,compares),
                         (current,actual,current_words,current_compares))
        self.assertNotIn('modeled ',parent)
        self.assertNotIn('-stable',current)
        self.assertEqual(parent.count('iApply'),13)
        self.assertEqual([len(word)-2 for word in words],[10]+[26,26,10]*4)
        self.assertEqual(compares,['0b'+'x'*10]+
                         ['0b'+'x'*26,'0b'+'x'*26,'0b1010010011']*4)
        self.assertEqual(expected[:8],['0bxxxxxxxx','0b00000000']*2+
                         ['0b1010010011','0b0000000000','0','0'])
        self.assertEqual(words[1],'0b10110100101100111100110011')
        self.assertIn("CaptureSource 8'bx; ResetValue 8'bx;",benchmark.model(2,8,visible=True))
        self.assertNotIn('ScanRegister value',benchmark.model(2,8))

    def test_generated_rewrite_checks_actual_unknown_bits_and_each_clear(self):
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
            benchmark.replay_observations(data.replace('iNote 0bxxxxxxxx\n',
                'iNote 0b00000000\n',1),2,expected)

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


if __name__=='__main__':
    unittest.main()
