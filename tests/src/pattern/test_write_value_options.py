#!/usr/bin/env python3
"""Validated optimization hints never replace the requested write payload."""
import re
import unittest

import test_simultaneous_scan_chains as helpers

OPTION = "-ignore_when_value_already_present"
MODES = ("auto", "off", "on")
PIN = "Module Chains {DataInPort data; DataOutPort q {Source data;}}"
SCAN = """Module Chains {
 ScanInPort si;
 ScanRegister r[3:0] {ScanInSource si; CaptureSource r; ResetValue 4'b0;}
 ScanOutPort so {Source r[0];}
}"""
DATA = """Module Chains {
 DataInPort bus[3:0]; DataInPort enable;
 DataRegister r[3:0] {WriteDataSource bus; WriteEnSource enable; ResetValue 4'b0;}
 DataOutPort q[3:0] {Source r;}
}"""
ALIASES = """Module Chains {
 DataInPort bus[3:0]; DataOutPort q[3:0] {Source bus;}
 Enum choices {Idle = 4'b0; Go = 4'b1010;}
 Alias low[1:0] = bus[1:0];
 Alias inverted[3:0] = ~bus;
 Alias tagged[3:0] = bus {RefEnum choices;}
}"""


class WriteOptions(unittest.TestCase):
    setUp = helpers.SimultaneousChains.setUp
    run_tool = helpers.SimultaneousChains.run_tool
    replay = helpers.SimultaneousChains.replay

    def test_all_modes_preserve_primary_value(self):
        for mode in MODES:
            with self.subTest(mode=mode):
                output = self.replay(PIN, "iState data 0; iWrite data 1 " +
                                     OPTION + " " + mode +
                                     "; iApply; iRead q 1; iApply;\n")
                self.assertIn(b"iWrite data 0b1", output)

    def test_scan_and_data_register_values(self):
        for mode in MODES:
            for source, setup, observed, written in (
                    (SCAN, "iReset; ", "r", b"iWrite r 0b1001"),
                    (DATA, "iReset; iState bus 0; iState enable 0; ",
                     "q", b"iWrite bus 0b1001")):
                with self.subTest(mode=mode, observed=observed):
                    output = self.replay(source, setup + "iWrite r 9 " +
                                         OPTION + " " + mode + "; iApply; iRead " +
                                         observed + " 9; iApply;\n")
                    self.assertIn(written, output)

    def test_alias_enum_and_partial_values(self):
        for target, value, initial, wanted in (
                ("low", "2", 0, 2), ("inverted", "5", 0, 10),
                ("tagged", "Go", 0, 10), ("bus", "0b1x0x", 10, 8)):
            with self.subTest(target=target):
                output = self.replay(ALIASES, "iState bus " + str(initial) +
                                     "; iWrite " + target + " " + value + " " +
                                     OPTION + " off; iApply; iRead q " +
                                     str(wanted) + "; iApply;\n")
                # Interpret emitted pin writes, independently of expected reads.
                bits = list(format(initial, "04b"))
                writes = re.findall(rb"^iWrite bus 0b([01x]{4})$", output, re.M)
                self.assertTrue(writes, output)
                for write in writes:
                    for index, bit in enumerate(write.decode()):
                        if bit != "x":
                            bits[index] = bit
                self.assertEqual(int("".join(bits), 2), wanted, output)

    def test_invalid_values_preserve_output_even_when_canceled(self):
        for value, expected in (("2", "truncates significant bits"),
                                ("Missing", "RefEnum")):
            for mode in MODES:
                icl, pdl = self.root / "bad.icl", self.root / "bad.pdl"
                icl.write_text(PIN)
                pdl.write_text("iWrite data " + value + " " + OPTION + " " +
                               mode + "; iReset;\n")
                target = self.root / "result.pdl"
                target.write_bytes(b"keep invalid-value artifact\n")
                message = self.run_tool(icl, pdl, ok=False)
                self.assertIn(expected, message)
                self.assertEqual(target.read_bytes(), b"keep invalid-value artifact\n")

    def test_tcl_once_only_and_canceled_queue(self):
        output = self.replay(PIN, """set calls 0
proc next_value {} {global calls; incr calls; return 1}
set mode off
iState data 0
iWrite data [next_value] -ignore_when_value_already_present $mode
iApply
if {$calls != 1} {error {value evaluated more than once}}
iRead q 1; iApply
""")
        self.assertIn(b"iWrite data 0b1", output)
        self.replay(PIN, "iWrite data 1 " + OPTION +
                    " on; iReset; iState data 0; iRead q 0; iApply;\n")

    def test_merge_calls_keep_their_values(self):
        source = """Module Chains {
 DataInPort a; DataInPort b;
 DataOutPort qa {Source a;} DataOutPort qb {Source b;}
}"""
        output = self.replay(source, """iPDLLevel 1
iProcsForModule Chains
iProc left {} {iWrite a 1 -ignore_when_value_already_present off; iApply}
iProc right {} {iWrite b 1 -ignore_when_value_already_present on; iApply}
iState a 0; iState b 0
iMerge -begin
iCall left
iCall right
iMerge -end
iRead qa 1; iRead qb 1; iApply
""")
        self.assertIn(b"iWrite a 0b1", output)
        self.assertIn(b"iWrite b 0b1", output)


if __name__ == "__main__":
    unittest.main()
