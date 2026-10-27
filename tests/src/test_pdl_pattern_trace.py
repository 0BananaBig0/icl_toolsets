#!/usr/bin/env python3
"""Verify static pattern comparison reuses the PDL tree without executing Tcl."""
from pathlib import Path
import json
import os
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[2]
BINARY = Path(os.environ.get('PDL_TRACE', ROOT / 'build/pdl_pattern_trace')).resolve()


class Trace(unittest.TestCase):
    def grouped(self, source, design=None):
        with tempfile.TemporaryDirectory() as temp:
            root = Path(temp)
            (root / 'input.pdl').write_text(source)
            (root / 'input.icl').write_text(design or '''Module M {
 ScanInPort si;
 ScanRegister r[3:0] { ScanInSource si; }
 ScanRegister s[3:0] { ScanInSource r[0]; }
 Alias low[1:0] = r[1:0];
 Alias inverse[3:0] = ~r;
 ScanOutPort so { Source s[0]; }
}''')
            return subprocess.run([str(BINARY), str(root / 'input.pdl'), '',
                                   '--apply-groups', 'M', '--icl', str(root / 'input.icl')],
                                  capture_output=True, text=True, timeout=10)

    def test_apply_group_write_order_and_bit_spelling(self):
        a = self.grouped('iWrite r 5; iRead s 3; iWrite s 9; iApply')
        b = self.grouped('iWrite s 0x9; iWrite r[0] 1; iWrite r[3:1] 0b010; '
                         'iRead s 0b0011; iApply')
        self.assertEqual(a.returncode, 0, a.stderr)
        self.assertEqual(b.returncode, 0, b.stderr)
        self.assertEqual(json.loads(a.stdout), json.loads(b.stdout))

    def test_apply_groups_keep_boundaries_values_and_last_assignments(self):
        base = self.grouped('iWrite r 0; iWrite low 1; iApply; iWrite s 2; iApply')
        same = self.grouped('iWrite inverse 14; iApply; iWrite s 2; iApply')
        self.assertEqual(base.returncode, 0, base.stderr)
        self.assertEqual(same.returncode, 0, same.stderr)
        self.assertEqual(json.loads(base.stdout), json.loads(same.stdout))
        for source in ('iWrite low 1; iWrite r 0; iApply; iWrite s 2; iApply',
                       'iWrite s 2; iApply; iWrite r 1; iApply',
                       'iWrite r 1; iWrite s 2; iApply',
                       'iWrite r 1; iApply -together; iWrite s 2; iApply',
                       'iWrite r 1; iApply; iRunLoop 1 -tck; iWrite s 2; iApply'):
            with self.subTest(source=source):
                other = self.grouped(source)
                self.assertEqual(other.returncode, 0, other.stderr)
                self.assertNotEqual(json.loads(base.stdout), json.loads(other.stdout))

    def test_grouped_state_is_preserved_and_dynamic_or_unapplied_input_rejected(self):
        a = self.grouped('iState r 3; iWrite s 2; iApply')
        b = self.grouped('iState r[3:2] 0; iState r[1:0] 3; iWrite s 2; iApply')
        self.assertEqual(a.returncode, 0, a.stderr)
        self.assertEqual(b.returncode, 0, b.stderr)
        self.assertEqual(json.loads(a.stdout), json.loads(b.stdout))
        changed = self.grouped('iState r 2; iWrite s 2; iApply')
        self.assertEqual(changed.returncode, 0, changed.stderr)
        self.assertNotEqual(json.loads(a.stdout), json.loads(changed.stdout))
        for source in ('iWrite r $x; iApply', 'iWrite r 1',
                       'iWrite r 1; iReset; iApply', 'iWrite missing 1; iApply'):
            with self.subTest(source=source):
                run = self.grouped(source)
                self.assertNotEqual(run.returncode, 0)
                self.assertEqual(run.stdout, '')

    def test_unknown_write_retains_previous_assignment_and_reads_stay_visible(self):
        a = self.grouped('iWrite r 5; iWrite r 0bxxxx; iRead s; iApply')
        b = self.grouped('iWrite r 5; iRead s 0bxxxx; iApply')
        self.assertEqual(a.returncode, 0, a.stderr)
        self.assertEqual(b.returncode, 0, b.stderr)
        self.assertEqual(json.loads(a.stdout), json.loads(b.stdout))
        c = self.grouped('iWrite r 5; iApply')
        self.assertNotEqual(json.loads(a.stdout), json.loads(c.stdout))

    def test_state_unknown_does_not_erase_prior_controlling_history(self):
        design = ("Module M { ScanInPort si; ScanRegister r { ScanInSource si; "
                  "DefaultLoadValue 0; } ScanOutPort so { Source r; } }")
        a = self.grouped('iState r 1; iState r 0bx; iWrite r 0bx; iApply', design)
        b = self.grouped('iState r 0bx; iWrite r 0bx; iApply', design)
        self.assertEqual(a.returncode, 0, a.stderr)
        self.assertEqual(b.returncode, 0, b.stderr)
        self.assertNotEqual(json.loads(a.stdout), json.loads(b.stdout))

    def test_repeated_state_bits_inside_alias_keep_history(self):
        design = ("Module M { ScanInPort si; ScanRegister r { ScanInSource si; "
                  "DefaultLoadValue 0; } Alias repeated[1:0] = r, r; "
                  "ScanOutPort so { Source r; } }")
        alias = self.grouped('iState repeated 0b1x; iWrite r 0bx; iApply', design)
        ordered = self.grouped('iState r 1; iState r 0bx; iWrite r 0bx; iApply', design)
        self.assertEqual(alias.returncode, 0, alias.stderr)
        self.assertEqual(ordered.returncode, 0, ordered.stderr)
        self.assertEqual(json.loads(alias.stdout), json.loads(ordered.stdout))

    def test_access_together_obligation_is_not_silently_erased(self):
        design = ("Module M { DataInPort a; DataInPort b; "
                  "Alias pair[1:0] = a, b { AccessTogether; } }")
        for command in ('iWrite pair[1] 1', 'iRead pair[0] 0', 'iState pair 1'):
            run = self.grouped(command + '; iApply', design)
            self.assertNotEqual(run.returncode, 0)
            self.assertIn('AccessTogether', run.stderr)
        plain = self.grouped('iWrite a 1; iApply', design)
        self.assertEqual(plain.returncode, 0, plain.stderr)

    def test_large_vector_matches_partitioned_writes(self):
        design = 'Module M { DataInPort r[65535:0]; }'
        whole = self.grouped('iWrite r 1; iApply', design)
        parts = self.grouped('iWrite r[65535:1] 0; iWrite r[0] 1; iApply', design)
        self.assertEqual(whole.returncode, 0, whole.stderr)
        self.assertEqual(parts.returncode, 0, parts.stderr)
        self.assertEqual(json.loads(whole.stdout), json.loads(parts.stdout))
        self.assertEqual(len(json.loads(whole.stdout)['operations'][0]['writes']), 65536)

    def test_aggregate_expansion_budget_rejects_before_emitting_trace(self):
        run = self.grouped('iWrite r 1; ' * 5 + 'iApply',
                           'Module M { DataInPort r[65535:0]; }')
        self.assertNotEqual(run.returncode, 0)
        self.assertIn('bit expansion limit', run.stderr)
        self.assertEqual(run.stdout, '')

    def run_trace(self, source, entry=''):
        with tempfile.TemporaryDirectory() as temp:
            path = Path(temp) / 'input.pdl'
            path.write_text(source)
            return subprocess.run([str(BINARY), str(path), entry], capture_output=True,
                                  text=True, timeout=10)

    def test_wrapper_and_main_have_identical_operations(self):
        source = 'iProcsForModule M\niWrite r[0] 0b1\niApply\n'
        wrapped = ('iProcsForModule M\niProc oracle {} {\n# vendor comment\n'
                   'iWrite r[0] 0b1; iApply\n}\n')
        a, b = self.run_trace(source), self.run_trace(wrapped, 'oracle')
        self.assertEqual(a.returncode, 0, a.stderr)
        self.assertEqual(b.returncode, 0, b.stderr)
        self.assertEqual(json.loads(a.stdout), json.loads(b.stdout))
        self.assertEqual(json.loads(a.stdout)['operations'],
                         [['iWrite', 'r[0]', '0b1'], ['iApply']])

    def test_operation_order_and_values_are_not_discarded(self):
        a = self.run_trace('iWrite r 1\niApply\niWrite r 0\niApply')
        b = self.run_trace('iWrite r 0\niApply\niWrite r 1\niApply')
        self.assertEqual(a.returncode, 0, a.stderr)
        self.assertEqual(b.returncode, 0, b.stderr)
        self.assertNotEqual(json.loads(a.stdout), json.loads(b.stdout))

    def test_state_assumptions_are_retained_in_strict_comparison(self):
        source = 'iState gate[0] 0b1 -LastWrittenValue\niWrite r 5\niApply\n'
        actual = self.run_trace(source)
        wrapped = self.run_trace('iProcsForModule M\niProc oracle {} {\n' + source + '}\n', 'oracle')
        self.assertEqual(actual.returncode, 0, actual.stderr)
        self.assertEqual(wrapped.returncode, 0, wrapped.stderr)
        expected = [['iState', 'gate[0]', '0b1', '-LastWrittenValue'],
                    ['iWrite', 'r', '5'], ['iApply']]
        self.assertEqual(json.loads(actual.stdout)['operations'], expected)
        self.assertEqual(json.loads(wrapped.stdout)['operations'], expected)
        for changed in (source.replace('0b1', '0b0'), 'iWrite r 5\niApply\n'):
            result = self.run_trace(changed)
            self.assertEqual(result.returncode, 0, result.stderr)
            self.assertNotEqual(json.loads(result.stdout)['operations'], expected)

    def test_dynamic_commands_ambiguous_entries_and_missing_entry_rejected(self):
        cases = [('iWrite r [exec echo bad]\niApply', ''),
                 ('iWrite r[0][exec echo bad] 1\niApply', ''),
                 (r'iWrite r \x31' + '\niApply', ''),
                 ('iWrite r $value\niApply', ''),
                 ('iState r $value\niApply', ''),
                 ('iCall run', ''),
                 ('iProcsForModule M\niProc oracle {} {iApply}\niApply', 'oracle'),
                 ('iProcsForModule M\niProc other {} {iApply}', 'oracle'),
                 ('iProcsForModule M\niProc oracle {value} {iApply}', 'oracle'),
                 ('iProcsForModule M\niProc oracle {} {iApply}\n'
                  'iProc oracle {} {iApply}', 'oracle')]
        for source, entry in cases:
            with self.subTest(source=source):
                run = self.run_trace(source, entry)
                self.assertNotEqual(run.returncode, 0)
                self.assertEqual(run.stdout, '')


if __name__ == '__main__':
    unittest.main()
