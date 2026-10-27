#!/usr/bin/env python3
"""Build the actual source archive with Bison/Flex and static Tcl unavailable."""
import os
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[2]


class SourceDistribution(unittest.TestCase):
    def run_command(self, command, directory):
        run = subprocess.run(command, cwd=directory, text=True,
                             capture_output=True, timeout=180)
        self.assertEqual(run.returncode, 0, run.stdout[-6000:] + run.stderr[-6000:])
        return run

    def test_archive_build_and_installed_pdl_tools(self):
        bison = os.environ.get('BISON', '/usr/bin/bison')
        z3_prefix = str(Path(os.environ.get('Z3_PREFIX', ROOT / 'build/deps/z3-4.13.3')).resolve())
        self.run_command(['make', 'dist', f'BISON={bison}'], ROOT)
        version = self.run_command(['make', '-s', 'version'], ROOT).stdout.strip()
        archive = ROOT / 'build/dist' / f'icl-format-{version}.tar.gz'
        with tempfile.TemporaryDirectory() as directory:
            temporary = Path(directory)
            self.run_command(['tar', 'xzf', str(archive), '-C', directory], ROOT)
            extracted = temporary / f'icl-format-{version}'
            self.run_command(['make', 'dynamic', '-j4', 'BISON=false', 'FLEX=false',
                              'TCL_STATIC_LIB=/does/not/exist', f'Z3_PREFIX={z3_prefix}'], extracted)
            prefix = temporary / 'installed'
            self.run_command(['make', 'install-dynamic', f'prefix={prefix}',
                              'BISON=false', 'FLEX=false', f'Z3_PREFIX={z3_prefix}'], extracted)
            for name in ('icl-format', 'icl-lsp', 'pdl-format', 'pdl-lsp', 'icl-pattern-generation'):
                run = self.run_command([str(prefix / 'bin' / name), '--version'], temporary)
                self.assertTrue(run.stdout.startswith(name + ' '))
            self.run_command(
                [sys.executable, str(extracted / 'tests/src/test_pdl_tools.py'),
                 '--formatter', str(prefix / 'bin/pdl-format'),
                 '--server', str(prefix / 'bin/pdl-lsp')], temporary)
            sentinel = prefix / 'bin/unrelated'
            sentinel.write_text('preserve\n')
            self.run_command(['make', 'uninstall', f'prefix={prefix}',
                              'BISON=false', 'FLEX=false', f'Z3_PREFIX={z3_prefix}'], extracted)
            self.assertEqual({path.name for path in (prefix / 'bin').iterdir()},
                             {'unrelated'})
            self.assertEqual(sentinel.read_text(), 'preserve\n')
            self.assertFalse((prefix / 'include/icl-format').exists())


if __name__ == '__main__':
    unittest.main()
