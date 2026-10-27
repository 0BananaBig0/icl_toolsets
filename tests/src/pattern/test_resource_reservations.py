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

    def test_reset_requires_release(self):
        error = self.generate('iTake gate; iWrite gate 0; iApply; iReset', ok=False)
        self.assertIn('written reservations', error)

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
        for target in ('missing', 'si', 'gate_alias', 'gate[0]', 'gate -invalid'):
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
                self.assertEqual(text.count('iApply'), 2)
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
