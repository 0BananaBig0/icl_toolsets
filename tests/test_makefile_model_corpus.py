#!/usr/bin/env python3
"""The corpus target must execute tools from its selected BUILD_DIR."""
from pathlib import Path
import json
import os
import sys
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]


class ModelCorpusBuildDirectory(unittest.TestCase):
    def invoke(self, build):
        return subprocess.run(
            ["make", "--no-print-directory", "model-corpus-check",
             "BUILD_DIR=" + str(build), "MAKE=/usr/bin/true",
             "--old-file=" + str(build / "icl_corpus_inventory"),
             "--old-file=" + str(build / "pattern_netlist_inventory")],
            cwd=ROOT, capture_output=True, text=True, timeout=30)

    def prepare(self, build, parser_text):
        parser, backend = build / "icl_corpus_inventory", build / "pattern_netlist_inventory"
        parser.write_text(parser_text)
        backend.write_text("#!/bin/sh\necho unexpected-backend >&2\nexit 99\n")
        parser.chmod(0o755)
        backend.chmod(0o755)
        report = build / "model_source_audit.json"
        report.write_text('{"ok":true,"old":true}')
        return report

    def test_selected_parser_is_not_replaced_by_default_build(self):
        with tempfile.TemporaryDirectory() as directory:
            build = Path(directory)
            report = self.prepare(build, "#!/bin/sh\necho selected-parser-sentinel >&2\nexit 23\n")
            result = self.invoke(build)
            self.assertNotEqual(result.returncode, 0,
                                "selected failing parser was silently ignored")
            self.assertIn("selected-parser-sentinel", result.stderr)
            self.assertFalse(json.loads(report.read_text())["ok"])
            self.assertNotIn("old", json.loads(report.read_text()))

    def test_dedup_failure_invalidates_prior_success(self):
        with tempfile.TemporaryDirectory() as directory:
            build = Path(directory)
            script = ("#!" + sys.executable + "\nimport json,sys\n"
                      "print(json.dumps([{'ok':True,'modules':[]} for _ in sys.argv[2:]]))\n")
            report = self.prepare(build, script)
            result = self.invoke(build)
            self.assertNotEqual(result.returncode, 0)
            self.assertEqual(json.loads((build / "model_source_corpus.json").read_text())["modules"], 0)
            self.assertFalse(json.loads(report.read_text())["ok"])
            self.assertNotIn("old", json.loads(report.read_text()))
            self.assertNotIn("unexpected-backend", result.stderr)

    def test_producer_and_protocol_suites_use_selected_parser(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            parser, marker = root / "parser", root / "used"
            parser.write_text("#!" + sys.executable + "\nfrom pathlib import Path\n"
                              "Path(" + repr(str(marker)) + ").touch()\n"
                              "raise SystemExit(23)\n")
            parser.chmod(0o755)
            env = os.environ.copy()
            env["PARSER_INVENTORY"] = str(parser)
            for suite in ("tests/tests/test_audit_icl_corpus.py",
                          "tests/tests/src/pattern/test_audit_model_corpus.py"):
                with self.subTest(suite=suite):
                    marker.unlink(missing_ok=True)
                    result = subprocess.run([sys.executable, suite], cwd=ROOT,
                                            env=env, capture_output=True,
                                            text=True, timeout=30)
                    self.assertNotEqual(result.returncode, 0)
                    self.assertTrue(marker.is_file(), "selected parser was ignored")


if __name__ == "__main__":
    unittest.main()
