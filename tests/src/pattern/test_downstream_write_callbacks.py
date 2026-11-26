#!/usr/bin/env python3
"""Transparent downstream writes invoke callback prerequisites before native work."""
import json
from pathlib import Path
import subprocess
import unittest

import test_simultaneous_scan_chains as cli
import test_modeled_readback as observations
import test_mixed_callback_groups as groups
import test_callback_execution as callbacks

MODEL = """Module Chains {
 DataInPort en; DataInPort p[3:0]; DataOutPort endpoint[3:0] {Source p;}
 DataInPort spare[3:0]; DataOutPort spare_out[3:0] {Source spare;}
 DataRegister value[3:0] {WriteCallBack Chains store <D> <R>;}
 DataRegister dest[3:0] {WriteDataSource value; WriteEnSource en;}
 DataRegister other[3:0] {WriteDataSource value; WriteEnSource en;}
 DataOutPort q[3:0] {Source dest;}
 Alias inverse[3:0] = ~dest;
}"""
PROCS = """iPDLLevel 1
iProcsForModule Chains
set ::calls 0
iProc store {data reg} {
 incr ::calls; iNote "$reg=$data"; iWrite en 0; iWrite p $data
}
"""


class DownstreamWriteCallbacks(unittest.TestCase):
    setUp = cli.SimultaneousChains.setUp
    run_tool = observations.ModeledReadback.run_tool
    replay = cli.SimultaneousChains.replay
    notes = groups.MixedCallbackGroups.notes

    def generation_replay(self, body, source=MODEL, second=""):
        icl, pdl = self.root / "source.icl", self.root / "source.pdl"
        icl.write_text(source)
        pdl.write_text(PROCS + body)
        extra = ()
        if second:
            other = self.root / "second.pdl"
            other.write_text(second)
            extra = ("--pdl", str(other))
        original = self.run_tool(icl, pdl, modeled=False, extra=extra)
        self.assertEqual(original, self.run_tool(
            self.root / "debug/Chains.icl", pdl, "flat/result.pdl",
            modeled=False, extra=extra))
        self.assertEqual(original, self.run_tool(
            icl, self.root / "result.pdl", "generated/result.pdl", modeled=False))
        return original

    def rejected(self, body, diagnostic, source=MODEL, modeled=False, extra=()):
        icl, pdl = self.root / "source.icl", self.root / "source.pdl"
        icl.write_text(source)
        pdl.write_text(PROCS + body)
        artifacts = [self.root / "result.pdl", self.root / "debug/Chains.icl"]
        for artifact in artifacts:
            artifact.parent.mkdir(parents=True, exist_ok=True)
            artifact.write_bytes(b"retained")
        message = self.run_tool(icl, pdl, ok=False, modeled=modeled, extra=extra)
        self.assertIn(diagnostic, message)
        for artifact in artifacts:
            self.assertEqual(artifact.read_bytes(), b"retained")
        return message

    def reset_replay(self, source, text, definitions=(), second=""):
        icl, pdl = self.root / "source.icl", self.root / "source.pdl"
        icl.write_text(source)
        pdl.write_text(text)
        extra = ()
        if second:
            other = self.root / "second.pdl"
            other.write_text(second)
            extra = ("--pdl", str(other))
        output = self.run_tool(
            icl, pdl, modeled=False,
            extra=(*extra, "--local_reset", str(self.root / "reset.pdl")))
        rewritten = (self.root / "reset.pdl").read_text()
        for definition in definitions:
            self.assertIn(definition, rewritten)
        self.assertEqual(output, self.run_tool(
            self.root / "debug/Chains.icl", pdl, "flat/result.pdl",
            modeled=False,
            extra=(*extra, "--local_reset", str(self.root / "flat/reset.pdl"))))
        self.assertEqual(output, self.run_tool(
            icl, self.root / "result.pdl", "generated/result.pdl", modeled=False))
        # Rewritten source already includes the second file.
        self.assertEqual(output, self.run_tool(
            icl, self.root / "reset.pdl", "rewritten/result.pdl", modeled=False))
        return output, rewritten

    def test_nested_prerequisites_keep_outer_pending_writes(self):
        source = MODEL.replace("DataInPort en;", "DataInPort go; DataInPort en;").replace(
            " Alias inverse", """ DataRegister peer[3:0] {WriteCallBack Chains store <D> <R>;}
 DataRegister remote[3:0] {WriteDataSource peer; WriteEnSource go;}
 Alias inverse""")
        body = """iProc store {data reg} {
 incr ::calls
 if {$reg eq "value"} {iWrite remote 6; iApply}
 iNote "$reg=$data"; iWrite en 0; iWrite go 0; iWrite p $data
}
iWrite spare 0; iWrite dest 9; iApply
if {$::calls != 2} {error count}
"""
        output = self.generation_replay(body, source)
        self.assertEqual(self.notes(output), [b"peer=0b0110", b"value=0b1001"])
        same_target = body.replace("iWrite remote 6; iApply",
                                   "iState value $data; iWrite remote 6; iWrite dest $data; iApply")
        self.assertEqual(self.notes(self.generation_replay(same_target, source)),
                         [b"peer=0b0110", b"value=0b1001"])

    def test_held_destinations_need_no_callback(self):
        for body in ["iState dest 9; iWrite dest 9; iApply",
                     "iState dest 3; iState other 9; iWrite dest 3; iWrite other 9; iApply"]:
            with self.subTest(body=body):
                output = self.generation_replay(body + "; if {$::calls} {error count}")
                self.assertEqual(self.notes(output), [])
        unbound = MODEL.replace("Chains store <D> <R>", "Chains missing <D> <R>")
        self.assertEqual(self.notes(self.generation_replay(
            "iState dest 9; iWrite dest 9; iApply", unbound)), [])
        output = self.generation_replay("iState dest 3; iWrite dest 9; iApply")
        self.assertEqual(self.notes(output), [b"value=0b1001"])

    def test_zero_invocation_groups_preserve_native_options(self):
        for body in [
                "iWrite value 9; iApply; iWrite dest 9; iApply -together",
                "iState value 9; iState dest 9; iWrite dest 0bxxxx; iApply -together",
                "iWrite value 9; iApply; iProc access {} {iWrite dest 9; iApply}; "
                "iMerge -begin; iCall access; iMerge -end"]:
            with self.subTest(body=body):
                self.generation_replay(body)
        source = MODEL.replace(" Alias inverse", " Alias pair[7:0] = dest,other {AccessTogether;} Alias inverse")
        output = self.generation_replay(
            "iState value 9; iState dest 9; iState other 9; iWrite pair 0x99; iApply -together", source)
        self.assertEqual(self.notes(output), [])
        icl, pdl = self.root / "source.icl", self.root / "source.pdl"
        icl.write_text(MODEL)
        pdl.write_text(PROCS + "iState value 9; iState dest 9; iWrite dest 0bxxxx; iApply -together")
        self.run_tool(icl, pdl, modeled=True)

    def test_inferred_order_uses_effective_operand_positions(self):
        source = MODEL.replace("DataInPort en;", "DataInPort go; DataInPort en;").replace(
            " Alias inverse", """ DataRegister peer[3:0] {WriteCallBack Chains store <D> <R>;}
 DataRegister remote[3:0] {WriteDataSource peer; WriteEnSource go;}
 Alias reverse[7:0] = remote,dest;
 Alias inverse""")
        proc = "iProc store {data reg} {incr ::calls; iNote \"$reg=$data\"; iWrite en 0; iWrite go 0; iWrite p $data}; "
        for writes in ["iWrite reverse 0x69", "iWrite value 3; iWrite remote 6; iWrite value 9; iWrite dest 9"]:
            with self.subTest(writes=writes):
                output = self.generation_replay(proc + writes + "; iApply", source)
                self.assertEqual(self.notes(output), [b"peer=0b0110", b"value=0b1001"])

    def test_direct_and_forwarded_paths(self):
        output = self.generation_replay(
            "iWrite dest 9; iApply; if {$::calls != 1} {error count}")
        self.assertEqual(self.notes(output), [b"value=0b1001"])
        self.assertIn(b"iWrite p 0b1001", output)
        self.assertIn(b"iState value 0b1001", output)
        self.assertLess(output.index(b"iWrite p 0b1001"),
                        output.rindex(b"iWrite en 0b1"))
        forwarded = MODEL.replace("Module Chains {", """Module Leaf {
 DataInPort data[3:0]; DataInPort enable;
 DataRegister dest[3:0] {WriteDataSource data; WriteEnSource enable;}
}
Module Chains {""").replace(" Alias inverse", """
 Instance child Of Leaf {InputPort data = value; InputPort enable = en;}
 Alias inverse""")
        result = self.generation_replay(
            "iWrite child.dest 9; iApply; if {$::calls != 1} {error count}",
            forwarded)
        self.assertEqual(self.notes(result), [b"value=0b1001"])
        segmented = forwarded.replace(
            "DataInPort data[3:0];",
            "DataInPort data[0:1]; DataInPort data[2:3];")
        result = self.generation_replay(
            "iWrite child.dest 5; iApply; if {$::calls != 1} {error count}",
            segmented)
        self.assertEqual(self.notes(result), [b"value=0b0101"])

    def test_sparse_inverted_segments(self):
        for declaration in (
                "DataRegister sparse[1:0] {WriteDataSource value[3],value[1]; WriteEnSource en;}",
                "DataRegister sparse[0:1] {WriteDataSource value[3],value[1]; WriteEnSource en;}"):
            source = ("Module Chains {DataInPort en; DataInPort p[3:0]; "
                      "DataRegister value[3:0] {WriteCallBack Chains store <D> <R>;} "
                      + declaration + "}")
            result = self.generation_replay("iWrite sparse 2; iApply", source)
            self.assertEqual(self.notes(result), [b"value=0b1x0x"])
        result = self.generation_replay("iWrite inverse 6; iApply")
        self.assertEqual(self.notes(result), [b"value=0b1001"])
        source = """Module Chains {
 DataInPort en; DataInPort p[3:0];
 DataRegister value[3:0] {WriteCallBack Chains store <D> <R>;}
 DataRegister tiny {WriteDataSource value[0]; WriteEnSource en;}
 Alias duplicate[1:0] = tiny,~tiny;
}"""
        result = self.generation_replay("iWrite duplicate 0; iApply", source)
        self.assertEqual(self.notes(result), [b"value=0bxxx1"])

    def test_effective_masks_and_no_care(self):
        output = self.generation_replay(
            "iWrite dest 3; iWrite dest 9; iApply; "
            "if {$::calls != 1} {error superseded}")
        self.assertEqual(self.notes(output), [b"value=0b1001"])
        output = self.generation_replay(
            "iState value 0; iState dest 0; iWrite dest 0bxxxx; iApply; "
            "if {$::calls != 0} {error no_care}; "
            "iWrite dest 9; iReset; if {$::calls != 0} {error canceled}")
        self.assertEqual(self.notes(output), [])

    def test_multiple_targets_and_explicit_requests(self):
        for body in ("iWrite dest 9; iWrite other 9",
                     "iWrite value 9; iWrite dest 9",
                     "iWrite dest 9; iWrite value 9"):
            output = self.generation_replay(
                body + "; iApply; if {$::calls != 1} {error duplicate}")
            self.assertEqual(self.notes(output), [b"value=0b1001"])
        output = self.generation_replay(
            "iWrite value 9; iApply; iWrite dest 9; iApply; "
            "if {$::calls != 1} {error unexpected_extra_callback}")
        self.assertEqual(self.notes(output), [b"value=0b1001"])
        output = self.generation_replay(
            "iWrite value 9; iApply; iWrite dest 9; iApply; "
            "iWrite dest 6; iApply; if {$::calls != 2} {error changed_count}")
        self.assertEqual(self.notes(output), [b"value=0b1001", b"value=0b0110"])
        output = self.generation_replay(
            "iWrite value 9; iApply; iState dest 9; "
            "iWrite dest 0bxxxx; iWrite value 9; iApply; "
            "if {$::calls != 2} {error skipped_explicit_callback}")
        self.assertEqual(self.notes(output), [b"value=0b1001", b"value=0b1001"])
        output = self.generation_replay(
            "iState value 9; iWrite dest 9; iApply; "
            "if {$::calls != 0} {error assumed_callback}")
        self.assertEqual(self.notes(output), [])
        self.rejected("iWrite dest 3; iWrite other 9; iApply", "callback")
        self.rejected("iWrite dest 3; iWrite value 9; iApply", "callback")

    def test_body_apply_and_reset(self):
        output = self.generation_replay("""
iProc store {data reg} {
 incr ::calls; iWrite en 0; iWrite p $data; iApply; iNote $data
}
iWrite dest 9; iApply; if {$::calls != 1} {error duplicate}
""")
        self.assertEqual(self.notes(output), [b"0b1001"])
        output = self.generation_replay("""
iProc store {data reg} {incr ::calls; iReset; iWrite en 0; iWrite p $data}
iWrite dest 9; iApply; if {$::calls != 1} {error count}
""")
        self.assertNotIn(b"iWrite en 0b1", output)
        self.assertIn(b"iWrite p 0b1001", output)

    def test_caller_goals_and_ownership(self):
        self.rejected("iWrite p 3; iWrite dest 9; iApply", "caller")
        output = self.generation_replay("iWrite p 9; iWrite dest 9; iApply")
        self.assertEqual(self.notes(output), [b"value=0b1001"])
        self.rejected("iTake value; iWrite dest 9; iApply", "callback storage")
        self.rejected("iWrite dest 9; iApply -together", "callback")
        self.rejected("iMerge -begin; iWrite dest 9; iApply; iMerge -end", "iMerge")
        output = self.generation_replay(
            "iProc access {} {iWrite dest 9; iApply}; "
            "iMerge -begin; iCall access; iMerge -end")
        self.assertEqual(self.notes(output), [b"value=0b1001"])

    def test_preflight_and_failure_recovery(self):
        source = MODEL.replace(" Alias inverse", """
 DataRegister peer[3:0] {WriteCallBack Chains missing <D>;}
 DataRegister remote[3:0] {WriteDataSource peer; WriteEnSource en;}
 Alias inverse""")
        output = self.generation_replay("""
iWrite dest 9; iWrite remote 9
if {![catch {iApply}]} {error missing_not_detected}
if {$::calls != 0} {error early_effects}
iReset
""", source)
        self.assertEqual(self.notes(output), [])
        self.rejected("iWrite dest 9; iWrite remote 9; iApply",
                      "undefined iProc", source)
        output = self.generation_replay("""
iProc store {data reg} {incr ::calls; iWrite dest $data; iApply}
iWrite dest 9
if {![catch {iApply}]} {error cycle_not_detected}
if {![catch {iApply}]} {error poison_lost}
iReset
iProc store {data reg} {incr ::calls; iWrite en 0; iWrite p $data}
iWrite dest 6; iApply; if {$::calls != 2} {error retry_count}
""")
        self.assertIn(b"iWrite p 0b0110", output)

    def test_read_and_unsupported_mode_guards(self):
        output = self.generation_replay(
            "iRead q 9; iApply; if {$::calls != 0} {error read_is_not_write}")
        self.assertEqual(self.notes(output), [])
        icl,pdl=self.root / "source.icl",self.root / "source.pdl"
        icl.write_text(MODEL);pdl.write_text(PROCS+"iWrite dest 9; iApply\n")
        self.assertIn(b"iWrite en 0b1",self.run_tool(icl,pdl,modeled=True))
        self.rejected("iWrite dest 9; iApply", "callback", extra=("--physical_pdl",))

    def test_prefix_namespace_and_split_inputs(self):
        source = """Module Leaf {
 DataInPort data[3:0]; DataInPort enable;
 DataRegister dest[3:0] {WriteDataSource data; WriteEnSource enable;}
}
Module Chains {
 DataInPort en; DataInPort p[3:0];
 DataRegister value[3:0] {WriteCallBack Chains::api store <D> <R>;}
 Instance child Of Leaf {InputPort data=value; InputPort enable=en;}
}"""
        result = self.generation_replay("""
iProcsForModule Chains -iProcNameSpace api
iProc store {data reg} {
 incr ::calls; iNote "api:$reg=$data"; iWrite en 0; iWrite p $data
}
set ::args 0; iPrefix child; iWrite dest [expr {[incr ::args]+8}]
""", source, "iPrefix -; iApply; "
            "if {$::calls != 1 || $::args != 1} {error split_count}")
        self.assertEqual(self.notes(result), [b"api:value=0b1001"])

    def test_once_only_arguments_and_order(self):
        source = MODEL.replace(" Alias inverse", """
 DataRegister peer[3:0] {WriteCallBack Chains store <D> <R>;}
 DataRegister remote[3:0] {WriteDataSource peer; WriteEnSource en;}
 Alias inverse""")
        for body, expected in (
                ("iWrite other 9; iWrite remote 6; iWrite dest 9",
                 [b"value=0b1001", b"peer=0b0110"]),
                ("iWrite dest 3; iWrite remote 6; iWrite dest 9",
                 [b"peer=0b0110", b"value=0b1001"])):
            output = self.generation_replay(
                "set ::args 0; " + body + "; iApply; "
                "if {$::calls != 2} {error order_count}", source)
            self.assertEqual(self.notes(output), expected)
        output = self.generation_replay("""
set ::args 0
iProc request {data} {return [iWrite dest $data]}
iCall request [expr {[incr ::args]+8}]; iApply
if {$::args != 1 || $::calls != 1} {error once_only}
""")
        self.assertEqual(self.notes(output), [b"value=0b1001"])

    def test_reset_rewrite_retains_procedure_bytes(self):
        source = callbacks.RESET_MODEL.replace(
            " DataRegister value[3:0] {",
            " DataInPort gate;\n"
            " DataRegister dest[3:0] {WriteDataSource value; WriteEnSource gate;}\n"
            " DataRegister value[3:0] {").replace(
                " DataInPort p; ResetPort global;",
                " DataInPort p; DataInPort go; ResetPort global;").replace(
                    "InputPort reset = reset_gate;}",
                    "InputPort reset = reset_gate; InputPort gate = go;}")
        request = "iProc downstream {data} {iWrite child.dest $data}\n"
        output, rewritten = self.reset_replay(
            source, callbacks.RESET_PROCS + request + "iReset; iCall downstream 9\n",
            (callbacks.STORE_DEF, callbacks.REQUEST_DEF, request), "iApply\n")
        self.assertIn(b"iWrite child.wire 0b1001", output)
        self.assertIn(b"iWrite go 0b1", output)
        self.assertIn("rst", rewritten)

    def test_repeated_and_returned_sites(self):
        request = "iProc request {data} {return [iWrite dest $data]}\n"
        output, rewritten = self.reset_replay(
            MODEL, PROCS + request + """
set ::args 0
foreach value {1 2 5} {iCall request [expr {$value+[incr ::args]-$::args}]; iApply}
if {$::calls != 3 || $::args != 3} {error repeated_count}
""", (request,))
        self.assertEqual(self.notes(output),
                         [b"value=0b0001", b"value=0b0010", b"value=0b0101"])
        self.assertEqual(rewritten.count(request), 1)

    def test_nested_and_canceled_groups(self):
        source = MODEL.replace(" DataInPort en;", " DataInPort go; DataInPort en;").replace(
            " Alias inverse", """
 DataRegister peer[3:0] {WriteCallBack Chains store <D> <R>;}
 DataRegister remote[3:0] {WriteDataSource peer; WriteEnSource go;}
 Alias inverse""")
        body = """
iProc store {data reg} {
 incr ::calls
 if {$reg eq "value"} {iWrite en 0; iApply; iWrite remote 6; iApply}
 iNote "$reg=$data"; iWrite en 0; iWrite go 0; iWrite p $data
}
iWrite dest 9; iApply; if {$::calls != 2} {error nested_count}
"""
        output = self.generation_replay(body, source)
        self.assertEqual(self.notes(output), [b"peer=0b0110", b"value=0b1001"])
        output = self.generation_replay(body.replace(
            'if {$reg eq "value"}',
            'if {$reg eq "peer"} {iReset}; if {$reg eq "value"}'), source)
        self.assertNotIn(b"iWrite en 0b1", output)
        self.assertNotIn(b"iWrite go 0b1", output)
        output = self.generation_replay("""
iProc store {data reg} {
 incr ::calls; if {$reg eq "value"} {iReset}
 iWrite en 0; iWrite p $data
}
iWrite dest 9; iWrite remote 6; iApply
if {$::calls != 2} {error selected_count}
""", source)
        self.assertNotIn(b"iWrite en 0b1", output)
        self.assertNotIn(b"iWrite go 0b1", output)

    def test_logical_and_observation_boundaries(self):
        icl, pdl = self.root / "observed.icl", self.root / "observed.pdl"
        icl.write_text(MODEL)
        pdl.write_text(PROCS+"""
iState p 3; iRead endpoint 0; iApply; iGetStatus
iProc store {data reg} {
 iWrite en 0; iWrite p $data; iApply
 iRead endpoint 0; iApply; iNote [iGetReadData endpoint -dec]
}
iWrite dest 9; iApply
""")
        self.assertEqual(self.notes(self.run_tool(icl,pdl,modeled=True)),[b"9"])
        icl, pdl = self.root / "cancel.icl", self.root / "cancel.pdl"
        icl.write_text(MODEL)
        pdl.write_text(PROCS + """
iState p 3; iRead endpoint 0; iApply; iGetStatus
iWrite dest 9; iReset
iRead endpoint 0; iApply; iNote [iGetReadData endpoint -dec]
if {$::calls != 0} {error canceled_invocation}
""")
        self.assertEqual(self.notes(self.run_tool(icl, pdl, modeled=True)), [b"3"])
        source = MODEL.replace(" Alias inverse", """
 DataRegister sparse[1:0] {WriteDataSource value[3],value[1]; WriteEnSource en;}
 Alias inverse""")
        # The matched explicit control cannot schedule these collateral unknowns.
        self.rejected("iWrite sparse 2; iApply", "cannot justify", source)

    def test_scale_receipts_and_physical_controls(self):
        script = Path(__file__).with_name("benchmark_downstream_write_callbacks.py")
        result = subprocess.run(
            ["python3", str(script), "--binary", str(cli.BIN),
             "--parent", str(cli.BIN), "--sanitized", str(cli.BIN),
             "--output", str(self.root / "scale"), "--sizes", "2",
             "--wide-widths", "--repeats", "1"],
            capture_output=True, text=True, timeout=90)
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        report = json.loads((self.root / "scale/results.json").read_text())
        self.assertEqual(len(report["runs"]), 10)
        self.assertEqual(report["wide_runs"], [])
        for row in report["runs"]:
            self.assertEqual(row["connected_callback_bits"], 8)
            self.assertEqual(row["connected_destination_bits"], 8)
            self.assertEqual(row["csu_count"], 0)
            self.assertEqual(row["shifted_bits"], 0)
            self.assertTrue(row["pio_only_verified"])
            self.assertEqual(row["logical_callback_events"],
                             0 if row["mode"] == "unused" else 8)
            self.assertEqual(row["callback_invocations"],
                             0 if row["mode"] == "unused" or
                             row["form"] == "generated" else 8)
            self.assertEqual(row["destination_writes"],
                             0 if row["mode"] == "unused" else 8)

    def test_wide_dependency_budget(self):
        script = Path(__file__).with_name("benchmark_downstream_write_callbacks.py")
        result = subprocess.run(
            ["python3", str(script), "--binary", str(cli.BIN),
             "--parent", str(cli.BIN), "--sanitized", str(cli.BIN),
             "--output", str(self.root / "wide"), "--sizes",
             "--wide-widths", "16", "--repeats", "1"],
            capture_output=True, text=True, timeout=90)
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        report = json.loads((self.root / "wide/results.json").read_text())
        self.assertEqual(report["runs"], [])
        self.assertEqual(len(report["wide_runs"]), 1)
        row = report["wide_runs"][0]
        self.assertEqual(row["connected_callback_bits"], 16)
        self.assertEqual(row["connected_destination_bits"], 16)
        self.assertEqual(row["callback_invocations"], 4)


if __name__ == "__main__":
    unittest.main()
