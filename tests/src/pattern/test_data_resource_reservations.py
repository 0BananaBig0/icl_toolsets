#!/usr/bin/env python3
"""Self-constructed IEEE 7.9.19 data-resource ownership and replay cases."""
import pathlib
import subprocess
import sys
import tempfile
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[3]
BIN = pathlib.Path(sys.argv.pop(1)).resolve() if len(sys.argv) > 1 else ROOT / 'build/icl-pattern-generation'
FIXTURES = ROOT / 'tests/cases/ours/pattern_generation'


class DataResources(unittest.TestCase):
    def setUp(self):
        temp = tempfile.TemporaryDirectory()
        self.addCleanup(temp.cleanup)
        self.root = pathlib.Path(temp.name)
        self.top = 'SelectableData'
        self.icl = FIXTURES / 'data_registers/selectable.icl'

    def run_tool(self, body=None, ok=True, pdl='source.pdl', icl=None, output='result.pdl', extra=()):
        if body is not None:
            (self.root / 'source.pdl').write_text(f'iProcsForModule {self.top}\n' + body)
        result = subprocess.run([str(BIN), '--icl', str(icl or self.icl), '--pdl', pdl,
                                 '--output', output, *extra], cwd=self.root,
                                capture_output=True, text=True, timeout=45)
        self.assertEqual(result.returncode == 0, ok, result.stdout + result.stderr)
        return (self.root / output).read_text() if ok else result.stdout

    def replay(self, text):
        self.assertEqual(text, self.run_tool(pdl='result.pdl', output='replay.pdl'))
        self.assertEqual(text, self.run_tool(icl=self.root / f'debug/{self.top}.icl', output='flat.pdl'))

    def test_primary_register_rejects_implicit_change_and_allows_explicit_update(self):
        prefix = 'iReset; iTake value; iWrite value 5; iApply; '
        output = self.run_tool(prefix + 'iWrite value 9; iApply', extra=('--debug',))
        self.assertIn('iWrite data 0b1001', output)
        self.replay(output)
        error = self.run_tool(prefix + 'iWrite data 9; iWrite enable 1; iApply', ok=False)
        self.assertIn('reserved value', error)
        self.assertEqual((self.root / 'result.pdl').read_text(), output)
        self.run_tool(prefix + 'iRelease value; iWrite data 9; iWrite enable 1; iApply')

    def test_primary_port_reservation_constrains_register_assignment(self):
        prefix = 'iReset; iTake data; iWrite data 5; iApply; '
        self.assertIn('reserved data', self.run_tool(prefix + 'iWrite value 9; iApply', ok=False))
        output = self.run_tool(prefix + 'iWrite data 9; iWrite value 9; iApply', extra=('--debug',))
        self.replay(output)
        self.run_tool(prefix + 'iRelease data; iWrite value 9; iApply')

    def test_scan_register_reservations_on_data_storage_slices(self):
        self.top = 'ScanData'
        self.icl = self.root / 'source.icl'
        source = (FIXTURES / 'scan_data_registers/selectable.icl').read_text()
        source = source.replace('  ScanOutPort so', '  Alias kept[1:0] = ~value[3:2];\n  ScanOutPort so')
        self.icl.write_text(source)
        prefix = 'iReset; iTake kept; iWrite value 5; iApply; '
        output = self.run_tool(prefix + 'iWrite bus 22; iApply', extra=('--debug',))
        self.assertIn('iWrite bus 0b10110', output)
        self.replay(output)
        self.assertIn('reserved value', self.run_tool(prefix + 'iWrite bus 26; iApply', ok=False))
        self.assertEqual((self.root / 'result.pdl').read_text(), output)
        self.run_tool(prefix + 'iRelease kept; iWrite bus 26; iApply')

    def test_instance_ownership_contains_data_storage(self):
        prefix = 'iReset; iTake {}; iWrite value 5; iApply; '
        self.assertIn('reserved value', self.run_tool(prefix + 'iWrite data 9; iWrite enable 1; iApply', ok=False))
        self.run_tool(prefix + 'iReset; iWrite data 9; iWrite enable 1; iApply')

    def test_output_alias_preserves_computed_port_value(self):
        self.top = 'Ports'
        self.icl = self.root / 'source.icl'
        self.icl.write_text('''Module Ports {
  DataInPort data[3:0];
  DataOutPort observed[3:0] { Source data; }
  Alias inverted[3:0] = ~observed;
}
''')
        prefix = 'iTake inverted; iWrite inverted 10; iApply; '
        output = self.run_tool(prefix + 'iWrite inverted 6; iApply', extra=('--debug',))
        self.replay(output)
        self.assertIn('reserved observed', self.run_tool(prefix + 'iWrite data 9; iApply', ok=False))
        self.assertEqual((self.root / 'result.pdl').read_text(), output)

    def test_expected_reads_do_not_establish_taken_hardware_state(self):
        self.run_tool('iReset; iTake value; iRead value 5; iApply; '
                      'iWrite data 9; iWrite enable 1; iApply')
        self.top = 'ScanData'
        self.icl = FIXTURES / 'scan_data_registers/selectable.icl'
        self.run_tool('iReset; iTake value; iRead value 5; iApply; iWrite bus 25; iApply')

    def test_partial_port_protection_and_nested_ownership(self):
        prefix = 'iReset; iTake data[3:2]; iWrite data[3:2] 1; iApply; '
        output = self.run_tool(prefix + 'iWrite value 6; iApply', extra=('--debug',))
        self.replay(output)
        self.assertIn('reserved data', self.run_tool(prefix + 'iWrite value 10; iApply', ok=False))
        definitions = 'iReset; iProc own {} {iTake data; iWrite data 5; iApply}\n'
        self.run_tool(definitions + 'iCall own; iWrite value 9; iApply')
        self.assertIn('reserved data', self.run_tool(
            definitions + 'iTake data; iCall own; iWrite value 9; iApply', ok=False))

    def test_end_state_conflict_is_preserved_in_both_backends(self):
        for family, top in (('data_registers', 'SelectableData'), ('scan_data_registers', 'ScanData')):
            with self.subTest(family=family):
                self.top = top
                source = (FIXTURES / family / 'selectable.icl').read_text()
                source = source.replace('  DataRegister value',
                                        "  Alias idle = value[0] { iApplyEndState 1'b0; }\n  DataRegister value")
                self.icl = self.root / 'source.icl'
                self.icl.write_text(source)
                (self.root / 'result.pdl').write_text('previous output')
                next_write = 'data 5' if family == 'data_registers' else 'bus 0'
                error = self.run_tool('iReset; iTake value; iWrite value 5; iApply; '
                                      f'iWrite {next_write}; iApply', ok=False)
                self.assertIn('iApplyEndState conflicts with reserved value', error)
                self.assertEqual((self.root / 'result.pdl').read_text(), 'previous output')

    def test_addressed_bank_preserves_taken_data_across_batches(self):
        self.top = 'ScanData'
        self.icl = self.root / 'source.icl'
        source = (FIXTURES / 'scan_data_registers/addressed_eight.icl').read_text()
        source = source.replace('bus[7:0]', 'bus[9:0]').replace("8'b0", "10'b0")
        source = source.replace('bus[6:4]', 'bus[8:4]').replace('bus[7]', 'bus[9]')
        source = source.replace('address[2:0]', 'address[4:0]')
        begin, end = source.index('    DataRegister r0'), source.index('\n  }', source.index('    DataRegister r0'))
        source = source[:begin] + ''.join(
            f"    DataRegister r{i}[3:0] {{ AddressValue {i}; ResetValue 4'b0; }}\n"
            for i in range(32)) + source[end:]
        self.icl.write_text(source)
        print(f'addressed resource bank: registers=32 icl_bytes={len(source.encode())}')
        setup = 'iReset; ' + ''.join(f'iTake bank.r{i}; iWrite bank.r{i} 5; ' for i in range(16)) + 'iApply; '
        output = self.run_tool(setup + ''.join(f'iWrite bank.r{i} 9; ' for i in range(16, 32)) + 'iApply',
                               extra=('--debug',))
        self.assertEqual(output.count('iApply\n'), 32)
        for address in range(32):
            data = 5 if address < 16 else 9
            self.assertIn(f'iWrite bus 0b{512 | (address << 4) | data:010b}', output)
        self.replay(output)
        self.assertIn('reserved bank.r0', self.run_tool(setup + 'iWrite bus 521; iApply', ok=False))
        self.assertEqual((self.root / 'result.pdl').read_text(), output)
        for taken in ('', '[3:2]'):
            with self.subTest(taken=taken):
                setup = 'iReset; ' + ''.join(
                    f'iTake bank.r{i}{taken}; iWrite bank.r{i} 5; ' for i in range(32)) + 'iApply; '
                replacement = setup + ''.join(f'iWrite bank.r{i} 9; ' for i in range(32)) + 'iApply; '
                output = self.run_tool(replacement, extra=('--debug',))
                self.assertEqual(output.count('iApply\n'), 64)
                self.replay(output)
                self.assertIn('reserved bank.r0', self.run_tool(replacement + 'iWrite bus 517; iApply', ok=False))
                if taken:
                    self.run_tool(replacement + 'iWrite bus 522; iApply')


if __name__ == '__main__':
    unittest.main()
