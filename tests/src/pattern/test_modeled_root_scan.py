#!/usr/bin/env python3
"""Modeled root iScan state and later ordinary-access replay."""
import unittest
import subprocess
from itertools import permutations

import test_opaque_root_scan as opaque
import benchmark_native_ordinary_scans as native_bench

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

SIB = (opaque.ROOT / 'tests/cases/ours/pattern_generation/gated_sib_raw/model.icl').read_text()


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

    def test_root_raw_queue_order_permutations(self):
        commands = ['iWrite value 0x3c', 'iRead value 0',
                    'iScan client 8 -si 0xa5 -so 0x3c']
        reference = self.replay('iReset; ' + '; '.join(commands) + '; iApply; '
                                'iRead value 0xa5; iApply\n')
        for order in permutations(commands):
            with self.subTest(order=order):
                result = self.replay('iReset; ' + '; '.join(order) + '; iApply; '
                                     'iRead value 0xa5; iApply\n')
                self.assertEqual(result, reference)

    def test_root_raw_queue_future_route_and_overwrites(self):
        reference = self.replay('iReset; iWrite open 1; iRead open 0; '
                                'iScan client 9 -si 0b101001011 -so 1; iApply; '
                                'iRead child.value 0xa5; iRead open 1; iApply\n', SIB)
        body = ('iReset; iScan client 8 -si 0; iWrite open 0; '
                'iScan client 9 -si 0b101001011 -so 1; '
                'iRead open 0; iWrite open 1; iApply; '
                'iRead child.value 0xa5; iRead open 1; iApply\n')
        self.assertEqual(self.replay(body, SIB), reference)
        self.rejected('iReset; iScan client 9; iWrite open 1; '
                      'iState open 0; iApply\n', 'length', SIB)

    def test_root_raw_queue_calls_and_owner_lifetime(self):
        definitions = ('iProc scan {} {iScan client 9 -si 0b101001011 -so 1}\n'
                       'iProc prepare {} {iWrite open 1; iRead open 0}\n')
        forward = definitions + 'iReset; iCall scan; iCall prepare; iApply\n'
        reference = definitions + 'iReset; iCall prepare; iCall scan; iApply\n'
        self.assertEqual(self.replay(forward, SIB), self.replay(reference, SIB))
        for persistent in ['', ' -persistent']:
            definitions = 'iProc prepare {} {iTake open' + persistent + '; iWrite open 1}\n'
            body = definitions + ('iReset; iScan client 9 -si 0b00000000x; '
                                  'iCall prepare; iApply\n')
            if persistent:
                self.rejected(body, 'reserved', SIB)
            else:
                self.replay(body, SIB)
        self.rejected('iReset; iTake open; iScan client 9 -si 0b00000000x; '
                      'iWrite open 1; iApply\n', 'reserved', SIB)

    def test_root_raw_queue_cancellation_and_early_validation(self):
        self.replay('iReset; iScan client 9; iReset; iApply\n', SIB)
        self.replay('iScan client 9; iWrite open 1; iReset; iApply\n', SIB)
        for scan in ['iScan client 9 -si 512', 'iScan missing 9',
                     'iScan client -chain other 9', 'iScan client 1048577']:
            with self.subTest(scan=scan):
                self.rejected('iReset; ' + scan + '; iWrite open 1; iReset\n', 'scan', SIB)
        invalid = MODEL.replace('ScanRegister value',
                                "ScanMux routing SelectedBy sel { 1'b0: si; 1'b1: si; } ScanRegister value")
        invalid = invalid.replace('ScanInSource si;', 'ScanInSource routing;')
        self.rejected('iReset; iScan client 8; iWrite value 1; iReset\n', 'scan', invalid)

    def test_root_raw_queue_keeps_child_and_together_boundaries(self):
        self.rejected('iReset; iScan client 8; iRead value 0; iApply -together\n', 'together')
        self.rejected('iReset; iScan child.local 8; iRead child.value 0; iApply\n', 'root', CHILD)

    def test_mixed_root_raw_prepares_future_route(self):
        mixed = ('iReset; iWrite open 1; iRead open 0; '
                 'iScan client 9 -si 0b101001011 -so 1; iApply; '
                 'iRead child.value 0xa5; iRead open 1; iApply\n')
        reference = mixed.replace('iRead open 0; iScan', 'iRead open 0; iApply; iScan')
        self.assertEqual(self.replay(mixed, SIB), self.replay(reference, SIB))

    def test_mixed_root_raw_static_read_and_write_preparation(self):
        for setup, expected in [('iWrite value 0x3c', '0x3c'),
                                ('iRead value 0', '0')]:
            with self.subTest(setup=setup):
                mixed = f'iReset; {setup}; iScan client 8 -si 0xa5 -so {expected}; iApply\n'
                reference = mixed.replace('; iScan', '; iApply; iScan')
                self.assertEqual(self.replay(mixed), self.replay(reference))

    def test_mixed_root_raw_reservations_and_atomic_failures(self):
        # The preparation establishes protection before the raw assignment replaces it.
        prefix = 'iReset; iTake open; iWrite open 1; '
        result = self.replay(prefix + 'iScan client 9 -si 0b101001010; iApply; '
                             'iRelease open; iWrite child.value 5; iApply\n', SIB)
        self.assertIn('-si 0b101001010', result)
        self.rejected(prefix + 'iScan client 9 -si 0b101001010; iApply; '
                      'iWrite child.value 5; iApply\n', 'reserv', SIB)
        self.rejected(prefix + 'iScan client 9 -si 0b10100101x; iApply\n', 'reserved', SIB)
        self.rejected(prefix + 'iScan client 8 -si 0; iApply\n', 'length', SIB)
        self.rejected(prefix + 'iScan client 9 -si 0b101001010 -stable; iApply\n', 'stable', SIB)
        self.rejected('iReset; iTake open; iState open 0; iWrite child.value 5; '
                      'iScan client 9; iApply\n', 'reserv', SIB)

    def test_mixed_root_raw_cancellation_and_discarded_validation(self):
        self.replay('iReset; iWrite open 1; iScan client 9 -si 0; iReset; iApply\n', SIB)
        self.replay('iWrite open 1; iScan client 9 -si 0; iReset; iApply\n', SIB)
        self.replay('iReset; iWrite open 1; iScan client 8 -si 0; '
                    'iScan client 9 -si 0b101001011; iApply\n', SIB)
        for scan in ['iScan client 9 -si 512', 'iScan missing 9',
                     'iScan client -chain other 9', 'iScan client 1048577']:
            with self.subTest(scan=scan):
                self.rejected('iReset; iWrite open 1; ' + scan + '; iReset\n', 'scan', SIB)
        invalid = MODEL.replace('ScanRegister value',
                                "ScanMux routing SelectedBy sel { 1'b0: si; 1'b1: si; } ScanRegister value")
        invalid = invalid.replace('ScanInSource si;', 'ScanInSource routing;')
        self.rejected('iReset; iWrite value 1; iScan client 8; iReset\n', 'scan', invalid)

    def test_mixed_root_raw_pending_state_and_cross_call_queue(self):
        self.rejected('iReset; iWrite open 1; iScan client 9; '
                      'iState open 0; iApply\n', 'length', SIB)
        definitions = ('iProc prepare {} {iWrite open 1}\n'
                       'iProc scan {} {iScan client 9 -si 0b101001011}\n')
        mixed = definitions + 'iReset; iCall prepare; iCall scan; iApply\n'
        reference = definitions + 'iReset; iCall prepare; iApply; iCall scan; iApply\n'
        self.assertEqual(self.replay(mixed, SIB), self.replay(reference, SIB))

    def test_mixed_root_raw_explicit_scope_boundaries(self):
        self.rejected('iReset; iRead value 0; iScan client 8; iApply -together\n', 'together')
        self.rejected('iReset; iWrite open 1; iScan child.local 8; iApply\n', 'root', SIB)

    def test_mixed_root_raw_pending_ownership_lifetime(self):
        for persistent in ['', ' -persistent']:
            with self.subTest(persistent=persistent):
                definitions = 'iProc prepare {} {iTake open' + persistent + '; iWrite open 1}\n'
                body = definitions + 'iReset; iCall prepare; iScan client 9 -si 0b00000000x; iApply\n'
                if persistent:
                    self.rejected(body, 'reserved', SIB)
                else:
                    self.replay(body, SIB)
        self.rejected('iProc prepare {} {iWrite open 1}\n'
                      'iReset; iTake open; iCall prepare; '
                      'iScan client 9 -si 0b00000000x; iApply\n', 'reserved', SIB)

    def test_native_ordinary_wide_visible_preparation(self):
        model = SIB.replace('[7:0]', '[1048574:0]')
        model = model.replace("ResetValue 8'b0", "ResetValue 1048575'b0")
        result = self.replay('iReset; iWrite open 1; iRead open 0; '
                             'iScan client 1048576 -si 1; iApply; '
                             'iRead open 1; iApply\n', model)
        self.assertIn('iWrite open 0b1', result)
        self.assertIn('iRead open 0b0', result)
        self.assertIn('iScan client 1048576', result)

    def test_native_ordinary_distinct_visible_sibs(self):
        count = 1024
        model = native_bench.many_model(count)
        commands = 'iReset; ' + ''.join(f'iWrite r{i}.enable 1; ' for i in range(count))
        commands += f'iScan client {9 * count} -si 0b' + '101001011' * count + '; iApply; '
        commands += 'iRead r0.leaf.value 0xa5; iRead r0.enable 1; iApply\n'
        result = self.replay(commands, model)
        self.assertIn('iScan client 9216', result)

    def test_native_ordinary_unknown_fill_and_capture_observers(self):
        unknown = MODEL.replace("ResetValue 8'b0", "ResetValue 8'bx")
        result = self.replay('iReset; iWrite value[7] 1; iApply\n', unknown)
        self.assertIn('iWrite value 0b1', result)
        external = MODEL.replace('CaptureSource value;', "CaptureSource 8'b00010110;")
        result = self.replay('iReset; iTake value[3]; iState value 0; '
                             'iWrite value 0xa5; iRead value 0x3c; iApply; '
                             'iWrite value[0] 1; iApply\n', external)
        self.assertIn('iRead value 0b00111100', result)
        self.assertIn('iWrite value 0b10100101', result)

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
        for command in ['iMerge -begin; iMerge -end']:
            with self.subTest(command=command):
                self.rejected('iReset; ' + command + '; iScan client 8; iApply\n', 'scan')

    def test_raw_reserved_explicit_update_refreshes_partial_history(self):
        result = self.replay('iReset; iTake value[3]; iWrite value 8; iApply\n'
                             'iScan client 8 -si 0xa5; iApply\n'
                             'iWrite value[1] 1; iApply; iRead value 0xa7; iApply\n')
        self.assertIn('iWrite value 0b10100111', result)
        self.assertNotIn('iTake', result)

    def test_raw_reserved_surrounding_storage_and_default_input(self):
        result = self.replay('iReset; iTake head; iState head 5; '
                             'iTake tail; iState tail 21; '
                             'iTake child.value; iState child.value 255; '
                             'iScan child.local 8; iApply; '
                             'iRead head 5; iRead tail 21; iRead child.value 0; iApply\n', CHILD)
        self.assertIn('-si 0b1010000000010101', result)
        self.assertIn('iRead child.value 0b00000000', result)

    def test_raw_unwritten_take_and_pre_take_writes_remain_free(self):
        for prefix in ['iTake open;', 'iWrite open 0; iApply; iTake open;']:
            with self.subTest(prefix=prefix):
                self.replay('iReset; ' + prefix + 'iScan client 1 -si 0; iApply; '
                            'iWrite child.value 5; iApply\n', SIB)
        protected = 'iReset; iTake open; iWrite open 0; iApply; iScan client 1; iApply; '
        self.rejected(protected + 'iWrite child.value 5; iApply\n', 'reserv', SIB)
        self.replay(protected + 'iRelease open; iWrite child.value 5; iApply\n', SIB)
        self.replay(protected + 'iScan client 1 -si 1; iApply; '
                    'iWrite child.value 5; iApply\n', SIB)

    def test_raw_reserved_unknown_atomic_and_overwritten_setup(self):
        prefix = 'iReset; iTake value[7]; iState value 128; '
        self.rejected(prefix + 'iScan client 8 -si 0bxxxxxxxx; iApply\n', 'reserved')
        self.replay(prefix + 'iScan client 8 -si 0bxxxxxxxx; '
                    'iScan client 8 -si 0x5a; iApply; '
                    'iWrite value[0] 1; iApply; iRead value 0x5b; iApply\n')
        self.replay(prefix + 'iScan client 8 -si 0b0xxxxxxx; iApply\n')
        self.replay(prefix + 'iRelease value[7]; iScan client 8 -si 0bxxxxxxxx; iApply\n')
        self.rejected(prefix + 'iScan client 8 -si 256; iScan client 8 -si 0; iApply\n', 'scan')

    def test_raw_reservation_scope_release_and_reset(self):
        for take, locked in [('iTake open', False), ('iTake open -persistent', True)]:
            with self.subTest(take=take):
                commands = ('iProc own {} {' + take + '; iState open 0; '
                            'iScan client 1; iApply}\n'
                            'iReset; iCall own; iWrite child.value 5; iApply\n')
                if locked:
                    self.rejected(commands, 'reserv', SIB)
                else:
                    self.replay(commands, SIB)
        self.replay('iProc scan {} {iScan client 1; iApply}\n'
                    'iReset; iTake open; iState open 0; iCall scan; '
                    'iReset; iWrite child.value 5; iApply\n', SIB)
        self.rejected('iProc scan {} {iScan client 1; iApply}\n'
                      'iReset; iTake open; iState open 0; iCall scan; '
                      'iWrite child.value 5; iApply\n', 'reserv', SIB)

    def test_raw_established_port_ownership_is_explicitly_unsupported(self):
        self.rejected('iReset; iTake sel; iState sel 0; '
                      'iScan client 8; iApply\n', 'storage')

    def test_raw_reserved_inverted_ascending_alias(self):
        model = MODEL.replace('ScanOutPort so',
                              'Alias reversed[0:7] = ~value; ScanOutPort so')
        model = model.replace('ScanInSource si;', 'ScanInSource ~si;')
        self.replay('iReset; iTake reversed[4]; iWrite reversed[4] 0; iApply; '
                    'iScan client 8 -si 0x5a; iApply; '
                    'iWrite value[1] 1; iApply; iRead value 0xa7; iApply\n', model)

    def test_raw_pending_state_revalidates_route_and_call_ownership(self):
        self.replay('iReset; iScan client 1 -si 1; iState open 0; iApply; '
                    'iRead open 1; iApply\n', SIB)
        self.rejected('iReset; iScan client 1 -si 1; iState open 1; iApply\n',
                      'length', SIB)
        # A take established inside a returned call has expired at caller Apply.
        self.replay('iProc own {} {iTake value; iState value 0; '
                    'iScan client 8 -si 0bxxxxxxxx}\n'
                    'iReset; iCall own; iApply\n')
        self.replay('iReset; iTake value; iState value 0 -LastReadValue; '
                    'iScan client 8 -si 0bxxxxxxxx -so 0; iApply\n')
        self.rejected('iReset; iScan client 1 -si 0bx; iApply; '
                      'iState open 0 -LastReadValue; iScan client 1 -so 0; iApply\n',
                      'scan', SIB)

    def test_raw_pending_child_footprint_after_same_length_route_change(self):
        model = (opaque.ROOT / 'tests/cases/ours/pattern_generation/dynamic_child_raw/local_stability.icl').read_text()
        self.replay('iReset; iTake sibling.value; iState sibling.value 0x5a; '
                    'iScan child.local 8 -si 0xa5; iState child.value 1; iApply; '
                    'iRead sibling.value 0x5a; iRead child.value 0xa5; iApply\n', model)

    def test_raw_wide_reservation_setup_and_explicit_refresh(self):
        width = 1048576
        model = MODEL.replace('[7:0]', f'[{width - 1}:0]')
        model = model.replace("ResetValue 8'b0", f"ResetValue {width}'b0")
        model = model.replace("DefaultLoadValue 8'b00000000", f"DefaultLoadValue {width}'b0")
        payload = '10100101' * (width // 8)
        try:
            result = self.replay('iReset; iTake value; iState value 0; '
                                 f'iScan client {width} -si 0b{payload}; iApply\n', model)
        except subprocess.TimeoutExpired:
            self.fail('one-million-bit reservation setup/explicit refresh exceeds the 15s CLI budget')
        self.assertIn('-si 0b' + payload, result)

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

    def test_raw_tck_idle_counts_and_following_accesses(self):
        for count in [0, 1, 7, 18446744073709551615]:
            with self.subTest(count=count):
                result = self.replay('iReset; iScan client 8 -si 0xa5; iApply\n'
                                     'iRunLoop ' + str(count) + ' -tck\n'
                                     'iWrite child.value[7] 0; iApply\n'
                                     'iRead child.value 0x25; iApply\n', HIER)
                self.assertIn('iRunLoop ' + str(count) + ' -tck', result)
                self.assertIn('iWrite child.value 0b00100101', result)

    def test_raw_tck_idle_child_implicit_and_partial_x(self):
        for implicit in [False, True]:
            with self.subTest(implicit=implicit):
                model = HIER
                target = 'child.local'
                if implicit:
                    model = model.replace(' ScanInterface local { Port si; Port so; Port sel; Port ce; Port se; Port ue; Port tck; Port reset; }\n', '')
                    target = 'client'
                result = self.replay('iReset; iScan client 8 -si 0xa5; iApply; iRunLoop 3\n'
                                     'iScan ' + target + ' 8 -si 0b10x01x01; iApply\n'
                                     'iRunLoop 5 -tck; iWrite child.value[1] 1; iApply\n'
                                     'iRead child.value 0xaf; iApply\n', model)
                self.assertIn('iWrite child.value 0b10101111', result)
                self.assertIn('iRunLoop 3 -tck', result)
                self.assertIn('iRunLoop 5 -tck', result)

    def test_raw_tck_idle_pending_and_invalid_atomic(self):
        self.rejected('iReset; iScan client 8; iRunLoop 0; iReset\n', 'unapplied', HIER)
        for command, diagnostic in [('iRunLoop 0bx', 'known'),
                                    ('iRunLoop 18446744073709551616', 'truncates'),
                                    ('iRunLoop 1 -sck tck', 'ClockPort'),
                                    ('iRunLoop -time 1ns', 'absolute')]:
            with self.subTest(command=command):
                self.rejected('iReset; iScan client 8; iApply; ' + command + '\n', diagnostic, HIER)
        self.rejected('iReset; iScan client 8; iApply; iRunLoop 1\n', 'opaque', opaque.MODEL)

    def test_default_chain_root_and_child_replay(self):
        result = self.replay('iReset; iScan client -chain default 8 -si 0xa5; iApply\n'
                             'iScan child.local -chain default 8 -si 0x5a -so 0xa5; iApply\n'
                             'iRead child.value 0x5a; iApply\n', HIER)
        self.assertEqual(result.count('iScan client 8'), 2)
        self.assertNotIn('-chain', result)
        self.assertIn('iRead child.value 0b01011010', result)

    def test_default_chain_invalid_discarded_and_tap_atomic(self):
        for selection, diagnostic in [('-chain other 8', 'chain'),
                                      ('-chain default', 'length'),
                                      ('-chain default 7', 'length'),
                                      ('-chain default 8 -si 0x100', 'scan')]:
            with self.subTest(selection=selection):
                self.rejected('iScan child.local ' + selection + '; iReset\n', diagnostic, HIER)
        self.rejected('iScan -ir client -chain default 8; iReset\n', 'scan', HIER)

    def test_sib_raw_open_close_reopen_retains_child(self):
        result = self.replay('iReset; iScan client 1 -si 1; iApply\n'
                             'iScan client 9 -si 0b101001011 -stable; iApply\n'
                             'iScan client 9 -si 0b001111000; iApply\n'
                             'iScan client 1 -si 1; iApply\n'
                             'iRead child.value 0x3c; iApply\n', SIB)
        self.assertEqual(result.count('iScan '), 4)
        self.assertIn('iScan client 9 -si 0b101001011', result)
        self.assertIn('iRead child.value 0b00111100', result)

    def test_sib_raw_partial_x_and_current_state_after_ordinary_write(self):
        result = self.replay('iReset; iWrite open 1; iApply\n'
                             'iScan client 9 -si 0b101001011 -stable; iApply\n'
                             'iScan client 9 -si 0b10x01x011 -stable; iApply\n'
                             'iWrite child.value[1] 1; iApply\n'
                             'iRead child.value 0xaf; iApply\n', SIB)
        self.assertIn('iWrite child.value 0b10101111', result)

    def test_sib_raw_invalid_stability_and_length_atomic(self):
        self.rejected('iReset; iScan client 1 -si 1 -stable; iApply\n', 'stable', SIB)
        self.rejected('iReset; iScan client 9; iApply\n', 'length', SIB)
        self.replay('iScan client 1; iReset\n', SIB)
        self.rejected('iScan client 1; iApply\n', 'iReset', SIB)
        self.rejected('iReset; iScan child.local 8; iApply\n', 'root', SIB)
        self.rejected('iReset; iScan client 1 -si 1; iApply\n'
                      'iScan client 9 -si 0 -stable; iApply\n', 'stable', SIB)

    def test_sib_raw_bypass_is_not_event_gating(self):
        ungated = SIB.replace('InputPort sel = open;', 'InputPort sel = sel;')
        self.rejected('iReset; iScan client 1 -si 1; iApply\n', 'raw scan', ungated)
        serial = SIB.replace('SelectedBy open', 'SelectedBy child.so')
        self.rejected('iReset; iScan client 1; iApply\n', '', serial)

    def test_sib_raw_same_width_route_change_is_not_stable(self):
        other = SIB[SIB.index(' Instance child Of'):SIB.index(' ScanMux route')]
        other = other.replace('Instance child Of', 'Instance other Of')
        other = other.replace('InputPort sel = open;', 'InputPort sel = ~open;')
        model = SIB.replace(' ScanMux route', other + ' ScanMux route')
        model = model.replace("1'b0: si;", "1'b0: other.so;")
        self.rejected('iReset; iScan client 9 -si 1 -stable; iApply\n',
                      'stable', model)
        result = self.replay('iReset; iScan client 9 -si 1; iApply\n'
                             'iScan client 9 -si 0b101001011 -stable; iApply\n'
                             'iRead child.value 0xa5; iApply\n', model)
        self.assertIn('iRead child.value 0b10100101', result)

    def test_sib_raw_held_forwarding_and_unknown_route(self):
        model = SIB.replace(' ScanMux route', ' Alias gate = open;\n ScanMux route')
        model = model.replace('SelectedBy open', 'SelectedBy gate')
        model = model.replace('InputPort sel = open;', 'InputPort sel = gate;')
        self.replay('iReset; iScan client 1 -si 1; iApply\n'
                    'iScan client 9 -si 0b101001011 -stable; iApply\n', model)
        self.rejected('iReset; iScan client 1 -si 0bx; iApply\n'
                      'iScan client 1; iApply\n', 'known', model)
        self.rejected('iReset; iScan client 1 -si 0bx -stable; iApply\n',
                      'stable', model)

    def test_active_child_raw_retargets_current_sib_route(self):
        result = self.replay('iReset; iScan client 1 -si 1; iApply\n'
                             'iScan child.local 8 -si 0xa5 -so 0x3c -stable; iApply\n'
                             'iRead child.value 0xa5; iApply\n', SIB)
        self.assertIn('iScan client 9 -si 0b101001011 -so 0b00111100x -stable', result)
        self.assertIn('iRead child.value 0b10100101', result)

    def test_active_child_raw_cache_after_ordinary_route_change(self):
        self.replay('iReset; iWrite open 1; iApply\n'
                    'iScan child.local 8 -si 0xa5; iApply\n'
                    'iWrite open 0; iApply; iWrite open 1; iApply\n'
                    'iScan child.local 8 -si 0x3c -stable; iApply\n'
                    'iRead child.value 0x3c; iApply\n', SIB)
        self.rejected('iReset; iWrite open 1; iApply\n'
                      'iScan child.local 8 -si 0xa5; iApply\n'
                      'iWrite open 0; iApply; iScan child.local 8; iApply\n',
                      'contiguous', SIB)

    def test_active_child_raw_closed_overlap_and_discarded_setup_atomic(self):
        self.rejected('iReset; iScan child.local 8; iApply\n', 'contiguous', SIB)
        self.rejected('iReset; iScan client 1 -si 1; iApply\n'
                      'iScan child.local 7; iApply\n', 'length', SIB)
        self.rejected('iReset; iScan client 1 -si 1; iApply\n'
                      'iScan client 9; iScan child.local 8; iApply\n',
                      'overlapping', SIB)
        self.rejected('iReset; iScan client 1 -si 1; iApply\n'
                      'iScan child.local 8 -bad; iScan child.local 8; iApply\n',
                      'option', SIB)

    def test_active_child_raw_disjoint_siblings_and_latest_assignment(self):
        block = SIB[SIB.index(' Instance child Of'):SIB.index(' ScanMux route')]
        sibling = block.replace('Instance child Of', 'Instance sibling Of')
        sibling = sibling.replace('InputPort si = si;', 'InputPort si = child.so;')
        model = SIB.replace(' ScanMux route', sibling + ' ScanMux route')
        model = model.replace("1'b1: child.so;", "1'b1: sibling.so;")
        result = self.replay('iReset; iScan client 1 -si 1; iApply\n'
                             'iScan child.local 8 -si 0x12 -stable\n'
                             'iScan child.local 8 -si 0xa5 -stable\n'
                             'iScan sibling.local 8 -si 0x3c\niApply\n'
                             'iRead child.value 0xa5; iRead sibling.value 0x3c; iApply\n',
                             model)
        scans = [line for line in result.splitlines() if line.startswith('iScan ')]
        self.assertEqual(len(scans), 2)
        self.assertIn('iScan client 17 -si 0b10100101001111001', scans[1])
        self.assertNotIn('-stable', scans[1])

    def test_active_child_raw_local_stability_across_other_route_change(self):
        block = SIB[SIB.index(' Instance child Of'):SIB.index(' ScanMux route')]
        sibling = block.replace('Instance child Of', 'Instance sibling Of')
        sibling = sibling.replace('InputPort sel = open;', 'InputPort sel = child.gate;')
        model = SIB.replace(block, block.replace('InputPort si = si;', 'InputPort si = route;')
                            .replace('InputPort sel = open;', 'InputPort sel = sel;') + sibling)
        model = model.replace('SelectedBy open', 'SelectedBy child.gate')
        model = model.replace("1'b1: child.so;", "1'b1: sibling.so;")
        model = model.replace(' ScanRegister open { ScanInSource route; CaptureSource open; ResetValue 1\'b0; }\n', '')
        model = model.replace('Source open;', 'Source child.so;')
        model = model.replace(' ScanOutPort so { Source value[0]; }',
                              ' DataOutPort gate { Source value[0]; }\n'
                              ' ScanOutPort so { Source value[0]; }')
        self.rejected('iReset; iScan client 8 -si 1 -stable; iApply\n',
                      'stable', model)
        opened = 'iReset; iScan child.local 8 -si 1 -stable; iApply\n'
        result = self.replay(opened + 'iScan sibling.local 8 -si 0xa5; iApply\n'
                             'iRead child.value 1; iRead sibling.value 0xa5; iApply\n', model)
        scans = [line for line in result.splitlines() if line.startswith('iScan ')]
        self.assertIn('iScan client 8 -si 0b00000001', scans[0])
        self.assertNotIn('-stable', scans[0])
        self.assertIn('iScan client 16 -si 0b1010010100000001', scans[1])
        result = self.replay(opened + 'iScan child.local 8 -si 0 -stable\n'
                             'iScan sibling.local 8 -si 0xa5\niApply\n'
                             'iRead child.value 0; iApply\n', model)
        scans = [line for line in result.splitlines() if line.startswith('iScan ')]
        self.assertIn('iScan client 16 -si 0b1010010100000000', scans[1])
        self.assertNotIn('-stable', scans[1])

    def test_active_child_raw_offset_changes_with_other_sib(self):
        leaf = SIB[:SIB.index('Module Top')]
        model = leaf + """Module Top {
 ScanInPort si; SelectPort sel; CaptureEnPort ce; ShiftEnPort se;
 UpdateEnPort ue; TCKPort tck; ResetPort reset;
 Instance a Of RawSibLeaf {
  InputPort si = si; InputPort sel = a_en; InputPort ce = ce;
  InputPort se = se; InputPort ue = ue; InputPort tck = tck; InputPort reset = reset;
 }
 ScanMux a_route SelectedBy a_en { 1'b0: si; 1'b1: a.so; }
 ScanRegister a_en { ScanInSource a_route; CaptureSource a_en; ResetValue 1'b0; }
 Instance b Of RawSibLeaf {
  InputPort si = a_en; InputPort sel = b_en; InputPort ce = ce;
  InputPort se = se; InputPort ue = ue; InputPort tck = tck; InputPort reset = reset;
 }
 ScanMux b_route SelectedBy b_en { 1'b0: a_en; 1'b1: b.so; }
 ScanRegister b_en { ScanInSource b_route; CaptureSource b_en; ResetValue 1'b0; }
 ScanOutPort so { Source b_en; }
 ScanInterface client {
  Port si; Port so; Port sel; Port ce; Port se; Port ue; Port tck; Port reset;
  DefaultLoadValue 1'b0;
 }
}
"""
        result = self.replay('iReset; iScan client 2 -si 3; iApply\n'
                             'iScan b.local 8 -si 0x3c -stable; iApply\n'
                             'iScan client 18 -si 0b000000000001111001; iApply\n'
                             'iScan b.local 8 -si 0xa5; iApply\n'
                             'iRead b.value 0xa5; iApply\n', model)
        self.assertIn('iScan client 10 -si 0b0101001011', result)
        self.assertIn('iRead b.value 0b10100101', result)

    def test_active_child_raw_inversion_parity_and_partial_x(self):
        for input_inverse in (False, True):
            for child_inverse in (False, True):
                for root_inverse in (False, True):
                    model = SIB
                    if input_inverse:
                        model = model.replace('InputPort si = si;', 'InputPort si = ~si;')
                    if child_inverse:
                        model = model.replace('Source value[0];', 'Source ~value[0];')
                    if root_inverse:
                        model = model.replace('Source open;', 'Source ~open;')
                    with self.subTest(input_inverse=input_inverse,
                                      child_inverse=child_inverse,
                                      root_inverse=root_inverse):
                        result = self.replay('iReset; iScan client 1 -si 1; iApply\n'
                                    'iScan child.local 8 -si 0xa5 -so 0x3c -stable; iApply\n'
                                    'iScan child.local 8 -si 0b10x01x01 -stable; iApply\n'
                                    'iWrite child.value[1] 1; iApply\n'
                                    'iRead child.value 0xaf; iApply\n', model)

                        expected = ('11000011' if root_inverse else '00111100') + 'x'
                        scans = [line for line in result.splitlines()
                                 if line.startswith('iScan ')]
                        self.assertIn('-so 0b' + expected + ' -stable', scans[1])

    def test_active_child_raw_fixture(self):
        folder = opaque.ROOT / 'tests/cases/ours/pattern_generation/active_child_raw'
        commands = 'iReset' + (folder / 'source.pdl').read_text().split('iReset', 1)[1]
        result = self.replay(commands, (folder / 'model.icl').read_text())
        self.assertIn('iScan client 9 -si 0b101001011 -so 0b00000000x', result)
        self.assertIn('iRead child.value 0b10100101', result)
        self.assertIn('iRead open 0b1', result)

    def test_dynamic_child_raw_fixture_and_local_widths(self):
        folder = opaque.ROOT / 'tests/cases/ours/pattern_generation/dynamic_child_raw'
        model = (folder / 'model.icl').read_text()
        commands = 'iReset' + (folder / 'source.pdl').read_text().split('iReset', 1)[1]
        result = self.replay(commands, model)
        self.assertIn('iScan client 2 -si 0b11 -so 0b0x', result)
        self.assertIn('iScan client 10 -si 0b1010010111 -so 0b000000001x', result)
        self.assertIn('iRead child.leaf.value 0b00111100', result)

    def test_dynamic_child_raw_stable_local_change_and_unknown_atomic(self):
        model = (opaque.ROOT / 'tests/cases/ours/pattern_generation/dynamic_child_raw/model.icl').read_text()
        opened = 'iReset; iScan client 1 -si 1; iApply\n'
        self.rejected(opened + 'iScan child.local 1 -si 1 -stable; iApply\n', 'stable', model)
        self.rejected(opened + 'iScan child.local 9; iApply\n', 'length', model)
        self.rejected(opened + 'iScan client 2 -si 3 -stable; iApply\n', 'stable', model)
        self.replay(opened + 'iScan child.local 1 -si 0bx; iApply\n', model)
        self.rejected(opened + 'iScan child.local 1 -si 0bx; iApply\n'
                      'iScan child.local 1; iApply\n', 'known', model)
        self.rejected('iReset; iScan child.local 1; iApply\n', 'contiguous', model)

    def test_dynamic_child_raw_compensated_root_width_is_not_local_stability(self):
        model = (opaque.ROOT / 'tests/cases/ours/pattern_generation/dynamic_child_raw/model.icl').read_text()
        start = model.index(' Instance child Of')
        end = model.index(' ScanMux route', start)
        block = model[start:end]
        first = block.replace('Instance child Of', 'Instance r0 Of')
        second = block.replace('Instance child Of', 'Instance r1 Of')
        second = second.replace('InputPort si = si;', 'InputPort si = r0.so;')
        model = model[:start] + first + second + model[end:]
        model = model.replace("1'b1: child.so;", "1'b1: r1.so;")
        opened = ('iReset; iScan client 1 -si 1; iApply\n'
                  'iScan r1.local 1 -si 1; iApply\n'
                  'iScan r1.local 9 -si 0b101001011 -stable; iApply\n')
        self.rejected(opened + 'iScan r0.local 1 -si 1 -stable\n'
                      'iScan r1.local 9 -si 0b001111000\niApply\n', 'stable', model)
        self.replay(opened + 'iScan r0.local 1 -si 0 -stable\n'
                    'iScan r1.local 9 -si 0b101001011\niApply\n', model)

    def test_dynamic_child_raw_inversion_and_expected_so(self):
        original = (opaque.ROOT / 'tests/cases/ours/pattern_generation/dynamic_child_raw/model.icl').read_text()
        for input_inverse in (False, True):
            for child_inverse in (False, True):
                for root_inverse in (False, True):
                    prefix, top = original.split('Module Top', 1)
                    if input_inverse:
                        top = top.replace('InputPort si = si;', 'InputPort si = ~si;')
                    if child_inverse:
                        prefix = prefix.replace('Source enable;', 'Source ~enable;')
                    if root_inverse:
                        top = top.replace('Source open;', 'Source ~open;')
                    with self.subTest(input_inverse=input_inverse,
                                      child_inverse=child_inverse,
                                      root_inverse=root_inverse):
                        result = self.replay('iReset; iScan client 1 -si 1; iApply\n'
                                             'iScan child.local 1 -si 1; iApply\n'
                                             'iScan child.local 9 -si 0b101001011 '
                                             '-so 0b000000001 -stable; iApply\n'
                                             'iRead child.leaf.value 0xa5; iApply\n',
                                             prefix + 'Module Top' + top)
                        scans = [line for line in result.splitlines() if line.startswith('iScan ')]
                        expected = ('111111110' if root_inverse else '000000001') + 'x'
                        self.assertIn('-so 0b' + expected + ' -stable', scans[2])

    def test_dynamic_child_raw_equal_count_ids_and_suffix_parity_stable(self):
        original = (opaque.ROOT / 'tests/cases/ours/pattern_generation/dynamic_child_raw/model.icl').read_text()
        prefix = original.split('Module Top', 1)[0]
        top = """Module Top {
 ScanInPort si; SelectPort sel; CaptureEnPort ce; ShiftEnPort se;
 UpdateEnPort ue; TCKPort tck; ResetPort reset;
 Instance child Of DynamicRawBranch {
  InputPort si = si; InputPort sel = sel; InputPort ce = ce;
  InputPort se = se; InputPort ue = ue; InputPort tck = tck; InputPort reset = reset;
 }
 ScanOutPort so { Source child.so; }
 ScanInterface client {
  Port si; Port so; Port sel; Port ce; Port se; Port ue; Port tck; Port reset;
  DefaultLoadValue 9'b0;
 }
}
"""
        start = prefix.index(' Instance leaf Of')
        end = prefix.index(' ScanMux inner_route', start)
        block = prefix[start:end]
        left = block.replace('Instance leaf Of', 'Instance left Of')
        left = left.replace('InputPort sel = enable;', 'InputPort sel = ~enable;')
        right = block.replace('Instance leaf Of', 'Instance right Of')
        switched = prefix[:start] + left + right + prefix[end:]
        switched = switched.replace("1'b0: si; 1'b1: leaf.so;", "1'b0: left.so; 1'b1: right.so;")
        parity = prefix.replace('InputPort si = si; InputPort sel = enable;',
                                'InputPort si = input_route; InputPort sel = sel;')
        parity = parity.replace(' ScanMux inner_route',
                                " ScanMux input_route SelectedBy enable { 1'b0: si; 1'b1: ~si; }\n ScanMux inner_route")
        parity = parity.replace("1'b0: si; 1'b1: leaf.so;", "1'b0: leaf.so; 1'b1: ~leaf.so;")
        for name, branch, observed, data in [('ids', switched, 'child.right.value', '0b101001011'),
                                             ('suffix', parity, 'child.leaf.value', '0b010110101')]:
            with self.subTest(change=name):
                model = branch + top
                self.rejected('iReset; iScan child.local 9 -si 0b101001011 -stable; iApply\n',
                              'stable', model)
                result = self.replay('iReset; iScan child.local 9 -si 0b101001011; iApply\n'
                                     'iScan child.local 9 -si ' + data + ' -stable; iApply\n'
                                     'iRead ' + observed + ' 0xa5; iApply\n', model)
                self.assertIn('iRead ' + observed + ' 0b10100101', result)

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
