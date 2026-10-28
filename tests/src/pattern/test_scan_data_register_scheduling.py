#!/usr/bin/env python3
"""Serial DataRegister writes, captures and emitted-program replay."""
import pathlib
import subprocess
import sys
import tempfile
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[3]
BIN = pathlib.Path(sys.argv.pop(1)).resolve() if len(sys.argv) > 1 else ROOT / 'build/icl-pattern-generation'
FIXTURES = ROOT / 'tests/cases/ours/pattern_generation/scan_data_registers'


class ScanDataRegisters(unittest.TestCase):
    def setUp(self):
        temp = tempfile.TemporaryDirectory()
        self.addCleanup(temp.cleanup)
        self.root = pathlib.Path(temp.name)
        self.icl = FIXTURES / 'selectable.icl'

    def run_tool(self, body=None, source='source.pdl', output='result.pdl',
                 icl=None, ok=True, extra=()):
        if body is not None:
            (self.root / source).write_text('iProcsForModule ScanData\n' + body)
        result = subprocess.run([str(BIN), '--icl', str(icl or self.icl),
                                 '--top_module', 'ScanData', '--pdl', source,
                                 '--output', output, *extra], cwd=self.root,
                                capture_output=True, text=True, timeout=45)
        self.assertEqual(result.returncode == 0, ok, result.stdout + result.stderr)
        return (self.root / output).read_text() if ok else result.stdout

    def replay(self, text):
        self.assertEqual(text, self.run_tool(source='result.pdl', output='replay.pdl'))
        self.assertEqual(text, self.run_tool(icl='debug/ScanData.icl', output='flat.pdl'))

    def test_selectable_partial_write(self):
        text = self.run_tool('iReset\niWrite value 9\niApply\n'
                             'iWrite value[1:0] 2\niApply\n', extra=('--debug',))
        self.assertIn('iWrite bus 0b11001', text)
        self.assertIn('iWrite bus 0b11010', text)
        self.assertNotIn('iWrite value', text)
        self.assertEqual(text.count('iApply\n'), 2)
        self.replay(text)

    def test_eight_addressed_registers_and_replay(self):
        self.icl = FIXTURES / 'addressed_eight.icl'
        (self.root / 'source.pdl').write_text((FIXTURES / 'addressed_eight.pdl').read_text())
        text = self.run_tool(extra=('--debug',))
        self.assertEqual(text.count('iApply\n'), 8)
        for address in range(8):
            self.assertIn(f'iWrite bus 0b{(128 | (address << 4) | (address + 1)):08b}', text)
        self.replay(text)

    def test_addressed_writes(self):
        self.icl = self.root / 'addressed.icl'
        self.icl.write_text("""Module ScanData {
  ScanInPort si;
  ScanRegister bus[5:0] { ScanInSource si; ResetValue 6'b0; }
  Instance bank Of Bank { InputPort address = bus[4]; InputPort we = bus[5]; InputPort data = bus[3:0]; }
  ScanOutPort so { Source bus[0]; }
}
Module Bank {
  AddressPort address; WriteEnPort we; DataInPort data[3:0];
  OneHotDataGroup group[3:0] {
  DataRegister a[3:0] { AddressValue 1'b0; ResetValue 4'b0; }
  DataRegister b[3:0] { AddressValue 1'b1; ResetValue 4'b0; }
  }
}""")
        text = self.run_tool('iReset\niWrite bank.a 9\niWrite bank.b 6\niApply\n', extra=('--debug',))
        self.assertIn('iWrite bus 0b101001', text)
        self.assertIn('iWrite bus 0b110110', text)
        self.assertEqual(text.count('iApply\n'), 2)
        self.replay(text)

    def test_capture_before_update(self):
        text = self.run_tool('iReset\niWrite value 9\niApply\n'
                             'iRead value 9\niWrite value 6\niApply\n', extra=('--debug',))
        self.assertIn('iRead observed 0b1001', text)
        self.assertIn('iWrite bus 0b10110', text)
        self.assertEqual(text.count('iApply\n'), 2)
        self.replay(text)

    def test_inverted_partial_capture_without_inventing_storage(self):
        self.icl = self.root / 'inverted.icl'
        self.icl.write_text((FIXTURES / 'selectable.icl').read_text().replace(
            'CaptureSource value;', 'CaptureSource ~value;').replace("ResetValue 4'b0;", ''))
        text = self.run_tool('iRead value[2:1] 1\niApply\n', extra=('--debug',))
        self.assertIn('iRead observed 0bx10x', text)
        self.assertNotIn('iWrite bus', text)
        self.replay(text)

    def test_capture_mux_requires_setup_before_observation(self):
        self.icl = self.root / 'mux.icl'
        self.icl.write_text((FIXTURES / 'selectable.icl').read_text().replace(
            'ScanRegister observed', "DataMux mux[3:0] SelectedBy bus[4] { 1'b0: 4'b0; 1'b1: value; }\n  ScanRegister observed").replace(
                'CaptureSource value;', 'CaptureSource mux;'))
        text = self.run_tool('iReset\niRead value 0\niApply\n', extra=('--debug',))
        self.assertEqual(text.count('iApply\n'), 2)
        self.assertIn('iRead observed 0b0000', text)
        self.replay(text)

    def test_unobservable_capture_is_atomic_error(self):
        self.icl = self.root / 'missing.icl'
        self.icl.write_text((FIXTURES / 'selectable.icl').read_text().replace('CaptureSource value;', ''))
        (self.root / 'result.pdl').write_text('existing output\n')
        text = self.run_tool('iRead value 9\niApply\n', ok=False)
        self.assertIn('unobservable', text)
        self.assertEqual((self.root / 'result.pdl').read_text(), 'existing output\n')

    def test_data_end_state_replay(self):
        self.icl = self.root / 'end.icl'
        self.icl.write_text((FIXTURES / 'selectable.icl').read_text().replace(
            'ScanRegister observed', "Alias idle = value[0] { iApplyEndState 1'b0; }\n  ScanRegister observed"))
        text = self.run_tool('iReset\niWrite value 1\niApply\n', extra=('--debug',))
        self.assertEqual(text.count('iApply\n'), 2)
        self.replay(text)

    def test_write_and_read_together_with_scan_goal(self):
        text = self.run_tool('iReset\niWrite value 9\niWrite observed 6\niApply -together\n', extra=('--debug',))
        self.assertIn('iWrite bus 0b11001', text)
        self.assertIn('iWrite observed 0b0110', text)
        self.assertEqual(text.count('iApply'), 1)
        self.replay(text)

    def test_unrequested_data_storage_is_preserved(self):
        self.icl = self.root / 'shared.icl'
        self.icl.write_text((FIXTURES / 'selectable.icl').read_text().replace(
            'ScanRegister observed', "DataRegister other[3:0] { WriteDataSource bus[3:0]; WriteEnSource bus[4]; ResetValue 4'b0; }\n  ScanRegister observed"))
        self.run_tool('iReset\niWrite value 9\niApply\n', ok=False)

    def test_tap_write_and_capture(self):
        self.icl = FIXTURES / 'vendor.icl'
        extra = ('--debug', '--bsdl', 'AccessProbe', str(ROOT / 'tests/cases/ours/pattern_generation/access_link/simple.bsdl'))
        text = self.run_tool('iReset\niWrite core.value 9\niApply\n'
                             'iRead core.value 9\niWrite core.value 6\niApply\n', extra=extra)
        self.assertIn('iRead core.observed 0b1001', text)
        self.assertIn('iWrite core.bus 0b10110', text)
        self.assertEqual(text, self.run_tool(source='result.pdl', output='replay.pdl', extra=extra[1:]))

    def test_cascaded_write(self):
        self.icl = self.root / 'cascade.icl'
        self.icl.write_text((FIXTURES / 'selectable.icl').read_text().replace(
            'ScanRegister observed', "DataRegister leaf[3:0] { WriteDataSource value; ResetValue 4'b0; }\n  ScanRegister observed"))
        text = self.run_tool('iReset\niWrite leaf 9\niApply\n', extra=('--debug',))
        self.assertIn('iWrite bus 0b11001', text)
        self.replay(text)

    def test_data_access_together_alias(self):
        self.icl = self.root / 'alias.icl'
        self.icl.write_text((FIXTURES / 'selectable.icl').read_text().replace(
            'ScanRegister observed', 'Alias pair[7:0] = value, observed { AccessTogether; }\n  ScanRegister observed'))
        text = self.run_tool('iReset\niWrite pair 0x96\niApply\n', extra=('--debug',))
        self.assertIn('iWrite bus 0b11001', text)
        self.assertIn('iWrite observed 0b0110', text)
        self.assertEqual(text.count('iApply'), 1)
        self.replay(text)

    def test_reserved_scan_bus_cannot_be_used_for_data_write(self):
        text = self.run_tool('iReset\niTake bus\niWrite bus 0\niApply\n'
                             'iWrite value 9\niApply\n', ok=False)
        self.assertIn('reserved', text)

    def test_addressed_reads_share_capture_path(self):
        self.icl = self.root / 'addressed_read.icl'
        self.icl.write_text("""Module ScanData {
  ScanInPort si;
  ScanRegister bus[6:0] { ScanInSource si; ResetValue 7'b0; }
  Instance bank Of Bank {
    InputPort address = bus[4]; InputPort we = bus[5];
    InputPort re = bus[6]; InputPort data = bus[3:0];
  }
  ScanRegister observed[3:0] { ScanInSource bus[0]; CaptureSource bank.out; ResetValue 4'b0; }
  ScanOutPort so { Source observed[0]; }
}
Module Bank {
  AddressPort address; WriteEnPort we; ReadEnPort re; DataInPort data[3:0];
  OneHotDataGroup group[3:0] {
    DataRegister a[3:0] { AddressValue 1'b0; ResetValue 4'b0; }
    DataRegister b[3:0] { AddressValue 1'b1; ResetValue 4'b0; }
  }
  DataOutPort out[3:0] { Source group; }
}""")
        text = self.run_tool('iReset\niRead bank.a 9\niRead bank.b 6\niApply\n', extra=('--debug',))
        self.assertIn('iRead observed 0b1001', text)
        self.assertIn('iRead observed 0b0110', text)
        self.assertEqual(text.count('iApply\n'), 3)
        self.replay(text)

    def test_together_only_synchronizes_writes(self):
        text = self.run_tool('iReset\niRead value 9\niRead observed 6\niApply -together\n', extra=('--debug',))
        self.assertEqual(text.count('iApply\n'), 2)
        self.assertIn('iRead observed 0b1001', text)
        self.assertIn('iRead observed 0b0110', text)
        self.replay(text)

    def test_repeated_read_last_comparison_wins(self):
        text = self.run_tool('iReset\niRead value 9\niRead value 6\niApply\n', extra=('--debug',))
        self.assertIn('iRead observed 0b0110', text)
        self.assertNotIn('iRead observed 0b1001', text)
        self.replay(text)

    def test_unqualified_read_replaces_prior_comparison(self):
        text = self.run_tool('iReset\niRead value 9\niRead value\niApply\n', extra=('--debug',))
        self.assertIn('iRead observed 0bxxxx', text)
        self.replay(text)

    def test_unchanged_data_goal_updates_its_scan_controller(self):
        self.icl = FIXTURES / 'bypass.icl'
        text = self.run_tool('iReset\niWrite value 0\niApply\n', extra=('--debug',))
        self.assertIn('iWrite bus ', text)
        self.assertEqual(text.count('iApply\n'), 2)
        self.replay(text)


if __name__ == '__main__':
    unittest.main()
