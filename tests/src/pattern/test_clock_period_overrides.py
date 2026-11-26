#!/usr/bin/env python3
"""Explicit periods must survive retargeting with their logical clock snapshots."""
import json
import subprocess
import time
import unittest

import test_system_clocks as helpers

OSC = 'Module Top {ToClockPort osc {Period 10ns;}}'
TREE = '''Module Pll {
 ClockPort ck;
 ToClockPort osc {Source ck; FreqMultiplier 3; FreqDivider 2;}
}
Module Top {
 ClockPort root; ClockPort other;
 Instance pll Of Pll {InputPort ck=root;}
 ToClockPort slow {Source pll.osc; FreqDivider 2;}
 ToClockPort fast {Source pll.osc; FreqMultiplier 3;}
}'''


class ClockPeriodOverrides(unittest.TestCase):
    setUp = helpers.SystemClocks.setUp
    run_tool = helpers.SystemClocks.run_tool
    files = helpers.SystemClocks.files
    replay = helpers.SystemClocks.replay
    rejected = helpers.SystemClocks.rejected

    def test_period_default_units_and_literal_forms(self):
        for spelling, expected in (('20', '20 ns'), ('20 ns', '20 ns'),
                                   ('1.25us', '1250 ns'), ('1.0e-9s', '1 ns'),
                                   ('7as', '7 as')):
            with self.subTest(period=spelling):
                output = self.replay(OSC, f'iClockOverride osc -period {spelling}; '
                                     'iClock osc; iRunLoop 2 -sck osc\n')
                self.assertIn(f'iClockOverride osc -period {expected}\n', output)
                self.assertIn('iClock osc\niRunLoop 2 -sck osc\n', output)

    def test_independent_checked_targets_keep_period_versions(self):
        output = self.replay(TREE, '''iClock slow
iClockOverride pll.osc -period 20ns
iClock fast
iRunLoop 5 -sck slow; iRunLoop 7 -sck fast
iClockOverride pll.osc -period 1us
iRunLoop 3 -sck slow; iRunLoop 4 -sck fast
iClock slow; iRunLoop 9 -sck slow
''')
        clocks = [line for line in output.splitlines()
                  if line.startswith(('iClock ', 'iClockOverride ', 'iRunLoop '))]
        self.assertEqual(clocks, [
            'iClock slow', 'iClockOverride pll.osc -period 20 ns', 'iClock fast',
            'iRunLoop 5 -sck slow', 'iRunLoop 7 -sck fast',
            'iClockOverride pll.osc -period 1 us', 'iRunLoop 3 -sck slow',
            'iRunLoop 4 -sck fast', 'iClock slow', 'iRunLoop 9 -sck slow'])

    def test_explicit_mixed_sources_and_ratios_are_retained(self):
        output = self.replay(TREE, '''
iClockOverride pll.osc -source other -freqMultiplier 6 -freqDivider 1
iClock slow; iRunLoop 3 -sck slow
iClockOverride pll.osc -period 20ns
iClock fast; iRunLoop 7 -sck fast
iClockOverride pll.osc -source root -freqMultiplier 3 -freqDivider 2
iClock slow; iRunLoop 9 -sck slow
''')
        self.assertIn('iClockOverride pll.osc -source other -freqMultiplier 6 -freqDivider 1', output)
        self.assertIn('iClockOverride pll.osc -source root -freqMultiplier 3 -freqDivider 2', output)
        self.assertIn('iRunLoop 3 -sck slow', output)
        self.assertIn('iRunLoop 7 -sck fast', output)

    def test_hierarchy_vectors_procedures_and_reset(self):
        source = '''Module ClockCell {ToClockPort out[5:4];}
Module Top {Instance cell Of ClockCell;}'''
        output = self.replay(source, '''iProcsForModule ClockCell
iProc configure {value unit} {
 iClockOverride out(5) -period $value $unit
 iClock out[5]; iRunLoop 2 -sck out[5]
}
iProcsForModule Top
iCall cell.configure 17 ns
iClockOverride cell.out[4] -period 2us
iClock cell.out(4); iRunLoop 3 -sck cell.out[4]
iReset; iRunLoop 5 -sck cell.out(5)
''')
        self.assertIn('iClockOverride cell.out(5) -period 17 ns', output)
        self.assertIn('iClockOverride cell.out(4) -period 2 us', output)
        self.assertIn('iRunLoop 5 -sck cell.out(5)', output)

    def test_invalid_periods_preserve_artifacts(self):
        for period in ('0', '0.0e-999ns', '-1ns', '1bogus', '1ns ns', '20 {}', '20us {}'):
            with self.subTest(period=period):
                self.rejected(OSC, f'iClockOverride osc -period {period}\n', 'period')
        for value in ('20', '20us'):
            self.rejected(OSC, f'set unit {{}}; iClockOverride osc -period {value} $unit\n',
                          'period')
        self.rejected(OSC, 'iClockOverride osc -period 20ns -freqDivider 2\n', 'period')

    def test_mixed_option_subsets_restore_icl_defaults(self):
        for mask in range(8):
            with self.subTest(mask=mask):
                options = []
                for bit, option, value in ((1, '-source', 'other'),
                                           (2, '-freqMultiplier', '7'),
                                           (4, '-freqDivider', '5')):
                    if mask & bit:
                        options += [option, value]
                source = 'other' if mask & 1 else 'pll.ck'
                multiplier, divider = (7 if mask & 2 else 3), (5 if mask & 4 else 2)
                partial = 'iClockOverride pll.osc' + ''.join(' ' + word for word in options)
                output = self.replay(TREE, '''iClockOverride pll.osc -source other -freqMultiplier 9 -freqDivider 4
iClock slow
iClockOverride pll.osc -period 20ns
''' + partial + '; iRunLoop 6 -sck slow; iClock fast; iRunLoop 6 -sck fast\n')
                lines = [line for line in output.splitlines()
                         if line.startswith(('iClock ', 'iClockOverride ', 'iRunLoop '))]
                self.assertEqual(lines, [
                    'iClockOverride pll.osc -source other -freqMultiplier 9 -freqDivider 4',
                    'iClock slow', 'iClockOverride pll.osc -period 20 ns',
                    f'iClockOverride pll.osc -source {source} -freqMultiplier {multiplier} -freqDivider {divider}',
                    'iRunLoop 6 -sck slow', 'iClock fast', 'iRunLoop 6 -sck fast'])

    def test_mixed_defaults_in_vector_procedures_and_reset(self):
        source = '''Module Cell {ClockPort ck[3:2];
 ToClockPort out[5:4] {Source ck; FreqMultiplier 3; FreqDivider 2;}}
Module Top {ClockPort root[7:6]; Instance cell Of Cell {InputPort ck=root;}}'''
        output = self.replay(source, '''iProcsForModule Cell
iProc configure {} {iClockOverride out[5] -freqDivider 5; iClock out[5]}
iProcsForModule Top
iClockOverride cell.out[4] -period 20ns
iCall cell.configure
iRunLoop 6 -sck cell.out[5]
iReset
iClockOverride cell.out[4]
iClock cell.out[4]; iRunLoop 6 -sck cell.out[4]
''')
        self.assertIn('iClockOverride cell.out(5) -source cell.ck(3) -freqMultiplier 3 -freqDivider 5\n', output)
        self.assertIn('iClockOverride cell.out(4) -source cell.ck(2) -freqMultiplier 3 -freqDivider 2\n', output)
        self.assertIn('iRunLoop 6 -sck cell.out(5)\n', output)

    def test_unrepresentable_mixed_defaults_preserve_artifacts(self):
        source = '''Module Top {ClockPort root; ClockPort other; DataInPort mode;
 ClockMux choose SelectedBy mode {1'b0:root;1'b1:other;}
 ToClockPort out {Source choose; FreqMultiplier 3; FreqDivider 2;}}'''
        prefix = 'iState mode 0; iClockOverride out -period 20ns; '
        self.rejected(source, prefix + 'iClockOverride out -freqDivider 5\n',
                      'source')
        output = self.replay(source, prefix + 'iClockOverride out -source other; '
                             'iClock out; iRunLoop 6 -sck out\n')
        self.assertIn('iClockOverride out -source other -freqMultiplier 3 -freqDivider 2\n', output)
        self.rejected(OSC, 'iClockOverride osc -period 20ns; '
                      'iClockOverride osc -freqDivider 5\n', 'source')

    def test_mixed_clock_defaults_scale(self):
        for count in (2, 8, 32):
            for updates in (8, 128, 1024):
                with self.subTest(count=count, updates=updates):
                    source = f'''Module Top {{ClockPort root[{count-1}:0];
 ToClockPort out[{count-1}:0] {{Source root; FreqMultiplier 3; FreqDivider 2;}}
 ToClockPort oscillator {{Period 10ns;}}}}'''
                    commands = ['iClockOverride oscillator -period 20ns']
                    expected = ['iClockOverride oscillator -period 20 ns']
                    for i in range(updates):
                        bit, divider = i % count, 1 + i % 7
                        commands += [f'iClockOverride out[{bit}] -freqDivider {divider}',
                                     f'iClock out[{bit}]', f'iRunLoop 6 -sck out[{bit}]']
                        expected += [f'iClockOverride out({bit}) -source root({bit}) -freqMultiplier 3 -freqDivider {divider}',
                                     f'iClock out({bit})', f'iRunLoop 6 -sck out({bit})']
                    started = time.monotonic()
                    output = self.replay(source, '; '.join(commands) + '\n')
                    elapsed = time.monotonic() - started
                    actual = [line for line in output.splitlines()
                              if line.startswith(('iClockOverride ', 'iClock ', 'iRunLoop '))]
                    self.assertEqual(actual, expected)
                    self.assertLess(elapsed, 10)
                    print(json.dumps(dict(oscillators=count, updates=updates, forms=5, seconds=elapsed)))

    def test_level_zero_numeric_formals(self):
        for formals, period, arguments, expected in (
                ('p', '$p', '20', '20 ns'), ('p', '$p us', '20', '20 us'),
                ('p unit', '$p $unit', '20 ps', '20 ps')):
            with self.subTest(period=period):
                icl, pdl = self.files(OSC, f'iProc configure {{{formals}}} {{'
                                     f'iClockOverride osc -period {period}; iClock osc}}; '
                                     f'iCall configure {arguments}; iRunLoop 3 -sck osc\n')
                pdl.write_text(pdl.read_text().replace('iPDLLevel 1', 'iPDLLevel 0'))
                output = self.run_tool(icl, pdl, reset=True)
                flat = self.root / 'debug/Top.icl'
                for i, (model, commands) in enumerate(((flat, pdl),
                                                       (icl, self.root / 'result.pdl'),
                                                       (flat, self.root / 'result.pdl'),
                                                       (icl, self.root / 'reset.pdl'))):
                    self.assertEqual(output, self.run_tool(model, commands,
                                                          f'formal-{i}/result.pdl'))
                self.assertIn(f'iClockOverride osc -period {expected}\n', output)
        icl, pdl = self.files(OSC, 'iProc configure {p} {iClockOverride osc -period $p us}; '
                             'iCall configure 0\n')
        pdl.write_text(pdl.read_text().replace('iPDLLevel 1', 'iPDLLevel 0'))
        target = self.root / 'result.pdl'
        target.write_text('previous artifact\n')
        self.assertIn('positive', self.run_tool(icl, pdl, ok=False))
        self.assertEqual(target.read_text(), 'previous artifact\n')

    def test_encoded_vector_override_source_is_canonical(self):
        source = '''Module Cell {ClockPort ref[9:8]; ToClockPort out[5:4] {Source ref;}}
Module Top {ClockPort root[2:1]; Instance c Of Cell {InputPort ref=root;}}'''
        original = ('iClockOverride c.out(5) -period 7 ps; iClock c.out(5); '
                    'iClockOverride c.out(4) -source c.ref(8) -freqMultiplier 2 -freqDivider 1; '
                    'iClock c.out(4); iRunLoop 3 -sck c.out(4); iRunLoop 9 -sck c.out(5)\n')
        physical = original.replace('c.out', 'n' + b'c.out'.hex()).replace(
            'c.ref', 'n' + b'c.ref'.hex())
        output = helpers.SystemClocks.physical_clock_replay(self, source, original, physical)
        self.assertEqual([line for line in output.splitlines()
                          if line.startswith('iClockOverride ')], [
            'iClockOverride c.out(5) -period 7 ps',
            'iClockOverride c.out(4) -source c.ref(8) -freqMultiplier 2 -freqDivider 1'])

    def test_local_reset_groups_and_actual_pio_history(self):
        source = TREE.replace('ClockPort root;',
                              'ClockPort root; DataInPort bus[3:0]; '
                              'DataOutPort observed[3:0] {Source bus;}')
        program = '''iProc setup {} {
 iState bus 0; iClock slow; iWrite bus 9; iApply
}
iCall setup
iClockOverride pll.osc -period 20ns
iClock fast; iRunLoop 5 -sck slow; iRunLoop 7 -sck fast
iRead observed 9; iApply
'''
        output = self.replay(source, program)
        self.assertIn('iClock slow', output)
        self.assertIn('iRunLoop 5 -sck slow', output)
        # Modeled histories still exclude clocks; exercise actual PIO separately.
        icl, pdl = self.files(source, 'iState bus 0; iWrite bus 9; iApply; '
                              'iRead observed 0; iApply; ' +
                              'iNote actual=[iGetReadData observed -bin]; '
                              'iNote [iGetStatus -clear]; iNote [iGetStatus]\n')
        generated = self.root / 'modeled/result.pdl'
        flat = self.root / 'modeled/debug/Top.icl'
        for i, (model, commands) in enumerate(((icl, pdl),
                                               (flat, pdl),
                                               (icl, generated),
                                               (flat, generated))):
            target = generated if i == 0 else self.root / f'modeled-{i}/result.pdl'
            run = subprocess.run([str(helpers.BIN), '--icl', str(model), '--pdl', str(commands),
                                  '--top_module', 'Top', '--output', str(target), '--debug',
                                  '--model_readback'], text=True, capture_output=True, timeout=45)
            self.assertEqual(run.returncode, 0, run.stdout + run.stderr)
            data = target.read_text()
            self.assertIn('iNote actual=0b1001', data)
            self.assertIn('iNote 2\niNote 0', data)
        icl, pdl = self.files(source, program + 'iNote [iGetReadData observed -bin]\n')
        target = self.root / 'guarded.pdl'
        target.write_text('previous artifact\n')
        run = subprocess.run([str(helpers.BIN), '--icl', str(icl), '--pdl', str(pdl),
                              '--top_module', 'Top', '--output', str(target),
                              '--model_readback'], text=True, capture_output=True, timeout=45)
        self.assertNotEqual(run.returncode, 0)
        self.assertIn('does not support iClock histories', run.stdout + run.stderr)
        self.assertEqual(target.read_text(), 'previous artifact\n')

    def test_pending_physical_and_unsafe_boundaries(self):
        source = TREE.replace('ClockPort root;', 'ClockPort root; DataInPort data;')
        self.rejected(source, 'iClockOverride pll.osc -period 20ns; iClock slow; '
                      'iWrite data 1; iRunLoop 1 -sck slow\n', 'unapplied')
        self.rejected(TREE.replace('ToClockPort slow',
                                   'ToTCKPort observed {Source pll.osc;} ToClockPort slow'),
                      'iClockOverride pll.osc -period 20ns\n', 'unsupported fanout')
        self.rejected('Module Top {ClockPort root;}',
                      'iClockOverride root -period 20ns\n', 'ToClockPort')
        self.rejected(OSC, 'iClockOverride osc -period 20ns; iRunLoop 1 -sck osc\n',
                      'prior iClock')
        icl, pdl = self.files(OSC, 'iClockOverride osc -period 20ns; '
                             'iClock osc; iRunLoop 1 -sck osc\n')
        target = self.root / 'physical.pdl'
        target.write_text('previous artifact\n')
        run = subprocess.run([str(helpers.BIN), '--icl', str(icl), '--pdl', str(pdl),
                              '--top_module', 'Top', '--output', str(target), '--physical_pdl'],
                             text=True, capture_output=True, timeout=45)
        self.assertNotEqual(run.returncode, 0)
        self.assertIn('physical PDL', run.stdout + run.stderr)
        self.assertEqual(target.read_text(), 'previous artifact\n')

    def test_period_clock_program_scale(self):
        for count in (2, 8, 32):
            for updates in (8, 128, 1024):
                with self.subTest(count=count, updates=updates):
                    source = f'Module Top {{ToClockPort osc[{count-1}:0] {{Period 10ns;}}}}'
                    commands, expected = [], []
                    for i in range(updates):
                        bit, period, cycles = i % count, 11 + i % 97, 1 + i % 7
                        commands += [f'iClockOverride osc[{bit}] -period {period}ns',
                                     f'iClock osc[{bit}]', f'iRunLoop {cycles} -sck osc[{bit}]']
                        expected += [f'iClockOverride osc({bit}) -period {period} ns',
                                     f'iClock osc({bit})', f'iRunLoop {cycles} -sck osc({bit})']
                    started = time.monotonic()
                    output = self.replay(source, '; '.join(commands) + '\n')
                    elapsed = time.monotonic() - started
                    actual = [line for line in output.splitlines()
                              if line.startswith(('iClockOverride ', 'iClock ', 'iRunLoop '))]
                    self.assertEqual(actual, expected)
                    self.assertLess(elapsed, 10)
                    print(json.dumps(dict(oscillators=count, updates=updates, forms=5, seconds=elapsed)))


if __name__ == '__main__':
    unittest.main()
