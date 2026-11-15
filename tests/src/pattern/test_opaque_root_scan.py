#!/usr/bin/env python3
"""IEEE1687 root opaque iScan lowering, replay and atomic boundaries."""
import pathlib
import subprocess
import sys
import tempfile
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[3]
BIN = pathlib.Path(sys.argv.pop(1)).resolve() if len(sys.argv) > 1 else ROOT / 'build/icl-pattern-generation'
MODEL = (ROOT / 'tests/cases/ours/pattern_generation/opaque_root_scan/model.icl').read_text().replace('Module Opaque', 'Module Top')


class OpaqueRootScan(unittest.TestCase):
    def setUp(self):
        temporary = tempfile.TemporaryDirectory()
        self.addCleanup(temporary.cleanup)
        self.root = pathlib.Path(temporary.name)

    def run_tool(self, icl, pdl, output='result.pdl', ok=True, reset=False):
        command = [str(BIN), '--icl', str(icl), '--pdl', str(pdl),
                   '--top_module', 'Top', '--output', str(self.root / output), '--debug']
        if reset:
            command += ['--local_reset', str(self.root / 'reset.pdl')]
        result = subprocess.run(command, text=True, capture_output=True, timeout=15)
        self.assertEqual(result.returncode == 0, ok, result.stdout + result.stderr)
        return (self.root / output).read_text() if ok else result.stdout + result.stderr

    def files(self, commands, model=MODEL):
        icl, pdl = self.root / 'source.icl', self.root / 'source.pdl'
        icl.write_text(model)
        pdl.write_text('iPDLLevel 1\niProcsForModule Top\n' + commands)
        return icl, pdl

    def replay(self, commands, model=MODEL):
        icl, pdl = self.files(commands, model)
        original = self.run_tool(icl, pdl, reset=True)
        flat = self.root / 'debug/Top.icl'
        self.assertEqual(original, self.run_tool(flat, pdl, 'flat/result.pdl'))
        self.assertEqual(original, self.run_tool(icl, self.root / 'result.pdl', 'replay/result.pdl'))
        self.assertEqual(original, self.run_tool(flat, self.root / 'result.pdl', 'both/result.pdl'))
        self.assertEqual(original, self.run_tool(icl, self.root / 'reset.pdl', 'reset/result.pdl'))
        self.assertEqual(pdl.read_text(), (self.root / 'reset.pdl').read_text())
        return original

    def rejected(self, commands, diagnostic, model=MODEL):
        icl, pdl = self.files(commands, model)
        artifacts = [self.root / 'result.pdl', self.root / 'reset.pdl', self.root / 'debug/Top.icl']
        for artifact in artifacts:
            artifact.parent.mkdir(parents=True, exist_ok=True)
            artifact.write_text('previous artifact\n')
        message = self.run_tool(icl, pdl, ok=False, reset=True)
        self.assertIn(diagnostic, message)
        for artifact in artifacts:
            self.assertEqual(artifact.read_text(), 'previous artifact\n')

    def test_payloads_and_padding(self):
        result = self.replay('iReset; iScan client 8 -si 0xa5 -so 0x3c -stable; iApply\n'
                             'iScan client 8 -si 3 -so 0b1; iApply\n'
                             'iScan client 8; iApply\n')
        self.assertIn('iScan client 8 -si 0b10100101 -so 0b00111100 -stable\niApply', result)
        self.assertIn('iScan client 8 -si 0b00000011 -so 0b00000001\niApply', result)
        self.assertIn('iScan client 8 -si 0b00000000 -so 0bxxxxxxxx\niApply', result)

    def test_required_default_load(self):
        self.rejected('iReset; iScan client 8; iApply\n', 'DefaultLoadValue',
                      MODEL.replace("    DefaultLoadValue 8'b00000000;\n", ''))
        self.rejected('iScan client 8; iApply\n', 'DefaultLoadValue',
                      MODEL.replace("8'b00000000", '0'))

    def test_based_length(self):
        result = self.replay('iScan client 0x8 -si 0x5a; iApply\n'
                             'iScan client 0b100 -si 0b1010; iApply\n')
        self.assertIn('iScan client 8 -si 0b01011010', result)
        self.assertIn('iScan client 4 -si 0b1010', result)

    def test_grouped_data_procedure_and_variable_length(self):
        result = self.replay("""iProc scan {data} {
 iScan client 8 -si $data -so {0bxx00 11xx}; iApply
}
iCall scan {0b1010 0101}
iScan client 12 -so 0xXXX -si { 0xA
 55 } -stable; iApply
""")
        self.assertIn('iScan client 8 -si 0b10100101 -so 0bxx0011xx', result)
        self.assertIn('iScan client 12 -si 0b101001010101 -so 0bxxxxxxxxxxxx -stable', result)
        self.assertNotIn('iReset', result)

    def test_queue_overwrite_reset_and_empty_apply(self):
        result = self.replay('iScan client 8 -si 1; iScan client 8 -si 2; iApply; iApply\n'
                             'iScan client 8 -si 3; iReset; iApply\n'
                             'iScan client 4 -si 4; iApply; iApply\n')
        self.assertEqual(result.count('iScan '), 2)
        self.assertNotIn('0b00000001', result)
        self.assertNotIn('0b00000011', result)
        self.assertIn('iScan client 4 -si 0b0100 -so 0bxxxx', result)

    def test_all_scans_discarded_replay(self):
        result = self.replay('iScan client 8; iReset; iApply\n')
        self.assertNotIn('iScan', result)
        self.assertTrue(result.endswith('iReset\n'))

    def test_stable_does_not_repeat_comparisons(self):
        result = self.replay('iScan client 8 -si 0x5a -so 0x3c -stable; iApply; iApply; iApply\n')
        self.assertEqual(result.count('iScan '), 1)
        self.assertEqual(result.count('0b00111100'), 1)

    def test_note_order_and_together(self):
        result = self.replay('iNote before; iScan client 8 -si 1; iNote pending; iApply -together; iNote after\n')
        self.assertIn('iNote before\niNote pending\niScan client', result)
        self.assertTrue(result.endswith('iApply -together\niNote after\n'))

    def test_numeric_bounds_and_discarded_validation(self):
        for scan in ['iScan client 0', 'iScan client 1048577', 'iScan client 8 -si 0b000000000',
                     'iScan client 8 -so 0bxxxxxxxxx', 'iScan client 6 -si 0xfd',
                     'iScan client 8 -si 256', 'iScan client 8 -so 0xXXX']:
            with self.subTest(scan=scan):
                self.rejected(scan + '; iReset\n', 'scan')
        self.rejected('iScan client 8 -si 256; iScan client 8 -si 0; iApply\n', 'scan')

    def test_target_and_unsupported_interactions(self):
        for command in ['iScan missing 8', 'iScan si 8', 'iScan -ir client 8',
                        'iScan client -chain default 8', 'iScan client 8 -skip_model_state_simulation']:
            with self.subTest(command=command):
                self.rejected(command + '; iApply\n', 'scan')
        for command in ['iTake client', 'iState sel 1', 'iClock tck',
                        'iOverrideScanInterface client -capture off', 'iRunLoop 2',
                        'iMerge -begin; iMerge -end']:
            with self.subTest(command=command):
                self.rejected(command + '; iScan client 8; iApply\n', 'opaque')

    def test_modeled_or_conditional_network_rejected(self):
        visible = MODEL.replace('ScanOutPort so;',
                                "ScanRegister value[7:0] { ScanInSource si; ResetValue 8'b0; }\n"
                                'ScanOutPort so { Source value[0]; }')
        self.rejected('iScan client 8; iApply\n', 'opaque', visible)
        computed = MODEL.replace('ResetPort reset;', 'ResetPort reset { ActivePolarity 0; }')
        self.rejected('iScan client 8; iApply\n', 'opaque',
                      MODEL.replace('ScanOutPort so;', 'ScanOutPort so { Enable sel; }'))
        # Direct active-low pins are still directly controllable, not computed.
        self.assertIn('iScan client', self.replay('iScan client 8; iApply\n', computed))

    def test_large_scan_linear_lowering(self):
        width = 1048576
        data = '0x' + 'a5' * (width // 8)
        result = self.replay('iScan client ' + str(width) + ' -si ' + data + ' -stable; iApply\n')
        self.assertIn('-si 0b' + '10100101' * (width // 8), result)
        self.assertEqual(result.count('iScan '), 1)


if __name__ == '__main__':
    unittest.main()
