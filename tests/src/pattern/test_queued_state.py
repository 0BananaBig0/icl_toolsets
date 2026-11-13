#!/usr/bin/env python3
"""State assumptions preserve pending accesses without an implicit apply."""
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


class QueuedState(unittest.TestCase):
    setUp = helpers.SimultaneousChains.setUp
    run_tool = helpers.SimultaneousChains.run_tool
    replay = helpers.SimultaneousChains.replay

    def test_pending_scan_read_uses_assumed_snapshot(self):
        out = self.replay(SCAN, 'iRead value 5; iState value 9; iApply;\n')
        self.assertEqual(out.count(b'iApply'), 1)
        self.assertIn(b'iRead value 0b1001', out)

    def test_state_overwrites_pending_write_value(self):
        out = self.replay(SCAN, 'iReset; iWrite value 5; iState value 9; iApply; iRead value 9; iApply;\n')
        self.assertIn(b'iWrite value 0b1001', out)
        self.assertEqual(out.count(b'iApply'), 2)

    def test_masked_queue_uses_latest_controlling_state(self):
        out = self.replay(SCAN, 'iReset; iWrite value[1:0] 0bx; iState inverted 5; iApply;\n')
        self.assertIn(b'iWrite value 0b1010', out)
        out = self.replay(SCAN, 'iReset; iWrite value 1; iState value 9; iWrite value 6; iApply;\n')
        self.assertIn(b'iWrite value 0b0110', out)

    def test_data_read_has_no_event(self):
        out = self.replay(DATA, 'iState enable 1; iState data 6; iRead q 9; iState value 9; iApply;\n')
        self.assertNotIn(b'iWrite', out)
        self.assertEqual(out.count(b'iApply'), 1)

    def test_taken_assumption_overwrites_pending_explicit_value(self):
        out = self.replay(SCAN, 'iReset; iTake value; iWrite value 5; iState value 9; iApply; iRead value 9; iApply;\n')
        self.assertIn(b'iWrite value 0b1001', out)

    def test_procedure_and_merge_keep_state_stream_order(self):
        body = ('iPDLLevel 1; iProcsForModule Chains; '
                'iProc work {v} {iRead value $v; iState value $v; iApply}; '
                'iMerge -begin; iCall work 9; iCall work 6; iMerge -end;\n')
        out = self.replay(SCAN, body)
        self.assertEqual(out.count(b'iApply'), 2)
        self.assertLess(out.index(b'iState value 0b1001'), out.index(b'iState value 0b0110'))

    def test_queue_crosses_input_file_boundary(self):
        icl = self.root / 'source.icl'
        first, second = self.root / 'first.pdl', self.root / 'second.pdl'
        icl.write_text(SCAN)
        first.write_text('iReset; iWrite value 5;\n')
        second.write_text('iState value 9; iApply;\n')
        result = subprocess.run([str(helpers.BIN), '--icl', str(icl), '--pdl', str(first), str(second),
                                 '--top_module', 'Chains', '--output', str(self.root / 'result.pdl')],
                                capture_output=True, text=True, timeout=45)
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        out = (self.root / 'result.pdl').read_bytes()
        self.assertEqual(out, self.run_tool(icl, self.root / 'result.pdl', 'replay.pdl'))

    def test_local_reset_uses_state_at_apply(self):
        source = """Module Child {
 ScanInPort si; ResetPort rst { ActivePolarity 0; }
 ScanRegister value[3:0] { ScanInSource si; CaptureSource value; ResetValue 0; }
 ScanOutPort so { Source value[0]; }
}
Module Chains {
 ScanInPort si;
 DataRegister release { ResetValue 0; }
 Instance c Of Child { InputPort si=si; InputPort rst=release; }
 ScanOutPort so { Source c.so; }
}
"""
        icl, pdl = self.root / 'source.icl', self.root / 'source.pdl'
        icl.write_text(source)
        body = 'iReset; iWrite c.value 9; iState release 1; iApply;\n'
        pdl.write_text(body)
        out = self.root / 'result.pdl'
        reset = self.root / 'reset.pdl'
        result = subprocess.run([str(helpers.BIN), '--icl', str(icl), '--pdl', str(pdl),
                                 '--top_module', 'Chains', '--output', str(out),
                                 '--local_reset', str(reset)], capture_output=True, text=True, timeout=45)
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        self.assertEqual(reset.read_text(), body)
        self.assertEqual(out.read_bytes(), self.run_tool(icl, reset, 'again.pdl'))
        pdl.write_text('iReset; iState release 1; iWrite c.value 9; iState release 0; iApply;\n')
        before = out.read_bytes(), reset.read_bytes()
        result = subprocess.run([str(helpers.BIN), '--icl', str(icl), '--pdl', str(pdl),
                                 '--top_module', 'Chains', '--output', str(out),
                                 '--local_reset', str(reset)], capture_output=True, text=True, timeout=45)
        self.assertNotEqual(result.returncode, 0)
        self.assertEqual((out.read_bytes(), reset.read_bytes()), before)

    def test_legal_assumption_group_before_later_reset_repair(self):
        source = """Module Child {
 ScanInPort si; ResetPort rst { ActivePolarity 0; }
 ScanRegister value[3:0] { ScanInSource si; CaptureSource value; ResetValue 0; }
 ScanOutPort so { Source value[0]; }
}
Module Chains {
 ScanInPort si; DataRegister release { ResetValue 0; }
 ScanRegister gate { ScanInSource si; CaptureSource gate; ResetValue 0; }
 Instance c Of Child { InputPort si=gate; InputPort rst=release; }
 Instance d Of Child { InputPort si=c.so; InputPort rst=gate; }
 ScanOutPort so { Source d.so; }
}
"""
        icl, pdl = self.root / 'source.icl', self.root / 'source.pdl'
        icl.write_text(source)
        pdl.write_text('iReset; iWrite c.value 9; iState release 1; iApply; iWrite d.value 5; iApply;\n')
        output, reset = self.root / 'result.pdl', self.root / 'reset.pdl'
        args = [str(helpers.BIN), '--icl', str(icl), '--pdl', str(pdl),
                '--top_module', 'Chains', '--output', str(output), '--local_reset', str(reset)]
        result = subprocess.run(args, capture_output=True, text=True, timeout=45)
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        text = reset.read_text()
        self.assertIn('iWrite gate 0b1', text)
        self.assertLess(text.index('iState release 1'), text.index('iWrite gate 0b1'))
        self.assertEqual(output.read_bytes(), self.run_tool(icl, reset, 'again.pdl'))

    def test_connected_output_keeps_its_own_expectation(self):
        out = self.replay(DATA, 'iState enable 0; iRead q 5; iState value 9; iApply;\n')
        self.assertIn(b'iRead q 0b0101', out)

    def test_alias_write_and_slice_assumption_overlap(self):
        out = self.replay(SCAN, 'iReset; iWrite inverted 5; iState value[1:0] 1; iApply;\n')
        self.assertIn(b'iWrite value 0b1001', out)

    def test_unknown_assumption_clears_all_prior_queued_values(self):
        out = self.replay(SCAN, 'iReset; iWrite value 5; iWrite value 6; iState value 0bx; iApply;\n')
        self.assertIn(b'iWrite value 0b0000', out)

    def test_long_alternating_queue_scale(self):
        for count in (1024, 4096):
            body = 'iReset; ' + ''.join(
                f'iWrite value {i % 16}; iState value {(i + 1) % 16}; '
                for i in range(count)) + 'iApply;\n'
            start = time.monotonic()
            out = self.replay(SCAN, body)
            self.assertEqual(out.count(b'iApply'), 1)
            print(f'queued state: {count} write/state pairs, {len(body)} PDL bytes, '
                  f'three replays {time.monotonic() - start:.3f}s')

    def test_reset_preview_group_scale(self):
        count, calls = 64, 16
        source = 'Module Chains { ScanInPort si;\n'
        previous = 'si'
        for i in range(count):
            source += f'ScanRegister r{i} {{ ScanInSource {previous}; ResetValue 0; }}\n'
            previous = f'r{i}'
        source += f'ScanOutPort so {{ Source {previous}; }} }}\n'
        body = (f'iPDLLevel 1; iProcsForModule Chains; iReset; '
                f'iProc step {{v}} {{iRead r{count-1} $v; iState r{count-1} $v; iApply}};\n')
        body += ''.join(f'iCall step {i % 2};\n' for i in range(calls))
        icl, pdl = self.root / 'source.icl', self.root / 'source.pdl'
        icl.write_text(source)
        pdl.write_text(body)
        start = time.monotonic()
        baseline = self.run_tool(icl, pdl)
        plain_time = time.monotonic() - start
        start = time.monotonic()
        output, reset = self.root / 'local.pdl', self.root / 'reset.pdl'
        result = subprocess.run([str(helpers.BIN), '--icl', str(icl), '--pdl', str(pdl),
                                 '--top_module', 'Chains', '--output', str(output),
                                 '--local_reset', str(reset), '--debug'],
                                capture_output=True, text=True, timeout=45)
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        self.assertEqual(output.read_bytes(), baseline)
        self.assertEqual(reset.read_text(), body)
        print(f'reset previews: {count} registers, {calls} groups, '
              f'{len(source)} ICL/{len(body)} PDL bytes, '
              f'normal {plain_time:.3f}s, with rewrite {time.monotonic()-start:.3f}s')

    def test_invalid_state_preserves_output_and_unapplied_queue(self):
        icl, pdl, out = self.root / 'source.icl', self.root / 'source.pdl', self.root / 'result.pdl'
        icl.write_text(SCAN)
        for body in ('iWrite value 5; iState si 0; iApply;',
                     'iWrite value 5; iState value 32; iApply;',
                     'iWrite value 5; iState value 9;'):
            pdl.write_text(body)
            out.write_bytes(b'previous\n')
            self.run_tool(icl, pdl, ok=False)
            self.assertEqual(out.read_bytes(), b'previous\n')


if __name__ == '__main__':
    unittest.main()
