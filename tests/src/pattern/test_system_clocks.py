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
        for definition in ['Period 10ns;', 'DifferentialInvOf root;']:
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
        self.rejected(source, 'iClockOverride child.osc -source root\n', 'unsupported')

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
        for extra in ('ToTCKPort observed { Source child.osc; }',
                      'ClockPort negative { DifferentialInvOf child.osc; }',
                      'Alias inverted = ~child.osc;'):
            with self.subTest(extra=extra):
                self.rejected('Module Osc { ToClockPort osc { Period 10ns; } }\n'
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


if __name__ == '__main__':
    unittest.main()
