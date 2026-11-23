#!/usr/bin/env python3
"""Mixed alias bits retain ordinary native accesses and explicit callbacks."""
import unittest

import test_simultaneous_scan_chains as cli
import test_modeled_readback as observations
import test_mixed_callback_groups as groups

MODEL = groups.MODEL.replace(" Alias low", """ Alias mixed[7:0] = spare,value;
 Alias inverse[7:0] = ~mixed;
 Alias reading[7:0] = spare_out,observed;
 Alias writing_read[7:0] = spare_out,value;
 Alias duplicate[5:0] = spare[0],~spare[0],value;
 Alias match[7:0] = p,value;
 Alias low""")
PROCS = groups.PROCS


class MixedCallbackAliases(unittest.TestCase):
    setUp = cli.SimultaneousChains.setUp
    run_tool = observations.ModeledReadback.run_tool
    replay = cli.SimultaneousChains.replay
    notes = groups.MixedCallbackGroups.notes

    def rejected(self, body, diagnostic, source=None, modeled=True, extra=()):
        return groups.MixedCallbackGroups.rejected(
            self, body, diagnostic, source or MODEL, modeled, extra)

    def generation_replay(self, source, body):
        icl, pdl = self.root / "source.icl", self.root / "source.pdl"
        icl.write_text(source)
        pdl.write_text(body)
        original = self.run_tool(icl, pdl, modeled=False)
        self.assertEqual(original, self.run_tool(
            self.root / "debug/Chains.icl", pdl, "flat/result.pdl", modeled=False))
        self.assertEqual(original, self.run_tool(
            icl, self.root / "result.pdl", "generated/result.pdl", modeled=False))
        return original

    def test_full_alias_and_actual_ordinary_values(self):
        body = PROCS + """set ::calls 0
iProc store {d r} {
 incr ::calls; if {$r ne "value" || $d ne "0b1001"} {error binding}
 iWrite p $d
}
iReset; iWrite mixed 0x39; iApply
if {$::calls != 1} {error count}
iRead spare_out 0; iRead q 0; iApply
iNote [iGetReadData spare_out -dec]; iNote [iGetReadData q -dec]
"""
        output = self.replay(MODEL, body)
        self.assertEqual(self.notes(output), [b"3", b"9"])
        self.assertEqual(output.count(b"iState value 0b1001"), 1)
        self.assertIn(b"iWrite spare 0b0011", output)

    def test_inverted_nested_and_partial_aliases(self):
        reads = ("; iRead spare_out 0; iRead q 0; iApply; "
                 "iNote [iGetReadData spare_out -dec]; iNote [iGetReadData q -bin]")
        output = self.replay(MODEL, PROCS + "iWrite inverse 0xC6; iApply" + reads)
        self.assertEqual(self.notes(output), [b"3", b"0b1001"])
        prefix = PROCS + "iState spare 0; iState p 0; "
        projected = self.replay(MODEL, prefix + "iWrite mixed[5:2] 0b1101; iApply" + reads)
        control = self.replay(MODEL, prefix +
                              "iWrite spare[1:0] 3; iWrite value[3:2] 1; iApply" + reads)
        self.assertEqual(projected, control)
        self.assertEqual(self.notes(projected)[0], b"3")
        self.assertIn(b"iState value 0b01xx", projected)

    def test_ascending_segmented_and_duplicate_bits(self):
        for declarations in ("DataInPort spare[0:3];",
                             "DataInPort spare[0:1]; DataInPort spare[2:3];",
                             "DataInPort spare[3:2]; DataInPort spare[1:0];"):
            source = MODEL.replace("DataInPort spare[3:0];", declarations)
            output = self.replay(source, PROCS + """iWrite mixed 0x39; iApply
iRead spare_out 0; iRead q 0; iApply
iNote [iGetReadData spare_out -dec]; iNote [iGetReadData q -dec]
""")
            self.assertEqual(self.notes(output), [b"3", b"9"])
        output = self.replay(MODEL, PROCS + """iState spare 10
iWrite duplicate 0b001001; iApply
iRead spare_out 0; iRead q 0; iApply
iNote [iGetReadData spare_out -dec]; iNote [iGetReadData q -dec]
""")
        self.assertEqual(self.notes(output), [b"11", b"9"])

    def test_multiple_callbacks_and_direction_binding(self):
        source = MODEL.replace(" Alias low", """ DataRegister other[3:0] {
 WriteCallBack Chains second <D> <R>;}
 Alias many[11:0] = spare,value,other; Alias low""")
        body = PROCS + """set ::order {}
iProc store {d r} {lappend ::order $r; iWrite p $d}
iProc second {d r} {lappend ::order $r; iWrite p $d}
iWrite many 0x399; iApply
if {$::order ne {value other}} {error order}
iRead spare_out 0; iRead q 0; iApply
iNote [iGetReadData spare_out -dec]; iNote [iGetReadData q -dec]
"""
        self.assertEqual(self.notes(self.replay(source, body)), [b"3", b"9"])
        # Read access to write-only abstract storage keeps its existing guard.
        self.rejected("iRead writing_read 0; iApply", "unobservable data read target value")

    def test_inner_apply_reset_and_cancellation(self):
        body = PROCS + """set ::calls 0
iProc store {d r} {
 incr ::calls; iApply
 iRead spare_out 0; iApply; iNote [iGetReadData spare_out -dec]
 iWrite p $d
}
iWrite mixed 0x39; iApply
if {$::calls != 1} {error duplicate}
iNote [iGetReadData spare_out -dec]
"""
        self.assertEqual(self.notes(self.replay(MODEL, body)), [b"3", b"3"])
        body = PROCS + """iState spare 5
iProc store {d r} {iReset; iWrite p $d}
iWrite mixed 0x39; iApply
iRead spare_out 0; iRead q 0; iApply
iNote [iGetReadData spare_out -dec]; iNote [iGetReadData q -dec]
"""
        self.assertEqual(self.notes(self.replay(MODEL, body)), [b"5", b"9"])
        body = PROCS + """iState spare 0; iState p 0
iWrite mixed 0x39; iReset
iRead spare_out 0; iRead q 0; iApply
iNote [iGetReadData spare_out -dec]; iNote [iGetReadData q -dec]
"""
        self.assertEqual(self.notes(self.replay(MODEL, body)), [b"0", b"0"])

    def test_static_preflight_and_publication_atomicity(self):
        source = MODEL.replace(" Alias low", """ DataRegister other[3:0] {
 WriteCallBack Chains missing <D>;} Alias badbind[11:0]=spare,value,other;
 Alias low""")
        body = """set ::calls 0
iProc store {d r} {incr ::calls; iWrite p $d}
iWrite badbind 0x399
if {![catch {iApply}]} {error accepted}
if {$::calls != 0} {error effects}
iReset
"""
        self.replay(source, PROCS + body)
        self.rejected("iWrite badbind 0x399; iApply", "undefined iProc", source=source)
        self.rejected("iWrite mixed 0x139; iApply", "truncates")
        self.rejected("iWrite mixed 0x39; iApply -together", "together")
        self.rejected("iProc request {} {iWrite mixed 0x39; iApply}; "
                      "iMerge -begin; iCall request; iMerge -end", "iMerge")

    def test_caller_goal_conflicts_and_compatible_controls(self):
        self.rejected("iWrite match 0x39; iApply", "caller")
        self.rejected("iProc store {d r} {iWrite mixed 0x29; iApply}; "
                      "iWrite spare 3; iWrite value 9; iApply", "caller")
        output = self.replay(MODEL, PROCS + """iWrite match 0x99; iApply
iRead q 0; iApply; iNote [iGetReadData q -dec]
""")
        self.assertEqual(self.notes(output), [b"9"])
        source = """Module Child {WriteEnPort we;}
Module Chains {WriteEnPort we; Instance child Of Child {InputPort we=we;}
 DataRegister value {WriteCallBack Chains write_enable <D>;}
 Alias both[1:0]=we,value;}
"""
        self.rejected("iProc write_enable {d} {iWrite child.we $d}; "
                      "iWrite both 0b01; iApply", "caller", source=source)

    def test_read_callback_generation_and_model_guard(self):
        body = PROCS + "iState spare 3; iState p 9; iRead reading 0x39; iApply"
        output = self.generation_replay(MODEL, body)
        self.assertIn(b"iRead spare_out 0b0011", output)
        self.assertIn(b"iRead q 0b1001", output)
        self.rejected("iState spare 3; iState p 9; iRead reading 0x39; iApply",
                      "callback histories")


if __name__ == "__main__":
    unittest.main()
