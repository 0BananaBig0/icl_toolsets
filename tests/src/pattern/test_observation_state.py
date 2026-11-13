#!/usr/bin/env python3
"""History-only iState survives generation without changing device planning."""
import subprocess
import time
import unittest

import test_simultaneous_scan_chains as helpers
from test_data_register_state import BASE as DATA

SCAN = """Module Chains {
 ScanInPort si;
 ScanRegister value[3:0] { ScanInSource si; CaptureSource value; ResetValue 0; }
 ScanOutPort so { Source value[0]; }
 Alias inverted[3:0] = ~value;
}
"""
FLAGS = ('-LastReadValue', '-LastMiscompareValue')


class ObservationState(unittest.TestCase):
    setUp = helpers.SimultaneousChains.setUp
    run_tool = helpers.SimultaneousChains.run_tool
    replay = helpers.SimultaneousChains.replay

    def test_history_does_not_change_scan_state_or_fill(self):
        for flag in FLAGS:
            out = self.replay(SCAN, f'iReset; iState value 9 {flag}; '
                              'iRead value 0; iApply;\n')
            self.assertIn(f'iState value 0b1001 {flag}'.encode(), out)
            self.assertIn(b'iRead value 0b0000', out)
            self.assertNotIn(b'iWrite value 0b1001', out)

    def test_history_does_not_replace_pending_values(self):
        for flag in FLAGS:
            out = self.replay(SCAN, 'iReset; iWrite value 5; iRead value 0; '
                              f'iState value 9 {flag}; iApply;\n')
            self.assertIn(b'iWrite value 0b0101', out)
            self.assertIn(b'iRead value 0b0000', out)
            self.assertEqual(out.count(b'iApply'), 1)

    def test_history_only_has_no_device_transaction(self):
        for flag in FLAGS:
            out = self.replay(DATA, f'iState value 0b1x01 {flag};\n')
            self.assertNotIn(b'iApply', out)
            self.assertNotIn(b'iWrite', out)
            self.assertIn(f'iState value 0b1x01 {flag}'.encode(), out)

    def test_data_and_primary_history_do_not_change_snapshot(self):
        for flag in FLAGS:
            out = self.replay(DATA, 'iState enable 0; iState value 6; '
                              f'iState enable 1 {flag}; iState value 9 {flag}; '
                              'iRead q 6; iApply;\n')
            self.assertNotIn(b'iWrite', out)
            self.assertIn(b'iRead q 0b0110', out)

    def test_history_does_not_lock_taken_unwritten_resources(self):
        for flag in FLAGS:
            out = self.replay(DATA, 'iReset; iState enable 1; iTake value; '
                              f'iState value 9 {flag}; iWrite value 6; iApply; '
                              'iRead q 6; iApply;\n')
            self.assertIn(b'iRead q 0b0110', out)

    def test_taken_unwritten_gate_remains_automatically_writable(self):
        source = SCAN.replace(' ScanRegister value',
                              ' ScanRegister gate { ScanInSource si; ResetValue 0; }\n ScanRegister value')
        source = source.replace('ScanInSource si; CaptureSource value',
                                'ScanInSource gate; CaptureSource value')
        source = source.replace(' ScanOutPort so { Source value[0]; }',
                                " ScanMux route SelectedBy gate { 1'b0: gate; 1'b1: value[0]; }\n"
                                ' ScanOutPort so { Source route; }')
        for flag in FLAGS:
            out = self.replay(source, f'iReset; iTake gate; iState gate 0 {flag}; '
                              'iWrite value 9; iApply;\n')
            self.assertIn(b'iWrite gate 0b1', out)

    def test_state_only_does_not_restore_end_state(self):
        source = """Module Chains {
 DataInPort pin;
 Alias idle = pin { iApplyEndState 1'b0; }
}
"""
        for flag in FLAGS + ('-LastWrittenValue',):
            out = self.replay(source, f'iState pin 1 {flag};\n')
            self.assertNotIn(b'iApply', out)
            self.assertNotIn(b'iWrite', out)

    def test_reset_control_history_still_requires_real_release(self):
        source = """Module Child {
 ScanInPort si; ResetPort rst { ActivePolarity 0; }
 ScanRegister value[3:0] { ScanInSource si; CaptureSource value; ResetValue 0; }
 ScanOutPort so { Source value[0]; }
}
Module Chains {
 ScanInPort si;
 ScanRegister gate { ScanInSource si; CaptureSource gate; ResetValue 0; }
 Instance c Of Child { InputPort si=gate; InputPort rst=gate; }
 ScanOutPort so { Source c.so; }
}
"""
        for flag in FLAGS:
            icl, pdl = self.root / 'source.icl', self.root / 'source.pdl'
            icl.write_text(source)
            pdl.write_text(f'iReset; iState gate 1 {flag}; iWrite c.value 9; iApply;\n')
            output, reset = self.root / 'result.pdl', self.root / 'reset.pdl'
            result = subprocess.run([str(helpers.BIN), '--icl', str(icl), '--pdl', str(pdl),
                                     '--top_module', 'Chains', '--output', str(output),
                                     '--local_reset', str(reset)], capture_output=True, text=True, timeout=45)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            self.assertIn('iWrite gate 0b1', reset.read_text())
            self.assertIn(flag, reset.read_text())
            self.assertEqual(output.read_bytes(), self.run_tool(icl, reset, 'again.pdl'))

    def test_instruction_history_does_not_assume_controller_state(self):
        fixture = helpers.ROOT / 'tests/cases/ours/pattern_generation/access_link'
        pdl, output = self.root / 'source.pdl', self.root / 'result.pdl'
        args = [str(helpers.BIN), '--icl', str(fixture / 'simple.icl'),
                '--bsdl', 'AccessProbe', str(fixture / 'simple.bsdl'),
                '--pdl', str(pdl), '--output', str(output), '--debug']
        for flag in FLAGS:
            pdl.write_text(f'iState tap.INSTRUCTION 0 {flag};\n')
            result = subprocess.run(args, capture_output=True, text=True, timeout=45)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            self.assertIn(flag, output.read_text())
            self.assertNotIn('iApply', output.read_text())
            pdl.write_bytes(output.read_bytes())
            again = subprocess.run(args, capture_output=True, text=True, timeout=45)
            self.assertEqual(again.returncode, 0, again.stdout + again.stderr)
        pdl.write_text('iState tap.INSTRUCTION 0;\n')
        result = subprocess.run(args, capture_output=True, text=True, timeout=45)
        self.assertNotEqual(result.returncode, 0)
        self.assertIn('controller-owned', result.stdout + result.stderr)

    def test_alias_slice_and_hierarchical_prefix(self):
        child = SCAN.replace('Module Chains', 'Module Child')
        source = child + '''Module Chains {
 ScanInPort si; Instance c Of Child { InputPort si=si; }
 ScanOutPort so { Source c.so; }
}'''
        for flag in FLAGS:
            out = self.replay(source, 'iPDLLevel 1; iProcsForModule Child; '
                              f'iProc history {{}} {{iState inverted[2:1] 0b1x {flag}}}; '
                              'iProcsForModule Chains; iReset; iCall c.history; '
                              'iRead c.value 0; iApply;\n')
            self.assertIn(f'iState {{c.inverted[2:1]}} 0b1x {flag}'.encode(), out)

    def test_history_retains_merge_order(self):
        body = ('iPDLLevel 1; iProcsForModule Chains; '
                'iProc first {} {iState value 9 -LastReadValue; iRead value 0; iApply}; '
                'iProc second {} {iState value 6 -LastMiscompareValue; iRead value 0; iApply}; '
                'iReset; iMerge -begin; iCall first; iCall second; iMerge -end;\n')
        out = self.replay(SCAN, body)
        self.assertEqual(out.count(b'iApply'), 2)
        self.assertLess(out.index(b'-LastReadValue'), out.index(b'-LastMiscompareValue'))

    def test_invalid_targets_values_and_readback_stay_diagnosed(self):
        source = SCAN.replace(' Alias', ' DataOutPort q[3:0] { Source value; }\n Alias')
        for body in ('iState q 0 -LastReadValue',
                     'iState value 16 -LastReadValue',
                     'iState value 0 -unknown',
                     'iState absent 0 -LastMiscompareValue',
                     'iState value 9 -LastReadValue; set v [iGetReadData value]'):
            icl, pdl = self.root / 'source.icl', self.root / 'source.pdl'
            icl.write_text(source)
            pdl.write_text(body + '\n')
            output = self.root / 'result.pdl'
            output.write_bytes(b'old output\n')
            self.run_tool(icl, pdl, ok=False)
            self.assertEqual(output.read_bytes(), b'old output\n')

    def test_history_stream_scale(self):
        count = 4096
        body = ''.join(f'iState value {i % 16} {FLAGS[i % 2]};\n'
                       for i in range(count))
        start = time.monotonic()
        out = self.replay(DATA, body)
        self.assertEqual(out.count(b'iState'), count)
        self.assertNotIn(b'iApply', out)
        print(f'observation history: {count} statements, {len(body)} PDL bytes, '
              f'three replays {time.monotonic() - start:.3f}s')


if __name__ == '__main__':
    unittest.main()
