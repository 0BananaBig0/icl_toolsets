#!/usr/bin/env python3
"""Opt-in modeled observations, independent of expected comparison data."""
import subprocess
import unittest

import test_simultaneous_scan_chains as helpers

BASE = """Module Chains {
 DataInPort p[3:0]; DataOutPort q[3:0] {Source p;}
 Alias inverted[3:0] = ~q;
}"""
LABEL = b"modeled combinational PIO readback; no live device"


class ModeledReadback(unittest.TestCase):
    setUp = helpers.SimultaneousChains.setUp
    replay = helpers.SimultaneousChains.replay

    def run_tool(self, icl, pdl, output="result.pdl", ok=True,
                 modeled=True, extra=()):
        result = subprocess.run(
            [str(helpers.BIN), "--icl", str(icl), "--pdl", str(pdl),
             "--top_module", "Chains", "--output", str(self.root / output),
             "--debug", *(["--model_readback"] if modeled else []), *extra],
            capture_output=True, text=True, timeout=45)
        self.assertEqual(result.returncode == 0, ok, result.stdout + result.stderr)
        return ((self.root / output).read_bytes() if ok else
                result.stdout + result.stderr)

    def test_actual_values_counts_clear_and_formats(self):
        output = self.replay(BASE, """iPDLLevel 1
iState p 15; iRead q 0; iApply
iNote [iGetReadData q -bin]
iNote [iGetReadData inverted -hex]
iNote [iGetReadData q -dec]
iNote [iGetMiscompares inverted -bin]
iNote [iGetStatus]; iNote [iGetStatus -clear]; iNote [iGetStatus]
""")
        self.assertIn(b"iNote 0b1111", output)
        self.assertIn(b"iNote 0x0", output)
        self.assertIn(b"iNote 15", output)
        self.assertEqual(output.count(b"iNote 4\n"), 2)
        self.assertIn(b"iNote 0\n", output)
        self.assertEqual(output.count(LABEL), 1)

    def test_masks_latest_reads_partial_history_and_accumulation(self):
        output = self.replay(BASE, """iPDLLevel 1
iState p 15; iRead q 0; iRead q[1:0] 0bx1; iApply
iNote [iGetMiscompares q -bin]; iNote [iGetStatus]
iRead q[0] 0; iApply
iNote [iGetStatus]; iNote [iGetReadData q -bin]
""")
        self.assertIn(b"iNote 0b1100", output)
        self.assertIn(b"iNote 2\n", output)
        self.assertIn(b"iNote 3\n", output)
        self.assertIn(b"iNote 0b1111", output)

    def test_unknown_actual_and_dont_care(self):
        output = self.replay(BASE, """iPDLLevel 1
iRead q 0; iApply
iNote [iGetReadData q -bin]; iNote [iGetMiscompares q -bin]
if {![catch {iGetStatus -clear}]} {error {unknown status returned}}
if {![catch {iGetStatus}]} {error {unknown status cleared}}
iRead q 0bxxxx; iApply; iNote [iGetMiscompares q -bin]
""")
        self.assertIn(b"iNote 0bxxxx", output)
        self.assertIn(b"iNote 0b0000", output)

    def test_reset_cancels_pending_but_preserves_count(self):
        output = self.replay(BASE, """iPDLLevel 1
iState p 15; iRead q 0; iApply; iNote [iGetStatus]
iWrite p 0; iRead q 0; iReset
iNote [iGetStatus]
iRead q 0bxxxx; iApply; iNote [iGetReadData q -bin]
""")
        self.assertEqual(output.count(b"iNote 4\n"), 2)
        self.assertIn(b"iNote 0b1111", output)

    def test_default_unavailable_and_failed_output_preserved(self):
        icl, pdl = self.root / "source.icl", self.root / "source.pdl"
        icl.write_text(BASE)
        pdl.write_text("iPDLLevel 1; iState p 15; iRead q 0; iApply; iGetReadData q")
        output = self.root / "result.pdl"
        output.write_bytes(b"preserve")
        self.assertIn("hardware readback",
                      self.run_tool(icl, pdl, ok=False, modeled=False))
        self.assertEqual(output.read_bytes(), b"preserve")
        pdl.write_text("iPDLLevel 1; iRead q 0; iApply; iGetStatus")
        self.assertIn("unknown", self.run_tool(icl, pdl, ok=False))
        self.assertEqual(output.read_bytes(), b"preserve")

    def test_prefix_procedure_once_only_and_input_files(self):
        source = """Module Leaf {
 DataInPort p[3:0]; DataOutPort q[3:0] {Source p;}
}
Module Chains {
 DataInPort p[3:0];
 Instance leaf Of Leaf {InputPort p = p;}
 DataOutPort q[3:0] {Source leaf.q;}
}"""
        icl, first, second = (self.root / n for n in
                              ("source.icl", "first.pdl", "second.pdl"))
        icl.write_text(source)
        first.write_text("""iPDLLevel 1
set calls 0
proc value {} {incr ::calls; return 15}
iState p [value]
iPrefix leaf; iRead q 0
""")
        second.write_text("""iApply
iNote [iGetReadData q -hex]
iNote [iGetStatus -clear]; iNote [iGetStatus]
if {$calls != 1} {error {argument evaluated twice}}
""")
        output = self.run_tool(icl, first, extra=("--pdl", str(second)))
        self.assertIn(b"iNote 0xf", output)
        self.assertIn(b"iNote 4\n", output)
        self.assertIn(b"iNote 0\n", output)
        self.assertEqual(output, self.run_tool(
            icl, self.root / "result.pdl", "replayed.pdl"))

    def test_queued_state_and_resources_match_generator(self):
        output = self.replay(BASE, """iPDLLevel 1
iTake p; iState p 0
iWrite p 0; iRead q 0; iState p 15; iApply -together
iNote [iGetReadData q -bin]; iNote [iGetStatus]
iRelease p
""")
        self.assertIn(b"iNote 0b1111", output)
        self.assertIn(b"iNote 4\n", output)

    def test_never_read_and_sparse_history(self):
        output = self.replay(BASE, """iPDLLevel 1
iNote [iGetReadData q -hex]
iState p 15; iRead q[1] 0; iApply
iNote [iGetReadData q -bin]; iNote [iGetMiscompares q -bin]
iNote [iGetReadData q -dec]
""")
        self.assertIn(b"iNote 0xX", output)
        self.assertIn(b"iNote 0bxx1x", output)
        self.assertIn(b"iNote X", output)

    def test_backlog_counts_each_apply_but_only_latest_history(self):
        output = self.replay(BASE, """iPDLLevel 1
iState p 15; iRead q 0; iApply
iState p 0; iRead q 0; iApply
iNote [iGetReadData q -bin]; iNote [iGetMiscompares q -bin]
iNote [iGetStatus]
iState p 3; iRead q 0; iApply; iNote [iGetStatus -clear]
iRead q 3; iApply; iNote [iGetStatus]
""")
        self.assertIn(b"iNote 0b0000", output)
        self.assertIn(b"iNote 4\n", output)
        self.assertIn(b"iNote 6\n", output)
        self.assertIn(b"iNote 0\n", output)

    def test_unknown_count_survives_later_known_apply_and_reset(self):
        output = self.replay(BASE, """iPDLLevel 1
iRead q 0; iApply
if {![catch {iGetStatus -clear}]} {error {unknown count returned}}
iState p 0; iRead q 0; iApply
if {![catch {iGetStatus}]} {error {unknown count lost}}
iReset
if {![catch {iGetStatus -clear}]} {error {reset cleared count}}
iNote [iGetReadData q -bin]
""")
        self.assertIn(b"iNote 0b0000", output)

    def test_unsupported_histories_preserve_output(self):
        icl, pdl = self.root / "source.icl", self.root / "source.pdl"
        output = self.root / "result.pdl"
        for source, history in (
                (BASE.replace(" DataOutPort", " DataRegister r {"
                              "WriteDataSource p[0];}\n DataOutPort"), ""),
                (BASE, "iRunLoop 1 -tck; "),
                (BASE, "iMerge -begin; iMerge -end; ")):
            with self.subTest(history=history):
                icl.write_text(source)
                pdl.write_text("iPDLLevel 1; " + history +
                               "iState p 15; iRead q 0; iApply; iGetReadData q")
                output.write_bytes(b"preserve")
                self.assertIn("modeled readback",
                              self.run_tool(icl, pdl, ok=False))
                self.assertEqual(output.read_bytes(), b"preserve")

    def test_raw_history_rejected_and_known_manual_history_preserved(self):
        source = """Module Chains {
 ScanInPort si; SelectPort sel; CaptureEnPort ce;
 ShiftEnPort se; UpdateEnPort ue; TCKPort ck;
 ScanRegister r[3:0] {ScanInSource si; CaptureSource r; ResetValue 0;}
 ScanOutPort so {Source r[0];}
 ScanInterface client {Port si; Port so; Port sel; Port ce; Port se; Port ue; Port ck;}
}"""
        icl, pdl = self.root / "source.icl", self.root / "source.pdl"
        icl.write_text(source)
        pdl.write_text("""iPDLLevel 1
iReset; iRead r 0; iApply
iState r 5 -LastReadValue; iNote [iGetReadData r -dec]
""")
        self.assertIn(b"iNote 5\n", self.run_tool(icl, pdl))
        pdl.write_text("""iPDLLevel 1
iReset; iScan client 4 -si 0 -so 0; iApply; iGetStatus
""")
        self.assertIn("modeled readback", self.run_tool(icl, pdl, ok=False))

    def test_local_reset_preserves_procedure_source(self):
        body = """iProc observe {} {
 iRead q 0; iApply
 iNote [iGetReadData q -bin]
}
"""
        icl, pdl = self.root / "source.icl", self.root / "source.pdl"
        icl.write_text(BASE)
        pdl.write_text("iPDLLevel 1\niProcsForModule Chains\n" + body +
                       "iState p 15; iCall observe\n")
        output = self.run_tool(
            icl, pdl, extra=("--local_reset", str(self.root / "reset.pdl")))
        self.assertIn(b"iNote 0b1111", output)
        reset = (self.root / "reset.pdl").read_text()
        self.assertIn(body, reset)
        self.assertEqual(pdl.read_text(), reset)

    def test_unused_mode_preserves_execution_without_eager_support_checks(self):
        source = BASE.replace(" DataOutPort", " DataRegister r {"
                              "WriteDataSource p[0];}\n DataOutPort")
        icl, pdl = self.root / "source.icl", self.root / "source.pdl"
        icl.write_text(source)
        pdl.write_text("iPDLLevel 1; iState p 15; iRead q 15; iApply")
        default = self.run_tool(icl, pdl, modeled=False)
        modeled = self.run_tool(icl, pdl)
        self.assertEqual(
            [line for line in modeled.splitlines() if LABEL not in line],
            default.splitlines())

    def test_wide_sparse_alias_and_no_compare_status(self):
        source = BASE.replace("[3:0]", "[1023:0]")
        output = self.replay(source, """iPDLLevel 1
iState p 1; iRead q[1023] 0bx; iRead inverted[0] 0; iApply
iNote [iGetReadData inverted[0] -dec]
iNote [iGetMiscompares q[1023] -dec]; iNote [iGetStatus]
""")
        self.assertEqual(output.count(b"iNote 0\n"), 3)

    def test_first_observation_uses_its_scheduled_frame(self):
        source = """Module Chains {
 DataInPort p; DataInPort sel;
 DataOutPort a {Source p; Enable sel;}
 DataOutPort b {Source p; Enable ~sel;}
 Alias group = a {AccessTogether;}
}"""
        for expected, count in (("1", 0), ("0", 2)):
            with self.subTest(expected=expected):
                output = self.replay(source, "iPDLLevel 1; iState p 1; "
                                     "iRead group " + expected + "; iRead b " +
                                     expected + "; iApply; "
                                     "iNote [iGetReadData a -bin]; "
                                     "iNote [iGetReadData b -bin]; "
                                     "iNote [iGetMiscompares b -bin]; "
                                     "iNote [iGetStatus]")
                self.assertEqual(output.count(b"iNote 0b1\n"),
                                 3 if count else 2)
                self.assertIn(("iNote " + str(count) + "\n").encode(), output)

    def test_clear_proves_zero_across_empty_unsupported_history(self):
        output = self.replay(BASE, """iPDLLevel 1
iState p 1; iRead q 0; iApply
iNote [iGetStatus -clear]
iMerge -begin; iMerge -end
iNote [iGetStatus]
""")
        self.assertIn(b"iNote 1\n", output)
        self.assertIn(b"iNote 0\n", output)


if __name__ == "__main__":
    unittest.main()
