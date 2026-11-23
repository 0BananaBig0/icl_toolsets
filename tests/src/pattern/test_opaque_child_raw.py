#!/usr/bin/env python3
"""Opaque child privacy, preparation length knowledge and actual scan histories."""
import subprocess
import unittest

import test_opaque_root_scan as cli
from benchmark_opaque_child_raw import model

MODEL = model()


class OpaqueChildRaw(unittest.TestCase):
    setUp = cli.OpaqueRootScan.setUp
    files = cli.OpaqueRootScan.files
    replay = cli.OpaqueRootScan.replay
    rejected = cli.OpaqueRootScan.rejected

    def run_tool(self, icl, pdl, output="result.pdl", ok=True, reset=False, modeled=True, extra=()):
        command = [str(cli.BIN), "--icl", str(icl), "--pdl", str(pdl), "--top_module", "Top", "--output", str(self.root / output), "--debug"]
        if modeled:
            command.append("--model_readback")
        if reset:
            command += ["--local_reset", str(self.root / "reset.pdl")]
        result = subprocess.run([*command, *extra], capture_output=True, text=True, timeout=30)
        self.assertEqual(result.returncode == 0, ok, result.stdout + result.stderr)
        return (self.root / output).read_text() if ok else result.stdout + result.stderr

    def notes(self, result):
        return [line[6:] for line in result.splitlines() if line.startswith("iNote ") and "modeled " not in line]

    def test_parent_first_actual_opaque_unknown_and_private_exports(self):
        result = self.replay("""iReset; iWrite head 5; iWrite tail 19; iApply
iScan child.local 8 -si 0xa5 -so 0x3c -stable; iApply
iNote [iGetReadData child.local -bin]
iRead head 5; iRead tail 19; iApply
iNote [iGetReadData head -dec]; iNote [iGetReadData tail -dec]
""", MODEL)
        self.assertEqual(self.notes(result), ["0bxxxxxxxx", "5", "19"])
        self.assertNotIn("__opaque_scan_state", result)
        self.assertNotIn("__opaque_scan_state", (self.root / "debug/Top.icl").read_text())
        self.assertIn("iScan client 16 -si 0b1011010010110011", result)

    def test_full_root_actual_and_independent_histories(self):
        result = self.replay("""iReset; iWrite head 5; iWrite tail 19; iApply
iScan client 16 -si 0b1011010010110011 -stable; iApply
iNote [iGetReadData client -bin]
if {![catch {iGetReadData child.local}]} {error {implicit child history invented}}
iScan child.local 8 -si 0x3c -stable; iApply
iNote [iGetReadData child.local -bin]; iNote [iGetReadData client -bin]
""", MODEL)
        self.assertEqual(self.notes(result), ["0b101xxxxxxxx10011", "0bxxxxxxxx", "0b101xxxxxxxx10011"])

    def test_mixed_preparation_consumes_current_request_once(self):
        result = self.replay("""iReset; iWrite head 5; iWrite tail 19
iScan child.local 8 -si 0xa5; iApply
iNote [iGetReadData child.local -bin]
iReset; iWrite head 3; iApply
""", MODEL)
        self.assertEqual(self.notes(result), ["0bxxxxxxxx"])
        self.rejected("iReset; iScan child.local 8 -si 1; iApply; "
                      "iWrite head 5; iScan child.local 8 -si 2; iApply\n", "length", MODEL)

    def test_stable_default_and_reset_recovery(self):
        result = self.replay("""iReset; iWrite head 5; iWrite tail 19; iApply
iScan child.local 8 -si 0x3c -stable; iApply
iWrite head 2; iApply; iRead tail 19; iApply
iScan client 16 -si 0b0100011110010011 -stable; iApply
iNote [iGetReadData client -bin]
iScan child.local 8 -si 7; iApply; iReset; iWrite head 4; iApply
""", MODEL)
        self.assertEqual(self.notes(result), ["0b010xxxxxxxx10011"])
        self.assertIn("iScan client 16 -si 0b1010011110010011", result)

    def test_empty_apply_and_canceled_invalid_width(self):
        self.replay("iReset; iScan child.local 8 -si 1; iApply; iApply; iReset; iApply\n", MODEL)
        self.replay("iScan child.local 9; iWrite head 1; iReset; iApply\n", MODEL)
        self.rejected("iReset; iScan child.local 9; iApply\n", "length", MODEL)
        self.rejected("iScan child.local 8 -si 256; iReset\n", "scan", MODEL)

    def test_masks_status_clear_and_unknown_actual(self):
        result = self.replay("""iReset; iWrite head 5; iWrite tail 19; iApply
iScan client 16 -si 0b1010000000010011 -so 0b000xxxxxxxx00000 -stable; iApply
iNote [iGetMiscompares client -bin]; iNote [iGetStatus -clear]; iNote [iGetStatus]
iScan child.local 8 -si 0 -so 0 -stable; iApply
iNote [iGetMiscompares child.local -bin]
if {![catch {iGetStatus -clear}]} {error {unknown opaque status accepted}}
""", MODEL)
        self.assertEqual(self.notes(result), ["0b1010000000010011", "5", "0", "0bxxxxxxxx"])

    def test_aliases_inversions_and_parent_actual(self):
        for invert_si, invert_so in ((False, False), (True, False), (False, True), (True, True)):
            source = MODEL.replace(" ScanRegister head", " Alias parents[7:0] = head, tail;\n ScanRegister head")
            if invert_si:
                source = source.replace("InputPort si = head[0]", "InputPort si = ~head[0]")
            if invert_so:
                source = source.replace("Source tail[0]", "Source ~tail[0]")
            result = self.replay("iReset; iWrite parents 0b10110011; iApply; "
                                 "iScan child.local 8 -si 0xa5 -stable; iApply; "
                                 "iRead parents 0b10110011; iApply; "
                                 "iNote [iGetReadData parents -bin]\n", source)
            self.assertEqual(self.notes(result), ["0b10110011"])

    def test_prefix_namespace_split_and_local_reset_source(self):
        icl, pdl = self.files("iProcsForModule ChildLeaf -iProcNameSpace ops\n"
                             "iProc shift {} {iScan local 8 -si 0xa5 -stable; iApply}\n"
                             "iProcsForModule Top\niReset; iWrite head 5; iApply\n", MODEL)
        second = self.root / "second.pdl"
        second.write_text("iPrefix child; iCall ops::shift; iNote [iGetReadData local -bin]; iPrefix -\n")
        result = self.run_tool(icl, pdl, reset=True, extra=("--pdl", str(second)))
        self.assertEqual(self.notes(result), ["0bxxxxxxxx"])
        self.assertNotIn("__opaque_scan_state", (self.root / "reset.pdl").read_text())

    def test_private_targets_rejected_atomically(self):
        for command in ("iWrite child.__opaque_scan_state 1; iApply",
                        "iRead child.__opaque_scan_state 0; iApply",
                        "iState child.__opaque_scan_state 1"):
            self.rejected("iReset; " + command + "\n", "identifier", MODEL)

    def test_reservations_and_failed_owner_release(self):
        self.replay("iReset; iTake tail; iWrite tail 19; iApply; iWrite head 5; iApply; "
                    "iScan child.local 8 -si 0xa5 -stable; iApply; iRelease tail\n", MODEL)
        self.rejected("iReset; iTake sel; iState sel 0; "
                      "iScan child.local 8 -si 1; iApply\n", "reserved", MODEL)

    def test_clocks_overrides_time_and_merge_rejected_atomically(self):
        for command in ("iRunLoop 1", "iRunLoop -time 1ns", "iClock tck",
                        "iClockOverride tck -freqDivider 2",
                        "iOverrideScanInterface client -capture off",
                        "iMerge -begin; iMerge -end"):
            self.rejected("iReset; " + command + "; iScan child.local 8 -si 0; iApply\n",
                          "opaque", MODEL)

    def test_physical_rejected_even_without_raw(self):
        icl, pdl = self.files("iReset; iWrite head 5; iApply\n", MODEL)
        pdl.write_text(pdl.read_text().replace("iPDLLevel 1", "iPDLLevel 0"))
        target = self.root / "result.pdl"
        target.write_text("retained")
        self.assertIn("opaque", self.run_tool(icl, pdl, ok=False, modeled=False, extra=("--physical_pdl",)))
        self.assertEqual(target.read_text(), "retained")

    def test_readback_requires_opt_in(self):
        icl, pdl = self.files("iReset; iScan child.local 8 -si 1 -stable; iApply; iGetReadData child.local", MODEL)
        self.assertIn("hardware readback", self.run_tool(icl, pdl, ok=False, modeled=False))

    def test_register_free_child_uses_native_backend(self):
        source = MODEL.replace(" ScanRegister head[2:0] { ScanInSource si; CaptureSource head; ResetValue 3'b0; }\n", "")
        source = source.replace(" ScanRegister tail[4:0] { ScanInSource child.so; CaptureSource tail; ResetValue 5'b0; }\n", "")
        source = source.replace("InputPort si = head[0]", "InputPort si = si")
        source = source.replace("Source tail[0]", "Source child.so").replace("DefaultLoadValue 16'b0", "DefaultLoadValue 8'b0")
        result = self.replay("iReset; iScan child.local 8 -si 0xa5 -stable; iApply; "
                             "iNote [iGetReadData child.local -bin]\n", source)
        self.assertEqual(self.notes(result), ["0bxxxxxxxx"])

    def test_width_and_side_channel_limits(self):
        self.rejected("iReset; iScan child.local 7; iApply\n", "length", MODEL)
        side = MODEL.replace(" ScanRegister head", " DataOutPort leak {Source child.so;}\n ScanRegister head")
        self.rejected("iReset; iScan child.local 8; iApply\n", "Source requires", side)


if __name__ == "__main__":
    unittest.main()
