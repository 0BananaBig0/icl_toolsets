#!/usr/bin/env python3
"""Independent DataRegister merge streams through the shared scheduler."""
import time
import unittest
from pathlib import Path

import test_merge_observations as helpers


def primary(count=2):
    names = [chr(ord('a') + i) for i in range(count)] if count == 2 else [f'r{i}' for i in range(count)]
    return 'Module MergeNetwork {\n' + ''.join(
        f" DataInPort d{name}[3:0]; DataInPort e{name};\n"
        f" DataRegister {name}[3:0] {{ WriteDataSource d{name}; WriteEnSource e{name}; ResetValue 4'b0; }}\n"
        f" DataOutPort o{name}[3:0] {{ Source {name}; }}\n" for name in names) + '}\n'


SERIAL = """Module MergeNetwork {
 ScanInPort si;
 ScanRegister ba[4:0] { ScanInSource si; CaptureSource 1'b0, a; ResetValue 5'b0; }
 ScanRegister bb[4:0] { ScanInSource ba; CaptureSource 1'b0, b; ResetValue 5'b0; }
 ScanOutPort so { Source bb[0]; }
 DataRegister a[3:0] { WriteDataSource ba[3:0]; WriteEnSource ba[4]; ResetValue 4'b0; }
 DataRegister b[3:0] { WriteDataSource bb[3:0]; WriteEnSource bb[4]; ResetValue 4'b0; }
}
"""

RESET_DEPENDENCY = """Module Child {
 ResetPort rst; DataInPort data[3:0];
 DataRegister value[3:0] { WriteDataSource data; ResetValue 4'b0; }
 DataOutPort observed[3:0] { Source value; }
}
Module MergeNetwork {
 ScanInPort si; DataInPort data[3:0]; ResetPort global;
 ScanRegister root { ScanInSource si; ResetValue 1'b0; }
 DataMux reset_gate SelectedBy root { 1'b0: 1'b1; 1'b1: global; }
 ScanOutPort so { Source root; }
 Instance child Of Child { InputPort rst = reset_gate; InputPort data = data; }
 DataOutPort observed[3:0] { Source child.observed; }
}
"""


class MergeDataRegisters(unittest.TestCase):
    run_tool = helpers.MergeObservations.run_tool
    generate = helpers.MergeObservations.generate
    program = helpers.MergeObservations.program
    groups = helpers.MergeObservations.groups
    replay = helpers.MergeObservations.replay

    def setUp(self):
        helpers.MergeObservations.setUp(self)
        (self.root / 'source.icl').write_text(primary())

    def test_independent_primary_writes(self):
        output = self.generate(self.program('iWrite a 5; iApply', 'iWrite b 10; iApply'), '--debug')
        self.assertEqual(len(self.groups(output)), 1)
        self.assertIn('iWrite da 0b0101', output)
        self.assertIn('iWrite db 0b1010', output)
        self.replay(output)

    def test_primary_read_write_apply_order(self):
        output = self.generate(self.program(
            'iWrite a 5; iApply; iRead a 5; iWrite a[1:0] 2; iApply',
            'iWrite b 10; iApply; iRead b 10; iWrite b 11; iApply'), '--debug')
        groups = self.groups(output)
        # Primary observations precede the pin changes that perform the writes.
        self.assertEqual(len(groups), 3)
        self.assertEqual(groups[1], {})
        self.assertEqual((groups[0]['da'], groups[2]['da']), (5, 6))
        self.assertEqual((groups[0]['db'], groups[2]['db']), (10, 11))
        self.assertIn('iRead oa 0b0101', output.split('iApply\n')[1])
        self.assertIn('iRead ob 0b1010', output.split('iApply\n')[1])
        self.replay(output)

    def test_serial_registers_share_scans(self):
        (self.root / 'source.icl').write_text(SERIAL)
        output = self.generate(self.program(
            'iWrite a 5; iApply; iRead a 5; iWrite a 6; iApply',
            'iWrite b 10; iApply; iRead b 10; iWrite b 11; iApply'), '--debug')
        self.assertEqual(len(self.groups(output)), 2)
        self.assertIn('iRead ba 0bx0101', output)
        self.assertIn('iRead bb 0bx1010', output)
        self.replay(output)

    def test_partial_masked_and_inverted_aliases(self):
        source = primary().replace('Module MergeNetwork {', '''Module MergeNetwork {
 Alias inverted[0:3] = ~a[0], ~a[1], ~a[2], ~a[3];
 Alias nested[1:0] = inverted[0:1];
''')
        (self.root / 'source.icl').write_text(source)
        for first in ('iWrite a[1:0] 1; iApply', 'iWrite a 0bxx01; iApply',
                      'iWrite nested 1; iApply'):
            with self.subTest(first=first):
                output = self.generate(self.program(first, 'iWrite b 10; iApply'), '--debug')
                self.assertEqual(len(self.groups(output)), 1)
                self.replay(output)

    def test_mixed_parallel_and_serial_data(self):
        source = SERIAL.replace(
            "ScanRegister ba[4:0] { ScanInSource si; CaptureSource 1'b0, a; ResetValue 5'b0; }",
            'DataInPort da[3:0]; DataInPort ea; DataOutPort oa[3:0] { Source a; }')
        source = source.replace('ScanInSource ba;', 'ScanInSource si;').replace(
            'WriteDataSource ba[3:0]; WriteEnSource ba[4];', 'WriteDataSource da; WriteEnSource ea;')
        (self.root / 'source.icl').write_text(source)
        output = self.generate(self.program(
            'iWrite a 5; iApply; iRead a 5; iApply',
            'iWrite b 10; iApply; iRead b 10; iApply'), '--debug')
        self.assertIn('iWrite da 0b0101', output)
        self.assertIn('iWrite bb 0b11010', output)
        self.assertIn('iRead oa 0b0101', output)
        self.assertIn('iRead bb 0bx1010', output)
        self.replay(output)

    def test_addressed_data_registers(self):
        fixture = Path(__file__).resolve().parents[2] / 'cases/ours/pattern_generation/data_registers/addressed.icl'
        source = fixture.read_text()
        (self.root / 'source.icl').write_text(source.replace('AddressedData', 'MergeNetwork'))
        output = self.generate(self.program(
            'iWrite a 5; iApply; iRead a 5; iApply',
            'iWrite b 10; iApply; iRead b 10; iApply'), '--debug')
        self.assertIn('iWrite data 0b0101', output)
        self.assertIn('iWrite data 0b1010', output)
        self.assertIn('iRead observed 0b0101', output)
        self.assertIn('iRead observed 0b1010', output)
        self.replay(output)

    def test_same_physical_storage_stays_ordered(self):
        (self.root / 'source.icl').write_text(primary().replace(
            'Module MergeNetwork {', 'Module MergeNetwork { Alias alias[3:0] = a;'))
        output = self.generate(self.program('iWrite alias[0] 1; iApply',
                                            'iWrite a[3] 1; iApply'), '--debug')
        self.assertEqual([g['da'] for g in self.groups(output)], [1, 9])
        self.replay(output)

    def test_resource_and_together_barriers(self):
        for first in ('iTake a; iWrite a 5; iApply; iRelease a',
                      'iWrite a 5; iApply -together',
                      'iWrite a 5; iApply; iNote barrier'):
            with self.subTest(first=first):
                output = self.generate(self.program(first, 'iWrite b 10; iApply'), '--debug')
                self.assertEqual(len(self.groups(output)), 2)
                self.replay(output)

    def test_shared_bus_requires_ordered_pin_frames(self):
        source = primary().replace('DataInPort db[3:0]; ', '').replace('WriteDataSource db;', 'WriteDataSource da;')
        (self.root / 'source.icl').write_text(source)
        output = self.generate(self.program(
            'iWrite a 5; iApply; iRead a 5; iApply',
            'iWrite b 10; iApply; iRead b 10; iApply'), '--debug')
        self.assertIn('iWrite da 0b0101', output)
        self.assertIn('iWrite da 0b1010', output)
        self.assertIn('iRead oa 0b0101', output)
        self.assertIn('iRead ob 0b1010', output)
        self.replay(output)

    def test_invalid_operand_preserves_output(self):
        (self.root / 'result.pdl').write_text('previous output\n')
        self.generate(self.program('iWrite a 5; iApply', 'iWrite b 99; iApply'), ok=False)
        self.assertEqual((self.root / 'result.pdl').read_text(), 'previous output\n')

    def test_scan_reset_conflict_retries_from_fresh_state(self):
        (self.root / 'source.icl').write_text(RESET_DEPENDENCY)
        source = self.program('iWrite root 0; iApply', 'iWrite child.value 5; iApply')
        serial = self.generate(source.replace('iMerge -begin\n', '').replace('iMerge -end\n', ''))
        output = self.generate(source, '--debug')
        self.assertEqual(output, serial)
        self.assertIn('iWrite root 0b1', output)
        self.assertIn('iWrite data 0b0101', output)
        self.replay(output)

    def test_final_state_follows_merged_order_and_reservations(self):
        (self.root / 'source.icl').write_text(RESET_DEPENDENCY)
        first = 'iWrite root 1; iApply; iWrite root 0; iApply'
        for second, final in (('iWrite child.value 5; iApply', 0),
                              ('iTake child.value; iWrite child.value 5; iApply; iRelease child.value', 5)):
            with self.subTest(reserved=bool(final)):
                source = self.program(first, second) + f'iRead child.value {final}; iApply\n'
                output = self.generate(source, '--debug')
                self.assertIn(f'iRead observed 0b{final:04b}', output)
                self.replay(output)

    def test_many_independent_registers(self):
        count = 64
        (self.root / 'source.icl').write_text(primary(count))
        source = 'iPDLLevel 1\niProcsForModule MergeNetwork\niReset\n'
        source += ''.join(f'iProc p{i} {{}} {{iWrite r{i} 5; iApply; iWrite r{i}[0] 0; iApply}}\n'
                          for i in range(count))
        source += 'iMerge -begin\n' + ''.join(f'iCall p{i}\n' for i in range(count)) + 'iMerge -end\n'
        start = time.monotonic()
        output = self.generate(source, '--debug')
        self.assertEqual(len(self.groups(output)), 2)
        self.replay(output)
        print(f'data merge: {count} two-apply calls, {len(source)} PDL bytes, three replays {time.monotonic() - start:.3f}s')


if __name__ == '__main__':
    unittest.main()
