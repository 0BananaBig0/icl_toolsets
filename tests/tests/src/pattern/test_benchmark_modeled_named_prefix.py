#!/usr/bin/env python3
"""Named-history measurements validate implicit frames and actual observations."""
import importlib.util
import json
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[4]
SCRIPT = ROOT/'tests/src/pattern/benchmark_modeled_named_prefix.py'


def load():
    spec = importlib.util.spec_from_file_location('named_benchmark',SCRIPT)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


class ModeledNamedBenchmark(unittest.TestCase):
    def test_controls_retain_real_implicit_frames_and_old_cared_histories(self):
        benchmark = load()
        source,notes,queries,frames = benchmark.commands(2,8)
        self.assertEqual(source.count('iScan client'),4)
        self.assertEqual(len(frames),4)
        self.assertEqual(frames[0][0],('c0','0b00100101','0b11111111',True))
        self.assertEqual(frames[1][0],('c0','0b00100101','0bxxxxxxxx',False))
        self.assertEqual(notes[:4],['0b00000000','0b11111111','absent_c1','8'])
        self.assertEqual(notes[5:9],['0b00000000','0b11111111','0b00000000','0b00000000'])
        self.assertTrue(any('catch' in query for query in queries))
        self.assertIn('ScanRegister r0[7:0]',benchmark.model(2,8))

    def test_rewriter_validates_actual_capture_masks_and_nonzero_clear(self):
        benchmark = load()
        _,notes,queries,_ = benchmark.commands(2,8)
        data='iPDLLevel 0\n'
        for note,query in zip(notes,queries):
            if '-clear' in query:data+='iNote -comment "modeled readback status clear '+note+'"\n'
            data+='iNote '+note+'\n'
        live = benchmark.replay_observations(data,2,8)
        self.assertIn('catch',live)
        self.assertEqual(live.count('iGetStatus -clear'),4)
        self.assertNotIn('modeled readback status clear ',live)
        for bad in (data.replace('iNote 0b11111111','iNote 0b00000000',1),
                    data.replace('status clear 8','status clear 0',1),
                    data.replace('iNote absent_c1\n','',1)):
            with self.assertRaises(ValueError):benchmark.replay_observations(bad,2,8)

    def test_missing_sanitizer_does_not_start_measurements(self):
        with tempfile.TemporaryDirectory() as folder:
            output=Path(folder)/'results'
            p=subprocess.run([sys.executable,str(SCRIPT),'--binary','/bin/true',
                '--parent','/bin/true','--output',str(output)],capture_output=True,text=True)
            self.assertEqual(p.returncode,2)
            self.assertIn('--sanitized',p.stderr)
            self.assertFalse(output.exists())

    def test_only_exact_implicit_metadata_can_be_removed_from_comparison(self):
        benchmark=load()
        _,notes,queries,frames=benchmark.commands(2,8)
        groups=[];group=[]
        for note,query in zip(notes,queries):
            if '-clear' in query:group.append('iNote -comment "modeled readback status clear '+note+'"')
            group.append('iNote '+note)
            if query=='iNote [iGetStatus]':groups.append(group);group=[]
        source='iPDLLevel 0\niProcsForModule Chains\niReset\n';parent=source
        for frame,observations in zip(frames,groups):
            for chain,word,expected,explicit in frame:
                scan='iScan client -chain %s 8 -si %s -so %s -stable'%(chain,word,expected)
                if not explicit:source+='iNote -comment "'+benchmark.PREFIX+scan+'"\n'
                source+=scan+'\n';parent+=scan+'\n'
            suffix='iApply\n'+'\n'.join(observations)+'\n'
            source+=suffix;parent+=suffix
        self.assertEqual(benchmark.normalized_output(source,2,8,True),parent)
        self.assertEqual(benchmark.normalized_output(parent,2,8,False),parent)
        bad_note=source.replace(benchmark.PREFIX+'iScan client -chain c1 8 -si 0b00000000',
                                benchmark.PREFIX+'iScan client -chain c1 8 -si 0b00000001',1)
        observed=[line for line in source.splitlines() if line.startswith('iNote ') and benchmark.PREFIX not in line]
        physical=[line for line in source.splitlines() if not line.startswith('iNote ') or benchmark.PREFIX in line]
        moved='\n'.join(physical+observed)+'\n'
        for bad in (bad_note,moved,source.replace('iScan client -chain c0 8 -si 0b00100101 -so',
                                               'iScan client -chain c0 8 -si 0b00100100 -so',1)):
            with self.assertRaises(ValueError):benchmark.normalized_output(bad,2,8,True)

    def test_zero_exit_without_output_fails_every_actual_measurement(self):
        with tempfile.TemporaryDirectory() as folder:
            output=Path(folder)/'results'
            p=subprocess.run([sys.executable,str(SCRIPT),'--binary','/bin/true',
                '--parent','/bin/true','--sanitized','/bin/true','--output',str(output)],capture_output=True,text=True)
            self.assertEqual(p.returncode,1,p.stdout+p.stderr)
            rows=json.loads((output/'results.json').read_text())['runs']
            self.assertEqual(len(rows),216)
            self.assertTrue(all(row['exit']==0 and not row['actual_histories_verified'] for row in rows))
            self.assertTrue(all(row['raw_shifted_bits']==row['raw_scan_count']==row['csu_count']==0 for row in rows))


if __name__=='__main__':unittest.main()
