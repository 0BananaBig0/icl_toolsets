#!/usr/bin/env python3
"""Actual DataRegister readback through PIO and lazy manual history order."""
import unittest
import subprocess

import test_simultaneous_scan_chains as cli

import test_modeled_readback as helpers
import test_data_register_callback_model as callbacks

BASE = """Module Chains {
 DataInPort data[3:0]; DataInPort enable;
 DataRegister value[3:0] {WriteDataSource data; WriteEnSource enable; ResetValue 0;}
 DataOutPort q[3:0] {Source value;}
 Alias inverted[3:0] = ~value;
}"""


class ModeledDataReadback(unittest.TestCase):
    setUp = helpers.ModeledReadback.setUp
    run_tool = helpers.ModeledReadback.run_tool
    replay = helpers.ModeledReadback.replay

    def test_actual_storage_outputs_and_aliases(self):
        for target, actual, mask in (("value", "9", "1001"),
                                     ("q", "9", "1001"),
                                     ("inverted", "6", "0110")):
            with self.subTest(target=target):
                output = self.replay(
                    BASE, "iPDLLevel 1; iState enable 0; iState value 9; "
                    "iRead " + target + " 0; iApply; "
                    "iNote [iGetReadData " + target + " -dec]; "
                    "iNote [iGetMiscompares " + target + " -bin]; "
                    "iNote [iGetStatus]")
                self.assertIn(("iNote " + actual + "\n").encode(), output)
                self.assertIn(("iNote 0b" + mask + "\n").encode(), output)
                self.assertIn(b"iNote 2\n", output)

    def test_read_before_write_observes_old_storage(self):
        output = self.replay(BASE, """iPDLLevel 1
iReset; iState enable 0; iState value 9
iRead value 0; iWrite value 6; iApply
iNote [iGetReadData value -hex]; iNote [iGetStatus]
iRead q 6; iApply; iNote [iGetReadData q -hex]; iNote [iGetStatus]
""")
        self.assertIn(b"iNote 0x9", output)
        self.assertIn(b"iNote 0x6", output)
        self.assertEqual(output.count(b"iNote 2\n"), 2)

    def test_undelivered_read_plane_keeps_later_manual_override(self):
        output = self.replay(BASE, """iPDLLevel 1
iState enable 0; iState value 9; iRead value 0; iApply
iState value 15 -LastReadValue
iNote [iGetMiscompares value -bin]
iNote [iGetReadData value -dec]; iNote [iGetStatus]
iRead q 9; iApply; iNote [iGetReadData q -dec]
iNote [iGetReadData value -dec]
""")
        self.assertIn(b"iNote 0b1001", output)
        self.assertEqual(output.count(b"iNote 15\n"), 2)
        self.assertIn(b"iNote 9\n", output)
        self.assertIn(b"iNote 2\n", output)

    def test_undelivered_mismatch_plane_and_later_observation(self):
        output = self.replay(BASE, """iPDLLevel 1
iState enable 0; iState value 9; iRead value 0; iApply
iState value 0 -LastMiscompareValue
iNote [iGetReadData value -dec]
iNote [iGetMiscompares value -bin]; iNote [iGetStatus]
iRead value 0; iApply; iNote [iGetMiscompares value -bin]
iNote [iGetStatus]
""")
        self.assertIn(b"iNote 9\n", output)
        self.assertIn(b"iNote 0b0000", output)
        self.assertIn(b"iNote 0b1001", output)
        self.assertIn(b"iNote 2\n", output)
        self.assertIn(b"iNote 4\n", output)

    def test_partial_inverted_manual_planes_across_lazy_queries(self):
        output = self.replay(BASE, """iPDLLevel 1
iState enable 0; iState value 9; iRead value 0; iApply
iState inverted[1:0] 0 -LastReadValue
iNote [iGetReadData inverted[1:0] -dec]
iNote [iGetMiscompares value -bin]
iNote [iGetReadData value -dec]; iNote [iGetReadData inverted -dec]
iState inverted[3:2] 3 -LastMiscompareValue
iNote [iGetMiscompares value -dec]; iNote [iGetStatus]
iRead q 9; iApply; iNote [iGetReadData q -dec]
""")
        notes = [line[6:] for line in output.splitlines()
                 if line.startswith(b"iNote ") and helpers.LABEL not in line]
        self.assertEqual(notes, [b"0", b"0b1001", b"11", b"4", b"13", b"2", b"9"])

    def test_unknown_storage_and_manual_history_do_not_clear_unknown_count(self):
        output = self.replay(BASE, """iPDLLevel 1
iState enable 0; iRead value 0; iApply
iNote [iGetReadData value -hex]
if {![catch {iGetStatus -clear}]} {error {unknown count returned}}
iState value 15 -LastReadValue
iNote [iGetReadData value -dec]
iState value 0; iRead value 0; iApply
if {![catch {iGetStatus}]} {error {unknown count erased}}
iNote [iGetReadData value -dec]
""")
        for note in (b"iNote 0xX", b"iNote 15\n", b"iNote 0\n"):
            self.assertIn(note, output)

    def test_delivered_history_and_backlog_manual_order(self):
        output = self.replay(BASE, """iPDLLevel 1
iState enable 0; iState value 10; iRead value 0; iApply
iNote [iGetReadData value -dec]
iState inverted 0 -LastReadValue; iState value 0 -LastMiscompareValue
iNote [iGetReadData value -dec]; iNote [iGetMiscompares value -dec]
iNote [iGetStatus]
iRead value 0; iApply
iState value 0 -LastReadValue; iState value 0 -LastMiscompareValue
iRead value 0; iApply
iNote [iGetReadData value -dec]; iNote [iGetMiscompares value -dec]
iNote [iGetStatus]
""")
        self.assertEqual(output.count(b"iNote 10\n"), 3)
        for value in (15, 0, 2, 6):
            self.assertIn(("iNote " + str(value) + "\n").encode(), output)

    def test_controlling_state_rewrites_queued_storage_access(self):
        for flag in ("", " -LastWrittenValue"):
            with self.subTest(flag=flag):
                output = self.replay(BASE, """iPDLLevel 1
iReset; iState enable 0; iState value 0
iWrite value 0; iRead value 0; iState value 5""" + flag + """
iApply; iNote [iGetReadData value -dec]; iNote [iGetStatus]
""")
                self.assertIn(b"iNote 5\n", output)
                self.assertIn(b"iNote 0\n", output)

    def test_reset_cancels_queue_and_observes_reset_storage(self):
        output = self.replay(BASE, """iPDLLevel 1
iReset; iState enable 0; iState value 9
iRead value 0; iApply; iNote [iGetStatus]
iWrite value 6; iRead value 0; iReset
iNote [iGetReadData value -dec]
iRead value 0; iApply
iNote [iGetReadData value -dec]; iNote [iGetStatus -clear]
iNote [iGetStatus]
""")
        self.assertNotIn(b"iWrite data 0b0110", output)
        self.assertEqual(output.count(b"iNote 2\n"), 2)
        self.assertIn(b"iNote 9\n", output)
        self.assertEqual(output.count(b"iNote 0\n"), 2)

    def test_reservation_then_release_and_actual_update(self):
        output = self.replay(BASE, """iPDLLevel 1
iReset; iState enable 0; iState value 9; iTake value
iRead value 0; iApply; iNote [iGetReadData value -dec]
iRelease value; iWrite value 6; iApply
iRead value 6; iApply; iNote [iGetReadData value -dec]
iNote [iGetStatus]
""")
        for value in (9, 6, 2):
            self.assertIn(("iNote " + str(value) + "\n").encode(), output)

    def test_dag_and_feedback_observe_before_real_events(self):
        for load, declarations, setup in (
                ("2'b11", "", ""),
                ("value[0],1'b1", "", ""),
                ("first", "DataRegister first[1:0] {WriteDataSource 2'b11;}",
                 "iState first 0; ")):
            source = """Module Chains {
 DataInPort trigger;
 """ + declarations + """
 DataRegister value[1:0] {WriteDataSource """ + load + """;}
 DataOutPort q[1:0] {Source value;}
}"""
            with self.subTest(load=load):
                output = self.replay(
                    source, "iPDLLevel 1; iReset; iState trigger 0; "
                    "iState value 0; " + setup +
                    "iRead value 0; iWrite value 3; iApply; "
                    "iNote [iGetReadData value -dec]; "
                    "iRead q 3; iApply; iNote [iGetReadData q -dec]; "
                    "iNote [iGetStatus]")
                self.assertIn(b"iWrite trigger", output)
                self.assertEqual(output.count(b"iNote 0\n"), 2)
                self.assertIn(b"iNote 3\n", output)

    def test_prefix_files_and_once_only_arguments(self):
        source = BASE.replace("Module Chains", "Module Child") + """Module Chains {
 DataInPort data[3:0]; DataInPort enable;
 Instance c Of Child {InputPort data = data; InputPort enable = enable;}
 DataOutPort q[3:0] {Source c.q;}
}"""
        icl, first, second = (self.root / n for n in
                              ("source.icl", "first.pdl", "second.pdl"))
        icl.write_text(source)
        first.write_text("""iPDLLevel 1
set calls 0
proc value {} {incr ::calls; return 9}
iState enable 0; iState c.value [value]; iPrefix c; iRead value 0
""")
        second.write_text("""iApply
iNote [iGetReadData value -dec]; iNote [iGetStatus]
iState value 15 -LastReadValue; iNote [iGetReadData value -dec]
if {$calls != 1} {error {argument evaluated twice}}
""")
        output = self.run_tool(icl, first, extra=("--pdl", str(second)))
        for value in (9, 2, 15):
            self.assertIn(("iNote " + str(value) + "\n").encode(), output)
        self.assertEqual(output, self.run_tool(
            self.root / "debug/Chains.icl", first, "flat/result.pdl",
            extra=("--pdl", str(second))))
        self.assertEqual(output, self.run_tool(
            icl, self.root / "result.pdl", "replayed.pdl"))

    def test_local_reset_keeps_procedure_bytes(self):
        body = """iProc observe {} {
 iRead value 0; iApply
 iNote [iGetReadData value -dec]
 iState value 15 -LastReadValue
 iNote [iGetMiscompares value -bin]
 iNote [iGetReadData value -dec]
}
"""
        icl, pdl = self.root / "source.icl", self.root / "source.pdl"
        icl.write_text(BASE)
        pdl.write_text("iPDLLevel 1\niProcsForModule Chains\n" + body +
                       "iState enable 0; iState value 9; iCall observe\n")
        output = self.run_tool(
            icl, pdl, extra=("--local_reset", str(self.root / "reset.pdl")))
        self.assertIn(body, (self.root / "reset.pdl").read_text())
        self.assertIn(b"iNote 15\n", output)
        self.assertEqual(pdl.read_bytes(), (self.root / "reset.pdl").read_bytes())

    def test_legacy_label_migrates_once_and_default_is_unchanged(self):
        help_result = subprocess.run([str(cli.BIN), "--help"], capture_output=True,
                                     text=True, timeout=45)
        self.assertEqual(help_result.returncode, 0, help_result.stderr)
        self.assertIn("Model PIO/DataRegister readback", help_result.stdout)
        legacy = "modeled combinational PIO readback; no live device"
        output = self.replay(
            BASE, "iPDLLevel 1; iNote {" + legacy + "}; "
            "iState enable 0; iState value 9; iRead value 0; iApply; "
            "iNote [iGetReadData value -dec]")
        self.assertEqual(output.count(helpers.LABEL), 1)
        self.assertNotIn(legacy.encode(), output)
        icl, pdl = self.root / "source.icl", self.root / "source.pdl"
        pdl.write_text("iPDLLevel 1; iNote {" + legacy + "}; "
                       "iState enable 0; iState value 9; iRead value 0; iApply")
        default = self.run_tool(icl, pdl, "default.pdl", modeled=False)
        self.assertIn(legacy.encode(), default)
        self.assertNotIn(helpers.LABEL, default)

    def test_asymmetric_ascending_storage_and_manual_slice(self):
        source = BASE.replace("[3:0]", "[0:3]")
        output = self.replay(source, """iPDLLevel 1
iState enable 0; iState value 10; iRead value 0; iApply
iState inverted[0:1] 0 -LastReadValue
iNote [iGetMiscompares value -dec]
iNote [iGetReadData value -dec]; iNote [iGetReadData inverted -dec]
iNote [iGetStatus]
""")
        for value in (10, 14, 1, 2):
            self.assertIn(("iNote " + str(value) + "\n").encode(), output)

    def test_atomic_default_unreset_history_and_callback_boundaries(self):
        icl, pdl = self.root / "source.icl", self.root / "source.pdl"
        target = self.root / "result.pdl"
        read = ("iState enable 0; iState value 9; iRead value 0; "
                "iApply; iGetReadData value")
        for source, commands, modeled, message in (
                (BASE, read, False, "hardware readback"),
                (BASE, "iState enable 0; iState value 9; iRead value 0; "
                 "iWrite value 6; iApply; iGetReadData value", True,
                 "cannot justify"),
                (BASE, "iReset; iRunLoop 1 -tck; " + read, True,
                 "does not support iRunLoop"),
                (BASE, "iMerge -begin; iMerge -end; " + read, True,
                 "does not support iMerge"),
                (callbacks.MODEL, "iRead first.observed 0; iApply", True,
                 "ReadCallBack execution is not implemented"),
                (callbacks.MODEL, "iWrite first.value 1; iApply", True,
                 "WriteCallBack execution is not implemented")):
            with self.subTest(message=message):
                icl.write_text(source)
                pdl.write_text("iPDLLevel 1; " + commands)
                target.write_bytes(b"preserve")
                self.assertIn(message, self.run_tool(
                    icl, pdl, ok=False, modeled=modeled))
                self.assertEqual(target.read_bytes(), b"preserve")


if __name__ == "__main__":
    unittest.main()
