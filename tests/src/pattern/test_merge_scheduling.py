#!/usr/bin/env python3
"""Merge ordering, conservative batching and fixed-point regression."""
import pathlib
import subprocess
import sys
import tempfile
import time
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[3]
FIXTURE = ROOT / 'tests/cases/ours/pattern_generation/merge'
BIN = pathlib.Path(sys.argv.pop(1)).resolve() if len(sys.argv) > 1 else ROOT / 'build/icl-pattern-generation'


class MergeScheduling(unittest.TestCase):
    def setUp(self):
        temporary = tempfile.TemporaryDirectory()
        self.addCleanup(temporary.cleanup)
        self.root = pathlib.Path(temporary.name)
        (self.root / 'source.icl').write_bytes((FIXTURE / 'network.icl').read_bytes())

    def run_tool(self, *args, ok=True):
        result = subprocess.run([str(BIN), *map(str, args)], cwd=self.root,
                                capture_output=True, text=True, timeout=60)
        self.assertEqual(result.returncode == 0, ok, result.stdout + result.stderr)
        return result

    def generate(self, text, *args, ok=True):
        (self.root / 'source.pdl').write_text(text)
        result = self.run_tool('--icl', 'source.icl', '--pdl', 'source.pdl',
                               '--output', 'result.pdl', *args, ok=ok)
        return (self.root / 'result.pdl').read_text() if ok else result.stdout

    def program(self, first, second, body='iCall first\niCall second'):
        return ('iPDLLevel 1\niProcsForModule MergeNetwork\niReset\n'
                f'iProc first {{}} {{{first}}}\niProc second {{}} {{{second}}}\n'
                f'iMerge -begin\n{body}\niMerge -end\n')

    def groups(self, output):
        groups, pending = [], {}
        for line in output.splitlines():
            words = line.split()
            if words and words[0] == 'iWrite':
                pending[words[1]] = int(words[2], 2)
            elif words and words[0] == 'iApply':
                groups.append(pending)
                pending = {}
        self.assertFalse(pending)
        self.assertNotIn('iMerge', output)
        self.assertNotIn('iCall', output)
        return groups

    def test_independent_and_fixed_points(self):
        output = self.generate((FIXTURE / 'parallel.pdl').read_text(), '--debug')
        self.assertEqual(self.groups(output), [{'a': 5, 'b': 10}])
        self.assertLess(output.index('iNote'), output.index('iWrite'))
        self.run_tool('--icl', 'source.icl', '--pdl', 'result.pdl', '--output', 'replay.pdl')
        self.assertEqual(output, (self.root / 'replay.pdl').read_text())
        flat = self.root / 'debug/MergeNetwork.icl'
        self.run_tool('--icl', flat, '--pdl', 'source.pdl', '--output', 'flat/out.pdl', '--debug')
        self.assertEqual(output, (self.root / 'flat/out.pdl').read_text())
        self.assertEqual(flat.read_bytes(), (self.root / 'flat/debug/MergeNetwork.icl').read_bytes())

    def test_conflicting_and_repeated_calls(self):
        text = self.program('iWrite a 5; iApply', 'iWrite a 10; iApply',
                            'iCall first\niCall second\niCall first')
        groups = self.groups(self.generate(text))
        self.assertEqual([g['a'] for g in groups], [5, 10, 5])
        text = text.replace('iCall first\niCall second\niCall first',
                            'for {set n 0} {$n < 3} {incr n} {iCall first}')
        self.assertEqual(len(self.groups(self.generate(text))), 3)

    def test_recursive_merge_and_call_notes(self):
        text = self.program('iWrite a 5; iApply', 'iWrite b 10; iApply')
        text = text.replace('iMerge -begin\niCall first',
                            'iProc nested {} {iMerge -begin; iCall first; iCall second; iMerge -end}\n'
                            'iMerge -begin\niCall nested\niCall first')
        self.assertGreaterEqual(len(self.groups(self.generate(text))), 2)
        text = self.program('iWrite a 5; iApply; iNote inside', 'iWrite b 10; iApply',
                            'iCall first\niNote outside\niCall second')
        output = self.generate(text)
        self.assertLess(output.index('outside'), output.index('iWrite'))
        self.assertGreater(output.index('inside'), output.index('iApply'))

    def test_barriers_partial_values_and_apply_order(self):
        cases = [('iWrite a 5; iApply -together', 2),
                 ('iWrite a 5; iApply; iRead a 5; iApply', 3),
                 ('iWrite a[0] 1; iApply', 2),
                 ('iWrite a 0bXX01; iApply', 2)]
        for first, count in cases:
            with self.subTest(first=first):
                output = self.generate(self.program(first, 'iWrite b 10; iApply'))
                self.assertEqual(len(self.groups(output)), count)

    def test_multi_apply_calls_preserve_each_sequence(self):
        text = self.program('iWrite a 5; iApply; iWrite a 6; iApply',
                            'iWrite b 10; iApply; iWrite b 11; iApply; iWrite b 12; iApply')
        output = self.generate(text, '--debug')
        self.assertEqual(self.groups(output),
                         [{'a': 5, 'b': 10}, {'a': 6, 'b': 11}, {'a': 6, 'b': 12}])
        self.run_tool('--icl', 'source.icl', '--pdl', 'result.pdl', '--output', 'replay.pdl')
        self.assertEqual(output, (self.root / 'replay.pdl').read_text())
        self.run_tool('--icl', 'debug/MergeNetwork.icl', '--pdl', 'source.pdl',
                       '--output', 'flat/result.pdl', '--debug')
        self.assertEqual(output, (self.root / 'flat/result.pdl').read_text())
        self.assertEqual((self.root / 'debug/MergeNetwork.icl').read_bytes(),
                         (self.root / 'flat/debug/MergeNetwork.icl').read_bytes())

    def test_multi_apply_conflicts_and_empty_applies_are_barriers(self):
        output = self.generate(self.program('iWrite a 5; iApply; iWrite b 6; iApply',
                                           'iWrite b 10; iApply; iWrite a 11; iApply'))
        self.assertEqual(len(self.groups(output)), 4)
        output = self.generate(self.program('iWrite a 5; iApply; iApply',
                                           'iWrite b 10; iApply'))
        self.assertEqual(len(self.groups(output)), 3)

    def test_idle_barrier_on_tap(self):
        fixture = ROOT / 'tests/cases/ours/pattern_generation/access_link'
        (self.root / 'source.icl').write_bytes((fixture / 'simple.icl').read_bytes())
        text = self.program('iWrite core.r 5; iApply; iRunLoop 3 -tck',
                            'iWrite core.r 10; iApply').replace('MergeNetwork', 'AccessTop')
        output = self.generate(text, '--bsdl', 'AccessProbe', fixture / 'simple.bsdl')
        self.assertIn('iRunLoop 3 -tck', output)
        self.assertLess(output.index('iApply'), output.index('iRunLoop'))
        self.assertLess(output.index('iRunLoop'), output.rindex('iApply'))

    def test_invalid_merge_preserves_output(self):
        invalid = [(self.program('iReset; iWrite a 5; iApply', 'iWrite b 10; iApply'),
                    'iReset is forbidden'),
                   (self.program('iWrite a 5', 'iWrite b 10; iApply'), 'unapplied'),
                   ('iProcsForModule MergeNetwork\niWrite a 1\niMerge -begin\niMerge -end',
                    'unapplied'),
                   ('iMerge -begin\niMerge -begin\niMerge -end\niMerge -end',
                    'nested iMerge')]
        for text, message in invalid:
            with self.subTest(text=text):
                (self.root / 'result.pdl').write_text('previous output\n')
                error = self.generate(text, ok=False)
                self.assertIn(message, error)
                self.assertEqual((self.root / 'result.pdl').read_text(), 'previous output\n')

    def test_local_reset_hoisted_before_merge(self):
        (self.root / 'source.icl').write_bytes((FIXTURE / 'local_reset.icl').read_bytes())
        text = self.program('iWrite a.value 5; iApply', 'iWrite b.value 10; iApply')
        self.generate(text, '--local_reset', 'reset.pdl')
        reset = (self.root / 'reset.pdl').read_text()
        self.assertIn('iWrite reset 0b0\niApply\niMerge -begin', reset)
        self.assertIn('iProc first {} {iWrite a.value 5; iApply}', reset)
        self.run_tool('--icl', 'source.icl', '--pdl', 'reset.pdl', '--output', 'reset-result.pdl')

    def test_combined_failure_uses_fresh_serial_state(self):
        source = (FIXTURE / 'local_reset.icl').read_text().replace("ResetValue 1'b1", "ResetValue 1'b0")
        (self.root / 'source.icl').write_text(source)
        cases = [('iWrite a.value 5; iApply', 'iWrite reset 1; iApply', 2),
                 ('iWrite a.value 5; iApply; iWrite a.value 6; iApply',
                  'iWrite reset 0; iApply; iWrite reset 1; iApply', 4)]
        for first, second, count in cases:
            with self.subTest(applies=count):
                text = self.program(first, second)
                output = self.generate(text)
                serial = text.replace('iMerge -begin\n', '').replace('iMerge -end\n', '')
                self.assertEqual(output, self.generate(serial))
                self.assertEqual(len(self.groups(output)), count)

    def test_many_calls(self):
        count = 256
        source = 'Module MergeNetwork { ScanInPort si;\n'
        text = 'iPDLLevel 1\niProcsForModule MergeNetwork\niReset\n'
        for i in range(count):
            source += f"ScanRegister r{i} {{ScanInSource {'si' if not i else 'r' + str(i-1)}; ResetValue 1'b0;}}\n"
            text += f'iProc p{i} {{}} {{iWrite r{i} 1; iApply}}\n'
        source += f'ScanOutPort so {{Source r{count-1};}}\n}}\n'
        text += 'iMerge -begin\n' + ''.join(f'iCall p{i}\n' for i in range(count)) + 'iMerge -end\n'
        (self.root / 'source.icl').write_text(source)
        start = time.monotonic()
        output = self.generate(text)
        self.assertEqual(self.groups(output), [{f'r{i}': 1 for i in range(count)}])
        print(f'merge {count} calls: {time.monotonic()-start:.3f}s; source bytes={len(source)+len(text)}')


if __name__ == '__main__':
    unittest.main()
