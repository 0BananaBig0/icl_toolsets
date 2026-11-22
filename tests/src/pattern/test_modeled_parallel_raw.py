#!/usr/bin/env python3
"""Actual modeled parallel raw returns and exact source/flat/generated replay."""
import unittest

import test_modeled_readback as helpers
import test_modeled_scan_readback as scan
import test_simultaneous_scan_chains as cli

MODEL = cli.NETWORK
START = "iPDLLevel 1\niReset\n"


class ModeledParallelRaw(unittest.TestCase):
    setUp = helpers.ModeledReadback.setUp
    run_tool = helpers.ModeledReadback.run_tool
    replay = helpers.ModeledReadback.replay
    notes = scan.ModeledScanReadback.notes

    def assert_failure(self, source, commands, diagnostic, extra=()):
        icl, pdl = self.root / "bad.icl", self.root / "bad.pdl"
        icl.write_text(source)
        pdl.write_text(commands)
        output = self.root / "result.pdl"
        output.write_bytes(b"old artifact\n")
        message = self.run_tool(icl, pdl, ok=False, extra=extra)
        self.assertIn(diagnostic, message)
        self.assertEqual(output.read_bytes(), b"old artifact\n")

    def test_actual_returns_care_counts_and_one_apply(self):
        output = self.replay(MODEL, START + """
iScan client -chain first 4 -si 9 -so 15
iScan client -chain second 2 -si 2 -so 3
iApply -together
iNote [iGetReadData client -chain first -dec]
iNote [iGetReadData client -chain second -dec]
iNote [iGetMiscompares client -chain first -bin]
iNote [iGetMiscompares client -chain second -bin]
iNote [iGetStatus -clear]; iNote [iGetStatus]
iScan client -chain first 4 -si 6 -so 0
iScan client -chain second 2 -si 1 -so 0
iApply
iNote [iGetReadData client -chain first -dec]
iNote [iGetReadData client -chain second -dec]
iNote [iGetStatus]
""")
        self.assertEqual(self.notes(output),
                         [b"0", b"0", b"0b1111", b"0b11", b"6", b"0",
                          b"9", b"2", b"3"])
        self.assertEqual(output.count(b"iApply"), 2)
        self.assertEqual(output.count(b"iScan"), 4)

    def test_latest_queue_keeps_each_chain(self):
        output = self.replay(MODEL, START + """
iScan client -chain first 4 -si 3
iScan client -chain second 2 -si 1
iScan client -chain first 4 -si 9
iApply
iScan client -chain first 4 -si 6
iScan client -chain second 2 -si 2
iApply
iNote [iGetReadData client -chain first -dec]
iNote [iGetReadData client -chain second -dec]
""")
        self.assertEqual(self.notes(output), [b"9", b"1"])
        self.assertEqual(output.count(b"iScan"), 4)

    def test_implicit_field_preserves_fill_and_source_history(self):
        output = self.replay(MODEL, START + """
iScan client -chain first 4 -si 9
iScan client -chain second 2 -si 2
iApply
iScan client -chain first 4 -si 6
iApply
iNote [iGetReadData client -chain first -dec]
iNote [iGetReadData client -chain second -dec]
iNote [iGetMiscompares client -chain second -bin]
iRead b 2; iApply
iNote [iGetReadData b -dec]
""")
        self.assertEqual(self.notes(output), [b"9", b"0", b"0b00", b"2"])
        self.assertIn(b"-chain second 2 -si 0b10 -so 0bxx -stable", output)

    def test_implicit_field_does_not_create_a_history(self):
        output = self.replay(MODEL, START + """
iScan client -chain first 4 -si 9; iApply
if {![catch {iGetReadData client -chain second}]} {error {implicit history}}
if {[iGetReadData a -bin] ne "0bxxxx"} {error {raw fabricated register history}}
iNote [iGetReadData client -chain first -hex]
""")
        self.assertEqual(self.notes(output), [b"0x0"])

    def test_named_default_is_literal(self):
        output = self.replay(MODEL.replace("Chain first", "Chain default"),
                             START + """
iScan client -chain default 4 -si 9
iScan client -chain second 2 -si 2
iApply
iScan client -chain default 4 -si 6; iApply
iNote [iGetReadData client -chain default -dec]
""")
        self.assertEqual(self.notes(output), [b"9"])
        self.assertIn(b"-chain default", output)

    def test_constant_unknown_and_inverted_capture(self):
        variants = (
            (MODEL.replace("CaptureSource a", "CaptureSource 4'b0101"),
             b"0b0101", b"0b0101"),
            (MODEL.replace("CaptureSource a", "CaptureSource 4'bxxxx"),
             b"0bxxxx", b"0bxxxx"),
            (MODEL.replace("Source a[0]", "Source ~a[0]"),
             b"0b1111", b"0b1111"),
        )
        for source, value, mismatch in variants:
            with self.subTest(value=value):
                output = self.replay(source, START + """
iScan client -chain first 4 -si 9 -so 0; iApply
iNote [iGetReadData client -chain first -bin]
iNote [iGetMiscompares client -chain first -bin]
""")
                self.assertEqual(self.notes(output), [value, mismatch])

    def test_ascending_indices_and_input_output_inversion(self):
        source = MODEL.replace("a[3:0]", "a[0:3]").replace("a[0]", "a[3]")
        source = source.replace("ScanInSource si[2]", "ScanInSource ~si[2]")
        source = source.replace("b[0]; }", "~b[0]; }")
        output = self.replay(source, START + """
iScan client -chain first 4 -si 9
iScan client -chain second 2 -si 2
iApply
iNote [iGetReadData client -chain second -dec]
iScan client -chain first 4 -si 6
iScan client -chain second 2 -si 1
iApply
iNote [iGetReadData client -chain first -dec]
iNote [iGetReadData client -chain second -dec]
""")
        self.assertEqual(self.notes(output), [b"3", b"9", b"2"])

    def test_reset_cancellation_before_replay_and_history_survives(self):
        output = self.replay(MODEL, """iPDLLevel 1
iScan client -chain first 3 -si 1
iReset
iScan client -chain first 4 -si 9; iApply
iScan client -chain second 3 -si 1; iReset
iNote [iGetReadData client -chain first -dec]
iScan client -chain first 4 -si 6; iApply
iNote [iGetReadData client -chain first -dec]
""")
        self.assertEqual(self.notes(output), [b"0", b"0"])

    def test_split_files_share_chain_queue(self):
        icl, first, second = (self.root / name for name in
                              ("source.icl", "first.pdl", "second.pdl"))
        icl.write_text(MODEL)
        first.write_text(START + "iScan client -chain first 4 -si 9\n")
        second.write_text("""iScan client -chain second 2 -si 2
iApply
iScan client -chain first 4 -si 6; iApply
iNote [iGetReadData client -chain first -dec]
""")
        output = self.run_tool(icl, first, extra=("--pdl", str(second)))
        self.assertEqual(self.notes(output), [b"9"])

    def test_held_storage_and_ordinary_preparation(self):
        output = self.replay(MODEL, START + """
iTake b; iWrite b 2; iApply
iWrite a 3
iScan client -chain first 4 -si 9; iApply
iNote [iGetReadData client -chain first -dec]
iRead b 2; iApply
iNote [iGetReadData b -dec]
iRelease b
""")
        self.assertEqual(self.notes(output), [b"3", b"2"])

    def test_invalid_fields_and_mixed_together_preserve_artifact(self):
        cases = (
            ("iScan client -chain first 3 -si 1; iApply\n", "length"),
            ("iScan client -chain absent 4 -si 1; iApply\n", "chain"),
            ("iScan -ir client -chain first 4 -si 1; iApply\n", "IR scan"),
            ("iScan client -chain first 4 -si 0binvalid; iReset\n", "invalid"),
            ("iWrite a 3; iScan client -chain first 4 -si 1; "
             "iApply -together\n", "mixed raw"),
        )
        for commands, diagnostic in cases:
            with self.subTest(commands=commands):
                self.assert_failure(MODEL, START + commands, diagnostic)

    def test_no_reset_and_disabled_readback_remain_explicit(self):
        self.assert_failure(MODEL, "iPDLLevel 1\n"
                  "iScan client -chain first 4 -si 1; iApply\n", "iReset")
        icl, pdl = self.root / "model.icl", self.root / "model.pdl"
        icl.write_text(MODEL)
        pdl.write_text(START + "iScan client -chain first 4 -si 9; iApply\n"
                       "iNote [iGetReadData client -chain first]\n")
        message = self.run_tool(icl, pdl, ok=False, modeled=False)
        self.assertIn("unavailable", message)

    def test_unsupported_topologies_preserve_artifact(self):
        cases = (
            (MODEL.replace("Source a[0], b[0]", "Source a[0], a[0]"),
             "shared"),
            (MODEL.replace("ScanOutPort so[5:4] {",
                           "ScanOutPort so[5:4] {Enable 1'b1;"), "parallel raw"),
            (MODEL.replace("ScanInPort", "ScanRegister hidden "
                           "{ScanInSource si[3]; ResetValue 0;} ScanInPort", 1),
             "parallel raw"),
            (MODEL.replace("ScanInPort", "Alias idle = a[0] "
                           "{iApplyEndState 0;} ScanInPort", 1), "parallel raw"),
        )
        for source, diagnostic in cases:
            with self.subTest(diagnostic=diagnostic):
                self.assert_failure(source, START +
                          "iScan client -chain first 4 -si 9; iApply\n",
                          diagnostic)


    def test_physical_merge_timing_and_resource_limits_stay_rejected(self):
        self.assert_failure(MODEL, START +
                            "iScan client -chain first 4 -si 1; iApply\n",
                            "unsupported operation in physical PDL: iScan",
                            extra=("--physical_pdl",))
        self.assert_failure(MODEL, START +
                            "iMerge -begin; iScan client -chain first 4 -si 1; "
                            "iApply; iMerge -end\n", "iMerge")
        self.assert_failure(MODEL, START + "iRunLoop 1 -tck\n"
                            "iScan client -chain first 4 -si 1; iApply\n"
                            "iGetReadData client -chain first\n",
                            "modeled readback")
        wide = MODEL.replace("a[3:0]", "a[524288:0]").replace(
            "b[1:0]", "b[524288:0]").replace(
            "ResetValue 4'b0", "ResetValue 0").replace(
            "ResetValue 2'b0", "ResetValue 0")
        self.assert_failure(wide, START +
                            "iScan client -chain first 524289 -si 1; iApply\n",
                            "parallel raw")
        self.assert_failure(MODEL, START +
                            "iTake a; iWrite a 0; iApply\n"
                            "iScan client -chain first 4 -si 0bxxxx; iApply\n",
                            "reserved resource is unknown")


    def test_replaced_physical_length_matches_native_queue(self):
        output = self.replay(MODEL, START + """
iScan client -chain first 3 -si 7
iScan client -chain second 2 -si 2
iScan client -chain first 4 -si 9
iApply
iScan client -chain first 4 -si 6; iApply
iNote [iGetReadData client -chain first -dec]
""")
        self.assertEqual(self.notes(output), [b"9"])

    def test_common_select_bits_and_unused_vector_state(self):
        variants = (
            MODEL.replace("SelectPort sel;", "SelectPort sel[1:0];").replace(
                "} Port sel;", "} Port sel[0];"),
            MODEL.replace("SelectPort sel;", "SelectPort sel; SelectPort other;").replace(
                "} Port sel;", "} Port sel; Port other;"),
        )
        for source in variants:
            output = self.replay(source, START + """
iScan client -chain first 4 -si 9
iScan client -chain second 2 -si 2
iApply
iScan client -chain first 4 -si 6; iApply
iNote [iGetReadData client -chain first -dec]
""")
            self.assertEqual(self.notes(output), [b"9"])


if __name__ == "__main__":
    unittest.main()
