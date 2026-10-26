#!/usr/bin/env python3
"""Generator CLI and original/flat/generated replay contracts."""
import pathlib
import subprocess
import sys
import tempfile
import unittest

BIN = pathlib.Path(sys.argv.pop(1)).resolve() if len(sys.argv) > 1 else pathlib.Path("build/icl-pattern-generation").resolve()
ICL = """Module Network {
  ScanInPort si;
  ScanRegister gate { ScanInSource si; }
  ScanRegister target[3:0] { ScanInSource gate; }
  ScanMux route SelectedBy gate { 1'b0: gate; 1'b1: target[0]; }
  ScanOutPort so { Source route; }
}
"""
PDL = """iPDLLevel 1
iProcsForModule Network
iState gate 0
iProc write_value {value} { iWrite target $value; iApply }
iCall write_value 10
"""


class Generator(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = pathlib.Path(self.temp.name)
        (self.root / "source.icl").write_text(ICL)
        (self.root / "source.pdl").write_text(PDL)

    def run_tool(self, *arguments, ok=True):
        result = subprocess.run([str(BIN), *map(str, arguments)], cwd=self.root,
                                text=True, capture_output=True, timeout=30)
        self.assertEqual(result.returncode == 0, ok, result.stdout + result.stderr)
        return result

    def test_help_version(self):
        self.assertIn("--icl", self.run_tool("--help").stdout)
        self.assertIn("icl-pattern-generation", self.run_tool("--version").stdout)

    def test_round_trips(self):
        result = self.run_tool("--icl", "source.icl", "--pdl", "source.pdl", "--debug", "--log", "run.log")
        self.assertIn("warning", result.stdout)
        self.assertIn("out/Network.pdl", result.stdout)
        self.assertEqual((self.root / "run.log").read_text(), result.stdout)
        original = (self.root / "out/Network.pdl").read_text()
        self.assertNotIn("iProc ", original)
        self.assertNotIn("iCall ", original)
        self.assertIn("iWrite gate 0b1", original)
        flat = self.root / "out/debug/Network.icl"
        first_flat = flat.read_text()
        self.run_tool("--icl", flat, "--pdl", "source.pdl", "--output", "again/result.pdl", "--debug")
        self.assertEqual(original, (self.root / "again/result.pdl").read_text())
        self.assertEqual(first_flat, (self.root / "again/debug/Network.icl").read_text())
        self.run_tool("--icl", "source.icl", "--pdl", "out/Network.pdl", "--output", "replay.pdl")
        self.assertEqual(original, (self.root / "replay.pdl").read_text())

    def test_input_aliases_and_failure_publication(self):
        source = (self.root / "source.pdl").read_bytes()
        self.run_tool("--icl", "source.icl", "--pdl", "source.pdl", "--output", "source.pdl", ok=False)
        self.assertEqual(source, (self.root / "source.pdl").read_bytes())
        (self.root / "link.pdl").hardlink_to(self.root / "source.pdl")
        self.run_tool("--icl", "source.icl", "--pdl", "source.pdl", "--log", "link.pdl", ok=False)
        self.assertEqual(source, (self.root / "source.pdl").read_bytes())
        (self.root / "bad.pdl").write_text("iWrite missing 1; iApply\n")
        self.run_tool("--icl", "source.icl", "--pdl", "bad.pdl", "--output", "failed.pdl", ok=False)
        self.assertFalse((self.root / "failed.pdl").exists())

    def test_reset_and_ordered_redefinition(self):
        source = ICL.replace("ScanInPort si;", "ScanInPort si; ResetPort rst;", 1).replace(
            "ScanRegister gate { ScanInSource si; }",
            "ScanRegister gate { ScanInSource si; ResetValue 1'b0; }")
        (self.root / "source.icl").write_text(source)
        (self.root / "source.pdl").write_text(PDL.replace("iState gate 0", "iReset"))
        (self.root / "later.pdl").write_text(
            "iProcsForModule Network\niProc write_value {value} {iWrite target 3; iApply}\niCall write_value 10\n")
        result = self.run_tool("--icl", "source.icl", "--pdl", "source.pdl", "later.pdl")
        self.assertIn("overrides", result.stdout)
        original = (self.root / "out/Network.pdl").read_text()
        self.assertLess(original.index("0b1010"), original.index("0b0011"))
        self.assertIn("iReset\n", original)
        self.run_tool("--icl", "source.icl", "--pdl", "out/Network.pdl", "--output", "replay.pdl")
        self.assertEqual(original, (self.root / "replay.pdl").read_text())

    def test_read_obligations_and_staging(self):
        (self.root / "source.pdl").write_text(
            "iState gate 1\niRead target 0b1010\niWrite target 3\niApply\niRead target\niApply\n")
        self.run_tool("--icl", "source.icl", "--pdl", "source.pdl")
        original = (self.root / "out/Network.pdl").read_text()
        self.assertIn("iRead target 0b1010", original)
        self.assertIn("iRead target 0bxxxx", original)
        self.run_tool("--icl", "source.icl", "--pdl", "out/Network.pdl", "--output", "replay.pdl")
        self.assertEqual(original, (self.root / "replay.pdl").read_text())
        # A staging failure must leave an existing debug artifact untouched.
        (self.root / "bad-output.pdl").mkdir()
        (self.root / "debug").mkdir()
        debug = self.root / "debug/Network.icl"
        debug.write_text("previous artifact\n")
        result = self.run_tool("--icl", "source.icl", "--pdl", "source.pdl",
                               "--output", "bad-output.pdl", "--debug", ok=False)
        self.assertNotIn("info: generated", result.stdout)
        self.assertEqual(debug.read_text(), "previous artifact\n")
        self.assertFalse(list((self.root / "debug").glob(".icl-toolsets-*")))


if __name__ == "__main__":
    unittest.main()
