#!/usr/bin/env python3
"""Indexed scan inputs retain bit identity through scheduling and replay."""
import pathlib
import subprocess
import sys
import tempfile
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[3]
BIN = pathlib.Path(sys.argv.pop(1)).resolve() if len(sys.argv) > 1 else ROOT / 'build/icl-pattern-generation'
FIXTURES = ROOT / 'tests/cases/ours/pattern_generation/vector_scan_inputs'


class VectorScanInputs(unittest.TestCase):
    def setUp(self):
        temporary = tempfile.TemporaryDirectory()
        self.addCleanup(temporary.cleanup)
        self.root = pathlib.Path(temporary.name)

    def run_tool(self, icl, pdl, output='result.pdl', ok=True):
        result = subprocess.run(
            [str(BIN), '--icl', str(icl), '--pdl', str(pdl),
             '--top_module', 'VectorScanTop', '--output', str(self.root / output), '--debug'],
            capture_output=True, text=True, timeout=45)
        self.assertEqual(result.returncode == 0, ok, result.stdout + result.stderr)
        return (self.root / output).read_bytes() if ok else result.stdout + result.stderr

    def replay(self, source, pdl):
        icl = self.root / 'source.icl'
        icl.write_text(source)
        original = self.run_tool(icl, pdl)
        flat = self.root / 'debug/VectorScanTop.icl'
        serialized = flat.read_bytes()
        self.assertEqual(original, self.run_tool(flat, pdl, 'flat/result.pdl'))
        self.assertEqual(serialized, (self.root / 'flat/debug/VectorScanTop.icl').read_bytes())
        self.assertEqual(original, self.run_tool(icl, self.root / 'result.pdl', 'replay/result.pdl'))
        return original

    def test_mux_inputs_ascending_and_descending(self):
        source = (FIXTURES / 'network.icl').read_text()
        for bounds in ('5:2', '2:5'):
            with self.subTest(bounds=bounds):
                text = self.replay(source.replace('5:2', bounds), FIXTURES / 'source.pdl')
                self.assertIn(b'iWrite data 0b1001', text)
                self.assertIn(b'iWrite data 0b0110', text)
                self.assertIn(b'iRead data 0b1001', text)
                self.assertIn(b'iRead data 0b0110', text)

    def test_indexed_one_hot_return(self):
        text = self.replay((FIXTURES / 'one_hot.icl').read_text(), FIXTURES / 'one_hot.pdl')
        self.assertIn(b'iWrite a.data 0b1001', text)
        self.assertIn(b'iWrite b.data 0b0110', text)

    def test_nonzero_single_bit_input(self):
        source = (FIXTURES / 'network.icl').read_text()
        self.replay(source.replace('[5:2]', '[2]').replace('si[4]', 'si[2]'),
                    FIXTURES / 'source.pdl')

    def test_unknown_and_contending_one_hot_enables_preserve_outputs(self):
        source = (FIXTURES / 'one_hot.icl').read_text()
        for expression in ("1'bx", "1'b1"):
            with self.subTest(expression=expression):
                icl = self.root / 'source.icl'
                icl.write_text(source.replace("select == 1'b1", expression))
                (self.root / 'result.pdl').write_bytes(b'previous result\n')
                self.run_tool(icl, FIXTURES / 'one_hot.pdl', ok=False)
                self.assertEqual((self.root / 'result.pdl').read_bytes(), b'previous result\n')

    def test_wide_input_bus(self):
        source = (FIXTURES / 'network.icl').read_text().replace('5:2', '4095:0')
        source = source.replace('data[3:0]', 'data[63:0]').replace("4'b0", "64'b0")
        self.replay(source, FIXTURES / 'source.pdl')

    def test_implicit_controls_are_not_ignored(self):
        (self.root / 'result.pdl').write_bytes(b'previous result\n')
        error = self.run_tool(FIXTURES / 'guarded.icl', FIXTURES / 'guarded.pdl', ok=False)
        self.assertIn('event scheduling', error)
        self.assertEqual((self.root / 'result.pdl').read_bytes(), b'previous result\n')

    def test_reset_only_vector_child_remains_supported(self):
        source = (FIXTURES / 'guarded.icl').read_text()
        source = source.replace('SelectPort selected;\n  ShiftEnPort shift;', 'ResetPort reset;')
        source = source.replace('InputPort selected', 'InputPort reset')
        text = self.replay(source, FIXTURES / 'guarded.pdl')
        self.assertIn(b'iWrite child.data 0b1', text)


if __name__ == '__main__':
    unittest.main()
