#!/usr/bin/env python3
"""Reset prerequisites after side-effecting main-command arguments."""
import re
import time
import unittest

import test_compound_reset_sources as helpers


class ArgumentResetSources(unittest.TestCase):
    setUp = helpers.CompoundResetSources.setUp
    run_tool = helpers.CompoundResetSources.run_tool

    def check(self, source, icl=helpers.FIXTURE):
        (self.root / 'source.pdl').write_text(source)
        self.run_tool('--icl', icl, '--pdl', 'source.pdl',
                      '--output', 'out.pdl', '--local_reset', 'reset.pdl', '--debug')
        rewritten = (self.root / 'reset.pdl').read_text()
        self.assertIn('apply ', rewritten)
        self.run_tool('--icl', icl, '--pdl', 'reset.pdl',
                      '--output', 'reset-pattern.pdl', '--local_reset', 'again.pdl')
        self.assertEqual(rewritten, (self.root / 'again.pdl').read_text())
        for pattern in ('out.pdl', 'reset-pattern.pdl'):
            self.run_tool('--icl', icl, '--pdl', pattern,
                          '--output', 'fixed.pdl')
            self.assertEqual((self.root / pattern).read_bytes(),
                             (self.root / 'fixed.pdl').read_bytes())
        self.run_tool('--icl', 'debug/Network.icl', '--pdl', 'source.pdl',
                      '--output', 'flat.pdl', '--local_reset', 'flat-reset.pdl')
        self.assertEqual((self.root / 'out.pdl').read_bytes(),
                         (self.root / 'flat.pdl').read_bytes())
        self.assertEqual(rewritten, (self.root / 'flat-reset.pdl').read_text())
        return rewritten, (self.root / 'reset-pattern.pdl').read_text()

    def opposing_resets(self):
        path = self.root / 'opposing.icl'
        path.write_text("""Module Cell {
 ScanInPort si; ResetPort rst;
 ScanRegister value[3:0] { ScanInSource si; CaptureSource value; ResetValue 0; }
 ScanOutPort so { Source value[0]; }
}
Module Network {
 ScanInPort si; ResetPort rst;
 ScanRegister control { ScanInSource si; ResetValue 0; }
 DataMux reset_a SelectedBy control { 1'b0: rst; 1'b1: 1'b1; }
 DataMux reset_b SelectedBy control { 1'b0: 1'b1; 1'b1: rst; }
 Instance a Of Cell { InputPort si = control; InputPort rst = reset_a; }
 Instance b Of Cell { InputPort si = a.so; InputPort rst = reset_b; }
 ScanOutPort so { Source b.so; }
}
""")
        return path

    def physical_fixture(self):
        icl = self.opposing_resets()
        source = icl.read_text().replace('Module Network {', 'Module Core {')
        source = source.replace('ScanInPort si; ResetPort rst;\n ScanRegister control',
                                'ScanInPort si; ResetPort rst;\n'
                                ' SelectPort sel; CaptureEnPort ce; ShiftEnPort se;\n'
                                ' UpdateEnPort ue; TCKPort tck;\n ScanRegister control')
        source = source.replace('Source b.so; }\n}', 'Source b.so; }\n'
                                ' ScanInterface client { Port si; Port so; Port sel; '
                                'Port ce; Port se; Port ue; Port tck; }\n}')
        source += """Module Network {
 TRSTPort TRST;
 Instance core Of Core { InputPort rst=~TRST; }
 AccessLink tap Of STD_1149_1_2001 {
  BSDLEntity AccessProbe;
  IJTAG_ON { ScanInterface { core.client; } }
  IJTAG_OFF { ScanInterface { core.client; } }
 }
}
"""
        icl.write_text(source)
        bsdl = helpers.FIXTURE.parents[1] / 'access_link/simple.bsdl'
        # Use the existing AccessProbe BSDL fixture.
        self.assertIn('entity AccessProbe', bsdl.read_text())
        commands = ('iPDLLevel 0\niProcsForModule Network\niReset\n'
                    'iWrite tap.INSTRUCTION 4; iRead tap.INSTRUCTION 1; iApply\n'
                    'iWrite core.control 1; iWrite core.a.value 0; '
                    'iRead core.a.value 0; iRead core.b.value 0; iApply\n'
                    'iWrite core.control 1; iWrite core.b.value 3; '
                    'iRead core.a.value 0; iRead core.b.value 0; iApply\n'
                    'iRead core.b.value 3; iApply\n'
                    'iWrite core.control 0; iWrite core.b.value 3; '
                    'iRead core.a.value 0; iRead core.b.value 3; iApply\n'
                    'iWrite core.a.value 3; iRead core.a.value 0; '
                    'iRead core.b.value 0; iApply\n'
                    'iRead core.a.value 3; iRead core.b.value 0; iApply\n')
        (self.root / 'source.pdl').write_text(commands)
        return icl, bsdl, commands

    def test_physical_reset_loads_replay_exactly(self):
        icl, bsdl, commands = self.physical_fixture()
        common = ('--bsdl', 'AccessProbe', bsdl, '--output', 'out.pdl')
        self.run_tool('--icl', icl, '--pdl', 'source.pdl', *common, ok=False)
        self.run_tool('--physical_pdl', '--icl', icl, '--pdl', 'source.pdl',
                      *common, '--debug')
        output = (self.root / 'out.pdl').read_bytes()
        self.assertEqual(output.count(b'iApply'), commands.count('iApply'))
        self.assertIn(b'iWrite core.b.value 0b0011', output)
        for model, pdl in ((icl, 'out.pdl'), ('debug/Network.icl', 'source.pdl')):
            self.run_tool('--physical_pdl', '--icl', model, '--pdl', pdl,
                          '--bsdl', 'AccessProbe', bsdl, '--output', 'again.pdl')
            self.assertEqual(output, (self.root / 'again.pdl').read_bytes())

    def test_physical_compare_failure_preserves_artifacts(self):
        icl, bsdl, commands = self.physical_fixture()
        commands = commands.replace('iRead core.b.value 0; iApply',
                                    'iRead core.b.value 2; iApply', 1)
        (self.root / 'source.pdl').write_text(commands)
        for name in ('out.pdl', 'debug/Network.icl'):
            path = self.root / name
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_text('previous artifact\n')
        result = self.run_tool('--physical_pdl', '--icl', icl, '--pdl', 'source.pdl',
                               '--bsdl', 'AccessProbe', bsdl, '--output', 'out.pdl',
                               '--debug', ok=False)
        self.assertIn('serial bit 1: expected 1, observed 0', result.stdout)
        for name in ('out.pdl', 'debug/Network.icl'):
            self.assertEqual((self.root / name).read_text(), 'previous artifact\n')

    def test_physical_closed_route_reports_target(self):
        icl, bsdl, _ = self.physical_fixture()
        icl.write_text(icl.read_text().replace('ScanOutPort so { Source b.so; }',
            "ScanMux route SelectedBy control { 1'b0: control; 1'b1: b.so; }\n"
            ' ScanOutPort so { Source route; }'))
        # Ordinary native probing excludes locally wired reset networks.
        icl.write_text(icl.read_text().replace(' ResetPort rst;', '')
            .replace('InputPort rst = reset_a;', '')
            .replace('InputPort rst = reset_b;', '')
            .replace('InputPort rst=~TRST;', '')
            .replace(" DataMux reset_a SelectedBy control { 1'b0: rst; 1'b1: 1'b1; }", '')
            .replace(" DataMux reset_b SelectedBy control { 1'b0: 1'b1; 1'b1: rst; }", ''))
        (self.root / 'source.pdl').write_text(
            'iPDLLevel 0\niReset\niWrite tap.INSTRUCTION 4; iApply\n'
            'iWrite core.b.value 3; iApply\n')
        for name in ('out.pdl', 'debug/Network.icl'):
            path = self.root / name
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_text('previous artifact\n')
        result = self.run_tool('--physical_pdl', '--icl', icl, '--pdl', 'source.pdl',
                               '--bsdl', 'AccessProbe', bsdl, '--output', 'out.pdl',
                               '--debug', ok=False)
        self.assertIn('target outside current TAP route: core.b.value', result.stdout)
        for name in ('out.pdl', 'debug/Network.icl'):
            self.assertEqual((self.root / name).read_text(), 'previous artifact\n')
        self.run_tool('--icl', icl, '--pdl', 'source.pdl', '--bsdl',
                      'AccessProbe', bsdl, '--output', 'ordinary.pdl')
        self.assertGreater((self.root / 'ordinary.pdl').read_text().count('iApply'), 2)

    def test_physical_route_trace_and_work_limit_causes(self):
        for kind, message in (
                ('work', 'physical TAP scan event-work limit exceeded'),
                ('route', 'unknown or disconnected replay scan path')):
            with self.subTest(kind=kind):
                icl, bsdl, _ = self.physical_fixture()
                source = icl.read_text()
                commands = 'iReset\niWrite tap.INSTRUCTION 4; iApply\n'
                if kind == 'work':
                    source = source.replace('ScanRegister value[3:0]',
                                            'ScanRegister value[1023:0]')
                else:
                    source = source.replace(
                        'ScanRegister control { ScanInSource si; ResetValue 0; }',
                        'ScanRegister control { ScanInSource si; }')
                    source = source.replace('ScanOutPort so { Source b.so; }',
                        "ScanMux route SelectedBy control { 1'b0: control; 1'b1: b.so; }\n"
                        ' ScanOutPort so { Source route; }')
                    commands += 'iWrite core.b.value 3; iApply\n'
                icl.write_text(source)
                (self.root / 'source.pdl').write_text(commands)
                for name in ('out.pdl', 'debug/Network.icl'):
                    path = self.root / name
                    path.parent.mkdir(parents=True, exist_ok=True)
                    path.write_text('previous artifact\n')
                result = self.run_tool('--physical_pdl', '--icl', icl,
                    '--pdl', 'source.pdl', '--bsdl', 'AccessProbe', bsdl,
                    '--output', 'out.pdl', '--debug', ok=False)
                self.assertIn(message, result.stdout)
                for name in ('out.pdl', 'debug/Network.icl'):
                    self.assertEqual((self.root / name).read_text(), 'previous artifact\n')

    def test_physical_mode_rejects_source_rewrite(self):
        icl, bsdl, _ = self.physical_fixture()
        result = self.run_tool('--physical_pdl', '--icl', icl, '--pdl', 'source.pdl',
                               '--bsdl', 'AccessProbe', bsdl, '--local_reset', 'reset.pdl',
                               ok=False)
        self.assertIn('--physical_pdl cannot use --local_reset', result.stdout)

    def test_physical_unknown_loads_and_extra_commands_rejected(self):
        icl, bsdl, commands = self.physical_fixture()
        cases = (
            (commands.replace('iWrite core.b.value 3', 'iWrite core.b.value 0b00x1', 1),
             'binary scan loads'),
            (commands + 'iRunLoop 1 -tck\n', 'unsupported operation in physical PDL'),
            (commands + 'iState core.control 0\n', 'unsupported operation in physical PDL'),
        )
        for source, message in cases:
            with self.subTest(message=message):
                (self.root / 'source.pdl').write_text(source)
                result = self.run_tool('--physical_pdl', '--icl', icl,
                                       '--pdl', 'source.pdl', '--bsdl', 'AccessProbe',
                                       bsdl, '--output', 'out.pdl', ok=False)
                self.assertIn(message, result.stdout)

    def test_physical_mode_requires_tap_and_no_end_states(self):
        icl, bsdl, _ = self.physical_fixture()
        icl.write_text(icl.read_text().replace('Module Network {',
                      "Module Network { Alias idle = core.control { iApplyEndState 1'b0; }"))
        result = self.run_tool('--physical_pdl', '--icl', icl, '--pdl', 'source.pdl',
                               '--bsdl', 'AccessProbe', bsdl, ok=False)
        self.assertIn('cannot use iApplyEndState', result.stdout)
        icl.write_text('Module Network { DataInPort pin; }')
        (self.root / 'source.pdl').write_text('iWrite pin 1; iApply\n')
        result = self.run_tool('--physical_pdl', '--icl', icl,
                               '--pdl', 'source.pdl', ok=False)
        self.assertIn('exact physical TAP CSU', result.stdout)

    def test_physical_repeated_reset_retains_unreset_fill(self):
        icl, bsdl, _ = self.physical_fixture()
        icl.write_text(icl.read_text().replace('InputPort rst = reset_a;',
                                              "InputPort rst = 1'b0;"))
        for reset in ('iReset', 'iReset -sync'):
            with self.subTest(reset=reset):
                source = ('iPDLLevel 0\niReset\niWrite tap.INSTRUCTION 4; iApply\n'
                          'iWrite core.a.value 15; iApply\n' + reset +
                          '\niWrite tap.INSTRUCTION 4; iApply\n'
                          'iRead core.a.value 15; iApply\n'
                          'iRead core.a.value 15; iApply\n')
                (self.root / 'source.pdl').write_text(source)
                self.run_tool('--physical_pdl', '--icl', icl, '--pdl', 'source.pdl',
                              '--bsdl', 'AccessProbe', bsdl, '--output', 'out.pdl',
                              '--debug')
                output = (self.root / 'out.pdl').read_bytes()
                for model, pdl in ((icl, 'out.pdl'), ('debug/Network.icl', 'source.pdl')):
                    self.run_tool('--physical_pdl', '--icl', model, '--pdl', pdl,
                                  '--bsdl', 'AccessProbe', bsdl, '--output', 'again.pdl')
                    self.assertEqual(output, (self.root / 'again.pdl').read_bytes())

    def test_physical_bypassed_unknown_update_invalidates_fill(self):
        icl, bsdl, _ = self.physical_fixture()
        source = icl.read_text()
        begin = source.index('Module Core {')
        end = source.index('Module Network {', begin)
        source = """Module Core {
 ScanInPort si; ResetPort rst;
 SelectPort sel; CaptureEnPort ce; ShiftEnPort se; UpdateEnPort ue; TCKPort tck;
 DataMux pick[3:0] SelectedBy control { 1'b0: 4'bxxxx; 1'b1: b; }
 ScanRegister b[3:0] { CaptureSource pick; ScanInSource si; ResetValue 0; }
 ScanMux route SelectedBy control { 1'b0: si; 1'b1: b[0]; }
 ScanRegister control { ScanInSource route; CaptureSource control; ResetValue 0; }
 ScanOutPort so { Source control; }
 ScanInterface client { Port si; Port so; Port sel; Port ce; Port se; Port ue; Port tck; }
}
""" + source[end:]
        icl.write_text(source)
        head = ('iPDLLevel 0\niReset\niWrite tap.INSTRUCTION 4; iApply\n'
                'iWrite core.control 1; iApply\n')
        (self.root / 'source.pdl').write_text(head + 'iRead core.control 1; iApply\n')
        for name in ('out.pdl', 'debug/Network.icl'):
            path = self.root / name
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_text('previous artifact\n')
        result = self.run_tool('--physical_pdl', '--icl', icl, '--pdl', 'source.pdl',
                               '--bsdl', 'AccessProbe', bsdl, '--output', 'out.pdl',
                               '--debug', ok=False)
        self.assertIn('scan load is not fully binary: core.b (1xxx)', result.stdout)
        for name in ('out.pdl', 'debug/Network.icl'):
            self.assertEqual((self.root / name).read_text(), 'previous artifact\n')
        explicit = (head + 'iWrite core.b 15; iRead core.b 0b1xxx; '
                    'iRead core.control 1; iApply\niRead core.b 15; iApply\n')
        (self.root / 'source.pdl').write_text(explicit)
        self.run_tool('--physical_pdl', '--icl', icl, '--pdl', 'source.pdl',
                      '--bsdl', 'AccessProbe', bsdl, '--output', 'out.pdl', '--debug')
        output = (self.root / 'out.pdl').read_bytes()
        self.assertEqual(output.count(b'iApply'), 4)
        for model, pdl in ((icl, 'out.pdl'), ('debug/Network.icl', 'source.pdl')):
            self.run_tool('--physical_pdl', '--icl', model, '--pdl', pdl,
                          '--bsdl', 'AccessProbe', bsdl, '--output', 'again.pdl')
            self.assertEqual(output, (self.root / 'again.pdl').read_bytes())

    def test_varying_reset_domains_at_one_write_site(self):
        source = ('iPDLLevel 1\niReset\n'
                  'foreach target {b a} {\n'
                  ' iWrite ${target}.value 3; iApply\n'
                  ' iRead ${target}.value 3; iApply\n}\n')
        rewritten, output = self.check(source, self.opposing_resets())
        self.assertIn('switch -exact', rewritten)
        self.assertIn('iRead b.value 0b0011', output)
        self.assertIn('iRead a.value 0b0011', output)

    def test_varying_call_arguments_preserve_definitions(self):
        definition = ('iProcsForModule Cell\niProc default {value token} {\n'
                      ' # Preserve this Unicode and literal body: λ\n'
                      ' if {$token ne {literal; [iReset]; $missing 🧪}} {error changed_token}\n'
                      ' iWrite value $value; iApply; iRead value $value; iApply\n}\n')
        source = ('iPDLLevel 1\n' + definition + 'iProcsForModule Network\niReset\n'
                  'set count 0\nset token {literal; [iReset]; $missing 🧪}\n'
                  'foreach target {b a} {\n'
                  ' ::iCall ${target}.default [incr count; expr 3] $token\n}\n'
                  'if {$count != 2} {error duplicate_arguments}\n')
        rewritten, output = self.check(source, self.opposing_resets())
        self.assertIn(definition, rewritten)
        self.assertIn('switch -exact', rewritten)
        self.assertIn('iRead b.value 0b0011', output)
        self.assertIn('iRead a.value 0b0011', output)

        # A literal default key must remain distinct from switch's final fallback.
        source = ('iPDLLevel 1\niProcsForModule Network\n'
                  'iProc default {} {iWrite b.value 3; iApply}\n'
                  'iProc other {} {iWrite a.value 3; iApply}\niReset\n'
                  'foreach command {default other} {iCall $command}\n')
        rewritten, _ = self.check(source, self.opposing_resets())
        self.assertIn('switch -exact', rewritten)

    def test_varying_prefixed_vector_arguments(self):
        source = ('iPDLLevel 1\niReset\nset count 0\n'
                  'foreach target {b a} {\n iPrefix $target\n'
                  ' iWrite value[1:0] [incr count]; iApply\n'
                  ' iRead value[1:0] $count; iApply\n}\niPrefix -\n'
                  'if {$count != 2} {error duplicate_arguments}\n')
        rewritten, output = self.check(source, self.opposing_resets())
        self.assertIn('switch -exact', rewritten)
        self.assertIn('iRead b.value 0bxx01', output)
        self.assertIn('iRead a.value 0bxx10', output)

    def test_varying_calls_across_input_files(self):
        definition = ('iPDLLevel 1\niProcsForModule Cell\n'
                      'iProc put {value} {iWrite value $value; iApply}\n')
        body = ('iProcsForModule Network\niReset\n'
                'foreach target {b a} {iCall ${target}.put 3}\n')
        icl = self.opposing_resets()
        (self.root / 'first.pdl').write_text(definition)
        (self.root / 'second.pdl').write_text(body)
        self.run_tool('--icl', icl, '--pdl', 'first.pdl', 'second.pdl',
                      '--output', 'out.pdl', '--local_reset', 'reset.pdl', '--debug')
        rewritten = (self.root / 'reset.pdl').read_text()
        expected = (self.root / 'out.pdl').read_text()
        self.assertTrue(rewritten.startswith(definition))
        self.assertIn('switch -exact', rewritten)
        again, output = self.check(rewritten, icl)
        self.assertEqual(rewritten, again)
        self.assertEqual(expected, output)

    def test_identical_arguments_follow_recorded_release_order(self):
        for procedure in (False, True):
            with self.subTest(procedure=procedure):
                declaration = ('iProc put {} {iWrite b.value 3; iApply}\n'
                               if procedure else '')
                command = 'iCall put' if procedure else 'iWrite b.value 3; iApply'
                source = ('iPDLLevel 1\niProcsForModule Network\n' + declaration +
                          'iReset\nforeach state {0 1 0 1} {\n'
                          ' iWrite control $state; iApply\n ' + command + '\n}\n')
                rewritten, output = self.check(source, self.opposing_resets())
                self.assertIn('IclPatternReset', rewritten)
                self.assertGreaterEqual(output.count('iWrite b.value 0b0011'), 4)
                if procedure:
                    self.assertIn(declaration, rewritten)

    def test_invocation_order_includes_empty_and_caught_calls(self):
        declaration = ('iProc put {} {\n'
                       ' if {$::phase == 1} {return empty}\n'
                       ' iWrite b.value 3; iApply\n'
                       ' if {$::phase == 2} {error expected_marker}\n}\n')
        source = ('iPDLLevel 1\niProcsForModule Network\n' + declaration +
                  'iReset\nforeach phase {0 1 2 3} {\n'
                  ' iWrite control [expr {$phase % 2}]; iApply\n'
                  ' set rc [catch {iCall put} result]\n'
                  ' if {$phase == 1 && ($rc || $result ne "empty")} {error wrong_return}\n'
                  ' if {$phase == 2 && (!$rc || ![string match {*expected_marker*} $result])} {error wrong_error}\n'
                  '}\n')
        rewritten, _ = self.check(source, self.opposing_resets())
        self.assertIn(declaration, rewritten)
        self.assertIn('IclPatternReset', rewritten)

    def test_invocation_arguments_evaluate_once_before_consumption(self):
        declaration = 'iProc put {value} {iWrite b.value $value; iApply}\n'
        source = ('iPDLLevel 1\niProcsForModule Network\n' + declaration +
                  'iReset\nset evaluated 0\nforeach phase {0 1 2 3} {\n'
                  ' iWrite control [expr {$phase % 2}]; iApply\n'
                  ' set rc [catch {iCall put [if {$phase == 1} {error argument_marker} '
                  'else {incr evaluated; expr 3}]} result]\n'
                  ' if {$phase == 1 && (!$rc || ![string match {*argument_marker*} $result])} {error wrong_error}\n'
                  '}\nif {$evaluated != 3} {error repeated_argument}\n')
        rewritten, _ = self.check(source, self.opposing_resets())
        self.assertIn(declaration, rewritten)
        self.assertIn('IclPatternReset', rewritten)

    def test_invocation_namespace_cleanup_and_interference_guards(self):
        source = ('iPDLLevel 1\nnamespace eval IclPatternReset0 {variable user keep0}\n'
                  'namespace eval IclPatternReset1 {variable user keep1}\niReset\n'
                  'foreach state {0 1 0 1} {\n'
                  ' iWrite control $state; iApply\n iWrite b.value 3; iApply\n}\n'
                  'if {$::IclPatternReset0::user ne "keep0" || '
                  '$::IclPatternReset1::user ne "keep1"} {error user_state}\n')
        icl = self.opposing_resets()
        rewritten, _ = self.check(source, icl)
        helper = re.search(r'namespace exists (::IclPatternReset\d+)', rewritten).group(1)
        self.assertEqual(helper, '::IclPatternReset2')
        (self.root / 'clean.pdl').write_text(
            rewritten + f'\niPDLLevel 1\nif {{[namespace exists {helper}]}} {{error leaked_helper}}\n')
        self.run_tool('--icl', icl, '--pdl', 'clean.pdl', '--output', 'clean-out.pdl')
        variations = [
            (rewritten.replace('foreach state {0 1 0 1}', 'foreach state {0 1 0 1 0}'),
             'unexpected local-reset invocation'),
            (rewritten.replace('foreach state {0 1 0 1}', 'foreach state {0 1}'),
             'missing local-reset invocation'),
            (rewritten.replace('iWrite b.value 3; iApply', 'iWrite b.value 2; iApply'),
             'unrecognized local-reset caller arguments'),
            (f'namespace eval {helper} {{variable user keep}}\n' + rewritten,
             'local-reset namespace collision'),
            (rewritten.replace('foreach state', f'set {helper}::user keep\nforeach state'),
             'local-reset namespace interference'),
        ]
        for modified, diagnostic in variations:
            with self.subTest(diagnostic=diagnostic):
                (self.root / 'modified.pdl').write_text(modified)
                (self.root / 'protected.pdl').write_text('previous output\n')
                result = self.run_tool('--icl', icl, '--pdl', 'modified.pdl',
                                       '--output', 'protected.pdl', ok=False)
                self.assertIn(diagnostic, result.stdout + result.stderr)
                self.assertEqual((self.root / 'protected.pdl').read_text(), 'previous output\n')

    def test_invocation_order_crosses_files_and_restores_level0_eof(self):
        declaration = 'iProc put {} {iWrite b.value 3; iApply}\n'
        first = 'iPDLLevel 1\niProcsForModule Network\n' + declaration + '# continued ' + chr(92)
        second = ('iReset\nforeach state {0 1 0 1} {\n'
                  ' iWrite control $state; iApply\n iCall put\n}\niPDLLevel 0\n')
        (self.root / 'first.pdl').write_text(first)
        (self.root / 'second.pdl').write_text(second)
        icl = self.opposing_resets()
        self.run_tool('--icl', icl, '--pdl', 'first.pdl', 'second.pdl',
                      '--output', 'out.pdl', '--local_reset', 'reset.pdl', '--debug')
        rewritten = (self.root / 'reset.pdl').read_text()
        self.assertIn(declaration, rewritten)
        self.assertTrue(rewritten.endswith('iPDLLevel 0\n'))
        self.run_tool('--icl', icl, '--pdl', 'reset.pdl',
                      '--output', 'replay.pdl', '--local_reset', 'again.pdl')
        self.assertEqual(rewritten, (self.root / 'again.pdl').read_text())
        self.run_tool('--icl', 'debug/Network.icl', '--pdl', 'first.pdl', 'second.pdl',
                      '--output', 'flat.pdl', '--local_reset', 'flat-reset.pdl')
        self.assertEqual(rewritten, (self.root / 'flat-reset.pdl').read_text())
        (self.root / 'level1-command.pdl').write_text('set forbidden 1\n')
        result = self.run_tool('--icl', icl, '--pdl', 'reset.pdl', 'level1-command.pdl',
                               '--output', 'protected.pdl', ok=False)
        self.assertIn('level-0', result.stdout + result.stderr)

    def test_invocation_preserves_prefixed_vector_argument_effects(self):
        source = ('iPDLLevel 1\niReset\nset evaluated 0\n'
                  'foreach state {0 1 0 1} {\n'
                  ' iPrefix -; iWrite control $state; iApply\n'
                  ' iPrefix b\n'
                  ' iWrite value[3:0] [incr evaluated; expr 3]; iApply\n'
                  '}\niPrefix -\nif {$evaluated != 4} {error repeated_argument}\n')
        rewritten, _ = self.check(source, self.opposing_resets())
        self.assertIn('IclPatternReset', rewritten)
        self.assertIn('value\\[3:0\\]', rewritten)

    def test_invocation_record_scale(self):
        sizes = []
        for count in (64, 256, 512):
            source = ('iPDLLevel 1\niReset\n'
                      f'for {{set state 0}} {{$state < {count}}} {{incr state}} {{\n'
                      ' iWrite control [expr {$state % 2}]; iApply\n'
                      ' iWrite b.value 3; iApply\n}\n')
            started = time.monotonic()
            rewritten, _ = self.check(source, self.opposing_resets())
            sizes.append(len(rewritten))
            self.assertIn('IclPatternReset', rewritten)
            self.assertLess(rewritten.count('iWrite control 0b1'), 5)
            print(f'reset invocation dispatch: calls={count} source_bytes={len(source)} '
                  f'rewrite_bytes={len(rewritten)} five_runs_seconds={time.monotonic() - started:.6f}s',
                  flush=True)
        self.assertLess(sizes[2] - sizes[1], 4 * (512 - 256))

    def test_dispatch_preserves_queue_and_reservation_rejection(self):
        for source in [
                'iPDLLevel 1\niReset\nforeach target {b a} {iWrite ${target}.value 3}\niApply\n',
                'iPDLLevel 1\niReset\niTake control\niWrite control 0\niApply\n'
                'foreach target {b a} {iWrite ${target}.value 3; iApply}\n']:
            with self.subTest(source=source):
                (self.root / 'source.pdl').write_text(source)
                for name in ['out.pdl', 'reset.pdl']:
                    (self.root / name).write_text('previous artifact\n')
                self.run_tool('--icl', self.opposing_resets(), '--pdl', 'source.pdl',
                              '--output', 'out.pdl', '--local_reset', 'reset.pdl', ok=False)
                for name in ['out.pdl', 'reset.pdl']:
                    self.assertEqual((self.root / name).read_text(), 'previous artifact\n')

    def test_empty_and_caught_error_call_signatures(self):
        for partial in [False, True]:
            with self.subTest(partial=partial):
                failure = '; error expected_marker' if partial else ''
                source = ('iPDLLevel 1\niProcsForModule Network\n'
                          f'iProc put_b {{}} {{iWrite b.value 3; iApply{failure}}}\n'
                          'iProc no_op {} {return skipped}\n'
                          'iProc put_a {} {iWrite a.value 3; iApply}\niReset\n'
                          'foreach command {put_b no_op missing put_a} {\n'
                          ' set rc [catch {iCall $command} result]\n'
                          ' if {$command eq "no_op" && ($rc || $result ne "skipped")} {error wrong_return}\n'
                          ' if {$command eq "missing" && (!$rc || ![string match {*undefined iProc*} $result])} {error wrong_error}\n'
                          + (' if {$command eq "put_b" && (!$rc || ![string match {*expected_marker*} $result])} {error wrong_partial_error}\n' if partial else '')
                          + '}\n')
                rewritten, output = self.check(source, self.opposing_resets())
                self.assertIn('switch -exact', rewritten)
                self.assertIn('iWrite b.value 0b0011', output)
                self.assertIn('iWrite a.value 0b0011', output)

    def test_rewritten_reservation_ownership_is_checked(self):
        icl = self.opposing_resets()
        icl.write_text(icl.read_text().replace(
            'ScanOutPort so { Source b.so; }',
            'Instance r Of ResourceNetwork { InputPort si = b.so; } '
            'ScanOutPort so { Source r.so; }'))
        resource = helpers.FIXTURE.parent.parent / 'resources/network.icl'
        source = ('iPDLLevel 1\niProcsForModule Cell\n'
                  'iProc put {} {set ::depth [info frame]; iWrite value 3; iApply}\n'
                  'iProcsForModule Network\n'
                  'iProc scoped {} {iTake r.gate; iWrite r.gate 0; iApply}\n'
                  'iProc baseline {} {set ::baseline [info frame]}\niReset\nforeach unused {baseline} {iCall baseline}\n'
                  'foreach target {b a} {iCall ${target}.put}\n'
                  'if {$::depth > $::baseline} {iTake r.gate; iWrite r.gate 0; iApply} else {iCall scoped}\n'
                  'iWrite r.target 5; iApply\n')
        (self.root / 'source.pdl').write_text(source)
        self.run_tool('--icl', icl, resource, '--pdl', 'source.pdl', '--output', 'ordinary.pdl')
        for name in ['out.pdl', 'reset.pdl', 'debug/Network.icl']:
            path = self.root / name
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_text('previous artifact\n')
        failure = self.run_tool('--icl', icl, resource, '--pdl', 'source.pdl',
                                 '--output', 'out.pdl', '--local_reset', 'reset.pdl',
                                 '--debug', ok=False)
        self.assertIn('reserved r.gate', failure.stdout)
        for name in ['out.pdl', 'reset.pdl', 'debug/Network.icl']:
            self.assertEqual((self.root / name).read_text(), 'previous artifact\n')

    def test_varying_caller_scale(self):
        for count in [64, 256]:
            source = ('iPDLLevel 1\niReset\n'
                      f'for {{set i 0}} {{$i < {count}}} {{incr i}} {{\n'
                      ' set target [expr {$i % 2 ? "a" : "b"}]\n'
                      ' iWrite ${target}.value 3; iApply\n}\n')
            started = time.monotonic()
            rewritten, output = self.check(source, self.opposing_resets())
            self.assertIn('switch -exact', rewritten)
            self.assertEqual(output.count('iWrite a.value 0b0011'), count // 2)
            self.assertEqual(output.count('iWrite b.value 0b0011'), count // 2)
            print(f'reset caller dispatch: calls={count} source_bytes={len(source)} '
                  f'rewrite_bytes={len(rewritten)} five_runs_seconds={time.monotonic()-started:.6f}s')

    def test_direct_write_argument_resets(self):
        body = 'iWrite a.value [iReset; expr 3]\niApply\n'
        rewritten, output = self.check('iPDLLevel 1\n' + body)
        self.assertIn(body, rewritten)
        self.assertIn('iWrite a.value 0b0011', output)

    def test_square_bracket_targets_remain_pdl_vectors(self):
        for target, value, expected in (('a.value[0]', 1, '0001'),
                                         ('a.value[1:0]', 3, '0011'),
                                         ('{a.value[1:0]}', 3, '0011'),
                                         ('"a.value[1:0]"', 3, '0011')):
            with self.subTest(target=target):
                source = f'iPDLLevel 1\niWrite {target} [iReset; expr {value}]\niApply\n'
                _, output = self.check(source)
                self.assertIn('iWrite a.value 0b' + expected, output)

    def test_vector_index_evaluates_before_value_argument(self):
        source = ('iPDLLevel 1\nset index 1\n'
                  'iWrite a.value[$index] [iReset; set index 0; expr 1]\niApply\n'
                  'if {$index != 0} {error changed_index}\n')
        _, output = self.check(source)
        self.assertIn('iWrite a.value 0b0010', output)

    def test_expanded_argument_list(self):
        source = 'iPDLLevel 1\niWrite {*}[list a.value [iReset; expr 3]]\niApply\n'
        _, output = self.check(source)
        self.assertIn('iWrite a.value 0b0011', output)

    def test_call_preserves_procedure_bytes(self):
        head = ('iPDLLevel 1\niProcsForModule Network\n'
                'iProc put {value} {\n    # Keep this definition exactly.\n'
                '    iWrite a.value $value\n    iApply\n}\n')
        body = 'iCall put [iReset; expr 10]\n'
        rewritten, output = self.check(head + body)
        self.assertTrue(rewritten.startswith(head))
        self.assertIn(body, rewritten)
        self.assertIn('iWrite a.value 0b1010', output)

    def test_arguments_evaluate_once_in_caller_scope(self):
        source = ('iPDLLevel 1\nset count 0\n'
                  'iWrite [iReset; incr count; set target a.value] [incr count; expr 10]\n'
                  'iApply\n'
                  'if {$count != 2 || $target ne "a.value"} {error changed_arguments}\n'
                  'iWrite b.value $count\niApply\n')
        _, output = self.check(source)
        self.assertIn('iWrite a.value 0b1010', output)
        self.assertIn('iWrite b.value 0b0010', output)

    def test_literal_arguments_do_not_get_substituted_again(self):
        head = ('iPDLLevel 1\niProcsForModule Network\n'
                'iProc put {value token} {\n'
                ' if {$token ne {literal; [iReset]; $missing}} {error changed_literal}\n'
                ' iWrite a.value $value; iApply\n}\n')
        body = 'iCall put [iReset; expr 5] {literal; [iReset]; $missing}\n'
        rewritten, output = self.check(head + body)
        self.assertTrue(rewritten.startswith(head))
        self.assertIn(body, rewritten)
        self.assertIn('iWrite a.value 0b0101', output)

    def test_prefixed_target(self):
        source = 'iPDLLevel 1\niPrefix a\niWrite value [iReset; expr 7]\niApply\n'
        _, output = self.check(source)
        self.assertIn('iWrite a.value 0b0111', output)

    def test_repeated_literal_site(self):
        source = ('iPDLLevel 1\n'
                  'foreach v {1 2 3} {iWrite a.value [iReset; set v]; iApply}\n')
        _, output = self.check(source)
        for value in ('0001', '0010', '0011'):
            self.assertIn('iWrite a.value 0b' + value, output)
        source = ('iPDLLevel 1\n'
                  'foreach bit {0 1} {iWrite a.value[$bit] [iReset; expr 1]; iApply}\n')
        _, output = self.check(source)
        for value in ('0001', '0010'):
            self.assertIn('iWrite a.value 0b' + value, output)

    def test_nested_command_substitution(self):
        source = ('iPDLLevel 1\nset result [iWrite a.value [iReset; expr 3]]\n'
                  'iApply\nif {$result ne ""} {error changed_result}\n')
        _, output = self.check(source)
        self.assertIn('iWrite a.value 0b0011', output)

    def test_immutable_internal_reset_keeps_artifacts(self):
        (self.root / 'source.pdl').write_text(
            'iPDLLevel 1\niProcsForModule Network\n'
            'iProc bad {} {iReset; iWrite a.value 3; iApply}\niReset\niCall bad\n')
        for name in ('out.pdl', 'reset.pdl'):
            (self.root / name).write_text('previous artifact\n')
        self.run_tool('--icl', helpers.FIXTURE, '--pdl', 'source.pdl',
                      '--output', 'out.pdl', '--local_reset', 'reset.pdl', ok=False)
        for name in ('out.pdl', 'reset.pdl'):
            self.assertEqual((self.root / name).read_text(), 'previous artifact\n')

    def test_repeated_argument_reset_scale(self):
        count = 64
        source = ('iPDLLevel 1\nset count 0\n'
                  f'for {{set v 0}} {{$v < {count}}} {{incr v}} {{\n'
                  ' iWrite a.value [iReset; incr count; expr {$v & 15}]; iApply\n'
                  '}\n' + f'if {{$count != {count}}} {{error repeated_evaluation}}\n')
        start = time.monotonic()
        self.check(source)
        print(f'argument resets: {count} iterations, {len(source)} PDL bytes, five runs {time.monotonic() - start:.3f}s')


if __name__ == '__main__':
    unittest.main()
