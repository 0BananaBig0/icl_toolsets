#!/usr/bin/env python3
"""Feedback updates preserve real storage and replay through flat/generated PDL."""
import time
import unittest

import test_data_register_state as state_helpers
import test_data_reset_releases as reset_helpers
import test_simultaneous_scan_chains as helpers

BASE = """Module Chains {
 DataInPort enable;
 DataRegister value[3:0] {WriteDataSource value; WriteEnSource enable;}
 DataOutPort q[3:0] {Source value;}
}
"""


def reset_network(groups, width, depth, masked=False):
    """Independent feedback shifts and reset cascades; all sink reset values are zero."""
    source = """Module Sink {ScanInPort si; ResetPort rst;
 ScanRegister r {ScanInSource si; ResetValue 0;}
 ScanOutPort so {Source r;} DataOutPort q {Source r;}}
Module Chains {ScanInPort si; ResetPort global;
"""
    source += f"DataInPort enable[{groups-1}:0]; DataInPort mask[{groups-1}:0];\n"
    commands = "iReset; iState enable 0; iTake mask; iState mask 0b" + ("0" if masked else "1")*groups + "; "
    serial = "si"
    sinks = []
    for group in range(groups):
        source += (f"DataRegister v{group}[{width-1}:0] {{WriteDataSource 1'b0,v{group}[{width-1}:1]; "
                   f"WriteEnSource enable[{group}];}}\n"
                   f"LogicSignal pulse{group} {{mask[{group}] & (v{group}[0]^v{group}[{width-1}]);}}\n")
        commands += f"iState v{group} {(1<<width)-1}; "
        previous = None
        for level in range(depth):
            name = f"s{group}_{level}"
            selected = f"pulse{group}" if previous is None else f"~{previous}.q"
            source += (f"LogicSignal select_{name} {{{selected};}}\n"
                       f"DataMux gate_{name} SelectedBy select_{name} {{1'b0:global; 1'b1:1'b1;}}\n"
                       f"Instance {name} Of Sink {{InputPort si={serial}; InputPort rst=gate_{name};}}\n")
            commands += f"iState {name}.r 1; "
            sinks.append(f"{name}.q")
            serial, previous = f"{name}.so", name
    source += f"ScanOutPort so {{Source {serial};}} DataOutPort q[{groups*depth-1}:0] {{Source " + ",".join(sinks) + ";}}\n"
    commands += "iWrite mask 0b" + ("0" if masked else "1")*groups + "; "
    commands += "iWrite enable 0b" + "1"*groups + "; iApply; iRead q 0b" + ("1" if masked else "0")*(groups*depth) + "; iApply;\n"
    return source, commands


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

    def test_intermediate_held_bits_and_release(self):
        for scan in (False, True):
            source = BASE.replace("[3:0]", "[1:0]")
            source = source.replace("WriteDataSource value", "WriteDataSource 1'b0,value[1]")
            prefix = "iState enable 0; iState value 2; "
            target = "enable"
            if scan:
                source = source.replace("WriteEnSource enable", "WriteEnSource bus")
                source = source.replace(" DataInPort enable;", " DataInPort enable; ScanInPort si;\n"
                                        " ScanRegister bus {ScanInSource si;}\n"
                                        " ScanOutPort so {Source bus;}\n")
                prefix += "iState bus 0; "
                target = "bus"
            prefix += "iTake value[0]; iState value[0] 0; "
            self.reject(source, prefix + f"iWrite {target} 1; iApply;\n")
            self.replay(source, prefix + f"iRelease value[0]; iWrite {target} 1; "
                        "iApply; iRead q 0; iApply;\n")

    def test_transient_external_reset_retains_pulse(self):
        source = """Module Victim {
 ScanInPort si; ResetPort rst;
 ScanRegister value {ScanInSource si; ResetValue 0;}
 ScanOutPort so {Source value;}
}
Module Chains {
 ScanInPort si; ResetPort global;
 DataInPort enable; DataInPort mask;
 DataRegister value[1:0] {WriteDataSource 1'b0,value[1]; WriteEnSource enable;}
 LogicSignal asserted {mask & (value[0] ^ value[1]);}
 DataMux gate SelectedBy asserted {1'b0: global; 1'b1: 1'b1;}
 Instance victim Of Victim {InputPort si = si; InputPort rst = gate;}
 ScanOutPort so {Source victim.so;}
 DataOutPort q[1:0] {Source value;}
}"""
        prefix = "iReset; iState enable 0; iState value 3; iState victim.value 1; "
        self.replay(source, prefix + "iTake mask; iState mask 1; iWrite enable 1; "
                    "iApply; iRead q 0; iApply; iRead victim.value 0; iApply;\n")
        self.replay(source, prefix + "iTake mask; iState mask 1; iWrite value 0; "
                    "iApply; iRead q 0; iApply; iRead victim.value 0; iApply;\n")
        reset_helpers.DataResetReleases.rewrite(
            self, source, prefix + "iTake mask; iState mask 1; iTake victim.value; "
            "iState victim.value 1; iWrite enable 1; iApply;\n", ok=False)
        self.replay(source, prefix + "iTake mask; iState mask 0; iWrite enable 1; "
                    "iApply; iRead q 0; iApply; iRead victim.value 1; iApply;\n")

    def test_cascaded_feedback_resets_and_fill(self):
        source = """Module Sink {
 ScanInPort si; ResetPort rst {ActivePolarity 0;}
 ScanRegister r[2:3] {ScanInSource si; ResetValue 0;}
 ScanOutPort so {Source r[3];} DataOutPort q[2:3] {Source r;}
}
Module Chains {ScanInPort si; ResetPort global; DataInPort enable;
 ScanRegister bus {ScanInSource si; CaptureSource bus;}
 DataRegister v[3:2] {WriteDataSource 1'b0,v[3]; WriteEnSource enable;}
 Alias word[1:0] = v;
 LogicSignal pulse {word[0] ^ word[1];}
 DataMux gate SelectedBy pulse {1'b0:global; 1'b1:1'b1;}
 LogicSignal cascade_pulse {~a.q[2];}
 DataMux cascade_gate SelectedBy cascade_pulse {1'b0:global; 1'b1:1'b1;}
 Instance a Of Sink {InputPort si=bus; InputPort rst=~gate;}
 Instance b Of Sink {InputPort si=a.so; InputPort rst=~cascade_gate;}
 ScanOutPort so {Source b.so;}
 DataOutPort q[2:3] {Source b.q;}
}"""
        prefix = "iReset; iState enable 0; iState bus 0; iState v 3; iState a.r 3; iState b.r 3; "
        for mixed in (False, True):
            current = source.replace("WriteEnSource enable;", "WriteEnSource bus;") if mixed else source
            commands = prefix + ("iWrite bus 1; " if mixed else "iWrite enable 1; ")
            commands += ("iApply -together; iRead q 0; iApply; "
                         "iWrite b.r 0bxx; iApply; iRead q 0; iApply;\n")
            output = self.replay(current, commands)
            self.assertEqual(output, self.run_tool(self.root / "debug/Chains.icl",
                                                  self.root / "result.pdl", "flat-generated/result.pdl"))
        self.reject(source, prefix + "iTake b.r; iState b.r 3; iWrite enable 1; iApply;\n")

    def test_feedback_reset_context_guards(self):
        base = """Module Sink {ScanInPort si; ResetPort rst;
 ScanRegister r {ScanInSource si; ResetValue 0;}
 ScanOutPort so {Source r;} DataOutPort q {Source r;}}
Module Chains {ScanInPort si; ResetPort global; DataInPort enable; DataInPort mask;
 DataRegister v[1:0] {WriteDataSource 1'b0,v[1]; WriteEnSource enable;}
 LogicSignal pulse {mask & (v[0]^v[1]);}
 DataMux gate SelectedBy pulse {1'b0:global; 1'b1:1'b1;}
 Instance a Of Sink {InputPort si=si; InputPort rst=gate;}
 ScanOutPort so {Source a.so;} EXTRA}
"""
        for extra in ("TCKPort clock;", "DataRegister other {WriteDataSource a.q; WriteEnSource enable;}"):
            source = base.replace("EXTRA", extra)
            prefix = "iReset; iState v 3; iState a.r 1; iState enable 0; iTake mask; "
            self.reject(source, prefix + "iState mask 1; iWrite enable 1; iApply;\n")
            if "TCKPort" not in extra:
                self.replay(source, prefix + "iState mask 0; iWrite enable 1; iApply;\n")

    def test_unknown_convergence_cannot_erase_known_state(self):
        source = BASE.replace("WriteDataSource value", "WriteDataSource 1'bx,value[3:1]")
        self.reject(source, "iState value 15; iState enable 0; iWrite enable 1; iApply;\n")

    def test_unknown_feedback_mixed_history(self):
        source = """Module Chains {
 ScanInPort si;
 ScanRegister dummy {ScanInSource si;}
 ScanOutPort so {Source dummy;}
 DataRegister f[3:0] {WriteDataSource f; DefaultLoadValue 9;}
 DataOutPort q[3:0] {Source f;}
}"""
        for value in (None, "0b1x01", "0b0000"):
            prefix = f"iState f {value}; " if value is not None else ""
            expected = value if value is not None else "0bxxxx"
            self.replay(source, prefix + "iWrite dummy 1; iApply; iWrite dummy 0; "
                        f"iApply; iRead q {expected}; iApply;\n")
        disabled = source.replace("WriteDataSource f;", "WriteDataSource 15; WriteEnSource ~f[0];")
        self.replay(disabled, "iState f 0; iWrite f 15; iWrite dummy 1; iApply -together; "
                    "iRead q 15; iApply;\n")

    def test_primary_pending_read_feedback_pulse(self):
        source = """Module Chains {
 DataInPort en; DataInPort zero;
 LogicSignal x {b ^ c;}
 LogicSignal seed {en | (zero & a);}
 DataRegister a {WriteDataSource x; WriteEnSource en;}
 DataRegister b {WriteDataSource seed; WriteEnSource en;}
 DataRegister c {WriteDataSource b; WriteEnSource en;}
 DataOutPort q {Source a;}
}"""
        prefix = "iState en 0; iState zero 0; iState a 0; iState b 0; iState c 0; "
        error = self.reject(source, prefix + "iWrite en 1; iRead q 0; iApply -together;\n")
        self.assertIn("primary", error)
        self.replay(source, prefix + "iRead q 0; iApply; iWrite en 1; iApply;\n")

    def test_implicit_one_hot_read_controls(self):
        source = """Module Bank {
 AddressPort address; DataInPort data; WriteEnPort we; ReadEnPort re;
 OneHotDataGroup group {DataRegister r {AddressValue 0;}}
 DataOutPort out {Source group;}
}
Module Chains {
 ScanInPort si;
 ScanRegister dummy {ScanInSource si; CaptureSource dummy;}
 ScanOutPort so {Source dummy;}
 AddressPort address; DataInPort data; WriteEnPort we; ReadEnPort re;
 DataInPort enable;
 Instance bank Of Bank {InputPort address=address; InputPort data=data;
                       InputPort we=we; InputPort re=re;}
 DataRegister f[1:0] {WriteDataSource f[0],bank.out; WriteEnSource enable;}
}"""
        prefix = ("iState enable 0; iWrite address 0; iWrite we 0; iWrite re 0; "
                  "iWrite data 1; iApply; iState bank.r 1; iState f 0; "
                  "iState dummy 0; iState enable 1; ")
        for explicit in (True, False):
            request = "iWrite re 1; " if explicit else ""
            output = self.replay(source, prefix + request +
                                 "iWrite f 3; iWrite dummy 1; iApply;\n")
            self.assertIn(b"iWrite re 0b1", output)

    def test_feedback_reset_actual_and_reset_only(self):
        source, commands = reset_network(1, 2, 2)
        commands = "iPDLLevel 1; " + commands.replace("iRead q 0b00", "iRead q 0b11")
        commands += "iNote [iGetReadData q -bin]; iNote [iGetMiscompares q -bin];\n"
        icl, pdl, reset = self.root / "input.icl", self.root / "input.pdl", self.root / "reset.pdl"
        icl.write_text(source)
        pdl.write_text(commands)
        output = self.run_tool(icl, pdl, extra=("--model_readback", "--local_reset", str(reset)))
        self.assertIn(b"iNote 0b00\n", output)
        self.assertIn(b"iNote 0b11\n", output)
        self.assertEqual(output, self.run_tool(self.root / "debug/Chains.icl", pdl,
                                              "flat-modeled/result.pdl", extra=("--model_readback",)))
        self.assertEqual(output, self.run_tool(icl, self.root / "result.pdl",
                                              "generated-modeled/result.pdl", extra=("--model_readback",)))
        expanded = self.run_tool(icl, reset, "reset-source/result.pdl", extra=("--model_readback",))
        self.assertEqual(expanded, self.run_tool(self.root / "debug/Chains.icl", reset,
                                                "reset-flat/result.pdl", extra=("--model_readback",)))


    def test_scan_prepared_primary_read_preserves_existing_plan(self):
        source = """Module Sink {ScanInPort si; ResetPort rst;
 ScanRegister r {ScanInSource si; ResetValue 0;}
 ScanOutPort so {Source r;} DataOutPort q {Source r;}}
Module Chains {ScanInPort si; ResetPort global;
 DataRegister v[1:0] {WriteDataSource v;}
 LogicSignal pulse {v[0]^v[1];}
 DataMux gate SelectedBy pulse {1'b0:global; 1'b1:1'b1;}
 Instance a Of Sink {InputPort si=si; InputPort rst=gate;}
 ScanOutPort so {Source a.so;}
 DataOutPort q {Source v[0]; Enable a.q;}}
"""
        body = "iReset; iState v 3; iState a.r 0; iRead q 1; iApply;\n"
        output = self.replay(source, body)
        self.assertIn(b"iWrite a.r 0b1", output)
        (self.root / "source.pdl").write_text(
            "iPDLLevel 1; " + body.replace("iRead q 1", "iRead q 0") +
            "iNote [iGetReadData q -bin]; iNote [iGetMiscompares q -bin];\n")
        observed = self.run_tool(self.root / "source.icl", self.root / "source.pdl",
                                 "modeled/result.pdl", extra=("--model_readback",))
        self.assertEqual(observed.count(b"iNote 0b1\n"), 2)

    def test_feedback_read_only_preparation_falls_back(self):
        source, commands = reset_network(1, 2, 1)
        source = source.replace("DataInPort enable[0:0];",
                                "DataInPort observe; DataInPort enable[0:0];")
        source = source.replace("DataOutPort q[0:0] {Source s0_0.q;}",
                                "DataOutPort q[0:0] {Source s0_0.q; Enable observe;}")
        commands = commands.replace("iReset;", "iReset; iState observe 0;", 1)
        output = self.replay(source, commands)
        self.assertIn(b"iWrite observe 0b1", output)

    def test_feedback_reset_source_order_and_scale(self):
        for count, width, depth in ((1, 2, 4), (16, 8, 1), (128, 8, 4)):
            for masked in (False, True):
                source, commands = reset_network(count, width, depth, masked)
                self.replay(source, commands)
                if count == 1:
                    start = source.index("DataRegister v0")
                    end = source.index("ScanOutPort so {Source s")
                    reversed_source = (source[:start] +
                                       "\n".join(reversed(source[start:end].splitlines())) +
                                       "\n" + source[end:])
                    self.replay(reversed_source, commands)


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
