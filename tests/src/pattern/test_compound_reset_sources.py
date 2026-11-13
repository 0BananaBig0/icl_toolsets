#!/usr/bin/env python3
"""Source-preserving reset prerequisites around main Tcl commands."""
import pathlib
import subprocess
import sys
import tempfile
import time
import unittest

BIN = pathlib.Path(sys.argv.pop(1)).resolve()
FIXTURE = pathlib.Path(__file__).resolve().parents[2] / "cases/ours/pattern_generation/local_reset/nested.icl"
RELEASE = "iWrite root 0b1\niApply\niWrite branch.release 0b1\niApply\n"


class CompoundResetSources(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = pathlib.Path(self.temp.name)

    def run_tool(self, *args, ok=True):
        result = subprocess.run([str(BIN), *map(str, args)], cwd=self.root,
                                capture_output=True, text=True, timeout=30)
        self.assertEqual(result.returncode == 0, ok, result.stdout + result.stderr)
        return result

    def check(self, source, expected):
        (self.root / "source.pdl").write_text(source)
        self.run_tool("--icl", FIXTURE, "--pdl", "source.pdl", "--output", "out.pdl",
                      "--local_reset", "reset.pdl", "--debug")
        self.assertEqual((self.root / "reset.pdl").read_text(), expected)
        self.run_tool("--icl", FIXTURE, "--pdl", "reset.pdl", "--output", "replay.pdl",
                      "--local_reset", "again.pdl")
        self.assertEqual((self.root / "again.pdl").read_text(), expected)
        output = (self.root / "out.pdl").read_bytes()
        self.run_tool("--icl", FIXTURE, "--pdl", "out.pdl", "--output", "fixed.pdl")
        self.assertEqual(output, (self.root / "fixed.pdl").read_bytes())
        self.run_tool("--icl", "debug/Network.icl", "--pdl", "source.pdl",
                      "--output", "flat.pdl", "--local_reset", "flat-reset.pdl")
        self.assertEqual(output, (self.root / "flat.pdl").read_bytes())
        self.assertEqual(expected, (self.root / "flat-reset.pdl").read_text())

    def test_repeated_pending_writes(self):
        head = "iPDLLevel 1\niReset\n"
        body = "foreach v {1 2} { iWrite a.value $v }\niApply\n"
        self.check(head + body, head + RELEASE + body)

    def test_prefix_changes_inside_compound(self):
        head = "iPDLLevel 1\niReset\niPrefix a\n"
        body = "foreach v {a b} { iPrefix $v; iWrite value 3 }\niApply\n"
        self.check(head + body, head + "iPrefix -\n" + RELEASE + "iPrefix a\n" + body)

    def test_nested_conditionals_and_applies(self):
        head = "iPDLLevel 1\niReset\nset enabled 1\n"
        body = "if {$enabled} { foreach v {1 2} { iWrite a.value $v; iApply } }\n"
        self.check(head + body, head + RELEASE + body)

    def test_dynamic_commands(self):
        head = "iPDLLevel 1\niReset\nset command iWrite\n"
        for body in ("$command a.value 3\niApply\n",
                     "eval {iWrite a.value 3; iApply}\n",
                     "if {1} {iWrite a.value 3}; # keep this comment\niApply\n"):
            with self.subTest(body=body):
                self.check(head + body, head + RELEASE + body)

    def test_compound_merge_across_files(self):
        head = ("iPDLLevel 1\niProcsForModule Network\n"
                "iProc put {v} { iWrite a.value $v; iApply }\niReset\n")
        body = "if {1} {iMerge -begin; iCall put 3}\n"
        tail = "iCall put 4\niMerge -end\n"
        (self.root / "first.pdl").write_text(head + body)
        (self.root / "second.pdl").write_text(tail)
        self.run_tool("--icl", FIXTURE, "--pdl", "first.pdl", "second.pdl",
                      "--output", "out.pdl", "--local_reset", "reset.pdl")
        expected = head + RELEASE + body + "\n\n" + tail
        self.assertEqual((self.root / "reset.pdl").read_text(), expected)
        self.run_tool("--icl", FIXTURE, "--pdl", "reset.pdl", "--output", "again.pdl",
                      "--local_reset", "fixed.pdl")
        self.assertEqual((self.root / "fixed.pdl").read_text(), expected)

    def test_compound_closes_existing_merge(self):
        head = "iPDLLevel 1\niReset\n"
        body = ("iMerge -begin\n"
                "foreach v {1 2} { if {$v == 1} {iMerge -end}; iWrite a.value $v }\n"
                "iApply\n")
        self.check(head + body, head + RELEASE + body)

    def test_queue_before_compound(self):
        head = "iPDLLevel 1\niReset\n"
        body = "iWrite b.value 7\nforeach v {1 2} { iWrite a.value $v }\niApply\n"
        self.check(head + body, head + RELEASE + body)

    def test_procedure_and_merge_preservation(self):
        head = ("iPDLLevel 1\niProcsForModule Network\n"
                "iProc put {v} { iWrite a.value $v; iApply }\niReset\n")
        body = "foreach v {1 2} { iMerge -begin; iCall put $v; iMerge -end }\n"
        self.check(head + body, head + RELEASE + body)
        body = "iMerge -begin\nforeach v {1 2} { iCall put $v }\niMerge -end\n"
        self.check(head + body, head + RELEASE + body)

    def test_tcl_control_flow(self):
        head = "iPDLLevel 1\niReset\n"
        for body in (
                "foreach v {1 2 3} { if {$v == 1} {continue}; iWrite a.value $v; break }\niApply\n",
                "catch { iWrite a.value 3; error stopped }; iApply\n",
                "if {1} { iWrite a.value 3; iApply; return }\niWrite absent 1\niApply\n"):
            with self.subTest(body=body):
                self.check(head + body, head + RELEASE + body)

    def test_reset_inside_loop_retains_literal_repairs(self):
        head = "iPDLLevel 1\n"
        body = "foreach v {1 2} { iReset; iWrite a.value $v; iApply }\n"
        expected = head + body.replace("iWrite a.value", RELEASE + "iWrite a.value")
        self.check(head + body, expected)

    def test_unrepairable_entry_preserves_artifacts(self):
        for body in ("iWrite a.value [iReset; expr 3]\niApply\n",):
            with self.subTest(body=body):
                (self.root / "source.pdl").write_text("iPDLLevel 1\n" + body)
                for path in ("out.pdl", "reset.pdl"):
                    (self.root / path).write_text("previous artifact\n")
                failure = self.run_tool("--icl", FIXTURE, "--pdl", "source.pdl",
                                        "--output", "out.pdl", "--local_reset", "reset.pdl", ok=False)
                self.assertIn("local-reset", failure.stdout)
                for path in ("out.pdl", "reset.pdl"):
                    self.assertEqual((self.root / path).read_text(), "previous artifact\n")

    def test_many_iterations(self):
        head = "iPDLLevel 1\niReset\n"
        body = "for {set v 0} {$v < 256} {incr v} { iWrite a.value [expr {$v % 16}] }\niApply\n"
        start = time.monotonic()
        self.check(head + body, head + RELEASE + body)
        print(f"reset source: 256 iterations, {len(head + body)} bytes, four runs {time.monotonic() - start:.3f}s")


if __name__ == "__main__":
    unittest.main()
