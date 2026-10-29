#!/usr/bin/env python3
"""Self-constructed IEEE 7.9.19 address/read/write enable ownership cases."""
import unittest

import test_primary_access_together as helpers

ADDRESSED = '''AddressPort address[1:0]; DataInPort data[3:0];
WriteEnPort we; ReadEnPort re;
OneHotDataGroup group[3:0] {
  DataRegister a[3:0] { AddressValue 1; ResetValue 4'b0; }
  DataRegister b[3:0] { AddressValue 2; ResetValue 4'b0; }
}
DataOutPort observed[3:0] { Source group; }'''


class ParallelControlReservations(unittest.TestCase):
    exercise = helpers.PrimaryAccessTogether.exercise

    def test_address_blocks_implicit_change_but_allows_replacement_and_release(self):
        prefix = 'iTake address; iWrite address 1; iApply; '
        for serial in (False, True):
            with self.subTest(serial=serial):
                error = self.exercise(ADDRESSED, prefix + 'iWrite b 9; iApply',
                                      serial=serial, ok=False)
                self.assertIn('reserved address', error)
                output = self.exercise(ADDRESSED,
                    prefix + 'iWrite address 2; iWrite b 9; iApply', serial=serial)
                self.assertIn('iWrite address 0b10', output)
                self.exercise(ADDRESSED,
                    prefix + 'iRelease address; iWrite b 9; iApply', serial=serial)

    def test_enable_reservations_block_implicit_activation(self):
        for port, action in (('we', 'iWrite a 9'), ('re', 'iRead a 0')):
            for serial in (False, True):
                with self.subTest(port=port, serial=serial):
                    prefix = f'iTake {port}; iWrite {port} 0; iApply; '
                    error = self.exercise(ADDRESSED, prefix + action + '; iApply',
                                          serial=serial, ok=False)
                    self.assertIn('reserved ' + port, error)
                    self.exercise(ADDRESSED,
                        prefix + f'iWrite {port} 1; {action}; iApply', serial=serial)
                    self.exercise(ADDRESSED,
                        prefix + f'iRelease {port}; {action}; iApply', serial=serial)

    def test_written_child_enable_can_be_explicitly_replaced(self):
        for kind in ('ReadEnPort', 'WriteEnPort'):
            for serial in (False, True):
                with self.subTest(kind=kind, serial=serial):
                    declarations = f'''{kind} pin;
Instance child Of Child {{ InputPort pin = pin; }}
Alias inverted = ~child.pin;'''
                    module = f'Module Child {{ {kind} pin; }}'
                    prefix = 'iTake child.pin; iWrite inverted 1; iApply; '
                    output = self.exercise(declarations,
                        prefix + 'iWrite inverted 0; iApply', serial=serial, modules=module)
                    self.assertIn('iWrite pin 0b1', output)
                    self.assertIn('iWrite pin 0b0', output)
                    self.exercise(declarations, prefix + 'iWrite pin 1; iApply',
                                  serial=serial, modules=module, ok=False)

    def test_child_write_does_not_replace_parent_ownership(self):
        for serial in (False, True):
            with self.subTest(serial=serial):
                declarations = """ReadEnPort pin;
Instance child Of Child { InputPort pin = pin; }"""
                error = self.exercise(declarations,
                    'iTake pin; iWrite pin 0; iApply; iWrite child.pin 1; iApply',
                    serial=serial, modules='Module Child { ReadEnPort pin; }', ok=False)
                self.assertIn('reserved pin', error)

    def test_partial_parent_write_keeps_other_taken_bits_protected(self):
        declarations = """ScanInPort si;
ScanRegister pins[1:0] { ScanInSource si; ResetValue 2'b0; }
ScanOutPort so { Source pins[0]; }
Instance child Of Child { InputPort pin = pins[0]; }"""
        prefix = 'iTake pins; iWrite pins 0; iApply; '
        self.exercise(declarations,
            prefix + 'iWrite pins[1] 1; iWrite child.pin 1; iApply',
            modules='Module Child { ReadEnPort pin; }', ok=False)
        self.exercise(declarations,
            prefix + 'iWrite pins[0] 1; iWrite child.pin 1; iApply',
            modules='Module Child { ReadEnPort pin; }')

    def test_instance_take_contains_control_ports(self):
        declarations = '''ReadEnPort pin;
Instance child Of Child { InputPort pin = pin; }'''
        module = 'Module Child { ReadEnPort pin; }'
        prefix = 'iTake child; iWrite child.pin 0; iApply; '
        error = self.exercise(declarations, prefix + 'iWrite pin 1; iApply',
                              serial=True, modules=module, ok=False)
        self.assertIn('reserved child.pin', error)
        self.exercise(declarations,
            prefix + 'iRelease child; iWrite pin 1; iApply', serial=True, modules=module)

    def test_address_alias_slices_keep_unowned_bits_free(self):
        declarations = '''AddressPort address[3:0];
Alias upper[1:0] = ~address[3:2];
DataOutPort observed[3:0] { Source address; }'''
        prefix = 'iTake upper; iWrite upper 2; iApply; '
        for serial in (False, True):
            with self.subTest(serial=serial):
                self.exercise(declarations,
                    prefix + 'iWrite observed 7; iApply', serial=serial)
                self.exercise(declarations,
                    prefix + 'iWrite observed 11; iApply', serial=serial, ok=False)
                self.exercise(declarations,
                    prefix + 'iWrite upper 1; iApply; iWrite observed 11; iApply',
                    serial=serial)

    def test_return_release_and_reset_clear_control_ownership(self):
        declarations = 'ReadEnPort pin; DataOutPort observed { Source pin; }'
        definition = 'iProc lock {} { iTake pin; iWrite pin 0; iApply }; '
        self.exercise(declarations,
            definition + 'iCall lock; iWrite observed 1; iApply', serial=True)
        self.exercise(declarations,
            definition + 'iTake pin; iCall lock; iWrite observed 1; iApply',
            serial=True, ok=False)
        self.exercise(declarations,
            definition + 'iTake pin; iCall lock; iReset; iWrite observed 1; iApply',
            serial=True)

    def test_many_forwarded_enable_reservations(self):
        declarations = "ScanInPort si; ScanRegister pins[63:0] { ScanInSource si; ResetValue 64'b0; }\nScanOutPort so { Source pins[0]; }\n" + ''.join(
            f'Instance c{i} Of Child {{ InputPort pin = pins[{i}]; }}\n' for i in range(64))
        commands = ''.join(f'iTake c{i}.pin; iWrite c{i}.pin 0; ' for i in range(64))
        commands += 'iApply; ' + ''.join(f'iWrite c{i}.pin 1; ' for i in range(64)) + 'iApply'
        output = self.exercise(declarations, commands,
                               modules='Module Child { ReadEnPort pin; }')
        self.assertIn('iWrite pins 0b' + '1' * 64, output)


if __name__ == '__main__':
    unittest.main()
