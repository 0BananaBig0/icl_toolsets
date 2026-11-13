#!/usr/bin/env python3
"""Relative functional-clock retargeting with source/flat/generated replay."""
import pathlib
import subprocess
import sys
import tempfile
import time
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[3]
BIN = pathlib.Path(sys.argv.pop(1)).resolve() if len(sys.argv) > 1 else ROOT / 'build/icl-pattern-generation'


class SystemClocks(unittest.TestCase):
    def setUp(self):
        temporary = tempfile.TemporaryDirectory()
        self.addCleanup(temporary.cleanup)
        self.root = pathlib.Path(temporary.name)

    def run_tool(self, icl, pdl, output='result.pdl', ok=True, reset=False):
        command = [str(BIN), '--icl', str(icl), '--pdl', str(pdl),
                   '--top_module', 'Top', '--output', str(self.root / output), '--debug']
        if reset:
            command += ['--local_reset', str(self.root / 'reset.pdl')]
        result = subprocess.run(command, text=True, capture_output=True, timeout=45)
        self.assertEqual(result.returncode == 0, ok, result.stdout + result.stderr)
        return (self.root / output).read_text() if ok else result.stdout + result.stderr

    def files(self, source, commands):
        icl, pdl = self.root / 'source.icl', self.root / 'source.pdl'
        icl.write_text(source)
        pdl.write_text('iPDLLevel 1\niProcsForModule Top\n' + commands)
        return icl, pdl

    def replay(self, source, commands):
        icl, pdl = self.files(source, commands)
        original = self.run_tool(icl, pdl, reset=True)
        flat = self.root / 'debug/Top.icl'
        self.assertEqual(original, self.run_tool(flat, pdl, 'flat/result.pdl'))
        self.assertEqual(original, self.run_tool(icl, self.root / 'result.pdl', 'replay/result.pdl'))
        self.assertEqual(original, self.run_tool(flat, self.root / 'result.pdl', 'both/result.pdl'))
        self.assertEqual(original, self.run_tool(icl, self.root / 'reset.pdl', 'reset/result.pdl'))
        return original

    def rejected(self, source, commands, diagnostic):
        icl, pdl = self.files(source, commands)
        artifacts = [self.root / 'result.pdl', self.root / 'reset.pdl', self.root / 'debug/Top.icl']
        for artifact in artifacts:
            artifact.parent.mkdir(parents=True, exist_ok=True)
            artifact.write_text('previous artifact\n')
        message = self.run_tool(icl, pdl, ok=False, reset=True)
        self.assertIn(diagnostic, message)
        for artifact in artifacts:
            self.assertEqual(artifact.read_text(), 'previous artifact\n')

    def test_root_and_zero_wait(self):
        result = self.replay('Module Top { ClockPort clk; }',
                             'iClock clk; iRunLoop 0 -sck clk; iRunLoop 17 -sck clk\n')
        self.assertIn('iClock clk\niRunLoop 0 -sck clk\niRunLoop 17 -sck clk', result)

    def test_nested_ratios_and_rounding(self):
        source = '''Module Stage {
  ClockPort input;
  ToClockPort output { Source input; FreqMultiplier 3; FreqDivider 2; }
}
Module Top {
  ClockPort clk;
  Instance child Of Stage { InputPort input = clk; }
  ToClockPort slow { Source child.output; FreqDivider 5; }
}'''
        result = self.replay(source, 'iClock slow; iRunLoop 2 -sck slow; iRunLoop 3 -sck slow\n')
        self.assertIn('iClock clk\niRunLoop 7 -sck clk\niRunLoop 10 -sck clk', result)

    def test_indexed_forwarding_and_prefix(self):
        source = '''Module Child {
  ClockPort ck[0:1];
  Alias swapped[1:0] = ck[1], ck[0];
  ToClockPort out[1:0] { Source swapped; FreqDivider 3; }
}
Module Top {
  ClockPort root[5:4];
  Instance child Of Child { InputPort ck = root; }
}'''
        result = self.replay(source, '''iProcsForModule Child
iProc wait {} { iClock out(1); iRunLoop 4 -sck out(1) }
iProcsForModule Top
iCall child.wait
iPrefix child
iClock out(0); iRunLoop 1 -sck out(0)
''')
        self.assertIn('iRunLoop 12 -sck root(4)', result)
        self.assertIn('iRunLoop 3 -sck root(5)', result)

    def test_clock_mux_uses_first_matching_entry(self):
        source = '''Module Top {
  ClockPort a; ClockPort b; DataInPort mode[1:0];
  ClockMux mux SelectedBy mode { 2'b01: a; 2'bxx: b; }
  ToClockPort out { Source mux; }
}'''
        result = self.replay(source, '''iState mode 1; iClock out; iRunLoop 2 -sck out
iState mode 2; iClock out; iRunLoop 3 -sck out
''')
        self.assertIn('iClock a\niRunLoop 2 -sck a', result)
        self.assertIn('iClock b\niRunLoop 3 -sck b', result)

    def test_pending_writes_are_not_flushed_by_clock_check(self):
        result = self.replay('Module Top { ClockPort clk; DataInPort value; }',
                             'iWrite value 1; iClock clk; iApply; iRunLoop 2 -sck clk\n')
        self.assertLess(result.index('iClock clk'), result.index('iWrite value'))
        self.rejected('Module Top { ClockPort clk; DataInPort value; }',
                      'iClock clk; iWrite value 1; iRunLoop 0 -sck clk\n', 'unapplied')

    def test_wait_requires_checked_target_even_at_zero(self):
        for count in (0, 1):
            with self.subTest(count=count):
                self.rejected('Module Top { ClockPort clk; }',
                              f'iRunLoop {count} -sck clk\n', 'requires prior iClock')

    def test_unknown_and_changed_route(self):
        source = '''Module Top {
  ClockPort a; ClockPort b; DataInPort mode;
  ClockMux mux SelectedBy mode { 1'b0: a; 1'b1: b; }
  ToClockPort out { Source mux; }
}'''
        self.rejected(source, 'iClock out\n', 'unknown clock selector')
        self.rejected(source, 'iState mode 0; iClock out; iState mode 1; iRunLoop 1 -sck out\n',
                      'changed since iClock')
        self.rejected(source, 'iState mode 0; iClock out; iWrite mode 1; iApply; iRunLoop 1 -sck out\n',
                      'changed since iClock')

    def test_equal_effective_reroute_and_multiple_targets(self):
        source = """Module Ratio {
  Parameter mult = 3; Parameter div = 2; ClockPort ck;
  ToClockPort out { Source ck; FreqMultiplier $mult; FreqDivider $div; }
}
Module Top {
  ClockPort root; DataInPort mode;
  Instance one Of Ratio { InputPort ck = root; }
  Instance two Of Ratio { InputPort ck = root; Parameter mult = 6; Parameter div = 4; }
  ClockMux mux SelectedBy mode { 1'b0: one.out; 1'b1: two.out; }
  ToClockPort out { Source mux; }
}"""
        result = self.replay(source, """iState mode 0; iClock out; iClock one.out
  iState mode 1; iRunLoop 4 -sck out; iRunLoop 4 -sck one.out
""")
        self.assertEqual(result.count('iRunLoop 3 -sck root'), 2)

    def test_ratio_change_and_reset_invalidate_applied_route(self):
        source = """Module Divider {
  ClockPort ck; ToClockPort out { Source ck; FreqDivider 2; }
}
Module Top {
  ClockPort root; ScanInPort si;
  ScanRegister mode { ScanInSource si; ResetValue 0; }
  ScanOutPort so { Source mode; }
  Instance divided Of Divider { InputPort ck = root; }
  ClockMux mux SelectedBy mode { 1'b0: root; 1'b1: divided.out; }
  ToClockPort out { Source mux; }
}"""
        self.rejected(source, 'iState mode 1; iClock out; iReset; iRunLoop 2 -sck out\n',
                      'changed since iClock')
        result = self.replay(source, 'iState mode 1; iClock out; iReset; iClock out; iRunLoop 2 -sck out\n')
        self.assertIn('iRunLoop 2 -sck root', result)

    def test_uncovered_disabled_and_invalid_targets(self):
        for choice, state, diagnostic in [
            ("1'b0: clk;", 1, 'uncovered clock selector'),
            ("1'b0: 1'b0; 1'b1: clk;", 0, 'inactive or inverted')]:
            with self.subTest(choice=choice):
                source = f"""Module Top {{ ClockPort clk; DataInPort sel;
  ClockMux mux SelectedBy sel {{ {choice} }}
  ToClockPort out {{ Source mux; }} }}"""
                self.rejected(source, f'iState sel {state}; iClock out\n', diagnostic)
        for body, target in [('DataInPort data;', 'data'),
                             ('ClockPort ck[1:0];', 'ck'),
                             ('ClockPort ck; Alias ali = ck;', 'ali')]:
            with self.subTest(target=target):
                self.rejected('Module Top { ' + body + ' }', f'iClock {target}\n',
                              'target requires one ClockPort')

    def test_clock_only_fanout_boundary(self):
        for extra in [
            'ToTCKPort observed { Source root; }',
            "DataMux observed SelectedBy 1'b0 { 1'b0: root; }",
            "ClockMux branch SelectedBy 1'b0 { 1'b0: other; 1'b1: root; } "
            "DataMux observed SelectedBy 1'b0 { 1'b0: branch; }",
            'Alias bad = ~root;',
            'ClockPort negative { DifferentialInvOf root; }',
        ]:
            with self.subTest(extra=extra):
                self.rejected('Module Top { ClockPort root; ClockPort other; ' + extra + ' }',
                              'iClock root; iRunLoop 1 -sck root\n', 'unsupported fanout')

    def test_unsupported_clock_paths_and_invalid_factors(self):
        for body, diagnostic in [
            ('ToClockPort out { Period 10ns; }', 'oscillator/differential'),
            ('ToClockPort out;', 'no supported system-clock source'),
            ('ClockPort root; ToClockPort out { Source ~root; }', 'inactive or inverted'),
            ('ClockPort root; ToClockPort out { Source root; FreqDivider 0; }', 'must be positive'),
            ('ClockPort root; ToClockPort out { Source root; FreqMultiplier 0; }', 'must be positive'),
        ]:
            with self.subTest(body=body):
                self.rejected('Module Top { ' + body + ' }', 'iClock out\n', diagnostic)

    def test_checked_arithmetic_and_factor_specialization(self):
        source = """Module Divider {
  Parameter ratio = 7; ClockPort ck;
  ToClockPort out { Source ck; FreqDivider $ratio; }
}
Module Top { ClockPort root;
  Instance child Of Divider { InputPort ck = root; Parameter ratio = 3; }
}"""
        result = self.replay(source, 'iClock child.out; iRunLoop 7 -sck child.out\n')
        self.assertIn('iRunLoop 21 -sck root', result)
        self.rejected(source, 'iClock child.out; iRunLoop 18446744073709551615 -sck child.out\n',
                      'cycle count limit')
        source = """Module Multiplier { ClockPort ck;
  ToClockPort out { Source ck; FreqMultiplier 9223372036854775807; }
}
Module Top { ClockPort root;
  Instance a Of Multiplier { InputPort ck = root; }
  ToClockPort out { Source a.out; FreqMultiplier 3; }
}"""
        self.rejected(source, 'iClock out\n', 'frequency ratio limit')
        cancelled = source.replace('FreqMultiplier 3', 'FreqDivider 9223372036854775807')
        result = self.replay(cancelled, 'iClock out; iRunLoop 18446744073709551615 -sck out\n')
        self.assertIn('iRunLoop 18446744073709551615 -sck root', result)

    def test_wait_does_not_clock_storage(self):
        source = """Module Top {
  ClockPort root; ScanInPort si;
  ScanRegister value[3:0] { ScanInSource si; CaptureSource 4'b0; }
  ScanOutPort so { Source value[0]; }
}"""
        result = self.replay(source, 'iState value 9; iClock root; iRunLoop 17 -sck root; iRead value 9; iApply\n')
        self.assertIn('iRead value 0b1001', result)

    def test_unchecked_second_target_on_same_root(self):
        self.rejected('Module Top { ClockPort root; ToClockPort out { Source root; } }',
                      'iClock root; iRunLoop 0 -sck out\n', 'requires prior iClock')

    def test_local_reset_preview_retains_checked_clocks(self):
        source = """Module Top {
  ClockPort root; ScanInPort si;
  ScanRegister value { ScanInSource si; ResetValue 0; }
  ScanOutPort so { Source value; }
}"""
        result = self.replay(source, """iReset; iClock root
iProc work {} {
  iState value 1; iWrite value 0; iApply; iRunLoop 1 -sck root
}
iCall work
""")
        self.assertIn('iRunLoop 1 -sck root', result)

    def test_deep_and_wide_clock_routes(self):
        for wide in (False, True):
            with self.subTest(wide=wide):
                if wide:
                    body = 'ClockPort root[65535:0]; ToClockPort out[255:0] { Source root[255:0]; FreqDivider 3; }'
                    commands = ''.join(f'iClock out({i % 256}); iRunLoop 2 -sck out({i % 256})\n' for i in range(1024))
                else:
                    body = 'ClockPort root;'
                    for i in range(4096):
                        previous = 'root' if i == 0 else f'c{i-1}.out'
                        body += f'Instance c{i} Of Stage {{ InputPort ck = {previous}; }}\n'
                    commands = 'iClock c4095.out; iRunLoop 2 -sck c4095.out\n' * 1024
                source = 'Module Top { ' + body + ' }'
                if not wide:
                    source = 'Module Stage { ClockPort ck; ToClockPort out { Source ck; } }\n' + source
                begin = time.monotonic()
                self.replay(source, commands)
                print(f'system clocks wide={wide} bytes={len(source)} checks=1024 forms=5 seconds={time.monotonic()-begin:.6f}', flush=True)


if __name__ == '__main__':
    unittest.main()
