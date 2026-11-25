#!/usr/bin/env python3
"""Crossing preparation uses real manual-parent controls and mixed raw flags."""
import json
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest
from unittest.mock import patch

import test_benchmark_dynamic_opaque_routes as shared

SCRIPT = Path(__file__).resolve().parents[4]/'tests/src/pattern/benchmark_opaque_crossing_preparation.py'


class OpaqueCrossingBenchmark(unittest.TestCase):
    test_generated_rewrite_checks_actual_unknown_bits_and_each_clear = shared.DynamicOpaqueBenchmark.test_generated_rewrite_checks_actual_unknown_bits_and_each_clear
    test_missing_sanitizer_cannot_produce_measurements = shared.DynamicOpaqueBenchmark.test_missing_sanitizer_cannot_produce_measurements
    def setUp(self):
        selection = patch.object(shared,'SCRIPT',SCRIPT)
        selection.start()
        self.addCleanup(selection.stop)

    def test_manual_parent_preparation_has_matching_native_words(self):
        benchmark = shared.load()
        parent,expected,words,compares = benchmark.commands(2,8,parent=True)
        current,actual,current_words,current_compares = benchmark.commands(2,8)
        self.assertEqual((expected,words,compares),(actual,current_words,current_compares))
        self.assertNotIn('modeled ',parent)
        self.assertNotIn('iWrite enable',current)
        self.assertEqual(parent.count('iWrite enable1 1; iApply'),4)
        self.assertEqual(parent.count('iApply'),17)
        self.assertEqual(current.count('iApply'),13)
        self.assertEqual([len(word)-2 for word in words],[10]+[18,26,26]*4)
        self.assertEqual(compares,['0b'+'x'*10]+['0b'+'x'*18,'0b'+'x'*26,'0b'+'x'*26]*4)
        self.assertEqual(words[1],'0b101101001011010011')
        self.assertEqual(expected[:8],['0bxxxxxxxx','0b00000000']*2+
            ['0b101xxxxxxxx1xxxxxxxx110011','0b'+'0'*26,'0','0'])
        self.assertNotIn('ScanRegister value',benchmark.model(2,8))
        self.assertEqual(len(benchmark.GRID),11)

    def test_zero_exit_without_products_fails_complete_matrix(self):
        with tempfile.TemporaryDirectory() as folder:
            result = subprocess.run([sys.executable,str(SCRIPT),'--binary','/bin/true',
                '--parent','/bin/true','--sanitized','/bin/true','--output',folder],
                capture_output=True,text=True,timeout=60)
            self.assertEqual(result.returncode,1,result.stdout+result.stderr)
            self.assertNotIn('Traceback',result.stderr)
            self.assertEqual(json.loads(result.stdout),dict(runs=168,expected_runs=168,
                                                            matrix_complete=True,all_verified=False))
            rows = json.loads((Path(folder)/'results.json').read_text())['runs']
            self.assertEqual(len(rows),168)
            self.assertTrue(all(x['exit']==0 and not x['actual_histories_verified'] for x in rows))


if __name__=='__main__':
    unittest.main()
