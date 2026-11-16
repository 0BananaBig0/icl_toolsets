#!/usr/bin/env python3
"""Literal unknown input survives qualified raw TAP replay."""
import unittest

import test_raw_tap_dr_scan as dr
import test_raw_tap_ir_scan as ir


class UnknownTapInput(unittest.TestCase):
    run_tool = ir.RawTapIrScan.run_tool
    source = ir.RawTapIrScan.source
    replay = ir.RawTapIrScan.replay
    rejected = ir.RawTapIrScan.rejected

    def setUp(self):
        ir.RawTapIrScan.setUp(self)

    def use_data(self):
        self.inputs = ['--icl', str(dr.FIXTURE / 'model.icl'), '--bsdl',
                       'RawDrProbe', str(dr.FIXTURE / 'model.bsdl')]
        self.source = dr.RawTapDrScan.source.__get__(self)
        self.replay = dr.RawTapDrScan.replay.__get__(self)

    def test_literal_dr_unknown_input(self):
        self.use_data()
        prefix = 'iReset; iWrite tap.INSTRUCTION 3; iApply; '
        self.replay(prefix + 'iScan jtag 8 -si 0x94 -so 0; iApply\n')
        result = self.replay(prefix +
            'iScan jtag 8 -si 0b10x101x0 -so 0; iApply; '
            'iWrite core.r[1] 1; iRead core.r 0b10x101x0; iApply\n')
        self.assertIn('iScan jtag 8 -si 0b10x101x0 -so 0b00000000', result)
        self.assertIn('iWrite core.r 0b10010110', result)

    def test_literal_ir_unknown_input(self):
        self.replay('iReset; iScan -ir jtag 4 -si 3 -so 0bxx01; iApply\n')
        result = self.replay('iReset; '
            'iScan -ir jtag 4 -si 0bx011 -so 0bxx01; iApply; '
            'iScan -ir jtag 4 -si 3 -so 0bxx01; iApply; '
            'iWrite core.r 0xa5; iRead core.r 0; iApply\n')
        self.assertIn('iScan -ir jtag 4 -si 0bx011 -so 0bxx01', result)
        self.assertIn('iWrite core.r 0b10100101', result)

    def test_all_unknown_and_retained_fill(self):
        self.replay('iReset; iScan -ir jtag 4 -si 0bxxxx; iApply; '
                    'iScan -ir jtag 4 -si 3; iApply; iWrite core.r 5; iApply\n')
        self.use_data()
        result = self.replay('iReset; iWrite tap.INSTRUCTION 3; iApply; '
            'iWrite core.r 0xff; iApply; '
            'iScan jtag 8 -si 0bxxxxxxxx -so 0xff; iApply; '
            'iWrite core.r[0] 0; iRead core.r 0bxxxxxxxx; iApply\n')
        self.assertIn('-si 0bxxxxxxxx -so 0b11111111', result)
        self.assertIn('iWrite core.r 0b11111110', result)

    def test_queue_order_overwrite_cancel_and_calls(self):
        self.use_data()
        setup = 'iWrite tap.INSTRUCTION 3; '
        raw = 'iScan jtag 8 -si 0b10x101x0 -so 0; '
        reference = self.replay('iReset; ' + setup + 'iApply; ' + raw + 'iApply\n')
        for body in [setup + raw, raw + setup,
                     'iScan jtag 9 -si 0bx; ' + raw + setup]:
            self.assertEqual(self.replay('iReset; ' + body + 'iApply\n'), reference)
        self.replay('iScan jtag 9 -si 0bx; iReset; iApply\n')
        self.assertEqual(self.replay('iProc load {} {' + raw + '}\n'
            'iReset; ' + setup + 'iApply; iCall load; iApply\n'), reference)

    def test_protected_unknown_rejects_and_partial_owner_succeeds(self):
        self.rejected('iReset; iTake tap.INSTRUCTION; iWrite tap.INSTRUCTION 3; '
                      'iApply; iScan -ir jtag 4 -si 0bx011; iApply\n', 'unknown')
        self.use_data()
        self.rejected('iReset; iWrite tap.INSTRUCTION 3; iApply; '
                      'iTake core.r; iWrite core.r 0x55; iApply; '
                      'iScan jtag 8 -si 0b10x101x0; iApply\n', 'unknown')
        self.replay('iReset; iWrite tap.INSTRUCTION 3; iApply; '
                    'iTake core.r[0]; iState core.r[0] 0; '
                    'iScan jtag 8 -si 0b10x101x0 -stable; iApply; '
                    'iWrite core.r[1] 1; iRead core.r 0b10x101x0; iApply\n')

    def test_comparison_is_authored_and_noncanonical_capture_rejects(self):
        self.use_data()
        result = self.replay('iReset; iWrite tap.INSTRUCTION 3; iApply; '
                            'iScan jtag 8 -si 0bxxxxxxxx -so 1; iApply\n')
        self.assertIn('-si 0bxxxxxxxx -so 0b00000001', result)
        model = self.root / 'disabled.icl'
        model.write_text((dr.FIXTURE / 'model.icl').read_text().replace(
            'InputPort mode = tap.long_setup;',
            "InputPort mode = tap.long_setup; InputPort capture = 1'b0;"))
        self.inputs[1] = str(model)
        self.rejected('iReset; iWrite tap.INSTRUCTION 3; iApply; '
                      'iScan jtag 8 -si 0bxxxxxxxx; iApply\n', 'qualified')

    def test_stable_unknown_post_route_rejects(self):
        self.use_data()
        model = self.root / 'feedback.icl'
        text = (dr.FIXTURE / 'model.icl').read_text().replace(
            '  ScanRegister r[7:0] { ScanInSource si;',
            "  ScanRegister extra {ScanInSource si; CaptureSource extra; ResetValue 0;}\n"
            "  ScanMux feedback SelectedBy r[0] {1'b0: si; 1'b1: extra;}\n"
            '  ScanRegister r[7:0] { ScanInSource feedback;')
        model.write_text(text)
        self.inputs[1] = str(model)
        self.rejected('iReset; iWrite tap.INSTRUCTION 3; iApply; '
                      'iScan jtag 8 -si 0b0000000x -stable; iApply\n', 'stable')
        self.replay('iReset; iWrite tap.INSTRUCTION 3; iApply; '
                    'iScan jtag 8 -si 0b10x101x0 -stable; iApply\n')

    def test_actual_unknown_cannot_bypass_end_state(self):
        self.use_data()
        model = self.root / 'end-state.icl'
        model.write_text((dr.FIXTURE / 'model.icl').read_text().replace(
            '  ScanMux mux', "  Alias idle = r[5] {iApplyEndState 1'b0;}\n  ScanMux mux"))
        self.inputs[1] = str(model)
        self.rejected('iReset; iWrite tap.INSTRUCTION 3; iApply; '
                      'iScan jtag 8 -si 0b10x101x0; iApply\n', 'qualified')
        self.replay('iReset; iWrite tap.INSTRUCTION 3; iApply; '
                    'iScan jtag 8 -si 0b100101x0; iApply\n')

    def test_builtin_unknown_input_keeps_private_targets_private(self):
        import test_raw_tap_builtin_dr as builtin
        builtin.RawTapBuiltinDr.setUp(self)
        self.source = builtin.RawTapBuiltinDr.source.__get__(self)
        self.replay = builtin.RawTapBuiltinDr.replay.__get__(self)
        result = self.replay('iReset; iScan jtag 32 -si 0bx -so 0x10000003; iApply; '
            'iScan -ir jtag 4 -si 15; iApply; '
            'iScan jtag 1 -si 0bx -so 0; iApply; '
            'iScan jtag 1 -si 1 -so 0; iApply\n')
        self.assertIn('-si 0b' + 'x' * 32, result)
        self.assertIn('iScan jtag 1 -si 0bx -so 0b0', result)
        self.assertNotIn('iWrite tap.DEVICE_ID', result)
        self.assertNotIn('iWrite tap.BYPASS', result)
        self.rejected('iReset; iScan jtag 32 -si 0bx; iApply; '
                      'iWrite tap.DEVICE_ID 0; iApply\n', 'synthetic')

    def test_wide_instruction_masks_and_existing_event_guard(self):
        import benchmark_raw_tap_ir_scan as bench
        _, model, bsdl, _, _ = next(row for row in bench.scenarios()
                                    if row[0] == 'w32-nodes1-repeat1')
        (self.root / 'wide.icl').write_text(model)
        (self.root / 'wide.bsdl').write_text(bsdl)
        self.inputs[1] = str(self.root / 'wide.icl')
        self.inputs[-1] = str(self.root / 'wide.bsdl')
        self.replay('iReset; iScan -ir jtag 32 -si 0b' +
                    'x' * 17 + '0' * 13 + '11; iApply; '
                    'iScan -ir jtag 32 -si 3; iApply\n')
        self.replay('iReset; iScan -ir jtag 32 -si 0b' + 'x' + '0' * 29 +
                    '11; iApply; iScan -ir jtag 32 -si 3; iApply\n')
        _, model, bsdl, _, _ = next(row for row in bench.scenarios()
                                    if row[0] == 'w4096-nodes1-repeat1')
        (self.root / 'wide.icl').write_text(model)
        (self.root / 'wide.bsdl').write_text(bsdl)
        self.rejected('iReset; iScan -ir jtag 4096 -si 0bx; iApply\n', 'qualified')

    def test_unequal_register_parity_aliases_and_ascending_indices(self):
        self.use_data()
        fixture = ir.tap.ROOT / 'tests/cases/ours/pattern_generation/raw_tap_multiple_dr'
        self.inputs[-2:] = ['MultipleDrProbe', str(fixture / 'model.bsdl')]
        for flags in range(8):
            for ascending in [False, True]:
                with self.subTest(flags=flags, ascending=ascending):
                    si, between, so = bool(flags & 1), bool(flags & 2), bool(flags & 4)
                    text = (fixture / 'model.icl').read_text().replace(
                        'Module MultipleDrTop {', 'Module RawDrTop {')
                    text = text.replace('ScanInSource si; CaptureSource a;',
                        'ScanInSource ' + ('~si' if si else 'si') + '; CaptureSource a;')
                    text = text.replace('  ScanRegister b',
                        '  Alias hop[2:0] = a; Alias forwarding[2:0] = hop;\n  ScanRegister b')
                    text = text.replace('ScanInSource a[0];',
                        'ScanInSource ' + ('~' if between else '') + 'forwarding[0];')
                    text = text.replace('Source mux;', 'Source ' + ('~' if so else '') + 'mux;')
                    if ascending:
                        text = text.replace('a[2:0]', 'a[0:2]').replace('b[4:0]', 'b[0:4]')
                        text = text.replace('b[0];', 'b[4];')
                    model = self.root / 'parity.icl'
                    model.write_text(text)
                    self.inputs[1] = str(model)
                    invert = lambda bits, odd: ''.join(
                        c if c == 'x' or not odd else '1' if c == '0' else '0' for c in bits)
                    a, b = invert('10x', si), invert('101x0', si != between)
                    result = self.replay('iReset; iWrite tap.INSTRUCTION 3; iApply; '
                        'iScan jtag 8 -si 0b10x101x0 -stable; iApply; '
                        f'iRead core.a 0b{a}; iRead core.b 0b{b}; iApply\n')
                    self.assertIn('-si 0b10x101x0 -so 0bxxxxxxxx', result)


if __name__ == '__main__':
    unittest.main()
