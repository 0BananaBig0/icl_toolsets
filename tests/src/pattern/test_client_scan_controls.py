#!/usr/bin/env python3
"""Self-constructed standalone client interfaces retain real control events."""
import pathlib
import subprocess
import sys
import tempfile
import time
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[3]
BIN = pathlib.Path(sys.argv.pop(1)).resolve() if len(sys.argv) > 1 else ROOT / 'build/icl-pattern-generation'
PORTS = '''ScanInPort si; SelectPort sel; CaptureEnPort ce;
  ShiftEnPort se; UpdateEnPort ue; TCKPort ck;'''
INTERFACE = '''ScanInterface client {
    Port si; Port so; Port sel; Port ce; Port se; Port ue; Port ck;
  }'''
NETWORK = f'''Module ClientTop {{
  {PORTS}
  ScanRegister value[3:0] {{ ScanInSource si; ResetValue 4'b0; }}
  ScanOutPort so {{ Source value[0]; }}
  {INTERFACE}
}}
'''
PDL = '''iPDLLevel 1
iProcsForModule ClientTop
iReset
iWrite value 9; iApply
iRead value 9; iWrite value 6; iApply
iRead value 6; iApply
'''


class ClientScanControls(unittest.TestCase):
    def setUp(self):
        temporary = tempfile.TemporaryDirectory()
        self.addCleanup(temporary.cleanup)
        self.root = pathlib.Path(temporary.name)

    def run_tool(self, icl, pdl, output='result.pdl', extra=(), ok=True):
        result = subprocess.run([str(BIN), '--icl', str(icl), '--pdl', str(pdl),
                                 '--top_module', 'ClientTop', '--output', str(self.root / output),
                                 '--debug', *map(str, extra)], text=True, capture_output=True, timeout=45)
        self.assertEqual(result.returncode == 0, ok, result.stdout + result.stderr)
        return (self.root / output).read_bytes() if ok else result.stdout + result.stderr

    def replay(self, source, pdl=PDL, reset=False):
        icl, commands = self.root / 'source.icl', self.root / 'source.pdl'
        icl.write_text(source)
        commands.write_text(pdl)
        extra = ('--local_reset', self.root / 'reset.pdl') if reset else ()
        original = self.run_tool(icl, commands, extra=extra)
        flat = self.root / 'debug/ClientTop.icl'
        self.assertEqual(original, self.run_tool(flat, commands, 'flat/result.pdl'))
        self.assertEqual(original, self.run_tool(icl, self.root / 'result.pdl', 'replay/result.pdl'))
        if reset:
            self.assertEqual(original, self.run_tool(icl, self.root / 'reset.pdl', 'reset/result.pdl'))
        return original

    def nested(self, connection='sel'):
        child = NETWORK.replace('ClientTop', 'Child')
        parent = f'''Module ClientTop {{
  {PORTS}
  DataInPort enable;
  Instance child Of Child {{
    InputPort si = si; InputPort sel = {connection};
    InputPort ce = ce; InputPort se = se; InputPort ue = ue; InputPort ck = ck;
  }}
  ScanOutPort so {{ Source child.so; }}
  {INTERFACE}
}}
'''
        return child + parent

    def idle_network(self):
        source = self.nested("1'b1")
        source = source.replace('ScanInSource si; ResetValue',
                                'ScanInSource si; CaptureSource value[2:0], value[3]; ResetValue')
        source = source.replace('InputPort ce = ce;', "InputPort ce = 1'b1;")
        source = source.replace('InputPort se = se;', "InputPort se = 1'b0;")
        source = source.replace('InputPort ue = ue;', "InputPort ue = 1'b1;")
        source = source.replace('ScanOutPort so { Source value[0]; }',
                                'ScanOutPort so { Source value[0]; } DataOutPort q[3:0] { Source value; }')
        source = source.replace('ScanOutPort so { Source child.so; }',
                                'ScanOutPort so { Source child.so; } DataOutPort observed[3:0] { Source child.q; }')
        return source

    def test_idle_without_tap_or_implicit_reset(self):
        for source in (NETWORK, NETWORK.replace(INTERFACE, ''),
                       NETWORK.replace(INTERFACE, '').replace(PORTS, 'ScanInPort si;')):
            with self.subTest(source=source):
                body = ('iPDLLevel 1; iRunLoop 2; '
                        'iState value 9 -LastReadValue; iRunLoop 0x100000000 -tck; '
                        'iNote [iGetReadData value -dec];\n')
                output = self.replay(source, body)
                self.assertIn(b'iRunLoop 2 -tck', output)
                self.assertIn(b'iRunLoop 4294967296 -tck', output)
                self.assertIn(b'iNote 9', output)
                self.assertNotIn(b'iReset', output)
                self.assertNotIn(b'iApply', output)

    def test_idle_actual_child_capture_and_update(self):
        body = ('iReset; iState child.value 8; iRunLoop 1; '
                'iRead observed 1; iApply; iRunLoop 2; iRead observed 4; iApply;\n')
        output = self.replay(self.idle_network(), body)
        self.assertEqual(output.count(b'iRunLoop'), 2)
        self.assertIn(b'iRead observed 0b0001', output)
        self.assertIn(b'iRead observed 0b0100', output)

    def test_idle_known_and_unknown_clock_gates(self):
        # Explicit ToTCKPort Source is the existing lowered-model gate extension.
        source = self.idle_network().replace('DataInPort enable;', '''DataInPort enable;
  ClockMux gated SelectedBy enable { 1'b0: 1'b0; 1'b1: ck; }
  ToTCKPort gated_tck { Source gated; }''')
        source = source.replace('InputPort ck = ck;', 'InputPort ck = gated_tck;')
        body = ('iReset; iState enable 0; iState child.value 8; iRunLoop 2; '
                'iRead observed 8; iApply; iState enable 1; iRunLoop 1; '
                'iRead observed 1; iApply;\n')
        self.replay(source, body)
        self.replay(source, 'iReset; iState enable 0bX; iState child.value 8; '
                    'iRunLoop 1;\n')

    def test_idle_failures_preserve_pattern_and_debug(self):
        # The explicit ToTCK Source extension retains the backend phase check.
        inverted = self.idle_network().replace(
            'DataInPort enable;',
            'DataInPort enable; ToTCKPort inverted_tck { Source ~ck; }')
        inverted = inverted.replace('InputPort ck = ck;', 'InputPort ck = inverted_tck;')
        cases = (
            (self.idle_network(),
             'iReset; iTake child.value; iState child.value 8; iRunLoop 1;',
             'reserved'),
            (inverted,
             'iReset; iState child.value 8; iRunLoop 1;', 'inverted'),
            (self.idle_network(),
             'iReset; iState child.value 8; iRunLoop 18446744073709551615;',
             'resource limit'),
        )
        for source, body, message in cases:
            with self.subTest(message=message):
                icl, pdl = self.root / 'source.icl', self.root / 'source.pdl'
                icl.write_text(source)
                pdl.write_text(body)
                output, debug = self.root / 'result.pdl', self.root / 'debug/ClientTop.icl'
                debug.parent.mkdir(exist_ok=True)
                output.write_bytes(b'previous pattern\n')
                debug.write_bytes(b'previous debug\n')
                error = self.run_tool(icl, pdl, ok=False)
                self.assertIn(message, error)
                self.assertEqual(output.read_bytes(), b'previous pattern\n')
                self.assertEqual(debug.read_bytes(), b'previous debug\n')

    def test_idle_implicit_selection_guard_and_masked_controls(self):
        source = self.idle_network().replace('SelectPort sel;', '', 1)
        source = source.replace('Port sel;', '', 1)
        source = source.replace("InputPort sel = 1'b1;", '')
        extra = (' SelectPort other;\n ScanInterface second { Port si; Port so; '
                 'Port other; Port ce; Port se; Port ue; Port ck; }\n')
        source = source[:source.rfind('}')] + extra + '}\n'
        icl, pdl = self.root / 'source.icl', self.root / 'source.pdl'
        icl.write_text(source)
        body = ('iReset; iState sel 1; iState other 0; iState child.value 8; '
                'iRunLoop 1;')
        pdl.write_text(body)
        (self.root / 'result.pdl').write_bytes(b'previous pattern\n')
        error = self.run_tool(icl, pdl, ok=False)
        self.assertIn('implicit selection', error)
        self.assertEqual((self.root / 'result.pdl').read_bytes(), b'previous pattern\n')
        masked = source.replace("InputPort ce = 1'b1;", "InputPort ce = 1'b0;")
        masked = masked.replace("InputPort ue = 1'b1;", "InputPort ue = 1'b0;")
        self.replay(masked, body + ' iRead observed 8; iApply;\n')

    def test_idle_reset_source_replay(self):
        self.replay(NETWORK, PDL.replace('iRead value 9;', 'iRunLoop 3; iRead value 9;'),
                    reset=True)

    def test_idle_vendor_fixture_replay(self):
        case = ROOT / 'tests/cases/ours/pattern_generation/client_controls'
        output = self.replay((case / 'network.icl').read_text(),
                             (case / 'idle_access.pdl').read_text())
        self.assertEqual(output.count(b'iRunLoop'), 2)
        self.assertEqual(output.count(b'iApply'), 3)

    def test_idle_stable_scale(self):
        source = NETWORK.replace('value[3:0]', 'value[65535:0]').replace("4'b0", "0")
        body = 'iRunLoop 18446744073709551615;\n'
        start = time.monotonic()
        out = self.replay(source, body)
        self.assertIn(b'iRunLoop 18446744073709551615 -tck', out)
        print(f'standalone idle: 65536 storage bits, UINT64_MAX cycles, '
              f'three replays {time.monotonic() - start:.3f}s')

    def test_explicit_and_inferred_clients(self):
        for source in (NETWORK, NETWORK.replace(INTERFACE, '')):
            output = self.replay(source, reset=True)
            self.assertIn(b'iWrite value 0b1001', output)
            self.assertIn(b'iRead value 0b0110', output)

    def test_nested_controls_and_primary_select(self):
        self.replay(self.nested(), PDL.replace(' value ', ' child.value '))
        body = PDL.replace(' value ', ' child.value ').replace('iReset', 'iReset\niWrite enable 1; iApply')
        self.replay(self.nested('enable'), body)

    def test_indexed_client_control_lanes(self):
        source = NETWORK
        for port in ('sel', 'ce', 'se', 'ue', 'ck'):
            source = source.replace(f' {port};', f' {port}[5:2];', 1)
            source = source.replace(f'Port {port};', f'Port {port}[3];')
        self.replay(source)

    def test_scan_select_dependency(self):
        source = self.nested('gate')
        source = source.replace('DataInPort enable;',
                                "ScanRegister gate { ScanInSource mux; ResetValue 1'b0; }")
        source = source.replace('ScanOutPort so { Source child.so; }', '''
  ScanMux mux SelectedBy gate { 1'b0: si; 1'b1: child.so; }
  ScanOutPort so { Source gate; }''')
        self.replay(source, PDL.replace(' value ', ' child.value '))

    def test_inverted_select_and_gated_clock(self):
        # Explicit ToTCKPort Source is the existing lowered-model gate extension.
        body = PDL.replace(' value ', ' child.value ')
        source = self.nested('inverse').replace('DataInPort enable;',
                    'DataInPort enable; LogicSignal inverse { ~enable; }')
        self.replay(source, body.replace('iReset', 'iReset\niWrite enable 0; iApply'))
        source = self.nested().replace('DataInPort enable;', '''DataInPort enable;
  ClockMux gated SelectedBy enable { 1'b0: 1'b0; 1'b1: ck; }
  ToTCKPort gated_tck { Source gated; }''')
        source = source.replace('InputPort ck = ck;', 'InputPort ck = gated_tck;')
        self.replay(source, body.replace('iReset', 'iReset\niWrite enable 1; iApply'))

    def test_local_reset_release_with_explicit_controls(self):
        source = self.nested().replace('Module Child {', 'Module Child { ResetPort rst;')
        source = source.replace('DataInPort enable;',
                                "ResetPort global; ScanRegister reset { ScanInSource si; ResetValue 1'b1; } "
                                "DataMux reset_gate SelectedBy reset { 1'b0: global; 1'b1: 1'b1; }")
        source = source.replace('InputPort si = si;', 'InputPort si = reset; InputPort rst = reset_gate;')
        output = self.replay(source, PDL.replace(' value ', ' child.value '), reset=True)
        self.assertIn(b'iWrite reset 0b0', output)
        self.assertIn(b'iWrite reset 0b0', (self.root / 'reset.pdl').read_bytes())

    def test_parallel_frames_do_not_pulse_client_controls(self):
        # IEEE 6.3.12(d), 6.4.6.10: the mux also drives an update-control output.
        source = NETWORK.replace('Module ClientTop {', '''Module ClientTop {
  DataInPort data;
  DataMux write_enable SelectedBy 1'b0 { 1'b0: ue; 1'b1: 1'b0; }
  ToUpdateEnPort forwarded_update { Source write_enable; }
  DataRegister d { WriteDataSource data; WriteEnSource write_enable; ResetValue 1'b0; }
  DataOutPort observed { Source d; }''')
        self.replay(source, 'iPDLLevel 1\niProcsForModule ClientTop\niReset\n'
                    'iState data 0; iWrite value 9; iApply; iWrite data 1; iApply; '
                    'iRead observed 0; iApply\n')

    def test_controls_and_output_preservation(self):
        source = self.nested()
        for role in ('sel', 'ce', 'se', 'ue', 'ck'):
            for bit in ('0', 'x'):
                with self.subTest(role=role, bit=bit):
                    controlled = source.replace(f'InputPort {role} = {role};',
                                                f"InputPort {role} = 1'b{bit};")
                    if role == 'ce' and bit == '0':
                        self.replay(controlled, PDL.replace(' value ', ' child.value '))
                        continue
                    icl, commands = self.root / 'source.icl', self.root / 'source.pdl'
                    icl.write_text(controlled)
                    commands.write_text(PDL.replace(' value ', ' child.value '))
                    (self.root / 'result.pdl').write_bytes(b'previous result\n')
                    self.run_tool(icl, commands, ok=False)
                    self.assertEqual((self.root / 'result.pdl').read_bytes(), b'previous result\n')

    def test_reserved_primary_select_cannot_be_changed(self):
        source = self.nested('enable')
        icl, commands = self.root / 'source.icl', self.root / 'source.pdl'
        icl.write_text(source)
        commands.write_text('iPDLLevel 1\niProcsForModule ClientTop\niReset\n'
                            'iTake enable; iWrite enable 0; iApply; iWrite child.value 9; iApply\n')
        (self.root / 'result.pdl').write_bytes(b'previous result\n')
        self.run_tool(icl, commands, ok=False)
        self.assertEqual((self.root / 'result.pdl').read_bytes(), b'previous result\n')

    def test_controlled_chain_scale(self):
        registers = []
        previous = 'si'
        for i in range(64):
            registers.append(f"ScanRegister r{i} {{ ScanInSource {previous}; ResetValue 1'b0; }}")
            previous = f'r{i}'
        source = f'Module ClientTop {{ {PORTS}\n' + '\n'.join(registers)
        source += f'\nScanOutPort so {{ Source {previous}; }}\n{INTERFACE}\n}}\n'
        pdl = 'iPDLLevel 0\niProcsForModule ClientTop\niReset\n'
        pdl += '\n'.join(f'iWrite r{i} {i % 2}' for i in range(64)) + '\niApply\n'
        start = time.monotonic()
        self.replay(source, pdl)
        print(f'client controls: 64 registers, {len(source)} bytes, three replays '
              f'{time.monotonic() - start:.3f}s')


if __name__ == '__main__':
    unittest.main()
