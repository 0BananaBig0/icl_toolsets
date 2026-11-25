#!/usr/bin/env python3
"""Opaque-root measurements must check real frames and live observations."""
import importlib.util
import json
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[4]
SCRIPT = ROOT/'tests/src/pattern/benchmark_opaque_root_serial_merges.py'


def load():
    spec = importlib.util.spec_from_file_location('root_benchmark',SCRIPT)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


class OpaqueRootBenchmark(unittest.TestCase):
    def test_sequential_control_and_merge_request_the_same_real_frames(self):
        benchmark = load()
        for kind,count in (('scalar',1),('named',2),('named',32)):
            parent,notes,frames = benchmark.commands(kind,count,8,parent=True)
            current,actual,current_frames = benchmark.commands(kind,count,8)
            self.assertEqual((notes,frames),(actual,current_frames))
            self.assertNotIn('iMerge ',parent)
            self.assertEqual(current.count('iMerge -begin'),4)
            self.assertEqual(current.count('iMerge -end'),4)
            self.assertEqual(parent.count('iCall load '),8)
            self.assertEqual(len(frames),8)
            self.assertTrue(all(len(frame)==count for frame in frames))
            self.assertEqual(frames[0][0][1],'0b00100101')
            self.assertEqual(frames[1][0][1],'0b00111000')
            self.assertNotIn('ScanRegister',benchmark.model(kind,count,8))

    def test_generated_rewrite_checks_actual_unknown_masks_and_each_clear(self):
        benchmark = load()
        notes,queries = benchmark.observations('named',2,8)
        source = 'iPDLLevel 0\niProcsForModule Chains\n'
        for note,query in zip(notes,queries):
            if '-clear' in query:
                source += 'iNote -comment "modeled readback status clear 0"\n'
            source += 'iNote '+note+'\n'
        rewritten = benchmark.replay_observations(source,'named',2,8)
        self.assertEqual(rewritten.count('iGetReadData client -chain c'),8)
        self.assertEqual(rewritten.count('iGetMiscompares client -chain c'),8)
        self.assertEqual(rewritten.count('iGetStatus -clear'),4)
        self.assertNotIn('modeled readback status clear ',rewritten)
        for bad in (source.replace('iNote 0bxxxxxxxx','iNote 0b00000000',1),
                    source.replace('modeled readback status clear 0','modeled readback status clear 1',1),
                    source.replace('iNote 0b00000000\n','',1)):
            with self.assertRaises(ValueError):
                benchmark.replay_observations(bad,'named',2,8)

    def test_missing_sanitizer_cannot_create_measurements(self):
        with tempfile.TemporaryDirectory() as folder:
            output = Path(folder)/'measurements'
            result = subprocess.run([sys.executable,str(SCRIPT),'--binary','/bin/true',
                '--parent','/bin/true','--output',str(output)],capture_output=True,text=True)
            self.assertEqual(result.returncode,2)
            self.assertIn('--sanitized',result.stderr)
            self.assertFalse(output.exists())

    def test_zero_exit_without_products_fails_the_complete_matrix(self):
        with tempfile.TemporaryDirectory() as folder:
            output = Path(folder)/'measurements'
            result = subprocess.run([sys.executable,str(SCRIPT),'--binary','/bin/true',
                '--parent','/bin/true','--sanitized','/bin/true','--output',str(output)],
                capture_output=True,text=True)
            self.assertEqual(result.returncode,1,result.stdout+result.stderr)
            rows = json.loads((output/'results.json').read_text())['runs']
            self.assertEqual(len(rows),282)
            self.assertTrue(all(row['exit']==0 and not row['actual_histories_verified'] for row in rows))
            self.assertTrue(all(row['raw_shifted_bits']==0 for row in rows))


if __name__=='__main__':
    unittest.main()
