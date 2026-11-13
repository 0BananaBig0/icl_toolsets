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
            self.check_installed_tools(prefix, extracted, temporary)
            sentinel = prefix / 'bin/unrelated'
            sentinel.write_text('preserve\n')
            self.run_command(['make', 'uninstall', f'prefix={prefix}',
                              'BISON=false', 'FLEX=false', f'Z3_PREFIX={z3_prefix}'], extracted)
            self.assertEqual({path.name for path in (prefix / 'bin').iterdir()},
                             {'unrelated'})
            self.assertEqual(sentinel.read_text(), 'preserve\n')
            self.assertFalse((prefix / 'include/icl-format').exists())

    def check_installed_tools(self, prefix, extracted, temporary):
        for name in ('icl-format', 'icl-lsp', 'pdl-format', 'pdl-lsp', 'icl-pattern-generation'):
            run = self.run_command([str(prefix / 'bin' / name), '--version'], temporary)
            self.assertTrue(run.stdout.startswith(name + ' '))
        self.run_command(
            [sys.executable, str(extracted / 'tests/src/test_pdl_tools.py'),
             '--formatter', str(prefix / 'bin/pdl-format'),
             '--server', str(prefix / 'bin/pdl-lsp')], temporary)
        generator = str(prefix / 'bin/icl-pattern-generation')
        fixture = extracted / 'tests/cases/ours/pattern_generation/local_reset'
        inputs = ['--icl', str(fixture / 'nested.icl')]
        sources = ['--pdl', str(fixture / 'procedures.pdl'), str(fixture / 'main.pdl')]
        self.run_command([generator, *inputs, *sources, '--output', 'pattern.pdl',
                          '--local_reset', 'reset.pdl', '--debug'], temporary)
        pattern = (temporary / 'pattern.pdl').read_bytes()
        self.run_command([generator, '--icl', 'debug/Network.icl', *sources,
                          '--output', 'flat.pdl'], temporary)
        self.assertEqual(pattern, (temporary / 'flat.pdl').read_bytes())
        self.run_command([generator, *inputs, '--pdl', 'pattern.pdl',
                          '--output', 'replay.pdl'], temporary)
        self.assertEqual(pattern, (temporary / 'replay.pdl').read_bytes())
        rewritten = (temporary / 'reset.pdl').read_text()
        self.assertTrue(rewritten.startswith((fixture / 'procedures.pdl').read_text()))
        self.assertIn('iWrite root 0b1\niApply\niWrite branch.release 0b1\niApply\n', rewritten)
        self.run_command([generator, *inputs, '--pdl', 'reset.pdl',
                          '--output', 'reset-pattern.pdl', '--local_reset', 'reset-again.pdl'], temporary)
        reset_pattern = (temporary / 'reset-pattern.pdl').read_bytes()
        for write in (b'iWrite a.value 0b1010', b'iWrite b.value 0b0011'):
            self.assertIn(write, reset_pattern)
        self.run_command([generator, *inputs, '--pdl', 'reset-pattern.pdl',
                          '--output', 'reset-replay.pdl'], temporary)
        self.assertEqual(reset_pattern, (temporary / 'reset-replay.pdl').read_bytes())
        self.assertEqual(rewritten, (temporary / 'reset-again.pdl').read_text())


if __name__ == '__main__':
    unittest.main()
