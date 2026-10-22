#!/usr/bin/env python3
"""The block oracle must preserve whitespace inside comments and strings."""
import importlib.util
from pathlib import Path
import subprocess
import sys
import unittest
from unittest.mock import patch

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'tests'))
spec = importlib.util.spec_from_file_location('block_review', ROOT / 'tests/block_review.py')
review = importlib.util.module_from_spec(spec)
spec.loader.exec_module(review)


class BlockOracle(unittest.TestCase):
    def test_string_or_comment_content_changes_are_detected(self):
        sources = [
            'Module M { Attribute label = "two words"; }',
            'Module M { /* two words */ Attribute flag; }',
        ]
        for source in sources:
            with self.subTest(source=source):
                corrupt = source.replace('two words', 'twowords').encode()
                result = subprocess.CompletedProcess([], 0, stdout=corrupt, stderr=b'')
                with patch.object(review.subprocess, 'run', return_value=result):
                    self.assertIsNotNone(review.run('formatter', source))

    def test_layout_only_change_is_permitted(self):
        source = 'Module M { Attribute flag; }'
        formatted = b'Module M {\n  Attribute flag;\n}\n'
        result = subprocess.CompletedProcess([], 0, stdout=formatted, stderr=b'')
        with patch.object(review.subprocess, 'run', return_value=result):
            self.assertIsNone(review.run('formatter', source))

    def test_common_comment_indent_can_change_but_relative_indent_cannot(self):
        source = 'Module M { /* header\n     two words\n       nested\n     */ }'
        for body, allowed in [('  two words\n    nested\n  */', True),
                              ('  two words\n  nested\n  */', False)]:
            with self.subTest(allowed=allowed):
                output = ('Module M {\n  /* header\n' + body + '\n}').encode()
                result = subprocess.CompletedProcess([], 0, stdout=output, stderr=b'')
                with patch.object(review.subprocess, 'run', return_value=result):
                    self.assertEqual(review.run('formatter', source) is None, allowed)


if __name__ == '__main__':
    unittest.main()
