#!/usr/bin/env python3
"""Explicit TAP instruction transactions survive flat PDL generation."""
import pathlib
import subprocess
import sys
import tempfile
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[3]
FIXTURE = ROOT / 'tests/cases/ours/pattern_generation/access_link'
BIN = pathlib.Path(sys.argv.pop(1)).resolve() if len(sys.argv) > 1 else ROOT / 'build/icl-pattern-generation'


class TapPdlReplay(unittest.TestCase):
    def setUp(self):
        temp = tempfile.TemporaryDirectory()
        self.addCleanup(temp.cleanup)
        self.root = pathlib.Path(temp.name)
        self.inputs = ['--icl', str(FIXTURE / 'simple.icl'), '--bsdl',
                       'AccessProbe', str(FIXTURE / 'simple.bsdl')]

    def run_tool(self, source, output='result.pdl', ok=True, inputs=None, extra=()):
        result = subprocess.run([str(BIN), *(self.inputs if inputs is None else inputs),
                                 '--pdl', str(source), '--output', output, *extra],
                                cwd=self.root, text=True, capture_output=True, timeout=40)
        self.assertEqual(result.returncode == 0, ok, result.stdout + result.stderr)
        return (self.root / output).read_text() if ok else result.stdout

    def generate(self, body, **kwargs):
        source = self.root / 'source.pdl'
        source.write_text('iProcsForModule AccessTop\niReset\n' + body)
        return self.run_tool(source, **kwargs)

    def test_implicit_ir_is_explicit_and_idempotent(self):
        text = self.generate('iWrite core.r 0x59\niApply\n', extra=('--debug',))
        self.assertEqual(text.count('iApply\n'), 2)
        self.assertIn('iWrite tap.INSTRUCTION 0b0011\n', text)
        self.assertIn('iRead tap.INSTRUCTION 0b0001\niApply\n', text)
        self.assertLess(text.index('iWrite tap.INSTRUCTION'), text.index('iWrite core.r'))
        self.assertEqual(text, self.run_tool(self.root / 'result.pdl', 'replay.pdl'))
        self.assertEqual(text, self.run_tool(self.root / 'source.pdl', 'flat.pdl',
                                            inputs=['--icl', 'debug/AccessTop.icl']))
        self.assertEqual(text, self.run_tool(self.root / 'result.pdl', 'flat-replay.pdl',
                                            inputs=['--icl', 'debug/AccessTop.icl']))

    def test_explicit_ir_preserves_comparison_and_group(self):
        for value in ('0b0001', '0bxx01', '0b1011'):
            with self.subTest(value=value):
                text = self.generate('iWrite tap.INSTRUCTION[3:0] 0b0011\n'
                                     f'iRead tap.INSTRUCTION[3:0] {value}\niApply\n'
                                     'iWrite core.r 5\niApply\n')
                self.assertEqual(text.count('iApply\n'), 2)
                self.assertIn(f'iRead tap.INSTRUCTION {value}\n', text)
                self.assertEqual(text, self.run_tool(self.root / 'result.pdl', 'replay.pdl'))

    def test_ir_read_only_retains_instruction_and_does_not_add_compare(self):
        text = self.generate('iWrite tap.INSTRUCTION 3\niApply\n'
                             'iRead tap.INSTRUCTION\niApply\n'
                             'iWrite core.r 5\niApply\n')
        self.assertEqual(text.count('iApply\n'), 3)
        self.assertEqual(text.count('iWrite tap.INSTRUCTION 0b0011\n'), 2)
        self.assertEqual(text.count('iRead tap.INSTRUCTION'), 1)
        self.assertIn('iRead tap.INSTRUCTION 0bxxxx\n', text)
        self.assertEqual(text, self.run_tool(self.root / 'result.pdl', 'replay.pdl'))

    def test_instruction_only_together_and_mixed_rejection(self):
        text = self.generate('iWrite tap.INSTRUCTION 3\niApply -together\n')
        self.assertIn('iApply -together\n', text)
        self.assertEqual(text, self.run_tool(self.root / 'result.pdl', 'replay.pdl'))
        previous = (self.root / 'result.pdl').read_bytes()
        error = self.generate('iWrite tap.INSTRUCTION 3\niWrite core.r 5\n'
                              'iApply -together\n', ok=False)
        self.assertIn('iApply -together', error)
        self.assertEqual(previous, (self.root / 'result.pdl').read_bytes())

    def test_ir_read_after_reset_retains_reset_instruction(self):
        text = self.generate('iRead tap.INSTRUCTION 1\niApply\n')
        self.assertIn('iWrite tap.INSTRUCTION 0b0010\n', text)
        self.assertEqual(text, self.run_tool(self.root / 'result.pdl', 'replay.pdl'))

    def test_ambiguous_reset_instruction_requires_explicit_write(self):
        model = self.root / 'multiple-reset.bsdl'
        model.write_text((FIXTURE / 'simple.bsdl').read_text().replace(
            'IDCODE (0010)', 'IDCODE (0010, 0101)'))
        self.inputs[-1] = str(model)
        for body in ('iRead tap.INSTRUCTION; iApply',
                     'iWrite tap.INSTRUCTION[0] 1; iRead tap.INSTRUCTION; iApply'):
            with self.subTest(body=body):
                error = self.generate(body, ok=False)
                self.assertIn('ambiguous instruction', error)
        text = self.generate('iWrite tap.INSTRUCTION 3; iRead tap.INSTRUCTION; iApply')
        self.assertIn('iWrite tap.INSTRUCTION 0b0011', text)
        self.assertEqual(text, self.run_tool(self.root / 'result.pdl', 'replay.pdl'))
        text = self.generate('iWrite core.r 5; iApply')
        self.assertIn('iWrite tap.INSTRUCTION 0b0011', text)

    def test_controller_assumptions_and_synthetic_data_still_rejected(self):
        error = self.generate('iState tap.INSTRUCTION 3\n', ok=False)
        self.assertIn('controller-owned', error)
        error = self.generate('iWrite tap.DEVICE_ID 0\niApply\n', ok=False)
        self.assertIn('synthetic TAP data registers', error)

    def test_sib_closes_with_update_and_reopens_for_next_update(self):
        fixture = FIXTURE.parent / 'resources'
        self.inputs[1] = str(fixture / 'vendor.icl')
        text = self.run_tool(fixture / 'vendor_close_reopen.pdl')
        self.assertEqual(text.count('iApply\n'), 5)
        groups = text.split('iApply\n')
        self.assertIn('iWrite tap.INSTRUCTION', groups[0])
        for opening, closing, value in ((1, 2, '10100101'), (3, 4, '01011010')):
            self.assertIn('iWrite core.gate 0b01\n', groups[opening])
            self.assertIn('iWrite core.gate 0b00\n', groups[closing])
            self.assertIn(f'iWrite core.leaf.value 0b{value}\n', groups[closing])
        self.assertEqual(text, self.run_tool(self.root / 'result.pdl', 'replay.pdl'))

    def test_transitive_reset_preserves_scanned_reset_held_storage(self):
        fixture = FIXTURE.parent / 'local_reset'
        self.inputs = ['--icl', str(fixture / 'nested.icl'), str(fixture / 'vendor.icl'),
                       '--bsdl', 'AccessProbe', str(FIXTURE / 'simple.bsdl'),
                       '--top_module', 'ResetAccessTop']
        text = self.run_tool(fixture / 'vendor.pdl', extra=('--local_reset', 'reset.pdl'))
        groups = text.split('iApply\n')
        self.assertIn('iWrite core.root 0b1\n', groups[1])
        self.assertIn('iRead core.branch.release 0bx\n', groups[1])
        self.assertNotIn('iWrite core.branch.release', groups[1])
        self.assertIn('iWrite core.branch.release 0b1\n', groups[2])
        self.assertEqual(text, self.run_tool(self.root / 'result.pdl', 'replay.pdl'))
        reset = (self.root / 'reset.pdl').read_text()
        self.assertNotIn('tap.INSTRUCTION', reset)
        self.assertNotIn('iRead core.branch.release', reset)


if __name__ == '__main__':
    unittest.main()
