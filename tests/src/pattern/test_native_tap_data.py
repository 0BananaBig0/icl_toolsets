#!/usr/bin/env python3
"""Selected fragmented TAP data paths avoid symbolic event construction."""
import os
from pathlib import Path
import re
import signal
import subprocess
import sys
import tempfile
import unittest

import benchmark_raw_tap_multiple_dr as bench

BIN=Path(sys.argv.pop(1)).resolve() if len(sys.argv)>1 else bench.ir.ROOT/'build/icl-pattern-generation'


class NativeTapData(unittest.TestCase):
    def setUp(self):
        temp=tempfile.TemporaryDirectory()
        self.addCleanup(temp.cleanup)
        self.root=Path(temp.name)
        _,model,bsdl,_,_=next(x for x in bench.scenarios() if x[0]=='w64-registers64-unused0-repeat1-aliases0')
        model=re.sub(r'(ScanRegister r[0-9]+)\[0:0\]',r'\1[3:0]',model)
        (self.root/'model.icl').write_text(model)
        (self.root/'model.bsdl').write_text(bsdl)

    def run_tool(self,pdl,output,flat=False,ok=True):
        command=[str(BIN),'--icl','debug/MultipleDrTop.icl' if flat else 'model.icl',
                 '--pdl',pdl,'--output',output]
        if not flat:command+=['--bsdl','MultipleDrProbe','model.bsdl','--debug']
        process=subprocess.Popen(command,cwd=self.root,stdout=subprocess.PIPE,stderr=subprocess.PIPE,start_new_session=True)
        try:stdout,stderr=process.communicate(timeout=15 if ok else 45)
        except subprocess.TimeoutExpired:
            os.killpg(process.pid,signal.SIGKILL)
            process.communicate()
            self.fail('selected 64-register/256-bit TAP transaction exceeded 15 seconds (timeout124)')
        self.assertEqual(process.returncode,0 if ok else 2,(stdout+stderr).decode(errors='replace'))
        if not ok:
            self.assertFalse((self.root/output).exists())
            return stdout+stderr
        return (self.root/output).read_bytes()

    def test_fragmented_path_replay_and_partial_history(self):
        prefix='iPDLLevel 1\niProcsForModule MultipleDrTop\niReset\niWrite tap.INSTRUCTION 3; iRead tap.INSTRUCTION 1; iApply\n'
        tail='iWrite core.r63[0] 0; iRead core.r63 3; iApply\niRead core.r61 10; iRead core.r62 5; iRead core.r63 2; iApply\n'
        (self.root/'source.pdl').write_text(prefix+'iScan jtag 256 -si 2643 -so 0; iApply\n'+tail)
        result=self.run_tool('source.pdl','result.pdl')
        self.assertIn(b'iScan jtag 256',result)
        self.assertIn(b'iWrite core.r63 0b0010',result)
        self.assertEqual(result,self.run_tool('source.pdl','flat.pdl',True))
        self.assertEqual(result,self.run_tool('result.pdl','generated.pdl'))
        ordinary=prefix
        for command,packet in [('iWrite',2643),('iRead',0)]:
            ordinary+=''.join(f'{command} core.r{i} {(packet>>((63-i)*4))&15};\n' for i in range(64))
        (self.root/'ordinary.pdl').write_text(ordinary+'iApply\n'+tail)
        raw=result.split(b'iApply\n',2)
        manual=self.run_tool('ordinary.pdl','ordinary-result.pdl').split(b'iApply\n',2)
        self.assertEqual(bench.canonical(b'iApply\n'.join(raw[:2])),
                         bench.canonical(b'iApply\n'.join(manual[:2])))
        self.assertEqual(raw[2],manual[2])

    def test_capture_source_and_unknown_observer_masks(self):
        model=(bench.FIXTURE/'model.icl').read_text().replace('CaptureSource a;','CaptureSource ~a;')
        (self.root/'model.icl').write_text(model)
        source='iPDLLevel 1\niProcsForModule MultipleDrTop\niReset\niWrite tap.INSTRUCTION 3; iApply\n'
        source+='iScan jtag 8 -si 0x96 -so 0bxxx00000 -stable; iApply\n'
        source+='iScan jtag 8 -si 0x3c -so 0x76 -stable; iApply\n'
        (self.root/'source.pdl').write_text(source)
        result=self.run_tool('source.pdl','result.pdl')
        self.assertIn(b'-so 0bxxx00000',result)
        self.assertEqual(result,self.run_tool('source.pdl','flat.pdl',True))
        self.assertEqual(result,self.run_tool('result.pdl','generated.pdl'))

    def test_different_physical_capture_retains_symbolic_fallback(self):
        (self.root/'model.icl').write_bytes((bench.FIXTURE/'model.icl').read_bytes())
        (self.root/'source.pdl').write_text('iPDLLevel 1\niProcsForModule MultipleDrTop\niReset\n'
            'iWrite tap.INSTRUCTION 3; iApply\niScan jtag 8 -si 0x96 -so 1; iApply\n')
        result=self.run_tool('source.pdl','result.pdl')
        self.assertIn(b'-so 0b00000001',result)
        self.assertEqual(result,self.run_tool('source.pdl','flat.pdl',True))
        self.assertEqual(result,self.run_tool('result.pdl','generated.pdl'))

    def test_disabled_capture_does_not_fulfill_read(self):
        model=(bench.FIXTURE/'model.icl').read_text().replace('InputPort mode = tap.long_setup;',
            "InputPort mode = tap.long_setup; InputPort capture = 1'b0;")
        (self.root/'model.icl').write_text(model)
        (self.root/'source.pdl').write_text('iPDLLevel 1\niProcsForModule MultipleDrTop\niReset\n'
            'iWrite tap.INSTRUCTION 3; iApply\niRead core.a; iApply\n')
        self.assertIn(b'iRead core.a',self.run_tool('source.pdl','rejected.pdl',ok=False))

    def test_direct_reset_retains_symbolic_fallback(self):
        model=(bench.FIXTURE/'model.icl').read_text().replace('  DataInPort mode;',
            '  DataInPort mode; ResetPort rst;')
        model=model.replace('  ScanInPort TDI;', '  ResetPort reset; ScanInPort TDI;')
        model=model.replace('InputPort mode = tap.long_setup;',
            'InputPort mode = tap.long_setup; InputPort rst = reset;')
        (self.root/'model.icl').write_text(model)
        (self.root/'source.pdl').write_text('iPDLLevel 1\niProcsForModule MultipleDrTop\niReset\n'
            'iWrite tap.INSTRUCTION 3; iApply\niScan jtag 8 -si 0x96 -so 0; iApply\n'
            'iWrite core.a[0] 1; iApply\niRead core.a 5; iRead core.b 22; iApply\n')
        result=self.run_tool('source.pdl','result.pdl')
        self.assertIn(b'iWrite core.a 0b101',result)
        self.assertEqual(result,self.run_tool('source.pdl','flat.pdl',True))
        self.assertEqual(result,self.run_tool('result.pdl','generated.pdl'))


if __name__=='__main__':
    unittest.main()
