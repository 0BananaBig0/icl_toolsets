#!/usr/bin/env python3
"""Actual serial-return history, independent of expected values and updates."""
import unittest

import test_simultaneous_scan_chains as cli
import test_modeled_readback as helpers

BASE = """Module Chains {
 ScanInPort si;
 ScanRegister r[3:0] {ScanInSource si; CaptureSource r; ResetValue 0;}
 ScanOutPort so {Source r[0];}
 Alias inverted[3:0] = ~r;
}"""


class ModeledScanReadback(unittest.TestCase):
    setUp = helpers.ModeledReadback.setUp
    run_tool = helpers.ModeledReadback.run_tool
    replay = helpers.ModeledReadback.replay

    def notes(self, output):
        return [line[6:] for line in output.splitlines()
                if line.startswith(b"iNote ") and b"modeled " not in line]

    def test_actual_bits_alias_and_count_clear(self):
        output = self.replay(BASE, """iPDLLevel 1
iReset; iWrite r 9; iApply; iRead r 0; iApply
iNote [iGetReadData r -dec]; iNote [iGetReadData inverted -dec]
iNote [iGetMiscompares inverted -bin]
iNote [iGetStatus -clear]; iNote [iGetStatus]
""")
        self.assertEqual(self.notes(output), [b"9", b"6", b"0b1001", b"2", b"0"])

    def test_read_before_write_and_later_history(self):
        output = self.replay(BASE, """iPDLLevel 1
iReset; iWrite r 9; iApply
iRead r 0; iWrite r 6; iApply; iNote [iGetReadData r -dec]
iRead r 0; iApply; iNote [iGetReadData r -dec]; iNote [iGetStatus]
""")
        self.assertEqual(self.notes(output), [b"9", b"6", b"4"])

    def test_path_inversions_and_ascending_declarations(self):
        for ascending in (False, True):
            for inverted in (False, True):
                source = BASE
                if ascending:
                    source = source.replace("[3:0]", "[0:3]").replace("Source r[0]", "Source r[3]")
                if inverted:
                    source = source.replace("ScanInSource si", "ScanInSource ~si").replace("Source r[", "Source ~r[")
                with self.subTest(ascending=ascending, inverted=inverted):
                    output = self.replay(source, """iPDLLevel 1
iReset; iWrite r 10; iApply; iRead r 0; iApply
iNote [iGetReadData r -dec]; iNote [iGetMiscompares r -bin]
""")
                    self.assertEqual(self.notes(output), [b"10", b"0b1010"])

    def test_two_register_order_and_sparse_latest_expected(self):
        source = BASE.replace(" ScanOutPort", """
 ScanRegister next[2:0] {ScanInSource ~r[0]; CaptureSource next; ResetValue 0;}
 ScanOutPort""").replace("Source r[0];", "Source ~next[0];")
        output = self.replay(source, """iPDLLevel 1
iReset; iWrite r 9; iWrite next 3; iApply
iRead r[3:2] 0; iRead r[3] 0bx; iRead next 0; iApply
iNote [iGetReadData r -bin]; iNote [iGetMiscompares r -bin]
iNote [iGetReadData next -dec]; iNote [iGetStatus]
""")
        self.assertEqual(self.notes(output), [b"0b10xx", b"0b00xx", b"3", b"2"])

    def test_unknown_capture_and_dont_care(self):
        source = BASE.replace("CaptureSource r", "CaptureSource data").replace(
            " ScanInPort si;", " DataInPort data[3:0]; ScanInPort si;")
        output = self.replay(source, """iPDLLevel 1
iReset; iRead r 0bxxxx; iApply
iNote [iGetReadData r -bin]; iNote [iGetMiscompares r -bin]; iNote [iGetStatus]
iRead r 0; iApply
if {![catch {iGetStatus -clear}]} {error {unknown count cleared}}
iNote [iGetReadData r -bin]
""")
        self.assertEqual(self.notes(output), [b"0bxxxx", b"0b0000", b"0", b"0bxxxx"])

    def test_unequal_client_lanes(self):
        output = self.replay(cli.NETWORK, """iPDLLevel 1
iReset; iWrite a 9; iWrite b 2; iApply -together
iRead a 0; iRead b 0; iWrite a 6; iWrite b 1; iApply -together
iNote [iGetReadData a -dec]; iNote [iGetReadData b -dec]; iNote [iGetStatus]
""")
        self.assertEqual(self.notes(output), [b"9", b"2", b"3"])

    def test_scan_accessed_data_register_projection(self):
        source = (cli.ROOT / "tests/cases/ours/pattern_generation/scan_data_registers/selectable.icl").read_text()
        source = source.replace("Module ScanData", "Module Chains").replace(
            "CaptureSource value", "CaptureSource ~value").replace(
            "  ScanOutPort", "  Alias inverted[3:0] = ~value;\n  ScanOutPort")
        output = self.replay(source, """iPDLLevel 1
iReset; iWrite value 9; iApply
iRead value 0; iWrite value 6; iApply
iNote [iGetReadData value -dec]; iNote [iGetMiscompares value -bin]
iRead inverted 0; iApply
iNote [iGetReadData inverted -dec]; iNote [iGetStatus]
""")
        self.assertEqual(self.notes(output), [b"9", b"0b1001", b"9", b"4"])

    def test_manual_planes_and_lazy_backlog(self):
        output = self.replay(BASE, """iPDLLevel 1
iReset; iWrite r 9; iApply; iRead r 0; iApply
iState r 15 -LastReadValue
iNote [iGetMiscompares r -bin]; iNote [iGetReadData r -dec]
iRead r 0; iApply; iState r 0 -LastMiscompareValue
iNote [iGetReadData r -dec]; iNote [iGetMiscompares r -bin]; iNote [iGetStatus]
""")
        self.assertEqual(self.notes(output), [b"0b1001", b"15", b"9", b"0b0000", b"4"])

    def test_reset_held_downstream_still_shifts_returned_bits(self):
        source = """Module Held {
 ScanInPort si; ResetPort rst;
 ScanRegister held[1:0] {ScanInSource si; CaptureSource held; ResetValue 0;}
 ScanOutPort so {Source held[0];}
}
Module Chains {
 ScanInPort si;
 ScanRegister r[3:0] {ScanInSource si; CaptureSource r; ResetValue 0;}
 Instance tail Of Held {InputPort si = r[0]; InputPort rst = 1'b1;}
 ScanOutPort so {Source tail.so;}
}"""
        output = self.replay(source, """iPDLLevel 1
iReset; iState r 9; iRead r 0; iApply
iNote [iGetReadData r -dec]; iNote [iGetStatus]
""")
        self.assertEqual(self.notes(output), [b"9", b"2"])

    def test_bound_tap_is_lazy_and_rejection_preserves_output(self):
        fixture = cli.ROOT / "tests/cases/ours/pattern_generation/access_link"
        source = (fixture / "simple.icl").read_text().replace("Module AccessTop", "Module Chains")
        icl, pdl = self.root / "source.icl", self.root / "source.pdl"
        icl.write_text(source)
        extra = ("--bsdl", "AccessProbe", str(fixture / "simple.bsdl"))
        pdl.write_text("iPDLLevel 1; iReset; iWrite core.r 9; iApply")
        modeled = self.run_tool(icl, pdl, extra=extra)
        default = self.run_tool(icl, pdl, "default.pdl", modeled=False, extra=extra)
        self.assertEqual([line for line in modeled.splitlines() if helpers.LABEL not in line],
                         default.splitlines())
        pdl.write_text("iPDLLevel 1; iReset; iRead core.r 0; iApply; iGetStatus")
        output = self.root / "result.pdl"
        output.write_bytes(b"preserve")
        self.assertIn("bound TAPs", self.run_tool(icl, pdl, ok=False, extra=extra))
        self.assertEqual(output.read_bytes(), b"preserve")

    def test_both_legacy_notes_migrate_once(self):
        for note in ("modeled combinational PIO readback; no live device",
                     "modeled PIO/DataRegister readback; no live device"):
            output = self.replay(BASE, "iPDLLevel 1; iNote {" + note + "}; "
                                 "iReset; iRead r 0; iApply; iNote [iGetStatus]")
            self.assertEqual(output.count(helpers.LABEL), 1)
            self.assertNotIn(note.encode(), output)

    def test_child_setup_frames_and_first_observation(self):
        child = """Module Child {
 ScanInPort si; ScanOutPort so {Source r[0];} SelectPort sel;
 ScanRegister r[3:0] {ScanInSource si; CaptureSource r; ResetValue 0;}
}
"""
        source = cli.NETWORK.replace(
            "ScanRegister a[3:0] { ScanInSource si[3]; CaptureSource a; ResetValue 4'b0; }",
            "Instance child Of Child {InputPort si = si[3]; InputPort sel = b[0];}")
        source = source.replace("Source a[0], b[0]", "Source path, b[0]").replace(
            "  SelectPort sel;",
            "  ScanMux path SelectedBy b[0] {1'b0: si[3]; 1'b1: child.so;}\n  SelectPort sel;")
        output = self.replay(child + source, """iPDLLevel 1
iReset; iWrite child.r 9; iApply
iState b 0; iRead child.r 0; iRead b 0; iApply
iNote [iGetReadData child.r -dec]; iNote [iGetReadData b -dec]; iNote [iGetStatus]
""")
        self.assertEqual(self.notes(output), [b"9", b"0", b"2"])

    def test_native_client_readback_keeps_fast_replay(self):
        fixture = cli.ROOT / "tests/cases/ours/pattern_generation/native_ordinary_scans/model.icl"
        source = fixture.read_text().replace("Module NativeOrdinaryTop", "Module Chains")
        for capture, expected, count in (("value", "9", "2"),
                                         ("8'b00001010", "10", "2"),
                                         ("8'bxxxxxxxx", "X", None)):
            model = source.replace("CaptureSource value", "CaptureSource " + capture)
            status = ("iNote [iGetStatus]" if count is not None else
                      "if {![catch {iGetStatus}]} {error {unknown count lost}}")
            with self.subTest(capture=capture):
                output = self.replay(model, """iPDLLevel 1
iReset; iWrite open 1; iApply; iWrite child.value 9; iApply
iRead child.value 0; iApply
iNote [iGetReadData child.value -dec]
""" + status)
                self.assertEqual(self.notes(output),
                                 [expected.encode()] + ([count.encode()] if count else []))

    def test_wide_native_observation_retains_batched_shift(self):
        fixture = cli.ROOT / "tests/cases/ours/pattern_generation/native_ordinary_scans/model.icl"
        source = fixture.read_text().replace("Module NativeOrdinaryTop", "Module Chains")
        source = source.replace("[7:0]", "[4095:0]").replace("8'b0", "4096'b0")
        output = self.replay(source, """iPDLLevel 1
iReset; iWrite open 1; iApply; iWrite child.value 0x""" + "f" * 1024 + """; iApply
iRead child.value 0; iApply
iNote [iGetReadData child.value -bin]; iNote [iGetStatus]
""")
        self.assertEqual(self.notes(output), [b"0b" + b"1" * 4096, b"4096"])

    def test_regular_read_trial_projects_constant_capture(self):
        fixture = cli.ROOT / "tests/cases/ours/pattern_generation/control_mux_operations/capture.icl"
        source = fixture.read_text().replace("Module ClientTop", "Module Chains")
        output = self.replay(source, """iPDLLevel 1
iReset; iState cap_enable 1; iRead child.value 0; iApply
iNote [iGetReadData child.value -dec]; iNote [iGetStatus]
""")
        self.assertEqual(self.notes(output), [b"10", b"2"])

    def test_unsupported_operation_preserves_output(self):
        icl, pdl = self.root / "source.icl", self.root / "source.pdl"
        icl.write_text(BASE)
        pdl.write_text("iPDLLevel 1; iReset; iRead r 0; iApply; iRunLoop 1 -tck; iGetStatus")
        output = self.root / "result.pdl"
        output.write_bytes(b"preserve")
        self.assertIn("modeled readback", self.run_tool(icl, pdl, ok=False))
        self.assertEqual(output.read_bytes(), b"preserve")


if __name__ == "__main__":
    unittest.main()
