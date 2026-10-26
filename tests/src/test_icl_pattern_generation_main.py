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

    def test_tap_round_trips(self):
        fixture = pathlib.Path(__file__).resolve().parents[1] / "cases/ours/pattern_generation/access_link"
        for name in ("simple.icl", "simple.bsdl"):
            (self.root / name).write_bytes((fixture / name).read_bytes())
        (self.root / "source.pdl").write_text(
            "iProcsForModule AccessTop\niReset -sync\niWrite core.r 0x59\niApply -together\n"
            "iRunLoop 0x100000000 -tck\niRunLoop -tck 0b11\niRunLoop 0\n"
            "iRead core.r 0x59\niWrite core.r 0xa6\niApply\n")
        inputs = ("--icl", "simple.icl", "--bsdl", "AccessProbe", "simple.bsdl")
        self.run_tool(*inputs, "--pdl", "source.pdl", "--debug")
        original = (self.root / "out/AccessTop.pdl").read_text()
        self.assertIn("iReset -sync\n", original)
        self.assertEqual(original.count("iApply -together"), 1)
        self.assertIn("iRunLoop 4294967296 -tck\n", original)
        self.assertIn("iRunLoop 3 -tck\n", original)
        self.assertIn("iRunLoop 0 -tck\n", original)
        self.assertIn("iWrite core.r 0b01011001", original)
        self.assertNotIn("__", original)
        flat = self.root / "out/debug/AccessTop.icl"
        self.run_tool("--icl", flat, "--pdl", "source.pdl", "--output", "again/result.pdl", "--debug")
        self.assertEqual(original, (self.root / "again/result.pdl").read_text())
        self.assertEqual(flat.read_text(), (self.root / "again/debug/AccessTop.icl").read_text())
        self.run_tool(*inputs, "--pdl", "out/AccessTop.pdl", "--output", "replay.pdl")
        self.assertEqual(original, (self.root / "replay.pdl").read_text())
        source = (self.root / "simple.bsdl").read_bytes()
        self.run_tool(*inputs, "--pdl", "source.pdl", "--output", "simple.bsdl", ok=False)
        self.assertEqual(source, (self.root / "simple.bsdl").read_bytes())

    def test_tap_instruction_switching(self):
        fixture = pathlib.Path(__file__).resolve().parents[1] / "cases/ours/pattern_generation/access_link"
        source = (fixture / "simple.icl").read_text().replace(
            "  AccessLink tap", "  Instance other Of AccessInstrument { InputPort mode = 1'b1; }\n  AccessLink tap"
        ).replace("IJTAG_OFF { ScanInterface { core.client; } }",
                  "IJTAG_OFF { ScanInterface { other.client; } }")
        (self.root / "source.icl").write_text(source)
        (self.root / "source.pdl").write_text(
            "iProcsForModule AccessTop\niReset\niWrite core.r 3\niApply\n"
            "iWrite other.r 9\niApply\niWrite core.r 5\niApply\n")
        inputs = ("--icl", "source.icl", "--bsdl", "AccessProbe", fixture / "simple.bsdl")
        self.run_tool(*inputs, "--pdl", "source.pdl")
        original = (self.root / "out/AccessTop.pdl").read_text()
        self.assertEqual(original.count("iApply"), 3)
        self.assertIn("iWrite other.r 0b00001001", original)
        self.run_tool(*inputs, "--pdl", "out/AccessTop.pdl", "--output", "replay.pdl")
        self.assertEqual(original, (self.root / "replay.pdl").read_text())
        (self.root / "source.pdl").write_text(
            "iProcsForModule AccessTop\niReset\niWrite core.r 3\niWrite other.r 9\niApply -together\n")
        result = self.run_tool(*inputs, "--pdl", "source.pdl", "--output", "impossible.pdl", ok=False)
        self.assertIn("iApply -together", result.stdout)
        self.assertFalse((self.root / "impossible.pdl").exists())

    def test_bsdl_arguments(self):
        for args in (("--bsdl",), ("--bsdl", "AccessProbe"),
                     ("--bsdl", "AccessProbe", "--debug"), ("--bsdl", "", "file")):
            self.assertIn("requires ENTITY FILE", self.run_tool(*args, ok=False).stdout)

    def test_apply_together_round_trips(self):
        (self.root / "source.pdl").write_text(
            "iState gate 0\niWrite gate 1\niWrite target 10\niApply -together\n")
        self.run_tool("--icl", "source.icl", "--pdl", "source.pdl", "--debug")
        original = (self.root / "out/Network.pdl").read_text()
        self.assertEqual(original.count("iApply"), 2)
        self.assertEqual(original.count("iApply -together"), 1)
        self.assertIn("iWrite target 0b1010", original)
        self.run_tool("--icl", "out/debug/Network.icl", "--pdl", "source.pdl", "--output", "flat.pdl")
        self.assertEqual(original, (self.root / "flat.pdl").read_text())
        self.run_tool("--icl", "source.icl", "--pdl", "out/Network.pdl", "--output", "again.pdl")
        self.assertEqual(original, (self.root / "again.pdl").read_text())

    def test_idle_transient_reset_fill(self):
        fixture = pathlib.Path(__file__).resolve().parents[1] / "cases/ours/pattern_generation/access_link"
        (self.root / "source.pdl").write_text(
            "iReset\niState trigger.r 2\niState gate.r 0\niState core.r 0xff\n"
            "iRunLoop 1\niWrite core.r 0bXXXXXXXX\niApply\n")
        inputs = ("--icl", fixture / "idle_reset.icl", "--bsdl", "AccessProbe", fixture / "simple.bsdl")
        self.run_tool(*inputs, "--pdl", "source.pdl", "--debug")
        output = (self.root / "out/AccessTop.pdl").read_text()
        self.assertIn("iWrite core.r 0b00000000\n", output)
        self.assertNotIn("iWrite core.r 0b11111111\n", output)
        self.run_tool(*inputs, "--pdl", "out/AccessTop.pdl", "--output", "again.pdl")
        self.assertEqual(output, (self.root / "again.pdl").read_text())
        self.run_tool("--icl", "out/debug/AccessTop.icl", "--pdl", "source.pdl", "--output", "flat.pdl")
        self.assertEqual(output, (self.root / "flat.pdl").read_text())

    def test_idle_rejections(self):
        fixture = pathlib.Path(__file__).resolve().parents[1] / "cases/ours/pattern_generation/access_link"
        inputs = ("--icl", fixture / "simple.icl", "--bsdl", "AccessProbe", fixture / "simple.bsdl")
        for source, message in (
                ("iRunLoop 1", "explicit iReset"),
                ("iReset; iWrite core.r 1; iRunLoop 1", "unapplied"),
                ("iReset; iRunLoop -time 10ns", "system-clock and period models"),
                ("iReset; iRunLoop 1 -sck clock", "system-clock and period models"),
                ("iReset; iRunLoop 0xX00000000000000000", "must be known"),
                ("iReset; iRunLoop 18446744073709551616", "truncates significant bits")):
            with self.subTest(source=source):
                (self.root / "source.pdl").write_text(source + "\n")
                result = self.run_tool(*inputs, "--pdl", "source.pdl", "--output", "failed.pdl", ok=False)
                self.assertIn(message, result.stdout)
                self.assertFalse((self.root / "failed.pdl").exists())
        (self.root / "source.pdl").write_text("iRunLoop 1\n")
        result = self.run_tool("--icl", "source.icl", "--pdl", "source.pdl", ok=False)
        self.assertIn("single TAP", result.stdout)

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
