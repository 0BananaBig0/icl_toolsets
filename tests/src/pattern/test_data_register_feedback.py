#!/usr/bin/env python3
"""Feedback updates preserve real storage and replay through flat/generated PDL."""
import time
import unittest

import test_data_register_state as state_helpers
import test_simultaneous_scan_chains as helpers

BASE = """Module Chains {
 DataInPort enable;
 DataRegister value[3:0] {WriteDataSource value; WriteEnSource enable;}
 DataOutPort q[3:0] {Source value;}
}
"""


class Feedback(unittest.TestCase):
    setUp = helpers.SimultaneousChains.setUp
    run_tool = helpers.SimultaneousChains.run_tool
    replay = helpers.SimultaneousChains.replay
    reject = state_helpers.DataState.reject

    def test_self_hold_known_and_unknown(self):
        for enable in ("0", "1"):
            for value in ("0b1010", "0b1x01"):
                self.replay(BASE, f"iState enable 0; iState value {value}; "
                            f"iWrite enable {enable}; iApply; iRead q {value}; iApply;\n")

    def test_convergent_self_and_mutual_feedback(self):
        for source in (BASE.replace("WriteDataSource value", "WriteDataSource 1'b0,value[3:1]"),
                       BASE.replace("WriteDataSource value", "WriteDataSource other")
                       .replace(" DataOutPort", " DataRegister other[3:0] {WriteDataSource "
                                "1'b0,value[3:1]; WriteEnSource enable;}\n DataOutPort")):
            other = "iState other 15; " if " other[" in source else ""
            for value in ("15", "0bxxxx"):
                self.replay(source, f"iState enable 0; iState value {value}; " + other +
                            "iWrite enable 1; iApply; iRead q 0; iApply;\n")

    def test_self_disabling_enable_retains_definite_write(self):
        source = """Module Chains {
 DataInPort trigger;
 DataRegister flag {WriteDataSource 1'b1; WriteEnSource ~flag;}
 DataOutPort q {Source flag;}
}"""
        self.replay(source, "iState trigger 0; iState flag 0; iWrite trigger 1; "
                    "iWrite flag 1; iApply -together; iRead q 1; iApply;\n")

    def test_scan_capture_before_feedback_updates(self):
        source = """Module Chains {
 ScanInPort si;
 ScanRegister bus {ScanInSource si;}
 DataRegister value[3:0] {WriteDataSource 1'b0,value[3:1]; WriteEnSource bus;}
 ScanRegister observed[3:0] {ScanInSource bus; CaptureSource value;}
 ScanOutPort so {Source observed[0];}
 DataOutPort q[3:0] {Source value;}
}"""
        self.replay(source, "iState bus 0; iState value 15; iWrite bus 1; "
                    "iRead observed 15; iApply; iRead q 0; iApply;\n")

    def test_feedback_reset_and_new_event(self):
        source = BASE.replace("DataInPort enable;", "DataInPort enable; ResetPort rst;")
        source = source.replace("WriteDataSource value", "WriteDataSource 1'b0,value[3:1]")
        source = source.replace("WriteEnSource enable;", "WriteEnSource enable; ResetValue 0;")
        self.replay(source, "iReset; iState value 15; iState enable 0; "
                    "iWrite enable 1; iApply; iRead q 0; iApply;\n")

    def test_other_fixed_point_is_not_a_write(self):
        self.reject(BASE, "iState value 10; iState enable 1; iWrite value 0; iApply;\n")

    def test_known_oscillation_preserves_existing_artifact(self):
        source = BASE.replace("WriteDataSource value", "WriteDataSource ~value")
        self.reject(source, "iState value 0; iState enable 0; iWrite enable 1; iApply;\n")
        self.replay(source, "iState value 0; iState enable 0; iWrite enable 0; "
                    "iApply; iRead q 0; iApply;\n")

    def test_feedback_and_matched_dag_scale(self):
        for count in (16, 128):
            for feedback in (False, True):
                source = "Module Chains {DataInPort enable; DataInPort data[3:0];\n"
                for index in range(count):
                    load = f"1'b0,v{index}[3:1]" if feedback else "data"
                    source += (f"DataRegister v{index}[3:0] {{WriteDataSource {load}; "
                               "WriteEnSource enable;}\n")
                source += "DataOutPort q[" + str(count * 4 - 1) + ":0] {Source "
                source += ",".join(f"v{i}" for i in range(count)) + ";}}\n"
                body = "iState enable 0; iState data 0; "
                body += "".join(f"iState v{i} 15; " for i in range(count))
                body += "iWrite enable 1; iApply; iRead q 0; iApply;\n"
                started = time.monotonic()
                output = self.replay(source, body)
                print(f"feedback={feedback} groups={count} bits={4*count} "
                      f"icl_bytes={len(source)} pdl_bytes={len(body)} "
                      f"output_bytes={len(output)} three_replays_seconds={time.monotonic()-started:.3f}")


if __name__ == "__main__":
    unittest.main()
