#!/usr/bin/env python3
"""Independent tree checks plus source/flat/generated-PDL fixed points."""
import pathlib
import subprocess
import sys
import tempfile
import time
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[3]
FIXTURE = ROOT / 'tests/cases/ours/pattern_generation/dependency_scheduling'
BIN = pathlib.Path(sys.argv.pop(1)).resolve() if len(sys.argv) > 1 else ROOT / 'build/icl-pattern-generation'


def padded(body):
    return ('Module DependencyProbe { ScanInPort si;\n' + body + '\n' +
            ''.join(f"ScanRegister pad{i} {{ ScanInSource si; ResetValue 1'b0; }}\n"
                    for i in range(32)) + '}\n')


class DependencyScheduling(unittest.TestCase):
    def setUp(self):
        temporary = tempfile.TemporaryDirectory()
        self.addCleanup(temporary.cleanup)
        self.directory = pathlib.Path(temporary.name)

    def run_tool(self, *args):
        start = time.monotonic()
        result = subprocess.run([str(BIN), *map(str, args)], cwd=self.directory,
                                capture_output=True, text=True, timeout=60)
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        return time.monotonic() - start

    def check_tree(self, text, deep, count=None):
        count = count if count is not None else 40 if deep else 64
        gates = [0] * count
        targets = {}
        pending = {}
        scans = 0
        for line in text.splitlines():
            words = line.split()
            if not words:
                continue
            if words[0] == 'iWrite':
                self.assertEqual(len(words), 3)
                pending[words[1]] = int(words[2], 2)
            elif words[0] == 'iApply':
                visible = set()
                for i in range(count):
                    if deep and i and not gates[i - 1]:
                        break
                    visible.add(f'gate{i}')
                    if not deep and gates[i]:
                        visible.add(f'target{i}')
                if deep and all(gates):
                    visible.add('target')
                self.assertTrue(set(pending) <= visible, (pending, visible))
                for name, value in pending.items():
                    if name.startswith('gate'):
                        gates[int(name[4:])] = value
                    else:
                        targets[name] = value
                pending.clear()
                scans += 1
            else:
                self.assertIn(words[0], ('iPDLLevel', 'iProcsForModule', 'iReset'))
        self.assertFalse(pending)
        self.assertEqual(scans, 41 if deep else 2)
        self.assertEqual(gates, [1] * count)
        self.assertEqual(targets, {'target': 0xa5} if deep else
                         {f'target{i}': 0xa5 for i in range(count)})

    def check_case(self, name, top):
        source = FIXTURE / (name + '.icl')
        pdl = FIXTURE / (name + '.pdl')
        elapsed = self.run_tool('--icl', source, '--pdl', pdl,
                                '--output', 'result.pdl', '--debug')
        original = (self.directory / 'result.pdl').read_bytes()
        self.check_tree(original.decode(), name == 'deep')
        replay_seconds = self.run_tool('--icl', source, '--pdl', 'result.pdl', '--output', 'again.pdl')
        self.assertEqual(original, (self.directory / 'again.pdl').read_bytes())
        flat = self.directory / f'debug/{top}.icl'
        flat_seconds = self.run_tool('--icl', flat, '--pdl', pdl, '--output', 'flat/result.pdl', '--debug')
        self.assertEqual(original, (self.directory / 'flat/result.pdl').read_bytes())
        self.assertEqual(flat.read_bytes(), (self.directory / f'flat/debug/{top}.icl').read_bytes())
        print(f'{name}: source_seconds={elapsed:.6f} replay_seconds={replay_seconds:.6f} '
              f'flat_seconds={flat_seconds:.6f}', flush=True)

    def test_deep_tree(self):
        self.check_case('deep', 'DeepDependencies')

    def test_deep_tree_with_protected_storage(self):
        source = (FIXTURE / 'deep.icl').read_text().replace(
            "ScanRegister gate0 { ScanInSource si;",
            "ScanRegister held[1:0] { ScanInSource si; ResetValue 2'b10; }\n"
            "  ScanRegister gate0 { ScanInSource held[0];")
        (self.directory / 'source.icl').write_text(source)
        (self.directory / 'source.pdl').write_text(
            'iReset\niTake held -persistent\niState held 2\niWrite target 0xa5\niApply\n')
        elapsed = self.run_tool('--icl', 'source.icl', '--pdl', 'source.pdl',
                                '--output', 'result.pdl', '--debug')
        text = (self.directory / 'result.pdl').read_text()
        held = [line for line in text.splitlines() if line.startswith('iWrite held ')]
        self.assertEqual(held, ['iWrite held 0b10'] * 41)
        self.check_tree('\n'.join(line for line in text.splitlines()
                                  if not line.startswith(('iState ', 'iWrite held '))), True)
        for icl, pdl in [('source.icl', 'result.pdl'),
                         ('debug/DeepDependencies.icl', 'source.pdl')]:
            self.run_tool('--icl', icl, '--pdl', pdl, '--output', 'again.pdl')
            self.assertEqual(text, (self.directory / 'again.pdl').read_text())
        print(f'protected deep40: bytes={len(source)} seconds={elapsed:.6f}', flush=True)

    def test_deep_tree_with_explicit_protected_control_update(self):
        source = (FIXTURE / 'deep.icl').read_text().replace(
            "ScanRegister gate0 { ScanInSource si; ResetValue 1'b0; }",
            "ScanRegister gate0[1:0] { ScanInSource si; ResetValue 2'b10; }")
        source = source.replace('gate0;', 'gate0[0];').replace(
            'SelectedBy gate0 {', 'SelectedBy gate0[0] {')
        (self.directory / 'source.icl').write_text(source)
        for write, value in (('iWrite gate0[0] 1', '11'),
                             ('iWrite gate0 0bX1', '11'),
                             ('iWrite gate0 0b01', '01')):
            with self.subTest(write=write):
                (self.directory / 'source.pdl').write_text(
                    f'iReset\niTake gate0\niState gate0 2\n{write}\niWrite target 0xa5\niApply\n')
                self.run_tool('--icl', 'source.icl', '--pdl', 'source.pdl', '--output', 'result.pdl')
                text = (self.directory / 'result.pdl').read_text()
                writes = [line for line in text.splitlines() if line.startswith('iWrite gate0 ')]
                self.assertEqual(writes, [f'iWrite gate0 0b{value}'] * 41)
                normalized = text.replace(f'iWrite gate0 0b{value}', 'iWrite gate0 0b1')
                self.check_tree('\n'.join(line for line in normalized.splitlines()
                                          if not line.startswith('iState ')), True)
                self.run_tool('--icl', 'source.icl', '--pdl', 'result.pdl', '--output', 'again.pdl')
                self.assertEqual(text, (self.directory / 'again.pdl').read_text())

    def test_protected_control_cannot_take_intermediate_value(self):
        (self.directory / 'source.icl').write_text(padded('''
ScanRegister gate[1:0] { ScanInSource si; ResetValue 2'b00; }
ScanRegister target[3:0] { ScanInSource gate[0]; ResetValue 4'b0; }
ScanMux route SelectedBy gate[0] { 1'b0: gate[0]; 1'b1: target[0]; }
ScanOutPort so { Source route; }
'''))
        for write in ('', 'iWrite gate 2;'):
            with self.subTest(write=write):
                (self.directory / 'source.pdl').write_text(
                    f'iReset; iTake gate; iState gate 0; {write} iWrite target 5; iApply')
                (self.directory / 'result.pdl').write_text('previous artifact')
                result = subprocess.run([str(BIN), '--icl', 'source.icl', '--pdl', 'source.pdl',
                                         '--output', 'result.pdl'], cwd=self.directory,
                                        text=True, capture_output=True, timeout=40)
                self.assertNotEqual(result.returncode, 0)
                self.assertIn('reserved gate', result.stdout)
                self.assertEqual((self.directory / 'result.pdl').read_text(), 'previous artifact')

    def test_wide_tree(self):
        self.check_case('wide', 'WideDependencies')

    def test_wider_tree(self):
        source = (FIXTURE / 'wide.icl').read_text().replace(
            '  ScanOutPort so { Source path63; }\n}', '')
        for i in range(64, 256):
            source += f''' ScanRegister gate{i} {{ ScanInSource path{i - 1}; ResetValue 1'b0; }}
 ScanRegister target{i}[7:0] {{ ScanInSource gate{i}; ResetValue 8'b0; }}
 ScanMux path{i} SelectedBy gate{i} {{ 1'b0: gate{i}; 1'b1: target{i}[0]; }}
'''
        source += ' ScanOutPort so { Source path255; }\n}\n'
        (self.directory / 'source.icl').write_text(source)
        (self.directory / 'source.pdl').write_text('iReset\n' + ''.join(
            f'iWrite target{i} 0xa5\n' for i in range(256)) + 'iApply\n')
        seconds = self.run_tool('--icl', 'source.icl', '--pdl', 'source.pdl', '--output', 'result.pdl')
        self.check_tree((self.directory / 'result.pdl').read_text(), False, 256)
        print(f'wide256: bytes={len(source)} registers=512 seconds={seconds:.6f}', flush=True)

    def test_unknown_initial_selectors_do_not_publish(self):
        (self.directory / 'source.pdl').write_text('iWrite target 0xa5\niApply\n')
        (self.directory / 'result.pdl').write_text('previous artifact\n')
        result = subprocess.run([str(BIN), '--icl', str(FIXTURE / 'deep.icl'),
                                 '--pdl', 'source.pdl', '--output', 'result.pdl'],
                                cwd=self.directory, text=True, capture_output=True, timeout=60)
        self.assertNotEqual(result.returncode, 0)
        self.assertIn('reachability not proved', result.stdout)
        self.assertEqual((self.directory / 'result.pdl').read_text(), 'previous artifact\n')

    def test_explicit_selector_close_and_reopen(self):
        pdl = (FIXTURE / 'wide.pdl').read_text().replace(
            'iApply\n', 'iWrite gate0 0\niApply -together\niWrite target0 0x5a\niApply\n')
        (self.directory / 'source.pdl').write_text(pdl)
        self.run_tool('--icl', FIXTURE / 'wide.icl', '--pdl', 'source.pdl', '--output', 'result.pdl')
        result = (self.directory / 'result.pdl').read_text()
        groups = result.split('iApply')
        self.assertEqual(len(groups) - 1, 4)
        self.assertIn('iWrite gate0 0b0\n', groups[1])
        self.assertIn('iWrite target0 0b10100101\n', groups[1])
        self.assertIn('iWrite gate0 0b1\n', groups[2])
        self.assertIn('iWrite target0 0b01011010\n', groups[3])
        self.assertIn('iApply -together', result)
        self.run_tool('--icl', FIXTURE / 'wide.icl', '--pdl', 'result.pdl', '--output', 'again.pdl')
        self.assertEqual(result, (self.directory / 'again.pdl').read_text())

    def test_partial_control_write_keeps_original_fill(self):
        (self.directory / 'source.icl').write_text(padded('''
ScanRegister control[1:0] { ScanInSource si; ResetValue 2'b00; }
ScanRegister target[3:0] { ScanInSource control[0]; ResetValue 4'b0; }
ScanMux route SelectedBy control[1] { 1'b0: control[0]; 1'b1: target[0]; }
ScanOutPort so { Source route; }
'''))
        for request in ('iWrite control[0] 1', 'iWrite control 0bX1'):
            with self.subTest(request=request):
                (self.directory / 'source.pdl').write_text(
                    f'iReset\n{request}\niWrite target 5\niApply -together\n')
                self.run_tool('--icl', 'source.icl', '--pdl', 'source.pdl', '--output', 'result.pdl')
                text = (self.directory / 'result.pdl').read_text()
                final = text.split('iApply')[1]
                self.assertIn('iWrite control 0b01\n', final)
                self.assertIn('iWrite target 0b0101\n', final)
                self.assertIn('iApply -together', text)
                self.run_tool('--icl', 'source.icl', '--pdl', 'result.pdl', '--output', 'again.pdl')
                self.assertEqual(text, (self.directory / 'again.pdl').read_text())

    def test_small_tree_matches_independent_search(self):
        (self.directory / 'source.icl').write_text(padded('''
ScanRegister outer { ScanInSource si; }
ScanRegister inner { ScanInSource outer; }
ScanRegister target[3:0] { ScanInSource inner; }
ScanMux inner_path SelectedBy inner { 1'b0: inner; 1'b1: target[0]; }
ScanMux outer_path SelectedBy outer { 1'b0: outer; 1'b1: inner_path; }
ScanOutPort so { Source outer_path; }
'''))
        for outer in (0, 1):
            for inner in (0, 1):
                with self.subTest(outer=outer, inner=inner):
                    frontier = {(outer, inner, False): 0}
                    minimum = None
                    for scans in range(1, 5):
                        following = {}
                        for (a, b, touched), cost in frontier.items():
                            for next_a in (0, 1):
                                for next_b in (0, 1):
                                    if not a and next_b != b:
                                        continue
                                    key = (next_a, next_b, touched or bool(a and b))
                                    cycles = cost + (6 if a and b else 2 if a else 1)
                                    following[key] = min(following.get(key, cycles), cycles)
                        complete = [cost for (_, _, touched), cost in following.items() if touched]
                        if complete:
                            minimum = scans, min(complete)
                            break
                        frontier = following
                    self.assertIsNotNone(minimum)
                    (self.directory / 'source.pdl').write_text(
                        f'iState outer {outer}\niState inner {inner}\niWrite target 5\niApply\n')
                    self.run_tool('--icl', 'source.icl', '--pdl', 'source.pdl', '--output', 'result.pdl')
                    text = (self.directory / 'result.pdl').read_text()
                    shifted = sum(len(line.split()[2]) - 2 for line in text.splitlines()
                                  if line.startswith('iWrite '))
                    self.assertEqual((text.count('iApply\n'), shifted), minimum)

    def test_exclusive_routes_fall_back(self):
        (self.directory / 'source.icl').write_text(padded('''
ScanRegister sel { ScanInSource si; ResetValue 1'b0; }
ScanRegister left[3:0] { ScanInSource sel; ResetValue 4'b0; }
ScanRegister right[3:0] { ScanInSource sel; ResetValue 4'b0; }
ScanMux route SelectedBy sel { 1'b0: left[0]; 1'b1: right[0]; }
ScanOutPort so { Source route; }
'''))
        (self.directory / 'source.pdl').write_text('iReset\niWrite left 5\niWrite right 10\niApply\n')
        self.run_tool('--icl', 'source.icl', '--pdl', 'source.pdl', '--output', 'result.pdl')
        text = (self.directory / 'result.pdl').read_text()
        self.assertEqual(text.count('iApply\n'), 2)
        self.assertIn('iWrite left 0b0101\n', text)
        self.assertIn('iWrite right 0b1010\n', text)
        self.run_tool('--icl', 'source.icl', '--pdl', 'result.pdl', '--output', 'again.pdl')
        self.assertEqual(text, (self.directory / 'again.pdl').read_text())

    def test_access_together_alias(self):
        source = (FIXTURE / 'wide.icl').read_text().rsplit('}', 1)[0]
        source += ' Alias pair[15:0] = target0, target1 { AccessTogether; }\n}\n'
        (self.directory / 'source.icl').write_text(source)
        (self.directory / 'source.pdl').write_text('iReset\niWrite pair[15:8] 0xa5\niApply\n')
        self.run_tool('--icl', 'source.icl', '--pdl', 'source.pdl', '--output', 'result.pdl')
        text = (self.directory / 'result.pdl').read_text()
        final = text.split('iApply')[1]
        self.assertIn('iWrite target0 0b10100101\n', final)
        self.assertIn('iWrite target1 0b00000000\n', final)
        self.assertIn('iApply -together', text)
        self.assertEqual(text.count('iApply'), 2)

    def test_tap_select_dependencies(self):
        source = '''Module SelectedLeaf {
 ScanInPort si; SelectPort select; CaptureEnPort capture;
 ShiftEnPort shift; UpdateEnPort update; TCKPort clock;
 ScanRegister value[7:0] { ScanInSource si; ResetValue 8'b0; }
 ScanOutPort so { Source value[0]; }
 ScanInterface client {
  Port si; Port so; Port select; Port capture; Port shift; Port update; Port clock;
 }
}
Module SelectedNetwork {
 ScanInPort si; SelectPort sel; CaptureEnPort capture;
 ShiftEnPort shift; UpdateEnPort update; TCKPort clock;
'''
        for i in range(16):
            previous = f'path{i - 1}' if i else 'si'
            source += f''' ScanRegister gate{i} {{ ScanInSource {previous}; ResetValue 1'b0; }}
 LogicSignal enabled{i} {{ sel & gate{i}; }}
 Instance target{i} Of SelectedLeaf {{
  InputPort si = gate{i}; InputPort select = enabled{i};
  InputPort capture = capture; InputPort shift = shift;
  InputPort update = update; InputPort clock = clock;
 }}
 ScanMux path{i} SelectedBy gate{i} {{ 1'b0: gate{i}; 1'b1: target{i}.so; }}
'''
        source += ''' ScanOutPort so { Source path15; }
 ScanInterface client {
  Port si; Port so; Port sel; Port capture; Port shift; Port update; Port clock;
 }
}
Module SelectedTop {
 Instance core Of SelectedNetwork;
 AccessLink tap Of STD_1149_1_2001 {
  BSDLEntity AccessProbe;
  IJTAG_ON { ScanInterface { core.client; } }
  IJTAG_OFF { ScanInterface { core.client; } }
 }
}
'''
        (self.directory / 'source.icl').write_text(source)
        (self.directory / 'source.pdl').write_text('iReset\n' + ''.join(
            f'iWrite core.target{i}.value 0xa5\n' for i in range(16)) + 'iApply -together\n')
        inputs = ('--icl', 'source.icl', '--bsdl', 'AccessProbe',
                  FIXTURE.parent / 'access_link/simple.bsdl')
        self.run_tool(*inputs, '--pdl', 'source.pdl', '--output', 'result.pdl', '--debug')
        result = (self.directory / 'result.pdl').read_text()
        self.assertEqual(result.count('iApply'), 3)
        self.assertIn('iWrite tap.INSTRUCTION', result.split('iApply')[0])
        self.assertEqual(result.count('iApply -together'), 1)
        for i in range(16):
            self.assertIn(f'iWrite core.target{i}.value 0b10100101\n', result)
            self.assertIn(f'iWrite core.gate{i} 0b1\n', result.split('iApply')[2])
        self.run_tool(*inputs, '--pdl', 'result.pdl', '--output', 'again.pdl')
        self.assertEqual(result, (self.directory / 'again.pdl').read_text())
        self.run_tool('--icl', 'debug/SelectedTop.icl', '--pdl', 'source.pdl',
                      '--output', 'flat.pdl')
        self.assertEqual(result, (self.directory / 'flat.pdl').read_text())
        source_pdl = (self.directory / 'source.pdl').read_text().replace(
            'iReset\n', 'iReset\niTake core.gate0\niState core.gate0 0\niWrite core.gate0 1\n')
        (self.directory / 'source.pdl').write_text(source_pdl)
        self.run_tool(*inputs, '--pdl', 'source.pdl', '--output', 'protected.pdl')
        protected = (self.directory / 'protected.pdl').read_text()
        self.assertEqual(protected.count('iApply'), 3)
        self.assertEqual([line for line in protected.splitlines()
                          if line.startswith('iWrite core.gate0 ')],
                         ['iWrite core.gate0 0b1'] * 2)
        self.run_tool(*inputs, '--pdl', 'protected.pdl', '--output', 'again.pdl')
        self.assertEqual(protected, (self.directory / 'again.pdl').read_text())


if __name__ == '__main__':
    unittest.main()
