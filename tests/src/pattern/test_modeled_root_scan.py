#!/usr/bin/env python3
"""Modeled root iScan state and later ordinary-access replay."""
import unittest

import test_opaque_root_scan as opaque

MODEL = opaque.MODEL.replace('ScanOutPort so;', """ScanRegister value[7:0] {
  ScanInSource si; CaptureSource value; ResetValue 8'b0;
}
ScanOutPort so { Source value[0]; }""")
CHAIN = opaque.MODEL.replace('ScanOutPort so;', """ScanRegister a[0:1] {
  ScanInSource si; CaptureSource a; ResetValue 2'b0;
}
ScanRegister b[2:0] {
  ScanInSource a[1]; CaptureSource b; ResetValue 3'b0;
}
ScanOutPort so { Source b[0]; }""").replace("DefaultLoadValue 8'b00000000", "DefaultLoadValue 5'b0")


class ModeledRootScan(unittest.TestCase):
    # Reuse the existing real CLI replay and atomic publication assertions.
    setUp = opaque.OpaqueRootScan.setUp
    files = opaque.OpaqueRootScan.files
    run_tool = opaque.OpaqueRootScan.run_tool

    def replay(self, commands, model=MODEL):
        return opaque.OpaqueRootScan.replay(self, commands, model)

    def rejected(self, commands, diagnostic, model=MODEL):
        return opaque.OpaqueRootScan.rejected(self, commands, diagnostic, model)

    def test_raw_then_read_write(self):
        result = self.replay('iReset; iScan client 8 -si 0xa5; iApply\n'
                             'iRead value 0xa5; iApply; iWrite value 0x3c; iApply\n'
                             'iScan client 8 -si 0xa5 -so 0x3c -stable; iApply\n')
        self.assertEqual(result.count('iScan '), 2)
        self.assertIn('iRead value 0b10100101', result)
        self.assertIn('iWrite value 0b00111100', result)
        self.assertIn('-so 0b00111100 -stable', result)

    def test_partial_unknown_retains_known_controlling_bits(self):
        result = self.replay('iReset; iScan client 8 -si 0b10x01x01; iApply\n'
                             'iWrite value[1] 1; iApply\n')
        self.assertIn('iWrite value 0b10001011', result)

    def test_unequal_chain_and_ascending_indices(self):
        result = self.replay('iReset; iScan client 5 -si 0b10110; iApply\n'
                             'iRead a 0b10; iRead b 0b110; iApply\n', CHAIN)
        self.assertIn('iRead a 0b10', result)
        self.assertIn('iRead b 0b110', result)

    def test_reset_and_discard_only_replay(self):
        result = self.replay('iReset; iScan client 8 -si 0xa5; iReset; iApply\n'
                             'iScan client 8 -si 0x5a; iApply\n')
        self.assertEqual(result.count('iScan '), 1)
        self.assertIn('-si 0b01011010', result)
        self.replay('iReset; iScan client 8 -si 0xa5; iReset; iApply\n')

    def test_overwrite_and_explicit_capture(self):
        result = self.replay('iReset; iScan client 8 -si 1; iScan client 8 -si 2; iApply\n'
                             'iRead value 2; iApply\n')
        self.assertEqual(result.count('iScan '), 1)
        self.assertIn('iRead value 0b00000010', result)

    def test_invalid_length_and_initial_state(self):
        self.rejected('iScan client 8; iApply\n', 'iReset')
        self.rejected('iReset; iScan client 7; iApply\n', 'length')
        self.rejected('iReset; iScan client 8 -si 256; iReset\n', 'scan')

    def test_mixed_pending_accesses_are_explicit(self):
        for commands in ['iScan client 8; iWrite value 1',
                         'iRead value 1; iScan client 8']:
            with self.subTest(commands=commands):
                self.rejected('iReset; ' + commands + '; iApply\n', 'scan')

    def test_unqualified_topology_and_directives(self):
        for model in [MODEL.replace('ScanRegister value',
                                    "ScanMux routing SelectedBy sel { 1'b0: si; 1'b1: si; } ScanRegister value")
                           .replace('ScanInSource si;', 'ScanInSource routing;'),
                      MODEL.replace('ScanOutPort so { Source value[0]; }',
                                    'ScanOutPort so { Source value[0]; Enable sel; }'),
                      MODEL.replace('ScanOutPort so',
                                    "Alias idle[7:0] = value { iApplyEndState 8'b0; } ScanOutPort so")]:
            with self.subTest(model=model):
                self.rejected('iReset; iScan client 8; iApply\n', 'scan', model)
        for command in ['iTake value', 'iState value 0',
                        'iMerge -begin; iMerge -end', 'iRunLoop 1']:
            with self.subTest(command=command):
                self.rejected('iReset; ' + command + '; iScan client 8; iApply\n', 'scan')

    def test_serial_input_alias_polarity(self):
        cases = [
            (MODEL.replace('ScanInSource si;', 'ScanInSource ~si;'), '01011010', '11011010'),
            (MODEL.replace('ScanInPort si;', 'ScanInPort si; Alias input = ~si;')
                  .replace('ScanInSource si;', 'ScanInSource input;'), '01011010', '11011010'),
            (MODEL.replace('ScanInPort si;',
                           'ScanInPort si; Alias first = ~si; Alias input = ~first;')
                  .replace('ScanInSource si;', 'ScanInSource input;'), '10100101', '10100101')]
        for model, captured, written in cases:
            with self.subTest(model=model):
                result = self.replay('iReset; iScan client 8 -si 0xa5; iApply\n'
                                     'iRead value 0b' + captured + '; iApply\n'
                                     'iWrite value[7] 1; iApply\n', model)
                self.assertIn('iScan client 8 -si 0b10100101', result)
                self.assertIn('iRead value 0b' + captured, result)
                self.assertIn('iWrite value 0b' + written, result)

    def test_serial_output_alias_ordering(self):
        cases = [
            MODEL.replace('Source value[0]', 'Source ~value[0]'),
            MODEL.replace('ScanOutPort so', 'Alias output[7:0] = ~value; ScanOutPort so')
                 .replace('Source value[0]', 'Source output[0]'),
            MODEL.replace('ScanOutPort so', 'Alias output[0:7] = value; ScanOutPort so')
                 .replace('Source value[0]', 'Source output[7]'),
            MODEL.replace('ScanOutPort so',
                          'Alias output[1:0] = value[0], value[7]; ScanOutPort so')
                 .replace('Source value[0]', 'Source output[1]')]
        for model in cases:
            with self.subTest(model=model):
                result = self.replay('iReset; iScan client 8 -si 0xa5 -so 0xXX; iApply\n'
                                     'iRead value 0xa5; iApply; iWrite value[7] 0; iApply\n', model)
                self.assertIn('iScan client 8 -si 0b10100101 -so 0bxxxxxxxx', result)
                self.assertIn('iWrite value 0b00100101', result)

    def test_serial_midchain_parity(self):
        inverted = CHAIN.replace('ScanInSource a[1];', 'ScanInSource ~a[1];')
        aliased = CHAIN.replace('ScanRegister b', 'Alias bridge = ~a[1]; ScanRegister b')
        aliased = aliased.replace('ScanInSource a[1];', 'ScanInSource bridge;')
        cases = [(inverted, '10', '001'), (aliased, '10', '001'),
                 (inverted.replace('Source b[0]', 'Source ~b[0]'), '10', '001'),
                 (inverted.replace('ScanInSource si;', 'ScanInSource ~si;')
                          .replace('Source b[0]', 'Source ~b[0]'), '01', '110')]
        for model, a, b in cases:
            with self.subTest(model=model):
                result = self.replay('iReset; iScan client 5 -si 0b10110; iApply\n'
                                     'iRead a 0b' + a + '; iRead b 0b' + b + '; iApply\n', model)
                self.assertIn('iRead a 0b' + a, result)
                self.assertIn('iRead b 0b' + b, result)

    def test_serial_alias_partial_unknown_fill(self):
        model = MODEL.replace('ScanInPort si;', 'ScanInPort si; Alias input = ~si;')
        model = model.replace('ScanInSource si;', 'ScanInSource input;')
        result = self.replay('iReset; iScan client 8 -si 0b10x01x01; iApply\n'
                             'iWrite value[0] 1; iApply\n', model)
        self.assertIn('iScan client 8 -si 0b10x01x01', result)
        self.assertIn('iWrite value 0b01010011', result)

    def test_serial_alias_wrong_tap_atomic(self):
        model = MODEL.replace('ScanOutPort so', 'Alias output[0:7] = value; ScanOutPort so')
        model = model.replace('Source value[0]', 'Source output[0]')
        self.rejected('iReset; iScan client 8; iApply\n', 'rightmost', model)

    def test_large_modeled_scan(self):
        width = 1048576
        model = MODEL.replace('[7:0]', '[' + str(width - 1) + ':0]')
        model = model.replace("ResetValue 8'b0", "ResetValue " + str(width) + "'b0")
        model = model.replace("DefaultLoadValue 8'b00000000", "DefaultLoadValue " + str(width) + "'b0")
        result = self.replay('iReset; iScan client ' + str(width) + ' -si 0x' +
                             'a5' * (width // 8) + ' -stable; iApply\n', model)
        self.assertIn('-si 0b' + '10100101' * (width // 8), result)


if __name__ == '__main__':
    unittest.main()
