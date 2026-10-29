#!/usr/bin/env python3
"""Resource ownership and protected retargeting regression."""
import pathlib
import subprocess
import sys
import tempfile
import time
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[3]
FIXTURE = ROOT / 'tests/cases/ours/pattern_generation/resources'
BIN = pathlib.Path(sys.argv.pop(1)).resolve() if len(sys.argv) > 1 else ROOT / 'build/icl-pattern-generation'


class Reservations(unittest.TestCase):
    def setUp(self):
        temp = tempfile.TemporaryDirectory()
        self.addCleanup(temp.cleanup)
        self.root = pathlib.Path(temp.name)
        (self.root / 'source.icl').write_bytes((FIXTURE / 'network.icl').read_bytes())

    def generate(self, body, ok=True, extra=(), binding="ResourceNetwork"):
        (self.root / 'source.pdl').write_text(f'iPDLLevel 1\niProcsForModule {binding}\niReset\n' + body)
        result = subprocess.run([str(BIN), '--icl', 'source.icl', '--pdl', 'source.pdl',
                                 '--output', 'result.pdl', *extra], cwd=self.root,
                                text=True, capture_output=True, timeout=40)
        self.assertEqual(result.returncode == 0, ok, result.stdout + result.stderr)
        return (self.root / 'result.pdl').read_text() if ok else result.stdout

    def test_take_only_does_not_lock(self):
        output = self.generate('iTake gate\niWrite target 5\niApply\n')
        self.assertIn('iWrite gate 0b01', output)
        self.generate('iWrite gate 0; iApply; iTake gate; iWrite target 5; iApply')
        self.generate('iTake gate; iRead gate 0; iApply; iWrite target 5; iApply')

    def test_applied_write_locks_and_release_unlocks(self):
        prefix = 'iTake gate\niWrite gate 0\niApply\n'
        (self.root / 'result.pdl').write_text('prior output')
        failure = self.generate(prefix + 'iWrite target 5\niApply\n', ok=False)
        self.assertIn('reserv', failure)
        self.assertIn('gate', failure)
        self.assertEqual((self.root / 'result.pdl').read_text(), 'prior output')
        output = self.generate(prefix + 'iRelease gate\niWrite target 5\niApply\n')
        self.assertIn('iWrite target 0b0101', output)

    def test_take_in_pending_group_and_explicit_update(self):
        prefix = 'iWrite gate 0\niTake gate\niApply\n'
        self.generate(prefix + 'iWrite target 5\niApply\n', ok=False)
        output = self.generate(prefix + 'iWrite gate 1\niWrite target 5\niApply\n')
        self.assertIn('iWrite gate 0b01', output)
        self.assertIn('iWrite target 0b0101', output)

    def test_parent_child_ownership_and_releases(self):
        definitions = ('iProc own {} {iTake gate; iWrite gate 0; iApply}\n'
                       'iProc release {} {iRelease gate}\n'
                       'iProc access {} {iWrite target 5; iApply}\n')
        self.generate(definitions + 'iCall own\niCall access\n')
        parent = definitions + 'iTake gate\niWrite gate 0\niApply\niCall own\n'
        self.generate(parent + 'iCall access\n', ok=False)
        self.generate(parent + 'iCall release\niCall access\n')

    def test_alias_partial_writes_and_instances(self):
        for write in ('iWrite gate[0] 0', 'iWrite gate_alias 0'):
            with self.subTest(write=write):
                self.generate(f'iTake gate\n{write}\niApply\niWrite target 5\niApply\n', ok=False)
        (self.root / 'source.icl').write_bytes((FIXTURE / 'hierarchy.icl').read_bytes())
        body = 'iTake core\niWrite core.gate 0\niApply\n'
        self.generate(body + 'iWrite core.target 5\niApply\n', ok=False)
        output = self.generate(body + 'iRelease core\niWrite core.target 5\niApply\n',
                               extra=('--debug',))
        result = subprocess.run([str(BIN), '--icl', 'debug/ResourceNetwork.icl',
                                 '--pdl', 'source.pdl', '--output', 'flat.pdl'],
                                cwd=self.root, text=True, capture_output=True, timeout=40)
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        self.assertEqual(output, (self.root / 'flat.pdl').read_text())

    def test_explicit_partial_update_keeps_other_reserved_bits(self):
        output = self.generate('iTake gate; iWrite gate 2; iApply; '
                               'iWrite gate[0] 1; iWrite target 5; iApply')
        self.assertIn('iWrite gate 0b11', output)
        self.assertNotIn('iWrite gate 0b01', output)

    def test_partial_write_only_protects_explicitly_written_bits(self):
        output = self.generate('iTake gate; iWrite gate[1] 1; iApply; '
                               'iWrite target 5; iApply')
        self.assertIn('iWrite gate 0b11', output)

    def test_slice_and_alias_reservations(self):
        path = self.root / 'source.icl'
        path.write_text(path.read_text().replace(
            'Alias gate_alias[1:0] = gate;',
            'Alias gate_alias[1:0] = gate; Alias inverted_high = ~gate[1];'))
        for take, write in (('gate[1]', 'iWrite gate 2'),
                            ('gate_alias[1]', 'iWrite gate 2'),
                            ('inverted_high', 'iWrite inverted_high 0')):
            with self.subTest(take=take):
                output = self.generate(f'iTake {take}; {write}; iApply; '
                                       'iWrite target 5; iApply')
                self.assertIn('iWrite gate 0b11', output)
        error = self.generate('iTake gate_alias[0]; iWrite gate 0; iApply; '
                              'iWrite target 5; iApply', ok=False)
        self.assertIn('reserved gate', error)

    def test_partial_release_and_overlapping_ownership(self):
        prefix = 'iTake gate; iWrite gate 0; iApply; '
        output = self.generate(prefix + 'iRelease gate_alias[0]; iWrite target 5; iApply')
        self.assertIn('iWrite gate 0b01', output)
        output = self.generate('iProc own {} {iTake gate[0]; iWrite gate 0; iApply}\n'
                               'iTake gate[1]; iWrite gate[1] 1; iApply; iCall own; '
                               'iWrite target 5; iApply')
        self.assertIn('iWrite target 0b0101', output)
        # Returning from the child must not release an overlapping parent take.
        error = self.generate('iProc own {} {iTake gate_alias[0]; iWrite gate 0; iApply}\n'
                              'iTake gate[0]; iWrite gate 0; iApply; iCall own; '
                              'iWrite target 5; iApply', ok=False)
        self.assertIn('reserved gate', error)

    def test_reversed_concatenated_aliases_and_ascending_register(self):
        path = self.root / 'source.icl'
        path.write_text(path.read_text().replace(
            'Alias gate_alias[1:0] = gate;',
            'Alias reversed[0:1] = gate[0], ~gate[1]; '
            'Alias duplicate[1:0] = gate[1], gate[1];'))
        for take, write in (('reversed[1]', 'iWrite reversed[1] 0'),
                            ('duplicate', 'iWrite gate[1] 1')):
            with self.subTest(take=take):
                output = self.generate(f'iTake {take}; {write}; iApply; '
                                       'iWrite target 5; iApply')
                self.assertIn('iWrite gate 0b11', output)
        path.write_text(path.read_text().replace('gate[1:0]', 'gate[0:1]')
                        .replace('SelectedBy gate[0]', 'SelectedBy gate[1]')
                        .replace('ScanInSource gate[0]', 'ScanInSource gate[1]')
                        .replace("1'b0: gate[0]", "1'b0: gate[1]"))
        output = self.generate('iTake gate[0]; iWrite gate[0] 1; iApply; '
                               'iWrite target 5; iApply')
        self.assertIn('iWrite gate 0b11', output)

    def test_masked_assumptions_and_unknown_writes(self):
        output = self.generate('iTake gate; iState gate[1] 1; '
                               'iWrite target 5; iApply')
        self.assertIn('iWrite gate 0b11', output)
        output = self.generate('iTake gate[0]; iState gate 0bX1; '
                               'iWrite target 5; iApply')
        self.assertIn('iWrite gate 0b01', output)
        output = self.generate('iTake gate[1]; iWrite gate 0b1X; iApply; '
                               'iWrite target 5; iApply')
        self.assertIn('iWrite gate 0b11', output)
        self.assertIn('unknown', self.generate('iTake gate[1]; iState gate 0bX0', ok=False))

    def test_masked_reset_rewrite_and_replay(self):
        source = (FIXTURE / 'synthetic_reset.icl').read_text().replace(
            "ScanRegister reset { ScanInSource si; ResetValue 1'b1; }",
            "ScanRegister reset[1:0] { ScanInSource si; ResetValue 2'b01; }")
        source = source.replace('= reset;', '= reset[0];').replace(
            'ScanInSource reset;', 'ScanInSource reset[0];').replace(
            'SelectedBy reset {', 'SelectedBy reset[0] {')
        (self.root / 'source.icl').write_text(source)
        for take in ('iTake reset[1];', 'iTake reset[1]; iWrite reset[1] 0; iApply;',
                     'iState reset[1] 0bX; iTake reset[1];'):
            with self.subTest(take=take):
                body = take + 'iWrite a.value 5; iApply; iWrite b 6; iApply'
                output = self.generate(body, extra=('--local_reset', 'reset.pdl', '--debug'))
                reset = (self.root / 'reset.pdl').read_text()
                if 'iWrite' not in take:
                    self.assertIn('iWrite {reset[0]} 0b0', reset)
                    self.assertNotIn('iWrite reset 0b', reset)
                for icl, pdl in [('source.icl', 'reset.pdl'),
                                 ('source.icl', 'result.pdl'),
                                 ('debug/ResourceNetwork.icl', 'source.pdl')]:
                    result = subprocess.run([str(BIN), '--icl', icl, '--pdl', pdl,
                                             '--output', 'replay.pdl'], cwd=self.root,
                                            text=True, capture_output=True, timeout=40)
                    self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
                    self.assertEqual(output, (self.root / 'replay.pdl').read_text())

    def test_reset_preamble_splits_ascending_ranges(self):
        source = (FIXTURE / 'synthetic_reset.icl').read_text().replace(
            "ScanRegister reset { ScanInSource si; ResetValue 1'b1; }",
            "ScanRegister reset[0:7] { ScanInSource si; ResetValue 8'b1; }")
        source = source.replace('= reset;', '= reset[7];').replace(
            'ScanInSource reset;', 'ScanInSource reset[7];').replace(
            'SelectedBy reset {', 'SelectedBy reset[7] {')
        (self.root / 'source.icl').write_text(source)
        body = ('iTake reset[0]; iTake reset[3]; iTake reset[6]; '
                'iWrite a.value 5; iApply; iWrite b 6; iApply')
        output = self.generate(body, extra=('--local_reset', 'reset.pdl'))
        reset = (self.root / 'reset.pdl').read_text()
        for target, value in (('reset[1:2]', '00'), ('reset[4:5]', '00'), ('reset[7]', '0')):
            self.assertIn(f'iWrite {{{target}}} 0b{value}', reset)
        result = subprocess.run([str(BIN), '--icl', 'source.icl', '--pdl', 'reset.pdl',
                                 '--output', 'replay.pdl'], cwd=self.root,
                                text=True, capture_output=True, timeout=40)
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        self.assertEqual(output, (self.root / 'replay.pdl').read_text())

    def test_end_state_restoration_only_conflicts_with_protected_bits(self):
        path = self.root / 'source.icl'
        path.write_text(path.read_text().replace(
            'Alias gate_alias[1:0] = gate;',
            "Alias idle = gate[0] { iApplyEndState 1'b0; }"))
        output = self.generate('iTake gate[1]; iWrite gate 3; iApply')
        self.assertIn('iWrite gate 0b10', output)
        error = self.generate('iTake gate[0]; iWrite gate 3; iApply', ok=False)
        self.assertIn('iApplyEndState conflicts with reserved', error)

    def test_tap_slice_reservations_and_fixed_points(self):
        # The vendor probe uses an ungated hidden target. Our TAP scheduler
        # currently requires off-path storage to be gated; use the existing
        # gated vendor fixture to isolate bit ownership from that limitation.
        source_icl = (FIXTURE / 'vendor.icl').read_text().replace(
            'LogicSignal leaf_select',
            'Alias inverted_high = ~gate[1]; LogicSignal leaf_select')
        (self.root / 'source.icl').write_text(source_icl)
        bsdl = FIXTURE.parent / 'access_link/simple.bsdl'
        for name in ('slice_high', 'slice_low', 'slice_release', 'slice_alias',
                     'slice_partial_write'):
            with self.subTest(name=name):
                source = (FIXTURE / f'{name}.pdl').read_text().replace(
                    'SliceTop', 'ResourceAccessTop').replace('core.target', 'core.leaf.value')
                (self.root / 'source.pdl').write_text(source)
                inputs = ['--icl', 'source.icl', '--bsdl', 'AccessProbe', str(bsdl)]
                result = subprocess.run([str(BIN), *inputs, '--pdl', 'source.pdl',
                                         '--output', 'result.pdl', '--debug'], cwd=self.root,
                                        text=True, capture_output=True, timeout=40)
                self.assertEqual(result.returncode == 0, name != 'slice_low', result.stdout + result.stderr)
                if name == 'slice_low':
                    self.assertIn('reserved core.gate', result.stdout)
                    continue
                output = (self.root / 'result.pdl').read_text()
                for icl, pdl in [('source.icl', 'result.pdl'),
                                 ('debug/ResourceAccessTop.icl', 'source.pdl')]:
                    replay = subprocess.run([str(BIN), '--icl', icl, '--pdl', pdl,
                                             '--bsdl', 'AccessProbe', str(bsdl),
                                             '--output', 'replay.pdl'], cwd=self.root,
                                            text=True, capture_output=True, timeout=40)
                    self.assertEqual(replay.returncode, 0, replay.stdout + replay.stderr)
                    self.assertEqual(output, (self.root / 'replay.pdl').read_text())

    def test_large_masked_register_and_greedy_scheduling(self):
        source = (FIXTURE / 'network.icl').read_text()
        source = source.replace("[1:0]", "[255:0]").replace("2'b00", "256'b0")
        padding = ''.join(f"ScanRegister pad{i} {{ ScanInSource si; ResetValue 1'b0; }}\n"
                          for i in range(32))
        source = source[:source.rfind('}')] + padding + '}\n'
        (self.root / 'source.icl').write_text(source)
        body = ('iTake gate; iTake gate[255:1]; iWrite gate[255:1] 1; iApply; '
                'iRelease gate[0]; iWrite target 5; iApply')
        start = time.monotonic()
        output = self.generate(body)
        self.assertIn('iWrite gate 0b' + '0' * 254 + '11', output)
        self.assertIn('iWrite target 0b0101', output)
        print(f'masked reservation: width=256 registers=34 bytes={len(source)+len(body)} '
              f'seconds={time.monotonic()-start:.3f}')

    def test_explicit_reset_releases_written_and_unwritten_ownership(self):
        for target in ('gate', 'gate_alias', 'gate[0]'):
            for persistent in ('', ' -persistent'):
                for assigned in ('', 'iWrite gate 0; iApply;'):
                    with self.subTest(target=target, persistent=persistent, assigned=assigned):
                        prefix = f'iTake {target}{persistent}; {assigned} iReset; '
                        output = self.generate(prefix + 'iWrite gate 0; iApply; iWrite target 5; iApply')
                        self.assertIn('iWrite target 0b0101', output)
                        self.assertEqual(output.count('iReset'), 2)
                        error = self.generate(prefix + 'iTake gate; iWrite gate 0; iApply; '
                                              'iWrite target 5; iApply', ok=False)
                        self.assertIn('reserved gate', error)
                        self.assertEqual((self.root / 'result.pdl').read_text(), output)

    def test_reset_in_nested_call_clears_inherited_ownership_and_replays(self):
        body = ('iProc clear {} {iReset}\n'
                'iProc own {} {iTake gate -persistent; iWrite gate 0; iApply; '
                'iCall clear; iWrite gate 0; iApply}\n'
                'iTake gate_alias; iCall own; iWrite target 5; iApply\n')
        output = self.generate(body, extra=('--debug', '--local_reset', 'reset.pdl'))
        reset = (self.root / 'reset.pdl').read_text()
        self.assertIn('iProc clear {} {iReset}', reset)
        for icl, pdl in (('source.icl', 'result.pdl'), ('source.icl', 'reset.pdl'),
                         ('debug/ResourceNetwork.icl', 'source.pdl')):
            result = subprocess.run([str(BIN), '--icl', icl, '--pdl', pdl,
                                     '--output', 'replay.pdl'], cwd=self.root,
                                    text=True, capture_output=True, timeout=40)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            self.assertEqual(output, (self.root / 'replay.pdl').read_text())

    def test_tap_reset_releases_ownership_for_both_reset_sequences(self):
        (self.root / 'source.icl').write_bytes((FIXTURE / 'slices.icl').read_bytes())
        context = ('--bsdl', 'AccessProbe', FIXTURE.parent / 'access_link/simple.bsdl')
        for reset in ('iReset', 'iReset -sync'):
            for index, value in ((0, 0), (1, 1)):
                with self.subTest(reset=reset, index=index):
                    body = (f'iTake core.gate[{index}]; iWrite core.gate[{index}] {value}; '
                            f'iApply; {reset}; iWrite core.gate 0; iApply; '
                            'iWrite core.target 5; iApply')
                    output = self.generate(body, extra=context, binding='SliceTop')
                    self.assertIn('iWrite core.target 0b0101', output)
                    result = subprocess.run([str(BIN), '--icl', 'source.icl', *map(str, context),
                                             '--pdl', 'result.pdl', '--output', 'replay.pdl'],
                                            cwd=self.root, text=True, capture_output=True, timeout=40)
                    self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
                    self.assertEqual(output, (self.root / 'replay.pdl').read_text())

    def test_reset_still_rejects_unapplied_actions(self):
        (self.root / 'result.pdl').write_text('previous output')
        error = self.generate('iTake gate; iWrite gate 0; iReset', ok=False)
        self.assertIn('unapplied', error)
        self.assertEqual((self.root / 'result.pdl').read_text(), 'previous output')

    def test_persistent_survives_calls_until_release(self):
        definitions = ('iProc own {} {iTake gate -persistent; iWrite gate 0; iApply}\n'
                       'iProc local {} {iTake gate; iWrite gate 0; iApply}\n'
                       'iProc release {} {iRelease gate}\n')
        prefix = definitions + 'iCall own; iCall local; iCall own; '
        error = self.generate(prefix + 'iWrite target 5; iApply', ok=False)
        self.assertIn('reserved gate', error)
        self.generate(prefix + 'iCall release; iWrite target 5; iApply')
        self.generate(definitions + 'iCall own; iRelease gate; iTake gate; '
                      'iWrite target 5; iApply')

    def test_default_instance_and_prefix(self):
        for take in ('iTake', 'iTake {}', 'iTake -persistent', 'iTake {} -persistent'):
            with self.subTest(take=take):
                error = self.generate(f'{take}; iWrite gate 0; iApply; iWrite target 5; iApply', ok=False)
                self.assertIn('reserved gate', error)
        (self.root / 'source.icl').write_bytes((FIXTURE / 'hierarchy.icl').read_bytes())
        prefix = ('iProcsForModule ReservedLeaf\n'
                  'iProc own {} {iTake -persistent; iWrite gate 0; iApply}\n'
                  'iProcsForModule ResourceNetwork\niPrefix core\niCall own\niPrefix -\n')
        error = self.generate(prefix + 'iWrite core.target 5; iApply', ok=False)
        self.assertIn('reserved core.gate', error)
        output = self.generate(prefix + 'iRelease core; iWrite core.target 5; iApply',
                               extra=('--debug',))
        result = subprocess.run([str(BIN), '--icl', 'debug/ResourceNetwork.icl',
                                 '--pdl', 'source.pdl', '--output', 'flat.pdl'],
                                cwd=self.root, text=True, capture_output=True, timeout=40)
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        self.assertEqual(output, (self.root / 'flat.pdl').read_text())

    def test_assumptions_establish_and_replace_protection(self):
        error = self.generate('iTake gate; iState gate 0; iWrite target 5; iApply', ok=False)
        self.assertIn('reserved gate', error)
        output = self.generate('iTake gate; iState gate 0; iState gate 1; iWrite target 5; iApply')
        self.assertEqual(output.count('iApply'), 1)
        self.assertIn('iState gate 0b01', output)
        self.generate('iState gate 0; iTake gate; iWrite target 5; iApply')
        output = self.generate('iTake gate; iWrite gate 2; iApply; iState gate[0] 1; '
                               'iWrite target 5; iApply')
        self.assertIn('iWrite gate 0b11', output)
        for body, message in (
                ('iTake gate; iState gate 0bXX', 'unknown'),
                ('iTake gate; iWrite gate 0; iState gate 1; iApply', 'pending')):
            with self.subTest(body=body):
                self.assertIn(message, self.generate(body, ok=False))

    def test_persistent_assumption_across_files_and_fixed_points(self):
        prefix = ('iProc own {} {iTake gate -persistent; iState gate 0}\n'
                  'iMerge -begin; iCall own; iMerge -end\n')
        error = self.generate(prefix + 'iWrite target 5; iApply', ok=False)
        self.assertIn('reserved gate', error)
        (self.root / 'later.pdl').write_text('iRelease gate; iWrite target 5; iApply')
        output = self.generate(prefix, extra=('--pdl', 'later.pdl', '--debug', '--local_reset', 'reset.pdl'))
        for icl, pdl, later in [('source.icl', 'result.pdl', []),
                               ('source.icl', 'reset.pdl', []),
                               ('debug/ResourceNetwork.icl', 'source.pdl', ['later.pdl'])]:
            result = subprocess.run([str(BIN), '--icl', icl, '--pdl', pdl, *later,
                                     '--output', 'replay.pdl'], cwd=self.root,
                                    text=True, capture_output=True, timeout=40)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            self.assertEqual(output, (self.root / 'replay.pdl').read_text())

    def test_reset_rewrite_cannot_override_reserved_control(self):
        fixture = ROOT / 'tests/cases/ours/pattern_generation/merge/local_reset.icl'
        (self.root / 'source.icl').write_text(fixture.read_text().replace('MergeNetwork', 'ResourceNetwork'))
        body = ('iProc work {} {iWrite reset 0; iApply; iWrite a.value 5; iApply}\n'
                'iTake reset\niWrite reset 1\niApply\niCall work\n')
        self.generate(body)
        previous = (self.root / 'result.pdl').read_text()
        error = self.generate(body, ok=False, extra=('--local_reset', 'reset.pdl'))
        self.assertIn('reserved reset', error)
        self.assertEqual((self.root / 'result.pdl').read_text(), previous)
        self.assertFalse((self.root / 'reset.pdl').exists())

    def test_reset_planner_chooses_available_alternative(self):
        fixtures = ROOT / 'tests/cases/ours/pattern_generation/local_reset'
        (self.root / 'source.icl').write_bytes((fixtures / 'alternatives.icl').read_bytes())
        context = ('--icl', fixtures / 'nested.icl', '--top_module', 'ResetChoice')
        for take in ('iTake b; iWrite b 0; iApply;', 'iTake b;',
                     'iTake b; iState b 0;', 'iTake b -persistent;'):
            with self.subTest(take=take):
                body = take + '\niWrite child.value 5; iApply\n'
                original = self.generate(body, extra=context, binding='ResetChoice')
                self.generate(body, extra=(*context, '--local_reset', 'reset.pdl'),
                              binding='ResetChoice')
                self.assertEqual(original, (self.root / 'result.pdl').read_text())
                reset = (self.root / 'reset.pdl').read_text()
                self.assertIn('iWrite a 0b1\niApply\n', reset)
                self.assertNotIn('iWrite b 0b1', reset)
                replay = subprocess.run([str(BIN), '--icl', 'source.icl', str(fixtures / 'nested.icl'),
                                         '--top_module', 'ResetChoice', '--pdl', 'reset.pdl',
                                         '--local_reset', 'reset-again.pdl', '--output', 'replay.pdl'],
                                        cwd=self.root, text=True, capture_output=True, timeout=40)
                self.assertEqual(replay.returncode, 0, replay.stdout + replay.stderr)
                self.assertEqual(reset, (self.root / 'reset-again.pdl').read_text())
                self.assertIn('iWrite child.value 0b0101', (self.root / 'replay.pdl').read_text())

    def test_reset_planner_rejects_blocked_and_honors_release(self):
        fixtures = ROOT / 'tests/cases/ours/pattern_generation/local_reset'
        (self.root / 'source.icl').write_bytes((fixtures / 'alternatives.icl').read_bytes())
        context = ('--icl', fixtures / 'nested.icl', '--top_module', 'ResetChoice',
                   '--local_reset', 'reset.pdl')
        (self.root / 'result.pdl').write_text('prior flat output')
        (self.root / 'reset.pdl').write_text('prior reset output')
        prefix = 'iTake a; iTake b;'
        failure = self.generate(prefix + 'iWrite child.value 5; iApply', ok=False,
                                extra=context, binding='ResetChoice')
        self.assertIn('taken-but-unwritten resource a', failure)
        self.assertIn('taken-but-unwritten resource b', failure)
        self.assertEqual((self.root / 'result.pdl').read_text(), 'prior flat output')
        self.assertEqual((self.root / 'reset.pdl').read_text(), 'prior reset output')
        self.generate(prefix + 'iRelease b; iWrite child.value 5; iApply',
                      extra=context, binding='ResetChoice')
        rewritten = (self.root / 'reset.pdl').read_text()
        self.assertIn('iWrite b 0b1\niApply', rewritten)
        self.assertNotIn('iWrite a 0b1', rewritten)

    def test_reset_planner_many_reserved_alternatives(self):
        fixtures = ROOT / 'tests/cases/ours/pattern_generation/local_reset'
        names = [f'r{i:02}' for i in range(40)]
        source = 'Module ResetChoice { ScanInPort si;\n'
        previous = 'si'
        for name in names:
            source += f"ScanRegister {name} {{ ScanInSource {previous}; ResetValue 1'b0; }}\n"
            previous = name
        source += 'LogicSignal release_reset { ~(' + ' | '.join(names) + '); }\n'
        source += ('Instance child Of Leaf { InputPort si = ' + previous +
                   '; InputPort rst = release_reset; }\nScanOutPort so { Source child.so; } }\n')
        (self.root / 'source.icl').write_text(source)
        body = ''.join(f'iTake {n}; iWrite {n} 0; ' for n in names[1:])
        body += 'iApply; iWrite child.value 5; iApply'
        start = time.monotonic()
        self.generate(body, binding='ResetChoice',
                      extra=('--icl', fixtures / 'nested.icl', '--top_module', 'ResetChoice',
                             '--local_reset', 'reset.pdl'))
        elapsed = time.monotonic() - start
        reset = (self.root / 'reset.pdl').read_text()
        self.assertIn('iWrite r00 0b1\niApply', reset)
        for name in names[1:]:
            self.assertNotIn(f'iWrite {name} 0b1', reset)
        print(f'reset choices: controls=40 reserved=39 bytes={len(source)} seconds={elapsed:.3f}')

    def test_tap_idle_keeps_reserved_value(self):
        fixture = ROOT / 'tests/cases/ours/pattern_generation/access_link'
        (self.root / 'source.icl').write_text((fixture / 'simple.icl').read_text().replace(
            'AccessTop', 'ResourceNetwork'))
        output = self.generate('iTake core.r; iWrite core.r 5; iApply; '
                               'iRunLoop 3 -tck; iRead core.r 5; iApply',
                               extra=('--bsdl', 'AccessProbe', str(fixture / 'simple.bsdl')))
        self.assertIn('iRunLoop 3 -tck', output)
        self.assertIn('iRead core.r 0b00000101', output)

    def test_reset_rewrite_cannot_create_a_reservation(self):
        (self.root / 'source.icl').write_bytes((FIXTURE / 'synthetic_reset.icl').read_bytes())
        body = 'iTake reset; iWrite a.value 5; iApply; iWrite b 6; iApply'
        original = self.generate(body)
        (self.root / 'reset.pdl').write_text('prior reset output')
        error = self.generate(body, ok=False, extra=('--local_reset', 'reset.pdl'))
        self.assertIn('taken-but-unwritten', error)
        self.assertEqual((self.root / 'result.pdl').read_text(), original)
        self.assertEqual((self.root / 'reset.pdl').read_text(), 'prior reset output')
        # Without that take, the inserted source must still be executable.
        self.generate(body.replace('iTake reset;', ''), extra=('--local_reset', 'reset.pdl'))
        result = subprocess.run([str(BIN), '--icl', 'source.icl', '--pdl', 'reset.pdl',
                                 '--output', 'reset-replay.pdl'], cwd=self.root,
                                text=True, capture_output=True, timeout=40)
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)

    def test_many_reserved_registers(self):
        count = 128
        source = 'Module ResourceNetwork { ScanInPort si;\n'
        body = ''
        for i in range(count + 1):
            source += f"ScanRegister r{i} {{ScanInSource {'si' if not i else 'r' + str(i-1)}; ResetValue 1'b0;}}\n"
            if i < count:
                body += f'iTake r{i}; iWrite r{i} 0;\n'
        source += f'ScanOutPort so {{Source r{count};}}\n}}\n'
        body += f'iApply\niWrite r{count} 1\niApply\n'
        (self.root / 'source.icl').write_text(source)
        start = time.monotonic()
        output = self.generate(body)
        self.assertEqual(output.count('iApply\n'), 2)
        for i in range(count):
            self.assertNotIn(f'iWrite r{i} 0b1', output)
        print(f'{count} protected registers: {time.monotonic()-start:.3f}s; bytes={len(source)+len(body)}')

    def test_merge_and_fixed_points(self):
        text = ('iProc first {} {iTake gate; iWrite gate 0; iApply}\n'
                'iProc second {} {iWrite target 5; iApply}\n'
                'iMerge -begin\niCall first\niCall second\niMerge -end\n')
        output = self.generate(text, extra=('--debug',))
        self.assertNotIn('iTake', output)
        self.assertNotIn('iRelease', output)
        for icl, pdl, dest in [('source.icl', 'result.pdl', 'replay.pdl'),
                               ('debug/ResourceNetwork.icl', 'source.pdl', 'flat.pdl')]:
            result = subprocess.run([str(BIN), '--icl', icl, '--pdl', pdl, '--output', dest],
                                    cwd=self.root, text=True, capture_output=True, timeout=40)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            self.assertEqual(output, (self.root / dest).read_text())

    def test_unsupported_resource_forms(self):
        for target in ('missing', 'si', 'gate[2]', 'gate -invalid'):
            with self.subTest(target=target):
                self.generate(f'iTake {target}\niWrite target 5\niApply\n', ok=False)

    def test_vendor_reservation_probes_and_replay(self):
        bsdl = FIXTURE.parent / 'access_link/simple.bsdl'
        for name in ('persistent', 'default', 'release'):
            with self.subTest(name=name):
                source = FIXTURE / f'vendor_{name}.pdl'
                inputs = ['--icl', str(FIXTURE / 'vendor.icl'), '--bsdl', 'AccessProbe', str(bsdl)]
                result = subprocess.run([str(BIN), *inputs, '--pdl', str(source),
                                         '--output', 'result.pdl', '--local_reset', 'reset.pdl',
                                         '--debug'], cwd=self.root, text=True,
                                        capture_output=True, timeout=40)
                self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
                text = (self.root / 'result.pdl').read_text()
                self.assertIn('iWrite core.leaf.value 0b10100101', text)
                self.assertEqual(text.count('iApply'), 3)
                self.assertIn('iWrite tap.INSTRUCTION', text.split('iApply')[0])
                if name != 'release':
                    self.assertEqual([line for line in text.splitlines()
                                      if line.startswith('iWrite core.gate ')],
                                     ['iWrite core.gate 0b11'] * 2)
                for icl, pdl in [(FIXTURE / 'vendor.icl', self.root / 'result.pdl'),
                                 (FIXTURE / 'vendor.icl', self.root / 'reset.pdl'),
                                 (self.root / 'debug/ResourceAccessTop.icl', source)]:
                    replay = subprocess.run([str(BIN), '--icl', str(icl), '--pdl', str(pdl),
                                             '--bsdl', 'AccessProbe', str(bsdl),
                                             '--output', 'replay.pdl'], cwd=self.root,
                                            text=True, capture_output=True, timeout=40)
                    self.assertEqual(replay.returncode, 0, replay.stdout + replay.stderr)
                    self.assertEqual(text, (self.root / 'replay.pdl').read_text())


if __name__ == '__main__':
    unittest.main()
