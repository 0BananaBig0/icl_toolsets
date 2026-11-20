#!/usr/bin/env python3
"""Typed vector functions retain source/flat/generated/reset behavior."""
import subprocess
import time
import unittest

import test_system_clocks as helpers

DATA = """Module Top {
 DataInPort p[3:2]; DataInPort p[1:0];
 DataOutPort out[3:0] { Source p; }
}
"""
CLOCKS = """Module Stage {
 ClockPort ck[3:2]; ClockPort ck[1:0];
 ToClockPort out[3:2] { Source ck[3:2]; FreqDivider 2; }
 ToClockPort out[1:0] { Source ck[1:0]; FreqDivider 3; }
}
Module Top {
 ClockPort root[7:4];
 ClockPort alt[7:6]; ClockPort alt[5:4];
 ClockPort n726f6f74[7:6]; ClockPort n726f6f74[5:4];
 Instance child Of Stage { InputPort ck=root; }
}
"""


class SegmentedPorts(unittest.TestCase):
    setUp = helpers.SystemClocks.setUp
    run_tool = helpers.SystemClocks.run_tool
    files = helpers.SystemClocks.files
    replay = helpers.SystemClocks.replay
    rejected = helpers.SystemClocks.rejected

    def test_input_order_orientation_and_parameters(self):
        for ports in (
            'DataInPort p[1:0]; DataInPort p[3:2];',
            'DataInPort p[0:1]; DataInPort p[2:3];',
            'DataInPort p[0]; DataInPort p[2]; DataInPort p[1]; DataInPort p[3];',
            'Parameter W=4; DataInPort p[$W-1:2]; DataInPort p[1:0];',
        ):
            with self.subTest(ports=ports):
                source = 'Module Top {' + ports + 'DataOutPort out[3:0] {Source p;}}'
                output = self.replay(source, 'iWrite p 13; iApply; iRead out 13; iApply')
                self.assertIn('iRead out 0b1101', output)

    def test_output_functions_and_selected_enums(self):
        source = """Module Top {
 Enum High { Hi=2'b11; } Enum Low { Lo=2'b01; }
 DataInPort p[3:2] {RefEnum High; Attribute tag="upper";}
 DataInPort p[1:0] {RefEnum Low; Attribute tag="lower";}
 DataOutPort out[1:0] {Source p[1:0];}
 DataOutPort out[3:2] {Source p[3:2];}
}"""
        self.replay(source, 'iWrite p[3:2] Hi; iWrite p[1:0] Lo; iApply; '
                    'iRead out 13; iApply')

    def test_mixed_input_and_output_functions(self):
        self.replay('Module Top {DataInPort p[1]; SelectPort p[0]; '
                    'DataOutPort out {Source p[1];}}',
                    'iWrite p[1] 1; iApply; iRead out 1; iApply')
        self.replay('Module Top {DataInPort p[1]; DataOutPort p[0] {Source p[1];}}',
                    'iWrite p[1] 1; iApply; iRead p[0] 1; iApply')

    def test_alias_with_its_own_enum_is_unambiguous(self):
        source = """Module Top {
 Enum Words { On=2'b10; }
 DataInPort p[1]; DataInPort p[0];
 Alias word[1:0]=p {RefEnum Words;}
 LogicSignal enabled {word == On;}
 DataOutPort out {Source enabled;}
}"""
        self.replay(source, 'iWrite p 2; iApply; iRead out 1; iApply')

    def test_whole_group_does_not_borrow_a_fragment_enum(self):
        source = """Module Top {
 Enum Words { On=2'b10; }
 DataInPort p[1] {RefEnum Words;} DataInPort p[0] {RefEnum Words;}
 LogicSignal enabled {p == On;}
 DataOutPort out {Source enabled;}
}"""
        self.rejected(source, 'iNote check', 'matching RefEnum')
        repeated = source.replace(
            'DataInPort p[1] {RefEnum Words;} DataInPort p[0] {RefEnum Words;}',
            'DataInPort p {RefEnum Words;}').replace('p == On', 'p,p == On')
        self.rejected(repeated, 'iNote check', 'matching RefEnum')

    def test_group_history_and_never_read_bits(self):
        source = DATA.replace('DataOutPort out[3:0] { Source p; }',
                              'DataOutPort out[3:2] {Source p[3:2];} '
                              'DataOutPort out[1:0] {Source p[1:0];}')
        output = self.replay(source, 'iNote [iGetReadData out]; '
                             'iRead out[3:2] 0bxx; iApply; '
                             'iNote [iGetReadData out[1:0]]')
        self.assertIn('iNote 0bxxxx', output)
        self.assertIn('iNote 0bxx', output)
        self.rejected(source, 'iRead out[3:2] 0bxx; iApply; '
                      'iNote [iGetReadData out]', 'hardware readback')

    def test_group_masked_status_and_no_invented_observations(self):
        output = self.replay(DATA, 'iWrite p 13; iApply; iRead out 0bxxxx; '
                             'iApply; iNote [iGetStatus]')
        self.assertIn('iNote 0', output)
        self.rejected(DATA, 'iWrite p 13; iApply; iRead out 13; '
                      'iApply; iNote [iGetStatus]', 'hardware readback')

    def test_group_resource_slices_preserve_other_functions(self):
        self.replay(DATA, 'iTake p[3:2]; iWrite p 13; iApply; '
                    'iWrite p[1:0] 2; iApply; iRead out 14; iApply; '
                    'iRelease p[3:2]')

    def test_invalid_grouped_access_preserves_artifacts(self):
        source = """Module Top {
 DataInPort p[1]; ScanInPort p[0];
 ScanRegister r {ScanInSource p[0];}
 ScanOutPort so {Source r;}
}"""
        self.rejected(source, 'iWrite p 2; iReset', 'write target requires')
        self.rejected(DATA, 'iRead p 0; iApply', 'reads require')
        self.rejected(DATA, 'iWrite p[4] 0; iApply', 'index outside')

    def test_invalid_source_groups_preserve_artifacts(self):
        for ports, message in (
            ('DataInPort p[0]; DataInPort p[2];', 'contiguous'),
            ('DataInPort p[1:0]; DataInPort p[1];', 'overlapping'),
            ('DataInPort p[1:0]; DataInPort p[2:3];', 'direction'),
            ('DataInPort p; DataInPort p[0];', 'duplicate'),
        ):
            with self.subTest(ports=ports):
                self.rejected('Module Top {' + ports + '}', 'iNote check', message)

    def test_grouped_clock_names_overrides_and_defaults(self):
        self.replay(CLOCKS, 'iClock child.out(3); iRunLoop 2 -sck child.out(3); '
                    'iClockOverride child.out(3) -source alt(7) -freqDivider 5; '
                    'iRunLoop 2 -sck child.out(3); iClock child.out(3); '
                    'iRunLoop 2 -sck child.out(3); '
                    'iClock n726f6f74(7); iRunLoop 1 -sck n726f6f74(7); '
                    'iClockOverride child.out(3); iClock child.out(3); '
                    'iRunLoop 2 -sck child.out(3)')

    def run_tap(self, source, commands, ok=True):
        icl, pdl = self.files(source, commands)
        output = self.root / 'result.pdl'
        output.write_text('previous output\n')
        fixture = helpers.ROOT / 'tests/cases/ours/pattern_generation/access_link'
        result = subprocess.run([
            str(helpers.BIN), '--icl', str(icl), '--pdl', str(pdl),
            '--bsdl', 'AccessProbe', str(fixture / 'simple.bsdl'),
            '--top_module', 'Top', '--output', str(output), '--debug',
        ], capture_output=True, text=True, timeout=45)
        self.assertEqual(result.returncode == 0, ok, result.stdout + result.stderr)
        if not ok:
            self.assertEqual(output.read_text(), 'previous output\n')
            return result.stdout + result.stderr
        original = output.read_text()
        # Restored flat metadata carries the already-bound TAP model.
        self.assertEqual(original, self.run_tool(
            self.root / 'debug/Top.icl', pdl, 'flat/result.pdl'))
        self.assertEqual(original, self.run_tool(
            self.root / 'debug/Top.icl', output, 'replay/result.pdl'))
        return original

    def tap_source(self):
        path = helpers.ROOT / 'tests/cases/ours/pattern_generation/access_link/simple.icl'
        return path.read_text().replace('Module AccessTop {', 'Module Top {')

    def test_bsdl_top_model_allows_unrelated_port_functions(self):
        source = self.tap_source().replace('Module Top {', 'Module Top {'
            'DataInPort p[1]; DataInPort p[0]; DataOutPort out[1:0] {Source p;}')
        self.run_tap(source, 'iReset; iWrite p 2; iApply; '
                     'iWrite tap.INSTRUCTION 4; iApply; iWrite core.r 90; iApply; '
                     'iRead core.r 90; iApply; iRead out 2; iApply')

    def test_generated_route_names_preserve_logical_port_bases(self):
        name = 'IclPatternScanRoute4'
        source = self.tap_source().replace(
            'Module Top {', 'Module Top {'
            f'DataInPort {name}[1]; DataInPort {name}[0]; '
            f'DataOutPort out[1:0] {{Source {name};}}')
        self.run_tap(source, f'iReset; iWrite {name} 2; iApply; '
                     'iRead out 2; iApply')

    def grouped_tap_source(self, binding):
        source = self.tap_source().replace(
            'ScanInPort si;', 'ScanInPort si[1]; DataInPort si[0];')
        source = source.replace('ScanInSource si;', 'ScanInSource si[1];')
        source = source.replace("1'b0: si;", "1'b0: si[1];")
        source = source.replace('Port si;', 'Port si[1];')
        return source.replace('InputPort mode = tap.long_setup;',
                              'InputPort mode = tap.long_setup; ' + binding)

    def test_tap_can_bind_an_unrelated_function_of_the_same_bus(self):
        self.run_tap(self.grouped_tap_source("InputPort si[0]=1'b0;"),
                     'iReset; iWrite tap.INSTRUCTION 4; iApply; '
                     'iWrite core.r 90; iApply; iRead core.r 90; iApply')

    def test_tap_rejects_explicit_source_on_selected_scan_input(self):
        for value in ("1'b0", "1'bx"):
            message = self.run_tap(
                self.grouped_tap_source('InputPort si[1]=' + value + ';'),
                'iReset', ok=False)
            self.assertIn('already has an InputPort binding', message)

    def test_representative_many_function_replays(self):
        width = 128
        bits = ''.join('1' if i % 3 == 0 else '0' for i in range(width))
        for fragmented in (False, True):
            ports = (''.join(f'DataInPort p[{i}];' for i in range(width))
                     if fragmented else f'DataInPort p[{width-1}:0];')
            source = 'Module Top {' + ports + f'DataOutPort out[{width-1}:0] {{Source p;}}}}'
            start = time.monotonic()
            self.replay(source, f'iWrite p 0b{bits}; iApply; iRead out 0b{bits}; iApply')
            print(f'segmented ports: fragments={width if fragmented else 1} '
                  f'bits={width} bytes={len(source)} forms=5 '
                  f'seconds={time.monotonic()-start:.6f}', flush=True)


if __name__ == '__main__':
    unittest.main()
