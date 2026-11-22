#!/usr/bin/env python3
"""Raw interface histories use actual scan returns, never expected/input data."""
import unittest

import test_modeled_readback as helpers
import test_modeled_scan_readback as scan
import test_opaque_parallel_scan as opaque
import test_modeled_tap_readback as tap


ROOT = scan.BASE.replace(
    " Alias inverted", " SelectPort sel; CaptureEnPort ce; ShiftEnPort se; "
    "UpdateEnPort ue; TCKPort tck; ResetPort reset; "
    "ScanInterface client {Port si; Port so; Port sel; Port ce; Port se; "
    "Port ue; Port tck; Port reset;}\n Alias inverted")
OPAQUE = opaque.MODEL.replace("Module Top", "Module Chains")


class ModeledRawReadback(unittest.TestCase):
    setUp = helpers.ModeledReadback.setUp
    run_tool = helpers.ModeledReadback.run_tool
    replay = helpers.ModeledReadback.replay
    notes = scan.ModeledScanReadback.notes

    def test_actual_raw_word_mask_counts_and_clear(self):
        output = self.replay(ROOT, """iPDLLevel 1
iReset; iWrite r 9; iApply
iScan client 4 -si 6 -so 0; iApply
iNote [iGetReadData client -dec]; iNote [iGetMiscompares client -bin]
iNote [iGetStatus -clear]; iNote [iGetStatus]
iScan client 4 -si 0; iApply; iNote [iGetReadData client -hex]
""")
        self.assertEqual(self.notes(output), [b"9", b"0b1001", b"2", b"0", b"0x6"])

    def test_raw_and_register_histories_are_independent(self):
        output = self.replay(ROOT, """iPDLLevel 1
iReset; iWrite r 9; iApply; iRead r 0; iApply
iScan client 4 -si 6 -so 0; iApply
iNote [iGetReadData r -dec]; iNote [iGetReadData client -dec]
iRead r 0; iApply
iNote [iGetReadData r -dec]; iNote [iGetReadData client -dec]
iNote [iGetStatus]
""")
        self.assertEqual(self.notes(output), [b"9", b"9", b"6", b"9", b"6"])

    def test_actual_output_inversion(self):
        source = ROOT.replace("Source r[0]", "Source ~r[0]")
        output = self.replay(source, """iPDLLevel 1
iReset; iWrite r 9; iApply
iScan client 4 -si 6 -so 0; iApply
iNote [iGetReadData client -dec]; iNote [iGetMiscompares client -bin]
""")
        self.assertEqual(self.notes(output), [b"6", b"0b0110"])

    def test_opaque_unequal_chains_unknown_counts_formats(self):
        output = self.replay(OPAQUE, """iPDLLevel 1
iReset
iScan client -chain first 4 -si 9 -so 0
iScan client -chain second 7 -si 5
iApply
iNote [iGetReadData client -chain first -hex]
iNote [iGetReadData client -chain second -bin]
iNote [iGetMiscompares client -chain first -bin]
iNote [iGetMiscompares client -chain second -dec]
if {![catch {iGetStatus -clear}]} {error {unknown status returned}}
if {![catch {iGetStatus}]} {error {unknown status cleared}}
""")
        self.assertEqual(self.notes(output), [b"0xX", b"0bxxxxxxx", b"0bxxxx", b"0"])

    def test_opaque_variable_width_latest_queue_reset_preserves_history(self):
        output = self.replay(OPAQUE, """iPDLLevel 1
iReset
iScan client -chain first 4 -si 0
iScan client -chain first 9 -si 0 -stable
iApply
iNote [iGetReadData client -chain first -bin]
iScan client -chain first 3 -si 0; iReset
iNote [iGetReadData client -chain first -bin]
iScan client -chain first 5 -si 0; iApply
iNote [iGetReadData client -chain first -bin]
""")
        self.assertEqual(self.notes(output), [b"0bxxxxxxxxx", b"0bxxxxxxxxx", b"0bxxxxx"])

    def test_stable_implicit_lane_does_not_repeat_comparisons(self):
        output = self.replay(OPAQUE, """iPDLLevel 1
iReset
iScan client -chain first 4 -si 9 -stable; iApply
iScan client -chain second 7 -si 5; iApply
iNote [iGetMiscompares client -chain first -bin]
iNote [iGetStatus]
""")
        self.assertEqual(self.notes(output), [b"0b0000", b"0"])

    def test_named_default_identity(self):
        source = OPAQUE.replace("Chain first", "Chain default")
        output = self.replay(source, """iPDLLevel 1
iReset; iScan client -chain default 4 -si 9; iApply
iNote [iGetReadData client -chain default -bin]
""")
        self.assertEqual(self.notes(output), [b"0bxxxx"])

    def test_getters_need_applied_width_and_valid_chain(self):
        output = self.replay(OPAQUE, """iPDLLevel 1
if {![catch {iGetReadData client -chain first}]} {error {missing width accepted}}
iReset; iScan client -chain first 4; iApply
if {![catch {iGetReadData client}]} {error {missing chain accepted}}
if {![catch {iGetReadData client -chain wrong}]} {error {invalid chain accepted}}
if {![catch {iGetReadData client -chain second}]} {error {implicit history invented}}
iNote [iGetReadData client -chain first]
""")
        self.assertEqual(self.notes(output), [b"0bxxxx"])

    def test_disjoint_children_and_prefix_project_local_parity(self):
        leaf = """Module Leaf {
 ScanInPort si; SelectPort sel; CaptureEnPort ce; ShiftEnPort se;
 UpdateEnPort ue; TCKPort tck; ResetPort reset;
 ScanRegister value[3:0] {ScanInSource si; CaptureSource value; ResetValue 0;}
 ScanOutPort so {Source value[0];}
 ScanInterface local {Port si; Port so; Port sel; Port ce; Port se; Port ue;
                      Port tck; Port reset;}
}
"""
        narrow = leaf.replace("Module Leaf", "Module Narrow").replace(
            "[3:0]", "[2:0]").replace("Source value[0]", "Source ~value[0]")
        source = leaf + narrow + """Module Chains {
 ScanInPort si; SelectPort sel; CaptureEnPort ce; ShiftEnPort se;
 UpdateEnPort ue; TCKPort tck; ResetPort reset;
 Instance a Of Leaf {InputPort si = si; InputPort sel = sel; InputPort ce = ce;
  InputPort se = se; InputPort ue = ue; InputPort tck = tck; InputPort reset = reset;}
 Instance b Of Narrow {InputPort si = a.so; InputPort sel = sel; InputPort ce = ce;
  InputPort se = se; InputPort ue = ue; InputPort tck = tck; InputPort reset = reset;}
 ScanOutPort so {Source b.so;}
 ScanInterface client {Port si; Port so; Port sel; Port ce; Port se; Port ue;
                      Port tck; Port reset;}
}
"""
        output = self.replay(source, """iPDLLevel 1
iReset; iWrite a.value 9; iWrite b.value 5; iApply
iScan a.local 4 -si 6 -so 0; iScan b.local 3 -si 3 -so 0; iApply
iNote [iGetReadData a.local -dec]; iNote [iGetReadData b.local -dec]
iNote [iGetStatus]
if {![catch {iGetReadData client}]} {error {unrequested root history invented}}
iScan client 7 -si 0; iApply; iNote [iGetReadData client -bin]
iPrefix a; iScan local 4 -si 0; iApply
iNote [iGetReadData local -dec]
""")
        self.assertEqual(self.notes(output), [b"9", b"2", b"3", b"0b1001100", b"0"])

    def test_split_files_and_once_only_tcl_values(self):
        icl, first, second = (self.root / name for name in
                              ("source.icl", "first.pdl", "second.pdl"))
        icl.write_text(OPAQUE)
        first.write_text("""iPDLLevel 1
set calls 0
proc input {} {global calls; incr calls; return 9}
iReset; iScan client -chain first 4 -si [input]
""")
        second.write_text("""iApply
iNote [iGetReadData client -chain first -bin]
if {$calls != 1} {error {Tcl input was reevaluated}}
""")
        output = self.run_tool(icl, first, extra=("--pdl", str(second)))
        self.assertEqual(self.notes(output), [b"0bxxxx"])

    def test_help_describes_raw_and_physical_model_comparisons(self):
        import subprocess
        result = subprocess.run([str(tap.cli.BIN), "--help"], capture_output=True,
                                text=True, timeout=30)
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertIn("raw interface/chain history", result.stdout)
        self.assertIn("count observed comparison failures", result.stdout)

    def test_failed_model_execution_preserves_artifacts(self):
        icl, pdl = self.root / "source.icl", self.root / "source.pdl"
        icl.write_text(ROOT)
        for body in (
                "iWrite r 9; iScan client 4; iApply -together",
                "iScan client 3; iApply",
                "iScan client 4 -si 16; iReset"):
            pdl.write_text("iPDLLevel 1; iReset; " + body +
                           "; iNote [iGetReadData client]")
            output = self.root / "result.pdl"
            output.write_bytes(b"retained output")
            self.run_tool(icl, pdl, ok=False)
            self.assertEqual(output.read_bytes(), b"retained output")


class ModeledRawTapReadback(unittest.TestCase):
    setUp = tap.ModeledTapReadback.setUp
    run_tool = tap.ModeledTapReadback.run_tool
    notes = tap.ModeledTapReadback.notes

    def case(self, body, folder="raw_tap_ir", top="RawIrTop",
             entity="RawIrProbe", source=None, physical=False):
        fixture = tap.cli.ROOT / "tests/cases/ours/pattern_generation" / folder
        if source is None:
            source = (fixture / "model.icl").read_text().replace(
                "ScanInSource si; ResetValue", "ScanInSource si; CaptureSource r; ResetValue")
        return tap.ModeledTapReadback.replay(
            self, body, source=source, top=top,
            bsdl=[(entity, fixture / "model.bsdl")], physical=physical)

    def test_ir_dr_latest_width_actual(self):
        for physical in (False,):
            with self.subTest(physical=physical):
                output = self.case("""iReset
iScan -ir jtag 4 -si 3 -so 0; iApply
iNote [iGetReadData jtag -bin]; iNote [iGetMiscompares jtag -bin]
iWrite core.r 9; iApply
iScan jtag 8 -si 6 -so 0; iApply
iNote [iGetReadData jtag -dec]
iScan -ir jtag 4 -si 4 -so 0; iApply
iNote [iGetReadData jtag -bin]; iNote [iGetStatus]
""", physical=physical)
                self.assertEqual(self.notes(output),
                                 [b"0b0001", b"0b0001", b"9", b"0b0001", b"4"])

    def test_literal_x_uses_real_capture_not_binary_proof(self):
        output = self.case("""iReset
iWrite tap.INSTRUCTION 3; iApply; iWrite core.r 9; iApply
iScan jtag 8 -si 0b0x1001x0 -so 0; iApply
iNote [iGetReadData jtag -dec]; iNote [iGetMiscompares jtag -bin]
iNote [iGetStatus]
""")
        self.assertEqual(self.notes(output), [b"9", b"0b00001001", b"2"])

    def test_builtin_idcode_and_bypass_capture(self):
        output = self.case("""iReset
iScan jtag 32 -si 0x89abcdef -so 0; iApply
iNote [iGetReadData jtag -hex]
iScan -ir jtag 4 -si 15; iApply
iScan jtag 1 -si 1 -so 1; iApply
iNote [iGetReadData jtag -bin]; iNote [iGetMiscompares jtag -bin]
iNote [iGetStatus]
""", folder="raw_tap_builtin_dr", top="BuiltinDrTop", entity="BuiltinDrProbe")
        self.assertEqual(self.notes(output), [b"0x10000003", b"0b0", b"0b1", b"4"])

    def test_multiple_register_suffix_parity(self):
        fixture = tap.cli.ROOT / "tests/cases/ours/pattern_generation/raw_tap_multiple_dr"
        source = (fixture / "model.icl").read_text().replace(
            "ScanInSource a[0];", "ScanInSource ~a[0];").replace(
            "Source mux;", "Source ~mux;")
        output = self.case("""iReset
iWrite tap.INSTRUCTION 3; iApply
iWrite core.a 5; iWrite core.b 22; iApply
iScan jtag 8 -si 0 -so 0 -stable; iApply
iNote [iGetReadData jtag -bin]; iNote [iGetStatus]
""", folder="raw_tap_multiple_dr", top="MultipleDrTop",
             entity="MultipleDrProbe", source=source)
        # a contributes MSBs with the two inversions cancelling; b is inverted.
        self.assertEqual(self.notes(output), [b"0b10101001", b"4"])

    def test_physical_raw_dialect_stays_rejected(self):
        fixture = tap.cli.ROOT / "tests/cases/ours/pattern_generation/raw_tap_ir"
        icl, pdl = self.root / "source.icl", self.root / "source.pdl"
        icl.write_text((fixture / "model.icl").read_text())
        pdl.write_text("iPDLLevel 1; iReset; iScan -ir jtag 4 -si 3; iApply")
        output = self.root / "result.pdl"
        output.write_bytes(b"retained output")
        message = self.run_tool(
            icl, pdl, top="RawIrTop",
            bsdl=[("RawIrProbe", fixture / "model.bsdl")],
            physical=True, ok=False)
        self.assertIn("unsupported operation in physical PDL: iScan", message)
        self.assertEqual(output.read_bytes(), b"retained output")

    def test_implicit_unnamed_and_default_alias(self):
        fixture = tap.cli.ROOT / "tests/cases/ours/pattern_generation/raw_tap_ir"
        source = (fixture / "model.icl").read_text().replace(
            "  ScanInterface jtag {Port TDI; Port TDO; Port TMS; Port TCK; Port TRST;}\n", "")
        output = self.case("""iReset
iScan -ir unnamed -chain default 4 -si 3 -so 0; iApply
iNote [iGetReadData unnamed -chain default -dec]
""", source=source)
        self.assertEqual(self.notes(output), [b"1"])


if __name__ == "__main__":
    unittest.main()
