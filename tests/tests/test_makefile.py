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
    def test_coverage_includes_inline_code_emitted_by_pattern_tests(self):
        run = subprocess.run(['make', '-n', 'coverage', 'BISON=/usr/bin/bison'],
                             cwd=ROOT, capture_output=True, text=True, check=True)
        gcov = next(shlex.split(line) for line in run.stdout.splitlines()
                    if line.startswith('gcov '))
        for test in ('pattern_tap_test', 'pattern_netlist_test',
                     'pattern_design_test', 'pattern_expression_test',
                     'pattern_bsdl_test', 'pattern_access_link_test',
                     'pattern_address_test', 'pattern_logic_test',
                     'pattern_runtime_test', 'pattern_flatten_test'):
            with self.subTest(test=test):
                self.assertIn(f'build/cov/{test}-{test}.gcno', gcov)

    def test_regression_entrypoints_share_the_suite(self):
        for target in ('check', 'check-dynamic'):
            with self.subTest(target=target):
                run = subprocess.run(['make', '-n', target, 'BISON=/usr/bin/bison'],
                                     cwd=ROOT, capture_output=True, text=True)
                self.assertEqual(run.returncode, 0, run.stderr)
                for required in ('tests/regression.py', 'tests/lsp_test.py',
                                 'tests/fuzz.py', 'tests/grammar_coverage.py',
                                 'tests/vim-icl-pdl-modern/test_plugin.py',
                                 'build/pattern_flatten_test'):
                    self.assertIn(required, run.stdout)
                if target == 'check-dynamic':
                    self.assertNotIn(' -static ', run.stdout)
                    self.assertNotIn('build/pattern_runtime_test-static', run.stdout)

    def test_coverage_cleanup_preserves_other_worktrees(self):
        run = subprocess.run(['make', '-n', 'clean'], cwd=ROOT,
                             capture_output=True, text=True, check=True)
        removals = [shlex.split(line)[2:] for line in run.stdout.splitlines()
                    if line.startswith('rm -rf ')]
        self.assertEqual(removals, [['build', '.worktrees/coverage']])

    def test_grammar_coverage_header_dependencies(self):
        target = 'build/gramcov/format_job.o'
        subprocess.run(['make', target, 'BISON=/usr/bin/bison'], cwd=ROOT,
                       capture_output=True, text=True, check=True)
        for header in ('src/icl_semantics.h', 'src/text_position.h'):
            with self.subTest(header=header):
                # Hold generated headers fixed: their empty rule can otherwise
                # make a dry run pretend to rebuild unrelated objects.
                run = subprocess.run(['make', '-n', '-o', 'build/gramcov/icl_parser.hh',
                                      '-W', header, target,
                                      'BISON=/usr/bin/bison'], cwd=ROOT,
                                     capture_output=True, text=True)
                self.assertEqual(run.returncode, 0, run.stderr)
                self.assertIn('-c src/format_job.cc', run.stdout)

    def test_unit_header_dependencies(self):
        subprocess.run(['make', 'build/unit_tests', 'BISON=/usr/bin/bison'],
                       cwd=ROOT, capture_output=True, text=True, check=True)
        for header in ('src/text_position.h', 'src/utf8.h'):
            with self.subTest(header=header):
                run = subprocess.run(['make', '-n', '-W', header, 'build/unit_tests',
                                      'BISON=/usr/bin/bison'],
                                     cwd=ROOT, capture_output=True, text=True)
                self.assertEqual(run.returncode, 0, run.stderr)
                self.assertIn('tests/unit_tests.cc', run.stdout)

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
