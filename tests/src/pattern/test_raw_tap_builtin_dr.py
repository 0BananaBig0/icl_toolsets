#!/usr/bin/env python3
"""Raw builtin DR may use internal models without exposing synthetic PDL targets."""
import unittest
import test_raw_tap_dr_scan as dr


class RawTapBuiltinDr(unittest.TestCase):
    def setUp(self):
        dr.RawTapDrScan.setUp(self)
        fixture = dr.ir.tap.ROOT / 'tests/cases/ours/pattern_generation/raw_tap_builtin_dr'
        self.inputs = ['--icl', str(fixture / 'model.icl'), '--bsdl',
                       'BuiltinDrProbe', str(fixture / 'model.bsdl')]

    def source(self, body):
        path = self.root / 'source.pdl'
        path.write_text('iPDLLevel 1\niProcsForModule BuiltinDrTop\n' + body)
        return path

    def replay(self, body, inputs=None):
        source = self.source(body)
        text = self.run_tool(source, inputs=inputs, extra=('--debug', '--local_reset', 'reset.pdl'))
        self.assertEqual(text, self.run_tool(self.root / 'result.pdl', 'generated.pdl', inputs=inputs))
        flat = ['--icl', 'debug/BuiltinDrTop.icl']
        self.assertEqual(text, self.run_tool(source, 'flat.pdl', inputs=flat))
        self.assertEqual(text, self.run_tool(self.root / 'result.pdl', 'flat-generated.pdl', inputs=flat))
        self.assertEqual(text, self.run_tool(self.root / 'reset.pdl', 'reset-replay.pdl', inputs=inputs))
        return text
    run_tool = dr.RawTapDrScan.run_tool
    rejected = dr.RawTapDrScan.rejected

    def test_reset_idcode_and_selected_bypass_replay(self):
        result = self.replay('iReset; iScan jtag 32 -si 0x89abcdef -so 0x10000003 -stable; iApply; '
                             'iScan -ir jtag 4 -si 15 -so 1; iApply; '
                             'iScan jtag 1 -si 1 -so 0 -stable; iApply; iRunLoop 1 -tck; '
                             'iScan jtag 1 -si 0 -so 0; iApply\n')
        self.assertIn('iScan jtag 32 -si 0b10001001101010111100110111101111', result)
        self.assertIn('iScan jtag 1 -si 0b1 -so 0b0 -stable', result)
        self.assertNotIn('iWrite tap.DEVICE_ID', result)
        self.assertNotIn('iWrite tap.BYPASS', result)

    def test_ordinary_ir_preparation_orders_cancel_and_width(self):
        prepare = 'iWrite tap.INSTRUCTION 15; '
        raw = 'iScan jtag 1 -si 1 -so 0; '
        reference = self.replay('iReset; ' + prepare + 'iApply; ' + raw + 'iApply\n')
        for body in [prepare + raw, raw + prepare, 'iScan jtag 9; ' + raw + prepare]:
            self.assertEqual(self.replay('iReset; ' + body + 'iApply\n'), reference)
        self.replay('iScan jtag 65; iReset; iApply\n')
        self.rejected('iReset; iScan jtag 31; iApply\n', 'length')
        self.rejected('iScan jtag 32; iApply\n', 'iReset')

    def test_synthetic_source_access_stays_forbidden_before_and_after_raw(self):
        for prefix in ['iReset; ', 'iReset; iScan jtag 32 -si 0; iApply; ']:
            for command in ['iWrite tap.DEVICE_ID 0', 'iRead tap.DEVICE_ID 0',
                            'iWrite tap.BYPASS 1', 'iRead tap.BYPASS 0']:
                with self.subTest(prefix=prefix, command=command):
                    self.rejected(prefix + command + '; iApply\n',
                                  'synthetic TAP data registers are not PDL targets')

    def test_owned_ir_and_inferred_default_chain(self):
        self.replay('iReset; iTake tap.INSTRUCTION; iWrite tap.INSTRUCTION 15; iApply; '
                    'iScan jtag 1 -si 1 -so 0; iApply; '
                    'iWrite tap.INSTRUCTION[0] 0; iApply\n')
        model = self.root / 'implicit.icl'
        model.write_text((dr.ir.tap.ROOT / 'tests/cases/ours/pattern_generation/raw_tap_builtin_dr/model.icl').read_text().replace(
            '  ScanInterface jtag {Port TDI; Port TDO; Port TMS; Port TCK; Port TRST;}\n', ''))
        inputs = self.inputs.copy()
        inputs[1] = str(model)
        result = self.replay('iReset; iScan unnamed -chain default 32 -si 0 -so 0x10000003; iApply\n', inputs)
        self.assertIn('iScan unnamed 32 -si 0b' + '0' * 32, result)
        self.assertNotIn('-chain', result)


if __name__ == '__main__':
    unittest.main()
