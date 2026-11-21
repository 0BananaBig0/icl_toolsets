#!/usr/bin/env python3
"""Actual primary events activate DAG and feedback DataRegisters consistently."""
import unittest

import test_data_register_state as state_helpers
import test_simultaneous_scan_chains as helpers

BASE = """Module Chains {
 DataInPort trigger;
 DataRegister value[1:0] {WriteDataSource 2'b11;}
 DataOutPort q[1:0] {Source value;}
 Alias inverted[1:0] = ~value;
}"""


class PrimaryEvents(unittest.TestCase):
    setUp = helpers.SimultaneousChains.setUp
    run_tool = helpers.SimultaneousChains.run_tool
    replay = helpers.SimultaneousChains.replay
    reject = state_helpers.DataState.reject

    def test_constant_goal_emits_an_actual_pin_event(self):
        for target, data, initial in (("value", "3", "0"),
                                      ("inverted", "0", "0"),
                                      ("value", "0bx1", "2")):
            with self.subTest(target=target):
                output = self.replay(BASE, "iState trigger 0; iState value " + initial +
                                     "; iWrite " + target + " " + data +
                                     "; iApply; iRead q 3; iApply;\n")
                self.assertIn(b"iWrite trigger 0b0", output)

    def test_cascade_and_feedback_use_the_same_real_event(self):
        for source, setup in (
                (BASE.replace("WriteDataSource 2'b11", "WriteDataSource first")
                 .replace(" DataRegister value", " DataRegister first[1:0] "
                          "{WriteDataSource 2'b11;}\n DataRegister value"),
                 "iState first 0; "),
                (BASE.replace("WriteDataSource 2'b11",
                              "WriteDataSource value[0],1'b1"), "")):
            with self.subTest(setup=setup):
                output = self.replay(source, "iState trigger 0; iState value 0; " +
                                     setup + "iWrite value 3; iApply; "
                                     "iRead q 3; iApply;\n")
                self.assertIn(b"iWrite trigger 0b0", output)

    def test_read_only_and_canceled_queues_emit_no_pin_event(self):
        for canceled in ("", "iWrite value 3; iReset; iState value 0; "):
            output = self.replay(BASE, "iState trigger 0; iState value 0; " +
                                 canceled + "iRead q 0; iApply;\n")
            self.assertNotIn(b"iWrite trigger", output)
            self.assertNotIn(b"iWrite value", output)

    def test_unreachable_or_owned_events_preserve_output(self):
        blocked = BASE.replace("WriteDataSource 2'b11;",
                               "WriteDataSource 2'b11; WriteEnSource 1'b0;")
        reset = """Module ResetCell {
 ResetPort rst;
 DataRegister value[1:0] {WriteDataSource 2'b11; ResetValue 0;}
 DataOutPort q[1:0] {Source value;}
}
Module Chains {
 DataInPort trigger; DataInPort held; ResetPort rst;
 DataMux reset SelectedBy held {1'b0: rst; 1'b1: 1'b1;}
 Instance child Of ResetCell {InputPort rst = reset;}
 Alias value[1:0] = child.value;
 DataOutPort q[1:0] {Source child.q;}
}"""
        side = BASE.replace(" DataOutPort",
                            " DataRegister side {WriteDataSource 1'b1;}\n DataOutPort")
        for source, setup in (
                (BASE.replace("DataInPort trigger;", ""), ""),
                (blocked, "iState trigger 0; "),
                (blocked.replace("1\u0027b0", "1\u0027bx"), "iState trigger 0; "),
                (reset, "iState trigger 0; iTake held; iState held 1; "),
                (side, "iState trigger 0; iTake side; iState side 0; ")):
            with self.subTest(setup=setup):
                message = self.reject(source, setup + "iState value 0; "
                                      "iWrite value 3; iApply;\n")
                self.assertIn("write requirements", message)

    def test_fixed_primary_source_keeps_goal_intent(self):
        for mixed in (False, True):
            source = """Module Chains {
 DataInPort p[1:0]; DataInPort trigger;
 DataRegister value[1:0] {WriteDataSource p;}
 DataOutPort q[1:0] {Source value;}
}"""
            if mixed:
                source = source.replace("DataInPort trigger;", """DataInPort trigger;
 ScanInPort si;
 ScanRegister scan {ScanInSource si; CaptureSource scan;}
 ScanOutPort so {Source scan;}
""")
            for option in ("", " -together"):
                for initial, goal in (("0", "3"), ("2", "0bx1")):
                    with self.subTest(mixed=mixed, option=option, goal=goal):
                        setup = "iState scan 0; " if mixed else ""
                        write = "iWrite scan 1; " if mixed else ""
                        output = self.replay(
                            source, "iTake p; iState p 3; iState trigger 0; " +
                            "iState value " + initial + "; " + setup + write +
                            "iWrite value " + goal + "; iApply" + option +
                            "; iRead q 3; iApply;\n")
                        self.assertNotIn(b"iWrite p 0b00", output)
                        self.assertNotIn(b"iWrite p 0b10", output)

    def test_primary_source_order_preserves_goal_intent(self):
        for reverse in (False, True):
            ports = ["DataInPort a;", "DataInPort z[1:0];"]
            if reverse:
                ports.reverse()
            source = ("Module Chains {" + " ".join(ports) +
                      "DataRegister value[1:0] {WriteDataSource z;} "
                      "DataOutPort q[1:0] {Source value;}}")
            for option in ("", " -together"):
                for initial, goal in (("0", "3"), ("2", "0bx1")):
                    with self.subTest(reverse=reverse, option=option, goal=goal):
                        output = self.replay(
                            source, "iTake z; iState z 3; iState a 0; " +
                            "iState value " + initial + "; iWrite value " +
                            goal + "; iApply" + option +
                            "; iRead q 3; iApply;" + chr(10))
                        self.assertNotIn(b"iWrite z 0b00", output)
                        self.assertNotIn(b"iWrite z 0b10", output)

    def test_mixed_scan_event_has_matching_data_effect(self):
        source = BASE.replace("DataInPort trigger;", """DataInPort trigger;
 ScanInPort si;
 ScanRegister scan {ScanInSource si; CaptureSource scan;}
 ScanOutPort so {Source scan;}
""")
        output = self.replay(source, "iState trigger 0; iState value 0; iState scan 0; "
                             "iWrite scan 1; iWrite value 3; iApply; "
                             "iRead q 3; iApply;\n")
        self.assertIn(b"iWrite scan 0b1", output)


if __name__ == "__main__":
    unittest.main()
