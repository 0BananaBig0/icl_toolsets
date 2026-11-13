#!/usr/bin/env python3
"""Standalone client phases also clock storage outside the observed path."""
import pathlib
import subprocess
import sys
import tempfile
import time
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[3]
BIN = pathlib.Path(sys.argv.pop(1)).resolve() if len(sys.argv) > 1 else ROOT / 'build/icl-pattern-generation'
RESOURCE = ROOT / 'tests/cases/ours/pattern_generation/resources'
NETWORK = (RESOURCE / 'slices.icl').read_text().split('Module SliceTop')[0]
PDL = 'iPDLLevel 1\niProcsForModule SliceInstrument\niReset\niWrite target 5; iApply\n'


class HiddenClients(unittest.TestCase):
    def setUp(self):
        temp = tempfile.TemporaryDirectory()
        self.addCleanup(temp.cleanup)
        self.root = pathlib.Path(temp.name)

    def run_tool(self, icl, pdl, output='result.pdl', ok=True):
        result = subprocess.run([str(BIN), '--icl', str(icl), '--pdl', str(pdl),
                                 '--top_module', 'SliceInstrument', '--output', str(self.root / output),
                                 '--debug'], capture_output=True, text=True, timeout=60)
        self.assertEqual(result.returncode == 0, ok, result.stdout + result.stderr)
        return (self.root / output).read_bytes() if ok else result.stdout + result.stderr

    def replay(self, source=NETWORK, pdl=PDL):
        icl, commands = self.root / 'source.icl', self.root / 'source.pdl'
        icl.write_text(source)
        commands.write_text(pdl)
        output = self.run_tool(icl, commands)
        self.assertEqual(output, self.run_tool(self.root / 'debug/SliceInstrument.icl', commands,
                                              'flat/result.pdl'))
        self.assertEqual(output, self.run_tool(icl, self.root / 'result.pdl', 'replay/result.pdl'))
        return output

    def reject(self, source, pdl=PDL):
        icl, commands = self.root / 'source.icl', self.root / 'source.pdl'
        icl.write_text(source)
        commands.write_text(pdl)
        (self.root / 'result.pdl').write_bytes(b'previous output\n')
        error = self.run_tool(icl, commands, ok=False)
        self.assertEqual((self.root / 'result.pdl').read_bytes(), b'previous output\n')
        return error

    def test_hidden_capture_shift_update(self):
        output = self.replay()
        self.assertIn(b'iWrite target 0b0101', output)
        self.assertGreaterEqual(output.count(b'iApply'), 2)

    def test_indexed_inputs_preserve_other_bits(self):
        source = NETWORK.replace('ScanInPort si;', 'ScanInPort si[5:2];')
        source = source.replace('ScanInSource si;', 'ScanInSource si[3];').replace('Port si;', 'Port si[3];')
        self.replay(source)

    def test_resource_slice_cases(self):
        for name in ('slice_high', 'slice_low', 'slice_release', 'slice_alias', 'slice_partial_write'):
            with self.subTest(name=name):
                body = (RESOURCE / f'{name}.pdl').read_text().replace('SliceTop', 'SliceInstrument').replace('core.', '')
                if name == 'slice_low':
                    self.reject(NETWORK, body)
                else:
                    self.replay(NETWORK, body)

    def test_hidden_reservation_cannot_be_destroyed(self):
        body = PDL.replace('iWrite target 5; iApply',
                           'iTake target; iState target 0; iWrite gate 1; iApply')
        self.reject(NETWORK, body)

    def wrapper(self, capture='capture', shift='shift'):
        child = NETWORK.replace('SliceInstrument', 'Child')
        parent = '''Module SliceInstrument {
  ScanInPort si; ScanOutPort so { Source child.so; }
  SelectPort selected; CaptureEnPort capture; ShiftEnPort shift;
  UpdateEnPort update; TCKPort clock;
  Instance child Of Child {
    InputPort si = si; InputPort selected = selected;
    InputPort capture = CAPTURE; InputPort shift = SHIFT;
    InputPort update = update; InputPort clock = clock;
  }
  ScanInterface client {
    Port si; Port so; Port selected; Port capture; Port shift; Port update; Port clock;
  }
}
'''.replace('CAPTURE', capture).replace('SHIFT', shift)
        return child + parent

    def test_suppressed_capture_and_unknown_shift(self):
        self.replay(self.wrapper(capture="1'b0"), PDL.replace(' target ', ' child.target '))
        self.reject(self.wrapper(shift="1'bx"), PDL.replace(' target ', ' child.target '))

    def test_primary_enable_is_scheduled_before_hidden_access(self):
        source = self.wrapper().replace('Module SliceInstrument {',
                                         'Module SliceInstrument { DataInPort enable;')
        source = source.replace('InputPort selected = selected;', 'InputPort selected = enable;')
        body = PDL.replace(' target ', ' child.target ')
        for initial in ('', 'iState enable 0;\n', 'iState enable 1;\n'):
            with self.subTest(initial=initial):
                output = self.replay(source, body.replace('iReset\n', 'iReset\n' + initial))
                self.assertIn(b'iWrite child.target 0b0101', output)
                if initial != 'iState enable 1;\n':
                    self.assertIn(b'iWrite enable 0b1', output)
        self.reject(source, body.replace('iReset\n', 'iReset\niTake enable; iWrite enable 0; iApply;\n'))

    def test_simultaneous_unequal_hidden_chains(self):
        source = NETWORK.replace('ScanInPort si;', 'ScanInPort si[1:0];')
        source = source.replace('ScanInSource si;', 'ScanInSource si[0];')
        source = source.replace('  SelectPort selected;', '''
  ScanRegister other[3:0] { ScanInSource si[1]; CaptureSource other; ResetValue 4'b0; }
  ScanOutPort other_so { Source other[0]; }
  SelectPort selected;''')
        source = source.replace('Port si; Port so;',
                                'Chain first { Port si[0]; Port so; } Chain second { Port si[1]; Port other_so; }')
        self.replay(source, PDL.replace('iWrite target 5;', 'iWrite other 9; iWrite target 5;'))

    def test_larger_hidden_storage_and_work_limit(self):
        for width in (32, 128, 1024):
            source = NETWORK.replace('target[3:0]', f'target[{width - 1}:0]')
            source = source.replace("ResetValue 4'b0", f"ResetValue {width}'b0")
            start = time.monotonic()
            if width == 1024:
                self.assertIn('scan event resource limit', self.reject(source))
            else:
                self.replay(source)
                print(f'hidden client: {width} storage bits, {len(source)} bytes, three replays {time.monotonic() - start:.3f}s')


if __name__ == '__main__':
    unittest.main()
