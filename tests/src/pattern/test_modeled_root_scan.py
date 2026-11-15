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

HIER = (opaque.ROOT / 'tests/cases/ours/pattern_generation/hierarchical_root_scan/model.icl').read_text()

CHILD = (opaque.ROOT / 'tests/cases/ours/pattern_generation/static_child_scans/model.icl').read_text()


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

    def test_hierarchical_root_state_and_procedure(self):
        commands = ("iProcsForModule Leaf\n"
                    "iProc check {expected} { iRead value $expected; iApply }\n"
                    "iProcsForModule Top\n"
                    "iReset; iScan client 8 -si 0xa5; iApply\n"
                    "iCall child.check 0xa5; iWrite child.value[7] 0; iApply\n"
                    "iRead child.value 0x25; iApply\n")
        result = self.replay(commands, HIER)
        self.assertIn('iWrite child.value 0b00100101', result)
        self.assertIn('iRead child.value 0b10100101', result)

    def test_hierarchical_nested_and_implicit_client(self):
        leaf, root = HIER.split('Module Top ', 1)
        wrapper = 'Module Wrapper ' + root.replace('Instance child Of Leaf', 'Instance inner Of Leaf')
        wrapper = wrapper.replace('Source child.so', 'Source inner.so')
        nested = leaf + wrapper + 'Module Top ' + root.replace('Instance child Of Leaf', 'Instance child Of Wrapper')
        result = self.replay('iReset; iScan client 8 -si 0xa5; iApply\n'
                             'iRead child.inner.value 0xa5; iApply\n', nested)
        self.assertIn('iRead child.inner.value 0b10100101', result)
        implicit = HIER.replace(' ScanInterface local { Port si; Port so; Port sel; Port ce; Port se; Port ue; Port tck; Port reset; }\n', '')
        self.assertIn('iScan client', self.replay('iReset; iScan client 8 -si 0xa5; iApply\n', implicit))

    def test_hierarchical_repeated_parameterized_leaf(self):
        parameterized = HIER.replace('Module Leaf {', 'Module Leaf { Parameter W = 8;')
        parameterized = parameterized.replace('value[7:0]', 'value[$W-1:0]')
        parameterized = parameterized.replace("ResetValue 8'b0", "ResetValue 0")
        parameterized = parameterized.replace('Instance child Of Leaf {', 'Instance child Of Leaf { Parameter W = 12;')
        parameterized = parameterized.replace("DefaultLoadValue 8'b0", "DefaultLoadValue 12'b0")
        result = self.replay('iReset; iScan client 12 -si 0xa55; iApply\n'
                             'iRead child.value 0xa55; iApply\n', parameterized)
        self.assertIn('iRead child.value 0b101001010101', result)
        start, end = HIER.index(' Instance child Of Leaf'), HIER.index(' ScanOutPort so { Source child.so; }')
        block = HIER[start:end]
        repeated = HIER[:start] + block.replace('Instance child', 'Instance first')
        repeated += block.replace('Instance child', 'Instance second').replace('InputPort si = si;', 'InputPort si = first.so;') + HIER[end:]
        repeated = repeated.replace('Source child.so', 'Source second.so').replace("DefaultLoadValue 8'b0", "DefaultLoadValue 16'b0")
        result = self.replay('iReset; iScan client 16 -si 0xa55a; iApply\n'
                             'iWrite first.value[7] 0; iApply\n'
                             'iRead first.value 0x25; iRead second.value 0x5a; iApply\n', repeated)
        self.assertIn('iWrite first.value 0b00100101', result)
        self.assertIn('iRead second.value 0b01011010', result)

    def test_hierarchical_binding_polarity_and_active_low_reset(self):
        model = HIER.replace('InputPort si = si;', 'InputPort si = ~si;')
        result = self.replay('iReset; iScan client 8 -si 0xa5; iApply\n'
                             'iWrite child.value[7] 1; iApply\n', model)
        self.assertIn('iWrite child.value 0b11011010', result)
        active_low = HIER.replace('ResetPort reset;', 'ResetPort reset { ActivePolarity 0; }')
        self.assertIn('iScan client', self.replay('iReset; iScan client 8 -si 0xa5; iApply\n', active_low))
        driver = 'Module Driver { TCKPort tck; ToTCKPort forwarded { Source tck; } }\n'
        forwarded = driver + HIER.replace(' Instance child Of Leaf',
                                          ' Instance driver Of Driver { InputPort tck = tck; }\n Instance child Of Leaf')
        forwarded = forwarded.replace('InputPort tck = tck; InputPort reset',
                                      'InputPort tck = driver.forwarded; InputPort reset')
        self.assertIn('iScan client', self.replay('iReset; iScan client 8 -si 0xa5; iApply\n', forwarded))

    def test_hierarchical_declared_unbound_inputs_rejected_before_reset(self):
        for name in ['tck', 'ce', 'se', 'ue', 'sel']:
            with self.subTest(name=name):
                model = HIER.replace('InputPort ' + name + ' = ' + name + ';', '')
                self.rejected('iScan client 8; iReset\n', 'scan', model)
        implicit_reset = HIER.replace('InputPort reset = reset;', '')
        self.assertIn('iScan client', self.replay(
            'iReset; iScan client 8 -si 0xa5; iApply\n', implicit_reset))
        for name in ['tck', 'ce', 'se', 'ue', 'sel']:
            with self.subTest(inverted=name):
                model = HIER.replace('InputPort ' + name + ' = ' + name + ';',
                                     'InputPort ' + name + ' = ~' + name + ';')
                self.rejected('iScan client 8; iReset\n',
                              'scan' if name == 'sel' else 'non-inverted', model)
        foreign = HIER.replace('Module Top {', 'Module Top { TCKPort other;')
        foreign = foreign.replace('InputPort tck = tck;', 'InputPort tck = other;')
        self.rejected('iReset; iScan client 8; iApply\n', 'interface', foreign)

    def test_child_scan_preserves_surrounding_controlling_values(self):
        result = self.replay('iReset; iWrite head 5; iWrite tail 0x13; iApply\n'
                             'iScan child.local 8 -si 0xa5 -so 0x3c -stable; iApply\n'
                             'iRead head 5; iRead tail 0x13; iRead child.value 0xa5; iApply\n', CHILD)
        self.assertIn('iScan client 16 -si 0b1011010010110011 -so 0bxxx00111100xxxxx -stable', result)
        self.assertIn('iRead child.value 0b10100101', result)

    def test_child_whole_chain_prefix_procedure_and_partial_x(self):
        commands = ("iProcsForModule Leaf\n"
                    "iProc raw {} { iScan local 8 -si 0xa5; iApply }\n"
                    "iProcsForModule Top\niReset; iCall child.raw\n"
                    "iPrefix child; iScan local 8 -si 0b10x01x01; iApply\n"
                    "iWrite value[1] 1; iApply\n")
        result = self.replay(commands, HIER)
        self.assertIn('iScan client 8 -si 0b10100101', result)
        self.assertIn('iWrite child.value 0b10101111', result)

    def test_child_disjoint_queue_and_same_target_overwrite(self):
        start, end = HIER.index(' Instance child Of Leaf'), HIER.index(' ScanOutPort so { Source child.so; }')
        block = HIER[start:end]
        model = HIER[:start] + block.replace('Instance child', 'Instance first')
        model += block.replace('Instance child', 'Instance second').replace('InputPort si = si;', 'InputPort si = first.so;') + HIER[end:]
        model = model.replace('Source child.so', 'Source second.so').replace("DefaultLoadValue 8'b0", "DefaultLoadValue 16'b0")
        result = self.replay('iReset; iScan first.local 8 -si 0xa5 -so 0x12 -stable\n'
                             'iScan second.local 8 -si 0x5a -so 0x34 -stable\n'
                             'iScan first.local 8 -si 0x3c -so 0x56 -stable; iApply\n'
                             'iRead first.value 0x3c; iRead second.value 0x5a; iApply\n', model)
        self.assertIn('iScan client 16 -si 0b0011110001011010 -so 0b0101011000110100 -stable', result)
        self.assertEqual(result.count('iScan '), 1)

    def test_child_local_load_and_root_comparison_polarity(self):
        model = CHILD.replace('InputPort si = head[0];', 'InputPort si = ~head[0];')
        model = model.replace('Source value[0]', 'Source ~value[0]')
        for output_inverse in [False, True]:
            with self.subTest(output_inverse=output_inverse):
                icl = model.replace('Source tail[0]', 'Source ~tail[0]') if output_inverse else model
                expected = '11000011' if output_inverse else '00111100'
                result = self.replay('iReset; iWrite head 5; iWrite tail 0x13; iApply\n'
                                     'iScan child.local 8 -si 0xa5 -so 0x3c; iApply\n'
                                     'iRead child.value 0xa5; iApply\n', icl)
                self.assertIn('iScan client 16 -si 0b1010101101010011 -so 0bxxx' + expected + 'xxxxx', result)

    def test_child_invalid_discarded_and_overlap_atomic(self):
        for target, width in [('missing', 8), ('child.value', 8), ('child.local', 7)]:
            with self.subTest(target=target, width=width):
                self.rejected('iScan ' + target + ' ' + str(width) + '; iReset\n', 'scan', CHILD)
        self.rejected('iReset; iScan client 16; iScan child.local 8; iApply\n', 'overlap', CHILD)
        self.replay('iReset; iScan child.local 8; iReset; iApply\n', CHILD)

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
