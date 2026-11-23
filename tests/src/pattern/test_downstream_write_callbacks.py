#!/usr/bin/env python3
"""Transparent downstream writes invoke callback prerequisites before native work."""
import unittest

import test_simultaneous_scan_chains as cli
import test_modeled_readback as observations
import test_mixed_callback_groups as groups

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

    def generation_replay(self, body, source=MODEL):
        icl, pdl = self.root / "source.icl", self.root / "source.pdl"
        icl.write_text(source)
        pdl.write_text(PROCS + body)
        original = self.run_tool(icl, pdl, modeled=False)
        self.assertEqual(original, self.run_tool(
            self.root / "debug/Chains.icl", pdl, "flat/result.pdl", modeled=False))
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
        self.rejected("iProc access {} {iWrite dest 9; iApply}; "
                      "iMerge -begin; iCall access; iMerge -end", "callback")

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
        self.rejected("iWrite dest 9; iApply", "callback", modeled=True)
        self.rejected("iWrite dest 9; iApply", "callback", extra=("--physical_pdl",))


if __name__ == "__main__":
    unittest.main()
