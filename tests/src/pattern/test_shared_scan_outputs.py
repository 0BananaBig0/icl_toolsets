#!/usr/bin/env python3
"""Self-constructed shared-output cases exercise physical scan-path identity."""
import pathlib
import subprocess
import sys
import tempfile
import time
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[3]
BIN = pathlib.Path(sys.argv.pop(1)).resolve() if len(sys.argv) > 1 else ROOT / 'build/icl-pattern-generation'

NETWORK = """Module SharedTop {
  ScanInPort si;
  ScanRegister data[3:0] { ScanInSource si; ResetValue 4'b0; }
  ScanOutPort a { Source data[0]; }
  ScanOutPort b { Source ~data[0]; }
}
"""
PDL = """iPDLLevel 0
iProcsForModule SharedTop
iReset
iWrite data 0b1001
iApply
iRead data 0b1001
iWrite data 0b0110
iApply
iRead data 0b0110
iApply
"""


class SharedScanOutputs(unittest.TestCase):
    def setUp(self):
        temporary = tempfile.TemporaryDirectory()
        self.addCleanup(temporary.cleanup)
        self.root = pathlib.Path(temporary.name)

    def run_tool(self, icl, pdl, output='result.pdl', ok=True):
        result = subprocess.run(
            [str(BIN), '--icl', str(icl), '--pdl', str(pdl),
             '--top_module', 'SharedTop', '--output', str(self.root / output), '--debug'],
            capture_output=True, text=True, timeout=45)
        self.assertEqual(result.returncode == 0, ok, result.stdout + result.stderr)
        return (self.root / output).read_bytes() if ok else result.stdout + result.stderr

    def replay(self, source, pdl=PDL):
        icl, commands = self.root / 'source.icl', self.root / 'source.pdl'
        icl.write_text(source)
        commands.write_text(pdl)
        original = self.run_tool(icl, commands)
        flat = self.root / 'debug/SharedTop.icl'
        serialized = flat.read_bytes()
        self.assertEqual(original, self.run_tool(flat, commands, 'flat/result.pdl'))
        self.assertEqual(serialized, (self.root / 'flat/debug/SharedTop.icl').read_bytes())
        self.assertEqual(original, self.run_tool(icl, self.root / 'result.pdl', 'replay/result.pdl'))
        return original

    def test_direct_and_inverted_observations(self):
        for inversion in ('', '~'):
            with self.subTest(inversion=inversion):
                source = NETWORK.replace('Source data[0]', f'Source {inversion}data[0]')
                text = self.replay(source)
                self.assertEqual(text.count(b'iApply\n'), 3)
                self.assertIn(b'iWrite data 0b1001', text)
                self.assertIn(b'iRead data 0b0110', text)

    def test_vector_outputs_and_shared_mux(self):
        source = NETWORK.replace('ScanOutPort a { Source data[0]; }', '''
  ScanRegister select { ScanInSource data[0]; ResetValue 1'b0; }
  ScanMux path SelectedBy select { 1'b0: data[0]; 1'b1: select; }
  ScanOutPort a[5:2] { Source path, ~path, path, ~path; }''')
        source = source.replace('Source ~data[0]', 'Source path')
        self.replay(source)

    def test_hierarchical_forwarding(self):
        source = '''Module Child {
  ScanInPort si;
  ScanRegister data[3:0] { ScanInSource si; ResetValue 4'b0; }
  ScanOutPort a { Source data[0]; }
  ScanOutPort b { Source ~data[0]; }
}
Module SharedTop {
  ScanInPort si;
  Instance child Of Child { InputPort si = si; }
  ScanOutPort a { Source child.a; }
  ScanOutPort b { Source child.b; }
}
'''
        self.replay(source, PDL.replace(' data ', ' child.data '))

    def test_enabled_observations(self):
        for a, b in (("1'b0", "1'b1"), ("1'b1", "1'b0"),
                     ("1'b1", "1'b1"), ('unknown', "1'b1")):
            with self.subTest(a=a, b=b):
                source = NETWORK.replace('ScanInPort si;', 'ScanInPort si; DataInPort unknown;')
                source = source.replace('Source data[0];', f'Source data[0]; Enable {a};')
                source = source.replace('Source ~data[0];', f'Source ~data[0]; Enable {b};')
                self.replay(source)

    def test_changing_enabled_observations(self):
        source = NETWORK.replace('ScanOutPort a { Source data[0]; }', '''
  ScanRegister select { ScanInSource data[0]; ResetValue 1'b0; }
  LogicSignal inverse { ~select; }
  ScanOutPort a { Source select; Enable inverse; }''')
        source = source.replace('Source ~data[0];', 'Source ~select; Enable select;')
        self.replay(source, PDL.replace('iWrite data 0b1001', 'iWrite select 1\niWrite data 0b1001')
                              .replace('iWrite data 0b0110', 'iWrite select 0\niWrite data 0b0110'))

    def test_gated_forwarding_and_vectors(self):
        source = NETWORK.replace('ScanOutPort a { Source data[0]; }', '''
  ScanOutPort inner { Source data[0]; Enable 1'b1; }
  ScanOutPort a[3:2] { Source inner, ~inner; Enable 1'b0; }''')
        self.replay(source)

    def test_no_observable_path_preserves_output(self):
        for enable in ("1'b0", "1'bx"):
            source = NETWORK
            source = source.replace('Source data[0];', f'Source data[0]; Enable {enable};')
            source = source.replace('Source ~data[0];', f'Source ~data[0]; Enable {enable};')
            icl, pdl = self.root / 'source.icl', self.root / 'source.pdl'
            icl.write_text(source)
            pdl.write_text(PDL)
            (self.root / 'result.pdl').write_bytes(b'previous result\n')
            self.run_tool(icl, pdl, ok=False)
            self.assertEqual((self.root / 'result.pdl').read_bytes(), b'previous result\n')

    def test_distinct_paths_preserve_outputs(self):
        for source in (
                NETWORK.replace('Source ~data[0]', 'Source si'),
                NETWORK.replace('ScanOutPort b',
                                'ScanRegister other { ScanInSource si; } ScanOutPort b')
                       .replace('Source ~data[0]', 'Source other')):
            with self.subTest(source=source):
                icl, pdl = self.root / 'source.icl', self.root / 'source.pdl'
                icl.write_text(source)
                pdl.write_text(PDL)
                (self.root / 'result.pdl').write_bytes(b'previous result\n')
                error = self.run_tool(icl, pdl, ok=False)
                self.assertIn('shared-control', error)
                self.assertEqual((self.root / 'result.pdl').read_bytes(), b'previous result\n')

    def test_wide_shared_observation(self):
        outputs = '\n'.join(f"ScanOutPort out{i} {{ Source data[0]; Enable 1'b{int(i == 255)}; }}"
                            for i in range(256))
        source = NETWORK.replace("data[3:0]", "data[63:0]").replace("4'b0", "64'b0")
        source = source.replace('ScanOutPort b { Source ~data[0]; }', outputs)
        start = time.monotonic()
        self.replay(source)
        print(f'shared observations: 257 outputs, 64 storage bits, {len(source)} bytes, '
              f'3 replays: {time.monotonic() - start:.3f}s')


if __name__ == '__main__':
    unittest.main()
