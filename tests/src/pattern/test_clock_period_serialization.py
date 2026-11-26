#!/usr/bin/env python3
"""Exact period operands without changing native clock program semantics."""
from decimal import Decimal, localcontext
import json
import time
import unittest

import test_clock_period_overrides as periods
import test_system_clocks as helpers

SCALES = {'s': 18, 'ms': 15, 'us': 12, 'ns': 9, 'ps': 6, 'fs': 3, 'as': 0}


class ClockPeriodSerialization(unittest.TestCase):
    setUp = helpers.SystemClocks.setUp
    run_tool = helpers.SystemClocks.run_tool
    files = helpers.SystemClocks.files
    replay = helpers.SystemClocks.replay

    def test_integer_and_default_units_are_separate_words(self):
        for value, unit in [('20', unit) for unit in SCALES] + [
                ('17', ''), ('1000', 'ns'), ('00020', 'ns')]:
            with self.subTest(value=value, unit=unit):
                output = self.replay(periods.OSC,
                                     f'iClockOverride osc -period {value}{unit}; '
                                     'iClock osc; iRunLoop 2 -sck osc\n')
                lines = [line.split() for line in output.splitlines()
                         if line.startswith('iClockOverride ')]
                self.assertEqual(lines, [['iClockOverride', 'osc', '-period', value, unit or 'ns']])

    def test_exact_decimal_and_scientific_refinement(self):
        cases = [('1.25', 'us', '1250', 'ns'), ('1.0e-9', 's', '1', 'ns'),
                 ('001.2500', 'us', '1250', 'ns'), ('10.0e-3', 'ms', '10', 'us'),
                 ('1.25e-3', 'ms', '1250', 'ns'), ('0.001', 'ns', '1', 'ps'),
                 ('1.0e-18', 's', '1', 'as'), ('1.25', 'fs', '1250', 'as'),
                 ('1000.0', 'ns', '1000', 'ns'), ('1.0e-0', 's', '1', 's')]
        for value, unit, integer, result_unit in cases:
            for separator in ('', ' '):
                with self.subTest(value=value, unit=unit, separator=separator):
                    output = self.replay(periods.OSC,
                                         f'iClockOverride osc -period {value}{separator}{unit}; '
                                         'iClock osc; iRunLoop 3 -sck osc\n')
                    lines = [line.split() for line in output.splitlines()
                             if line.startswith('iClockOverride ')]
                    self.assertEqual(lines, [['iClockOverride', 'osc', '-period',
                                              integer, result_unit]])
                    with localcontext() as context:
                        context.prec = 80
                        original = Decimal(value) * Decimal(10) ** SCALES[unit]
                        emitted = Decimal(lines[0][3]) * Decimal(10) ** SCALES[lines[0][4]]
                        self.assertEqual(emitted, original)

    def test_native_only_literals_remain_lossless(self):
        for value, unit in [('0.1', 'as'), ('1.0e-19', 's'),
                            ('1.0e-9223372036854775808', 'as'),
                            ('1.0e-18446744073709551616', 's'),
                            ('1.0e-999999999999999999999999999999', 'ns')]:
            with self.subTest(value=value, unit=unit):
                output = self.replay(periods.OSC,
                                     f'iClockOverride osc -period {value}{unit}; '
                                     'iClock osc; iRunLoop 3 -sck osc\n')
                lines = [line.split() for line in output.splitlines()
                         if line.startswith('iClockOverride ')]
                self.assertEqual(lines, [['iClockOverride', 'osc', '-period', value, unit]])

    def test_procedures_and_mixed_overrides_keep_order(self):
        output = self.replay(periods.TREE, '''iClock slow
iProc configure {value unit} {iClockOverride pll.osc -period $value $unit}
iCall configure 1.25 us
iClock fast; iRunLoop 3 -sck slow; iRunLoop 4 -sck fast
iClockOverride pll.osc -source root -freqMultiplier 3 -freqDivider 2
iClock slow; iRunLoop 9 -sck slow
''')
        self.assertEqual([line for line in output.splitlines()
                          if line.startswith(('iClock ', 'iClockOverride ', 'iRunLoop '))], [
            'iClock slow', 'iClockOverride pll.osc -period 1250 ns', 'iClock fast',
            'iRunLoop 3 -sck slow', 'iRunLoop 4 -sck fast',
            'iClockOverride pll.osc -source root -freqMultiplier 3 -freqDivider 2',
            'iClock slow', 'iRunLoop 9 -sck slow'])

    def test_large_exact_cancellation_is_bounded(self):
        zeros = 100000
        value = '1' + '0' * zeros + f'.0e-{zeros}s'
        program = f'iClockOverride osc -period {value}; iClock osc; iRunLoop 2 -sck osc\n'
        started = time.monotonic()
        output = self.replay(periods.OSC, program)
        elapsed = time.monotonic() - started
        actual = [line for line in output.splitlines() if line.startswith('iClockOverride ')]
        self.assertTrue(actual == ['iClockOverride osc -period 1 s'],
                        'the long coefficient and exponent must cancel exactly')
        self.assertLess(elapsed, 10)
        print(json.dumps(dict(kind='period-literal', coefficient_zeros=zeros,
                              input_bytes=len(program), forms=5, seconds=elapsed)))


if __name__ == '__main__':
    unittest.main()
