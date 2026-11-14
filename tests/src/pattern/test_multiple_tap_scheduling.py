#!/usr/bin/env python3
"""Common-phase TAP lanes retain shared state and canonical PDL replay."""
import re
import time
import unittest

import test_tap_pdl_replay as helpers


def network(widths=(4, 7), ir_widths=(4, 5), shared_tms=False, separate_clocks=False):
    source = """Module TapCell {
 Parameter W = 4;
 ScanInPort si; ScanOutPort so { Source value[0]; }
 SelectPort select; CaptureEnPort capture; ShiftEnPort shift;
 UpdateEnPort update; TCKPort clock;
 ResetPort reset { ActivePolarity 0; }
 ScanRegister value[$W-1:0] { ScanInSource si; ResetValue 0; }
 DataOutPort q[$W-1:0] { Source value; }
 ScanInterface client {
  Port si; Port so; Port select; Port capture; Port shift; Port update;
  Port clock;
 }
}
Module MultiTapTop {
"""
    template = (helpers.FIXTURE / 'simple.bsdl').read_text()
    models = []
    for lane, (width, ir_width) in enumerate(zip(widths, ir_widths)):
        suffix = str(lane)
        source += f""" TRSTPort TRST{suffix};
 Instance c{lane} Of TapCell {{ Parameter W = {width}; InputPort reset = TRST{suffix}; }}
 AccessLink tap{lane} Of STD_1149_1_2001 {{
  BSDLEntity Probe{lane};
  IJTAG_ON {{ ScanInterface {{ c{lane}.client; }} }}
  IJTAG_OFF {{ ScanInterface {{ c{lane}.client; }} }}
 }}
"""
        model = template.replace('AccessProbe', 'Probe' + suffix)
        if separate_clocks:
            model = model.replace('TCK', 'TCK' + suffix)
        for pin in ('TDI', 'TDO', 'TMS', 'TRST'):
            if pin != 'TMS' or not shared_tms:
                model = model.replace(pin, pin + suffix)
        model = model.replace('entity is 4;', f'entity is {ir_width};')
        model = re.sub(r'\(([01]+)\)',
                       lambda match: '(' + (('1' * ir_width) if set(match[1]) == {'1'}
                                            else match[1].zfill(ir_width)) + ')', model)
        capture = ('1' if lane % 2 else '0') + '0' * (ir_width - 3) + '01'
        model = model.replace('entity is "0001";', f'entity is "{capture}";')
        models.append(model)
    return source + '}\n', '\n'.join(models)


class MultipleTaps(unittest.TestCase):
    setUp = helpers.TapPdlReplay.setUp
    run_tool = helpers.TapPdlReplay.run_tool

    def prepare(self, source, models, count, body):
        icl, bsdl, pdl = (self.root / name for name in ('network.icl', 'models.bsdl', 'source.pdl'))
        icl.write_text(source)
        bsdl.write_text(models)
        pdl.write_text('iProcsForModule MultiTapTop\n' + body)
        self.inputs = ['--icl', str(icl)]
        for lane in range(count):
            self.inputs += ['--bsdl', 'Probe' + str(lane), str(bsdl)]
        return pdl

    def replay(self, body, widths=(4, 7), ir_widths=(4, 5), shared_tms=False, separate_clocks=False):
        source, models = network(widths, ir_widths, shared_tms, separate_clocks)
        pdl = self.prepare(source, models, len(widths), body)
        output = self.run_tool(pdl, extra=('--debug',))
        self.assertEqual(output, self.run_tool(self.root / 'result.pdl', 'replay.pdl'))
        flat = ['--icl', 'debug/MultiTapTop.icl']
        self.assertEqual(output, self.run_tool(pdl, 'flat.pdl', inputs=flat))
        self.assertEqual(output, self.run_tool(self.root / 'result.pdl', 'flat-replay.pdl', inputs=flat))
        return output

    def test_unequal_lanes_write_and_read_together(self):
        text = self.replay('iReset; iWrite c0.value 9; iWrite c1.value 85; iApply -together; '
                           'iRead c0.value 9; iRead c1.value 85; iApply;\n')
        self.assertIn('iWrite tap0.INSTRUCTION 0b0011', text)
        self.assertIn('iWrite tap1.INSTRUCTION 0b00011', text)
        self.assertIn('iRead tap0.INSTRUCTION 0b0001', text)
        self.assertIn('iRead tap1.INSTRUCTION 0b10001', text)
        self.assertEqual(text.count('iApply'), 3)

    def test_shared_tms_uses_the_same_phase(self):
        self.replay('iReset; iWrite c0.value 5; iWrite c1.value 42; iApply; '
                    'iRead c0.value 5; iRead c1.value 42; iApply;\n', shared_tms=True)

    def test_unrequested_lane_preserves_taken_data(self):
        self.replay('iReset; iWrite c1.value 85; iApply; iTake c1.value; '
                    'iState c1.value 85; iWrite c0.value 9; iApply; '
                    'iRead c1.value 85; iRead c0.value 9; iApply;\n')

    def test_explicit_ir_comparison_and_implicit_other_ir(self):
        text = self.replay('iReset; iWrite tap0.INSTRUCTION 3; '
                           'iRead tap0.INSTRUCTION 0bxx01; iWrite c1.value 85; iApply;\n')
        self.assertIn('iRead tap0.INSTRUCTION 0bxx01', text)
        self.assertIn('iRead tap1.INSTRUCTION 0b10001', text)

    def test_root_reset_port_is_asserted_and_released_with_tap_reset(self):
        for polarity in (0, 1):
            for synchronous in (False, True):
                with self.subTest(polarity=polarity, synchronous=synchronous):
                    source, models = network((4,), (4,))
                    source = source.replace('TRSTPort TRST0;',
                                            f'TRSTPort TRST0; ResetPort rst {{ ActivePolarity {polarity}; }}')
                    source = source.replace('InputPort reset = TRST0;',
                                            'InputPort reset = ' + ('~rst' if polarity else 'rst') + ';')
                    reset = 'iReset -sync' if synchronous else 'iReset'
                    pdl = self.prepare(source, models, 1, reset + '; iWrite c0.value 9; iApply;\n')
                    text = self.run_tool(pdl, extra=('--debug',))
                    self.assertIn('iWrite c0.value 0b1001', text)
                    self.assertEqual(text, self.run_tool(self.root / 'result.pdl', 'replay.pdl'))

    def test_only_one_requested_lane_keeps_builtin_paths_internal(self):
        text = self.replay('iReset; iWrite c0.value 9; iApply; iRead c0.value 9; iApply;\n')
        self.assertNotIn('iWrite tap1.DEVICE_ID', text)
        self.assertNotIn('iWrite tap1.BYPASS', text)
        self.assertNotIn('iRead tap1.DEVICE_ID', text)
        self.assertNotIn('iRead tap1.BYPASS', text)

    def test_both_instruction_registers_share_a_group(self):
        self.replay('iReset; iWrite tap0.INSTRUCTION 3; iWrite tap1.INSTRUCTION 3; '
                    'iApply -together; iWrite c0.value 9; iWrite c1.value 85; iApply;\n')

    def test_missing_reset_and_virtual_reset_preserve_output(self):
        for implicit in (False, True):
            with self.subTest(implicit=implicit):
                source, models = network()
                if implicit:
                    source = source.replace('InputPort reset = TRST1;', '')
                pdl = self.prepare(source, models, 2,
                                   ('iReset; ' if implicit else '') + 'iWrite c0.value 9; iApply;\n')
                (self.root / 'result.pdl').write_text('previous output\n')
                error = self.run_tool(pdl, ok=False)
                self.assertIn('explicit reset connectivity' if implicit else 'explicit iReset', error)
                self.assertEqual((self.root / 'result.pdl').read_text(), 'previous output\n')

    def test_distinct_equivalent_clock_roots_share_one_phase(self):
        for synchronous in (False, True):
            with self.subTest(synchronous=synchronous):
                reset = 'iReset -sync' if synchronous else 'iReset'
                self.replay(reset + '; iWrite c0.value 9; iWrite c1.value 85; iApply -together; '
                            'iRunLoop 3; iRead c0.value 9; iRead c1.value 85; iApply;\n',
                            separate_clocks=True)

    def test_single_tap_instrument_uses_an_additional_clock_root(self):
        source, models = network((4,), (4,))
        source = source.replace('Module MultiTapTop {', 'Module MultiTapTop { TCKPort extra_clock;')
        source = source.replace('InputPort reset = TRST0;',
                                'InputPort reset = TRST0; InputPort clock = extra_clock;')
        pdl = self.prepare(source, models, 1,
                           'iReset; iWrite c0.value 9; iApply; iRunLoop 2; iRead c0.value 9; iApply;\n')
        text = self.run_tool(pdl, extra=('--debug',))
        self.assertEqual(text, self.run_tool(self.root / 'result.pdl', 'replay.pdl'))
        self.assertEqual(text, self.run_tool(pdl, 'flat.pdl',
                                            inputs=['--icl', 'debug/MultiTapTop.icl']))

    def test_additional_clock_root_retains_gate_qualification(self):
        for gate in ('1', '0', 'x', 'inverted'):
            with self.subTest(gate=gate):
                source, models = network((4,), (4,))
                source = source.replace('Module MultiTapTop {',
                                        'Module MultiTapTop { TCKPort extra_clock; '
                                        + f"LogicSignal gate {{ 1'b{gate if gate != 'inverted' else '1'}; }} "
                                        + "ClockMux gated SelectedBy gate { 1'b0: 1'b0; 1'b1: extra_clock; } "
                                        + "ToTCKPort gated_tck { Source gated; }")
                # Existing lowered ToTCK Source extension forwards the gated clock.
                if gate == 'inverted':
                    source = source.replace("1'b1: extra_clock;", "1'b1: ~extra_clock;")
                source = source.replace('InputPort reset = TRST0;',
                                        'InputPort reset = TRST0; InputPort clock = gated_tck;')
                pdl = self.prepare(source, models, 1, 'iReset; iWrite c0.value 9; iApply;\n')
                if gate == '1':
                    text = self.run_tool(pdl, extra=('--debug',))
                    self.assertEqual(text, self.run_tool(self.root / 'result.pdl', 'replay.pdl'))
                    self.assertEqual(text, self.run_tool(pdl, 'flat.pdl',
                                                        inputs=['--icl', 'debug/MultiTapTop.icl']))
                else:
                    (self.root / 'result.pdl').write_text('previous output\n')
                    error = self.run_tool(pdl, ok=False)
                    if gate == 'inverted':
                        self.assertIn('inverted child TCK phase', error)
                    self.assertEqual((self.root / 'result.pdl').read_text(), 'previous output\n')

    def test_equivalent_root_scale(self):
        for count in (4, 16):
            with self.subTest(count=count):
                widths = tuple(4 + lane % 5 for lane in range(count))
                ir_widths = tuple(4 + lane % 3 for lane in range(count))
                body = 'iReset; ' + ''.join(f'iWrite c{lane}.value {lane % 15 + 1}; '
                                            for lane in range(count)) + 'iApply -together;\n'
                started = time.monotonic()
                text = self.replay(body, widths, ir_widths, separate_clocks=True)
                source, models = network(widths, ir_widths, separate_clocks=True)
                print(f'equivalent TCK scale: roots={count}, model_bytes={len(source) + len(models)}, '
                      f'output_bytes={len(text)}, four_replays_seconds={time.monotonic() - started:.6f}',
                      flush=True)

    def test_mixed_trst_availability_uses_common_reset_clocks(self):
        source, models = network()
        models = models.replace('  attribute TAP_SCAN_RESET of TRST1 : signal is true;\n', '')
        source = source.replace('InputPort reset = TRST1;', 'InputPort reset = TRST0;')
        pdl = self.prepare(source, models, 2,
                           'iReset; iWrite c0.value 9; iWrite c1.value 85; iApply -together;\n')
        text = self.run_tool(pdl, extra=('--debug',))
        self.assertEqual(text, self.run_tool(self.root / 'result.pdl', 'replay.pdl'))
        self.assertEqual(text, self.run_tool(pdl, 'flat.pdl',
                                            inputs=['--icl', 'debug/MultiTapTop.icl']))

    def test_cross_tap_control_and_capture_data_share_one_state(self):
        source, models = network((4, 4), (4, 5))
        source = source.replace('DataOutPort q',
                                'ToCaptureEnPort capture_out { Source capture; }\n DataOutPort q')
        source = source.replace('Parameter W = 4;',
                                'Parameter W = 4; DataInPort observed[$W-1:0];', 1)
        source = source.replace('ScanInSource si; ResetValue 0;',
                                'ScanInSource si; CaptureSource observed; ResetValue 0;')
        source = source.replace('InputPort reset = TRST0;',
                                'InputPort reset = TRST0; InputPort observed = c1.q;')
        source = source.replace('InputPort reset = TRST1;',
                                'InputPort reset = TRST1; InputPort capture = c0.capture_out; '
                                'InputPort observed = c0.q;')
        pdl = self.prepare(source, models, 2,
                           'iReset; iWrite c0.value 5; iWrite c1.value 10; iApply -together; '
                           'iRead c0.value 10; iRead c1.value 5; iApply;\n')
        text = self.run_tool(pdl, extra=('--debug',))
        self.assertEqual(text, self.run_tool(self.root / 'result.pdl', 'replay.pdl'))
        self.assertEqual(text, self.run_tool(pdl, 'flat.pdl',
                                            inputs=['--icl', 'debug/MultiTapTop.icl']))

    def test_shared_serial_storage_is_rejected(self):
        source, models = network()
        source = source.replace('c1.client;', 'c0.client;')
        pdl = self.prepare(source, models, 2, 'iReset; iWrite c0.value 9; iApply;\n')
        (self.root / 'result.pdl').write_text('previous output\n')
        error = self.run_tool(pdl, ok=False)
        self.assertRegex(error, 'multiple|shared|conflicting')
        self.assertEqual((self.root / 'result.pdl').read_text(), 'previous output\n')

    def test_multiple_tap_scale(self):
        for count in (4, 16, 32):
            with self.subTest(count=count):
                widths = tuple(4 + lane % 5 for lane in range(count))
                ir_widths = tuple(4 + lane % 3 for lane in range(count))
                setup = 'iReset; '
                if count == 32:
                    setup += ''.join(f'iWrite tap{lane}.INSTRUCTION 3; ' for lane in range(count))
                    setup += 'iApply -together; '
                body = setup + ''.join(f'iWrite c{lane}.value {lane % ((1 << widths[lane]) - 1) + 1}; '
                                            for lane in range(count)) + 'iApply -together;\n'
                started = time.monotonic()
                text = self.replay(body, widths, ir_widths)
                source, models = network(widths, ir_widths)
                print(f'multiple TAP scale: taps={count}, model_bytes={len(source) + len(models)}, '
                      f'output_bytes={len(text)}, four_replays_seconds={time.monotonic() - started:.6f}',
                      flush=True)

    def test_primary_frames_and_end_state_replay(self):
        for count in (1, 2):
            with self.subTest(count=count):
                source, models = network((7,) * count, (4,) * count)
                target = f'c{count - 1}.value'
                source = source.replace('Module MultiTapTop {',
                                        'Module MultiTapTop { DataInPort pin; DataOutPort pout { Source pin; } '
                                        + f"Alias idle = {target}[0] {{ iApplyEndState 1'b0; }}")
                pdl = self.prepare(source, models, count,
                                   f'iReset; iWrite pin 1; iWrite {target} 85; iApply; '
                                   f'iRead pout 1; iRead {target} 85; iApply;\n')
                text = self.run_tool(pdl, extra=('--debug',))
                self.assertIn('iWrite pin 0b1', text)
                self.assertIn(f'iWrite {target} 0b1010100', text)
                self.assertEqual(text, self.run_tool(self.root / 'result.pdl', 'replay.pdl'))
                self.assertEqual(text, self.run_tool(pdl, 'flat.pdl',
                                                    inputs=['--icl', 'debug/MultiTapTop.icl']))

    def test_hidden_instrument_events_remain_joint(self):
        source, models = network()
        source = source.replace('Source value[0];', 'Source route;')
        source = source.replace('ScanRegister value',
                                "ScanRegister gate { ScanInSource si; ResetValue 1'b0; }\n ScanRegister value")
        source = source.replace('ScanRegister value[$W-1:0] { ScanInSource si;',
                                'ScanRegister value[$W-1:0] { ScanInSource gate;')
        source = source.replace('DataOutPort q',
                                "ScanMux route SelectedBy gate { 1'b0: gate; 1'b1: value[0]; }\n DataOutPort q")
        pdl = self.prepare(source, models, 2,
                           'iReset; iWrite tap0.INSTRUCTION 3; iWrite tap1.INSTRUCTION 3; iApply; '
                           'iWrite c0.gate 1; iWrite c1.gate 1; iApply; '
                           'iWrite c0.value 5; iWrite c1.value 85; iApply; '
                           'iWrite c1.gate 0; iApply; iWrite c0.value 9; iApply; '
                           'iRead c1.value 85; iApply;\n')
        text = self.run_tool(pdl, extra=('--debug',))
        self.assertEqual(text, self.run_tool(self.root / 'result.pdl', 'replay.pdl'))
        self.assertEqual(text, self.run_tool(pdl, 'flat.pdl',
                                            inputs=['--icl', 'debug/MultiTapTop.icl']))

    def test_instruction_frame_expires_unrepresented_scan_override(self):
        source, models = network()
        source = source.replace('Module MultiTapTop {',
                                "Module MultiTapTop { Alias idle = c0.value[0] { iApplyEndState 1'b0; }")
        pdl = self.prepare(source, models, 2,
                           'iReset; iWrite c0.value 9; iApply; '
                           'iWrite tap1.INSTRUCTION 3; iApply;\n')
        text = self.run_tool(pdl, extra=('--debug',))
        self.assertIn('iWrite c0.value 0b1000', text)
        self.assertEqual(text, self.run_tool(self.root / 'result.pdl', 'replay.pdl'))
        self.assertEqual(text, self.run_tool(pdl, 'flat.pdl',
                                            inputs=['--icl', 'debug/MultiTapTop.icl']))

    def test_local_reset_rewrite_keeps_explicit_reset_calls(self):
        source, models = network()
        body = 'iReset; iWrite c0.value 9; iWrite c1.value 85; iApply;\n'
        pdl = self.prepare(source, models, 2, body)
        text = self.run_tool(pdl, extra=('--local_reset', 'reset.pdl'))
        rewritten = (self.root / 'reset.pdl').read_text()
        self.assertEqual(rewritten.count('iReset'), 1)
        self.assertNotIn('tap0.INSTRUCTION', rewritten)
        self.assertNotIn('tap1.INSTRUCTION', rewritten)
        self.assertEqual(text, self.run_tool(self.root / 'reset.pdl', 'reset-replay.pdl'))

    def test_cross_tap_access_together_alias(self):
        source, models = network()
        source = source.replace('Module MultiTapTop {',
                                'Module MultiTapTop { Alias pair[10:0] = c0.value, c1.value { AccessTogether; }')
        pdl = self.prepare(source, models, 2,
                           'iReset; iWrite pair 1237; iApply; iRead pair 1237; iApply;\n')
        text = self.run_tool(pdl, extra=('--debug',))
        self.assertIn('iWrite c0.value 0b1001', text)
        self.assertIn('iWrite c1.value 0b1010101', text)
        self.assertIn('iApply -together', text)
        self.assertEqual(text.count('iApply'), 3)
        self.assertEqual(text, self.run_tool(self.root / 'result.pdl', 'replay.pdl'))
        self.assertEqual(text, self.run_tool(pdl, 'flat.pdl',
                                            inputs=['--icl', 'debug/MultiTapTop.icl']))

    def test_trst_assumptions_remain_rejected(self):
        for value in ('0', '0bx'):
            with self.subTest(value=value):
                source, models = network((4,), (4,))
                body = f'iReset; iState TRST0 {value}; iState TRST0 1; iWrite c0.value 9; iApply;\n'
                pdl = self.prepare(source, models, 1, body)
                previous = b'previous output\n'
                (self.root / 'result.pdl').write_bytes(previous)
                error = self.run_tool(pdl, ok=False)
                self.assertIn('state assumption for TRST0', error)
                self.assertEqual((self.root / 'result.pdl').read_bytes(), previous)


if __name__ == '__main__':
    unittest.main()
