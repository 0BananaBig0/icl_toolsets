#!/usr/bin/env python3
"""Generator CLI and original/flat/generated replay contracts."""
import os
import json
import pathlib
import re
import subprocess
import sys
import tempfile
import unittest

BIN = pathlib.Path(sys.argv.pop(1)).resolve() if len(sys.argv) > 1 else pathlib.Path("build/icl-pattern-generation").resolve()
ICL = """Module Network {
  ScanInPort si;
  ScanRegister gate { ScanInSource si; }
  ScanRegister target[3:0] { ScanInSource gate; }
  ScanMux route SelectedBy gate { 1'b0: gate; 1'b1: target[0]; }
  ScanOutPort so { Source route; }
}
"""
PDL = """iPDLLevel 1
iProcsForModule Network
iState gate 0
iProc write_value {value} { iWrite target $value; iApply }
iCall write_value 10
"""


class Generator(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = pathlib.Path(self.temp.name)
        (self.root / "source.icl").write_text(ICL)
        (self.root / "source.pdl").write_text(PDL)

    def run_tool(self, *arguments, ok=True):
        result = subprocess.run([str(BIN), *map(str, arguments)], cwd=self.root,
                                text=True, capture_output=True, timeout=30)
        self.assertEqual(result.returncode == 0, ok, result.stdout + result.stderr)
        return result

    def test_help_version(self):
        self.assertIn("--icl", self.run_tool("--help").stdout)
        self.assertIn("icl-pattern-generation", self.run_tool("--version").stdout)

    def test_non_data_sources_preserve_outputs(self):
        (self.root / "debug").mkdir()
        outputs = ("result.pdl", "reset.pdl", "debug/Network.icl")
        (self.root / "source.pdl").write_text("iProcsForModule Network\n")
        for source in ("clock", "wrapped"):
            with self.subTest(source=source):
                text = ("Module Network { TCKPort clock; Alias wrapped = clock; "
                    f"DataOutPort observed {{ Source {source}; }} }}\n")
                (self.root / "source.icl").write_text(text)
                for name in outputs:
                    (self.root / name).write_text("previous artifact\n")
                result = self.run_tool("--icl", "source.icl", "--pdl", "source.pdl",
                                       "--output", "result.pdl", "--debug",
                                       "--local_reset", "reset.pdl", ok=False)
                self.assertIn("data signal", result.stdout + result.stderr)
                self.assertIn("source.icl:", result.stdout + result.stderr)
                for name in outputs:
                    self.assertEqual((self.root / name).read_text(), "previous artifact\n")

    def test_flat_data_source_kinds_are_checked(self):
        text = ("Module Network { TCKPort clock; DataInPort data; "
            "DataOutPort observed { Source data; } }\n")
        (self.root / "source.icl").write_text(text)
        (self.root / "source.pdl").write_text(
            "iProcsForModule Network\niWrite data 1\niApply\niRead observed 1\niApply\n")
        self.run_tool("--icl", "source.icl", "--pdl", "source.pdl",
                      "--output", "result.pdl", "--debug")
        flat = (self.root / "debug/Network.icl").read_text()
        original = (self.root / "result.pdl").read_bytes()
        self.run_tool("--icl", "debug/Network.icl", "--pdl", "result.pdl",
                      "--output", "again.pdl")
        self.assertEqual((self.root / "again.pdl").read_bytes(), original)
        self.assertIn("Source n64617461;", flat)
        (self.root / "invalid.icl").write_text(
            flat.replace("Source n64617461;", "Source n636c6f636b;"))
        result = self.run_tool("--icl", "invalid.icl", "--pdl", "source.pdl",
                               "--output", "result.pdl", ok=False)
        self.assertIn("data signal", result.stdout + result.stderr)
        self.assertEqual((self.root / "result.pdl").read_bytes(), original)

    def test_clock_source_kinds_preserve_outputs(self):
        (self.root / "source.pdl").write_text("iProcsForModule Network\n")
        for source in ("bad", "wrapped"):
            (self.root / "source.icl").write_text(
                "Module Network { ResetPort bad; Alias wrapped = bad; "
                f"ToClockPort out {{ Source {source}; }} }}")
            (self.root / "result.pdl").write_text("previous artifact\n")
            result = self.run_tool("--icl", "source.icl", "--pdl", "source.pdl",
                                   "--output", "result.pdl", ok=False)
            self.assertIn("clock or data signal", result.stdout + result.stderr)
            self.assertEqual((self.root / "result.pdl").read_text(), "previous artifact\n")

    def test_flat_clock_sources_and_differential_references(self):
        (self.root / "source.icl").write_text(
            "Module Child { ClockPort positive; ClockPort negative { DifferentialInvOf positive; } } "
            "Module Network { ClockPort root; ResetPort bad; Instance child Of Child { InputPort positive = root; } "
            "ToClockPort out { Source root; } }")
        (self.root / "source.pdl").write_text("iProcsForModule Network\n")
        self.run_tool("--icl", "source.icl", "--pdl", "source.pdl",
                      "--output", "result.pdl", "--debug")
        original = (self.root / "result.pdl").read_bytes()
        self.run_tool("--icl", "debug/Network.icl", "--pdl", "source.pdl",
                      "--output", "again.pdl")
        self.assertEqual((self.root / "again.pdl").read_bytes(), original)
        flat = (self.root / "debug/Network.icl").read_text()
        self.assertIn("Source n726f6f74;", flat)
        (self.root / "invalid.icl").write_text(
            flat.replace("Source n726f6f74;", "Source n626164;"))
        result = self.run_tool("--icl", "invalid.icl", "--pdl", "source.pdl",
                               "--output", "result.pdl", ok=False)
        self.assertIn("clock or data signal", result.stdout + result.stderr)
        self.assertEqual((self.root / "result.pdl").read_bytes(), original)

    def test_differential_binding_metadata_validation(self):
        (self.root / "source.icl").write_text('''Module Leaf { DataOutPort data { Source 1'b0; Enable 1'b1; } }
Module Mid { Instance leaf Of Leaf; }
Module Child { ClockPort p; ClockPort n { DifferentialInvOf p; } }
Module Network { ClockPort root; Instance provider Of Leaf;
OneHotDataGroup group { Port provider.data; }
Instance mid Of Mid; Alias deep = mid.leaf.data;
Instance child Of Child { InputPort p = root; InputPort n = group; }
}''')
        (self.root / "source.pdl").write_text("iProcsForModule Network\n")
        self.run_tool("--icl", "source.icl", "--pdl", "source.pdl",
                      "--output", "result.pdl", "--debug")
        flat = (self.root / "debug/Network.icl").read_text()
        self.run_tool("--icl", "debug/Network.icl", "--pdl", "source.pdl",
                      "--output", "again.pdl")
        match = re.search(r'Attribute icl_pattern_origins_v1 = (".*");', flat)
        self.assertIsNotNone(match)
        for conflict in (True, False):
            metadata = json.loads(json.loads(match.group(1)))
            node = next(n for n in metadata['nodes'] if n[1] == 'child.n')
            if conflict:
                node[4] = True
                node[8] = ['x']
                diagnostic = 'invalid flat differential binding'
            else:
                node[8] = [['n64656570', '0', False, 0]]
                diagnostic = 'only available to PDL'
            quoted = json.dumps(json.dumps(metadata, separators=(',', ':')))
            (self.root / 'invalid.icl').write_text(
                flat[:match.start(1)] + quoted + flat[match.end(1):])
            result = self.run_tool('--icl', 'invalid.icl', '--pdl', 'source.pdl',
                                   '--output', 'invalid.pdl', ok=False)
            self.assertIn(diagnostic, result.stdout + result.stderr)

    def test_tck_input_source_kinds(self):
        (self.root / 'source.pdl').write_text('iProcsForModule Network\n')
        for declaration in ('ClockPort source;', 'ToClockPort source;',
                            "ClockPort ck; ClockMux source SelectedBy 1'b0 { 1'b0: ck; }",
                            'ResetPort source;', 'ScanInPort source;'):
            for reference in ('source', 'wrapped'):
                (self.root / 'source.icl').write_text(
                    'Module Child { TCKPort ck; } Module Network { ' + declaration +
                    ' Alias wrapped = source; Instance child Of Child { InputPort ck = ' + reference + '; } }')
                (self.root / 'result.pdl').write_text('previous artifact\n')
                result = self.run_tool('--icl', 'source.icl', '--pdl', 'source.pdl',
                                       '--output', 'result.pdl', ok=False)
                self.assertIn('TCK or data signal', result.stdout + result.stderr)
                self.assertEqual((self.root / 'result.pdl').read_text(), 'previous artifact\n')
        for declaration in ('TCKPort source;', 'ToTCKPort source;',
                            'DataInPort source;', "LogicSignal source { 1'b0; }"):
            (self.root / 'source.icl').write_text(
                'Module Child { TCKPort ck; } Module Network { ' + declaration +
                ' Alias wrapped = source; Instance child Of Child { InputPort ck = wrapped; } }')
            self.run_tool('--icl', 'source.icl', '--pdl', 'source.pdl',
                          '--output', 'result.pdl', '--debug')
            self.run_tool('--icl', 'debug/Network.icl', '--pdl', 'source.pdl',
                          '--output', 'again.pdl')

    def test_reset_source_kinds(self):
        (self.root / 'source.pdl').write_text('iProcsForModule Network\n')
        for declaration in ('TCKPort source;', 'ClockPort source;',
                            'CaptureEnPort source;', 'ShiftEnPort source;',
                            'UpdateEnPort source;', 'TMSPort source;', 'ScanInPort source;'):
            for consumer in ('ToResetPort out { Source wrapped; }',
                             'Instance child Of Child { InputPort reset = wrapped; }'):
                (self.root / 'source.icl').write_text(
                    'Module Child { ResetPort reset; } Module Network { ' + declaration +
                    ' Alias wrapped = source; ' + consumer + ' }')
                (self.root / 'result.pdl').write_text('previous artifact\n')
                result = self.run_tool('--icl', 'source.icl', '--pdl', 'source.pdl',
                                       '--output', 'result.pdl', '--top_module', 'Network', ok=False)
                self.assertIn('reset or data signal', result.stdout + result.stderr)
                self.assertEqual((self.root / 'result.pdl').read_text(), 'previous artifact\n')

    def test_scan_control_source_kinds(self):
        (self.root / 'source.pdl').write_text('iProcsForModule Network\n')
        for family, diagnostic in [('CaptureEn', 'capture-enable'),
                                   ('ShiftEn', 'shift-enable'), ('UpdateEn', 'update-enable')]:
            for source_kind in ('TCK', 'Clock', 'Reset', 'ScanIn', 'CaptureEn', 'ShiftEn', 'UpdateEn'):
                if source_kind == family:
                    continue
                for consumer in (f'To{family}Port out {{ Source wrapped; }}',
                                 'Instance child Of Child { InputPort ctl = wrapped; }'):
                    (self.root / 'source.icl').write_text(
                        f'Module Child {{ {family}Port ctl; }} Module Network {{ {source_kind}Port source; '
                        'Alias wrapped = source; ' + consumer + ' }')
                    (self.root / 'result.pdl').write_text('previous artifact\n')
                    result = self.run_tool('--icl', 'source.icl', '--pdl', 'source.pdl',
                                           '--top_module', 'Network', '--output', 'result.pdl', ok=False)
                    self.assertIn(diagnostic + ' or data signal', result.stdout + result.stderr)
                    self.assertEqual((self.root / 'result.pdl').read_text(), 'previous artifact\n')

    def test_tms_trst_source_kinds(self):
        (self.root / 'source.pdl').write_text('iProcsForModule Network\n')
        for family in ('TMS', 'TRST'):
            for source_kind in ('TCK', 'Clock', 'Reset', 'ScanIn', 'CaptureEn', 'ShiftEn', 'UpdateEn', 'TMS', 'TRST'):
                if source_kind == family:
                    continue
                for consumer in (f'To{family}Port out {{ Source wrapped; }}',
                                 'Instance child Of Child { InputPort ctl = wrapped; }'):
                    (self.root / 'source.icl').write_text(
                        f'Module Child {{ {family}Port ctl; }} Module Network {{ {source_kind}Port source; '
                        'Alias wrapped = source; ' + consumer + ' }')
                    (self.root / 'result.pdl').write_text('previous artifact\n')
                    result = self.run_tool('--icl', 'source.icl', '--pdl', 'source.pdl',
                                           '--top_module', 'Network', '--output', 'result.pdl', ok=False)
                    self.assertIn(family + ' or data signal', result.stdout + result.stderr)
                    self.assertEqual((self.root / 'result.pdl').read_text(), 'previous artifact\n')

    def test_concat_scan_source_kinds(self):
        (self.root / 'source.pdl').write_text('iProcsForModule Network\n')
        debug = self.root / 'debug/Network.icl'
        debug.parent.mkdir(exist_ok=True)
        artifacts = (self.root / 'result.pdl', self.root / 'reset.pdl', debug)
        for family in ('TCK', 'Clock', 'Reset', 'CaptureEn', 'ShiftEn', 'UpdateEn', 'TMS', 'TRST'):
            for consumer in ('ScanOutPort out { Source wrapped; }',
                             'Instance child Of Child { InputPort si = wrapped; }',
                             "ScanMux mux SelectedBy 1'b0 { 1'b0 : wrapped; }"):
                with self.subTest(family=family, consumer=consumer):
                    (self.root / 'source.icl').write_text(
                        'Module Child { ScanInPort si; ScanOutPort so { Source si; } } '
                        f'Module Network {{ {family}Port source; Alias wrapped = source; ' + consumer + ' }')
                    for artifact in artifacts:
                        artifact.write_text('previous artifact\n')
                    result = self.run_tool('--icl', 'source.icl', '--pdl', 'source.pdl',
                                           '--top_module', 'Network', '--output', 'result.pdl',
                                           '--local_reset', 'reset.pdl', '--debug', ok=False)
                    self.assertIn('scan or data signal', result.stdout + result.stderr)
                    for artifact in artifacts:
                        self.assertEqual(artifact.read_text(), 'previous artifact\n')

    def test_bare_scan_source_kinds(self):
        (self.root / 'source.pdl').write_text('iProcsForModule Network\n')
        debug = self.root / 'debug/Network.icl'
        debug.parent.mkdir(exist_ok=True)
        artifacts = (self.root / 'result.pdl', self.root / 'reset.pdl', debug)
        declarations = ['DataInPort source;', 'SelectPort source;',
                        "DataOutPort source { Source 1'b0; }",
                        "ToSelectPort source { Source 1'b0; }",
                        "DataRegister source { WriteDataSource 1'b0; WriteEnSource 1'b1; }",
                        "LogicSignal source { 1'b0; }",
                        "DataMux source SelectedBy 1'b0 { 1'b0 : 1'b0; }",
                        'Instance data Of Data; OneHotDataGroup source { Port data.out; }']
        declarations += [family + 'Port source;' for family in
                         ('TCK', 'Clock', 'Reset', 'CaptureEn', 'ShiftEn', 'UpdateEn', 'TMS', 'TRST')]
        for declaration in declarations:
            for reference in ('source', 'wrapped'):
                with self.subTest(declaration=declaration, reference=reference):
                    (self.root / 'source.icl').write_text(
                        "Module Data { DataOutPort out { Source 1'b0; Enable 1'b1; } } "
                        'Module Network { ' + declaration + ' Alias wrapped = source; '
                        'ScanRegister target { ScanInSource ' + reference + '; } '
                        'ScanOutPort so { Source target; } }')
                    for artifact in artifacts:
                        artifact.write_text('previous artifact\n')
                    result = self.run_tool('--icl', 'source.icl', '--pdl', 'source.pdl',
                                           '--top_module', 'Network', '--output', 'result.pdl',
                                           '--local_reset', 'reset.pdl', '--debug', ok=False)
                    self.assertIn('requires a scan signal', result.stdout + result.stderr)
                    for artifact in artifacts:
                        self.assertEqual(artifact.read_text(), 'previous artifact\n')

    def test_alias_scan_register_taps(self):
        (self.root / 'source.pdl').write_text('iProcsForModule Network\n')
        debug = self.root / 'debug/Network.icl'
        debug.parent.mkdir(exist_ok=True)
        artifacts = (self.root / 'result.pdl', self.root / 'reset.pdl', debug)
        for bounds, middle in (('[3:0]', 2), ('[0:3]', 1)):
            for alias in (f'Alias wrapped = reg[{middle}];',
                          f'Alias first = reg[{middle}]; Alias wrapped = ~first;',
                          f'Alias first[1:0] = reg[{middle}], si; Alias wrapped = first[1];'):
                for consumer in ('ScanOutPort so { Source wrapped; }',
                                 'ScanRegister target { ScanInSource wrapped; }'):
                    with self.subTest(bounds=bounds, alias=alias, consumer=consumer):
                        (self.root / 'source.icl').write_text(
                            'Module Network { ScanInPort si; ScanRegister reg' + bounds +
                            ' { ScanInSource si; } ' + alias + consumer + ' }')
                        for artifact in artifacts:
                            artifact.write_text('previous artifact\n')
                        result = self.run_tool('--icl', 'source.icl', '--pdl', 'source.pdl',
                                               '--top_module', 'Network', '--output', 'result.pdl',
                                               '--local_reset', 'reset.pdl', '--debug', ok=False)
                        self.assertIn('rightmost bit', result.stdout + result.stderr)
                        for artifact in artifacts:
                            self.assertEqual(artifact.read_text(), 'previous artifact\n')

    def test_address_port_source_kinds(self):
        (self.root / 'source.pdl').write_text('iProcsForModule Network\n')
        debug = self.root / 'debug/Network.icl'
        debug.parent.mkdir(exist_ok=True)
        artifacts = (self.root / 'result.pdl', self.root / 'reset.pdl', debug)
        for family in ('Address', 'WriteEn', 'ReadEn'):
            sources = [(kind, kind + 'Port source;') for kind in
                       ('Address', 'WriteEn', 'ReadEn', 'TCK', 'Clock', 'Reset',
                        'CaptureEn', 'ShiftEn', 'UpdateEn', 'TMS', 'TRST', 'ScanIn', 'Select')
                       if kind != family]
            sources += [('constant ' + value, '')
                        for value in ("1'b0", "1'b1", "1'bx")]
            sources += [('data register', "DataRegister source { WriteDataSource 1'b0; WriteEnSource 1'b1; }"),
                        ('logic', "LogicSignal source { 1'b0; }"),
                        ('data mux', "DataMux source SelectedBy 1'b0 { 1'b0 : 1'b0; }"),
                        ('select output', "ToSelectPort source { Source 1'b0; }"),
                        ('data group', 'Instance proxy Of Proxy; OneHotDataGroup source { Port proxy.out; }')]
            for source_kind, declaration in sources:
                with self.subTest(family=family, source_kind=source_kind):
                    reference = 'wrapped' if declaration else source_kind[len('constant '):]
                    alias = 'Alias wrapped = source;' if declaration else ''
                    (self.root / 'source.icl').write_text(
                        "Module Proxy { DataOutPort out { Source 1'b0; Enable 1'b1; } } "
                        f'Module Child {{ {family}Port ctl; }} Module Network {{ '
                        + declaration + alias + ' Instance child Of Child { InputPort ctl = '
                        + reference + '; } }')
                    for artifact in artifacts:
                        artifact.write_text('previous artifact\n')
                    result = self.run_tool('--icl', 'source.icl', '--pdl', 'source.pdl',
                                           '--top_module', 'Network', '--output', 'result.pdl',
                                           '--local_reset', 'reset.pdl', '--debug', ok=False)
                    self.assertIn(family + 'Port, DataInPort, DataOutPort or ScanRegister',
                                  result.stdout + result.stderr)
                    for artifact in artifacts:
                        self.assertEqual(artifact.read_text(), 'previous artifact\n')

    def test_direct_control_data_drivers_preserve_outputs(self):
        (self.root / 'source.pdl').write_text('iProcsForModule Network\n')
        debug = self.root / 'debug/Network.icl'
        debug.parent.mkdir(exist_ok=True)
        artifacts = (self.root / 'result.pdl', self.root / 'reset.pdl', debug)
        declarations = ('DataInPort src;', 'DataInPort data; LogicSignal src { data; }',
                        'DataRegister src { WriteDataSource 0; WriteEnSource 0; ResetValue 0; }')
        for family in ('Reset', 'TMS', 'TRST'):
            for output in (False, True):
                for declaration in declarations:
                    for signal in ('src', 'wrapped'):
                        with self.subTest(family=family, output=output, declaration=declaration, signal=signal):
                            child = '' if output else f'Module Child {{ {family}Port ctl; }} '
                            consumer = (f'To{family}Port out {{ Source {signal}; }}' if output else
                                        f'Instance child Of Child {{ InputPort ctl = {signal}; }}')
                            (self.root / 'source.icl').write_text(
                                child + 'Module Network { ' + declaration +
                                ' Alias wrapped = src; ' + consumer + ' }')
                            for artifact in artifacts:
                                artifact.write_text('previous artifact\n')
                            result = self.run_tool('--icl', 'source.icl', '--pdl', 'source.pdl',
                                                   '--top_module', 'Network', '--output', 'result.pdl',
                                                   '--local_reset', 'reset.pdl', '--debug', ok=False)
                            self.assertIn('requires a direct DataMux data driver', result.stdout + result.stderr)
                            for artifact in artifacts:
                                self.assertEqual(artifact.read_text(), 'previous artifact\n')

    def test_reset_mux_input_structure_preserves_outputs(self):
        (self.root / 'source.pdl').write_text('iProcsForModule Network\n')
        debug = self.root / 'debug/Network.icl'
        debug.parent.mkdir(exist_ok=True)
        artifacts = (self.root / 'result.pdl', self.root / 'reset.pdl', debug)
        for family in ('Reset', 'TMS', 'TRST'):
            for output in (False, True):
                for rows in ("2'd0 : d0; 2'd1 : d1;",
                             "2'd0 : 1'b0; 2'd1 : 1'b1;",
                             "2'd0 : ctl; 2'd1 : d0; 2'd2 : d1;"):
                    for signal in ('mux', 'wrapped'):
                        with self.subTest(family=family, output=output, rows=rows, signal=signal):
                            child = '' if output else f'Module Child {{ {family}Port ctl; }} '
                            consumer = (f'To{family}Port out {{ Source {signal}; }}' if output else
                                        f'Instance child Of Child {{ InputPort ctl = {signal}; }}')
                            (self.root / 'source.icl').write_text(
                                child + f'Module Network {{ {family}Port ctl; '
                                'DataInPort mode[1:0]; DataInPort d0; DataInPort d1; '
                                f'DataMux mux SelectedBy mode {{ {rows} }} '
                                'Alias wrapped = ~mux; ' + consumer + ' }')
                            for artifact in artifacts:
                                artifact.write_text('previous artifact\n')
                            result = self.run_tool('--icl', 'source.icl', '--pdl', 'source.pdl',
                                                   '--top_module', 'Network', '--output', 'result.pdl',
                                                   '--local_reset', 'reset.pdl', '--debug', ok=False)
                            self.assertIn('requires a corresponding control path and at most two data inputs',
                                          result.stdout + result.stderr)
                            for artifact in artifacts:
                                self.assertEqual(artifact.read_text(), 'previous artifact\n')

        for family in ('Reset', 'TMS', 'TRST'):
            for output in (False, True):
                child = '' if output else f'Module Child {{ {family}Port ctl; }} '
                consumer = (f'To{family}Port out {{ Source mux; }}' if output else
                            'Instance child Of Child { InputPort ctl = mux; }')
                (self.root / 'source.icl').write_text(
                    child + f'Module Network {{ {family}Port ctl; '
                    'DataInPort mode[1:0]; DataInPort d0; DataInPort d1; '
                    "DataMux local SelectedBy mode[0] { 1'b0 : d0; 1'b1 : d1; } "
                    "DataMux unused SelectedBy mode { 2'd0 : ctl; 2'd1 : d0; 2'd2 : d1; } "
                    "DataMux mux SelectedBy mode[1] { 1'b0 : local; 1'b1 : ctl; } " + consumer + ' }')
                self.run_tool('--icl', 'source.icl', '--pdl', 'source.pdl',
                              '--top_module', 'Network', '--output', 'result.pdl', '--debug')
                expected = (self.root / 'result.pdl').read_bytes()
                for model, pdl in (('debug/Network.icl', 'source.pdl'), ('source.icl', 'result.pdl')):
                    self.run_tool('--icl', model, '--pdl', pdl, '--top_module', 'Network',
                                  '--output', 'again.pdl')
                    self.assertEqual((self.root / 'again.pdl').read_bytes(), expected)

    def test_capture_update_mux_structure_preserves_outputs(self):
        (self.root / 'source.pdl').write_text('iProcsForModule Network\n')
        debug = self.root / 'debug/Network.icl'
        debug.parent.mkdir(exist_ok=True)
        artifacts = (self.root / 'result.pdl', self.root / 'reset.pdl', debug)
        for family in ('CaptureEn', 'UpdateEn'):
            prefix = (f'Module Network {{ {family}Port src; {family}Port other; '
                      'DataInPort data; DataInPort mode[1:0]; Alias wrapped = src; ')
            output = f'To{family}Port out {{ Source mux; }} }}'
            models = {
                'one force': prefix + "DataMux mux SelectedBy mode { 0 : 1'b1; 1 : src; } " + output,
                'unknown force': prefix + "DataMux mux SelectedBy mode { 0 : 1'bx; 1 : src; } " + output,
                'extra data': prefix + "DataMux mux SelectedBy mode { 0 : 1'b0; 1 : src; 2 : data; } " + output,
                'extra control': prefix + "DataMux mux SelectedBy mode { 0 : 1'b0; 1 : src; 2 : other; } " + output,
                'inverted distinct control': prefix + "DataMux mux SelectedBy mode { 0 : 1'b0; 1 : src; 2 : ~wrapped; } " + output,
                'outer chain': (prefix + "DataMux inner SelectedBy mode { 0 : 1'b0; 1 : src; } "
                                "Alias forwarded = inner; DataMux mux SelectedBy mode { 0 : 1'b1; 1 : forwarded; } " + output),
                'unused gate': prefix + "DataMux mux SelectedBy mode { 0 : 1'b1; 1 : src; } }",
            }
            for label, source in models.items():
                with self.subTest(family=family, label=label):
                    (self.root / 'source.icl').write_text(source)
                    for artifact in artifacts:
                        artifact.write_text('previous artifact\n')
                    result = self.run_tool('--icl', 'source.icl', '--pdl', 'source.pdl',
                                           '--top_module', 'Network', '--output', 'result.pdl',
                                           '--local_reset', 'reset.pdl', '--debug', ok=False)
                    self.assertIn('requires zero and one corresponding control input',
                                  result.stdout + result.stderr)
                    for artifact in artifacts:
                        self.assertEqual(artifact.read_text(), 'previous artifact\n')

    def test_mux_expansion_and_string_codes_preserve_outputs(self):
        (self.root / 'source.pdl').write_text('iProcsForModule Network\n')
        debug = self.root / 'debug/Network.icl'
        debug.parent.mkdir(exist_ok=True)
        artifacts = (self.root / 'result.pdl', self.root / 'reset.pdl', debug)
        models = {}
        for kind in ('DataMux', 'ScanMux', 'ClockMux'):
            rows = ' '.join(f'{code} : data;' for code in range(65))
            models[kind + ' source expansion'] = (
                'Module Network { DataInPort data[65535:0]; DataInPort mode[6:0]; '
                f'{kind} mux[65535:0] SelectedBy mode {{ {rows} }} }}',
                'mux expansion bit limit exceeded')
            models[kind + ' string code'] = (
                'Module Network { Parameter CODE = "0"; DataInPort mode; '
                f'{kind} mux SelectedBy mode {{ $CODE : 0; }} }}',
                'mux selector code must be numeric')
        models['code expansion'] = (
            'Module Network { DataInPort mode[1048575:0]; '
            'DataMux mux SelectedBy mode { 0 | 1 | 2 : 0; } }',
            'mux expansion bit limit exceeded')
        models['padded source'] = (
            'Module Network { DataInPort mode[1048574:0]; '
            'DataMux mux[4:0] SelectedBy mode { 0 | 1 | 2 : 0; } }',
            'mux expansion bit limit exceeded')
        models['instance aggregate'] = (
            'Module Child { DataInPort mode[524287:0]; '
            'DataMux mux SelectedBy mode { 0 | 1 | 2 : 0; } } '
            'Module Network { Instance a Of Child; Instance b Of Child; }',
            'mux expansion bit limit exceeded')
        for label, (source, diagnostic) in models.items():
            with self.subTest(label=label):
                (self.root / 'source.icl').write_text(source)
                for artifact in artifacts:
                    artifact.write_text('previous artifact\n')
                result = self.run_tool('--icl', 'source.icl', '--pdl', 'source.pdl',
                                       '--top_module', 'Network', '--output', 'result.pdl',
                                       '--local_reset', 'reset.pdl', '--debug', ok=False)
                self.assertIn(diagnostic, result.stdout + result.stderr)
                for artifact in artifacts:
                    self.assertEqual(artifact.read_text(), 'previous artifact\n')

    def test_data_mux_impossible_sources(self):
        (self.root / 'source.pdl').write_text('iProcsForModule Network\n')
        debug = self.root / 'debug/Network.icl'
        debug.parent.mkdir(exist_ok=True)
        artifacts = (self.root / 'result.pdl', self.root / 'reset.pdl', debug)
        declarations = [kind + 'Port source;' for kind in
                        ('TCK', 'Clock', 'ShiftEn', 'ScanIn', 'Address', 'WriteEn', 'ReadEn', 'ToTCK')]
        declarations += ["ToShiftEnPort source { Source 1'b1; }",
                         'TMSPort tms; ToIRSelectPort source;']
        for declaration in declarations:
            with self.subTest(declaration=declaration):
                (self.root / 'source.icl').write_text(
                    'Module Network { ' + declaration +
                    " Alias wrapped = source; DataMux mux SelectedBy 1'b0 { 1'b0 : wrapped; } }")
                for artifact in artifacts:
                    artifact.write_text('previous artifact\n')
                result = self.run_tool('--icl', 'source.icl', '--pdl', 'source.pdl',
                                       '--top_module', 'Network', '--output', 'result.pdl',
                                       '--local_reset', 'reset.pdl', '--debug', ok=False)
                self.assertIn('data signal or supported control source', result.stdout + result.stderr)
                for artifact in artifacts:
                    self.assertEqual(artifact.read_text(), 'previous artifact\n')

    def test_data_mux_matching_control_use(self):
        (self.root / 'source.pdl').write_text('iProcsForModule Network\n')
        debug = self.root / 'debug/Network.icl'
        debug.parent.mkdir(exist_ok=True)
        artifacts = (self.root / 'result.pdl', self.root / 'reset.pdl', debug)
        models = {}
        for family in ('Reset', 'CaptureEn', 'UpdateEn', 'TMS', 'TRST'):
            body = (f'Module Network {{ DataInPort mode; {family}Port source; '
                    "DataMux mux SelectedBy mode { 1'b0 : " +
                    ("1'b1" if family == 'Reset' else "1'b0") + "; 1'b1 : source; } ")
            models[family + ' data only'] = body + 'DataOutPort observed { Source mux; } }'
            other = 'TMS' if family != 'TMS' else 'Reset'
            models[family + ' wrong family'] = (
                f'Module Child {{ {other}Port ctl; }} ' + body +
                'Instance child Of Child { InputPort ctl = mux; } }')
        prefix = ("Module Child { ResetPort ctl; } Module Network { ResetPort source; "
                  "DataInPort mode; DataMux mux SelectedBy mode { 1'b0 : 1'b1; 1'b1 : source; } ")
        for kind in ('ScanMux', 'ClockMux'):
            models[kind + ' choice'] = prefix + kind + " observed SelectedBy 1'b0 { 1'b0 : mux; } }"
        models['selector'] = (prefix + 'DataMux outer SelectedBy mux { '
                              "1'b0 : 1'b0; 1'b1 : 1'b1; } "
                              'Instance child Of Child { InputPort ctl = outer; } }')
        models['enable'] = prefix + "DataOutPort observed { Source 1'b0; Enable mux; } }"
        models['logic'] = (prefix + 'LogicSignal logic { mux; } '
                           'DataOutPort observed { Source logic; } }')
        models['data port'] = (
            'Module Forward { DataInPort d; DataOutPort out { Source d; } } ' + prefix +
            'Instance forward Of Forward { InputPort d = mux; } '
            'DataOutPort observed { Source forward.out; } }')
        models['observed vector bit'] = (
            'Module Child { ResetPort ctl; } Module Network { ResetPort source[1:0]; '
            "DataInPort mode; DataMux mux[1:0] SelectedBy mode { 1'b0 : 2'b11; 1'b1 : source; } "
            'Instance child Of Child { InputPort ctl = mux[1]; } '
            'DataOutPort observed { Source mux[0]; } }')
        for label, source in models.items():
            with self.subTest(label=label):
                (self.root / 'source.icl').write_text(source)
                for artifact in artifacts:
                    artifact.write_text('previous artifact\n')
                result = self.run_tool('--icl', 'source.icl', '--pdl', 'source.pdl',
                                       '--top_module', 'Network', '--output', 'result.pdl',
                                       '--local_reset', 'reset.pdl', '--debug', ok=False)
                self.assertIn('matching control use', result.stdout + result.stderr)
                for artifact in artifacts:
                    self.assertEqual(artifact.read_text(), 'previous artifact\n')

    def test_inferred_control_vector_roundtrips(self):
        (self.root / 'source.pdl').write_text('iProcsForModule Network\n')
        for family in ('Reset', 'TRST', 'TMS'):
            (self.root / 'source.icl').write_text(
                f'Module Child {{ {family}Port ctl[4:5]; }} Module Network {{ '
                f'{family}Port source[7]; To{family}Port output[1:0]; Instance child Of Child; }}')
            self.run_tool('--icl', 'source.icl', '--pdl', 'source.pdl',
                          '--output', 'result.pdl', '--debug')
            self.run_tool('--icl', 'debug/Network.icl', '--pdl', 'source.pdl',
                          '--output', 'again.pdl')

    def test_round_trips(self):
        result = self.run_tool("--icl", "source.icl", "--pdl", "source.pdl", "--debug", "--log", "run.log")
        self.assertIn("warning", result.stdout)
        self.assertIn("out/Network.pdl", result.stdout)
        self.assertEqual((self.root / "run.log").read_text(), result.stdout)
        original = (self.root / "out/Network.pdl").read_text()
        self.assertNotIn("iProc ", original)
        self.assertNotIn("iCall ", original)
        self.assertIn("iWrite gate 0b1", original)
        flat = self.root / "out/debug/Network.icl"
        first_flat = flat.read_text()
        self.run_tool("--icl", flat, "--pdl", "source.pdl", "--output", "again/result.pdl", "--debug")
        self.assertEqual(original, (self.root / "again/result.pdl").read_text())
        self.assertEqual(first_flat, (self.root / "again/debug/Network.icl").read_text())
        self.run_tool("--icl", "source.icl", "--pdl", "out/Network.pdl", "--output", "replay.pdl")
        self.assertEqual(original, (self.root / "replay.pdl").read_text())

    def test_primary_input_one_hot_write_round_trips(self):
        fixture = pathlib.Path(__file__).resolve().parents[1] / "cases/ours/pattern_generation/one_hot_width.icl"
        for top, value in (("EqualWidthProbe", "0x96"),
                           ("WidthProbe", "0bXXXXXXXX10010110"),
                           ("AscendingWidthProbe", "0bXXXXXXXX10010110")):
            with self.subTest(top=top):
                (self.root / "source.pdl").write_text(
                    f"iProcsForModule {top}\niWrite observed {value}\niApply\n")
                inputs = ("--icl", fixture, "--top_module", top)
                self.run_tool(*inputs, "--pdl", "source.pdl", "--output", "result.pdl",
                              "--debug", "--local_reset", "reset.pdl")
                output = (self.root / "result.pdl").read_text()
                self.assertIn("iWrite data 0b10010110\n", output)
                self.assertIn("iWrite enable 0b1\n", output)
                self.assertEqual((self.root / "reset.pdl").read_bytes(),
                                 (self.root / "source.pdl").read_bytes())
                self.run_tool(*inputs, "--pdl", "result.pdl", "--output", "again.pdl")
                self.assertEqual(output, (self.root / "again.pdl").read_text())
                flat = self.root / f"debug/{top}.icl"
                flat_bytes = flat.read_bytes()
                self.run_tool("--icl", flat, "--pdl", "source.pdl", "--output", "flat/result.pdl", "--debug")
                self.assertEqual(output, (self.root / "flat/result.pdl").read_text())
                self.assertEqual(flat_bytes, (self.root / f"flat/debug/{top}.icl").read_bytes())
        for top in ("WidthProbe", "AscendingWidthProbe"):
            for value in ("0x96", "0xff96", "0b10010110XXXXXXXX"):
                with self.subTest(top=top, rejected=value):
                    (self.root / "source.pdl").write_text(
                        f"iProcsForModule {top}\niWrite observed {value}\niApply\n")
                    original = b"previous output\n"
                    (self.root / "result.pdl").write_bytes(original)
                    failure = self.run_tool("--icl", fixture, "--top_module", top,
                                            "--pdl", "source.pdl", "--output", "result.pdl", ok=False)
                    self.assertIn("cannot justify", failure.stdout)
                    self.assertEqual((self.root / "result.pdl").read_bytes(), original)

    def test_deep_alias_and_pdl_targets_round_trip(self):
        fixture = pathlib.Path(__file__).resolve().parents[1] / "cases/ours/pattern_generation/connection_hierarchy.icl"
        (self.root / "source.icl").write_bytes(fixture.read_bytes())
        (self.root / "source.pdl").write_text(
            "iProcsForModule ConnectionHierarchy\niReset\niWrite deep 5\niApply\n"
            "iRead deep_port 5\niRead middle.leaf.value 5\niApply\n")
        self.run_tool("--icl", "source.icl", "--pdl", "source.pdl",
                      "--output", "result.pdl", "--debug")
        expected = (self.root / "result.pdl").read_bytes()
        self.assertIn(b"iWrite middle.leaf.value 0b0101", expected)
        flat = self.root / "debug/ConnectionHierarchy.icl"
        self.run_tool("--icl", flat, "--pdl", "source.pdl",
                      "--output", "flat/result.pdl", "--debug")
        self.assertEqual((self.root / "flat/result.pdl").read_bytes(), expected)
        self.assertEqual((self.root / "flat/debug/ConnectionHierarchy.icl").read_bytes(),
                         flat.read_bytes())
        self.run_tool("--icl", "source.icl", "--pdl", "result.pdl", "--output", "again.pdl")
        self.assertEqual((self.root / "again.pdl").read_bytes(), expected)

    def test_invalid_direct_hierarchy_preserves_outputs(self):
        fixture = pathlib.Path(__file__).resolve().parents[1] / "cases/ours/pattern_generation/connection_hierarchy.icl"
        source = fixture.read_text()
        (self.root / "source.pdl").write_text("iProcsForModule ConnectionHierarchy\niNote hierarchy\n")
        for target in ("child.value", "child.local", "child.data", "middle.leaf.q"):
            with self.subTest(target=target):
                position = source.rfind("}")
                invalid = source[:position] + f"DataOutPort bad[3:0] {{ Source {target}; }}\n" + source[position:]
                (self.root / "source.icl").write_text(invalid)
                (self.root / "result.pdl").write_text("preserve pattern\n")
                (self.root / "reset.pdl").write_text("preserve reset\n")
                debug = self.root / "debug/ConnectionHierarchy.icl"
                debug.parent.mkdir(exist_ok=True)
                debug.write_text("preserve debug\n")
                result = self.run_tool("--icl", "source.icl", "--pdl", "source.pdl",
                                       "--output", "result.pdl", "--local_reset", "reset.pdl",
                                       "--debug", ok=False)
                self.assertIn("source.icl:", result.stdout)
                self.assertIn("multi-level hierarchy is allowed only in Alias"
                              if target == "middle.leaf.q" else "must name an output port",
                              result.stdout)
                self.assertEqual((self.root / "result.pdl").read_text(), "preserve pattern\n")
                self.assertEqual((self.root / "reset.pdl").read_text(), "preserve reset\n")
                self.assertEqual(debug.read_text(), "preserve debug\n")

    def test_local_and_child_output_alias_chains_round_trip(self):
        fixture = pathlib.Path(__file__).resolve().parents[1] / "cases/ours/pattern_generation/connection_hierarchy.icl"
        source = fixture.read_text()
        position = source.rfind("}")
        source = source[:position] + (
            "Alias first[3:0] = child.q; Alias second[3:0] = first;\n"
            "DataOutPort via_alias[3:0] { Source second; }\n") + source[position:]
        (self.root / "source.icl").write_text(source)
        (self.root / "source.pdl").write_text(
            "iProcsForModule ConnectionHierarchy\niReset\niWrite child.value 5\niApply\n"
            "iRead second 5\niRead via_alias 5\niApply\n")
        self.run_tool("--icl", "source.icl", "--pdl", "source.pdl",
                      "--output", "result.pdl", "--debug")
        expected = (self.root / "result.pdl").read_bytes()
        self.assertIn(b"iWrite child.value 0b0101", expected)
        flat = self.root / "debug/ConnectionHierarchy.icl"
        self.run_tool("--icl", flat, "--pdl", "source.pdl",
                      "--output", "flat/result.pdl", "--debug")
        self.assertEqual((self.root / "flat/result.pdl").read_bytes(), expected)
        self.assertEqual((self.root / "flat/debug/ConnectionHierarchy.icl").read_bytes(),
                         flat.read_bytes())
        self.run_tool("--icl", "source.icl", "--pdl", "result.pdl", "--output", "again.pdl")
        self.assertEqual((self.root / "again.pdl").read_bytes(), expected)

    def test_alias_instance_boundaries_preserve_outputs(self):
        fixture = pathlib.Path(__file__).resolve().parents[1] / "cases/ours/pattern_generation/connection_hierarchy.icl"
        source = fixture.read_text()
        (self.root / "source.pdl").write_text("iProcsForModule ConnectionHierarchy\niNote alias\n")
        for target in ("child.value", "child.data", "child.local"):
            for wrapped in (False, True):
                with self.subTest(target=target, wrapped=wrapped):
                    extra = f"Alias a[3:0] = {target}; "
                    extra += ("Alias b[3:0] = a; DataOutPort bad[3:0] { Source b; }"
                              if wrapped else "DataOutPort bad[3:0] { Source a; }")
                    position = source.rfind("}")
                    (self.root / "source.icl").write_text(source[:position] + extra + source[position:])
                    debug = self.root / "debug/ConnectionHierarchy.icl"
                    debug.parent.mkdir(exist_ok=True)
                    artifacts = (self.root / "result.pdl", self.root / "reset.pdl", debug)
                    for artifact in artifacts:
                        artifact.write_text("preserve " + artifact.name + "\n")
                    result = self.run_tool("--icl", "source.icl", "--pdl", "source.pdl",
                                           "--output", "result.pdl", "--local_reset", "reset.pdl",
                                           "--debug", ok=False)
                    self.assertIn("source.icl:", result.stdout)
                    self.assertIn("only available to aliases or PDL", result.stdout)
                    for artifact in artifacts:
                        self.assertEqual(artifact.read_text(), "preserve " + artifact.name + "\n")

    def test_hidden_alias_wrappers_round_trip(self):
        fixture = pathlib.Path(__file__).resolve().parents[1] / "cases/ours/pattern_generation/connection_hierarchy.icl"
        source = fixture.read_text()
        position = source.rfind("}")
        source = source[:position] + (
            "Alias second[3:0] = ~first[0:3]; Alias first[3:0] = child.value;\n") + source[position:]
        (self.root / "source.icl").write_text(source)
        (self.root / "source.pdl").write_text(
            "iProcsForModule ConnectionHierarchy\niReset\niWrite second 1\niApply\n"
            "iRead child.value 7\niRead second 1\niApply\n")
        self.run_tool("--icl", "source.icl", "--pdl", "source.pdl",
                      "--output", "result.pdl", "--debug")
        expected = (self.root / "result.pdl").read_bytes()
        self.assertIn(b"iWrite child.value 0b0111", expected)
        flat = self.root / "debug/ConnectionHierarchy.icl"
        self.run_tool("--icl", flat, "--pdl", "source.pdl", "--output", "flat/result.pdl")
        self.assertEqual((self.root / "flat/result.pdl").read_bytes(), expected)
        self.run_tool("--icl", "source.icl", "--pdl", "result.pdl", "--output", "again.pdl")
        self.assertEqual((self.root / "again.pdl").read_bytes(), expected)

    def test_deep_alias_consumers_preserve_outputs(self):
        fixture = pathlib.Path(__file__).resolve().parents[1] / "cases/ours/pattern_generation/connection_hierarchy.icl"
        source = fixture.read_text()
        (self.root / "source.pdl").write_text("iProcsForModule ConnectionHierarchy\niNote alias\n")
        for extra in ("DataOutPort bad[3:0] { Source deep; }",
                      "DataOutPort bad[3:0] { Source deep_port; }",
                      "Alias wrapper[3:0] = deep;",
                      "Alias mixed[7:0] = child.q, middle.leaf.value; "
                      "DataOutPort bad[3:0] { Source mixed[7:4]; }"):
            for forward in (False, True):
                with self.subTest(extra=extra, forward=forward):
                    position = source.index("  Alias deep[") if forward else source.rfind("}")
                    (self.root / "source.icl").write_text(source[:position] + extra + source[position:])
                    debug = self.root / "debug/ConnectionHierarchy.icl"
                    debug.parent.mkdir(exist_ok=True)
                    artifacts = (self.root / "result.pdl", self.root / "reset.pdl", debug)
                    for artifact in artifacts:
                        artifact.write_text("preserve " + artifact.name + "\n")
                    result = self.run_tool("--icl", "source.icl", "--pdl", "source.pdl",
                                           "--output", "result.pdl", "--local_reset", "reset.pdl",
                                           "--debug", ok=False)
                    self.assertIn("source.icl:", result.stdout)
                    self.assertIn("multi-level alias", result.stdout)
                    for artifact in artifacts:
                        self.assertEqual(artifact.read_text(), "preserve " + artifact.name + "\n")

    def test_scan_output_rightmost_round_trips(self):
        for bounds, bit in (("[3:0]", 0), ("[0:3]", 3)):
            for inversion in ("", "~"):
                with self.subTest(bounds=bounds, inversion=inversion):
                    (self.root / "source.icl").write_text(
                        f"Module ScanOutput {{ ScanInPort si; ScanRegister r{bounds} "
                        "{ ScanInSource si; CaptureSource r; ResetValue 0; } "
                        f"ScanOutPort so {{ Source {inversion}r[{bit}]; }} "
                        "DataOutPort middle { Source r[2]; } }\n")
                    (self.root / "source.pdl").write_text(
                        "iProcsForModule ScanOutput\niReset\niWrite r[2] 1\niApply\n"
                        "iRead r[2] 1\niApply\n")
                    self.run_tool("--icl", "source.icl", "--pdl", "source.pdl",
                                  "--output", "result.pdl", "--debug")
                    expected = (self.root / "result.pdl").read_bytes()
                    self.assertIn(b"iWrite r ", expected)
                    flat = self.root / "debug/ScanOutput.icl"
                    self.run_tool("--icl", flat, "--pdl", "source.pdl",
                                  "--output", "flat/result.pdl", "--debug")
                    self.assertEqual((self.root / "flat/result.pdl").read_bytes(), expected)
                    self.assertEqual((self.root / "flat/debug/ScanOutput.icl").read_bytes(),
                                     flat.read_bytes())
                    self.run_tool("--icl", "source.icl", "--pdl", "result.pdl",
                                  "--output", "again.pdl")
                    self.assertEqual((self.root / "again.pdl").read_bytes(), expected)

    def test_invalid_scan_output_preserves_outputs(self):
        cases = (("[3:0]", "", "r[2]"), ("[0:3]", "", "~r[0]"),
                 ("[3:0]", "[3:0]", "r"), ("[0:3]", "[1:0]", "r[2:3]"),
                 ("[3:0]", "[1:0]", "r[0], r[2]"))
        (self.root / "source.pdl").write_text("iProcsForModule ScanOutput\niNote source\n")
        for bounds, output_bounds, source in cases:
            with self.subTest(bounds=bounds, source=source):
                (self.root / "source.icl").write_text(
                    f"Module ScanOutput {{ ScanInPort si; ScanRegister r{bounds} "
                    "{ ScanInSource si; } "
                    f"ScanOutPort so{output_bounds} {{ Source {source}; }} }}\n")
                debug = self.root / "debug/ScanOutput.icl"
                debug.parent.mkdir(exist_ok=True)
                artifacts = (self.root / "result.pdl", self.root / "reset.pdl", debug)
                for artifact in artifacts:
                    artifact.write_text("preserve " + artifact.name + "\n")
                result = self.run_tool("--icl", "source.icl", "--pdl", "source.pdl",
                                       "--output", "result.pdl", "--local_reset", "reset.pdl",
                                       "--debug", ok=False)
                self.assertIn("source.icl:", result.stdout)
                self.assertIn("ScanOutPort Source must use the rightmost bit of r", result.stdout)
                for artifact in artifacts:
                    self.assertEqual(artifact.read_text(), "preserve " + artifact.name + "\n")

    def test_tap_round_trips(self):
        fixture = pathlib.Path(__file__).resolve().parents[1] / "cases/ours/pattern_generation/access_link"
        for name in ("simple.icl", "simple.bsdl"):
            (self.root / name).write_bytes((fixture / name).read_bytes())
        (self.root / "source.pdl").write_text(
            "iProcsForModule AccessTop\niReset -sync\niWrite core.r 0x59\niApply -together\n"
            "iRunLoop 0x100000000 -tck\niRunLoop -tck 0b11\niRunLoop 0\n"
            "iRead core.r 0x59\niWrite core.r 0xa6\niApply\n")
        inputs = ("--icl", "simple.icl", "--bsdl", "AccessProbe", "simple.bsdl")
        self.run_tool(*inputs, "--pdl", "source.pdl", "--debug", "--local_reset", "reset.pdl")
        self.assertEqual((self.root / "reset.pdl").read_bytes(), (self.root / "source.pdl").read_bytes())
        original = (self.root / "out/AccessTop.pdl").read_text()
        self.assertIn("iReset -sync\n", original)
        self.assertEqual(original.count("iApply -together"), 1)
        self.assertIn("iRunLoop 4294967296 -tck\n", original)
        self.assertIn("iRunLoop 3 -tck\n", original)
        self.assertIn("iRunLoop 0 -tck\n", original)
        self.assertIn("iWrite core.r 0b01011001", original)
        self.assertNotIn("__", original)
        flat = self.root / "out/debug/AccessTop.icl"
        self.run_tool("--icl", flat, "--pdl", "source.pdl", "--output", "again/result.pdl", "--debug")
        self.assertEqual(original, (self.root / "again/result.pdl").read_text())
        self.assertEqual(flat.read_text(), (self.root / "again/debug/AccessTop.icl").read_text())
        self.run_tool(*inputs, "--pdl", "out/AccessTop.pdl", "--output", "replay.pdl")
        self.assertEqual(original, (self.root / "replay.pdl").read_text())
        source = (self.root / "simple.bsdl").read_bytes()
        self.run_tool(*inputs, "--pdl", "source.pdl", "--output", "simple.bsdl", ok=False)
        self.assertEqual(source, (self.root / "simple.bsdl").read_bytes())

    def test_tap_instruction_switching(self):
        fixture = pathlib.Path(__file__).resolve().parents[1] / "cases/ours/pattern_generation/access_link"
        source = (fixture / "simple.icl").read_text().replace(
            "  AccessLink tap", "  Instance other Of AccessInstrument { InputPort mode = 1'b1; }\n  AccessLink tap"
        ).replace("IJTAG_OFF { ScanInterface { core.client; } }",
                  "IJTAG_OFF { ScanInterface { other.client; } }")
        (self.root / "source.icl").write_text(source)
        (self.root / "source.pdl").write_text(
            "iProcsForModule AccessTop\niReset\niWrite core.r 3\niApply\n"
            "iWrite other.r 9\niApply\niWrite core.r 5\niApply\n")
        inputs = ("--icl", "source.icl", "--bsdl", "AccessProbe", fixture / "simple.bsdl")
        self.run_tool(*inputs, "--pdl", "source.pdl")
        original = (self.root / "out/AccessTop.pdl").read_text()
        self.assertEqual(original.count("iApply"), 6)
        self.assertEqual(original.count("iWrite tap.INSTRUCTION"), 3)
        self.assertIn("iWrite other.r 0b00001001", original)
        self.run_tool(*inputs, "--pdl", "out/AccessTop.pdl", "--output", "replay.pdl")
        self.assertEqual(original, (self.root / "replay.pdl").read_text())
        (self.root / "source.pdl").write_text(
            "iProcsForModule AccessTop\niReset\niWrite core.r 3\niWrite other.r 9\niApply -together\n")
        result = self.run_tool(*inputs, "--pdl", "source.pdl", "--output", "impossible.pdl", ok=False)
        self.assertIn("iApply -together", result.stdout)
        self.assertFalse((self.root / "impossible.pdl").exists())

    def test_bsdl_arguments(self):
        for args in (("--bsdl",), ("--bsdl", "AccessProbe"),
                     ("--bsdl", "AccessProbe", "--debug"), ("--bsdl", "", "file")):
            self.assertIn("requires ENTITY FILE", self.run_tool(*args, ok=False).stdout)

    def test_apply_together_round_trips(self):
        (self.root / "source.pdl").write_text(
            "iState gate 0\niWrite gate 1\niWrite target 10\niApply -together\n")
        self.run_tool("--icl", "source.icl", "--pdl", "source.pdl", "--debug")
        original = (self.root / "out/Network.pdl").read_text()
        self.assertEqual(original.count("iApply"), 2)
        self.assertEqual(original.count("iApply -together"), 1)
        self.assertIn("iWrite target 0b1010", original)
        self.run_tool("--icl", "out/debug/Network.icl", "--pdl", "source.pdl", "--output", "flat.pdl")
        self.assertEqual(original, (self.root / "flat.pdl").read_text())
        self.run_tool("--icl", "source.icl", "--pdl", "out/Network.pdl", "--output", "again.pdl")
        self.assertEqual(original, (self.root / "again.pdl").read_text())

    def test_access_together_round_trips(self):
        fixture = pathlib.Path(__file__).resolve().parents[1] / "cases/ours/pattern_generation/access_together.icl"
        (self.root / "source.icl").write_bytes(fixture.read_bytes())
        for operation in ("iWrite", "iRead"):
            with self.subTest(operation=operation):
                (self.root / "source.pdl").write_text(
                    "iState gate 0\niState a 0\niState b 0\niState c 0\n"
                    f"{operation} nested 1\n{operation} c 0\niApply\n")
                self.run_tool("--icl", "source.icl", "--pdl", "source.pdl", "--debug")
                original = (self.root / "out/Network.pdl").read_text()
                self.assertEqual(original.count("iApply"), 2)
                flat = self.root / "out/debug/Network.icl"
                self.assertIn("AccessTogether;", flat.read_text())
                first_flat = flat.read_text()
                self.run_tool("--icl", flat, "--pdl", "source.pdl", "--output", "again/result.pdl", "--debug")
                self.assertEqual(original, (self.root / "again/result.pdl").read_text())
                self.assertEqual(first_flat, (self.root / "again/debug/Network.icl").read_text())
                self.run_tool("--icl", "source.icl", "--pdl", "out/Network.pdl", "--output", "replay.pdl")
                self.assertEqual(original, (self.root / "replay.pdl").read_text())

    def test_access_together_overlapping_reads(self):
        fixture = pathlib.Path(__file__).resolve().parents[1] / "cases/ours/pattern_generation/access_together_overlap.icl"
        (self.root / "source.icl").write_bytes(fixture.read_bytes())
        (self.root / "source.pdl").write_text(
            "iState gate 0\niRead first 0\niRead second 0\niApply\n")
        self.run_tool("--icl", "source.icl", "--pdl", "source.pdl", "--debug")
        original = (self.root / "out/Network.pdl").read_text()
        scans = original.split("iApply")
        self.assertEqual(len(scans), 3)
        self.assertNotIn("iRead", scans[0])
        for target in ("a", "b", "c"):
            self.assertIn(f"iRead {target} 0b0", scans[1])
        self.run_tool("--icl", "out/debug/Network.icl", "--pdl", "source.pdl", "--output", "flat.pdl")
        self.assertEqual(original, (self.root / "flat.pdl").read_text())
        self.run_tool("--icl", "source.icl", "--pdl", "out/Network.pdl", "--output", "again.pdl")
        self.assertEqual(original, (self.root / "again.pdl").read_text())

    def test_local_reset_passthrough_preserves_procedures(self):
        source = (self.root / "source.pdl").read_bytes()
        self.run_tool("--icl", "source.icl", "--pdl", "source.pdl",
                      "--local_reset", "reset.pdl", "--debug")
        self.assertEqual((self.root / "reset.pdl").read_bytes(), source)
        original = (self.root / "out/Network.pdl").read_bytes()
        self.run_tool("--icl", "out/debug/Network.icl", "--pdl", "reset.pdl",
                      "--output", "again.pdl", "--local_reset", "again-reset.pdl")
        self.assertEqual((self.root / "again.pdl").read_bytes(), original)
        self.assertEqual((self.root / "again-reset.pdl").read_bytes(), source)
        self.run_tool("--icl", "source.icl", "--pdl", "source.pdl",
                      "--local_reset", "source.pdl", ok=False)
        self.assertEqual((self.root / "source.pdl").read_bytes(), source)
        (self.root / "blocked-reset").mkdir()
        self.run_tool("--icl", "source.icl", "--pdl", "source.pdl",
                      "--output", "out/Network.pdl", "--local_reset", "blocked-reset", ok=False)
        self.assertEqual((self.root / "out/Network.pdl").read_bytes(), original)
        (self.root / "second.pdl").write_text("iApply\n")
        self.run_tool("--icl", "source.icl", "--pdl", "source.pdl", "second.pdl",
                      "--local_reset", "multi.pdl")
        self.assertEqual((self.root / "multi.pdl").read_bytes(), source + b"\n\niApply\n")

    def test_local_reset_passthrough_checks_reset_connections(self):
        source = """Module Child {
 ScanInPort si; ResetPort rst;
 ScanRegister r { ScanInSource si; ResetValue 1'b0; }
 ScanOutPort so { Source r; }
}
Module Network {
 ScanInPort si; ResetPort rst;
 ScanRegister release { ScanInSource si; }
 Instance child Of Child { InputPort si = release; }
 ScanOutPort so { Source child.so; }
}
"""
        (self.root / "source.icl").write_text(source)
        (self.root / "source.pdl").write_text("iReset\niWrite child.r 1\niApply\n")
        self.run_tool("--icl", "source.icl", "--pdl", "source.pdl", "--local_reset", "reset.pdl")
        self.assertEqual((self.root / "reset.pdl").read_bytes(), (self.root / "source.pdl").read_bytes())
        for connection in ("1'bx", "loop"):
            invalid = source.replace("InputPort si = release;",
                                     f"InputPort si = release; InputPort rst = {connection};")
            if connection == "loop":
                invalid = invalid.replace(" ScanRegister release", " ToResetPort loop { Source loop; }\n ScanRegister release")
            (self.root / "source.icl").write_text(invalid)
            failure = self.run_tool("--icl", "source.icl", "--pdl", "source.pdl",
                                    "--local_reset", "invalid-reset.pdl", ok=False)
            self.assertIn("cycle" if connection == "loop" else "unknown local-reset",
                          failure.stdout)
            self.assertFalse((self.root / "invalid-reset.pdl").exists())
        source = source.replace(
            'ScanRegister release { ScanInSource si; }',
            "ScanRegister release { ScanInSource si; } "
            "DataMux reset_gate SelectedBy release { 1'b0 : 1'b1; 1'b1 : rst; }")
        source = source.replace("InputPort si = release;", "InputPort si = release; InputPort rst = reset_gate;")
        (self.root / "source.icl").write_text(source)
        (self.root / "reset.pdl").write_text("preserve reset output\n")
        (self.root / "result.pdl").write_text("preserve pattern output\n")
        self.run_tool("--icl", "source.icl", "--pdl", "source.pdl",
                      "--local_reset", "reset.pdl", "--output", "result.pdl")
        self.assertIn("iWrite release 0b1\niApply\niWrite child.r 1",
                      (self.root / "reset.pdl").read_text())

    def test_transitive_local_reset_rewrite_preserves_procedures(self):
        fixture = pathlib.Path(__file__).resolve().parents[1] / "cases/ours/pattern_generation/local_reset"
        self.run_tool("--icl", fixture / "nested.icl", "--pdl",
                      fixture / "procedures.pdl", fixture / "main.pdl",
                      "--local_reset", "reset.pdl", "--output", "result.pdl")
        rewritten = (self.root / "reset.pdl").read_text()
        self.assertTrue(rewritten.startswith((fixture / "procedures.pdl").read_text()))
        self.assertIn("iWrite root 0b1\niApply\niWrite branch.release 0b1\niApply\n", rewritten)
        self.assertTrue(rewritten.endswith("iCall configure 10\n"))
        self.assertNotIn("iWrite gate", rewritten)
        original = (self.root / "result.pdl").read_bytes()
        self.run_tool("--icl", fixture / "nested.icl", "--pdl", "reset.pdl",
                      "--local_reset", "again-reset.pdl", "--output", "replayed.pdl")
        self.assertEqual(rewritten, (self.root / "again-reset.pdl").read_text())
        self.assertIn(b"iWrite a.value 0b1010", original)
        self.assertIn(b"iWrite a.value 0b1010", (self.root / "replayed.pdl").read_bytes())

    def test_local_reset_groups_parallel_controls_before_user_queue(self):
        fixture = pathlib.Path(__file__).resolve().parents[1] / "cases/ours/pattern_generation/local_reset"
        source = "iReset\niWrite a.value 5; iWrite b.value 6; iApply\n"
        (self.root / "source.pdl").write_text(source)
        self.run_tool("--icl", fixture / "nested.icl", fixture / "parallel.icl",
                      "--top_module", "ParallelResetNetwork", "--pdl", "source.pdl",
                      "--local_reset", "reset.pdl")
        expected = "iReset\niWrite ra 0b1\niWrite rb 0b0\niApply\n" + source.split("\n", 1)[1]
        self.assertEqual((self.root / "reset.pdl").read_text(), expected)

    def test_local_reset_tap_round_trips(self):
        fixture = pathlib.Path(__file__).resolve().parents[1] / "cases/ours/pattern_generation"
        inputs = ("--icl", fixture / "local_reset/nested.icl", fixture / "local_reset/vendor.icl",
                  "--bsdl", "AccessProbe", fixture / "access_link/simple.bsdl",
                  "--top_module", "ResetAccessTop")
        self.run_tool(*inputs, "--pdl", fixture / "local_reset/vendor.pdl",
                      "--output", "result.pdl", "--local_reset", "reset.pdl", "--debug")
        original = (self.root / "result.pdl").read_bytes()
        rewritten = (self.root / "reset.pdl").read_text()
        self.assertIn("iWrite core.root 0b1\niApply\niWrite core.branch.release 0b1\niApply\n", rewritten)
        self.assertNotIn("iWrite core.gate", rewritten)
        self.run_tool(*inputs, "--pdl", "reset.pdl", "--output", "reset-result.pdl",
                      "--local_reset", "reset-again.pdl")
        self.assertEqual(rewritten, (self.root / "reset-again.pdl").read_text())
        self.run_tool(*inputs, "--pdl", "result.pdl", "--output", "again.pdl")
        self.assertEqual(original, (self.root / "again.pdl").read_bytes())
        flat = self.root / "debug/ResetAccessTop.icl"
        self.run_tool("--icl", flat, "--pdl", fixture / "local_reset/vendor.pdl",
                      "--output", "flat/result.pdl", "--debug")
        self.assertEqual(original, (self.root / "flat/result.pdl").read_bytes())
        self.assertEqual(flat.read_bytes(), (self.root / "flat/debug/ResetAccessTop.icl").read_bytes())

    def test_local_reset_prefix_and_called_body_failure(self):
        fixture = pathlib.Path(__file__).resolve().parents[1] / "cases/ours/pattern_generation/local_reset/nested.icl"
        body = "iProc configure {v} { iWrite value $v; iApply }"
        source = "iPDLLevel 1\niProcsForModule Leaf\n" + body + "\niReset\niPrefix a\niCall configure 7\n"
        (self.root / "source.pdl").write_text(source)
        self.run_tool("--icl", fixture, "--pdl", "source.pdl", "--local_reset", "reset.pdl")
        rewritten = (self.root / "reset.pdl").read_text()
        self.assertIn(body, rewritten)
        self.assertIn("iPrefix -\niWrite root 0b1\niApply\niWrite branch.release 0b1\niApply\niPrefix a\niCall", rewritten)
        self.run_tool("--icl", fixture, "--pdl", "reset.pdl", "--output", "replayed.pdl")
        source = "iPDLLevel 1\niProcsForModule Network\niProc bad {} { iWrite root 0; iApply; iWrite a.value 7; iApply }\niReset\niCall bad\n"
        (self.root / "source.pdl").write_text(source)
        (self.root / "reset.pdl").write_text("previous artifact\n")
        failure = self.run_tool("--icl", fixture, "--pdl", "source.pdl", "--local_reset", "reset.pdl", ok=False)
        self.assertIn("cannot repair", failure.stdout)
        self.assertIn("iCall bad", failure.stdout)
        self.assertEqual((self.root / "reset.pdl").read_text(), "previous artifact\n")

    def test_local_reset_repairs_argument_resets(self):
        fixture = pathlib.Path(__file__).resolve().parents[1] / "cases/ours/pattern_generation/local_reset/nested.icl"
        source = "iPDLLevel 1\niWrite a.value [iReset; expr 3]\niApply\n"
        (self.root / "source.pdl").write_text(source)
        self.run_tool("--icl", fixture, "--pdl", "source.pdl",
                      "--local_reset", "reset.pdl")
        rewritten = (self.root / "reset.pdl").read_text()
        self.assertIn("apply ", rewritten)
        self.assertIn("iWrite a.value [iReset; expr 3]", rewritten)

    def test_local_reset_nested_access_together(self):
        fixture = pathlib.Path(__file__).resolve().parents[1] / "cases/ours/pattern_generation/local_reset"
        source = (fixture / "parallel.icl").read_text()
        source = source.rsplit("}", 1)[0] + """
 Alias inner[7:0] = a.value, b.value { AccessTogether; }
 Alias outer[4:0] = ra, inner[7:4] { AccessTogether; }
}
"""
        (self.root / "source.icl").write_text(source)
        (self.root / "source.pdl").write_text("iReset\niWrite outer[4] 1\niApply\n")
        self.run_tool("--icl", fixture / "nested.icl", "source.icl", "--top_module",
                      "ParallelResetNetwork", "--pdl", "source.pdl", "--local_reset", "reset.pdl")
        self.assertIn("iWrite ra 0b1\niWrite rb 0b0\niApply\n",
                      (self.root / "reset.pdl").read_text())

    def test_local_reset_multi_file_continued_comment(self):
        (self.root / "source.pdl").write_text("iPDLLevel 1\niState gate 0\n# continued comment " + chr(92) + "\n")
        (self.root / "next.pdl").write_text("iWrite target 10\niApply\n")
        self.run_tool("--icl", "source.icl", "--pdl", "source.pdl", "next.pdl",
                      "--local_reset", "reset.pdl", "--output", "first.pdl")
        self.run_tool("--icl", "source.icl", "--pdl", "reset.pdl", "--output", "second.pdl")
        self.assertEqual((self.root / "first.pdl").read_bytes(), (self.root / "second.pdl").read_bytes())

    def test_idle_transient_reset_fill(self):
        fixture = pathlib.Path(__file__).resolve().parents[1] / "cases/ours/pattern_generation/access_link"
        (self.root / "source.pdl").write_text(
            "iReset\niState trigger.r 2\niState gate.r 0\niState core.r 0xff\n"
            "iRunLoop 1\niWrite core.r 0bXXXXXXXX\niApply\n")
        inputs = ("--icl", fixture / "idle_reset.icl", "--bsdl", "AccessProbe", fixture / "simple.bsdl")
        self.run_tool(*inputs, "--pdl", "source.pdl", "--debug")
        output = (self.root / "out/AccessTop.pdl").read_text()
        self.assertIn("iWrite core.r 0b00000000\n", output)
        self.assertNotIn("iWrite core.r 0b11111111\n", output)
        self.run_tool(*inputs, "--pdl", "out/AccessTop.pdl", "--output", "again.pdl")
        self.assertEqual(output, (self.root / "again.pdl").read_text())
        self.run_tool("--icl", "out/debug/AccessTop.icl", "--pdl", "source.pdl", "--output", "flat.pdl")
        self.assertEqual(output, (self.root / "flat.pdl").read_text())

    def test_top_level_port_assumptions(self):
        fixture = pathlib.Path(__file__).resolve().parents[1] / "cases/ours/pattern_generation/access_link"
        source = (fixture / "simple.icl").read_text().replace(
            "Module AccessTop {", "Module AccessTop {\n  SelectPort enabled; DataInPort mode;"
        ).replace("InputPort mode = tap.long_setup;", "InputPort mode = mode; InputPort select = enabled;")
        (self.root / "source.icl").write_text(source)
        (self.root / "source.pdl").write_text(
            "iReset\niState enabled 1\niState mode 1\niWrite core.r 0x59\niApply\n"
            "iState enabled 0\niRunLoop 1\niState enabled 1 -LastWrittenValue\n"
            "iRead core.r 0x59\niApply\n")
        inputs = ("--icl", "source.icl", "--bsdl", "AccessProbe", fixture / "simple.bsdl")
        self.run_tool(*inputs, "--pdl", "source.pdl", "--debug")
        output = (self.root / "out/AccessTop.pdl").read_text()
        self.assertIn("iState enabled 0b1\n", output)
        self.assertIn("iState mode 0b1\n", output)
        self.assertIn("iWrite core.r 0b01011001\n", output)
        self.run_tool(*inputs, "--pdl", "out/AccessTop.pdl", "--output", "again.pdl")
        self.assertEqual(output, (self.root / "again.pdl").read_text())
        self.run_tool("--icl", "out/debug/AccessTop.icl", "--pdl", "source.pdl", "--output", "flat.pdl")
        self.assertEqual(output, (self.root / "flat.pdl").read_text())
        for target in ("core.mode", "core.select", "core.so"):
            with self.subTest(target=target):
                (self.root / "bad.pdl").write_text(f"iReset; iState {target} 1\n")
                for design in (inputs, ("--icl", "out/debug/AccessTop.icl")):
                    result = self.run_tool(*design, "--pdl", "bad.pdl", "--output", "bad-result.pdl", ok=False)
                    self.assertIn("bad.pdl", result.stdout)
                    self.assertIn("top-level", result.stdout)
                    self.assertFalse((self.root / "bad-result.pdl").exists())

    def test_enumerated_operands(self):
        fixture = pathlib.Path(__file__).resolve().parents[1] / "cases/ours/pattern_generation/enumerated_values.icl"
        (self.root / "source.pdl").write_text(
            "iProcsForModule EnumChild\niProc configure {value} {iWrite r $value; iApply}\n"
            "iProcsForModule EnumTop\niCall a.configure Active\niCall b.configure Active\n"
            "iWrite a.r 0xf\niApply\niWrite a.r Mask\niApply\n"
            "iRead a.r Mask\niApply\niWrite a.complement Active\niApply\n"
            "iWrite a.r(2:3) Clear\niApply\niRead b.r Active\niRead a.r AllX\niApply\n")
        inputs = ("--icl", fixture)
        self.run_tool(*inputs, "--pdl", "source.pdl", "--debug")
        output = (self.root / "out/EnumTop.pdl").read_text()
        self.assertIn("iWrite a.r 0b0101\n", output)
        self.assertIn("iWrite b.r 0b1010\n", output)
        self.assertIn("iWrite a.r 0b1101\n", output)
        self.assertIn("iRead a.r 0bxx01\n", output)
        self.assertIn("iWrite a.r 0b0100\n", output)
        self.assertIn("iRead b.r 0b1010\n", output)
        self.assertIn("iRead a.r 0bxxxx\n", output)
        self.run_tool(*inputs, "--pdl", "out/EnumTop.pdl", "--output", "again.pdl")
        self.assertEqual(output, (self.root / "again.pdl").read_text())
        self.run_tool("--icl", "out/debug/EnumTop.icl", "--pdl", "source.pdl", "--output", "flat.pdl")
        self.assertEqual(output, (self.root / "flat.pdl").read_text())
        for command, message in (
                ("iWrite a.r Missing", "unknown enum value"),
                ("iRead a.unnamed Active", "no RefEnum"),
                ("iWrite a.r(2:3) Active", "truncates significant bits")):
            with self.subTest(command=command):
                (self.root / "bad.pdl").write_text(command + "; iApply\n")
                result = self.run_tool(*inputs, "--pdl", "bad.pdl", "--output", "bad-result.pdl", ok=False)
                self.assertIn("bad.pdl", result.stdout)
                self.assertIn(message, result.stdout)
                self.assertFalse((self.root / "bad-result.pdl").exists())

    def test_idle_rejections(self):
        fixture = pathlib.Path(__file__).resolve().parents[1] / "cases/ours/pattern_generation/access_link"
        inputs = ("--icl", fixture / "simple.icl", "--bsdl", "AccessProbe", fixture / "simple.bsdl")
        for source, message in (
                ("iRunLoop 1", "explicit iReset"),
                ("iReset; iWrite core.r 1; iRunLoop 1", "unapplied"),
                ("iReset; iRunLoop -time 10ns", "absolute period models"),
                ("iReset; iRunLoop 1 -sck clock", "unknown PDL target clock"),
                ("iReset; iRunLoop 0xX00000000000000000", "must be known"),
                ("iReset; iRunLoop 18446744073709551616", "truncates significant bits")):
            with self.subTest(source=source):
                (self.root / "source.pdl").write_text(source + "\n")
                result = self.run_tool(*inputs, "--pdl", "source.pdl", "--output", "failed.pdl", ok=False)
                self.assertIn(message, result.stdout)
                self.assertFalse((self.root / "failed.pdl").exists())
        (self.root / "source.pdl").write_text("iRunLoop 1\n")
        self.run_tool("--icl", "source.icl", "--pdl", "source.pdl")
        self.assertIn("iRunLoop 1 -tck\n", (self.root / "out/Network.pdl").read_text())

    def test_input_aliases_and_failure_publication(self):
        source = (self.root / "source.pdl").read_bytes()
        self.run_tool("--icl", "source.icl", "--pdl", "source.pdl", "--output", "source.pdl", ok=False)
        self.assertEqual(source, (self.root / "source.pdl").read_bytes())
        os.link(self.root / "source.pdl", self.root / "link.pdl")
        self.run_tool("--icl", "source.icl", "--pdl", "source.pdl", "--log", "link.pdl", ok=False)
        self.assertEqual(source, (self.root / "source.pdl").read_bytes())
        (self.root / "bad.pdl").write_text("iWrite missing 1; iApply\n")
        self.run_tool("--icl", "source.icl", "--pdl", "bad.pdl", "--output", "failed.pdl", ok=False)
        self.assertFalse((self.root / "failed.pdl").exists())

    def test_reset_and_ordered_redefinition(self):
        source = ICL.replace("ScanInPort si;", "ScanInPort si; ResetPort rst;", 1).replace(
            "ScanRegister gate { ScanInSource si; }",
            "ScanRegister gate { ScanInSource si; ResetValue 1'b0; }")
        (self.root / "source.icl").write_text(source)
        (self.root / "source.pdl").write_text(PDL.replace("iState gate 0", "iReset"))
        (self.root / "later.pdl").write_text(
            "iProcsForModule Network\niProc write_value {value} {iWrite target 3; iApply}\niCall write_value 10\n")
        result = self.run_tool("--icl", "source.icl", "--pdl", "source.pdl", "later.pdl")
        self.assertIn("overrides", result.stdout)
        original = (self.root / "out/Network.pdl").read_text()
        self.assertLess(original.index("0b1010"), original.index("0b0011"))
        self.assertIn("iReset\n", original)
        self.run_tool("--icl", "source.icl", "--pdl", "out/Network.pdl", "--output", "replay.pdl")
        self.assertEqual(original, (self.root / "replay.pdl").read_text())

    def test_read_obligations_and_staging(self):
        (self.root / "source.pdl").write_text(
            "iState gate 1\niRead target 0b1010\niWrite target 3\niApply\niRead target\niApply\n")
        self.run_tool("--icl", "source.icl", "--pdl", "source.pdl")
        original = (self.root / "out/Network.pdl").read_text()
        self.assertIn("iRead target 0b1010", original)
        self.assertIn("iRead target 0bxxxx", original)
        self.run_tool("--icl", "source.icl", "--pdl", "out/Network.pdl", "--output", "replay.pdl")
        self.assertEqual(original, (self.root / "replay.pdl").read_text())
        # A staging failure must leave an existing debug artifact untouched.
        (self.root / "bad-output.pdl").mkdir()
        (self.root / "debug").mkdir()
        debug = self.root / "debug/Network.icl"
        debug.write_text("previous artifact\n")
        result = self.run_tool("--icl", "source.icl", "--pdl", "source.pdl",
                               "--output", "bad-output.pdl", "--debug", ok=False)
        self.assertNotIn("info: generated", result.stdout)
        self.assertEqual(debug.read_text(), "previous artifact\n")
        self.assertFalse(list((self.root / "debug").glob(".icl-toolsets-*")))


if __name__ == "__main__":
    unittest.main()
