#!/usr/bin/env python3
"""Shared data-register events, primary retargeting and replay."""
import pathlib
import subprocess
import sys
import tempfile
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[3]
BIN = pathlib.Path(sys.argv.pop(1)).resolve() if len(sys.argv) > 1 else ROOT / 'build/icl-pattern-generation'
FIXTURES = ROOT / 'tests/cases/ours/pattern_generation/data_registers'


class DataRegisterScheduling(unittest.TestCase):
    def setUp(self):
        temp = tempfile.TemporaryDirectory()
        self.addCleanup(temp.cleanup)
        self.root = pathlib.Path(temp.name)
        self.top = 'SelectableData'
        self.icl = FIXTURES / 'selectable.icl'

    def run_tool(self, body=None, source='source.pdl', output='result.pdl',
                 icl=None, ok=True, extra=()):
        if body is not None:
            (self.root / source).write_text(f'iProcsForModule {self.top}\n' + body)
        result = subprocess.run([str(BIN), '--icl', str(icl or self.icl),
                                 '--top_module', self.top, '--pdl', source,
                                 '--output', output, *extra], cwd=self.root,
                                capture_output=True, text=True, timeout=40)
        self.assertEqual(result.returncode == 0, ok, result.stdout + result.stderr)
        return (self.root / output).read_text() if ok else result.stdout

    def replay(self, text):
        self.assertEqual(text, self.run_tool(source='result.pdl', output='replay.pdl'))
        self.assertEqual(text, self.run_tool(icl=f'debug/{self.top}.icl', output='flat.pdl'))
        self.assertEqual(text, self.run_tool(icl=f'debug/{self.top}.icl', source='result.pdl', output='both.pdl'))

    def test_repeated_read_last_comparison_wins(self):
        text = self.run_tool('iRead value 9\niRead value 6\niApply\n', extra=('--debug',))
        self.assertIn('iRead observed 0b0110', text)
        self.replay(text)

    def test_unqualified_read_replaces_prior_comparison(self):
        text = self.run_tool('iRead value 9\niRead value\niApply\n', extra=('--debug',))
        self.assertIn('iRead observed 0bxxxx', text)
        self.replay(text)

    def test_selectable_partial_write(self):
        text = self.run_tool('iReset\niWrite value 9\niApply\n'
                             'iWrite value[1:0] 2\niApply\n', extra=('--debug',))
        self.assertIn('iWrite data 0b1001', text)
        self.assertIn('iWrite data 0b1010', text)
        self.assertNotIn('iWrite value', text)
        self.assertEqual(text.count('iApply\n'), 2)
        self.replay(text)

    def test_unrequested_register_enable_cannot_remain_unknown(self):
        self.icl = self.root / 'shared_data.icl'
        self.icl.write_text('''Module SelectableData {
  DataInPort data[3:0]; DataInPort ea; DataInPort eb;
  DataRegister a[3:0] { WriteDataSource data; WriteEnSource ea; ResetValue 4'b0; }
  DataRegister b[3:0] { WriteDataSource data; WriteEnSource eb; ResetValue 4'b0; }
}''')
        text = self.run_tool('iReset\niWrite b 6\niApply\n', extra=('--debug',))
        self.assertIn('iWrite ea 0b0', text)
        self.assertIn('iWrite eb 0b1', text)
        self.replay(text)

    def test_data_end_state_survives_low_level_replay(self):
        self.icl = self.root / 'end.icl'
        self.icl.write_text((FIXTURES / 'selectable.icl').read_text().replace(
            'DataOutPort observed', "Alias idle = value[0] { iApplyEndState 1'b0; }\n  DataOutPort observed"))
        text = self.run_tool('iReset\niWrite value 1\niApply\n', extra=('--debug',))
        self.assertEqual(text.count('iApply\n'), 2)
        self.replay(text)

    def test_shared_address_end_states_replay_without_extra_transactions(self):
        self.top = 'AddressedData'
        self.icl = self.root / 'markers.icl'
        self.icl.write_text((FIXTURES / 'addressed.icl').read_text().replace(
            'DataOutPort observed',
            "Alias ida[3:0] = a { iApplyEndState 4'b0; }\n"
            "  Alias idb[3:0] = b { iApplyEndState 4'b0; }\n  DataOutPort observed"))
        text = self.run_tool('iReset\niWrite a 1\niWrite b 6\niApply\n', extra=('--debug',))
        self.replay(text)

    def test_output_read_cannot_overwrite_observed_storage(self):
        self.icl = self.root / 'destructive_read.icl'
        self.icl.write_text((FIXTURES / 'selectable.icl').read_text().replace(
            "ResetValue 4'b0;", '').replace('Source value;', 'Source value; Enable enable;'))
        (self.root / 'result.pdl').write_text('previous output\n')
        self.run_tool('iRead observed 9\niApply\n', ok=False)
        self.assertEqual((self.root / 'result.pdl').read_text(), 'previous output\n')

    def test_logic_output_read_cannot_overwrite_observed_storage(self):
        self.icl = self.root / 'destructive_logic_read.icl'
        self.icl.write_text('''Module SelectableData {
  DataInPort data; DataInPort enable;
  DataRegister value { WriteDataSource data; WriteEnSource enable; }
  LogicSignal inverted { ~value; }
  DataOutPort observed { Source inverted; Enable enable; }
}''')
        (self.root / 'result.pdl').write_text('previous output\n')
        self.run_tool('iRead observed 1\niApply\n', ok=False)
        self.assertEqual((self.root / 'result.pdl').read_text(), 'previous output\n')

    def test_satisfied_output_needs_no_extra_pin_drive(self):
        text = self.run_tool('iReset\niWrite observed 0\niApply\n', extra=('--debug',))
        self.assertNotIn('iWrite', text)
        self.replay(text)

    def test_address_defaults(self):
        self.top = 'AddressDefaultsTop'
        self.icl = ROOT / 'tests/cases/tessent/pattern_generation/address_defaults.icl'
        text = self.run_tool('iWrite child.r 0x96\niApply\n', extra=('--debug',))
        self.assertIn('iWrite address 0b00000101', text)
        self.assertIn('iWrite data 0b1001011000000000', text)
        self.assertIn('iWrite we 0b1', text)
        self.assertNotIn('iWrite child.r', text)
        self.replay(text)

    def test_selectable_read_comparison_does_not_invent_storage(self):
        text = self.run_tool('iRead value 9\niApply\n', extra=('--debug',))
        self.assertIn('iRead observed 0b1001', text)
        self.assertNotIn('iWrite data', text)
        self.replay(text)

    def test_inverted_read(self):
        self.icl = self.root / 'inverted.icl'
        self.icl.write_text((FIXTURES / 'selectable.icl').read_text().replace(
            'Source value;', 'Source ~value;'))
        text = self.run_tool('iRead value 9\niApply\n', extra=('--debug',))
        self.assertIn('iRead observed 0b0110', text)
        self.replay(text)

    def test_mux_read_and_unqualified_partial_comparison(self):
        self.icl = self.root / 'mux.icl'
        self.icl.write_text((FIXTURES / 'selectable.icl').read_text().replace(
            'DataOutPort observed',
            "DataInPort select;\n  DataMux read_mux[3:0] SelectedBy select { "
            "1'b0: 4'b0; 1'b1: value; }\n  DataOutPort observed").replace(
                'Source value;', 'Source read_mux;'))
        text = self.run_tool('iRead value[2:1] 1\niApply\niRead value\niApply\n', extra=('--debug',))
        self.assertIn('iWrite select 0b1', text)
        self.assertIn('iRead observed 0bx01x', text)
        self.assertIn('iRead observed 0bxxxx', text)
        self.replay(text)

    def test_nested_addressed_read(self):
        self.top = 'AddressDefaultsTop'
        self.icl = ROOT / 'tests/cases/tessent/pattern_generation/address_defaults.icl'
        text = self.run_tool('iRead child.r 0x96\niApply\n', extra=('--debug',))
        self.assertIn('iWrite address 0b00000101', text)
        self.assertIn('iRead out 0b10010110', text)
        self.replay(text)

    def test_nested_control_write_and_non_destructive_held_enable_read(self):
        self.top = 'AddressDefaultsTop'
        self.icl = ROOT / 'tests/cases/tessent/pattern_generation/address_defaults.icl'
        text = self.run_tool('iWrite child.r 0x96\niApply\n'
                             'iWrite child.re 1\niWrite re 1\niWrite we 1\n'
                             'iRead out 0x96\niApply\n', extra=('--debug',))
        self.assertIn('iRead out 0b10010110', text)
        self.replay(text)

    def test_addressed_reads_use_distinct_observations(self):
        self.top, self.icl = 'AddressedData', FIXTURES / 'addressed.icl'
        text = self.run_tool('iRead a 9\niRead b 6\niApply\n', extra=('--debug',))
        self.assertEqual(text.count('iApply\n'), 2)
        self.assertIn('iRead observed 0b1001', text)
        self.assertIn('iRead observed 0b0110', text)
        self.assertIn('iWrite re 0b1', text)
        self.assertNotIn('iWrite data', text)
        self.replay(text)

    def test_read_before_write_same_register(self):
        text = self.run_tool('iReset\niRead value 9\niWrite value 6\niApply\n', extra=('--debug',))
        self.assertEqual(text.count('iApply\n'), 2)
        self.assertLess(text.index('iRead observed'), text.index('iWrite data'))
        self.replay(text)

    def test_unobservable_read_preserves_output(self):
        self.icl = self.root / 'unobservable.icl'
        self.icl.write_text((FIXTURES / 'selectable.icl').read_text().replace(
            'Source value;', "Source 4'b0;"))
        (self.root / 'result.pdl').write_text('previous output\n')
        error = self.run_tool('iRead value 9\niApply\n', ok=False)
        self.assertIn('unobservable', error)
        self.assertEqual((self.root / 'result.pdl').read_text(), 'previous output\n')

    def test_shared_bus_targets_use_distinct_updates(self):
        self.top, self.icl = 'AddressedData', FIXTURES / 'addressed.icl'
        text = self.run_tool('iReset\niWrite a 9\niWrite b 6\niApply\n', extra=('--debug',))
        self.assertEqual(text.count('iApply\n'), 2)
        self.assertIn('iWrite address 0b01', text)
        self.assertIn('iWrite address 0b10', text)
        self.replay(text)

    def test_conflicting_together_preserves_outputs(self):
        self.top, self.icl = 'AddressedData', FIXTURES / 'addressed.icl'
        (self.root / 'result.pdl').write_text('previous output\n')
        error = self.run_tool('iReset\niWrite a 9\niWrite b 6\niApply -together\n', ok=False)
        self.assertIn('iApply -together', error)
        self.assertEqual((self.root / 'result.pdl').read_text(), 'previous output\n')


if __name__ == '__main__':
    unittest.main()
