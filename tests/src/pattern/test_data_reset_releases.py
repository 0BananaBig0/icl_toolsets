#!/usr/bin/env python3
"""Local-reset rewriting uses declared DataRegister write paths."""
import subprocess
import time
import unittest

import test_simultaneous_scan_chains as helpers
from test_data_reset_dependencies import BASE


class DataResetReleases(unittest.TestCase):
    setUp = helpers.SimultaneousChains.setUp
    run_tool = helpers.SimultaneousChains.run_tool

    def rewrite(self, source, body, ok=True):
        icl, pdl = self.root / 'source.icl', self.root / 'source.pdl'
        icl.write_text(source)
        pdl.write_text(body)
        output, reset = self.root / 'result.pdl', self.root / 'reset.pdl'
        output.write_bytes(b'previous output\n')
        reset.write_bytes(b'previous reset\n')
        args = [str(helpers.BIN), '--icl', str(icl), '--pdl', str(pdl),
                '--top_module', 'Chains', '--output', str(output),
                '--local_reset', str(reset), '--debug']
        result = subprocess.run(args, capture_output=True, text=True, timeout=45)
        self.assertEqual(result.returncode == 0, ok, result.stdout + result.stderr)
        self.assertEqual(pdl.read_text(), body)
        if not ok:
            self.assertEqual(output.read_bytes(), b'previous output\n')
            self.assertEqual(reset.read_bytes(), b'previous reset\n')
            return result.stdout + result.stderr
        self.assertEqual(output.read_bytes(), self.run_tool(
            self.root / 'debug/Chains.icl', pdl, 'original-flat/result.pdl'))
        self.assertEqual(output.read_bytes(), self.run_tool(icl, output, 'original-replay/result.pdl'))
        expanded = self.run_tool(icl, reset, 'again.pdl')
        self.assertEqual(expanded, self.run_tool(
            self.root / 'debug/Chains.icl', reset, 'flat/result.pdl'))
        self.assertEqual(expanded, self.run_tool(icl, self.root / 'again.pdl', 'replay/result.pdl'))
        return reset.read_text()

    def test_primary_driven_release(self):
        reset = self.rewrite(BASE, 'iReset; iWrite c.value 9; iApply;\n')
        self.assertIn('iWrite release 0b1', reset)
        self.assertLess(reset.index('iWrite release'), reset.index('iWrite c.value'))

    def test_preserves_procedure_definition(self):
        proc = 'iProc configure {v} {iWrite c.value $v; iApply}'
        body = f'iPDLLevel 1; iProcsForModule Chains; {proc}; iReset; iCall configure 9;\n'
        reset = self.rewrite(BASE, body)
        self.assertIn(proc, reset)
        self.assertLess(reset.index('iWrite release'), reset.index('iCall configure'))

    def test_saved_vendor_input_is_the_generated_reset_artifact(self):
        fixtures = helpers.ROOT / 'tests/cases/ours/pattern_generation'
        reset = self.root / 'certified.pdl'
        result = subprocess.run([
            str(helpers.BIN), '--icl', str(fixtures / 'data_reset_probe.icl'),
            '--pdl', str(fixtures / 'data_reset_release_request.pdl'),
            '--top_module', 'DataResetProbe', '--output', str(self.root / 'result.pdl'),
            '--local_reset', str(reset)], capture_output=True, text=True, timeout=45)
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        self.assertEqual(reset.read_bytes(), (fixtures / 'data_reset_release.pdl').read_bytes())

    def test_already_released_readonly_control_needs_no_write(self):
        source = BASE.replace('WriteDataSource data; WriteEnSource enable; ', '')
        body = 'iReset; iState release 1; iWrite c.value 9; iApply;\n'
        reset = self.rewrite(source, body)
        self.assertNotIn('iWrite release', reset)
        message = self.rewrite(source, 'iReset; iWrite c.value 9; iApply;\n', ok=False)
        self.assertIn('reset', message)

    def test_disabled_or_incompatible_data_path_fails_atomically(self):
        for source in (BASE.replace('WriteEnSource enable', "WriteEnSource 1'b0"),
                       BASE.replace('WriteDataSource data', "WriteDataSource 1'b0")):
            self.rewrite(source, 'iReset; iWrite c.value 9; iApply;\n', ok=False)

    def test_taken_control_is_not_assigned_by_rewriting(self):
        for setup in ('iTake release;', 'iTake release; iState release 0;'):
            message = self.rewrite(BASE, f'iReset; {setup} iWrite c.value 9; iApply;\n', ok=False)
            self.assertIn('release', message)
        reset = self.rewrite(BASE, 'iReset; iTake release; iRelease release; '
                             'iWrite c.value 9; iApply;\n')
        self.assertIn('iWrite release', reset)

    def test_slice_preserves_other_taken_bits(self):
        source = BASE.replace('DataInPort data;', 'DataInPort data[3:0];')
        source = source.replace('DataRegister release {', 'DataRegister release[3:0] {')
        source = source.replace('SelectedBy release {', 'SelectedBy release[0] {')
        reset = self.rewrite(source, 'iReset; iTake release[3:1]; '
                             'iWrite c.value 9; iApply;\n')
        self.assertIn('release[0]', reset)
        self.assertNotIn('iWrite release 0b', reset)

    def test_scan_driven_data_release(self):
        source = (helpers.ROOT / 'tests/cases/ours/pattern_generation/data_reset_release_replay.icl').read_text().replace('Module DataResetReleaseScan', 'Module Chains')
        reset = self.rewrite(source, 'iReset; iWrite c.value 9; iApply;\n')
        self.assertIn('iWrite release 0b1', reset)
        self.assertNotIn('iWrite data', reset)

    def test_reserved_upstream_write_enable_blocks_release(self):
        source = (helpers.ROOT / 'tests/cases/ours/pattern_generation/data_reset_release_replay.icl').read_text().replace('Module DataResetReleaseScan', 'Module Chains')
        message = self.rewrite(source, 'iReset; iTake enable; iState enable 0; '
                               'iWrite c.value 9; iApply;\n', ok=False)
        self.assertIn('enable', message)

    def test_mixed_scan_then_data_reset_ancestors(self):
        source = (helpers.ROOT / 'tests/cases/ours/pattern_generation/data_reset_release_replay.icl').read_text().replace('Module DataResetReleaseMixed', 'Module Chains')
        reset = self.rewrite(source, 'iReset; iWrite c.value 9; iApply;\n')
        self.assertLess(reset.index('iWrite gate'), reset.index('iWrite d.release'))
        self.assertLess(reset.index('iWrite d.release'), reset.index('iWrite c.value'))

    def test_mixed_data_then_scan_reset_ancestors(self):
        gate = """Module Gate {
 ScanInPort si; ResetPort rst { ActivePolarity 0; }
 ScanRegister value { ScanInSource si; CaptureSource value; ResetValue 0; }
 ScanOutPort so { Source value; } DataOutPort q { Source value; }
}
"""
        source = BASE.replace(' Instance c Of Child',
                              ' Instance g Of Gate { InputPort si=si; InputPort rst=reset_gate; }\n Instance c Of Child')
        source = source.replace('InputPort si = si;', 'InputPort si = g.so;')
        source = source.replace('InputPort rst = reset_gate;', 'InputPort rst = g_reset;')
        source = source.replace(' Instance c Of Child',
                                " DataMux g_reset SelectedBy g.q { 1'b0: 1'b0; 1'b1: global; }\n Instance c Of Child")
        reset = self.rewrite(gate + source, 'iReset; iWrite c.value 9; iApply;\n')
        self.assertLess(reset.index('iWrite release'), reset.index('iWrite g.value'))
        self.assertLess(reset.index('iWrite g.value'), reset.index('iWrite c.value'))

    def test_pending_assumption_can_avoid_release(self):
        body = 'iReset; iWrite c.value 9; iState release 1; iApply;\n'
        reset = self.rewrite(BASE, body)
        self.assertEqual(reset, body)

    def test_reset_choice_retains_unsigned_xor_order(self):
        source = BASE.replace('DataInPort data;', 'DataInPort data[3:0];')
        source = source.replace('DataRegister release {', 'DataRegister release[3:0] {')
        source = source.replace(' Instance c Of Child',
                                " LogicSignal ready { (release == 4'b0010) | (release == 4'b1101); }\n Instance c Of Child")
        source = source.replace('SelectedBy release {', 'SelectedBy ready {')
        for initial, expected in ((0, '0010'), (15, '1101')):
            model = source.replace('WriteEnSource enable; ResetValue 0;',
                                   f'WriteEnSource enable; ResetValue {initial};')
            reset = self.rewrite(model, 'iReset; iWrite c.value 9; iApply;\n')
            self.assertIn(f'iWrite release 0b{expected}', reset)

    def test_release_scale(self):
        for width in (64, 256):
            source = BASE.replace('DataInPort data;', f'DataInPort data[{width-1}:0];')
            source = source.replace('DataRegister release {', f'DataRegister release[{width-1}:0] {{')
            source = source.replace('SelectedBy release {', 'SelectedBy release[0] {')
            start = time.monotonic()
            reset = self.rewrite(source, 'iReset; iWrite c.value 9; iApply;\n')
            self.assertIn('iWrite release 0b', reset)
            print(f'data reset release: {width} control bits, {len(source)} ICL bytes, '
                  f'six runs {time.monotonic()-start:.3f}s')


if __name__ == '__main__':
    unittest.main()
