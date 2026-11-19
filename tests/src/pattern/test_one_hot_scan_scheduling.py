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

    def vector_source(self):
        return '''
Module Pair {
 ScanInPort si[1:0]; DataInPort enabled;
 ScanRegister low[3:0] { ScanInSource si[0]; CaptureSource low; ResetValue 0; }
 ScanRegister high[1:0] { ScanInSource si[1]; CaptureSource high; ResetValue 0; }
 ScanOutPort so[1:0] { Source high[0], low[0]; Enable enabled; }
}
Module OneHotTop {
 ScanInPort si[1:0]; DataInPort choice;
 LogicSignal left_enabled { ~choice; }
 LogicSignal right_enabled { choice; }
 Instance left Of Pair { InputPort si = si; InputPort enabled = left_enabled; }
 Instance right Of Pair { InputPort si = si; InputPort enabled = right_enabled; }
 OneHotScanGroup route[1:0] { Port left.so; Port right.so; }
 ScanOutPort so[1:0] { Source route; }

}
'''

    def test_vector_independent_returns_and_replay(self):
        source = self.vector_source()
        variants = [source,
                    source.replace('Port left.so;', 'Port ~left.so;')
                    .replace('Port right.so;', 'Port ~right.so;'),
                    source.replace('si[1:0]', 'si[5:4]')
                    .replace('si[0]', 'si[4]').replace('si[1]', 'si[5]')
                    .replace('so[1:0]', 'so[4:5]').replace('route[1:0]', 'route[4:5]'),
                    source.replace('ScanOutPort so[1:0] { Source route; }',
                                   'ScanOutPort out_low { Source route[0]; }\n'
                                   'ScanOutPort out_high { Source route[1]; }')]
        program = 'iProcsForModule OneHotTop\niReset\n'
        for choice, branch in [(0, 'left'), (1, 'right')]:
            program += (f'iWrite choice {choice}\niApply\n'
                        f'iWrite {branch}.low 9\niWrite {branch}.high 2\niApply -together\n'
                        f'iRead {branch}.low 9\niRead {branch}.high 2\niApply -together\n')
        (self.root / 'source.pdl').write_text(program)
        for source in variants:
            with self.subTest(source=source):
                (self.root / 'source.icl').write_text(source)
                text = self.run_tool(extra=('--local_reset', 'reset.pdl'))
                for branch in ['left', 'right']:
                    self.assertIn(f'iRead {branch}.low 0b1001', text)
                    self.assertIn(f'iRead {branch}.high 0b10', text)
                self.replay(text)
                self.assertEqual(text, self.run_tool(icl='debug/OneHotTop.icl',
                                                     pdl='result.pdl', output='both/result.pdl'))
                self.assertEqual(text, self.run_tool(pdl='reset.pdl', output='reset/result.pdl'))
        # An explicit declaration is an existing planner control for the same network.
        source = self.vector_source()
        (self.root / 'source.icl').write_text(source)
        expected = self.run_tool()
        source = source.rstrip()[:-1] + '''
  SelectPort sel; CaptureEnPort ce; ShiftEnPort se; UpdateEnPort ue; TCKPort ck;
  ScanInterface client {
    Chain low { Port si[0]; Port so[0]; }
    Chain high { Port si[1]; Port so[1]; }
    Port sel; Port ce; Port se; Port ue; Port ck;
  }
}
'''
        (self.root / 'source.icl').write_text(source)
        self.assertEqual(expected, self.run_tool(output='declared/result.pdl'))

    def test_vector_return_boundaries_preserve_artifacts(self):
        source = self.vector_source()
        variants = [
            (source.replace('ScanInSource si[1]', 'ScanInSource si[0]'), 'overlapping-event'),
            (source.replace('Source high[0], low[0]', 'Source low[0], low[0]'), 'overlapping-event'),
            (source.replace('Instance right Of Pair { InputPort si = si;',
                            'Instance right Of Pair { InputPort si = si[0], si[1];'),
             'unambiguous scan input'),
            (source.replace('ScanInSource si[1];', 'ScanInSource low[0];'), 'overlapping-event'),
            (source.replace('Instance right Of Pair { InputPort si = si;',
                            "Instance right Of Pair { InputPort si = 1'b0, si[0];"),
             'unambiguous scan input'),
            (source.replace('LogicSignal right_enabled { choice; }',
                            "LogicSignal right_enabled { 1'b1; }"), 'scan'),
            (source.replace('InputPort enabled = right_enabled;',
                            "InputPort enabled = 1'bx;"), 'scan'),
            (source.rstrip()[:-1] + ' SelectPort sel; CaptureEnPort ce; ShiftEnPort se; '
             'UpdateEnPort ue; TCKPort ck; ScanInterface client { Port si[0]; Port so[0]; '
             'Port sel; Port ce; Port se; Port ue; Port ck; } }', 'shared-control'),
            (source.rstrip()[:-1] + ' ShiftEnPort se; }', 'interface controls')]
        (self.root / 'source.pdl').write_text(
            'iProcsForModule OneHotTop\niReset\niWrite choice 0\niApply\n'
            'iWrite left.low 9\niWrite left.high 2\niApply -together\n')
        artifacts = ['result.pdl', 'reset.pdl', 'debug/OneHotTop.icl']
        for source, diagnostic in variants:
            with self.subTest(diagnostic=diagnostic, source=source):
                (self.root / 'source.icl').write_text(source)
                program = ('iProcsForModule OneHotTop\niReset\niWrite choice 0\niApply\n'
                           'iWrite left.low 9\niWrite left.high 2\niApply -together\n')
                (self.root / 'source.pdl').write_text(program)
                for name in artifacts:
                    path = self.root / name
                    path.parent.mkdir(parents=True, exist_ok=True)
                    path.write_text('previous artifact\n')
                message = self.run_tool(ok=False, extra=('--local_reset', 'reset.pdl'))
                self.assertIn(diagnostic, message)
                for name in artifacts:
                    self.assertEqual((self.root / name).read_text(), 'previous artifact\n')

    def test_vector_return_scale(self):
        lanes, branches = 8, 8
        source = 'Module Lane { ScanInPort si; DataInPort enabled;\n'
        source += 'ScanRegister data[3:0] { ScanInSource si; CaptureSource data; ResetValue 0; }\n'
        source += 'ScanOutPort so { Source data[0]; Enable enabled; } }\n'
        source += f'Module OneHotTop {{ ScanInPort si[{lanes - 1}:0]; DataInPort choice[2:0];\n'
        for lane in range(lanes):
            for branch in range(branches):
                source += (f'LogicSignal e{lane}_{branch} {{ choice == 3\'d{branch}; }}\n'
                           f'Instance b{lane}_{branch} Of Lane {{ InputPort si = si[{lane}]; '
                           f'InputPort enabled = e{lane}_{branch}; }}\n')
            source += 'OneHotScanGroup r%d { '%lane + ''.join(
                f'Port b{lane}_{branch}.so; ' for branch in range(branches)) + '}\n'
        source += f'ScanOutPort so[{lanes - 1}:0] {{ Source '+', '.join(
            f'r{lane}' for lane in reversed(range(lanes))) + '; }\n}\n'
        (self.root / 'source.icl').write_text(source)
        program = 'iProcsForModule OneHotTop\niReset\niWrite choice 7\niApply\n'
        program += ''.join(f'iWrite b{lane}_7.data {lane + 1}\n' for lane in range(lanes))
        program += 'iApply -together\n' + ''.join(
            f'iRead b{lane}_7.data {lane + 1}\n' for lane in range(lanes)) + 'iApply -together\n'
        (self.root / 'source.pdl').write_text(program)
        text = self.run_tool()
        self.assertEqual(text.count('iRead b'), lanes)
        self.replay(text)

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
