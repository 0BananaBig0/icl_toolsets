#!/usr/bin/env python3
"""One client clocks disjoint scan chains together, including unequal lengths."""
import pathlib
import subprocess
import sys
import tempfile
import time
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[3]
BIN = pathlib.Path(sys.argv.pop(1)).resolve() if len(sys.argv) > 1 else ROOT / 'build/icl-pattern-generation'
NETWORK = '''Module Chains {
  ScanInPort si[3:2];
  ScanRegister a[3:0] { ScanInSource si[3]; CaptureSource a; ResetValue 4'b0; }
  ScanRegister b[1:0] { ScanInSource si[2]; CaptureSource b; ResetValue 2'b0; }
  ScanOutPort so[5:4] { Source a[0], b[0]; }
  SelectPort sel; CaptureEnPort ce; ShiftEnPort se; UpdateEnPort ue; TCKPort ck;
  ScanInterface client { Chain first { Port si[3]; Port so[5]; } Chain second { Port si[2]; Port so[4]; } Port sel; Port ce; Port se; Port ue; Port ck; }
}
'''
PDL = '''iPDLLevel 1
iProcsForModule Chains
iReset
iWrite a 9; iWrite b 2; iApply -together
iRead a 9; iRead b 2; iWrite a 6; iWrite b 1; iApply -together
iRead a 6; iRead b 1; iApply -together
'''


class SimultaneousChains(unittest.TestCase):
    def setUp(self):
        temp = tempfile.TemporaryDirectory()
        self.addCleanup(temp.cleanup)
        self.root = pathlib.Path(temp.name)

    def run_tool(self, icl, pdl, output='result.pdl', ok=True):
        result = subprocess.run([str(BIN), '--icl', str(icl), '--pdl', str(pdl),
                                 '--top_module', 'Chains', '--output', str(self.root / output),
                                 '--debug'], capture_output=True, text=True, timeout=45)
        self.assertEqual(result.returncode == 0, ok, result.stdout + result.stderr)
        return (self.root / output).read_bytes() if ok else result.stdout + result.stderr

    def replay(self, source=NETWORK, pdl=PDL):
        icl, commands = self.root / 'source.icl', self.root / 'source.pdl'
        icl.write_text(source)
        commands.write_text(pdl)
        output = self.run_tool(icl, commands)
        self.assertEqual(output, self.run_tool(self.root / 'debug/Chains.icl', commands,
                                              'flat/result.pdl'))
        self.assertEqual(output, self.run_tool(icl, self.root / 'result.pdl', 'replay/result.pdl'))
        return output

    def test_unequal_vector_lanes_and_common_updates(self):
        for ascending in (False, True):
            source = NETWORK.replace('[3:2]', '[2:3]') if ascending else NETWORK
            self.assertEqual(self.replay(source).count(b'iApply'), 3)

    def test_inverted_paths(self):
        source = NETWORK.replace('ScanInSource si[3]', 'ScanInSource ~si[3]')
        source = source.replace('Source a[0], b[0]', 'Source ~a[0], ~b[0]')
        self.replay(source)

    def test_alternative_enabled_observations_per_chain(self):
        source = NETWORK.replace('ScanOutPort so[5:4] { Source a[0], b[0]; }', '''
  ScanOutPort so[5:4] { Source a[0], b[0]; Enable 1'b0; }
  ScanOutPort observed[7:6] { Source ~a[0], b[0]; Enable 1'b1; }''')
        source = source.replace('Port so[5];', 'Port observed[7];').replace('Port so[4];', 'Port observed[6];')
        self.replay(source)

    def test_other_chain_sets_selector_before_access(self):
        child = """Module Child {
  ScanInPort si; ScanOutPort so { Source a[0]; } SelectPort sel;
  ScanRegister a[3:0] { ScanInSource si; CaptureSource a; ResetValue 4'b0; }
}
"""
        source = NETWORK.replace("ScanRegister a[3:0] { ScanInSource si[3]; CaptureSource a; ResetValue 4'b0; }",
                                 'Instance child Of Child { InputPort si = si[3]; InputPort sel = b[0]; }')
        source = source.replace('Source a[0], b[0]', 'Source path, b[0]')
        source = source.replace('  SelectPort sel;',
                                "  ScanMux path SelectedBy b[0] { 1'b0: si[3]; 1'b1: child.so; }\n  SelectPort sel;")
        body = 'iPDLLevel 1\niProcsForModule Chains\niReset\niWrite child.a 9; iApply\niRead child.a 9; iApply\n'
        output = self.replay(child + source, body)
        self.assertIn(b'iWrite b 0b01', output)
        primary = source.replace('InputPort sel = b[0]', 'InputPort sel = enable')
        primary = primary.replace('SelectedBy b[0]', 'SelectedBy enable')
        primary = primary.replace('ScanInPort si[3:2];', 'ScanInPort si[3:2]; DataInPort enable;')
        self.replay(child + primary, body)

    def test_partial_alias_writes_preserve_other_lanes(self):
        source = NETWORK.replace('  SelectPort sel;', '  Alias low[3:0] = a[1:0], b;\n  SelectPort sel;')
        body = PDL[:PDL.index('iRead a 9')] + 'iWrite low 7; iApply\niRead a 9; iRead b 3; iApply\n'
        self.replay(source, body)

    def test_reservation_keeps_other_chain_storage(self):
        body = ('iPDLLevel 1\niProcsForModule Chains\niReset\n'
                'iTake b; iWrite b 2; iApply; iWrite a 9; iApply; iRead b 2; iApply\n')
        self.replay(NETWORK, body)

    def test_shared_input_or_storage_requires_event_solver(self):
        for source in (NETWORK.replace('ScanInSource si[2]', 'ScanInSource si[3]'),
                       NETWORK.replace('ScanInSource si[2]', 'ScanInSource a[0]'),
                       NETWORK.replace('Port si[3]; Port so[5];', 'Port si[2]; Port so[5];')
                       .replace('Port si[2]; Port so[4];', 'Port si[3]; Port so[4];')):
            icl, pdl = self.root / 'source.icl', self.root / 'source.pdl'
            icl.write_text(source)
            pdl.write_text(PDL)
            (self.root / 'result.pdl').write_bytes(b'previous output\n')
            self.run_tool(icl, pdl, ok=False)
            self.assertEqual((self.root / 'result.pdl').read_bytes(), b'previous output\n')

    def test_unlisted_output_does_not_replace_disabled_chain(self):
        source = NETWORK.replace('Source a[0], b[0];', "Source a[0], b[0]; Enable 1'b0;")
        source = source.replace('  SelectPort sel;', '  ScanOutPort extra { Source a[0]; }\n  SelectPort sel;')
        icl, pdl = self.root / 'source.icl', self.root / 'source.pdl'
        icl.write_text(source)
        pdl.write_text(PDL)
        (self.root / 'result.pdl').write_bytes(b'previous output\n')
        self.run_tool(icl, pdl, ok=False)
        self.assertEqual((self.root / 'result.pdl').read_bytes(), b'previous output\n')

    def test_many_lanes_scale(self):
        count = 64
        source = f'Module Chains {{ ScanInPort si[{count - 1}:0];\n'
        for i in range(count):
            source += f"ScanRegister r{i}[3:0] {{ ScanInSource si[{i}]; ResetValue 4'b0; }}\n"
        source += f'ScanOutPort so[{count - 1}:0] {{ Source '
        source += ', '.join(f'r{i}[0]' for i in reversed(range(count))) + '; }\n'
        source += 'SelectPort sel; ScanInterface client { Port sel;\n'
        source += ''.join(f'Chain c{i} {{ Port si[{i}]; Port so[{i}]; }}\n' for i in range(count))
        source += '}\n}\n'
        body = 'iPDLLevel 1\niProcsForModule Chains\niReset\n'
        body += '\n'.join(f'iWrite r{i} {i % 16}' for i in range(count)) + '\niApply -together\n'
        start = time.monotonic()
        self.replay(source, body)
        print(f'simultaneous chains: {count} lanes, {len(source)} bytes, three replays {time.monotonic() - start:.3f}s')


if __name__ == '__main__':
    unittest.main()
