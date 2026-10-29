#!/usr/bin/env python3
"""Self-constructed logical port requirements across parallel/scan frames."""
import unittest

import test_primary_access_together as helpers


class MixedPortWrites(unittest.TestCase):
    exercise = helpers.PrimaryAccessTogether.exercise
    assert_group = helpers.PrimaryAccessTogether.assert_group

    def test_primary_forwarded_and_inverted_ports(self):
        output = self.exercise('''DataInPort data[3:0];
DataOutPort out[3:0] { Source data; }
DataOutPort inverted[3:0] { Source ~data; }''',
            'iWrite inverted 10; iApply; iWrite out[1:0] 3; iApply', serial=True)
        self.assertIn('iWrite data 0b0101', output)
        # Each logical port retains independent controlling history: out starts at zero.
        self.assertIn('iWrite data 0b0011', output)

    def test_scan_forwarded_ports(self):
        output = self.exercise('''DataOutPort out[3:0] { Source sr; }''',
            'iWrite out 9; iApply', serial=True)
        self.assertIn('iWrite sr 0b1001', output)

    def test_hierarchical_data_input_requires_scan_update_even_when_unchanged(self):
        output = self.exercise('''Instance sink Of Sink { InputPort data = sr; }''',
            'iWrite sink.data 0; iApply', serial=True,
            modules='Module Sink { DataInPort data[3:0]; }')
        self.assertIn('iWrite sr 0b0000', output)

    def test_computed_port_uses_both_scan_and_primary_drivers(self):
        output = self.exercise('''DataInPort data[3:0];
DataOutPort out[7:0] { Source sr, data; }''',
            'iWrite out 0x59; iApply', serial=True)
        self.assertIn('iWrite sr 0b0101', output)
        self.assertIn('iWrite data 0b1001', output)

    def test_logic_port_constrains_both_drivers(self):
        output = self.exercise('''DataInPort pin;
LogicSignal parity { sr[0] ^ pin; }
DataOutPort out { Source parity; }''',
            'iWrite sr 1; iWrite out 0; iApply', serial=True)
        self.assertIn('iWrite pin 0b1', output)
        self.assertIn('iWrite sr 0b0001', output)

    def test_output_enable_and_scan_data_share_state(self):
        output = self.exercise('''DataInPort enable;
DataOutPort out[3:0] { Source sr; Enable enable; }''',
            'iWrite out 9; iApply; iRead out 9; iApply', serial=True)
        self.assertIn('iWrite enable 0b1', output)
        self.assertIn('iWrite sr 0b1001', output)

    def test_output_history_survives_physical_source_writes(self):
        output = self.exercise('DataOutPort out[3:0] { Source sr; }',
            'iWrite out 9; iApply; iWrite sr 5; iApply; iWrite out 0bx; iApply', serial=True)
        self.assertGreaterEqual(output.count('iWrite sr 0b1001'), 2)

    def test_computed_port_conflict_keeps_previous_output(self):
        error = self.exercise('''DataInPort data[3:0];
DataOutPort out[3:0] { Source data; }
DataOutPort inverted[3:0] { Source ~data; }''',
            'iWrite out 5; iWrite inverted 5; iApply', serial=True, ok=False)
        self.assertIn('iWrite out', error)
        self.assertIn('iWrite inverted', error)

    def test_reserved_source_cannot_change_for_computed_goal(self):
        error = self.exercise('DataOutPort out[3:0] { Source sr; }',
            'iTake sr; iWrite sr 5; iApply; iWrite out 9; iApply', serial=True, ok=False)
        self.assertIn('reserved sr', error)

    def test_computed_port_group_updates_on_one_scan(self):
        output = self.exercise('''DataOutPort out[3:0] { Source sr; }
Alias pair[7:0] = out, sr { AccessTogether; }''',
            'iWrite pair 0x99; iApply', serial=True)
        self.assert_group(output, ('iWrite sr 0b1001',), together=True)

    def test_group_uses_selected_mux_driver_not_inactive_branch(self):
        declarations = '''DataInPort pin; DataInPort selector;
DataMux mux SelectedBy selector { 1'b0: pin; 1'b1: sr[0]; }
DataOutPort out { Source mux; }
Alias pair[4:0] = out, sr { AccessTogether; }'''
        self.exercise(declarations,
            'iTake selector; iWrite selector 0; iApply; iWrite pair 0x19; iApply',
            serial=True, ok=False)
        output = self.exercise(declarations,
            'iTake selector; iWrite selector 1; iApply; iWrite pair 0x19; iApply',
            serial=True)
        self.assert_group(output, ('iWrite sr 0b1001',), together=True)

    def test_constant_mux_selector_is_not_an_update_event(self):
        self.exercise('''DataInPort pin;
DataMux mux SelectedBy 1'b0 { 1'b0: pin; 1'b1: sr[0]; }
DataOutPort out { Source mux; }
Alias pair[4:0] = out, sr { AccessTogether; }''',
            'iWrite pair 0x19; iApply', serial=True, ok=False)

    def test_masked_logic_operand_is_not_a_group_update(self):
        self.exercise('''DataInPort pin;
LogicSignal masked { (sr[0] & 1'b0) | pin; }
DataOutPort out { Source masked; }
Alias pair[4:0] = out, sr { AccessTogether; }''',
            'iWrite pair 0x19; iApply', serial=True, ok=False)

    def test_correlated_logic_update_can_establish_knownness(self):
        output = self.exercise("""DataInPort pin;
LogicSignal zero { pin ^ pin; }
DataOutPort out { Source zero; }""",
            'iWrite out 0; iApply', serial=True)
        self.assertIn('iWrite pin 0b', output)

    def test_correlated_logic_update_does_not_count_an_unrelated_scan(self):
        self.exercise("""DataInPort pin;
LogicSignal zero { pin ^ pin; }
DataOutPort out { Source zero; }
Alias pair[4:0] = out, sr { AccessTogether; }""",
            'iWrite pair 0x09; iApply', serial=True, ok=False)

    def test_grouped_logic_outputs_have_independent_activity_witnesses(self):
        output = self.exercise('''DataInPort pin[1:0];
LogicSignal zero { pin == 2'b00; }
LogicSignal one { pin == 2'b01; }
DataOutPort out0 { Source zero; } DataOutPort out1 { Source one; }
Alias pair[1:0] = out0, out1 { AccessTogether; }''',
            'iWrite pair 0; iApply', serial=True)
        self.assertRegex(output, r'iWrite pin 0b1[01]')
        self.assertIn('iApply -together', output)

    def test_reconvergent_controller_dependencies_are_not_enumerated(self):
        declarations = "LogicSignal l0 { sr[0] | sr[0]; }\n"
        for index in range(1, 64):
            declarations += f'LogicSignal l{index} {{ l{index-1} | l{index-1}; }}\n'
        declarations += 'Instance sink Of Sink { InputPort data = l63; }'
        output = self.exercise(declarations, 'iWrite sink.data 1; iApply', serial=True,
            modules='Module Sink { DataInPort data; }')
        self.assertIn('iWrite sr 0b0001', output)

    def test_inactive_disconnected_mux_branch_is_not_required(self):
        declarations = '''DataInPort pin; DataInPort selector;
ScanRegister dead { ScanInSource si; ResetValue 1'b0; }
DataMux mux SelectedBy selector { 1'b0: pin; 1'b1: dead; }
DataOutPort out { Source mux; }
Instance sink Of Sink { InputPort data = mux; }'''
        for target in ('out', 'sink.data'):
            with self.subTest(target=target):
                self.exercise(declarations,
                    f'iWrite selector 0; iWrite {target} 1; iApply', serial=True,
                    modules='Module Sink { DataInPort data; }')

    def test_tap_forwarded_primary_port(self):
        output = self.exercise('''DataInPort data[3:0];
DataOutPort out[3:0] { Source data; }''',
            'iWrite core.sr 5; iApply; iWrite out 9; iApply', tap=True)
        self.assertIn('iWrite data 0b1001', output)

    def test_mux_select_and_inverted_primary_data(self):
        output = self.exercise('''DataInPort data[3:0]; DataInPort select;
DataMux mux[3:0] SelectedBy select { 1'b0: 4'b0; 1'b1: ~data; }
DataOutPort out[3:0] { Source mux; }''',
            'iWrite out 5; iApply', serial=True)
        self.assert_group(output, ('iWrite data 0b1010', 'iWrite select 0b1'))

    def test_one_hot_padding_does_not_become_known_hardware(self):
        output = self.exercise('''DataInPort data[3:0]; DataInPort enable;
Instance driver Of Driver { InputPort data = data; InputPort enable = enable; }
OneHotDataGroup wide[7:0] { Port driver.value; }
DataOutPort out[7:0] { Source wide; }''',
            'iWrite out 0bxxxx1001; iApply', serial=True,
            modules='''Module Driver {
DataInPort data[3:0]; DataInPort enable;
DataOutPort value[3:0] { Source data; Enable enable; }
}''')
        self.assert_group(output, ('iWrite data 0b1001', 'iWrite enable 0b1'))

    def test_addressed_one_hot_output_justifies_read_controls(self):
        output = self.exercise('''AddressPort address[1:0]; DataInPort data[3:0];
WriteEnPort we; ReadEnPort re;
OneHotDataGroup group[3:0] {
  DataRegister a[3:0] { AddressValue 1; ResetValue 4'b0; }
  DataRegister b[3:0] { AddressValue 2; ResetValue 4'b0; }
}
DataOutPort out[3:0] { Source group; }''',
            'iWrite out 9; iApply', serial=True)
        self.assertIn('iWrite re 0b1', output)
        self.assertIn('iWrite data 0b1001', output)

    def test_constant_port_write_is_a_valid_noop(self):
        self.exercise("DataOutPort out[3:0] { Source 4'b1010; }",
            'iWrite out 10; iApply', serial=True)
        self.exercise("DataOutPort out[3:0] { Source 4'b1010; }",
            'iWrite out 5; iApply', serial=True, ok=False)

    def test_wide_computed_port(self):
        output = self.exercise('''DataInPort data[127:0];
DataOutPort out[127:0] { Source ~data; }''',
            'iWrite out 0; iApply', serial=True)
        self.assertIn('iWrite data 0b' + '1' * 128, output)

    def test_child_enable_uses_existing_address_resolution(self):
        output = self.exercise('''WriteEnPort we; ReadEnPort re;
Instance child Of Child { InputPort we = we; InputPort re = re; }''',
            'iWrite child.we 1; iWrite child.re 1; iApply', serial=True,
            modules='Module Child { WriteEnPort we; ReadEnPort re; }')
        self.assert_group(output, ('iWrite we 0b1', 'iWrite re 0b1'))

    def test_disconnected_child_enable_reports_its_cause(self):
        error = self.exercise('''WriteEnPort we; Instance child Of Child;''',
            'iWrite child.we 1; iApply', serial=True, ok=False,
            modules='Module Child { WriteEnPort we; }')
        self.assertIn('child.we: disconnected WriteEnPort', error)


if __name__ == '__main__':
    unittest.main()
