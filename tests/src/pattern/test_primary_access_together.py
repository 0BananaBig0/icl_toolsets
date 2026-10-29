#!/usr/bin/env python3
"""Self-constructed IEEE 1687 6.5.2(i) parallel AccessTogether cases."""
import pathlib
import subprocess
import sys
import tempfile
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[3]
BIN = pathlib.Path(sys.argv.pop(1)).resolve() if len(sys.argv) > 1 else ROOT / 'build/icl-pattern-generation'


class PrimaryAccessTogether(unittest.TestCase):
    def exercise(self, declarations, commands, *, serial=False, tap=False, ok=True):
        with tempfile.TemporaryDirectory() as directory:
            root = pathlib.Path(directory)
            scan = '''ScanInPort si;
  ScanRegister sr[3:0] { ScanInSource si; ResetValue 4'b0; }
  ScanOutPort so { Source sr[0]; }
''' if serial else ''
            suffix = ''
            arguments = []
            if tap:
                scan = '''Instance core Of SerialCore;
AccessLink tap Of STD_1149_1_2001 {
  BSDLEntity AccessProbe;
  IJTAG_ON { ScanInterface { core.client; } }
  IJTAG_OFF { ScanInterface { core.client; } }
}
'''
                suffix = '''Module SerialCore {
  ScanInPort si; ScanOutPort so { Source sr[0]; }
  SelectPort select; CaptureEnPort capture; ShiftEnPort shift;
  UpdateEnPort update; TCKPort clock;
  ScanRegister sr[3:0] { ScanInSource si; ResetValue 4'b0; }
  ScanInterface client {
    Port si; Port so; Port select; Port capture; Port shift; Port update; Port clock;
  }
}
'''
                arguments = ['--bsdl', 'AccessProbe', str(ROOT / 'tests/cases/ours/pattern_generation/access_link/simple.bsdl')]
            model = root / 'model.icl'
            model.write_text('Module Grouped {\n' + scan + declarations + '\n}\n' + suffix)
            source = root / 'source.pdl'
            source.write_text('iProcsForModule Grouped\niReset\n' + commands)

            def run(icl, pdl, name):
                output = root / name / 'result.pdl'
                output.parent.mkdir()
                output.write_text('previous output')
                result = subprocess.run(
                    [str(BIN), '--icl', str(icl), '--pdl', str(pdl),
                     '--output', str(output), '--debug', *arguments],
                    capture_output=True, text=True, timeout=45)
                self.assertEqual(result.returncode == 0, ok, result.stdout + result.stderr)
                if not ok:
                    self.assertEqual(output.read_text(), 'previous output')
                    return result.stdout + result.stderr
                return output.read_text()

            original = run(model, source, 'first')
            if not ok:
                return original
            flat = root / 'first/debug/Grouped.icl'
            self.assertEqual(original, run(flat, source, 'flat'))
            self.assertEqual(flat.read_text(), (root / 'flat/debug/Grouped.icl').read_text())
            self.assertEqual(original, run(model, root / 'first/result.pdl', 'replay'))
            return original

    def assert_group(self, output, writes=(), reads=(), together=False):
        for frame in output.split('iApply'):
            if all(command in frame for command in (*writes, *reads)):
                if together:
                    self.assertIn(frame + 'iApply -together', output)
                return
        self.fail('No common apply group for requested operations:\n' + output)

    def test_primary_pins_share_one_update(self):
        for serial in (False, True):
            with self.subTest(serial=serial):
                output = self.exercise('''DataInPort a[3:0]; DataInPort b[3:0];
Alias pair[7:0] = a, b { AccessTogether; }''',
                    'iWrite pair 0x59; iApply', serial=serial)
                self.assert_group(output, ('iWrite a 0b0101', 'iWrite b 0b1001'), together=True)

    def test_partial_alias_access_still_drives_other_member(self):
        output = self.exercise('''DataInPort a[3:0]; DataInPort b[3:0];
Alias pair[7:0] = a, b { AccessTogether; }''',
            'iWrite b 9; iApply; iWrite pair[7:4] 5; iApply', serial=True)
        self.assert_group(output, ('iWrite a 0b0101', 'iWrite b 0b1001'), together=True)

    def test_tap_scan_before_and_after_parallel_group(self):
        output = self.exercise('''DataInPort a[3:0]; DataInPort b[3:0];
Alias pair[7:0] = a, b { AccessTogether; }''',
            'iWrite core.sr 5; iApply; iWrite pair 0x59; iApply; '
            'iWrite core.sr 9; iApply', tap=True)
        self.assert_group(output, ('iWrite a 0b0101', 'iWrite b 0b1001'), together=True)

    def test_implicit_group_member_respects_end_state(self):
        output = self.exercise('''DataInPort a[3:0]; DataInPort b[3:0];
Alias idle[3:0] = b { iApplyEndState 4'b0; }
Alias pair[7:0] = a, b { AccessTogether; }''',
            'iWrite b 9; iApply; iWrite pair[7:4] 5; iApply', serial=True)
        self.assert_group(output, ('iWrite a 0b0101', 'iWrite b 0b0000'), together=True)

    def test_nested_overlapping_groups_and_inverted_slices(self):
        output = self.exercise('''DataInPort a[3:0]; DataInPort b[3:0]; DataInPort c[1:0];
Alias first[3:0] = ~a[3:2], b[1:0] { AccessTogether; }
Alias second[5:0] = first, c { AccessTogether; }''',
            'iWrite second 0b011011; iApply', serial=True)
        self.assert_group(output, ('iWrite a 0b1000', 'iWrite b 0b0010', 'iWrite c 0b11'), together=True)

    def test_primary_and_data_register_can_update_together(self):
        output = self.exercise('''DataInPort data[3:0]; DataInPort enable;
DataRegister value[3:0] { WriteDataSource data; WriteEnSource enable; ResetValue 4'b0; }
DataOutPort observed[3:0] { Source value; }
Alias pair[4:0] = enable, value { AccessTogether; }''',
            'iWrite pair 0x19; iApply; iRead value 9; iApply', serial=True)
        self.assert_group(output, ('iWrite data 0b1001', 'iWrite enable 0b1'), together=True)

    def test_implicit_enable_member_can_be_activated(self):
        for serial in (False, True):
            with self.subTest(serial=serial):
                output = self.exercise('''DataInPort data[3:0]; DataInPort enable;
DataRegister value[3:0] { WriteDataSource data; WriteEnSource enable; ResetValue 4'b0; }
Alias pair[4:0] = enable, value { AccessTogether; }''',
                    'iWrite enable 0; iApply; iWrite pair[3:0] 9; iApply', serial=serial)
                self.assert_group(output, ('iWrite data 0b1001', 'iWrite enable 0b1'), together=True)

    def test_scan_and_primary_group_rejects_separate_updates(self):
        error = self.exercise('''DataInPort pin;
Alias pair[4:0] = sr, pin { AccessTogether; }''',
            'iWrite pair 0x13; iApply', serial=True, ok=False)
        self.assertIn('AccessTogether', error)

    def test_primary_comparisons_share_one_observation(self):
        for serial in (False, True):
            with self.subTest(serial=serial):
                output = self.exercise('''DataInPort data[7:0];
DataOutPort a[3:0] { Source data[7:4]; }
DataOutPort b[3:0] { Source data[3:0]; }
Alias pair[7:0] = a, b { AccessTogether; }''',
                    'iWrite data 0x59; iApply; iRead pair 0x59; iApply', serial=serial)
                self.assert_group(output, reads=('iRead a 0b0101', 'iRead b 0b1001'))

    def test_reserved_member_keeps_its_value(self):
        output = self.exercise('''DataInPort a[3:0]; DataInPort b[3:0];
Alias pair[7:0] = a, b { AccessTogether; }''',
            'iTake b; iWrite b 9; iApply; iWrite pair[7:4] 5; iApply', serial=True)
        self.assert_group(output, ('iWrite a 0b0101', 'iWrite b 0b1001'), together=True)

    def test_mutually_exclusive_observations_cannot_satisfy_group(self):
        declarations = '''DataInPort data[3:0]; DataInPort select;
DataOutPort a[3:0] { Source data; Enable select; }
DataOutPort b[3:0] { Source data; Enable ~select; }
Alias pair[7:0] = a, b { AccessTogether; }'''
        self.exercise(declarations,
            'iWrite data 5; iApply; iRead a 5; iRead b 5; iApply', serial=True)
        error = self.exercise(declarations,
            'iWrite data 5; iApply; iRead pair 0x55; iApply', serial=True, ok=False)
        self.assertIn('AccessTogether', error)

    def test_partial_read_observes_unrequested_member(self):
        output = self.exercise('''DataInPort data[7:0];
DataOutPort a[3:0] { Source data[7:4]; }
DataOutPort b[3:0] { Source data[3:0]; }
Alias pair[7:0] = a, b { AccessTogether; }''',
            'iWrite data 0x59; iApply; iRead pair[7:4] 5; iApply', serial=True)
        self.assert_group(output, reads=('iRead a 0b0101', 'iRead b 0bxxxx'))

    def test_group_with_64_primary_members(self):
        pins = [f'p{index:02}' for index in range(64)]
        declarations = '\n'.join(f'DataInPort {pin};' for pin in pins)
        declarations += '\nAlias all_pins[63:0] = ' + ', '.join(pins) + ' { AccessTogether; }'
        output = self.exercise(declarations, 'iWrite all_pins 0xffffffffffffffff; iApply', serial=True)
        self.assert_group(output, tuple(f'iWrite {pin} 0b1' for pin in pins), together=True)


if __name__ == '__main__':
    unittest.main()
