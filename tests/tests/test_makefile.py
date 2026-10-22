#!/usr/bin/env python3
"""A parallel build must invoke each multi-output generator only once."""
from pathlib import Path
import subprocess
import unittest
ROOT = Path(__file__).resolve().parents[2]

class BuildGraph(unittest.TestCase):
    def test_source_distribution_needs_no_static_libraries(self):
        run = subprocess.run(['make', '-n', 'dist', 'BISON=/usr/bin/bison'], cwd=ROOT, capture_output=True, text=True)
        self.assertEqual(run.returncode, 0, run.stderr)
        self.assertNotIn(' -static ', run.stdout)
        self.assertIn('--exclude=__pycache__', run.stdout)

    def test_one_generator_invocation(self):
        if (ROOT / '.prebuilt').exists():
            self.skipTest('source archive ships generated parser/scanner')
        run = subprocess.run(['make', '-n', '-B', 'dynamic', 'BISON=/usr/bin/bison'], cwd=ROOT, capture_output=True, text=True)
        self.assertEqual(run.returncode, 0, run.stderr)
        bison = [line for line in run.stdout.splitlines() if line.startswith('/usr/bin/bison ') and '--output=' in line]
        flex = [line for line in run.stdout.splitlines() if line.startswith('flex ') and '--outfile=' in line]
        self.assertEqual(len(bison), 1, bison)
        self.assertEqual(len(flex), 1, flex)

if __name__ == '__main__': unittest.main()
