#!/usr/bin/env python3
"""The reference adapter must reject skipped suffixes and lexical errors."""
import importlib.util
import os
from pathlib import Path
import subprocess
import sys
import unittest

TESTS = Path(__file__).resolve().parents[1]
SPEC = importlib.util.spec_from_file_location('antlr_diff', TESTS / 'antlr_diff.py')
diff = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(diff)


class Availability(unittest.TestCase):
    def test_absent_reference_is_not_success(self):
        result = subprocess.run([sys.executable, str(TESTS / 'antlr_diff.py'), '--ref-dir', ''],
                                capture_output=True, text=True)
        self.assertEqual(77, result.returncode, result.stdout + result.stderr)
        self.assertIn('skipped', result.stdout)


@unittest.skipUnless(os.environ.get('ICL_REF_DIR'), 'ICL_REF_DIR unset; real ANTLR integration unrun')
class Reference(unittest.TestCase):
    def setUp(self):
        self.parse, error = diff.load_reference(os.environ['ICL_REF_DIR'])
        self.assertIsNotNone(self.parse, error)

    def test_invalid_utf8_is_lexical_rejection(self):
        ok, why = self.parse(str(TESTS / 'cases/fail/ours/partial_bom_ef_bb.icl'))
        self.assertFalse(ok)
        self.assertTrue(why)

    def test_complete_input(self):
        ok, why = self.parse(str(TESTS / 'cases/ours/reference/valid.icl'))
        self.assertTrue(ok, why)

    def test_trailing_tokens(self):
        ok, why = self.parse(str(TESTS / 'cases/ours/reference/trailing.icl'))
        self.assertFalse(ok)
        self.assertTrue(why)

    def test_lexical_suffix(self):
        ok, why = self.parse(str(TESTS / 'cases/ours/reference/lexical_suffix.icl'))
        self.assertFalse(ok)
        self.assertTrue(why)

    def test_lexer_error_in_body(self):
        ok, why = self.parse(str(TESTS / 'cases/ours/reference/lexical_body.icl'))
        self.assertFalse(ok)
        self.assertTrue(why)


if __name__ == '__main__':
    unittest.main()
