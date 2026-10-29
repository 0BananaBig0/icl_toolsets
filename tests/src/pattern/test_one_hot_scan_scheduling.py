#!/usr/bin/env python3
"""IEEE 1687 one-hot scan returns and flattened-program fixed points."""
import pathlib
import subprocess
import sys
import tempfile
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[3]
BIN = pathlib.Path(sys.argv.pop(1)).resolve() if len(sys.argv) > 1 else ROOT / 'build/icl-pattern-generation'
FIXTURES = ROOT / 'tests/cases/ours/pattern_generation/one_hot_scan'


class OneHotScan(unittest.TestCase):
    def setUp(self):
        temp = tempfile.TemporaryDirectory()
        self.addCleanup(temp.cleanup)
        self.root = pathlib.Path(temp.name)
        (self.root / 'source.icl').write_text((FIXTURES / 'network.icl').read_text())
        (self.root / 'source.pdl').write_text((FIXTURES / 'source.pdl').read_text())

    def run_tool(self, icl='source.icl', pdl='source.pdl', output='result.pdl', ok=True, extra=()):
        result = subprocess.run(
            [str(BIN), '--icl', icl, '--pdl', pdl, '--top_module', 'OneHotTop',
             '--output', output, '--debug', *extra], cwd=self.root,
            capture_output=True, text=True, timeout=45)
        self.assertEqual(result.returncode == 0, ok, result.stdout + result.stderr)
        return (self.root / output).read_text() if ok else result.stdout

    def replay(self, text):
        flat = (self.root / 'debug/OneHotTop.icl').read_text()
        self.assertEqual(text, self.run_tool(pdl='result.pdl', output='replay.pdl'))
        self.assertEqual(text, self.run_tool(icl='debug/OneHotTop.icl', output='flat/result.pdl'))
        self.assertEqual(flat, (self.root / 'flat/debug/OneHotTop.icl').read_text())

    def test_nested_write_read_and_replay(self):
        text = self.run_tool()
        self.assertIn('iWrite branches.a.data 0b1001', text)
        self.assertIn('iWrite branches.b.data 0b0110', text)
        self.assertIn('iRead branches.a.data 0b1001', text)
        self.assertIn('iRead branches.b.data 0b0110', text)
        self.replay(text)

    def test_inverted_returns(self):
        path = self.root / 'source.icl'
        path.write_text(path.read_text().replace('Port a.so;', 'Port ~a.so;')
                        .replace('Port branches.so;', 'Port ~branches.so;'))
        self.replay(self.run_tool())

    def test_tap_execution_and_flat_replay(self):
        path = self.root / 'source.icl'
        path.write_text(path.read_text().replace('Module OneHotTop', 'Module OneHotNetwork')
                        .replace('Source select[0]; }',
                                 'Source select[0]; }\n'
                                 '  SelectPort selected; CaptureEnPort capture;\n'
                                 '  ShiftEnPort shift; UpdateEnPort update; TCKPort clock;\n'
                                 '  ScanInterface client { Port si; Port so; Port selected;\n'
                                 '    Port capture; Port shift; Port update; Port clock; }')
                        + '''\nModule OneHotTop {
  Instance network Of OneHotNetwork {}
  AccessLink tap Of STD_1149_1_2001 {
    BSDLEntity AccessProbe;
    IJTAG_ON { ScanInterface { network.client; } }
    IJTAG_OFF { ScanInterface { network.client; } }
  }
}\n''')
        pdl = self.root / 'source.pdl'
        pdl.write_text(pdl.read_text().replace('branches.', 'network.branches.'))
        bsdl = FIXTURES.parent / 'access_link/simple.bsdl'
        text = self.run_tool(extra=('--bsdl', 'AccessProbe', str(bsdl)))
        # The debug file retains the TAP model, so neither replay needs BSDL.
        self.assertEqual(text, self.run_tool(icl='debug/OneHotTop.icl', output='flat/result.pdl'))
        self.assertEqual(text, self.run_tool(icl='debug/OneHotTop.icl', pdl='result.pdl',
                                             output='replay/result.pdl'))

    def test_unknown_and_contending_enables_preserve_output(self):
        original = (self.root / 'source.icl').read_text()
        for expression in ("1'b1", "1'bx"):
            with self.subTest(expression=expression):
                (self.root / 'source.icl').write_text(
                    original.replace("select == 2'b10", expression))
                (self.root / 'result.pdl').write_text('previous output\n')
                self.run_tool(ok=False)
                self.assertEqual((self.root / 'result.pdl').read_text(), 'previous output\n')

    def test_together_cannot_update_disjoint_branches(self):
        path = self.root / 'source.pdl'
        path.write_text(path.read_text().replace('iApply', 'iApply -together'))
        self.run_tool(ok=False)

    def test_reserved_selector_is_not_overwritten(self):
        (self.root / 'source.pdl').write_text(
            'iProcsForModule OneHotTop\niReset\niTake select\n'
            'iWrite select 0\niApply\niWrite branches.a.data 9\niApply\n')
        self.run_tool(ok=False)

    def test_many_distributed_returns(self):
        source = (FIXTURES / 'network.icl').read_text()
        source = source[:source.index('Module OneHotReturns')] + '''
Module OneHotTop {
  ScanInPort si;
  ScanRegister select[5:0] { ScanInSource route; ResetValue 6'b0; }
  ScanOutPort so { Source select[0]; }
'''
        for i in range(32):
            source += (f"  LogicSignal enable{i} {{ select == 6'd{i}; }}\n"
                       f'  Instance b{i} Of OneHotBranch {{ InputPort si = si; '
                       f'InputPort enabled = enable{i}; }}\n')
        source += '  OneHotScanGroup route {\n' + ''.join(
            f'    Port b{i}.so;\n' for i in range(32)) + '  }\n}\n'
        (self.root / 'source.icl').write_text(source)
        (self.root / 'source.pdl').write_text(
            'iProcsForModule OneHotTop\niReset\niWrite b31.data 9\niApply\n')
        text = self.run_tool()
        self.assertEqual(text.count('iApply\n'), 2)
        self.replay(text)


if __name__ == '__main__':
    unittest.main()
