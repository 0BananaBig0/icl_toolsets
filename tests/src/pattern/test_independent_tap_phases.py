#!/usr/bin/env python3
"""Different TAP instruction/data phases retain a shared update cycle."""
import unittest
import time
import hashlib
import json

import test_multiple_tap_scheduling as helpers


class IndependentTapPhases(unittest.TestCase):
    setUp = helpers.MultipleTaps.setUp
    prepare = helpers.MultipleTaps.prepare
    replay = helpers.MultipleTaps.replay
    run_tool = helpers.MultipleTaps.run_tool

    def test_different_taps_write_instruction_and_data_together(self):
        text = self.replay(
            'iReset; iWrite tap1.INSTRUCTION 3; iApply; '
            'iWrite tap0.INSTRUCTION 3; iWrite c1.value 85; iApply -together; '
            'iRead c1.value 85; iApply;\n')
        self.assertIn('iApply -together', text)
        self.assertIn('iWrite tap0.INSTRUCTION 0b0011', text)
        self.assertIn('iWrite c1.value 0b1010101', text)

    def test_write_only_mixed_group_keeps_unrequested_ir_capture(self):
        source, models = helpers.network((4, 4, 4), (4, 4, 4))
        body = ('iReset; iWrite tap1.INSTRUCTION 3; iApply; '
                'iWrite tap0.INSTRUCTION 3; iWrite c1.value 5; iApply -together;\n')
        for shared in (False, True):
            for modeled in (False, True):
                with self.subTest(shared=shared, modeled=modeled):
                    text = self.replay_source(
                        source, models.replace('TMS2', 'TMS0') if shared else models,
                        3, body, extra=('--model_readback',) if modeled else ())
                    self.assertEqual(text.count('iApply'), 2)
                    final = text.split('iApply')[-2]
                    self.assertIn('iWrite tap2.INSTRUCTION 0b0010\n', final)
                    self.assertIn('iRead tap2.INSTRUCTION 0b0001\n', final)
                    self.assertIn('iWrite tap0.INSTRUCTION 0b0011\n', final)
                    self.assertIn('iWrite c1.value 0b0101\n', final)
                    self.assertTrue(text.endswith('iApply -together\n'))

    def replay_source(self, source, models, count, body, extra=()):
        pdl = self.prepare(source, models, count, body)
        output = self.run_tool(pdl, extra=('--debug', *extra))
        self.assertEqual(output, self.run_tool(self.root / 'result.pdl', 'replay.pdl',
                                              extra=extra))
        self.assertEqual(output, self.run_tool(pdl, 'flat.pdl',
                                              inputs=['--icl', 'debug/MultiTapTop.icl'],
                                              extra=extra))
        self.assertEqual(output, self.run_tool(self.root / 'result.pdl', 'flat-replay.pdl',
                                              inputs=['--icl', 'debug/MultiTapTop.icl'],
                                              extra=extra))
        return output

    def scale(self, independent):
        for count, width in [(2, 8), (2, 32), (2, 64), (2, 256),
                             (4, 8), (8, 8), (16, 8)]:
            with self.subTest(count=count, width=width):
                source, models = helpers.network((width,) * count,
                                                 tuple(4 + i % 3 for i in range(count)))
                source = source.replace('ScanInSource si; ResetValue 0;',
                                        'ScanInSource si; CaptureSource value; ResetValue 0;')
                bits = '10100011' * (width // 8)
                setup = ''.join(f'iWrite tap{i}.INSTRUCTION 3; ' for i in range(1, count, 2))
                ir = ''.join(f'iWrite tap{i}.INSTRUCTION 3; ' for i in range(0, count, 2))
                data = ''.join(f'iWrite c{i}.value 0b{bits}; ' for i in range(1, count, 2))
                reads = ''.join(f'iRead c{i}.value 0; ' for i in range(1, count, 2))
                queries = ''.join(f'iNote c{i}=[iGetReadData c{i}.value -bin]; '
                                  for i in range(1, count, 2))
                body = ('iPDLLevel 1; iReset; ' + setup + 'iApply; ' + ir +
                        ('' if independent else 'iApply; ') + data + 'iApply -together; ' +
                        reads + 'iApply; ' + queries + 'iNote status=[iGetStatus];\n')
                started = time.monotonic()
                text = self.replay_source(source, models, count, body,
                                          extra=('--model_readback',))
                notes = [line[6:] for line in text.splitlines()
                         if line.startswith('iNote ') and 'modeled ' not in line]
                self.assertEqual(notes, [f'c{i}=0b{bits}' for i in range(1, count, 2)] +
                                 [f'status={bits.count("1") * (count // 2)}'])
                print(json.dumps(dict(independent=independent, taps=count, width=width,
                                      input_bytes=len(source) + len(models) + len(body),
                                      four_replays_seconds=time.monotonic() - started,
                                      sha256=hashlib.sha256(text.encode()).hexdigest())),
                      flush=True)

    def test_independent_phase_scale(self):
        self.scale(True)

    def test_common_phase_control_scale(self):
        self.scale(False)

    def test_reverse_orientation_and_explicit_instruction_comparison(self):
        text = self.replay(
            'iReset; iWrite tap0.INSTRUCTION 3; iApply; '
            'iWrite c0.value 9; iWrite tap1.INSTRUCTION 3; '
            'iRead tap1.INSTRUCTION 0bxx001; iApply -together; '
            'iRead c0.value 9; iWrite c1.value 85; iApply;\n')
        self.assertIn('iRead tap1.INSTRUCTION 0bxx001', text)

    def test_access_together_alias_combines_different_taps(self):
        source, models = helpers.network()
        source = source.replace('Module MultiTapTop {',
                                'Module MultiTapTop { Alias pair[10:0] = '
                                'tap0.INSTRUCTION, c1.value {AccessTogether;}')
        text = self.replay_source(
            source, models, 2,
            'iReset; iWrite tap1.INSTRUCTION 3; iApply; '
            'iWrite pair 469; iApply; iRead pair 213; iApply;\n')
        self.assertIn('iWrite c1.value 0b1010101', text)

    def test_distinct_equivalent_clock_roots(self):
        self.replay('iReset; iWrite tap1.INSTRUCTION 3; iApply; '
                    'iWrite tap0.INSTRUCTION 3; iWrite c1.value 85; '
                    'iApply -together;\n', separate_clocks=True)

    def test_four_taps_keep_instruction_and_data_groups_independent(self):
        self.replay(
            'iReset; iWrite tap1.INSTRUCTION 3; iWrite tap3.INSTRUCTION 3; iApply; '
            'iWrite tap0.INSTRUCTION 3; iWrite c1.value 85; '
            'iWrite tap2.INSTRUCTION 3; iWrite c3.value 165; iApply -together; '
            'iRead c1.value 85; iRead c3.value 165; iApply;\n',
            widths=(4, 7, 5, 8), ir_widths=(4, 5, 6, 7))

    def test_live_controls_and_taken_lane_survive_mixed_phases(self):
        self.replay(
            'iReset; iWrite c2.value 21; iApply; iTake c2.value; '
            'iWrite tap1.INSTRUCTION 3; iApply; '
            'iWrite tap0.INSTRUCTION 3; iWrite c1.value 85; iApply -together; '
            'iRead c2.value 21; iApply;\n', widths=(4, 7, 5), ir_widths=(4, 5, 6))
        source, models = helpers.network()
        source = source.replace('ScanInSource si; ResetValue 0;',
                                'ScanInSource si; CaptureSource value; ResetValue 0;')
        source = source.replace('DataOutPort q',
                                'ToCaptureEnPort capture_out {Source capture;} DataOutPort q')
        source = source.replace('InputPort reset = TRST1;',
                                'InputPort reset = TRST1; InputPort capture = c0.capture_out;')
        text = self.replay_source(
            source, models, 2,
            'iPDLLevel 1; iReset; iWrite tap1.INSTRUCTION 3; iApply; '
            'iWrite tap0.INSTRUCTION 3; iWrite c1.value 83; iApply -together; '
            'iRead c1.value 0; iApply; iNote [iGetReadData c1.value -bin]; '
            'iNote [iGetStatus];\n', extra=('--model_readback',))
        self.assertEqual([line[6:] for line in text.splitlines()
                          if line.startswith('iNote ') and 'modeled ' not in line],
                         ['0b1010011', '4'])

    def test_actual_mixed_capture_history_and_status(self):
        source, models = helpers.network()
        source = source.replace('ScanInSource si; ResetValue 0;',
                                'ScanInSource si; CaptureSource value; ResetValue 0;')
        source = source.replace('Module MultiTapTop {',
                                'Module MultiTapTop { Alias pair[10:0] = '
                                'tap0.INSTRUCTION, c1.value {AccessTogether;}')
        text = self.replay_source(
            source, models, 2,
            'iPDLLevel 1; iReset; iWrite tap1.INSTRUCTION 3; iApply; '
            'iWrite pair 467; iApply; iRead pair 0; iApply; '
            'iNote [iGetReadData pair -bin]; '
            'iNote [iGetMiscompares pair -bin]; '
            'iNote [iGetStatus -clear]; iNote [iGetStatus];\n',
            extra=('--model_readback',))
        notes = [line[6:] for line in text.splitlines()
                 if line.startswith('iNote ') and 'modeled ' not in line]
        self.assertEqual(notes, ['0b00011010011', '0b00011010011', '5', '0'])

    def test_local_reset_rewrite_retains_mixed_update(self):
        source, models = helpers.network()
        pdl = self.prepare(source, models, 2,
                           'iReset; iWrite tap1.INSTRUCTION 3; iApply; '
                           'iWrite tap0.INSTRUCTION 3; iWrite c1.value 83; '
                           'iApply -together;\n')
        text = self.run_tool(pdl, extra=('--local_reset', 'reset.pdl'))
        self.assertEqual(text, self.run_tool(self.root / 'reset.pdl', 'reset-replay.pdl'))
        self.assertEqual(text, self.run_tool(self.root / 'result.pdl', 'replay.pdl'))

    def test_reset_held_mixed_alias_capture_retains_group(self):
        source, models = helpers.network()
        source = source.replace('InputPort reset = TRST1;', "InputPort reset = 1'b0;")
        source = source.replace('Module MultiTapTop {',
                                'Module MultiTapTop { Alias pair[10:0] = '
                                'tap0.INSTRUCTION, c1.value {AccessTogether;}')
        self.replay_source(source, models, 2,
                           'iReset; iWrite tap1.INSTRUCTION 3; iApply; '
                           'iRead pair 0; iApply;\n')
        text = self.replay_source(source, models, 2,
                                  'iPDLLevel 1; iReset; '
                                  'iWrite tap1.INSTRUCTION 3; iApply; '
                                  'iRead pair 0b0000xxxxxxx; iApply; '
                                  'iNote [iGetReadData pair -bin]; '
                                  'iNote [iGetStatus -clear]; iNote [iGetStatus];\n',
                                  extra=('--model_readback',))
        self.assertEqual([line[6:] for line in text.splitlines()
                          if line.startswith('iNote ') and 'modeled ' not in line],
                         ['0b0001xxxxxxx', '1', '0'])

    def test_implicit_data_owner_setup_for_mixed_group(self):
        for count in (3, 4):
            with self.subTest(count=count):
                self.replay('iReset; iWrite tap0.INSTRUCTION 3; '
                            'iWrite c1.value 9; iApply -together;\n',
                            widths=(4,) * count, ir_widths=(4,) * count)

    def test_implicit_instruction_setup_preserves_pending_capture(self):
        source, models = helpers.network()
        source = source.replace('ScanInSource si; ResetValue 0;',
                                'ScanInSource si; CaptureSource value; ResetValue 0;')
        source = source.replace('DataOutPort q', 'ToSelectPort on {Source select;} DataOutPort q')
        source = source.replace('Module MultiTapTop {',
                                'Module MultiTapTop {Alias pair[10:0] = '
                                'tap0.INSTRUCTION,c1.value {AccessTogether;}')
        source = source.replace(' TRSTPort TRST1;',
                                " TRSTPort TRST1; DataMux prep SelectedBy c1.on "
                                "{1'b0:TRST1;1'b1:1'b0;}")
        source = source.replace('InputPort reset = TRST1;', 'InputPort reset = prep;')
        for expected in ('128', '211', '0bxxxxxxxxxxx'):
            pdl = self.prepare(source, models, 2, 'iReset; iState c1.value 83; '
                               f'iRead pair {expected}; iApply;')
            (self.root / 'result.pdl').write_text('prior output\n')
            self.run_tool(pdl, ok=False, extra=('--model_readback',))
            self.assertEqual((self.root / 'result.pdl').read_text(), 'prior output\n')
        safe = source.replace('InputPort reset = prep;', 'InputPort reset = TRST1;')
        output = self.replay_source(safe, models, 2,
                                   'iReset; iState c1.value 83; iRead pair 128; iApply; '
                                   'iNote actual=[iGetReadData c1.value -bin]; '
                                   'iNote mismatch=[iGetMiscompares pair -bin];',
                                   extra=('--model_readback',))
        self.assertIn('iNote actual=0b1010011', output)
        self.assertIn('iNote mismatch=0b00001010011', output)

    def test_same_tap_simultaneous_ir_and_dr_remains_rejected(self):
        source, models = helpers.network()
        pdl = self.prepare(source, models, 2,
                           'iReset; iWrite tap0.INSTRUCTION 3; '
                           'iWrite c0.value 9; iApply -together;\n')
        output = self.root / 'result.pdl'
        output.write_text('preserve previous output\n')
        self.assertIn('IR and DR', self.run_tool(pdl, ok=False))
        self.assertEqual(output.read_text(), 'preserve previous output\n')

    def test_shared_tms_remains_rejected(self):
        source, models = helpers.network(shared_tms=True)
        pdl = self.prepare(source, models, 2,
                           'iReset; iWrite tap1.INSTRUCTION 3; iApply; '
                           'iWrite tap0.INSTRUCTION 3; iWrite c1.value 85; '
                           'iApply -together;\n')
        output = self.root / 'result.pdl'
        output.write_text('preserve previous output\n')
        started = time.monotonic()
        self.assertIn('iApply -together', self.run_tool(pdl, ok=False))
        self.assertLess(time.monotonic() - started, 5.0)
        self.assertEqual(output.read_text(), 'preserve previous output\n')


if __name__ == '__main__':
    unittest.main()
