#!/usr/bin/env python3
"""Check the independent identifier matrix's oracle and selection failures."""
from pathlib import Path
import subprocess
import copy
import sys
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parents[2] / 'src'))
from test_icl_keyword_contexts import combinations, formatting_matches, SEEDS


class IdentifierMatrix(unittest.TestCase):
    def test_parameter_declaration_and_reference_are_renamed_together(self):
        cases = list(combinations([SEEDS / 'parameter.icl'], ['Source']))
        _, _, name, expected, text = cases[0]
        self.assertEqual(name, 'Source')
        self.assertTrue(expected)
        self.assertIn('Parameter Source = 8;', text)
        self.assertIn('data[$Source-1:0]', text)
        self.assertEqual(sum(not case[3] for case in cases), 4)

    def test_adjacent_callback_argument_is_not_an_invalid_identifier(self):
        for role, accepted in [('callback_procedure', True), ('module', False)]:
            cases = list(combinations([SEEDS / f'{role}.icl'], ['Module']))
            case, = [case for case in cases if case[2] == 'A$suffix']
            self.assertEqual(case[3], accepted)

    def test_all_maintained_templates_participate(self):
        seeds = sorted(SEEDS.glob('*.icl'))
        self.assertTrue(seeds)
        cases = list(combinations(seeds, ['Enum']))
        self.assertEqual({c[0] for c in cases}, {s.stem for s in seeds})
        self.assertFalse(any('ProbeName' in c[4] for c in cases))

    def test_lsp_formatting_requires_exact_text_and_replacement_range(self):
        text = 'Module M {}\n'
        expected = 'Module M {\n}\n'
        edits = [{'range': {'start': {'line': 0, 'character': 0},
                            'end': {'line': 1, 'character': 0}},
                  'newText': expected}]
        self.assertTrue(formatting_matches(text, expected, edits))
        for broken in ['ModuleM{}', text]:
            bad = copy.deepcopy(edits)
            bad[0]['newText'] = broken
            self.assertFalse(formatting_matches(text, expected, bad))
        bad = copy.deepcopy(edits)
        bad[0]['range']['end'] = {'line': 0, 'character': 0}
        self.assertFalse(formatting_matches(text, expected, bad))
        self.assertFalse(formatting_matches(text, expected, []))

    def test_empty_selection_is_not_a_pass(self):
        script = Path(__file__).resolve().parents[2] / 'src/test_icl_keyword_contexts.py'
        run = subprocess.run([sys.executable, str(script), '--binary', '/nonexistent',
                              '--lsp', '/nonexistent', '--inventory', '/nonexistent',
                              '--context', 'not-a-context'], capture_output=True, text=True)
        self.assertEqual(run.returncode, 2, run.stdout + run.stderr)
        self.assertIn('no matching contexts', run.stderr)


if __name__ == '__main__':
    unittest.main()
