#!/usr/bin/env python3
"""Ordinary instruction replay preserves masks, fill and fallback behavior."""
import unittest

import test_raw_tap_ir_scan as ir


class NativeTapIr(unittest.TestCase):
    run_tool = ir.RawTapIrScan.run_tool
    source = ir.RawTapIrScan.source
    replay = ir.RawTapIrScan.replay
    rejected = ir.RawTapIrScan.rejected

    def setUp(self):
        ir.RawTapIrScan.setUp(self)

    def test_already_equal_readonly_and_partial_fill(self):
        result = self.replay('iReset; '
            'iWrite tap.INSTRUCTION 3; iRead tap.INSTRUCTION 1; iApply; '
            'iWrite tap.INSTRUCTION 3; iRead tap.INSTRUCTION 1; iApply; '
            'iRead tap.INSTRUCTION 0bxx01; iApply -together; '
            'iWrite tap.INSTRUCTION[0] 0; iRead tap.INSTRUCTION 1; iApply; '
            'iScan -ir jtag 4 -si 3 -so 0bxx01; iApply; '
            'iWrite core.r 0xa5; iRead core.r 0; iApply\n')
        self.assertIn('iWrite tap.INSTRUCTION 0b0010', result)
        self.assertIn('iRead tap.INSTRUCTION 0bxx01', result)
        self.assertIn('iWrite core.r 0b10100101', result)

    def test_authored_comparison_and_owned_instruction(self):
        result = self.replay('iReset; iTake tap.INSTRUCTION; '
            'iWrite tap.INSTRUCTION 3; iRead tap.INSTRUCTION 15; iApply; '
            'iRunLoop 1 -tck; '
            'iWrite tap.INSTRUCTION[0] 0; iApply; '
            'iRead tap.INSTRUCTION 0bxx01; iApply\n')
        self.assertIn('iRead tap.INSTRUCTION 0b1111', result)
        self.assertIn('iWrite tap.INSTRUCTION 0b0010', result)

    def test_unknown_instruction_recovers_without_extra_comparisons(self):
        result = self.replay('iReset; '
            'iScan -ir jtag 4 -si 0bx011 -so 0bxx01; iApply; '
            'iWrite tap.INSTRUCTION 3; iApply; iWrite core.r 5; iApply\n')
        self.assertNotIn('iRead tap.INSTRUCTION', result)
        self.assertIn('iWrite tap.INSTRUCTION 0b0011', result)

    def test_noncanonical_capture_and_direct_reset_keep_fallback(self):
        original = (ir.FIXTURE / 'model.icl').read_text()
        disabled = original.replace('InputPort mode = tap.long_setup;',
            "InputPort mode = tap.long_setup; InputPort capture = 1'b0;")
        reset = original.replace('DataInPort mode;', 'DataInPort mode; ResetPort rst;')
        reset = reset.replace('  ScanInPort TDI;', '  ResetPort reset; ScanInPort TDI;')
        reset = reset.replace('InputPort mode = tap.long_setup;',
                              'InputPort mode = tap.long_setup; InputPort rst = reset;')
        for text in [disabled, reset]:
            with self.subTest(model=text):
                model = self.root / 'custom.icl'
                model.write_text(text)
                self.inputs[1] = str(model)
                self.replay('iReset; iWrite tap.INSTRUCTION 3; '
                            'iRead tap.INSTRUCTION 1; iApply; '
                            'iRead tap.INSTRUCTION 0bxx01; iApply\n')

    def test_primary_goal_and_end_state_are_not_dropped(self):
        model = self.root / 'primary.icl'
        model.write_text((ir.FIXTURE / 'model.icl').read_text().replace(
            'Module RawIrTop {',
            "Module RawIrTop {\n DataInPort marker; Alias idle = marker {iApplyEndState 1'b0;}"))
        self.inputs[1] = str(model)
        result = self.replay('iReset; iState marker 0; iWrite tap.INSTRUCTION 3; '
            'iRead tap.INSTRUCTION 1; iWrite marker 1; iApply; '
            'iRead tap.INSTRUCTION 0bxx01; iApply\n')
        self.assertIn('iWrite marker 0b1', result)
        self.assertIn('iWrite marker 0b0', result)


if __name__ == '__main__':
    unittest.main()
