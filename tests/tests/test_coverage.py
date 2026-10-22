#!/usr/bin/env python3
import importlib.util
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest
ROOT = Path(__file__).resolve().parents[2]
spec = importlib.util.spec_from_file_location('coverage_reader', ROOT / 'tests/coverage.py')
coverage = importlib.util.module_from_spec(spec)
spec.loader.exec_module(coverage)

class CoverageReader(unittest.TestCase):
    def test_counts_and_duplicate_source_lines(self):
        self.assertEqual(coverage.line_coverage(ROOT/'tests/cases/ours/coverage/counts.txt'), (2, 5, [2, 4, 5]))

    def test_never_executed_and_exception_edges_are_counted(self):
        self.assertEqual(coverage.branch_coverage(ROOT/'tests/cases/ours/coverage/counts.txt'), (1, 3, [6]))

    def test_missing_source_report_is_error(self):
        with tempfile.TemporaryDirectory() as tmp:
            Path(tmp, 'icl_driver.cc.gcov').write_text('1:1:x;\n')
            p = subprocess.run([sys.executable, str(ROOT/'tests/coverage.py'), '--strict'], cwd=tmp, capture_output=True, text=True)
        self.assertNotEqual(p.returncode, 0)
        self.assertIn('missing', p.stderr.lower())

    def test_empty_report_is_not_full_coverage(self):
        with tempfile.TemporaryDirectory() as tmp:
            for name in coverage.SOURCES: Path(tmp, name+'.gcov').write_text('-:0:Source:'+name+'\n')
            p = subprocess.run([sys.executable, str(ROOT/'tests/coverage.py'), '--strict'], cwd=tmp, capture_output=True, text=True)
        self.assertNotEqual(p.returncode, 0)
        self.assertIn('executable', p.stderr.lower())

if __name__ == '__main__': unittest.main()
