#!/usr/bin/env python3
"""Functional-clock waits with source/flat/generated/reset replay."""
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

    def test_invariant_time_waits(self):
        values = ['1.0s', '2.5ms', '3.25us', '4.125ns', '5.0ps', '6.0fs', '7.0as',
                  '1.0e-9s', '0.0e-999999as', '0001.2500us', '10ns']
        commands = ''.join('iRunLoop -time ' + value + '; ' for value in values)
        for source, program in [
                ('Module Top { ClockPort root; ClockPort root_n { DifferentialInvOf root; } }',
                 commands),
                ('Module Top { ToClockPort osc { Period 3ns; } }',
                 'iClock osc; iRunLoop 3 -sck osc; iRunLoop -time 1.25us; iRunLoop 2 -sck osc'),
                ('Module Top { ClockPort clk; DataInPort d; DataOutPort out { Source d; } }',
                 'iWrite d 1; iApply; iRunLoop -time 1.25us; iRead out 1; iApply'),
                ('Module Top { ClockPort clk; }',
                 'iProc pause {delay} { iRunLoop -time $delay }; iCall pause 1.25us'),
                ('Module Top { ClockPort clk; ClockPort other; ToClockPort out { Source clk; FreqDivider 2; } }',
                 'iClock out; iClockOverride out -source other -freqDivider 3; '
                 'iRunLoop -time 1.25us; iRunLoop 2 -sck out; iClock out; '
                 'iRunLoop -time 2.5us; iRunLoop 2 -sck out')]:
            with self.subTest(source=source, program=program):
                result = self.replay(source, program)
                self.assertIn('iRunLoop -time', result)
                if program == commands:
                    for value in values:
                        self.assertIn('iRunLoop -time ' + value + '\n', result)
                if '-source other' in program:
                    self.assertIn('iRunLoop 4 -sck clk', result)
                    self.assertIn('iRunLoop 6 -sck other', result)

    def test_invariant_time_wait_boundaries(self):
        for source, program, diagnostic in [
                ('Module Top { ScanInPort si; ScanRegister r { ScanInSource si; } '
                 'ScanOutPort so { Source r; } }',
                 'iRunLoop -time 1.25us', 'absolute period models'),
                ('Module Stage { ClockPort ck; ToClockPort out { Source ck; } } '
                 'Module Top { ClockPort root; Instance child Of Stage { InputPort ck = root; } '
                 'ToTCKPort observed { Source child.out; } }',
                 'iRunLoop -time 1.25us', 'clock fanout'),
                ('Module Top { DataInPort d; ToTCKPort forwarded { Source d; } }',
                 'iWrite d 1; iApply; iTake d; iRunLoop -time 1.25us',
                 'test-clock forwarding'),
                ('Module Endpoint { TCKPort tck; } Module Top { DataInPort d; '
                 'DataOutPort out { Source d; } '
                 'Instance child Of Endpoint { InputPort tck = d; } }',
                 'iWrite d 1; iApply; iTake d; iRunLoop -time 1.25us',
                 'test-clock forwarding'),
                ('Module Top { ClockPort clk; }',
                 'iState clk 0; iRunLoop -time 1.25us', 'state assumption for'),
                ('Module Top { ClockPort clk[5:4]; }',
                 'iState clk(5) 0; iRunLoop -time 0ns', 'state assumption for'),
                ('Module Top { ClockPort clk; }',
                 'iRunLoop -time 1.25us; iState clk 1; iRunLoop -time 2ns', 'state assumption for'),
                ('Module Top { ClockPort clk; }',
                 'set delay nonsense; iRunLoop -time $delay', 'invalid PDL time value'),
                ('Module Top { ClockPort clk; }',
                 'iRunLoop -time 1.25us -sck clk', 'iRunLoop'),
                ('Module Top { ClockPort clk; DataInPort d; }',
                 'iWrite d 1; iRunLoop -time 1.25us', 'unapplied'),
                ('Module Top { ClockPort clk; DataInPort d; }',
                 'iRead d 1; iRunLoop -time 1.25us', 'unapplied')]:
            with self.subTest(source=source, program=program):
                self.rejected(source, program + '\n', diagnostic)

    def test_invariant_time_wait_scale(self):
        depth = 4096
        stages = ["ClockMux stage0 SelectedBy 1'b0 { 1'b0: root; }"]
        stages += [f"ClockMux stage{i} SelectedBy 1'b0 {{ 1'b0: stage{i - 1}; }}"
                   for i in range(1, depth)]
        pairs = [f'Instance pair{i} Of Pair {{ InputPort p = stage4095; InputPort n = root_n; }}'
                 for i in range(depth)]
        source = ('Module Pair { ClockPort p; ClockPort n { DifferentialInvOf p; } } '
                  'Module Top { ClockPort root; ClockPort root_n { DifferentialInvOf root; } '
                  + ' '.join(stages + pairs) + ' }')
        start = time.monotonic()
        result = self.replay(source, 'iRunLoop -time 1.25us; ' * 1024)
        self.assertEqual(result.count('iRunLoop -time 1.25us'), 1024)
        print(f'invariant time pairs={depth} stages={depth} bytes={len(source)} '
              f'waits=1024 forms=5 seconds={time.monotonic() - start:.6f}', flush=True)

    def physical_clock_replay(self, source, original, physical, late_metadata=False):
        icl, pdl = self.files(source, original)
        expected = self.run_tool(icl, pdl)
        flat = self.root / 'debug/Top.icl'
        if late_metadata:
            lines = flat.read_text().splitlines(keepends=True)
            metadata = next(line for line in lines
                            if line.lstrip().startswith('Attribute icl_pattern_origins_v1'))
            lines.remove(metadata)
            lines.insert(len(lines) - 1, metadata)
            flat.write_text(''.join(lines))
        raw = self.root / 'physical.pdl'
        raw.write_text('iPDLLevel 1\niProcsForModule Top\n' + physical)
        result = self.run_tool(flat, raw, 'physical/result.pdl', reset=True)
        self.assertEqual(expected, result)
        self.assertEqual(expected, self.run_tool(icl, self.root / 'physical/result.pdl',
                                                'generated-source/result.pdl'))
        self.assertEqual(expected, self.run_tool(flat, self.root / 'physical/result.pdl',
                                                'generated-flat/result.pdl'))
        self.assertEqual(expected, self.run_tool(flat, self.root / 'reset.pdl',
                                                'reset-flat/result.pdl'))
        return result

    def test_flat_clock_physical_targets(self):
        for source, original, physical, expected in [
                ('Module Top { ClockPort clk; }',
                 'iClock clk; iRunLoop 7 -sck clk\n',
                 'iClock n636c6b; iRunLoop 7 -sck n636c6b\n', 'iRunLoop 7 -sck clk'),
                ('Module Top { ClockPort clk[5:4]; }',
                 'iClock clk(5); iRunLoop 3 -sck clk(5); iClock clk(4); iRunLoop 2 -sck clk(4)\n',
                 'iClock n636c6b(5); iRunLoop 3 -sck clk(5); iClock clk(4); iRunLoop 2 -sck n636c6b[4]\n',
                 'iRunLoop 2 -sck clk(4)'),
                ('Module Stage { ClockPort ck; ToClockPort out { Source ck; FreqDivider 3; } } '
                 'Module Top { ClockPort clk; Instance child Of Stage { InputPort ck = clk; } }',
                 'iClock child.out; iRunLoop 2 -sck child.out\n',
                 'iClock n6368696c642e6f7574; iRunLoop 2 -sck child.out\n', 'iRunLoop 6 -sck clk')]:
            with self.subTest(source=source):
                self.assertIn(expected, self.physical_clock_replay(source, original, physical))

    def test_flat_clock_physical_snapshot_versions(self):
        source = ('Module Top { ClockPort clk; ClockPort other; '
                  'ToClockPort out { Source clk; FreqDivider 2; } }')
        original = ('iClock out; iRunLoop 2 -sck out; '
                    'iClockOverride out -source other -freqDivider 3; '
                    'iRunLoop 2 -sck out; iClock out; iRunLoop 2 -sck out\n')
        physical = original.replace('iClock out', 'iClock n6f7574').replace('-sck out', '-sck n6f7574')
        result = self.physical_clock_replay(source, original, physical)
        self.assertIn('iRunLoop 4 -sck clk', result)
        self.assertIn('iRunLoop 6 -sck other', result)

    def test_flat_clock_physical_name_boundaries(self):
        source = ('Module Top { ClockPort clk; DataInPort d; Alias view = clk; '
                  'ToClockPort out { Source clk; } }')
        icl, pdl = self.files(source, 'iClock clk; iRunLoop 1 -sck clk\n')
        self.run_tool(icl, pdl)
        flat = (self.root / 'debug/Top.icl').read_text()
        for commands, diagnostic in [
                ('iClock n636C6b\n', 'unknown PDL target'),
                ('iClock n64\n', 'unknown PDL target'),
                ('iClock n76696577\n', 'unknown PDL target'),
                ('iClock n636c6b(0)\n', 'scalar target cannot be indexed'),
                ('iClock n636c6b(\n', 'invalid PDL object identifier'),
                ('iClockOverride n6f7574 -freqDivider 3\n', 'unknown PDL target'),
                ('iClockOverride out -source n636c6b\n', 'unknown PDL target'),
                ('iWrite n64 1; iApply\n', 'unknown PDL target')]:
            with self.subTest(commands=commands):
                self.rejected(flat, commands, diagnostic)
        self.rejected(source, 'iClock n636c6b\n', 'unknown PDL target')
        literal = 'n726f6f74'
        result = self.physical_clock_replay(
            'Module Top { ClockPort root; ClockPort ' + literal + '; }',
            'iClock ' + literal + '; iRunLoop 3 -sck ' + literal + '\n',
            'iClock ' + literal + '; iRunLoop 3 -sck n' + literal.encode().hex() + '\n')
        self.assertIn('iRunLoop 3 -sck ' + literal, result)

    def test_flat_clock_physical_name_scale(self):
        depth = 4096
        stages = ["ClockMux stage0 SelectedBy 1'b0 { 1'b0: root; }"]
        stages += [f"ClockMux stage{i} SelectedBy 1'b0 {{ 1'b0: stage{i - 1}; }}"
                   for i in range(1, depth)]
        pairs = [f'Instance pair{i} Of Pair {{ InputPort p = stage4095; InputPort n = root_n; }}'
                 for i in range(depth)]
        source = ('Module Pair { ClockPort p; ClockPort n { DifferentialInvOf p; } } '
                  'Module Top { ClockPort root; ClockPort root_n { DifferentialInvOf root; } '
                  + ' '.join(stages + pairs) + ' }')
        original = ''.join(f'iClock pair{i}.n; iRunLoop 1 -sck pair{i}.n; ' for i in range(1024))
        physical = ''.join('iClock n' + f'pair{i}.n'.encode().hex() +
                           '; iRunLoop 1 -sck n' + f'pair{i}.n'.encode().hex() + '; '
                           for i in range(1024))
        start = time.monotonic()
        result = self.physical_clock_replay(source, original, physical, late_metadata=True)
        self.assertEqual(result.count('iRunLoop 1 -sck root'), 1024)
        print(f'physical clocks pairs={depth} stages={depth} bytes={len(source)} '
              f'waits=1024 forms=5 seconds={time.monotonic() - start:.6f}', flush=True)

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
            ("1'b0: 1'b0; 1'b1: clk;", 0, 'inactive system-clock')]:
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
        for extra, diagnostic in [
            ('ToTCKPort observed { Source root; }', 'unsupported fanout'),
            ("DataMux observed SelectedBy 1'b0 { 1'b0: root; }",
             'data signal or supported control source'),
            ("ClockMux branch SelectedBy 1'b0 { 1'b0: other; 1'b1: root; } "
             "DataMux observed SelectedBy 1'b0 { 1'b0: branch; }",
             'data signal or supported control source'),
        ]:
            with self.subTest(extra=extra):
                self.rejected('Module Top { ClockPort root; ClockPort other; ' + extra + ' }',
                              'iClock root; iRunLoop 1 -sck root\n', diagnostic)

    def test_differential_root_and_output_targets(self):
        source = """Module Top {
  ClockPort positive[1:0];
  ClockPort negative[1:0] { DifferentialInvOf positive; }
  Alias swap[0:1] = positive[0], positive[1];
  ToClockPort converted[0:1] { DifferentialInvOf swap; }
}"""
        result = self.replay(source, 'iClock negative(1); iRunLoop 3 -sck negative(1); '
                             'iClock converted(0); iRunLoop 5 -sck converted(0)\n')
        self.assertIn('iClock positive(1)\niRunLoop 3 -sck positive(1)', result)
        self.assertIn('iClock positive(0)\niRunLoop 5 -sck positive(0)', result)

    def test_differential_hierarchical_ratio_and_oscillator(self):
        source = """Module Ratio {
  ClockPort input;
  ToClockPort positive { Source input; FreqMultiplier 3; FreqDivider 2; }
  ToClockPort negative { DifferentialInvOf positive; }
}
Module Osc {
  ToClockPort positive { Period 10ps; }
  ToClockPort negative { DifferentialInvOf positive; }
}
Module Top {
  ClockPort root;
  Instance ratio Of Ratio { InputPort input = root; }
  Instance osc Of Osc;
}"""
        result = self.replay(source, 'iProcsForModule Ratio\n'
                             'iProc wait {} { iClock negative; iRunLoop 10 -sck negative }\n'
                             'iProcsForModule Top\niCall ratio.wait\n'
                             'iPrefix osc; iClock negative; iRunLoop 4 -sck negative\n')
        self.assertIn('iRunLoop 7 -sck root', result)
        self.assertIn('iClock osc.positive\niRunLoop 4 -sck osc.positive', result)

    def test_differential_mux_checks(self):
        source = """Module Top {
  ClockPort a; ClockPort b; DataInPort mode;
  ClockMux mux SelectedBy mode { 1'b0: a; 1'b1: b; }
  ToClockPort negative { DifferentialInvOf mux; }
}"""
        result = self.replay(source, 'iState mode 0; iClock negative; '
                             'iRunLoop 2 -sck negative; iState mode 1; '
                             'iClock negative; iRunLoop 3 -sck negative\n')
        self.assertIn('iClock a\niRunLoop 2 -sck a', result)
        self.assertIn('iClock b\niRunLoop 3 -sck b', result)
        self.rejected(source, 'iClock negative\n', 'unknown clock selector')
        self.rejected(source, 'iState mode 0; iClock negative; iState mode 1; '
                      'iRunLoop 2 -sck negative\n', 'changed since iClock')

    def test_differential_fanout_and_bound_input_limits(self):
        self.rejected('Module Top { ClockPort positive; '
                      'ClockPort negative { DifferentialInvOf positive; } '
                      'ToTCKPort observed { Source negative; } }',
                      'iClock positive\n', 'requires a clock or data signal')
        source = ('Module Pair { ClockPort positive; '
                  'ClockPort negative { DifferentialInvOf positive; } } '
                  'Module Top { ClockPort root; Instance pair Of Pair { '
                  'InputPort positive = root; InputPort negative = root; } }')
        self.rejected(source, 'iClock pair.negative\n', 'bound differential')
        result = self.replay(source, 'iClock root; iRunLoop 2 -sck root\n')
        self.assertIn('iRunLoop 2 -sck root', result)

    # IEEE1687-2014 6.4.6.19: a bound negative input must complement its partner.
    def test_bound_differential_explicit_inverse_and_alias(self):
        for negative, top in [
                ('~root', ''),
                ('inverse', 'Alias inverse = ~root;')]:
            with self.subTest(negative=negative):
                source = ("Module Pair { ClockPort positive; ClockPort negative { "
                          "DifferentialInvOf positive; } } Module Top { ClockPort root; "
                          + top + " Instance pair Of Pair { InputPort positive = root; "
                          "InputPort negative = " + negative + "; } }")
                result = self.replay(source, 'iClock pair.negative; '
                                     'iRunLoop 7 -sck pair.negative\n')
                self.assertIn('iClock root\niRunLoop 7 -sck root', result)

    def test_bound_differential_shared_conversion_and_oscillator(self):
        for origin, count in [('Source input; FreqDivider 3;', 12), ('Period 10ns;', 4)]:
            with self.subTest(origin=origin):
                source = ("Module Pair { ClockPort positive; ClockPort negative { "
                          "DifferentialInvOf positive; } } Module Generator { ClockPort input; "
                          "ToClockPort output { " + origin + " } } "
                          "Module Top { ClockPort root; Instance generator Of Generator { "
                          "InputPort input = root; } Instance pair Of Pair { "
                          "InputPort positive = generator.output; "
                          "InputPort negative = ~generator.output; } }")
                result = self.replay(source, 'iClock pair.negative; iRunLoop 4 -sck pair.negative\n')
                target = 'generator.output' if 'Period' in origin else 'root'
                self.assertIn(f'iClock {target}\niRunLoop {count} -sck {target}', result)

    def test_bound_differential_vector_permutation(self):
        source = """Module Pair {
  ClockPort positive[0:1];
  ClockPort negative[0:1] { DifferentialInvOf positive; }
}
Module Top {
  ClockPort root[5:4];
  Alias swapped[1:0] = root[4], root[5];
  Instance pair Of Pair {
    InputPort positive = swapped;
    InputPort negative = ~swapped;
  }
}"""
        result = self.replay(source, 'iClock pair.negative(0); iRunLoop 3 -sck pair.negative(0); '
                             'iClock pair.negative(1); iRunLoop 5 -sck pair.negative(1)\n')
        self.assertIn('iRunLoop 3 -sck root(4)', result)
        self.assertIn('iRunLoop 5 -sck root(5)', result)

    def test_bound_differential_rejects_unproven_bindings(self):
        for positive, negative in [('root', 'root'), ('root', 'other'),
                                   ('root', '~first.output'),
                                   ('first.output', '~second.output')]:
            with self.subTest(negative=negative):
                source = ("Module Pair { ClockPort positive; ClockPort negative { "
                          "DifferentialInvOf positive; } } Module Divider { ClockPort input; "
                          "ToClockPort output { Source input; FreqDivider 2; } } "
                          "Module Top { ClockPort root; ClockPort other; "
                          "Instance first Of Divider { InputPort input = root; } "
                          "Instance second Of Divider { InputPort input = root; } "
                          "Instance pair Of Pair { InputPort positive = " + positive +
                          "; InputPort negative = " + negative + "; } }")
                self.rejected(source, 'iClock pair.negative\n', 'bound differential')

    def test_bound_differential_mux_drift_and_recovery(self):
        source = """Module Pair {
  ClockPort positive;
  ClockPort negative { DifferentialInvOf positive; }
}
Module Top {
  ClockPort root; ClockPort other; DataInPort sel;
  ClockMux route SelectedBy sel { 1'b0: ~root; 1'b1: ~other; }
  Instance pair Of Pair { InputPort positive = root; InputPort negative = route; }
}"""
        result = self.replay(source, 'iState sel 0; iClock pair.negative; '
                             'iRunLoop 3 -sck pair.negative; '
                             'iState sel 1; '
                             'iState sel 0; iClock pair.negative; iRunLoop 2 -sck pair.negative\n')
        self.assertIn('iRunLoop 3 -sck root', result)
        self.assertIn('iRunLoop 2 -sck root', result)
        self.rejected(source, 'iState sel 0; iClock pair.negative; iState sel 1; '
                      'iRunLoop 0 -sck pair.negative\n', 'bound differential')
        self.rejected(source, 'iClock pair.negative\n', 'unknown clock selector')

    def test_bound_differential_shared_route_scale(self):
        depth = 4096
        stages = ["ClockMux stage0 SelectedBy 1'b0 { 1'b0: root; }"]
        stages += [f"ClockMux stage{i} SelectedBy 1'b0 {{ 1'b0: stage{i - 1}; }}"
                   for i in range(1, depth)]
        pairs = [f'Instance pair{i} Of Pair {{ InputPort positive = stage{depth - 1}; '
                 f'InputPort negative = ~stage{depth - 1}; }}' for i in range(depth)]
        source = ('Module Pair { ClockPort positive; ClockPort negative { '
                  'DifferentialInvOf positive; } } Module Top { ClockPort root; '
                  + ' '.join(stages + pairs) + ' }')
        commands = ''.join(f'iClock pair{i}.negative; iRunLoop 1 -sck pair{i}.negative; '
                           for i in range(1024))
        start = time.monotonic()
        result = self.replay(source, commands)
        self.assertEqual(result.count('iRunLoop 1 -sck root'), 1024)
        print(f'bound differential pairs={depth} stages={depth} bytes={len(source)} '
              f'waits=1024 forms=5 seconds={time.monotonic() - start:.6f}', flush=True)

    def test_flat_clock_legacy_forwarding(self):
        for positive, negative, extra, count in [
                ('root', 'root_n', '', 3),
                ('root', 'wrapped', 'Alias wrapped = root_n;', 3),
                ('gen.output', 'gen.output_n',
                 'Instance gen Of Generator { InputPort input = root; }', 9)]:
            with self.subTest(negative=negative):
                source = ("Module Pair { ClockPort p; ClockPort n { DifferentialInvOf p; } } "
                          "Module Generator { ClockPort input; ToClockPort output { "
                          "Source input; FreqDivider 3; } ToClockPort output_n { "
                          "DifferentialInvOf output; } } Module Top { ClockPort root; "
                          "ClockPort root_n { DifferentialInvOf root; } " + extra +
                          " Instance pair Of Pair { InputPort p = " + positive +
                          "; InputPort n = " + negative + "; } }")
                result = self.replay(source, 'iClock pair.n; iRunLoop 3 -sck pair.n\n')
                self.assertIn(f'iRunLoop {count} -sck root', result)
                flat = (self.root / 'debug/Top.icl').read_text()
                self.assertIn('ToClockPort n706169722e70', flat)
                self.assertIn('ToClockPort n706169722e6e', flat)
                legacy = flat
                for name, kind in [('n706169722e70', 'DataOutPort'),
                                   ('n706169722e6e', 'ClockPort'),
                                   ('n67656e2e696e707574', 'DataOutPort')]:
                    legacy = legacy.replace('ToClockPort ' + name, kind + ' ' + name)
                path = self.root / 'legacy.icl'
                path.write_text(legacy)
                self.assertEqual(result, self.run_tool(path, self.root / 'source.pdl',
                                                       'legacy/result.pdl'))
                self.assertEqual(result, self.run_tool(path, self.root / 'result.pdl',
                                                       'legacy-generated/result.pdl'))
                self.assertEqual(flat, (self.root / 'legacy/debug/Top.icl').read_text())

    # Tessent2026.2 ICL65-1 permits named secondary sources on negative inputs.
    def test_secondary_clock_named_input_sources(self):
        for positive, negative, extra, first, second in [
                ('root', 'root_n', '', 3, 2),
                ('root', 'wrapped', 'Alias wrapped = root_n;', 3, 2),
                ('gen.output', 'gen.output_n',
                 'Instance gen Of Generator { InputPort input = root; }', 9, 6)]:
            with self.subTest(negative=negative):
                source = ("Module Pair { ClockPort positive; ClockPort negative { "
                          "DifferentialInvOf positive; } } Module Generator { ClockPort input; "
                          "ToClockPort output { Source input; FreqDivider 3; } "
                          "ToClockPort output_n { DifferentialInvOf output; } } "
                          "Module Top { ClockPort root; ClockPort root_n { DifferentialInvOf root; } "
                          + extra + " Instance pair Of Pair { InputPort positive = " + positive +
                          "; InputPort negative = " + negative + "; } }")
                result = self.replay(source, 'iClock pair.positive; iRunLoop 3 -sck pair.positive; '
                                     'iClock pair.negative; iRunLoop 2 -sck pair.negative\n')
                waits = [line for line in result.splitlines() if line.startswith('iRunLoop')]
                self.assertEqual(waits, [f'iRunLoop {first} -sck root', f'iRunLoop {second} -sck root'])

    def test_secondary_clock_nested_vector_bindings(self):
        source = """Module Pair {
  ClockPort positive[0:1];
  ClockPort negative[0:1] { DifferentialInvOf positive; }
}
Module Wrapper {
  ClockPort positive[1:0];
  ClockPort negative[1:0] { DifferentialInvOf positive; }
  Instance pair Of Pair { InputPort positive = positive; InputPort negative = negative; }
}
Module Top {
  ClockPort root[5:4]; ClockPort root_n[5:4] { DifferentialInvOf root; }
  Instance child Of Wrapper {
    InputPort positive = root[4], root[5]; InputPort negative = root_n[4], root_n[5];
  }
}"""
        result = self.replay(source, 'iClock child.pair.positive(0); '
                             'iRunLoop 3 -sck child.pair.positive(0); '
                             'iClock child.pair.negative(1); iRunLoop 7 -sck child.pair.negative(1)\n')
        waits = [line for line in result.splitlines() if line.startswith('iRunLoop')]
        self.assertEqual(waits, ['iRunLoop 3 -sck root(4)', 'iRunLoop 7 -sck root(5)'])

    def test_secondary_clock_partial_binding_holes(self):
        source = """Module Pair {
  ClockPort positive[1:0]; ClockPort negative[1:0] { DifferentialInvOf positive; }
}
Module Top {
  ClockPort root[1:0]; ClockPort root_n[1:0] { DifferentialInvOf root; }
  Instance pair Of Pair { InputPort positive = root; InputPort negative[1] = root_n[1]; }
}"""
        result = self.replay(source, 'iClock pair.negative(1); iRunLoop 9 -sck pair.negative(1)\n')
        self.assertIn('iRunLoop 9 -sck root(1)', result)
        self.rejected(source, 'iClock pair.negative(0)\n', 'inactive system-clock')

    def test_secondary_clock_kind_scope_and_phase_boundaries(self):
        base = ("Module Pair { ClockPort positive; ClockPort negative { DifferentialInvOf positive; } } "
                "Module Top { ClockPort root; ClockPort root_n { DifferentialInvOf root; } "
                "Alias wrapped = root_n; EXTRA Instance pair Of Pair { "
                "InputPort positive = root; InputPort negative = root_n; } }")
        for extra, diagnostic in [
                ('ToClockPort copied { Source wrapped; }', 'copied: Source requires a clock or data signal'),
                ("ClockMux copied SelectedBy 1'b0 { 1'b0: wrapped; }", 'copied: clock choice requires'),
                ('DataOutPort copied { Source wrapped; }', 'copied: Source requires a data signal'),
                ('ClockPort copied { DifferentialInvOf wrapped; }', 'copied: DifferentialInvOf requires')]:
            with self.subTest(extra=extra):
                self.rejected(base.replace('EXTRA', extra), 'iClock pair.positive\n', diagnostic)
        wrong_positive = base.replace('EXTRA', '').replace('InputPort positive = root;',
                                                          'InputPort positive = root_n;')
        self.rejected(wrong_positive, 'iClock pair.positive\n', 'pair.positive: Source requires')
        wrong_root = base.replace('EXTRA', 'ClockPort other; ClockPort other_n { DifferentialInvOf other; }')
        wrong_root = wrong_root.replace('InputPort negative = root_n;', 'InputPort negative = other_n;')
        self.rejected(wrong_root, 'iClock pair.negative\n', 'bound differential')
        inverted = base.replace('EXTRA', '').replace('InputPort negative = root_n;',
                                                   'InputPort negative = ~root_n;')
        self.rejected(inverted, 'iClock pair.negative\n', 'bound differential')
        provider = ("Module Generator { ClockPort input; ToClockPort output { Source input; } "
                    "ToClockPort output_n { DifferentialInvOf output; } Alias hidden = output_n; } ")
        cross_alias = provider + base.replace('EXTRA', 'Instance gen Of Generator { InputPort input = root; }')
        cross_alias = cross_alias.replace('InputPort negative = root_n;', 'InputPort negative = gen.hidden;')
        self.rejected(cross_alias, 'iClock pair.negative\n', 'hierarchical signal gen.hidden must name an output port')

    def test_inverted_clock_duration_routes(self):
        cases = [
            ('ToClockPort out { Source ~root; FreqDivider 3; }', 6),
            ('Alias inverse = ~root; ToClockPort out { Source inverse; FreqDivider 3; }', 6),
            ('Alias one = ~root; Alias two = ~one; ToClockPort out { Source two; }', 2),
            ('ToClockPort out { DifferentialInvOf ~root; }', 2),
        ]
        for body, count in cases:
            with self.subTest(body=body):
                result = self.replay('Module Top { ClockPort root; ' + body + ' }',
                                     'iClock out; iRunLoop 2 -sck out\n')
                self.assertIn(f'iClock root\niRunLoop {count} -sck root', result)
        result = self.replay('Module Top { ClockPort root; Alias inverse = ~root; }',
                             'iClock root; iRunLoop 2 -sck root\n')
        self.assertIn('iRunLoop 2 -sck root', result)
        result = self.replay(
            'Module Osc { ToClockPort osc { Period 10ns; } } '
            'Module Top { Instance child Of Osc; Alias inverse = ~child.osc; '
            'ToClockPort out { Source inverse; } }',
            'iClock out; iRunLoop 2 -sck out\n')
        self.assertIn('iClock child.osc\niRunLoop 2 -sck child.osc', result)

    def test_inverted_clock_mux_vector_and_hierarchy(self):
        source = """Module Top {
  ClockPort root[1:0]; DataInPort mode;
  Alias inverse[0:1] = ~root[0], ~root[1];
  ClockMux mux SelectedBy mode { 1'b0: inverse[0]; 1'b1: inverse[1]; }
  ToClockPort out { Source mux; FreqDivider 3; }
}"""
        result = self.replay(source, 'iState mode 0; iClock out; iRunLoop 2 -sck out; '
                             'iState mode 1; iClock out; iRunLoop 3 -sck out\n')
        self.assertIn('iRunLoop 6 -sck root(0)', result)
        self.assertIn('iRunLoop 9 -sck root(1)', result)
        result = self.replay(
            "Module Top { ClockPort root; DataInPort mode; "
            "ClockMux mux SelectedBy mode { 1'b0: root; 1'b1: ~root; } "
            "ToClockPort out { Source mux; } }",
            'iState mode 0; iClock out; iState mode 1; iRunLoop 2 -sck out\n')
        self.assertIn('iRunLoop 2 -sck root', result)
        source = """Module Stage {
  ClockPort input;
  ToClockPort out { Source ~input; FreqMultiplier 3; FreqDivider 2; }
}
Module Top { ClockPort root; Instance child Of Stage { InputPort input = ~root; } }
"""
        result = self.replay(source, 'iClock child.out; iRunLoop 10 -sck child.out\n')
        self.assertIn('iRunLoop 7 -sck root', result)

    def test_literal_clock_paths_remain_inactive(self):
        for value in ("1'b0", "1'b1", "1'bx", "~1'b0", "~1'b1"):
            with self.subTest(value=value):
                self.rejected('Module Top { ToClockPort out { Source ' + value + '; } }',
                              'iClock out; iRunLoop 0 -sck out\n', 'inactive system-clock')

    def test_inverted_clock_override_snapshots(self):
        source = ('Module Top { ClockPort root; ClockPort other; '
                  'ToClockPort out { Source ~root; FreqDivider 2; } }')
        result = self.replay(source, 'iClock out; '
                             'iClockOverride out -freqMultiplier 3; iRunLoop 6 -sck out; '
                             'iClock out; iRunLoop 6 -sck out; '
                             'iClockOverride out -source other; iRunLoop 6 -sck out; '
                             'iClock out; iRunLoop 6 -sck out\n')
        waits = [line for line in result.splitlines() if line.startswith('iRunLoop')]
        self.assertEqual(waits, ['iRunLoop 12 -sck root', 'iRunLoop 4 -sck root',
                                 'iRunLoop 4 -sck root', 'iRunLoop 12 -sck other'])

    def test_differential_clock_scale(self):
        for wide in (False, True):
            if wide:
                source = ('Module Top { ClockPort positive[65535:0]; '
                          'ClockPort negative[65535:0] { '
                          'DifferentialInvOf positive; } }')
                target, root = 'negative(65535)', 'positive(65535)'
            else:
                source = ('Module Stage { ClockPort input; '
                          'ToClockPort positive { Source input; } '
                          'ToClockPort negative { DifferentialInvOf positive; } } '
                          'Module Top { ClockPort root; ')
                previous = 'root'
                for i in range(4096):
                    source += f'Instance c{i} Of Stage {{ InputPort input = {previous}; }} '
                    previous = f'c{i}.positive'
                source += '}'
                target, root = 'c4095.negative', 'root'
            start = time.monotonic()
            result = self.replay(source, f'iClock {target}; iRunLoop 2 -sck {target}\n' * 1024)
            self.assertEqual(result.count(f'iRunLoop 2 -sck {root}'), 1024)
            print(f'differential clocks wide={wide} bytes={len(source)} checks=1024 '
                  f'forms=5 seconds={time.monotonic() - start:.6f}', flush=True)

    def test_differential_override_source_and_cycle(self):
        source = ('Module Top { ClockPort root; ClockPort other; '
                  'ToClockPort positive { Source root; FreqDivider 2; } '
                  'ToClockPort negative { DifferentialInvOf positive; } '
                  'ToClockPort out { Source other; } }')
        result = self.replay(source, 'iClock out; '
                             'iClockOverride out -source negative; '
                             'iRunLoop 3 -sck out; iClock out; iRunLoop 3 -sck out\n')
        self.assertIn('iRunLoop 3 -sck other', result)
        self.assertIn('iRunLoop 6 -sck root', result)
        self.rejected(source, 'iClock positive; '
                      'iClockOverride positive -source negative; '
                      'iRunLoop 1 -sck positive\n', 'cyclic system-clock override')

    def test_unsupported_clock_paths_and_invalid_factors(self):
        for body, diagnostic in [
            ('ToClockPort out;', 'no supported system-clock source'),
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

    def test_override_preserves_checked_scaling(self):
        source = """Module Ratio {
  ClockPort ck; ToClockPort out { Source ck; FreqMultiplier 3; FreqDivider 2; }
}
Module Top { ClockPort root; Instance child Of Ratio { InputPort ck = root; } }
"""
        result = self.replay(source, """iClock child.out
iRunLoop 3 -sck child.out
iClockOverride child.out -freqMultiplier 6 -freqDivider 1
iRunLoop 3 -sck child.out
iClock child.out
iRunLoop 3 -sck child.out
iClockOverride child.out -freqDivider 5
iRunLoop 3 -sck child.out
iClock child.out
iRunLoop 3 -sck child.out
""")
        waits = [line for line in result.splitlines() if line.startswith('iRunLoop')]
        self.assertEqual(waits, [f'iRunLoop {n} -sck root' for n in (2, 2, 1, 1, 5)])
        self.assertNotIn('iClockOverride', result)

    def test_override_omissions_restore_icl_defaults(self):
        source = """Module Ratio {
  ClockPort ck; ToClockPort out { Source ck; FreqMultiplier 3; FreqDivider 2; }
}
Module Top { ClockPort root; ClockPort other;
  Instance child Of Ratio { InputPort ck = root; }
}"""
        result = self.replay(source, """iClock child.out
iClockOverride child.out -source other -freqMultiplier 6 -freqDivider 1
iRunLoop 6 -sck child.out
iClock child.out; iRunLoop 6 -sck child.out
iClockOverride child.out
iClock child.out; iRunLoop 6 -sck child.out
""")
        waits = [line for line in result.splitlines() if line.startswith('iRunLoop')]
        self.assertEqual(waits, ['iRunLoop 4 -sck root', 'iRunLoop 1 -sck other',
                                 'iRunLoop 4 -sck root'])

    def test_override_versions_are_per_checked_target(self):
        source = """Module Ratio {
  ClockPort ck; ToClockPort out { Source ck; FreqMultiplier 3; FreqDivider 2; }
}
Module Top { ClockPort root;
  Instance child Of Ratio { InputPort ck = root; }
  ToClockPort copy { Source child.out; FreqMultiplier 2; }
}"""
        result = self.replay(source, """iClock child.out
iClockOverride child.out -freqMultiplier 6 -freqDivider 1
iClock copy
iRunLoop 6 -sck child.out; iRunLoop 6 -sck copy
iRunLoop 6 -sck child.out; iRunLoop 6 -sck copy
""")
        waits = [line for line in result.splitlines() if line.startswith('iRunLoop')]
        self.assertEqual(waits, [f'iRunLoop {n} -sck root' for n in (4, 1, 4, 1)])

    def test_override_survives_procedure_return_and_reset(self):
        source = """Module Top { ClockPort root;
  ToClockPort out { Source root; FreqDivider 2; }
}"""
        result = self.replay(source, """iProc configure {} {
  iClockOverride out -freqMultiplier 6 -freqDivider 1
}
iCall configure
iReset
iClock out; iRunLoop 3 -sck out
""")
        self.assertIn('iRunLoop 1 -sck root', result)

    def test_override_vector_bit_isolation(self):
        source = """Module Stage { ClockPort ck[0:1];
  ToClockPort out[1:0] { Source ck; FreqDivider 2; }
}
Module Top { ClockPort root[5:4]; ClockPort other;
  Instance child Of Stage { InputPort ck = root; }
}"""
        result = self.replay(source, """iClockOverride child.out(1) -source other -freqMultiplier 3 -freqDivider 1
iClock child.out(1); iClock child.out(0)
iRunLoop 3 -sck child.out(1); iRunLoop 3 -sck child.out(0)
""")
        self.assertIn('iRunLoop 1 -sck other', result)
        self.assertIn('iRunLoop 6 -sck root(4)', result)

    def test_override_snapshot_revalidates_old_mux(self):
        source = """Module Stage { ClockPort a; ClockPort b; DataInPort mode;
  ClockMux mux SelectedBy mode { 1'b0: a; 1'b1: b; }
  ToClockPort out { Source mux; FreqMultiplier 3; FreqDivider 2; }
}
Module Top { ClockPort first; ClockPort second; DataInPort mode;
  Instance child Of Stage { InputPort a = first; InputPort b = second; InputPort mode = mode; }
}"""
        commands = """iState mode 0; iClock child.out
iClockOverride child.out -source second -freqMultiplier 6 -freqDivider 1
iRunLoop 3 -sck child.out
"""
        result = self.replay(source, commands)
        self.assertIn('iRunLoop 2 -sck first', result)
        self.rejected(source, commands + 'iState mode 1; iRunLoop 3 -sck child.out\n',
                      'changed since iClock')
        result = self.replay(source, commands + 'iState mode 1; iClock child.out; iRunLoop 3 -sck child.out\n')
        self.assertIn('iRunLoop 1 -sck second', result)

    def test_override_state_in_local_reset_preview(self):
        source = """Module Top { ClockPort root; ToClockPort out { Source root; FreqDivider 2; }
  ScanInPort si; ScanRegister value { ScanInSource si; ResetValue 0; }
  ScanOutPort so { Source value; }
}"""
        result = self.replay(source, """iReset; iClock out
iClockOverride out -freqMultiplier 6 -freqDivider 1
iProc work {} {
  iState value 1; iWrite value 0; iApply; iRunLoop 3 -sck out
  iClock out; iRunLoop 3 -sck out
}
iCall work
""")
        self.assertIn('iRunLoop 6 -sck root', result)
        self.assertIn('iRunLoop 1 -sck root', result)

    def test_override_cycle_cannot_hide_behind_old_check(self):
        source = """Module Stage { ClockPort ck; ToClockPort out { Source ck; } }
Module Top { ClockPort root;
  Instance a Of Stage { InputPort ck = root; }
  Instance b Of Stage { InputPort ck = root; }
}"""
        self.rejected(source, """iClock a.out
iClockOverride a.out -source b.out
iClockOverride b.out -source a.out
iRunLoop 1 -sck a.out
""", 'cyclic system-clock')

    def test_override_source_can_establish_unconnected_port(self):
        result = self.replay('Module Top { ClockPort root; ToClockPort out; }',
                            'iClockOverride out -source root -freqDivider 3; iClock out; iRunLoop 2 -sck out\n')
        self.assertIn('iRunLoop 6 -sck root', result)
        self.rejected('Module Top { ClockPort root; ToClockPort out; }',
                      'iClockOverride out -freqDivider 3\n', 'requires a source')

    def test_override_preserves_pending_queue(self):
        source = 'Module Top { ClockPort root; ToClockPort out { Source root; } DataInPort data; }'
        result = self.replay(source, 'iWrite data 1; iClockOverride out -freqDivider 3; iClock out; iApply; iRunLoop 2 -sck out\n')
        self.assertLess(result.index('iClock root'), result.index('iWrite data'))
        self.assertIn('iRunLoop 6 -sck root', result)
        self.rejected(source, 'iWrite data 1; iClockOverride out -freqDivider 3; iClock out; iRunLoop 0 -sck out\n',
                      'unapplied')

    def test_override_invalid_arguments_are_atomic(self):
        source = 'Module Top { ClockPort root; ToClockPort out { Source root; } DataInPort data; }'
        for command, diagnostic in [
            ('iClockOverride root -freqDivider 3', 'requires a ToClockPort'),
            ('iClockOverride out -source data', 'target requires one ClockPort'),
            ('iClockOverride out -freqDivider 0', 'positive'),
            ('iClockOverride out -freqMultiplier 0bX', 'known'),
            ('iClockOverride out -freqDivider 18446744073709551616', 'truncates significant bits'),
            ('iClockOverride out -freqMultiplier -1', ''),
            ('iClockOverride out -freqDivider 2 -freqDivider 3', ''),
            ('iClockOverride out -unknown 3', ''),
        ]:
            with self.subTest(command=command):
                self.rejected(source, command + '\n', diagnostic)
        for definition in ['DifferentialInvOf root;']:
            self.rejected('Module Top { ClockPort root; ToClockPort out { ' + definition + ' } }',
                          'iClockOverride out -source root\n', 'oscillator/differential')

    def test_override_cannot_introduce_unsafe_fanout(self):
        source = """Module Stage { ClockPort ck; ToClockPort out { Source ck; } }
Module Top { ClockPort root; ClockPort other;
  Instance child Of Stage { InputPort ck = other; }
  ToTCKPort unsupported { Source child.out; }
}"""
        self.rejected(source, 'iClock root; iClockOverride child.out -source root\n', 'unsupported fanout')

    def test_override_uint64_factors_and_count_overflow(self):
        source = 'Module Top { ClockPort root; ToClockPort out { Source root; } }'
        largest = 18446744073709551615
        result = self.replay(source, f'iClockOverride out -freqMultiplier {largest} -freqDivider {largest}; iClock out; iRunLoop {largest} -sck out\n')
        self.assertIn(f'iRunLoop {largest} -sck root', result)
        self.rejected(source, f'iClockOverride out -freqDivider {largest}; iClock out; iRunLoop 2 -sck out\n',
                      'cycle count limit')

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


    def test_oscillator_period_forms_and_units(self):
        for spelling, canonical in [('17', '17 ns'), ('1_024', '1024 ns')] + [
                (f'10{gap}{unit}', f'10 {unit}')
                for unit in ('s', 'ms', 'us', 'ns', 'ps', 'fs', 'as') for gap in ('', ' ')]:
            with self.subTest(period=spelling):
                result = self.replay(f'Module Top {{ ToClockPort osc {{ Period {spelling}; }} }}',
                                     'iClock osc; iRunLoop 0 -sck osc; iRunLoop 7 -sck osc\n')
                self.assertIn('iClock osc\niRunLoop 0 -sck osc\niRunLoop 7 -sck osc', result)
                self.assertIn(f'Period {canonical};', (self.root / 'debug/Top.icl').read_text())

    def test_oscillator_hierarchy_vectors_and_scaling(self):
        source = '''Module Osc { ToClockPort osc[3:2] { Period 9223372036854775807 as; } }
Module Top {
  Instance child Of Osc;
  ToClockPort slow { Source child.osc[2]; FreqMultiplier 3; FreqDivider 10; }
}'''
        result = self.replay(source, '''iProcsForModule Osc
iProc wait {} { iClock osc(3); iRunLoop 9 -sck osc(3) }
iProcsForModule Top
iCall child.wait
iClock slow; iRunLoop 2 -sck slow; iRunLoop 3 -sck slow
''')
        self.assertIn('iRunLoop 9 -sck child.osc(3)', result)
        self.assertIn('iRunLoop 7 -sck child.osc(2)\niRunLoop 10 -sck child.osc(2)', result)

    def test_oscillator_equal_period_roots_are_distinct(self):
        source = '''Module Osc { ToClockPort osc { Period 10ns; } }
Module Top { Instance a Of Osc; Instance b Of Osc; DataInPort sel;
  ClockMux mux SelectedBy sel { 1'b0: a.osc; 1'b1: b.osc; }
  ToClockPort out { Source mux; }
}'''
        self.rejected(source, 'iState sel 0; iClock out; iState sel 1; iRunLoop 1 -sck out\n',
                      'changed since iClock')
        result = self.replay(source, '''iState sel 0; iClock out; iRunLoop 2 -sck out
iState sel 1; iClock out; iRunLoop 3 -sck out
''')
        self.assertIn('iRunLoop 2 -sck a.osc', result)
        self.assertIn('iRunLoop 3 -sck b.osc', result)

    def test_oscillator_override_source_and_old_snapshot(self):
        source = '''Module Osc { ToClockPort osc { Period 10ps; } }
Module Top { ClockPort root; Instance child Of Osc;
  ToClockPort out { Source root; FreqDivider 2; }
}'''
        result = self.replay(source, '''iClock out
iClockOverride out -source child.osc -freqDivider 3
iRunLoop 2 -sck out; iClock out; iRunLoop 2 -sck out
''')
        self.assertIn('iRunLoop 4 -sck root', result)
        self.assertIn('iRunLoop 6 -sck child.osc', result)

    def test_oscillator_invalid_period_and_conflicts(self):
        for period, diagnostic in [('0', 'Period must be positive'),
                                   ('0fs', 'Period must be positive'),
                                   ('9223372036854775808ns', '64-bit limit')]:
            with self.subTest(period=period):
                self.rejected(f'Module Top {{ ToClockPort osc {{ Period {period}; }} }}',
                              'iClock osc\n', diagnostic)
        for extra in ('Source root;', 'FreqMultiplier 2;', 'FreqDivider 2;',
                      'DifferentialInvOf root;'):
            with self.subTest(extra=extra):
                self.rejected('Module Top { ClockPort root; ToClockPort osc { Period 10ns; ' + extra + ' } }',
                              'iClock osc\n', 'exclude other clock properties')

    def test_oscillator_unsafe_fanout(self):
        for extra in ('ToTCKPort observed { Source child.osc; }',):
            with self.subTest(extra=extra):
                self.rejected('Module Osc { ToClockPort osc { Period 10ns; } }\n'
                              'Module Pair { ClockPort positive; ClockPort negative { '
                              'DifferentialInvOf positive; } }\n'
                              'Module Top { Instance child Of Osc; ' + extra + ' }',
                              'iClock child.osc\n', 'unsupported fanout')

    def test_oscillator_scale(self):
        for wide in (False, True):
            if wide:
                source = 'Module Top { ToClockPort osc[65535:0] { Period 1as; } }'
                target = 'osc(65535)'
            else:
                source = 'Module Osc { ToClockPort osc { Period 10ns; } }\n'
                source += 'Module Stage { ClockPort ck; ToClockPort out { Source ck; } }\n'
                source += 'Module Top { Instance osc Of Osc;\n'
                for i in range(4096):
                    previous = 'osc.osc' if i == 0 else f'c{i-1}.out'
                    source += f'Instance c{i} Of Stage {{ InputPort ck = {previous}; }}\n'
                source += '}'
                target = 'c4095.out'
            begin = time.monotonic()
            result = self.replay(source, f'iClock {target}; iRunLoop 2 -sck {target}\n' * 1024)
            self.assertEqual(result.count('iRunLoop 2 -sck'), 1024)
            print(f'oscillator wide={wide} bytes={len(source)} waits=1024 forms=5 seconds={time.monotonic()-begin:.6f}', flush=True)


    def test_oscillator_target_source_override_snapshots(self):
        source = '''Module Osc { ToClockPort osc { Period 10ns; } }
Module Top { ClockPort root; Instance child Of Osc; }'''
        result = self.replay(source, '''iClock child.osc; iRunLoop 2 -sck child.osc
iClockOverride child.osc -source root -freqMultiplier 2 -freqDivider 1
iRunLoop 2 -sck child.osc; iClock child.osc; iRunLoop 2 -sck child.osc
''')
        self.assertEqual(result.count('iRunLoop 2 -sck child.osc'), 2)
        self.assertIn('iClock root\niRunLoop 1 -sck root', result)

    def test_oscillator_target_indexed_and_replacement_oscillator(self):
        source = '''Module Osc { ToClockPort osc[0:1] { Period 10ps; } }
Module Top { Instance a Of Osc; Instance b Of Osc; }'''
        result = self.replay(source, '''iClock a.osc(0); iClock a.osc(1)
iClockOverride a.osc(0) -source b.osc(1) -freqDivider 3
iRunLoop 4 -sck a.osc(0); iClock a.osc(0)
iRunLoop 4 -sck a.osc(0); iRunLoop 4 -sck a.osc(1)
''')
        self.assertIn('iRunLoop 4 -sck a.osc(0)', result)
        self.assertIn('iRunLoop 12 -sck b.osc(1)', result)
        self.assertIn('iRunLoop 4 -sck a.osc(1)', result)

    def test_oscillator_target_omitted_factors_restore_defaults(self):
        source = '''Module Osc { ToClockPort osc { Period 10ns; } }
Module Top { ClockPort root; Instance child Of Osc; }'''
        result = self.replay(source, '''iClockOverride child.osc -source root -freqMultiplier 2 -freqDivider 3
iClock child.osc; iRunLoop 2 -sck child.osc
iClockOverride child.osc -source root -freqMultiplier 4
iClock child.osc; iRunLoop 4 -sck child.osc
iClockOverride child.osc -source root -freqDivider 5
iClock child.osc; iRunLoop 2 -sck child.osc
''')
        for cycles in (3, 1, 10):
            self.assertIn(f'iRunLoop {cycles} -sck root', result)

    def test_oscillator_factor_only_keeps_intrinsic_check(self):
        source = '''Module Osc { ToClockPort osc { Period 10ns; } }
Module Top { ClockPort root; Instance child Of Osc; }'''
        for options in ('-freqMultiplier 2', '-freqDivider 3'):
            with self.subTest(options=options):
                result = self.replay(source, 'iClock child.osc; iRunLoop 4 -sck child.osc\n'
                                     f'iClockOverride child.osc {options}\n'
                                     'iRunLoop 4 -sck child.osc\n')
                self.assertIn('iRunLoop 4 -sck child.osc\niRunLoop 4 -sck child.osc', result)
                self.assertNotIn('iClockOverride', result)

    def test_oscillator_factor_only_keeps_replaced_check(self):
        source = '''Module Osc { ToClockPort osc { Period 10ns; } }
Module Top { ClockPort root; Instance child Of Osc; }'''
        for options in ('-freqMultiplier 2', '-freqDivider 3'):
            with self.subTest(options=options):
                result = self.replay(source, 'iClock child.osc; iRunLoop 4 -sck child.osc\n'
                                     'iClockOverride child.osc -source root -freqDivider 3\n'
                                     'iClock child.osc; iRunLoop 4 -sck child.osc\n'
                                     f'iClockOverride child.osc {options}\n'
                                     'iRunLoop 4 -sck child.osc\n')
                self.assertIn('iRunLoop 4 -sck child.osc', result)
                self.assertIn('iRunLoop 12 -sck root\niRunLoop 12 -sck root', result)
                self.assertNotIn('iClockOverride', result)

    def test_oscillator_target_override_errors(self):
        source = '''Module Osc { ToClockPort osc { Period 10ns; } }
Module Top { ClockPort root; Instance child Of Osc;
  ToClockPort out { Source child.osc; }
}'''
        for options in ('', '-freqMultiplier 2', '-freqDivider 3'):
            with self.subTest(options=options):
                check = 'iClock child.osc\n' if options else ''
                diagnostic = ('inactive system-clock path' if options else
                              'oscillator override requires explicit -source')
                self.rejected(source, f'iClockOverride child.osc {options}\n' + check,
                              diagnostic)
                self.rejected(source, 'iClockOverride child.osc -source root\n'
                              f'iClockOverride child.osc {options}\n' + check, diagnostic)
        for replacement in ('child.osc', 'out'):
            with self.subTest(replacement=replacement):
                self.rejected(source, f'iClock child.osc; iClockOverride child.osc -source {replacement}\n',
                              'cyclic')


if __name__ == '__main__':
    unittest.main()
