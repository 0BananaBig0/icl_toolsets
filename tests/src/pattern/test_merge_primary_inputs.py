#!/usr/bin/env python3
"""Merge ownership for unconnected primary input targets."""
import time
import unittest

import test_merge_observations as helpers

SOURCE = """Module MergeNetwork {
 DataInPort a[3:0]; DataInPort b[3:0];
 AddressPort address[1:0]; ReadEnPort re; WriteEnPort we;
}
"""


class MergePrimaryInputs(unittest.TestCase):
    run_tool = helpers.MergeObservations.run_tool
    generate = helpers.MergeObservations.generate
    program = helpers.MergeObservations.program
    groups = helpers.MergeObservations.groups
    replay = helpers.MergeObservations.replay

    def setUp(self):
        helpers.MergeObservations.setUp(self)
        (self.root / 'source.icl').write_text(SOURCE)

    def test_direct_inputs_share_apply(self):
        output = self.generate(self.program('iWrite a 5; iApply', 'iWrite b 10; iApply'), '--debug')
        self.assertEqual(self.groups(output), [{'a': 5, 'b': 10}])
        self.replay(output)

    def test_masks_aliases_and_apply_order(self):
        (self.root / 'source.icl').write_text(SOURCE.replace('Module MergeNetwork {', '''Module MergeNetwork {
 Alias reverse[0:3] = ~a[0], ~a[1], ~a[2], ~a[3];
 Alias low[1:0] = a[1:0];
'''))
        source = self.program('iWrite reverse 0; iApply; iWrite low 2; iApply',
                              'iWrite b 10; iApply; iWrite b 0bxx11; iApply')
        output = self.generate(source, '--debug')
        self.assertEqual(self.groups(output), [{'a': 15, 'b': 10}, {'a': 14, 'b': 11}])
        self.replay(output)

    def test_same_port_slices_keep_fill_order(self):
        source = self.program('iWrite a[0] 1; iApply', 'iWrite a[3] 1; iApply')
        source = source.replace('iReset\n', 'iReset\niState a 0\n')
        output = self.generate(source, '--debug')
        self.assertEqual(self.groups(output), [{'a': 1}, {'a': 9}])
        self.replay(output)

    def test_primary_control_ports(self):
        output = self.generate(self.program('iWrite address 2; iWrite we 1; iApply',
                                            'iWrite re 1; iApply'), '--debug')
        self.assertEqual(self.groups(output), [{'address': 2, 're': 1, 'we': 1}])
        self.replay(output)

    def test_primary_and_register_streams(self):
        source = SOURCE.replace('Module MergeNetwork {', '''Module MergeNetwork {
 ScanInPort si; ScanRegister r[3:0] { ScanInSource si; ResetValue 4'b0; }
 ScanOutPort so { Source r[0]; }
''')
        (self.root / 'source.icl').write_text(source)
        output = self.generate(self.program('iWrite a 5; iApply; iWrite a 6; iApply',
                                            'iWrite r 10; iApply; iWrite r 11; iApply'), '--debug')
        self.assertIn('iWrite a 0b0101', output)
        self.assertIn('iWrite a 0b0110', output)
        self.assertIn('iWrite r 0b1010', output)
        self.assertIn('iWrite r 0b1011', output)
        self.replay(output)

    def test_connected_inputs_and_outputs_sharing_a_driver_keep_order(self):
        source = 'Module Child { DataInPort input[3:0]; }\n' + SOURCE.replace('Module MergeNetwork {', '''Module MergeNetwork {
 Instance child Of Child { InputPort input = a; }
 DataOutPort observed[3:0] { Source a; }
''')
        (self.root / 'source.icl').write_text(source)
        for target in ('child.input', 'observed'):
            with self.subTest(target=target):
                output = self.generate(self.program(f'iWrite {target} 5; iApply',
                                                    'iWrite a 10; iApply'), '--debug')
                self.assertEqual([g['a'] for g in self.groups(output)], [5, 10])
                self.replay(output)

    def test_resource_barriers_and_invalid_operands(self):
        output = self.generate(self.program('iTake a; iWrite a 5; iApply; iRelease a',
                                            'iWrite b 10; iApply'), '--debug')
        self.assertEqual(len(self.groups(output)), 2)
        self.replay(output)
        for second in ('iWrite b 99; iApply', 'iRead b 0; iApply'):
            with self.subTest(second=second):
                (self.root / 'result.pdl').write_text('previous output\n')
                self.generate(self.program('iWrite a 5; iApply', second), ok=False)
                self.assertEqual((self.root / 'result.pdl').read_text(), 'previous output\n')

    def test_many_primary_inputs(self):
        count = 64
        (self.root / 'source.icl').write_text('Module MergeNetwork {\n' + ''.join(
            f' DataInPort p{i}[3:0];\n' for i in range(count)) + '}\n')
        source = 'iPDLLevel 1\niProcsForModule MergeNetwork\niReset\n'
        source += ''.join(f'iProc put{i} {{}} {{iWrite p{i} 5; iApply; iWrite p{i}[0] 0; iApply}}\n'
                          for i in range(count))
        source += 'iMerge -begin\n' + ''.join(f'iCall put{i}\n' for i in range(count)) + 'iMerge -end\n'
        start = time.monotonic()
        output = self.generate(source, '--debug')
        self.assertEqual(len(self.groups(output)), 2)
        self.replay(output)
        print(f'primary merge: {count} two-apply calls, {len(source)} PDL bytes, three replays {time.monotonic() - start:.3f}s')


if __name__ == '__main__':
    unittest.main()
