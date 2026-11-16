#!/usr/bin/env python3
"""Known-binary raw IR transactions through a bound root client-TAP."""
import pathlib
import tempfile
import unittest

import test_tap_pdl_replay as tap

FIXTURE = tap.ROOT / 'tests/cases/ours/pattern_generation/raw_tap_ir'


class RawTapIrScan(unittest.TestCase):
    run_tool = tap.TapPdlReplay.run_tool

    def setUp(self):
        temp = tempfile.TemporaryDirectory()
        self.addCleanup(temp.cleanup)
        self.root = pathlib.Path(temp.name)
        self.inputs = ['--icl', str(FIXTURE / 'model.icl'), '--bsdl',
                       'RawIrProbe', str(FIXTURE / 'model.bsdl')]

    def source(self, body):
        path = self.root / 'source.pdl'
        path.write_text('iPDLLevel 1\niProcsForModule RawIrTop\n' + body)
        return path

    def replay(self, body, inputs=None):
        source = self.source(body)
        text = self.run_tool(source, inputs=inputs, extra=('--debug', '--local_reset', 'reset.pdl'))
        self.assertEqual(text, self.run_tool(self.root / 'result.pdl', 'generated.pdl', inputs=inputs))
        flat = ['--icl', 'debug/RawIrTop.icl']
        self.assertEqual(text, self.run_tool(source, 'flat.pdl', inputs=flat))
        self.assertEqual(text, self.run_tool(self.root / 'result.pdl', 'flat-generated.pdl', inputs=flat))
        self.assertEqual(text, self.run_tool(self.root / 'reset.pdl', 'reset-replay.pdl', inputs=inputs))
        return text

    def rejected(self, body, diagnostic):
        target = self.root / 'result.pdl'
        target.write_text('retained pattern\n')
        message = self.run_tool(self.source(body), ok=False)
        self.assertIn(diagnostic, message)
        self.assertEqual(target.read_text(), 'retained pattern\n')

    def test_explicit_raw_ir_and_following_data_replay(self):
        result = self.replay('iReset\n' + (FIXTURE / 'source.pdl').read_text().split('iReset\n', 1)[1])
        self.assertIn('iScan -ir jtag 4 -si 0b0011 -so 0bxx01', result)
        self.assertIn('iScan -ir jtag 4 -si 0b0100 -so 0bxx01', result)
        self.assertIn('iRead core.r 0b10100101', result)

    def test_inferred_interface_and_default_chain(self):
        model = self.root / 'implicit.icl'
        model.write_text((FIXTURE / 'model.icl').read_text().replace(
            '  ScanInterface jtag {Port TDI; Port TDO; Port TMS; Port TCK; Port TRST;}\n', ''))
        inputs = self.inputs.copy()
        inputs[1] = str(model)
        result = self.replay('iReset; iScan -ir unnamed -chain default 4 -si 3 -so 0bxx01 -stable; iApply\n', inputs)
        self.assertIn('iScan -ir unnamed 4 -si 0b0011 -so 0bxx01 -stable', result)
        self.assertNotIn('-chain', result)

    def test_raw_ir_queue_overwrite_ordinary_order_and_cancel(self):
        prepare = 'iWrite core.r 5; iRead core.r 0; '
        raw = 'iScan -ir jtag 4 -si 4 -so 0bxx01; '
        reference = self.replay('iReset; ' + prepare + 'iApply; ' + raw + 'iApply\n')
        for body in [prepare + raw, raw + prepare,
                     'iScan -ir jtag 5; ' + raw + prepare]:
            with self.subTest(body=body):
                self.assertEqual(self.replay('iReset; ' + body + 'iApply\n'), reference)
        self.replay('iScan -ir jtag 5; iReset; iApply\n')
        self.rejected('iReset; iScan -ir jtag 5; iApply\n', 'length')
        self.rejected('iScan -ir jtag 4; iApply\n', 'iReset')

    def test_raw_ir_observer_and_owned_partial_history(self):
        result = self.replay('iReset; iTake tap.INSTRUCTION; iWrite tap.INSTRUCTION 3; iApply; '
                             'iScan -ir jtag 4 -si 4 -so 0b1011; iApply; '
                             'iWrite tap.INSTRUCTION[0] 1; iApply\n')
        self.assertIn('-si 0b0100 -so 0b1011', result)
        self.assertIn('iWrite tap.INSTRUCTION 0b0101', result)

    def test_raw_ir_scope_binding_and_calls(self):
        self.rejected('iScan -ir core.client 4; iReset\n', 'client-TAP')
        implicit = self.root / 'unbound.icl'
        implicit.write_text((FIXTURE / 'model.icl').read_text())
        self.inputs = ['--icl', str(implicit)]
        self.rejected('iScan -ir jtag 4; iReset\n', 'BSDL')
        self.inputs = ['--icl', str(FIXTURE / 'model.icl'), '--bsdl',
                       'RawIrProbe', str(FIXTURE / 'model.bsdl')]
        self.replay('iProc load {} {iScan -ir jtag 4 -si 3 -so 1}\n'
                    'iReset; iCall load; iApply; iRunLoop 1 -tck; '
                    'iRead tap.INSTRUCTION 1; iApply\n')

    def test_raw_ir_rejects_multiple_bound_taps(self):
        from test_multiple_tap_scheduling import network
        model, bsdl = network()
        icl = self.root / 'multiple.icl'
        models = self.root / 'multiple.bsdl'
        icl.write_text(model)
        models.write_text(bsdl)
        source = self.root / 'multiple.pdl'
        source.write_text('iProcsForModule MultiTapTop\niScan -ir unnamed 4; iReset\n')
        message = self.run_tool(source, ok=False, inputs=[
            '--icl', str(icl), '--bsdl', 'Probe0', str(models),
            '--bsdl', 'Probe1', str(models)])
        self.assertIn('one bound scalar root client-TAP', message)

    def test_raw_ir_malformed_and_scope_remain_early(self):
        for command, diagnostic in [
                ('iScan -ir missing 4', 'client'),
                ('iScan -ir jtag 4 -si 16', 'scan'),
                ('iScan -ir jtag 4 -si 0bx001', 'known'),
                ('iScan -ir jtag -chain other 4', 'chain'),
                ('iScan -ir jtag 0', 'length'),
                ('iScan -ir jtag 1048577', 'length'),
                ('iScan jtag 4 -ir', 'invalid or incomplete option')]:
            with self.subTest(command=command):
                self.rejected(command + '; iReset\n', diagnostic)
        self.rejected('iReset; iScan jtag 4; iApply\n', 'raw scan')
        self.rejected('iReset; iScan -ir jtag 4; iWrite core.r 1; iApply -together\n', 'together')


if __name__ == '__main__':
    unittest.main()
