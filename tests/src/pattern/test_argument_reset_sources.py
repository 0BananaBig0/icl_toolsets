#!/usr/bin/env python3
"""Reset prerequisites after side-effecting main-command arguments."""
import time
import unittest

import test_compound_reset_sources as helpers


class ArgumentResetSources(unittest.TestCase):
    setUp = helpers.CompoundResetSources.setUp
    run_tool = helpers.CompoundResetSources.run_tool

    def check(self, source):
        (self.root / 'source.pdl').write_text(source)
        self.run_tool('--icl', helpers.FIXTURE, '--pdl', 'source.pdl',
                      '--output', 'out.pdl', '--local_reset', 'reset.pdl', '--debug')
        rewritten = (self.root / 'reset.pdl').read_text()
        self.assertIn('apply ', rewritten)
        self.run_tool('--icl', helpers.FIXTURE, '--pdl', 'reset.pdl',
                      '--output', 'reset-pattern.pdl', '--local_reset', 'again.pdl')
        self.assertEqual(rewritten, (self.root / 'again.pdl').read_text())
        for pattern in ('out.pdl', 'reset-pattern.pdl'):
            self.run_tool('--icl', helpers.FIXTURE, '--pdl', pattern,
                          '--output', 'fixed.pdl')
            self.assertEqual((self.root / pattern).read_bytes(),
                             (self.root / 'fixed.pdl').read_bytes())
        self.run_tool('--icl', 'debug/Network.icl', '--pdl', 'source.pdl',
                      '--output', 'flat.pdl', '--local_reset', 'flat-reset.pdl')
        self.assertEqual((self.root / 'out.pdl').read_bytes(),
                         (self.root / 'flat.pdl').read_bytes())
        self.assertEqual(rewritten, (self.root / 'flat-reset.pdl').read_text())
        return rewritten, (self.root / 'reset-pattern.pdl').read_text()

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
