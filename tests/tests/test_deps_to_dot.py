#!/usr/bin/env python3
"""Keep categorized compiler dependencies visible in the developer graph."""
import csv
import importlib.util
from pathlib import Path
import subprocess
import sys
from unittest.mock import patch
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[2]
spec = importlib.util.spec_from_file_location('deps_to_dot', ROOT / 'tests/deps_to_dot.py')
deps = importlib.util.module_from_spec(spec)
spec.loader.exec_module(deps)


class DependencyGraph(unittest.TestCase):
    def test_categories_and_generated_sources(self):
        with tempfile.TemporaryDirectory() as temporary:
            directory = Path(temporary)
            (directory / 'cpp_deps.makefile').write_text(
                'icl_lsp_main.o: src/icl/icl_lsp_main.cc \\\n'
                ' src/utility/lsp_server.h src/icl/format_job.h build/gen/icl_parser.hh\n')
            # Only the optional external renderer is stubbed; parse and emit real files.
            with patch.object(sys, 'argv', ['deps_to_dot.py', temporary]), \
                    patch.object(deps.subprocess, 'run',
                                 return_value=subprocess.CompletedProcess(['dot'], 0)):
                deps.main()
            with (directory / 'cpp_deps.csv').open() as stream:
                rows = list(csv.reader(stream))
            self.assertEqual(rows[0], ['source', 'depends_on'])
            self.assertEqual({tuple(row) for row in rows[1:]}, {
                ('src/icl/icl_lsp_main.cc', 'src/utility/lsp_server.h'),
                ('src/icl/icl_lsp_main.cc', 'src/icl/format_job.h'),
                ('src/icl/icl_lsp_main.cc', 'src/icl/icl_parser.yy'),
            })


if __name__ == '__main__':
    unittest.main()
