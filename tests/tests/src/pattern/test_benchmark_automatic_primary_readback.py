#!/usr/bin/env python3
"""Measurements must prove actual callback data, timing and parent hardware."""
import importlib.util
import json
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest

SCRIPT=Path(__file__).resolve().parents[4]/'tests/src/pattern/benchmark_automatic_primary_readback.py'

def load():
    spec=importlib.util.spec_from_file_location('automatic_benchmark',SCRIPT)
    module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module)
    return module

class AutomaticBenchmark(unittest.TestCase):
    def fixture(self):
        b=load();_,notes,queries=b.commands(2,8)
        hardware=b'iPDLLevel 0\niProcsForModule Chains\niReset\niWrite en0 0b0\niApply\n'
        data=hardware.replace(b'iReset\n',b.HEADER+b'iReset\n')
        for note,query in zip(notes,queries):
            if '-clear' in query:data+=('iNote -comment "modeled readback status clear '+note+'"\n').encode()
            data+=('iNote '+note+'\n').encode()
        return b,hardware,data

    def test_reference_keeps_asymmetric_endpoint_and_unknown_downstream(self):
        b=load();source,notes,queries=b.commands(2,8)
        self.assertIn('WriteCallBack Chains store0',b.model(2,8))
        self.assertIn('iWrite dest1',source)
        self.assertEqual(notes[:4],['0b00100101','0b00100101','0bxxxxxxxx','0b00000000'])
        self.assertEqual(notes[-3:],['6','6','0'])
        self.assertEqual(len(notes),len(queries))

    def test_only_validated_observation_bytes_are_removed(self):
        b,hardware,data=self.fixture()
        self.assertEqual(b.hardware_output(data,2,8),hardware)
        with self.assertRaises(ValueError):b.hardware_output(data[:-1],2,8)
        changed=data.replace(b'iPDLLevel 0\n',b'iPDLLevel 0\r\n',1)
        self.assertNotEqual(b.hardware_output(changed,2,8),hardware)

    def test_wrong_data_masks_and_clear_counts_reject(self):
        b,_,data=self.fixture()
        for bad in (data.replace(b'0b00100101',b'0b00000000',1),
                    data.replace(b'iNote 0bxxxxxxxx',b'iNote 0b00100101',1),
                    data.replace(b'status clear 6',b'status clear 0',1),
                    data.replace(b'iNote 0b00000000\n',b'',1)):
            with self.assertRaises(ValueError):b.hardware_output(bad,2,8)

    def test_observations_and_clear_annotations_cannot_move(self):
        b,_,data=self.fixture()
        moved=data.replace(b'iApply\niNote 0b00100101\n',b'iNote 0b00100101\niApply\n',1)
        with self.assertRaises(ValueError):b.hardware_output(moved,2,8)
        note=b'iNote -comment "modeled readback status clear 6"\n'
        with self.assertRaises(ValueError):b.hardware_output(data.replace(note,b'',1)+note,2,8)

    def test_generated_replay_reexecutes_all_actual_queries(self):
        b,_,data=self.fixture();live=b.replay_observations(data,2,8)
        self.assertIn('iGetReadData q1 -bin',live)
        self.assertEqual(live.count('iGetStatus -clear'),1)
        self.assertNotIn('modeled readback status clear',live)
        with self.assertRaises(ValueError):b.replay_observations(data.replace(b'0bxxxxxxxx',b'0b11111111',1),2,8)

    def test_zero_exit_without_output_never_verifies_measurements(self):
        with tempfile.TemporaryDirectory() as folder:
            output=Path(folder)/'results'
            p=subprocess.run([sys.executable,str(SCRIPT),'--binary','/bin/true',
                '--parent','/bin/true','--sanitized','/bin/true','--output',str(output)],capture_output=True,text=True)
            self.assertEqual(p.returncode,1,p.stdout+p.stderr)
            rows=json.loads((output/'results.json').read_text())['runs']
            self.assertEqual(len(rows),135)
            self.assertTrue(all(row['exit']==0 and not row['actual_histories_verified'] for row in rows))

if __name__=='__main__':unittest.main()
