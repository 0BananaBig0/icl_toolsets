#!/usr/bin/env python3
"""Build graph, distribution and installed-header integration checks."""
from pathlib import Path
import os
import shlex
import subprocess
import tempfile
import unittest
ROOT = Path(__file__).resolve().parents[2]

class BuildGraph(unittest.TestCase):
    def test_installed_headers_resolve_without_checkout(self):
        with tempfile.TemporaryDirectory() as directory:
            prefix = Path(directory)
            run = subprocess.run(
                ['make', 'install-dynamic', 'BISON=/usr/bin/bison',
                 f'prefix={prefix}', f'MANIFEST={prefix}/manifest'],
                cwd=ROOT, capture_output=True, text=True)
            self.assertEqual(run.returncode, 0, run.stdout + run.stderr)
            self.assertEqual({p.name for p in (prefix / 'bin').iterdir()},
                             {'icl-format', 'icl-lsp', 'pdl-format', 'pdl-lsp'})
            for name in ('icl-format', 'icl-lsp', 'pdl-format', 'pdl-lsp'):
                installed = subprocess.run([str(prefix / 'bin' / name), '--version'],
                                           capture_output=True, text=True, timeout=5)
                self.assertEqual(installed.returncode, 0, installed.stderr)
                self.assertTrue(installed.stdout.startswith(name + ' '))
            notices = prefix / 'share/doc/icl-format/third-party-notices.md'
            self.assertTrue(notices.is_file())
            self.assertIn('Regents of the University of', notices.read_text())
            self.assertIn(str(notices), (prefix / 'manifest').read_text())
            headers = sorted((prefix / 'include/icl-format').glob('*.h'))
            self.assertEqual({header.name for header in headers},
                             {header.name for header in (ROOT / 'src').glob('*.h')})
            source = ''.join(f'#include "{header.name}"\n' for header in headers)
            run = subprocess.run(
                shlex.split(os.environ.get('CXX', 'g++')) +
                ['-std=c++17', '-fsyntax-only', '-x', 'c++',
                 f'-I{prefix}/include/icl-format', '-'],
                cwd=prefix, input=source, capture_output=True, text=True)
            self.assertEqual(run.returncode, 0, run.stderr)

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
