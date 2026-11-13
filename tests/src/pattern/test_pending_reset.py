#!/usr/bin/env python3
"""Reset cancels queued actions, with no physical write or reset preamble."""
import pathlib
import subprocess
import sys
import tempfile
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[3]
BIN = pathlib.Path(sys.argv.pop(1)).resolve() if len(sys.argv) > 1 else ROOT / 'build/icl-pattern-generation'
FIXTURE = ROOT / 'tests/cases/ours/pattern_generation'


class PendingReset(unittest.TestCase):
    def setUp(self):
        temporary = tempfile.TemporaryDirectory()
        self.addCleanup(temporary.cleanup)
        self.root = pathlib.Path(temporary.name)
        (self.root / 'source.icl').write_bytes((FIXTURE / 'resources/network.icl').read_bytes())

    def run_tool(self, pdl, output='result.pdl', icl='source.icl', extra=(), ok=True):
        result = subprocess.run([str(BIN), '--icl', str(self.root / icl),
                                 '--pdl', *[str(self.root / name) for name in pdl],
                                 '--output', str(self.root / output), '--debug', *map(str, extra)],
                                text=True, capture_output=True, timeout=45)
        self.assertEqual(result.returncode == 0, ok, result.stdout + result.stderr)
        return (self.root / output).read_text() if ok else result.stdout + result.stderr

    def generate(self, body, extra=(), binding='ResourceNetwork'):
        (self.root / 'source.pdl').write_text(f'iPDLLevel 1\niProcsForModule {binding}\niReset\n' + body)
        return self.run_tool(['source.pdl'], extra=extra)

    def replay(self, output, binding='ResourceNetwork', extra=()):
        self.assertEqual(output, self.run_tool(['result.pdl'], 'replay/result.pdl', extra=extra))
        self.assertEqual(output, self.run_tool(['source.pdl'], 'flat/result.pdl',
                                               f'debug/{binding}.icl', extra=extra))

    def test_reset_cancels_read_write_and_final_pending_state(self):
        for reset in ('iReset', 'iReset -sync'):
            for queued in ('iWrite gate 0;', 'iRead gate 0;', 'iWrite gate 0; iRead gate 0;'):
                for suffix in ('', 'iApply'):
                    with self.subTest(reset=reset, queued=queued, suffix=suffix):
                        output = self.generate(f'iTake gate; {queued} {reset}; {suffix}')
                        self.assertNotIn('iWrite', output)
                        self.assertNotIn('iRead', output)
                        self.assertEqual(output.count('iReset'), 2)
                        self.replay(output)

    def test_reset_in_top_procedure_cancels_caller_queue(self):
        body = ('iProc reset_top {} {iReset}\n'
                'iTake gate; iWrite gate 0; iCall reset_top; '
                'iWrite target 5; iApply')
        output = self.generate(body, extra=('--local_reset', self.root / 'reset.pdl'))
        self.assertIn('iWrite target 0b0101', output)
        self.assertIn('iProc reset_top {} {iReset}', (self.root / 'reset.pdl').read_text())
        self.assertEqual(output, self.run_tool(['reset.pdl'], 'reset-replay/result.pdl'))
        self.replay(output)

    def test_canceled_local_reset_target_needs_no_release(self):
        (self.root / 'source.icl').write_bytes((FIXTURE / 'merge/local_reset.icl').read_bytes())
        body = 'iWrite a.value 5; iReset\n'
        output = self.generate(body, extra=('--local_reset', self.root / 'reset.pdl'),
                               binding='MergeNetwork')
        self.assertNotIn('iWrite', output)
        self.assertNotIn('iWrite reset', (self.root / 'reset.pdl').read_text())
        self.assertEqual(output, self.run_tool(['reset.pdl'], 'reset-replay/result.pdl'))

    def test_applied_access_before_reset_keeps_its_prerequisite(self):
        (self.root / 'source.icl').write_bytes((FIXTURE / 'merge/local_reset.icl').read_bytes())
        body = 'iProc work {} {iWrite a.value 5; iApply; iWrite b.value 6; iReset}\niCall work\n'
        output = self.generate(body, extra=('--local_reset', self.root / 'reset.pdl'),
                               binding='MergeNetwork')
        self.assertIn('iWrite reset 0b0', (self.root / 'reset.pdl').read_text())
        self.assertIn('iWrite a.value 0b0101', output)
        self.assertNotIn('0b0110', output)
        self.assertEqual(output, self.run_tool(['reset.pdl'], 'reset-replay/result.pdl'))

    def test_queue_clears_across_input_files(self):
        (self.root / 'first.pdl').write_text('iPDLLevel 1\niProcsForModule ResourceNetwork\niWrite gate 0\n')
        (self.root / 'second.pdl').write_text('iReset\n')
        output = self.run_tool(['first.pdl', 'second.pdl'])
        self.assertNotIn('iWrite', output)
        self.assertEqual(output.count('iReset'), 1)

    def test_tap_reset_matches_observed_queue_discard(self):
        (self.root / 'source.icl').write_bytes((FIXTURE / 'access_link/simple.icl').read_bytes())
        extra = ('--bsdl', 'AccessProbe', FIXTURE / 'access_link/simple.bsdl')
        for reset in ('iReset', 'iReset -sync'):
            output = self.generate(f'iWrite core.r 90; iApply; iWrite core.r 165; '
                                   f'iRead core.r 90; {reset}; iApply', extra, 'AccessTop')
            self.assertIn('iWrite core.r 0b01011010', output)
            self.assertNotIn('0b10100101', output)
            self.assertNotIn('iRead core.r', output)
            self.replay(output, 'AccessTop', extra)

    def test_invalid_discarded_operations_do_not_replace_output(self):
        source = (self.root / 'source.icl').read_text().replace(
            'Module ResourceNetwork {',
            "Module ResourceNetwork { Enum vals { Zero = 1'b0; } Alias input_alias = si;")
        (self.root / 'source.icl').write_text(source)
        for operation in ('iWrite absent 0', 'iRead gate[9] 0', 'iWrite gate invalid',
                          'iWrite vals 0', 'iRead vals 0', 'iWrite si 0',
                          'iRead input_alias 0', 'iWrite so 0'):
            with self.subTest(operation=operation):
                (self.root / 'source.pdl').write_text(
                    'iPDLLevel 1\niProcsForModule ResourceNetwork\n' + operation + '; iReset\n')
                (self.root / 'result.pdl').write_text('previous result\n')
                self.run_tool(['source.pdl'], ok=False)
                self.assertEqual((self.root / 'result.pdl').read_text(), 'previous result\n')


if __name__ == '__main__':
    unittest.main()
