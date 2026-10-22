#!/usr/bin/env python3
"""Distinct same-line reductions must never collapse into a single hit."""
import importlib.util
from pathlib import Path
import tempfile
import unittest
ROOT = Path(__file__).resolve().parents[2]
spec = importlib.util.spec_from_file_location('grammar_reader', ROOT/'tests/grammar_coverage.py')
grammar = importlib.util.module_from_spec(spec)
spec.loader.exec_module(grammar)

class GrammarCoverage(unittest.TestCase):
    def test_same_line_productions_are_distinct(self):
        with tempfile.TemporaryDirectory() as tmp:
            path = Path(tmp)/'parser.cc'
            path.write_text('const int yyrline_[] = {0, 41, 41, 41, 47};')
            # Generator slot0 is unused; slot1 is $accept; printed IDs = slot-1.
            self.assertEqual(grammar.rule_lines(path, 'yyrline'), {1: 41, 2: 41, 3: 47})

    def test_trace_retains_production_id(self):
        self.assertEqual(grammar.REDUCE.findall('Reducing stack by rule 2 (line 41):'), [('2', '41')])

    def test_missing_table_is_error(self):
        with tempfile.TemporaryDirectory() as tmp:
            path = Path(tmp)/'parser.cc'; path.write_text('// no table')
            with self.assertRaises(ValueError): grammar.rule_lines(path, 'yyrline')

if __name__ == '__main__': unittest.main()
