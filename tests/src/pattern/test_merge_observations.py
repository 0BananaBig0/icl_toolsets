#!/usr/bin/env python3
"""Self-constructed IEEE 1687 7.9.18 read/write merge ordering cases."""
import time
import unittest

import test_merge_scheduling as helpers


class MergeObservations(unittest.TestCase):
    setUp = helpers.MergeScheduling.setUp
    run_tool = helpers.MergeScheduling.run_tool
    generate = helpers.MergeScheduling.generate
    program = helpers.MergeScheduling.program
    groups = helpers.MergeScheduling.groups

    def replay(self, output):
        self.run_tool('--icl', 'source.icl', '--pdl', 'result.pdl',
                      '--output', 'replay.pdl')
        self.assertEqual(output, (self.root / 'replay.pdl').read_text())
        self.run_tool('--icl', 'debug/MergeNetwork.icl', '--pdl', 'source.pdl',
                      '--output', 'flat/result.pdl', '--debug')
        self.assertEqual(output, (self.root / 'flat/result.pdl').read_text())
        self.assertEqual((self.root / 'debug/MergeNetwork.icl').read_bytes(),
                         (self.root / 'flat/debug/MergeNetwork.icl').read_bytes())

    def test_observations_and_updates_merge_by_apply_depth(self):
        output = self.generate(self.program(
            'iRead a 0; iWrite a 5; iApply; iRead a 5; iWrite a 6; iApply',
            'iRead b 0; iWrite b 10; iApply; iRead b 10; iWrite b 11; iApply'),
            '--debug')
        self.assertEqual(self.groups(output), [{'a': 5, 'b': 10}, {'a': 6, 'b': 11}])
        frames = output.split('iApply\n')
        self.assertIn('iRead a 0b0000', frames[0])
        self.assertIn('iRead b 0b0000', frames[0])
        self.assertIn('iRead a 0b0101', frames[1])
        self.assertIn('iRead b 0b1010', frames[1])
        self.replay(output)

    def test_independent_read_only_calls_share_observation(self):
        output = self.generate(self.program('iRead a 0; iApply',
                                            'iRead b 0; iApply'), '--debug')
        self.assertEqual(len(self.groups(output)), 1)
        self.assertIn('iRead a 0b0000', output)
        self.assertIn('iRead b 0b0000', output)
        self.replay(output)

    def test_observations_without_expected_data_still_merge(self):
        output = self.generate(self.program('iRead a; iApply',
                                            'iRead b; iApply'), '--debug')
        self.assertEqual(len(self.groups(output)), 1)
        self.assertIn('iRead a 0bxxxx', output)
        self.assertIn('iRead b 0bxxxx', output)
        self.replay(output)

    def test_read_write_conflicts_preserve_call_order(self):
        output = self.generate(self.program('iWrite a 5; iApply',
                                            'iRead a 5; iApply'), '--debug')
        self.assertEqual(len(self.groups(output)), 2)
        self.assertGreater(output.index('iRead a'), output.index('iApply'))
        self.replay(output)

    def test_masked_observations_are_not_controlling_assignments(self):
        output = self.generate(self.program('iRead a 0bxx00; iApply',
                                            'iWrite b 10; iApply'), '--debug')
        self.assertEqual(len(self.groups(output)), 1)
        self.assertIn('iRead a 0bxx00', output)
        self.assertIn('iWrite b 0b1010', output)
        self.replay(output)

    def test_partial_reads_merge_with_independent_writes(self):
        output = self.generate(self.program('iRead a[0] 0; iApply',
                                            'iWrite b 10; iApply'), '--debug')
        self.assertEqual(len(self.groups(output)), 1)
        self.assertIn('iRead a 0bxxx0', output)
        self.replay(output)

    def test_aliased_observations_use_physical_registers(self):
        source = (helpers.FIXTURE / 'network.icl').read_text()
        source = source.replace('Module MergeNetwork {', '''Module MergeNetwork {
  Alias reversed[0:3] = ~a[0], ~a[1], ~a[2], ~a[3];
  Alias nested[1:0] = reversed[0:1];
''')
        (self.root / 'source.icl').write_text(source)
        for first in ('iRead nested 3; iApply',
                      'iRead reversed; iApply',
                      'iRead reversed[3:0] 15; iApply'):
            with self.subTest(first=first):
                output = self.generate(self.program(first, 'iWrite b 10; iApply'), '--debug')
                self.assertEqual(len(self.groups(output)), 1)
                self.assertIn('iRead a ', output)
                self.replay(output)

    def test_alias_conflicts_and_access_together_keep_order(self):
        source = (helpers.FIXTURE / 'network.icl').read_text()
        source = source.replace('Module MergeNetwork {', '''Module MergeNetwork {
  Alias both[1:0] = a[0], b[0];
  Alias grouped[1:0] = a[0], b[0] { AccessTogether; }
''')
        (self.root / 'source.icl').write_text(source)
        for first, second in (
                ('iRead both 0; iApply', 'iWrite b 10; iApply'),
                ('iRead a[0] 0; iApply', 'iRead a[3] 0; iApply'),
                ('iRead grouped[0] 0; iApply', 'iWrite a 5; iApply')):
            with self.subTest(first=first, second=second):
                output = self.generate(self.program(first, second), '--debug')
                self.assertEqual(len(self.groups(output)), 2)
                self.replay(output)

    def test_partial_read_streams_keep_each_apply_order(self):
        output = self.generate(self.program(
            'iRead a[0] 0; iApply; iRead a[3]; iApply',
            'iWrite b 10; iApply; iWrite b 11; iApply'), '--debug')
        self.assertEqual(self.groups(output), [{'a': 0, 'b': 10}, {'a': 0, 'b': 11}])
        self.assertIn('iRead a 0bxxx0', output.split('iApply\n')[0])
        self.assertIn('iRead a 0bxxxx', output.split('iApply\n')[1])
        self.replay(output)

    def test_notes_and_resource_ownership_remain_barriers(self):
        for first in ('iRead a 0; iApply; iNote -comment boundary',
                      'iTake a; iRead a 0; iApply; iRelease a'):
            with self.subTest(first=first):
                output = self.generate(self.program(first, 'iWrite b 10; iApply'),
                                       '--debug')
                self.assertEqual(len(self.groups(output)), 2)
                self.replay(output)

    def test_partial_and_masked_writes_merge_with_independent_streams(self):
        for first in ('iWrite a[0] 1; iApply', 'iWrite a 0bxx01; iApply',
                      'iWrite a[3:2] 0bxx; iWrite a[0] 1; iApply'):
            with self.subTest(first=first):
                output = self.generate(self.program(first, 'iWrite b 10; iApply'), '--debug')
                self.assertEqual(self.groups(output), [{'a': 1, 'b': 10}])
                self.replay(output)

    def test_masked_streams_retain_fill_and_last_write_order(self):
        output = self.generate(self.program(
            'iWrite a 5; iApply; iWrite a[0] 1; iWrite a 0bxx10; iApply; iWrite a 0bxxxx; iApply',
            'iRead b 0; iWrite b[3:2] 2; iApply; iRead b 8; iWrite b[1:0] 3; iApply'), '--debug')
        self.assertEqual(self.groups(output), [{'a': 5, 'b': 8}, {'a': 6, 'b': 11}, {'a': 6, 'b': 11}])
        self.assertIn('iRead b 0b0000', output.split('iApply\n')[0])
        self.assertIn('iRead b 0b1000', output.split('iApply\n')[1])
        self.replay(output)

    def test_nested_inverted_and_ascending_alias_writes(self):
        source = (helpers.FIXTURE / 'network.icl').read_text().replace('Module MergeNetwork {', '''
Module MergeNetwork {
  Alias reverse[0:3] = ~a[0], ~a[1], ~a[2], ~a[3];
  Alias low[1:0] = a[1:0];
  Alias nested[1:0] = low;
''')
        (self.root / 'source.icl').write_text(source)
        for first, value in (('iWrite reverse 1; iApply', 7),
                             ('iWrite nested 2; iApply', 2),
                             ('iWrite reverse[0:1] 0bxx; iWrite low 3; iApply', 3)):
            with self.subTest(first=first):
                output = self.generate(self.program(first, 'iWrite b[3:2] 2; iApply'), '--debug')
                self.assertEqual(self.groups(output), [{'a': value, 'b': 8}])
                self.replay(output)

    def test_write_alias_conflicts_keep_whole_register_order(self):
        source = (helpers.FIXTURE / 'network.icl').read_text().replace('Module MergeNetwork {', '''
Module MergeNetwork {
  Alias both[1:0] = a[0], b[0];
  Alias grouped[1:0] = a[0], b[0] { AccessTogether; }
''')
        (self.root / 'source.icl').write_text(source)
        for first, second in (('iWrite a[0] 1; iApply', 'iWrite a[3] 1; iApply'),
                              ('iWrite both 3; iApply', 'iWrite b[3] 1; iApply'),
                              ('iWrite grouped[0] 1; iApply', 'iWrite b[3] 1; iApply')):
            with self.subTest(first=first):
                output = self.generate(self.program(first, second), '--debug')
                self.assertEqual(len(self.groups(output)), 2)
                self.replay(output)

    def test_masked_combined_failure_retries_fresh_serial_state(self):
        source = (helpers.FIXTURE / 'local_reset.icl').read_text().replace(
            "ResetValue 1'b1", "ResetValue 1'b0")
        (self.root / 'source.icl').write_text(source)
        text = self.program('iWrite a.value[1:0] 3; iApply', 'iWrite reset 1; iApply')
        output = self.generate(text, '--debug')
        self.assertEqual(len(self.groups(output)), 2)
        self.replay(output)
        self.assertEqual(output, self.generate(text.replace('iMerge -begin\n', '').replace('iMerge -end\n', '')))

    def test_invalid_masked_merge_preserves_previous_output(self):
        source = (helpers.FIXTURE / 'network.icl').read_text().replace(
            'Module MergeNetwork {', 'Module MergeNetwork { Alias low[1:0] = a[1:0];')
        (self.root / 'source.icl').write_text(source)
        (self.root / 'result.pdl').write_bytes(b'previous output\n')
        self.generate(self.program('iWrite low 0b102; iApply', 'iWrite b 10; iApply'), ok=False)
        self.assertEqual((self.root / 'result.pdl').read_bytes(), b'previous output\n')

    def test_many_masked_write_calls(self):
        count = 64
        source = 'Module MergeNetwork { ScanInPort si;\n'
        text = 'iPDLLevel 1\niProcsForModule MergeNetwork\niReset\n'
        for i in range(count):
            previous = f'r{i - 1}[0]' if i else 'si'
            source += f"ScanRegister r{i}[3:0] {{ ScanInSource {previous}; ResetValue 4'b0; }}\n"
            text += f'iProc p{i} {{}} {{ iWrite r{i}[1:0] {i % 4}; iApply; iWrite r{i} 0bxx11; iApply }}\n'
        source += f'ScanOutPort so {{ Source r{count - 1}[0]; }}\n}}\n'
        text += 'iMerge -begin\n' + ''.join(f'iCall p{i}\n' for i in range(count)) + 'iMerge -end\n'
        (self.root / 'source.icl').write_text(source)
        start = time.monotonic()
        output = self.generate(text, '--debug')
        self.assertEqual(len(self.groups(output)), 2)
        self.replay(output)
        print(f'masked merge: {count} calls, two applies, {len(source) + len(text)} bytes, '
              f'three replays {time.monotonic() - start:.3f}s')

    def test_unequal_read_write_streams_preserve_order(self):
        output = self.generate(self.program('iRead a 0; iWrite a 5; iApply',
            'iRead b 0; iWrite b 10; iApply; iRead b 10; iApply'), '--debug')
        self.assertEqual(len(self.groups(output)), 2)
        frames = output.split('iApply\n')
        self.assertIn('iRead b 0b0000', frames[0])
        self.assertIn('iRead b 0b1010', frames[1])
        self.replay(output)

    def test_failed_combined_read_write_plan_retries_from_fresh_state(self):
        source = (helpers.FIXTURE / 'local_reset.icl').read_text().replace(
            "ResetValue 1'b1", "ResetValue 1'b0")
        (self.root / 'source.icl').write_text(source)
        text = self.program('iRead a.value 0; iWrite a.value 5; iApply',
                            'iWrite reset 1; iApply')
        output = self.generate(text, '--debug')
        self.assertEqual(len(self.groups(output)), 2)
        self.assertIn('iRead a.value 0b0000', output)
        self.replay(output)
        serial = text.replace('iMerge -begin\n', '').replace('iMerge -end\n', '')
        self.assertEqual(output, self.generate(serial))

    def test_many_sliced_read_write_calls(self):
        count = 64
        source = 'Module MergeNetwork { ScanInPort si;\n'
        text = 'iPDLLevel 1\niProcsForModule MergeNetwork\niReset\n'
        for i in range(count):
            previous = f'r{i - 1}' if i else 'si'
            previous = f'{previous}[0]' if i else previous
            source += f"ScanRegister r{i}[1:0] {{ ScanInSource {previous}; ResetValue 2'b0; }}\n"
            text += f'iProc p{i} {{}} {{ iRead r{i}[0] 0; iWrite r{i} 1; iApply; iRead r{i}[0] 1; iApply }}\n'
        source += f'ScanOutPort so {{ Source r{count - 1}[0]; }}\n}}\n'
        text += 'iMerge -begin\n' + ''.join(f'iCall p{i}\n' for i in range(count)) + 'iMerge -end\n'
        (self.root / 'source.icl').write_text(source)
        start = time.monotonic()
        output = self.generate(text, '--debug')
        self.assertEqual(len(self.groups(output)), 2)
        self.assertEqual(output.count('iRead '), count * 2)
        self.replay(output)
        print(f'merge observations {count} calls, 2 applies each, 3 replays: '
              f'{time.monotonic() - start:.3f}s; source bytes={len(source) + len(text)}')


if __name__ == '__main__':
    unittest.main()
