#!/usr/bin/env python3
"""Bound TAP readback uses actual IR/DR return bits and exact replay mode."""
import subprocess
import unittest

import test_simultaneous_scan_chains as cli
import test_modeled_readback as history
import test_multiple_tap_scheduling as multiple

FIXTURE = cli.ROOT / "tests/cases/ours/pattern_generation/access_link"


class ModeledTapReadback(unittest.TestCase):
    setUp = cli.SimultaneousChains.setUp

    def run_tool(self, icl, pdl, output="result.pdl", top="AccessTop",
                 bsdl=None, modeled=True, physical=False, ok=True, extra=()):
        args = [str(cli.BIN), "--icl", str(icl), "--pdl", str(pdl),
                "--top_module", top, "--output", str(self.root / output), "--debug"]
        for entity, path in (bsdl if bsdl is not None else
                             [("AccessProbe", FIXTURE / "simple.bsdl")]):
            args += ["--bsdl", entity, str(path)]
        if modeled:
            args.append("--model_readback")
        if physical:
            args.append("--physical_pdl")
        result = subprocess.run([*args, *extra], capture_output=True, text=True, timeout=60)
        self.assertEqual(result.returncode == 0, ok, result.stdout + result.stderr)
        return (self.root / output).read_bytes() if ok else result.stdout + result.stderr

    def source(self):
        return (FIXTURE / "simple.icl").read_text().replace(
            "ScanRegister r[7:0] { ScanInSource si; }",
            "ScanRegister r[7:0] {ScanInSource si; CaptureSource r; ResetValue 0;} "
            "Alias inverted[7:0] = ~r;")

    def replay(self, body, source=None, top="AccessTop", bsdl=None, physical=False):
        icl, pdl = self.root / "source.icl", self.root / "source.pdl"
        icl.write_text(source if source is not None else self.source())
        pdl.write_text("iPDLLevel 1\n" + body)
        output = self.run_tool(icl, pdl, top=top, bsdl=bsdl, physical=physical)
        self.assertEqual(output, self.run_tool(
            self.root / ("debug/" + top + ".icl"), pdl, "flat/result.pdl",
            top=top, bsdl=[], physical=physical))
        self.assertEqual(output, self.run_tool(
            icl, self.root / "result.pdl", "replay/result.pdl",
            top=top, bsdl=bsdl, physical=physical))
        return output

    def notes(self, output):
        return [line[6:] for line in output.splitlines()
                if line.startswith(b"iNote ") and b"modeled " not in line]

    def test_ir_capture_is_not_the_active_instruction(self):
        output = self.replay("""iReset
iRead tap.INSTRUCTION 0; iApply
iNote [iGetReadData tap.INSTRUCTION -bin]
iNote [iGetMiscompares tap.INSTRUCTION -bin]; iNote [iGetStatus]
""")
        self.assertEqual(self.notes(output), [b"0b0001", b"0b0001", b"1"])

    def test_ir_unknown_capture_and_masked_comparison(self):
        bsdl = self.root / "unknown.bsdl"
        bsdl.write_text((FIXTURE / "simple.bsdl").read_text().replace(
            'entity is "0001";', 'entity is "xx01";'))
        output = self.replay("""iReset
iRead tap.INSTRUCTION 0bxx00; iApply
iNote [iGetReadData tap.INSTRUCTION -bin]
iNote [iGetMiscompares tap.INSTRUCTION -bin]; iNote [iGetStatus]
""", bsdl=[("AccessProbe", bsdl)])
        self.assertEqual(self.notes(output), [b"0bxx01", b"0b0001", b"1"])

    def test_dr_miscompare_alias_and_clear(self):
        output = self.replay("""iReset; iWrite core.r 9; iApply
iRead core.r 0; iApply
iNote [iGetReadData core.r -dec]; iNote [iGetReadData core.inverted -dec]
iNote [iGetMiscompares core.r -bin]
iNote [iGetStatus -clear]; iNote [iGetStatus]
""")
        self.assertEqual(self.notes(output), [b"9", b"246", b"0b00001001", b"2", b"0"])

    def test_read_before_write_and_implicit_ir_setup(self):
        output = self.replay("""iReset; iWrite core.r 9; iApply
iRead core.r 0; iWrite core.r 6; iApply
iNote [iGetReadData core.r -dec]; iNote [iGetStatus]
iRead core.r 6; iApply
iNote [iGetReadData core.r -dec]; iNote [iGetStatus]
""")
        self.assertEqual(self.notes(output), [b"9", b"2", b"6", b"2"])
        self.assertIn(b"iRead tap.INSTRUCTION 0b0001", output)

    def test_sparse_ascending_aliases_and_manual_planes(self):
        source = self.source().replace("[7:0]", "[0:7]").replace("r[0]", "r[7]")
        output = self.replay("""iReset; iWrite core.r 9; iApply
iRead core.inverted[0] 0; iRead core.r[7] 0; iApply
iState core.r[0] 1 -LastReadValue
iNote [iGetReadData core.r -bin]
iNote [iGetMiscompares core.r -bin]; iNote [iGetStatus]
""", source)
        self.assertEqual(self.notes(output), [b"0b1xxxxxx1", b"0b1xxxxxx1", b"2"])

    def test_unknown_dr_capture_and_dont_care(self):
        source = self.source().replace("CaptureSource r;", "CaptureSource 8'b10x1xxxx;")
        output = self.replay("""iReset; iRead core.r 0bxxxxxxxx; iApply
iNote [iGetReadData core.r -hex]; iNote [iGetMiscompares core.r -bin]
iNote [iGetStatus]
iRead core.r 0; iApply
if {![catch {iGetStatus -clear}]} {error {unknown status cleared}}
iNote [iGetReadData core.r -bin]
""", source)
        self.assertEqual(self.notes(output), [b"0xxX", b"0b00000000", b"0", b"0b10x1xxxx"])

    def test_captured_data_register_projects_logical_bits(self):
        fixture = cli.ROOT / "tests/cases/ours/pattern_generation/scan_data_registers/vendor.icl"
        source = fixture.read_text()
        output = self.replay("""iReset; iWrite core.value 9; iApply
iRead core.value 0; iWrite core.value 6; iApply
iNote [iGetReadData core.value -dec]; iNote [iGetStatus]
iRead core.value 6; iApply; iNote [iGetReadData core.value -dec]
""", source, top="ScanData")
        self.assertEqual(self.notes(output), [b"9", b"2", b"6"])

    def test_multiple_unequal_tap_lanes(self):
        for shared_tms in (False, True):
            source, models = multiple.network(shared_tms=shared_tms)
            source = source.replace("ScanInSource si; ResetValue 0;",
                                    "ScanInSource si; CaptureSource value; ResetValue 0;")
            bsdl = self.root / "multiple.bsdl"
            bsdl.write_text(models)
            with self.subTest(shared_tms=shared_tms):
                output = self.replay("""iReset
iWrite c0.value 9; iWrite c1.value 66; iApply -together
iRead c0.value 0; iRead c1.value 0; iApply -together
iNote [iGetReadData c0.value -dec]; iNote [iGetReadData c1.value -dec]
iNote [iGetStatus]
""", source, top="MultiTapTop", bsdl=[("Probe0", bsdl), ("Probe1", bsdl)])
                self.assertEqual(self.notes(output), [b"9", b"66", b"4"])

    def test_physical_exact_csu_accepts_counted_mismatch(self):
        output = self.replay("""iReset
iWrite tap.INSTRUCTION 3; iApply
iWrite core.r 9; iApply
iRead core.r 0; iWrite core.r 6; iApply
iNote [iGetReadData core.r -dec]; iNote [iGetStatus]
iRead core.r 6; iApply; iNote [iGetReadData core.r -dec]
""", physical=True)
        self.assertEqual(self.notes(output), [b"9", b"2", b"6"])
        self.assertEqual(output.count(b"iApply"), 4)

    def test_default_physical_diagnostics_preserve_output(self):
        icl, pdl = self.root / "source.icl", self.root / "source.pdl"
        icl.write_text(self.source())
        pdl.write_text("iReset; iWrite tap.INSTRUCTION 3; iApply; "
                       "iWrite core.r 9; iApply; iRead core.r 0; iApply")
        output = self.root / "result.pdl"
        output.write_bytes(b"preserve")
        error = self.run_tool(icl, pdl, modeled=False, physical=True, ok=False)
        self.assertIn("TAP capture comparison differs", error)
        self.assertEqual(output.read_bytes(), b"preserve")

    def test_no_query_and_default_matching_controls(self):
        for physical in (False, True):
            icl, pdl = self.root / "source.icl", self.root / "source.pdl"
            icl.write_text(self.source())
            pdl.write_text("iReset; iWrite tap.INSTRUCTION 3; iApply; "
                           "iWrite core.r 9; iApply; iRead core.r 9; iApply")
            model = self.run_tool(icl, pdl, physical=physical)
            default = self.run_tool(icl, pdl, "default.pdl", physical=physical, modeled=False)
            self.assertEqual([line for line in model.splitlines() if history.LABEL not in line],
                             default.splitlines())

    def test_reservation_and_reset_cancel_preserve_history(self):
        output = self.replay("""iReset; iWrite core.r 9; iApply
iTake core.r; iRead core.r 0; iApply; iNote [iGetReadData core.r -dec]
iRelease core.r; iWrite core.r 6; iRead core.r 0; iReset
iNote [iGetReadData core.r -dec]
iRead core.r 0; iApply; iNote [iGetReadData core.r -dec]; iNote [iGetStatus]
""")
        self.assertEqual(self.notes(output), [b"9", b"9", b"0", b"2"])
        self.assertNotIn(b"iWrite core.r 0b00000110", output)

    def test_prefix_ordered_files_and_once_only_value(self):
        icl, first, second = (self.root / name for name in
                              ("source.icl", "first.pdl", "second.pdl"))
        icl.write_text(self.source())
        first.write_text("""iPDLLevel 1
set calls 0
proc value {} {incr ::calls; return 9}
iReset; iWrite core.r [value]; iApply; iPrefix core; iRead r 0
""")
        second.write_text("""iApply; iNote [iGetReadData r -dec]; iNote [iGetStatus]
if {$calls != 1} {error {value evaluated twice}}
""")
        output = self.run_tool(icl, first, extra=("--pdl", str(second)))
        self.assertEqual(self.notes(output), [b"9", b"2"])
        self.assertEqual(output, self.run_tool(
            self.root / "debug/AccessTop.icl", first, "flat/result.pdl", bsdl=[],
            extra=("--pdl", str(second))))
        self.assertEqual(output, self.run_tool(
            icl, self.root / "result.pdl", "replay/result.pdl"))

    def test_local_reset_preserves_procedure_bytes(self):
        body = """iProc observe {} {
 iRead core.r 0; iApply
 iNote [iGetReadData core.r -dec]
 iNote [iGetStatus]
}
"""
        icl, pdl = self.root / "source.icl", self.root / "source.pdl"
        icl.write_text(self.source())
        pdl.write_text("iPDLLevel 1\niProcsForModule AccessTop\n" + body +
                       "iReset; iWrite core.r 9; iApply; iCall observe\n")
        output = self.run_tool(
            icl, pdl, extra=("--local_reset", str(self.root / "reset.pdl")))
        self.assertEqual(self.notes(output), [b"9", b"2"])
        self.assertIn(body, (self.root / "reset.pdl").read_text())

    def test_synthetic_data_targets_retain_existing_guard(self):
        fixture = cli.ROOT / "tests/cases/ours/pattern_generation/raw_tap_builtin_dr"
        icl, pdl = fixture / "model.icl", self.root / "source.pdl"
        for target in ("tap.DEVICE_ID", "tap.BYPASS"):
            pdl.write_text("iReset; iRead " + target + " 0; iApply; iGetStatus")
            output = self.root / "result.pdl"
            output.write_bytes(b"preserve")
            error = self.run_tool(icl, pdl, top="BuiltinDrTop",
                                  bsdl=[("BuiltinDrProbe", fixture / "model.bsdl")],
                                  ok=False)
            self.assertIn("synthetic TAP data registers", error)
            self.assertEqual(output.read_bytes(), b"preserve")

    def test_unsupported_history_preserves_existing_output(self):
        icl, pdl = self.root / "source.icl", self.root / "source.pdl"
        icl.write_text(self.source())
        pdl.write_text("iReset; iWrite core.r 9; iApply; iRead core.r 0; iApply; "
                       "iRunLoop 1 -tck; iGetStatus")
        output = self.root / "result.pdl"
        output.write_bytes(b"preserve")
        self.assertIn("modeled readback", self.run_tool(icl, pdl, ok=False))
        self.assertEqual(output.read_bytes(), b"preserve")


if __name__ == "__main__":
    unittest.main()
