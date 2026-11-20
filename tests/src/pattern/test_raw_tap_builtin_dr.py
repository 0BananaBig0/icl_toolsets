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

    def configure_bsdl(self, opcodes='', registers='', attributes=''):
        text = (dr.ir.tap.ROOT / 'tests/cases/ours/pattern_generation/raw_tap_builtin_dr/model.bsdl').read_text()
        text = text.replace('SAMPLE (0001)', 'SAMPLE (0001), PRELOAD (0001)')
        if opcodes:
            text = text.replace('IJTAG_OFF (0100)', 'IJTAG_OFF (0100), ' + opcodes)
        if registers:
            text = text.replace('DEVICE_ID (IDCODE)', 'DEVICE_ID (IDCODE), ' + registers)
        if attributes:
            anchor = ('  attribute IDCODE_REGISTER' if 'INSTRUCTION_PRIVATE' in attributes
                      else '  attribute REGISTER_ACCESS')
            text = text.replace(anchor, '  ' + attributes + '\n' + anchor)
        path = self.root / 'model.bsdl'
        path.write_text(text)
        self.inputs[-1] = str(path)
        return path

    def test_custom_capture_and_reused_width(self):
        self.configure_bsdl('CUSTOM_A (0101), CUSTOM_B (0110), CUSTOM_C (0111)',
            'USERREG[2] (CUSTOM_A CAPTURES 10), userreg (custom_b CAPTURES 01, CUSTOM_C)')
        self.replay('iReset; iScan -ir jtag 4 -si 5 -so 1; iApply; '
                    'iScan jtag 2 -si 1 -so 2; iApply; '
                    'iScan -ir jtag 4 -si 6 -so 1; iApply; '
                    'iScan jtag 2 -si 2 -so 1; iApply; '
                    'iScan -ir jtag 4 -si 7; iApply; '
                    'iScan jtag 2 -si 0; iApply\n')

    def test_repeated_opcodes_and_casefolded_bindings(self):
        self.configure_bsdl('CUSTOM_A (0101), custom_a (0111), CUSTOM_A (0101)',
                            'USERREG[2] (Custom_A CAPTURES 10)')
        path = self.root / 'model.bsdl'
        path.write_text(path.read_text().replace('BYPASS', 'bypass'))
        model = self.root / 'model.icl'
        model.write_text(dr.pathlib.Path(self.inputs[1]).read_text().replace('IJTAG_ON {', 'ijtag_on {'))
        self.inputs[1] = str(model)
        self.replay('iReset; iScan -ir jtag 4 -si 7; iApply; '
                    'iScan jtag 2 -si 0 -so 2; iApply; '
                    'iWrite core.r 7; iApply\n')

    def test_implicit_bypass_with_and_without_identification(self):
        for identification in (True, False):
            with self.subTest(identification=identification):
                path = self.configure_bsdl()
                text = path.read_text().replace('BYPASS (1111), ', '').replace(', BYPASS (BYPASS)', '')
                if not identification:
                    text = text.replace('IDCODE (0010), ', '').replace(', DEVICE_ID (IDCODE)', '')
                    start = text.index('  attribute IDCODE_REGISTER')
                    end = text.index(';', start) + 1
                    text = text[:start] + text[end:]
                path.write_text(text)
                body = ('iReset; iScan -ir jtag 4 -si 15; iApply; ' if identification
                        else 'iReset; ')
                self.replay(body + 'iScan jtag 1 -si 1 -so 0; iApply\n')

    def test_private_omission_and_explicit_private_route(self):
        path = self.configure_bsdl('SECRET (1000)', '',
            'attribute INSTRUCTION_PRIVATE of BuiltinDrProbe : entity is "Secret";')
        self.replay('iReset; iScan -ir jtag 4 -si 15; iApply; '
                    'iScan jtag 1 -si 1 -so 0; iApply\n')
        self.rejected('iReset; iScan -ir jtag 4 -si 8; iApply; '
                      'iScan jtag 1; iApply\n', 'known complete register route')
        model = self.root / 'private.icl'
        model.write_text(dr.pathlib.Path(self.inputs[1]).read_text().replace('    IJTAG_OFF {',
            '    SECRET { ScanInterface {core.client;} ActiveSignals {long_setup;} }\n    IJTAG_OFF {'))
        self.inputs[1] = str(model)
        self.replay('iReset; iScan -ir jtag 4 -si 8; iApply; '
                    'iScan jtag 8 -si 1 -so 0; iApply\n')

    def test_capture_pattern_lists_keep_only_common_bits(self):
        path = self.configure_bsdl('USERCODE (0101)', '',
            'attribute USERCODE_REGISTER of BuiltinDrProbe : entity is '
            '"10101010101010101010101010101010, 10101010101010101010101010101000";')
        text = path.read_text().replace('is "0001";', 'is "0001,1001";')
        text = text.replace('"0001" & "0000000000000000" & "00000000001" & "1"',
            '"00010000000000000000000000000011, 00100000000000000000000000000011"')
        path.write_text(text)
        self.replay('iReset; iScan jtag 32 -si 0 -so 0b00xx0000000000000000000000000011; iApply; '
                    'iScan -ir jtag 4 -si 5 -so 0bx001; iApply; '
                    'iScan jtag 32 -si 0 -so 0b101010101010101010101010101010x0; iApply\n')

    def test_bad_capture_list_members_are_rejected(self):
        for source, replacement in (
                ('is "0001";', 'is "0001,0000";'),
                ('is "0001";', 'is "0001,1001,";'),
                ('"0001" & "0000000000000000" & "00000000001" & "1"',
                 '"00010000000000000000000000000011, 00010000000000000000000000000010"')):
            with self.subTest(replacement=replacement):
                path = self.configure_bsdl()
                path.write_text(path.read_text().replace(source, replacement))
                self.rejected('iReset\\n', 'capture')

    def test_shared_capture_masks_are_constraints(self):
        self.configure_bsdl('USERCODE (0010)', '',
            'attribute USERCODE_REGISTER of BuiltinDrProbe : entity is '
            '"XXXX0000000000000000000000000011";')
        self.replay('iReset; iScan jtag 32 -si 0 -so 0x10000003; iApply\n')

    def test_invalid_private_capture_and_width_metadata(self):
        cases = [
            ('SECRET (1000)', '', 'attribute INSTRUCTION_PRIVATE of BuiltinDrProbe : entity is "SECRET, secret";', 'private'),
            ('', '', 'attribute INSTRUCTION_PRIVATE of BuiltinDrProbe : entity is "UNKNOWN";', 'private'),
            ('', '', 'attribute INSTRUCTION_PRIVATE of BuiltinDrProbe : entity is "BYPASS";', 'private'),
            ('CUSTOM_A (0101)', 'USERREG[2] (CUSTOM_A CAPTURES 1)', '', 'capture'),
            ('CUSTOM_A (0101), CUSTOM_B (0110)', 'USERREG[2] (CUSTOM_A), USERREG[3] (CUSTOM_B)', '', 'width'),
            ('CUSTOM_A (0101)', 'USERREG (CUSTOM_A)', '', 'width'),
        ]
        for opcodes, registers, attributes, message in cases:
            with self.subTest(message=message, registers=registers, attributes=attributes):
                self.configure_bsdl(opcodes, registers, attributes)
                self.rejected('iReset\n', message)
        path = self.configure_bsdl()
        path.write_text(path.read_text().replace('BYPASS (BYPASS)', 'BYPASS (BYPASS CAPTURES 0)'))
        self.rejected('iReset\n', 'CAPTURES')

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
