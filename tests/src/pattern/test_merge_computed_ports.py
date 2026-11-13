#!/usr/bin/env python3
"""Physical ownership of connected and computed iMerge targets."""
import time
import unittest

import test_merge_observations as helpers

SOURCE = """Module MergeNetwork {
 DataInPort a[3:0]; DataInPort b[3:0];
 DataOutPort oa[3:0] { Source a; }
 DataOutPort ob[3:0] { Source b; }
}
"""


class MergeComputedPorts(unittest.TestCase):
    run_tool = helpers.MergeObservations.run_tool
    generate = helpers.MergeObservations.generate
    program = helpers.MergeObservations.program
    groups = helpers.MergeObservations.groups
    replay = helpers.MergeObservations.replay

    def setUp(self):
        helpers.MergeObservations.setUp(self)
        (self.root / 'source.icl').write_text(SOURCE)

    def test_independent_outputs_share_apply(self):
        output = self.generate(self.program('iWrite oa 5; iApply',
                                            'iWrite ob 10; iApply'), '--debug')
        self.assertEqual(self.groups(output), [{'a': 5, 'b': 10}])
        self.replay(output)

    def test_connected_inputs_and_inverted_output(self):
        source = 'Module Child { DataInPort input[3:0]; }\n' + SOURCE.replace(
            'Module MergeNetwork {', '''Module MergeNetwork {
 Instance child Of Child { InputPort input = a; }
 DataOutPort inverted[3:0] { Source ~b; }
''')
        (self.root / 'source.icl').write_text(source)
        output = self.generate(self.program('iWrite child.input 5; iApply',
                                            'iWrite inverted 10; iApply'), '--debug')
        self.assertEqual(self.groups(output), [{'a': 5, 'b': 5}])
        self.replay(output)

    def test_alias_masks_and_logical_controlling_history(self):
        (self.root / 'source.icl').write_text(SOURCE.replace('Module MergeNetwork {',
            'Module MergeNetwork { Alias low[1:0] = oa[1:0];'))
        output = self.generate(self.program(
            'iWrite oa 5; iApply; iWrite low 2; iApply',
            'iWrite ob 10; iApply; iWrite ob 0bxx11; iApply'), '--debug')
        self.assertEqual(self.groups(output), [{'a': 5, 'b': 10}, {'a': 6, 'b': 11}])
        self.replay(output)

    def test_shared_driver_keeps_whole_object_order(self):
        (self.root / 'source.icl').write_text(SOURCE.replace('Source b;', 'Source a;'))
        for first, second in (('iWrite oa 5; iApply', 'iWrite ob 10; iApply'),
                              ('iWrite oa[0] 1; iApply', 'iWrite a[3] 1; iApply')):
            with self.subTest(first=first):
                output = self.generate(self.program(first, second), '--debug')
                self.assertEqual(len(self.groups(output)), 2)
                self.replay(output)

    def test_independent_output_reads_share_apply(self):
        source = self.program('iRead oa 5; iApply', 'iRead ob 10; iApply')
        source = source.replace('iReset\n', 'iReset\niState a 5\niState b 10\n')
        output = self.generate(source, '--debug')
        self.assertEqual(len(self.groups(output)), 1)
        self.assertIn('iRead oa 0b0101', output)
        self.assertIn('iRead ob 0b1010', output)
        self.replay(output)

    def test_mux_logic_and_enable_dependencies(self):
        (self.root / 'source.icl').write_text(SOURCE.replace('Source a;', 'Source mux; Enable enable;').replace(
            'Module MergeNetwork {', '''Module MergeNetwork {
 DataInPort select; DataInPort enable;
 LogicSignal choice { select ^ a[0]; }
 DataMux mux[3:0] SelectedBy choice { 1'b0: a; 1'b1: ~a; }
'''))
        output = self.generate(self.program('iWrite oa 5; iApply',
                                            'iWrite ob 10; iApply'), '--debug')
        self.assertEqual(len(self.groups(output)), 1)
        self.assertEqual(self.groups(output)[0]['enable'], 1)
        self.assertEqual(self.groups(output)[0]['b'], 10)
        self.replay(output)
        for second in ('iWrite a 10; iApply', 'iWrite enable 0; iApply',
                       'iWrite select 1; iApply'):
            with self.subTest(second=second):
                output = self.generate(self.program('iWrite oa 5; iApply', second), '--debug')
                self.assertEqual(len(self.groups(output)), 2)
                self.replay(output)

    def test_one_hot_contributor_ownership(self):
        source = '''Module Driver {
 DataInPort data[3:0]; DataInPort enable;
 DataOutPort value[3:0] { Source data; Enable enable; }
}
''' + SOURCE.replace('Source a;', 'Source group;').replace(
            'Module MergeNetwork {', '''Module MergeNetwork {
 DataInPort enable;
 Instance driver Of Driver { InputPort data = a; InputPort enable = enable; }
 OneHotDataGroup group[3:0] { Port driver.value; }
''')
        (self.root / 'source.icl').write_text(source)
        output = self.generate(self.program('iWrite oa 5; iApply',
                                            'iWrite ob 10; iApply'), '--debug')
        self.assertEqual(self.groups(output), [{'a': 5, 'b': 10, 'enable': 1}])
        self.replay(output)
        output = self.generate(self.program('iWrite oa 5; iApply',
                                            'iWrite a 10; iApply'), '--debug')
        self.assertEqual(len(self.groups(output)), 2)
        self.replay(output)

    def test_addressed_one_hot_selection_keeps_ownership(self):
        (self.root / 'source.icl').write_text('''Module MergeNetwork {
 AddressPort address[1:0]; DataInPort data[3:0];
 WriteEnPort we; ReadEnPort re;
 OneHotDataGroup group[3:0] {
  DataRegister a[3:0] { AddressValue 1; ResetValue 4'b0; }
  DataRegister b[3:0] { AddressValue 2; ResetValue 4'b0; }
 }
 DataOutPort observed[3:0] { Source group; }
}
''')
        for target in ('address', 're'):
            with self.subTest(target=target):
                output = self.generate(self.program('iWrite observed 5; iApply',
                    f'iWrite {target} 1; iApply'), '--debug')
                self.assertEqual(len(self.groups(output)), 2)
                self.replay(output)

    def test_constant_output_and_invalid_constraint(self):
        (self.root / 'source.icl').write_text(SOURCE.replace('Source a;', "Source 4'b1010;"))
        output = self.generate(self.program('iWrite oa 10; iApply',
                                            'iWrite ob 5; iApply'), '--debug')
        self.assertIn('iWrite b 0b0101', output)
        self.replay(output)
        (self.root / 'result.pdl').write_text('previous output\n')
        self.generate(self.program('iWrite oa 9; iApply', 'iWrite ob 5; iApply'), ok=False)
        self.assertEqual((self.root / 'result.pdl').read_text(), 'previous output\n')

    def test_scan_and_primary_computed_targets(self):
        (self.root / 'source.icl').write_text(SOURCE.replace('DataInPort a[3:0];', '''
 ScanInPort si; ScanRegister a[3:0] { ScanInSource si; ResetValue 4'b0; }
 ScanOutPort so { Source a[0]; }
'''))
        output = self.generate(self.program('iWrite oa 5; iApply',
                                            'iWrite ob 10; iApply'), '--debug')
        self.assertIn('iWrite a 0b0101', output)
        self.assertIn('iWrite b 0b1010', output)
        self.replay(output)

    def test_resources_and_failed_output_preservation(self):
        output = self.generate(self.program('iTake oa; iWrite oa 5; iApply; iRelease oa',
                                            'iWrite ob 10; iApply'), '--debug')
        self.assertEqual(len(self.groups(output)), 2)
        self.replay(output)
        for second in ('iWrite ob 99; iApply', 'iRead b 0; iApply'):
            with self.subTest(second=second):
                (self.root / 'result.pdl').write_text('previous output\n')
                self.generate(self.program('iWrite oa 5; iApply', second), ok=False)
                self.assertEqual((self.root / 'result.pdl').read_text(), 'previous output\n')

    def test_many_independent_computed_targets(self):
        count = 64
        (self.root / 'source.icl').write_text('Module MergeNetwork {\n' + ''.join(
            f'DataInPort p{i}[3:0]; DataOutPort o{i}[3:0] {{ Source ~p{i}; }}\n'
            for i in range(count)) + '}\n')
        source = 'iPDLLevel 1\niProcsForModule MergeNetwork\niReset\n'
        source += ''.join(f'iProc put{i} {{}} {{iWrite o{i} 5; iApply; iWrite o{i}[0] 0; iApply}}\n'
                          for i in range(count))
        source += 'iMerge -begin\n' + ''.join(f'iCall put{i}\n' for i in range(count)) + 'iMerge -end\n'
        start = time.monotonic()
        output = self.generate(source, '--debug')
        self.assertEqual(len(self.groups(output)), 2)
        self.replay(output)
        print(f'computed merge: {count} two-apply calls, {len(source)} PDL bytes, three replays {time.monotonic() - start:.3f}s')


if __name__ == '__main__':
    unittest.main()
