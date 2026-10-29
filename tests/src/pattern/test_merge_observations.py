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

    def test_partial_reads_remain_conservative_barriers(self):
        output = self.generate(self.program('iRead a[0] 0; iApply',
                                            'iWrite b 10; iApply'), '--debug')
        self.assertEqual(len(self.groups(output)), 2)
        self.replay(output)

    def test_notes_and_resource_ownership_remain_barriers(self):
        for first in ('iRead a 0; iApply; iNote -comment boundary',
                      'iTake a; iRead a 0; iApply; iRelease a'):
            with self.subTest(first=first):
                output = self.generate(self.program(first, 'iWrite b 10; iApply'),
                                       '--debug')
                self.assertEqual(len(self.groups(output)), 2)
                self.replay(output)

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

    def test_many_read_write_calls(self):
        count = 64
        source = 'Module MergeNetwork { ScanInPort si;\n'
        text = 'iPDLLevel 1\niProcsForModule MergeNetwork\niReset\n'
        for i in range(count):
            previous = f'r{i - 1}' if i else 'si'
            source += f"ScanRegister r{i} {{ ScanInSource {previous}; ResetValue 1'b0; }}\n"
            text += f'iProc p{i} {{}} {{ iRead r{i} 0; iWrite r{i} 1; iApply; iRead r{i} 1; iApply }}\n'
        source += f'ScanOutPort so {{ Source r{count - 1}; }}\n}}\n'
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
