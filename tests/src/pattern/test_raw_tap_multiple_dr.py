#!/usr/bin/env python3
"""Ordered multi-register raw TAP data with physical serial parity."""
import unittest
import test_raw_tap_dr_scan as dr

FIXTURE=dr.ir.tap.ROOT/'tests/cases/ours/pattern_generation/raw_tap_multiple_dr'

class RawTapMultipleDr(unittest.TestCase):
    run_tool=dr.RawTapDrScan.run_tool
    rejected=dr.RawTapDrScan.rejected

    def setUp(self):
        dr.RawTapDrScan.setUp(self)
        self.inputs=['--icl',str(FIXTURE/'model.icl'),'--bsdl','MultipleDrProbe',str(FIXTURE/'model.bsdl')]

    def source(self,body):
        path=self.root/'source.pdl'
        path.write_text('iPDLLevel 1\niProcsForModule MultipleDrTop\n'+body)
        return path

    def replay(self,body,inputs=None):
        source=self.source(body)
        text=self.run_tool(source,inputs=inputs,extra=('--debug','--local_reset','reset.pdl'))
        self.assertEqual(text,self.run_tool(self.root/'result.pdl','generated.pdl',inputs=inputs))
        flat=['--icl','debug/MultipleDrTop.icl']
        self.assertEqual(text,self.run_tool(source,'flat.pdl',inputs=flat))
        self.assertEqual(text,self.run_tool(self.root/'result.pdl','flat-generated.pdl',inputs=flat))
        self.assertEqual(text,self.run_tool(self.root/'reset.pdl','reset-replay.pdl',inputs=inputs))
        return text

    def test_unequal_chain_and_partial_retained_history(self):
        result=self.replay('iReset\n'+(FIXTURE/'source.pdl').read_text().split('iReset\n',1)[1])
        self.assertIn('iScan jtag 8 -si 0b10010110 -so 0b00000000',result)
        self.assertIn('iWrite core.a 0b101',result)
        self.assertIn('-si 0b00111100 -so 0b10110110',result)

    def test_ordinary_ir_preparation_orders_and_owner_refresh(self):
        prepare='iWrite tap.INSTRUCTION 3; '
        raw='iScan jtag 8 -si 0x96 -so 0; '
        reference=self.replay('iReset; '+prepare+'iApply; '+raw+'iApply\n')
        for body in [prepare+raw,raw+prepare,'iScan jtag 9; '+raw+prepare]:
            self.assertEqual(self.replay('iReset; '+body+'iApply\n'),reference)
        result=self.replay('iReset; iWrite tap.INSTRUCTION 3; iApply; iTake core.a; iTake core.b; '
            'iWrite core.a 0; iWrite core.b 0; iApply; iScan jtag 8 -si 0x96 -so 0 -stable; iApply; '
            'iWrite core.a[0] 1; iApply\n')
        self.assertIn('iWrite core.a 0b101',result)
        self.replay('iScan jtag 9; iReset; iApply\n')

    def test_internal_parity_and_ascending_indices(self):
        for flags in range(8):
            for ascending in (False,True):
                with self.subTest(flags=flags,ascending=ascending):
                    si,between,so=bool(flags&1),bool(flags&2),bool(flags&4)
                    model=(FIXTURE/'model.icl').read_text()
                    model=model.replace('ScanInSource si; CaptureSource a;',
                        'ScanInSource '+('~si' if si else 'si')+'; CaptureSource a;')
                    model=model.replace('ScanInSource a[0];','ScanInSource '+('~' if between else '')+'a[0];')
                    model=model.replace('Source mux;', 'Source '+('~' if so else '')+'mux;')
                    if ascending:
                        model=model.replace('a[2:0]','a[0:2]').replace('a[0];','a[2];')
                        model=model.replace('b[4:0]','b[0:4]').replace('b[0];','b[4];')
                    path=self.root/'parity.icl';path.write_text(model)
                    self.inputs[1]=str(path)
                    a=4^(7 if si else 0);b=22^(31 if si^between else 0)
                    expected=(0xe0 if between^so else 0)|(0x1f if so else 0)
                    result=self.replay(f'iReset; iWrite tap.INSTRUCTION 3; iApply; '
                        f'iScan jtag 8 -si 0x96 -so {expected} -stable; iApply; '
                        f'iRead core.a {a}; iRead core.b {b}; iApply\n')
                    self.assertIn('iRead core.a 0b'+format(a,'03b'),result)
                    self.assertIn('iRead core.b 0b'+format(b,'05b'),result)

    def test_stable_dynamic_inter_register_parity_and_partial_fill(self):
        model=(FIXTURE/'model.icl').read_text().replace('ScanInSource a[0];','ScanInSource routing;')
        model=model.replace('  ScanRegister b',
            "  ScanMux routing SelectedBy b[0] {1'b0: a[0]; 1'b1: ~a[0];}\n  ScanRegister b")
        path=self.root/'dynamic.icl';path.write_text(model);self.inputs[1]=str(path)
        result=self.replay('iReset; iWrite tap.INSTRUCTION 3; iApply; '
            'iScan jtag 8 -si 0x97 -so 0 -stable; iApply; '
            'iScan jtag 8 -si 0x69 -so 0x77 -stable; iApply; '
            'iWrite core.a[0] 0; iApply; iRead core.a 2; iRead core.b 22; iApply\n')
        self.assertIn('iWrite core.a 0b010',result)

if __name__=='__main__':
    unittest.main()
