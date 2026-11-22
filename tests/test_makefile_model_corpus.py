#!/usr/bin/env python3
"""The corpus target must execute tools from its selected BUILD_DIR."""
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]


class ModelCorpusBuildDirectory(unittest.TestCase):
    def test_selected_parser_is_not_replaced_by_default_build(self):
        with tempfile.TemporaryDirectory() as directory:
            build = Path(directory)
            parser = build / "icl_corpus_inventory"
            backend = build / "pattern_netlist_inventory"
            parser.write_text("#!/bin/sh\necho selected-parser-sentinel >&2\nexit 23\n")
            parser.chmod(0o755)
            shutil.copy2(ROOT / "build/pattern_netlist_inventory", backend)
            result = subprocess.run(
                ["make", "--no-print-directory", "model-corpus-check",
                 "BUILD_DIR=" + str(build), "--old-file=" + str(parser),
                 "--old-file=" + str(backend)],
                cwd=ROOT, capture_output=True, text=True, timeout=30)
            self.assertNotEqual(result.returncode, 0,
                                "selected failing parser was silently ignored")
            self.assertIn("selected-parser-sentinel", result.stderr)


if __name__ == "__main__":
    unittest.main()
