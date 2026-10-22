#!/usr/bin/env python3
"""Check the independent oracle's transformations, not parser acceptance."""
from pathlib import Path
import sys
import subprocess
import unittest
sys.path.insert(0, str(Path(__file__).resolve().parents[2] / 'src'))
from test_icl_metamorphic import variants


class Transformations(unittest.TestCase):
    def test_strings_and_comments_are_not_rewritten(self):
        text = 'Module M { Attribute a = " a [3:0] \\\" b"; /* [4] */ // [5]\n}\n'
        cases = list(variants(text))
        self.assertTrue(cases)
        for name, result, _ in cases:
            with self.subTest(name=name):
                self.assertIn('" a [3:0] \\\" b"', result)
                self.assertIn('/* [4] */', result)
                self.assertRegex(result, r'// \[5\]\r?\n')
        self.assertFalse(any(name.startswith('index_depth') for name, _, _ in cases))

    def test_numeric_indices_gain_balanced_recursive_parentheses(self):
        cases = {name: (text, expected) for name, text, expected in
                 variants('Module M { DataInPort d[3:0]; Alias a = d[1]; }\n')}
        text, expected = cases['index_depth_4']
        self.assertTrue(expected)
        self.assertIn('d[((((3)))):((((0))))]', text)
        self.assertIn('d[((((1))))]', text)

    def test_empty_selection_fails(self):
        script = Path(__file__).resolve().parents[2] / 'src/test_icl_metamorphic.py'
        for block in ['not-a-block', 'eda-corpus']:
            with self.subTest(block=block):
                run = subprocess.run(
                    [sys.executable, str(script), '--inventory', '/nonexistent',
                     '--block', block], capture_output=True, text=True)
                self.assertEqual(run.returncode, 2, run.stdout + run.stderr)
                self.assertIn('error:', run.stderr)

    def test_negative_nested_comment_is_marked_rejected(self):
        cases = list(variants('Module M {}\n'))
        negatives = [(name, text) for name, text, accept in cases if not accept]
        self.assertEqual(len(negatives), 1)
        self.assertEqual(negatives[0][0], 'invalid_nested_comment')
        self.assertIn('/* outer /* nested */ */', negatives[0][1])


if __name__ == '__main__':
    unittest.main()
