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

    def test_primary_input_one_hot_write_round_trips(self):
        fixture = pathlib.Path(__file__).resolve().parents[1] / "cases/ours/pattern_generation/one_hot_width.icl"
        for top, value in (("EqualWidthProbe", "0x96"),
                           ("WidthProbe", "0bXXXXXXXX10010110"),
                           ("AscendingWidthProbe", "0bXXXXXXXX10010110")):
            with self.subTest(top=top):
                (self.root / "source.pdl").write_text(
                    f"iProcsForModule {top}\niWrite observed {value}\niApply\n")
                inputs = ("--icl", fixture, "--top_module", top)
                self.run_tool(*inputs, "--pdl", "source.pdl", "--output", "result.pdl",
                              "--debug", "--local_reset", "reset.pdl")
                output = (self.root / "result.pdl").read_text()
                self.assertIn("iWrite data 0b10010110\n", output)
                self.assertIn("iWrite enable 0b1\n", output)
                self.assertEqual((self.root / "reset.pdl").read_bytes(),
                                 (self.root / "source.pdl").read_bytes())
                self.run_tool(*inputs, "--pdl", "result.pdl", "--output", "again.pdl")
                self.assertEqual(output, (self.root / "again.pdl").read_text())
                flat = self.root / f"debug/{top}.icl"
                flat_bytes = flat.read_bytes()
                self.run_tool("--icl", flat, "--pdl", "source.pdl", "--output", "flat/result.pdl", "--debug")
                self.assertEqual(output, (self.root / "flat/result.pdl").read_text())
                self.assertEqual(flat_bytes, (self.root / f"flat/debug/{top}.icl").read_bytes())
        for top in ("WidthProbe", "AscendingWidthProbe"):
            for value in ("0x96", "0xff96", "0b10010110XXXXXXXX"):
                with self.subTest(top=top, rejected=value):
                    (self.root / "source.pdl").write_text(
                        f"iProcsForModule {top}\niWrite observed {value}\niApply\n")
                    original = b"previous output\n"
                    (self.root / "result.pdl").write_bytes(original)
                    failure = self.run_tool("--icl", fixture, "--top_module", top,
                                            "--pdl", "source.pdl", "--output", "result.pdl", ok=False)
                    self.assertIn("cannot justify", failure.stdout)
                    self.assertEqual((self.root / "result.pdl").read_bytes(), original)

    def test_tap_round_trips(self):
        fixture = pathlib.Path(__file__).resolve().parents[1] / "cases/ours/pattern_generation/access_link"
        for name in ("simple.icl", "simple.bsdl"):
            (self.root / name).write_bytes((fixture / name).read_bytes())
        (self.root / "source.pdl").write_text(
            "iProcsForModule AccessTop\niReset -sync\niWrite core.r 0x59\niApply -together\n"
            "iRunLoop 0x100000000 -tck\niRunLoop -tck 0b11\niRunLoop 0\n"
            "iRead core.r 0x59\niWrite core.r 0xa6\niApply\n")
        inputs = ("--icl", "simple.icl", "--bsdl", "AccessProbe", "simple.bsdl")
        self.run_tool(*inputs, "--pdl", "source.pdl", "--debug", "--local_reset", "reset.pdl")
        self.assertEqual((self.root / "reset.pdl").read_bytes(), (self.root / "source.pdl").read_bytes())
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
        self.assertEqual(original.count("iApply"), 6)
        self.assertEqual(original.count("iWrite tap.INSTRUCTION"), 3)
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

    def test_access_together_round_trips(self):
        fixture = pathlib.Path(__file__).resolve().parents[1] / "cases/ours/pattern_generation/access_together.icl"
        (self.root / "source.icl").write_bytes(fixture.read_bytes())
        for operation in ("iWrite", "iRead"):
            with self.subTest(operation=operation):
                (self.root / "source.pdl").write_text(
                    "iState gate 0\niState a 0\niState b 0\niState c 0\n"
                    f"{operation} nested 1\n{operation} c 0\niApply\n")
                self.run_tool("--icl", "source.icl", "--pdl", "source.pdl", "--debug")
                original = (self.root / "out/Network.pdl").read_text()
                self.assertEqual(original.count("iApply"), 2)
                flat = self.root / "out/debug/Network.icl"
                self.assertIn("AccessTogether;", flat.read_text())
                first_flat = flat.read_text()
                self.run_tool("--icl", flat, "--pdl", "source.pdl", "--output", "again/result.pdl", "--debug")
                self.assertEqual(original, (self.root / "again/result.pdl").read_text())
                self.assertEqual(first_flat, (self.root / "again/debug/Network.icl").read_text())
                self.run_tool("--icl", "source.icl", "--pdl", "out/Network.pdl", "--output", "replay.pdl")
                self.assertEqual(original, (self.root / "replay.pdl").read_text())

    def test_access_together_overlapping_reads(self):
        fixture = pathlib.Path(__file__).resolve().parents[1] / "cases/ours/pattern_generation/access_together_overlap.icl"
        (self.root / "source.icl").write_bytes(fixture.read_bytes())
        (self.root / "source.pdl").write_text(
            "iState gate 0\niRead first 0\niRead second 0\niApply\n")
        self.run_tool("--icl", "source.icl", "--pdl", "source.pdl", "--debug")
        original = (self.root / "out/Network.pdl").read_text()
        scans = original.split("iApply")
        self.assertEqual(len(scans), 3)
        self.assertNotIn("iRead", scans[0])
        for target in ("a", "b", "c"):
            self.assertIn(f"iRead {target} 0b0", scans[1])
        self.run_tool("--icl", "out/debug/Network.icl", "--pdl", "source.pdl", "--output", "flat.pdl")
        self.assertEqual(original, (self.root / "flat.pdl").read_text())
        self.run_tool("--icl", "source.icl", "--pdl", "out/Network.pdl", "--output", "again.pdl")
        self.assertEqual(original, (self.root / "again.pdl").read_text())

    def test_unimplemented_end_state_rejects_without_publication(self):
        fixture = pathlib.Path(__file__).resolve().parents[1] / "cases/ours/pattern_generation/apply_end_state.icl"
        (self.root / "source.icl").write_bytes(fixture.read_bytes())
        # The persistent alias directive applies even when PDL names storage.
        for command in ("iWrite trigger 1", "iWrite idle_trigger 1", "iRead trigger 0"):
            for debug in ((), ("--debug",)):
                with self.subTest(command=command, debug=debug):
                    (self.root / "source.pdl").write_text(
                        f"iProcsForModule EndState\niReset\n{command}\niApply\n")
                    (self.root / "result.pdl").write_text("previous output\n")
                    result = self.run_tool("--icl", "source.icl", "--pdl", "source.pdl",
                                           "--output", "result.pdl", *debug, ok=False)
                    self.assertIn("iApplyEndState", result.stdout)
                    self.assertIn("idle_trigger", result.stdout)
                    self.assertIn("source.icl:", result.stdout)
                    self.assertIn("not implemented", result.stdout)
                    self.assertEqual((self.root / "result.pdl").read_text(), "previous output\n")
                    self.assertFalse((self.root / "debug/EndState.icl").exists())

    def test_local_reset_passthrough_preserves_procedures(self):
        source = (self.root / "source.pdl").read_bytes()
        self.run_tool("--icl", "source.icl", "--pdl", "source.pdl",
                      "--local_reset", "reset.pdl", "--debug")
        self.assertEqual((self.root / "reset.pdl").read_bytes(), source)
        original = (self.root / "out/Network.pdl").read_bytes()
        self.run_tool("--icl", "out/debug/Network.icl", "--pdl", "reset.pdl",
                      "--output", "again.pdl", "--local_reset", "again-reset.pdl")
        self.assertEqual((self.root / "again.pdl").read_bytes(), original)
        self.assertEqual((self.root / "again-reset.pdl").read_bytes(), source)
        self.run_tool("--icl", "source.icl", "--pdl", "source.pdl",
                      "--local_reset", "source.pdl", ok=False)
        self.assertEqual((self.root / "source.pdl").read_bytes(), source)
        (self.root / "blocked-reset").mkdir()
        self.run_tool("--icl", "source.icl", "--pdl", "source.pdl",
                      "--output", "out/Network.pdl", "--local_reset", "blocked-reset", ok=False)
        self.assertEqual((self.root / "out/Network.pdl").read_bytes(), original)
        (self.root / "second.pdl").write_text("iApply\n")
        self.run_tool("--icl", "source.icl", "--pdl", "source.pdl", "second.pdl",
                      "--local_reset", "multi.pdl")
        self.assertEqual((self.root / "multi.pdl").read_bytes(), source + b"\n\niApply\n")

    def test_local_reset_passthrough_checks_reset_connections(self):
        source = """Module Child {
 ScanInPort si; ResetPort rst;
 ScanRegister r { ScanInSource si; ResetValue 1'b0; }
 ScanOutPort so { Source r; }
}
Module Network {
 ScanInPort si; ResetPort rst;
 ScanRegister release { ScanInSource si; }
 Instance child Of Child { InputPort si = release; }
 ScanOutPort so { Source child.so; }
}
"""
        (self.root / "source.icl").write_text(source)
        (self.root / "source.pdl").write_text("iReset\niWrite child.r 1\niApply\n")
        self.run_tool("--icl", "source.icl", "--pdl", "source.pdl", "--local_reset", "reset.pdl")
        self.assertEqual((self.root / "reset.pdl").read_bytes(), (self.root / "source.pdl").read_bytes())
        for connection in ("1'bx", "loop"):
            invalid = source.replace("InputPort si = release;",
                                     f"InputPort si = release; InputPort rst = {connection};")
            if connection == "loop":
                invalid = invalid.replace(" ScanRegister release", " ToResetPort loop { Source loop; }\n ScanRegister release")
            (self.root / "source.icl").write_text(invalid)
            failure = self.run_tool("--icl", "source.icl", "--pdl", "source.pdl",
                                    "--local_reset", "invalid-reset.pdl", ok=False)
            self.assertIn("cycle" if connection == "loop" else "unknown local-reset",
                          failure.stdout)
            self.assertFalse((self.root / "invalid-reset.pdl").exists())
        source = source.replace("InputPort si = release;", "InputPort si = release; InputPort rst = ~release;")
        (self.root / "source.icl").write_text(source)
        (self.root / "reset.pdl").write_text("preserve reset output\n")
        (self.root / "result.pdl").write_text("preserve pattern output\n")
        self.run_tool("--icl", "source.icl", "--pdl", "source.pdl",
                      "--local_reset", "reset.pdl", "--output", "result.pdl")
        self.assertIn("iWrite release 0b1\niApply\niWrite child.r 1",
                      (self.root / "reset.pdl").read_text())

    def test_transitive_local_reset_rewrite_preserves_procedures(self):
        fixture = pathlib.Path(__file__).resolve().parents[1] / "cases/ours/pattern_generation/local_reset"
        self.run_tool("--icl", fixture / "nested.icl", "--pdl",
                      fixture / "procedures.pdl", fixture / "main.pdl",
                      "--local_reset", "reset.pdl", "--output", "result.pdl")
        rewritten = (self.root / "reset.pdl").read_text()
        self.assertTrue(rewritten.startswith((fixture / "procedures.pdl").read_text()))
        self.assertIn("iWrite root 0b1\niApply\niWrite branch.release 0b1\niApply\n", rewritten)
        self.assertTrue(rewritten.endswith("iCall configure 10\n"))
        self.assertNotIn("iWrite gate", rewritten)
        original = (self.root / "result.pdl").read_bytes()
        self.run_tool("--icl", fixture / "nested.icl", "--pdl", "reset.pdl",
                      "--local_reset", "again-reset.pdl", "--output", "replayed.pdl")
        self.assertEqual(rewritten, (self.root / "again-reset.pdl").read_text())
        self.assertIn(b"iWrite a.value 0b1010", original)
        self.assertIn(b"iWrite a.value 0b1010", (self.root / "replayed.pdl").read_bytes())

    def test_local_reset_groups_parallel_controls_before_user_queue(self):
        fixture = pathlib.Path(__file__).resolve().parents[1] / "cases/ours/pattern_generation/local_reset"
        source = "iReset\niWrite a.value 5; iWrite b.value 6; iApply\n"
        (self.root / "source.pdl").write_text(source)
        self.run_tool("--icl", fixture / "nested.icl", fixture / "parallel.icl",
                      "--top_module", "ParallelResetNetwork", "--pdl", "source.pdl",
                      "--local_reset", "reset.pdl")
        expected = "iReset\niWrite ra 0b1\niWrite rb 0b0\niApply\n" + source.split("\n", 1)[1]
        self.assertEqual((self.root / "reset.pdl").read_text(), expected)

    def test_local_reset_tap_round_trips(self):
        fixture = pathlib.Path(__file__).resolve().parents[1] / "cases/ours/pattern_generation"
        inputs = ("--icl", fixture / "local_reset/nested.icl", fixture / "local_reset/vendor.icl",
                  "--bsdl", "AccessProbe", fixture / "access_link/simple.bsdl",
                  "--top_module", "ResetAccessTop")
        self.run_tool(*inputs, "--pdl", fixture / "local_reset/vendor.pdl",
                      "--output", "result.pdl", "--local_reset", "reset.pdl", "--debug")
        original = (self.root / "result.pdl").read_bytes()
        rewritten = (self.root / "reset.pdl").read_text()
        self.assertIn("iWrite core.root 0b1\niApply\niWrite core.branch.release 0b1\niApply\n", rewritten)
        self.assertNotIn("iWrite core.gate", rewritten)
        self.run_tool(*inputs, "--pdl", "reset.pdl", "--output", "reset-result.pdl",
                      "--local_reset", "reset-again.pdl")
        self.assertEqual(rewritten, (self.root / "reset-again.pdl").read_text())
        self.run_tool(*inputs, "--pdl", "result.pdl", "--output", "again.pdl")
        self.assertEqual(original, (self.root / "again.pdl").read_bytes())
        flat = self.root / "debug/ResetAccessTop.icl"
        self.run_tool("--icl", flat, "--pdl", fixture / "local_reset/vendor.pdl",
                      "--output", "flat/result.pdl", "--debug")
        self.assertEqual(original, (self.root / "flat/result.pdl").read_bytes())
        self.assertEqual(flat.read_bytes(), (self.root / "flat/debug/ResetAccessTop.icl").read_bytes())

    def test_local_reset_prefix_and_called_body_failure(self):
        fixture = pathlib.Path(__file__).resolve().parents[1] / "cases/ours/pattern_generation/local_reset/nested.icl"
        body = "iProc configure {v} { iWrite value $v; iApply }"
        source = "iPDLLevel 1\niProcsForModule Leaf\n" + body + "\niReset\niPrefix a\niCall configure 7\n"
        (self.root / "source.pdl").write_text(source)
        self.run_tool("--icl", fixture, "--pdl", "source.pdl", "--local_reset", "reset.pdl")
        rewritten = (self.root / "reset.pdl").read_text()
        self.assertIn(body, rewritten)
        self.assertIn("iPrefix -\niWrite root 0b1\niApply\niWrite branch.release 0b1\niApply\niPrefix a\niCall", rewritten)
        self.run_tool("--icl", fixture, "--pdl", "reset.pdl", "--output", "replayed.pdl")
        source = "iPDLLevel 1\niProcsForModule Network\niProc bad {} { iWrite root 0; iApply; iWrite a.value 7; iApply }\niReset\niCall bad\n"
        (self.root / "source.pdl").write_text(source)
        (self.root / "reset.pdl").write_text("previous artifact\n")
        failure = self.run_tool("--icl", fixture, "--pdl", "source.pdl", "--local_reset", "reset.pdl", ok=False)
        self.assertIn("cannot repair", failure.stdout)
        self.assertIn("iCall bad", failure.stdout)
        self.assertEqual((self.root / "reset.pdl").read_text(), "previous artifact\n")

    def test_local_reset_rejects_unfaithful_source_insertion(self):
        fixture = pathlib.Path(__file__).resolve().parents[1] / "cases/ours/pattern_generation/local_reset/nested.icl"
        for source in (
                "iPDLLevel 1\niReset\nforeach v {1 2} { iWrite a.value $v }\niApply\n",
                "iPDLLevel 1\niReset\nforeach v {a b} { iPrefix $v; iWrite value 3 }\niApply\n",
                "iPDLLevel 1\niWrite a.value [iReset; expr 3]\niApply\n"):
            with self.subTest(source=source):
                (self.root / "source.pdl").write_text(source)
                (self.root / "reset.pdl").write_text("previous artifact\n")
                failure = self.run_tool("--icl", fixture, "--pdl", "source.pdl",
                                        "--local_reset", "reset.pdl", ok=False)
                self.assertIn("local-reset", failure.stdout)
                self.assertEqual((self.root / "reset.pdl").read_text(), "previous artifact\n")

    def test_local_reset_nested_access_together(self):
        fixture = pathlib.Path(__file__).resolve().parents[1] / "cases/ours/pattern_generation/local_reset"
        source = (fixture / "parallel.icl").read_text()
        source = source.rsplit("}", 1)[0] + """
 Alias inner[7:0] = a.value, b.value { AccessTogether; }
 Alias outer[4:0] = ra, inner[7:4] { AccessTogether; }
}
"""
        (self.root / "source.icl").write_text(source)
        (self.root / "source.pdl").write_text("iReset\niWrite outer[4] 1\niApply\n")
        self.run_tool("--icl", fixture / "nested.icl", "source.icl", "--top_module",
                      "ParallelResetNetwork", "--pdl", "source.pdl", "--local_reset", "reset.pdl")
        self.assertIn("iWrite ra 0b1\niWrite rb 0b0\niApply\n",
                      (self.root / "reset.pdl").read_text())

    def test_local_reset_multi_file_continued_comment(self):
        (self.root / "source.pdl").write_text("iPDLLevel 1\niState gate 0\n# continued comment " + chr(92) + "\n")
        (self.root / "next.pdl").write_text("iWrite target 10\niApply\n")
        self.run_tool("--icl", "source.icl", "--pdl", "source.pdl", "next.pdl",
                      "--local_reset", "reset.pdl", "--output", "first.pdl")
        self.run_tool("--icl", "source.icl", "--pdl", "reset.pdl", "--output", "second.pdl")
        self.assertEqual((self.root / "first.pdl").read_bytes(), (self.root / "second.pdl").read_bytes())

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

    def test_top_level_port_assumptions(self):
        fixture = pathlib.Path(__file__).resolve().parents[1] / "cases/ours/pattern_generation/access_link"
        source = (fixture / "simple.icl").read_text().replace(
            "Module AccessTop {", "Module AccessTop {\n  SelectPort enabled; DataInPort mode;"
        ).replace("InputPort mode = tap.long_setup;", "InputPort mode = mode; InputPort select = enabled;")
        (self.root / "source.icl").write_text(source)
        (self.root / "source.pdl").write_text(
            "iReset\niState enabled 1\niState mode 1\niWrite core.r 0x59\niApply\n"
            "iState enabled 0\niRunLoop 1\niState enabled 1 -LastWrittenValue\n"
            "iRead core.r 0x59\niApply\n")
        inputs = ("--icl", "source.icl", "--bsdl", "AccessProbe", fixture / "simple.bsdl")
        self.run_tool(*inputs, "--pdl", "source.pdl", "--debug")
        output = (self.root / "out/AccessTop.pdl").read_text()
        self.assertIn("iState enabled 0b1\n", output)
        self.assertIn("iState mode 0b1\n", output)
        self.assertIn("iWrite core.r 0b01011001\n", output)
        self.run_tool(*inputs, "--pdl", "out/AccessTop.pdl", "--output", "again.pdl")
        self.assertEqual(output, (self.root / "again.pdl").read_text())
        self.run_tool("--icl", "out/debug/AccessTop.icl", "--pdl", "source.pdl", "--output", "flat.pdl")
        self.assertEqual(output, (self.root / "flat.pdl").read_text())
        for target in ("core.mode", "core.select", "core.so"):
            with self.subTest(target=target):
                (self.root / "bad.pdl").write_text(f"iReset; iState {target} 1\n")
                for design in (inputs, ("--icl", "out/debug/AccessTop.icl")):
                    result = self.run_tool(*design, "--pdl", "bad.pdl", "--output", "bad-result.pdl", ok=False)
                    self.assertIn("bad.pdl", result.stdout)
                    self.assertIn("top-level", result.stdout)
                    self.assertFalse((self.root / "bad-result.pdl").exists())

    def test_enumerated_operands(self):
        fixture = pathlib.Path(__file__).resolve().parents[1] / "cases/ours/pattern_generation/enumerated_values.icl"
        (self.root / "source.pdl").write_text(
            "iProcsForModule EnumChild\niProc configure {value} {iWrite r $value; iApply}\n"
            "iProcsForModule EnumTop\niCall a.configure Active\niCall b.configure Active\n"
            "iWrite a.r 0xf\niApply\niWrite a.r Mask\niApply\n"
            "iRead a.r Mask\niApply\niWrite a.complement Active\niApply\n"
            "iWrite a.r(2:3) Clear\niApply\niRead b.r Active\niRead a.r AllX\niApply\n")
        inputs = ("--icl", fixture)
        self.run_tool(*inputs, "--pdl", "source.pdl", "--debug")
        output = (self.root / "out/EnumTop.pdl").read_text()
        self.assertIn("iWrite a.r 0b0101\n", output)
        self.assertIn("iWrite b.r 0b1010\n", output)
        self.assertIn("iWrite a.r 0b1101\n", output)
        self.assertIn("iRead a.r 0bxx01\n", output)
        self.assertIn("iWrite a.r 0b0100\n", output)
        self.assertIn("iRead b.r 0b1010\n", output)
        self.assertIn("iRead a.r 0bxxxx\n", output)
        self.run_tool(*inputs, "--pdl", "out/EnumTop.pdl", "--output", "again.pdl")
        self.assertEqual(output, (self.root / "again.pdl").read_text())
        self.run_tool("--icl", "out/debug/EnumTop.icl", "--pdl", "source.pdl", "--output", "flat.pdl")
        self.assertEqual(output, (self.root / "flat.pdl").read_text())
        for command, message in (
                ("iWrite a.r Missing", "unknown enum value"),
                ("iRead a.unnamed Active", "no RefEnum"),
                ("iWrite a.r(2:3) Active", "truncates significant bits")):
            with self.subTest(command=command):
                (self.root / "bad.pdl").write_text(command + "; iApply\n")
                result = self.run_tool(*inputs, "--pdl", "bad.pdl", "--output", "bad-result.pdl", ok=False)
                self.assertIn("bad.pdl", result.stdout)
                self.assertIn(message, result.stdout)
                self.assertFalse((self.root / "bad-result.pdl").exists())

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
