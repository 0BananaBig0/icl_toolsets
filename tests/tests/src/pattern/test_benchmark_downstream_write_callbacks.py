#!/usr/bin/env python3
"""Callback benchmark oracles pin logical postconditions to physical frames."""
from pathlib import Path
import re
import sys
import unittest

ROOT = Path(__file__).resolve().parents[4]
sys.path.insert(0, str(ROOT / 'tests/src/pattern'))
import benchmark_downstream_write_callbacks as base
import benchmark_merged_callbacks as merged

# Literal output from the qualified two-target merged fixture.
CALLBACKS = '''iPDLLevel 0
iProcsForModule Chains
iReset
iNote r0=0b0001
iWrite en 0b0
iWrite p 0b0001
iApply
iState r0 0b0001
iNote r1=0b0001
iWrite en 0b0
iWrite p 0b0001
iApply
iState r1 0b0001
iWrite en 0b1
iApply
iNote r0=0b0010
iWrite en 0b0
iWrite p 0b0010
iApply
iState r0 0b0010
iNote r1=0b0010
iWrite en 0b0
iWrite p 0b0010
iApply
iState r1 0b0010
iWrite en 0b1
iApply
iNote r0=0b0011
iWrite en 0b0
iWrite p 0b0011
iApply
iState r0 0b0011
iNote r1=0b0011
iWrite en 0b0
iWrite p 0b0011
iApply
iState r1 0b0011
iWrite en 0b1
iApply
iNote r0=0b0100
iWrite en 0b0
iWrite p 0b0100
iApply
iState r0 0b0100
iNote r1=0b0100
iWrite en 0b0
iWrite p 0b0100
iApply
iState r1 0b0100
iWrite en 0b1
iApply
'''
UNUSED = '''iPDLLevel 0
iProcsForModule Chains
iReset
iWrite en 0b0
iWrite p 0b0001
iApply
iWrite en 0b0
iWrite p 0b0010
iApply
iWrite en 0b0
iWrite p 0b0011
iApply
iWrite en 0b0
iWrite p 0b0100
iApply
'''
FIRST = 'iState r0 0b0001\n'
WITHOUT_FIRST = CALLBACKS.replace(FIRST, '', 1)
CORRUPTIONS = {
    'payload': CALLBACKS.replace(FIRST, 'iState r0 0b0010\n', 1),
    'target_order': CALLBACKS.replace(FIRST, '@first@\n', 1).replace(
        'iState r1 0b0001\n', FIRST, 1).replace('@first@\n', 'iState r1 0b0001\n'),
    'before_frame': WITHOUT_FIRST.replace('iWrite en 0b0\n', FIRST + 'iWrite en 0b0\n', 1),
    'after_next_frame': WITHOUT_FIRST.replace('iState r1 0b0001\n', FIRST + 'iState r1 0b0001\n', 1),
}


class CallbackBenchmarkPostconditions(unittest.TestCase):
    checkers = (base.physical_counts, merged.checked_frames)

    def test_literal_callback_fixture(self):
        for check in self.checkers:
            with self.subTest(check=check.__name__):
                dimensions, frames = check(CALLBACKS, 2, 4, True)
                self.assertEqual(dimensions['callback_postconditions'], 8)
                self.assertEqual(dimensions['pio_frame_count'], 12)
                self.assertEqual(frames[0], [['en', '0b0'], ['p', '0b0001']])
                self.assertEqual(frames[-1], [['en', '0b1']])

    def test_unused_callbacks(self):
        for check in self.checkers:
            with self.subTest(check=check.__name__):
                dimensions, frames = check(UNUSED, 2, 4, False)
                self.assertEqual(dimensions['callback_postconditions'], 0)
                self.assertEqual(len(frames), 4)

    def test_empty_logical_applies_preserve_physical_positions(self):
        text = re.sub(r'(iState r1 0b[01]+\n)', r'\1iApply\n', CALLBACKS)
        for check in self.checkers:
            with self.subTest(check=check.__name__):
                dimensions, frames = check(text, 2, 4, True)
                self.assertEqual(dimensions['empty_apply_count'], 4)
                self.assertEqual(len(frames), 12)

    def test_corrupt_postconditions_reject(self):
        for label, text in CORRUPTIONS.items():
            for check in self.checkers:
                with self.subTest(fault=label, check=check.__name__):
                    with self.assertRaisesRegex(RuntimeError, 'postcondition'):
                        check(text, 2, 4, True)


if __name__ == '__main__':
    unittest.main()
