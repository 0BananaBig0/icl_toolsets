#!/usr/bin/env python3
"""Keep generated outputs out of Git even when accidentally force-added."""
import contextlib
import importlib.util
import io
from pathlib import Path
import sys
import unittest
from unittest.mock import patch

ROOT = Path(__file__).resolve().parents[2]
spec = importlib.util.spec_from_file_location('tree_clean', ROOT / 'tests/tree_clean_test.py')
tree_clean = importlib.util.module_from_spec(spec)
spec.loader.exec_module(tree_clean)


class TrackedArtifacts(unittest.TestCase):
    def result(self, paths):
        with patch.object(tree_clean, 'in_repo', return_value=True), \
             patch.object(tree_clean, 'tracked_files', return_value=paths), \
             patch.object(tree_clean, 'check_gitignore_comments', return_value=[]), \
             patch.object(sys, 'argv', ['tree_clean_test.py']), \
             contextlib.redirect_stdout(io.StringIO()):
            return tree_clean.main()

    def test_generated_files_are_rejected(self):
        for name in ['tests/__pycache__/test.cpython-314.pyc', 'src/legacy.pyc',
                     'tests/__pycache__/metadata', 'src/legacy.pyo',
                     'tests/generated/ours/performance/input.icl',
                     '.worktrees/run/src/main.cc', '.superpowers/sdd/state.json',
                     'build/dist/source.tar.gz', 'gen/icl_parser.cc']:
            with self.subTest(name=name):
                self.assertEqual(self.result([name]), 1)

    def test_maintained_sources_and_input_fixtures_are_allowed(self):
        self.assertEqual(self.result(['src/icl_parser.yy', 'tests/cases/standard/enum/result.icl',
                                      'tests/cases/ours/coverage/counts.txt', 'docs/icl-verification.md']), 0)


if __name__ == '__main__':
    unittest.main()
