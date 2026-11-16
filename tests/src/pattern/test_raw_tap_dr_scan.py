#!/usr/bin/env python3
"""Known-binary raw DR through one selected modeled TAP register."""
import pathlib
import test_raw_tap_ir_scan as ir
import unittest

FIXTURE = ir.tap.ROOT / 'tests/cases/ours/pattern_generation/raw_tap_dr'

class RawTapDrScan(unittest.TestCase):
    run_tool = ir.RawTapIrScan.run_tool
    rejected = ir.RawTapIrScan.rejected
    def setUp(self):
        ir.RawTapIrScan.setUp(self)
        self.inputs = ['--icl', str(FIXTURE / 'model.icl'), '--bsdl',
                       'RawDrProbe', str(FIXTURE / 'model.bsdl')]

    def source(self, body):
        path = self.root / 'source.pdl'
        path.write_text('iPDLLevel 1\niProcsForModule RawDrTop\n' + body)
        return path

    def replay(self, body, inputs=None):
        source = self.source(body)
        text = self.run_tool(source, inputs=inputs, extra=('--debug', '--local_reset', 'reset.pdl'))
        self.assertEqual(text, self.run_tool(self.root / 'result.pdl', 'generated.pdl', inputs=inputs))
        flat = ['--icl', 'debug/RawDrTop.icl']
        self.assertEqual(text, self.run_tool(source, 'flat.pdl', inputs=flat))
        self.assertEqual(text, self.run_tool(self.root / 'result.pdl', 'flat-generated.pdl', inputs=flat))
        self.assertEqual(text, self.run_tool(self.root / 'reset.pdl', 'reset-replay.pdl', inputs=inputs))
        return text

    def test_data_and_instruction_transitions_replay(self):
        result = self.replay('iReset\n' + (FIXTURE / 'source.pdl').read_text().split('iReset\n',1)[1])
        self.assertIn('iScan jtag 8 -si 0b10010110 -so 0b00000000', result)
        self.assertIn('iScan jtag 8 -si 0b00111100 -so 0b10010110', result)

    def test_ordinary_preparation_queue_orders_overwrite_and_cancel(self):
        prepare = 'iWrite core.r 0x55; iRead core.r 0; '
        raw = 'iScan jtag 8 -si 0x96 -so 0x55; '
        reference = self.replay('iReset; '+prepare+'iApply; '+raw+'iApply\n')
        for body in [prepare+raw, raw+prepare, 'iScan jtag 9; '+raw+prepare]:
            self.assertEqual(self.replay('iReset; '+body+'iApply\n'), reference)
        self.replay('iScan jtag 9; iReset; iApply\n')
        self.rejected('iReset; iScan jtag 8; iApply\n','length')
        self.rejected('iScan jtag 8; iApply\n','iReset')

    def test_owner_partial_and_observer_history(self):
        result=self.replay('iReset; iTake core.r; iWrite core.r 0x55; iApply; '
            'iScan jtag 8 -si 0x96 -so 0xbf -stable; iApply; '
            'iWrite core.r[0] 1; iApply; iRead core.r 0x97; iApply\n')
        self.assertIn('-si 0b10010110 -so 0b10111111 -stable',result)
        self.assertIn('iWrite core.r 0b10010111',result)

    def test_implicit_default_chain_calls_and_mixed_kinds(self):
        model=self.root/'implicit.icl'
        model.write_text((FIXTURE/'model.icl').read_text().replace(
          '  ScanInterface jtag {Port TDI; Port TDO; Port TMS; Port TCK; Port TRST;}\n',''))
        inputs=self.inputs.copy();inputs[1]=str(model)
        self.replay('iProc load {} {iScan unnamed -chain default 8 -si 0x96 -so 0}\n'
                    'iReset; iWrite tap.INSTRUCTION 3; iApply; iCall load; iApply\n',inputs)
        for a,b in [('iScan -ir jtag 4 -si 3','iScan jtag 8 -si 0x96'),
                    ('iScan jtag 8 -si 0x96','iScan -ir jtag 4 -si 3')]:
            self.rejected('iReset; iWrite tap.INSTRUCTION 3; iApply; '+a+'; '+b+'; iApply\n','mixed raw IR and data')

    def test_multiple_unknown_and_stable_post_route_reject_atomically(self):
        model=self.root/'multiple.icl'
        original=(FIXTURE/'model.icl').read_text()
        model.write_text(original.replace('  ScanMux mux',
            "  ScanRegister tail[3:0] {ScanInSource r[0]; ResetValue 4'b0;}\n  ScanMux mux").replace("1'b1: r[0]", "1'b1: tail[0]"))
        previous=self.inputs.copy();self.inputs[1]=str(model)
        self.rejected('iReset; iWrite tap.INSTRUCTION 3; iApply; iScan jtag 12 -si 1; iApply\n',
                      'one fully selected modeled register')
        self.inputs=previous
        model.write_text(original.replace('ScanInPort TDI;', 'DataInPort route; ScanInPort TDI;').replace(
            'InputPort mode = tap.long_setup;', 'InputPort mode = route;'))
        self.inputs[1]=str(model)
        self.rejected('iReset; iWrite tap.INSTRUCTION 3; iApply; iScan jtag 8; iApply\n',
                      'one fully selected modeled register')
        dynamic=original.replace('ScanInSource si; CaptureSource r; ResetValue 8', 'ScanInSource feedback; CaptureSource r; ResetValue 8')
        dynamic=dynamic.replace("  ScanMux mux SelectedBy mode { 1'b1: r[0]; 1'b0: si; }",
            "  ScanRegister extra {ScanInSource si; ResetValue 1'b0;}\n"
            "  ScanMux feedback SelectedBy r[0] {1'b0: si; 1'b1: extra;}\n"
            "  ScanMux mux SelectedBy mode {1'b1: r[0]; 1'b0: si;}")
        model.write_text(dynamic);self.inputs[1]=str(model)
        self.rejected('iReset; iWrite tap.INSTRUCTION 3; iApply; '
                      'iScan jtag 8 -si 0x97 -stable; iApply\n', 'stable')
        self.replay('iReset; iWrite tap.INSTRUCTION 3; iApply; '
                    'iScan jtag 8 -si 0x97; iApply; iRead core.extra 0; iApply\n')

    def test_stable_rejects_disconnected_alternate_and_inverted_post_input(self):
        original=(FIXTURE/'model.icl').read_text()
        for endpoint in ["1'b0", 'ALT', '~si']:
            with self.subTest(endpoint=endpoint):
                model=self.root/'post-route.icl'
                text=original.replace('ScanInSource si; CaptureSource r;',
                                      'ScanInSource feedback; CaptureSource r;')
                text=text.replace('  ScanMux mux',
                    "  ScanMux feedback SelectedBy r[0] {1'b0: si; 1'b1: "+endpoint+";}\n  ScanMux mux")
                if endpoint=='ALT':
                    text=text.replace('  ScanInPort si;', '  ScanInPort si; ScanInPort ALT;')
                    # A second child input is driven by a distinct physical input.
                    text=text.replace('ScanInPort TDI;', 'ScanInPort OTHER; ScanInPort TDI;')
                    text=text.replace('InputPort mode = tap.long_setup;',
                                      'InputPort mode = tap.long_setup; InputPort ALT = OTHER;')
                model.write_text(text)
                self.inputs[1]=str(model)
                self.rejected('iReset; iWrite tap.INSTRUCTION 3; iApply; '
                              'iScan jtag 8 -si 0x97 -stable; iApply\n', 'stable')

    def test_builtin_registers_keep_exact_current_width(self):
        self.rejected('iReset; iScan jtag 31; iApply\n', 'length')
        self.rejected('iReset; iScan -ir jtag 4 -si 15; iApply; iScan jtag 2; iApply\n', 'length')

    def test_malformed_data_remains_early(self):
        for command,diagnostic in [('iScan jtag 8 -si 0bx001','known'),
                ('iScan core.client 8','client-TAP'),
                ('iScan jtag -chain other 8','chain'),
                ('iScan jtag 0','length'),('iScan jtag 8 -si 256','scan')]:
            self.rejected(command+'; iReset\n',diagnostic)

if __name__ == '__main__':
    unittest.main()
