#!/usr/bin/env python3
"""Opaque parallel queues retain per-chain lengths and stable controlling data."""
import unittest
import time

import test_opaque_root_scan as helpers

MODEL = """Module Top {
 ScanInPort si[3:2]; ScanOutPort so[5:4];
 SelectPort sel; CaptureEnPort ce; ShiftEnPort se; UpdateEnPort ue;
 TCKPort tck; ResetPort reset;
 ScanInterface client {
  Port sel; Port ce; Port se; Port ue; Port tck; Port reset;
  Chain first {Port si[3]; Port so[5]; DefaultLoadValue 4'b0;}
  Chain second {Port si[2]; Port so[4]; DefaultLoadValue 7'b0;}
 }
}
"""


class OpaqueParallelScan(unittest.TestCase):
    setUp = helpers.OpaqueRootScan.setUp
    run_tool = helpers.OpaqueRootScan.run_tool
    files = helpers.OpaqueRootScan.files
    rejected = helpers.OpaqueRootScan.rejected

    def replay(self, commands, model=MODEL):
        return helpers.OpaqueRootScan.replay(self, commands, model)

    def test_unequal_named_payloads_and_one_apply(self):
        output = self.replay("""iScan client -chain second 7 -si 66 -so 3
iScan client -chain first 4 -si 10 -so 5
iApply -together
""")
        self.assertIn("iScan client -chain first 4 -si 0b1010 -so 0b0101", output)
        self.assertIn("iScan client -chain second 7 -si 0b1000010 -so 0b0000011", output)
        self.assertEqual(output.count("iApply"), 1)
        self.assertLess(output.index("-chain first"), output.index("-chain second"))

    def test_replacement_variable_width_reset_and_empty_apply(self):
        output = self.replay("""iScan client -chain first 4 -si 1
iScan client -chain first 5 -si 3
iScan client -chain second 7 -si 2
iApply; iApply
iScan client -chain first 4 -si 7; iReset; iApply
iScan client -chain first 8 -si 0x5a -stable; iApply
""")
        self.assertIn("-chain first 5 -si 0b00011", output)
        self.assertNotIn("-chain first 4 -si 0b0001", output)
        self.assertNotIn("-chain first 4 -si 0b0111", output)
        self.assertIn("-chain first 8 -si 0b01011010", output)
        self.assertEqual(output.count("iApply"), 2)

    def test_reset_defaults_supply_untargeted_chain_without_comparing(self):
        output = self.replay("iReset; iScan client -chain first 4 -si 9; iApply\n")
        self.assertIn("-chain first 4 -si 0b1001 -so 0bxxxx", output)
        self.assertIn("-chain second 7 -si 0b0000000 -so 0bxxxxxxx -stable", output)

    def test_stable_input_reapplied_with_cleared_expected_data(self):
        output = self.replay("""iReset
iScan client -chain first 4 -si 9 -so 0 -stable; iApply
iScan client -chain second 7 -si 5 -so 0 -stable; iApply
""")
        scans = [line for line in output.splitlines() if line.startswith('iScan ')]
        self.assertEqual(scans.count("iScan client -chain first 4 -si 0b1001 -so 0b0000 -stable"), 1)
        self.assertEqual(scans.count("iScan client -chain first 4 -si 0b1001 -so 0bxxxx -stable"), 1)
        self.assertEqual(output.count("iApply"), 2)

    def test_omitted_unknown_length_rejects_atomically(self):
        for commands in (
                "iScan client -chain first 4 -si 1; iApply\n",
                "iScan client -chain first 4 -si 1 -stable; "
                "iScan client -chain second 7 -si 2; iApply; "
                "iScan client -chain first 4 -si 3; iApply\n"):
            self.rejected(commands, "length", MODEL)

    def test_named_default_is_not_the_scalar_alias(self):
        source = MODEL.replace("Chain first", "Chain default")
        output = self.replay("iReset; iScan client -chain default 4 -si 5; iApply\n", source)
        self.assertIn("iScan client -chain default 4 -si 0b0101", output)

    def test_known_zero_status_uses_so_for_each_chain(self):
        output = self.replay("""iReset
iScan client -chain first 4 -si 15
iScan client -chain second 7 -si 127
iApply; iNote [iGetStatus]
""")
        self.assertIn("iNote 0\n", output)

    def test_defaults_and_invalid_chain_or_discarded_data(self):
        for commands, diagnostic, source in (
                ("iScan client 4; iApply", "chain", MODEL),
                ("iScan client -chain missing 4; iReset", "chain", MODEL),
                ("iScan client -chain First 4; iReset", "chain", MODEL),
                ("iScan client -chain first 4 -si 16; iReset", "scan", MODEL),
                ("iScan client -chain first 4 -si 16; "
                 "iScan client -chain first 4 -si 0; iApply", "scan", MODEL),
                ("iReset; iScan client -chain first 4; iApply", "DefaultLoadValue",
                 MODEL.replace("DefaultLoadValue 4'b0;", "")),
                ("iReset; iScan client -chain first 4; iApply", "DefaultLoadValue",
                 MODEL.replace("DefaultLoadValue 4'b0;", "DefaultLoadValue 0;"))):
            with self.subTest(commands=commands):
                self.rejected(commands + "\n", diagnostic, source)

    def test_shared_or_computed_endpoints_remain_rejected(self):
        for source, diagnostic in (
                (MODEL.replace("Port si[2]", "Port si[3]"), "ScanInterface"),
                (MODEL.replace("ScanOutPort so[5:4];",
                               "ScanOutPort so[5:4] {Enable sel;}"), "opaque"),
                (MODEL.replace("SelectPort sel;", "SelectPort sel[1:0];")
                 .replace("Port sel;", "Port sel[0];"), "opaque")):
            with self.subTest(diagnostic=diagnostic):
                self.rejected("iReset; iScan client -chain first 4; iApply\n",
                              diagnostic, source)

    def test_queued_payload_limit_includes_implicit_lanes(self):
        self.rejected("iReset; iScan client -chain first 1048576; iApply\n",
                      "resource", MODEL)

    def test_repeated_reset_does_not_expand_unused_defaults(self):
        body = "iReset;\n" * 800
        for parallel in (False, True):
            with self.subTest(parallel=parallel):
                source = MODEL if parallel else helpers.MODEL
                small = source if parallel else source.replace(
                    "8'b00000000", "4'b0")
                large = source.replace("4'b0" if parallel else "8'b00000000",
                                       "1048576'b0")
                commands = (body + "iScan client -chain first 4; "
                            "iScan client -chain second 7; iApply\n" if parallel
                            else body + "iScan client 4; iApply\n")
                samples, outputs = [], []
                for model in (small, large):
                    icl, pdl = self.files(commands, model)
                    start = time.monotonic()
                    outputs.append(self.run_tool(icl, pdl))
                    samples.append(time.monotonic() - start)
                self.assertEqual(outputs[0], outputs[1])
                print("opaque reset: parallel=", parallel, "resets=800 seconds=", samples)
                self.assertLess(samples[1], max(0.5, samples[0] * 8),
                                "reset expanded unused default payloads")

    def test_many_named_chains_scale(self):
        count, width = 32, 4096
        source = "Module Top {ScanInPort si[31:0]; ScanOutPort so[31:0]; SelectPort sel; "
        source += "ScanInterface client {Port sel; "
        source += "".join("Chain c" + str(i) + " {Port si[" + str(i) +
                          "]; Port so[" + str(i) + "]; DefaultLoadValue " +
                          str(width) + "'b0;}" for i in range(count)) + "}}"
        body = "".join("iScan client -chain c" + str(i) + " " + str(width) +
                       " -si " + str(i) + " -stable;\n" for i in reversed(range(count)))
        output = self.replay(body + "iApply\n", source)
        self.assertEqual(output.count("iScan "), count)
        self.assertEqual(output.count("iApply"), 1)


if __name__ == "__main__":
    unittest.main()
