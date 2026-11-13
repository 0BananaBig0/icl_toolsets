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
        body = PDL.replace(' value ', ' child.value ')
        source = self.nested('inverse').replace('DataInPort enable;',
                    'DataInPort enable; LogicSignal inverse { ~enable; }')
        self.replay(source, body.replace('iReset', 'iReset\niWrite enable 0; iApply'))
        source = self.nested().replace('DataInPort enable;', '''DataInPort enable;
  ClockMux gated SelectedBy enable { 1'b0: 1'b0; 1'b1: ck; }''')
        source = source.replace('InputPort ck = ck;', 'InputPort ck = gated;')
        self.replay(source, body.replace('iReset', 'iReset\niWrite enable 1; iApply'))

    def test_local_reset_release_with_explicit_controls(self):
        source = self.nested().replace('Module Child {', 'Module Child { ResetPort rst;')
        source = source.replace('DataInPort enable;',
                                "ScanRegister reset { ScanInSource si; ResetValue 1'b1; }")
        source = source.replace('InputPort si = si;', 'InputPort si = reset; InputPort rst = reset;')
        output = self.replay(source, PDL.replace(' value ', ' child.value '), reset=True)
        self.assertIn(b'iWrite reset 0b0', output)
        self.assertIn(b'iWrite reset 0b0', (self.root / 'reset.pdl').read_bytes())

    def test_parallel_frames_do_not_pulse_client_controls(self):
        # Synthetic state-model regression; Tessent rejects ue as WriteEnSource.
        source = NETWORK.replace('Module ClientTop {', '''Module ClientTop {
  DataInPort data;
  DataRegister d { WriteDataSource data; WriteEnSource ue; ResetValue 1'b0; }
  DataOutPort observed { Source d; }''')
        self.replay(source, 'iPDLLevel 1\niProcsForModule ClientTop\niReset\n'
                    'iState data 0; iWrite value 9; iApply; iWrite data 1; iApply; '
                    'iRead observed 0; iApply\n')

    def test_blocked_controls_preserve_output(self):
        source = self.nested()
        for role in ('sel', 'ce', 'se', 'ue', 'ck'):
            for bit in ('0', 'x'):
                with self.subTest(role=role, bit=bit):
                    icl, commands = self.root / 'source.icl', self.root / 'source.pdl'
                    icl.write_text(source.replace(f'InputPort {role} = {role};',
                                                 f"InputPort {role} = 1'b{bit};"))
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
