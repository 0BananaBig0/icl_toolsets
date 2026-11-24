#!/usr/bin/env python3
"""Mixed alias bits retain ordinary native accesses and explicit callbacks."""
import json
from pathlib import Path
import subprocess
import unittest

import test_simultaneous_scan_chains as cli
import test_modeled_readback as observations
import test_mixed_callback_groups as groups
import test_callback_execution as callbacks

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
    reset_replay = groups.MixedCallbackGroups.reset_replay

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
        output = self.generation_replay(
            MODEL, PROCS + "iProc request {} {iWrite mixed 0x39; iApply}; "
            "iMerge -begin; iCall request; iMerge -end")
        self.assertIn(b"iWrite p 0b1001", output)
        self.assertIn(b"iWrite spare 0b0011", output)

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

    def test_read_callback_generation_and_actual_mixed_histories(self):
        body = PROCS + "iState spare 3; iState p 9; iRead reading 0x39; iApply"
        output = self.generation_replay(MODEL, body)
        self.assertIn(b"iRead spare_out 0b0011", output)
        self.assertIn(b"iRead q 0b1001", output)
        observed = self.replay(MODEL, body+'''
iNote [iGetReadData spare_out -dec]; iNote [iGetReadData q -dec]; iNote [iGetStatus]
''')
        self.assertEqual(self.notes(observed), [b'3', b'9', b'0'])


    def test_actual_getters_inside_write_procedures(self):
        body = PROCS + """iProc store {d r} {
 iApply; iRead spare_out 9; iApply
 iNote [iGetReadData spare_out -dec]
 iNote [iGetMiscompares spare_out -bin]; iNote [iGetStatus -clear]
 iWrite p $d
}
iReset; iWrite mixed 0x39; iApply
iNote [iGetReadData spare_out -dec]; iNote [iGetStatus]
iNote [iGetReadData value -bin]
"""
        self.assertEqual(self.notes(self.replay(MODEL, body)),
                         [b"3", b"0b1010", b"2", b"3", b"0", b"0bxxxx"])

    def test_canceled_mixed_reads_preserve_prior_history(self):
        body = PROCS + """iReset; iState spare 3
iRead spare_out[3:2] 0; iApply
iRead reading 0x39; iReset
iNote [iGetReadData spare_out -bin]
iRead spare_out[1:0] 0; iApply
iNote [iGetReadData spare_out -bin]
"""
        self.assertEqual(self.notes(self.replay(MODEL, body)), [b"0b00xx", b"0b0011"])

    def test_prefix_namespace_split_files_once_only_arguments(self):
        source = """Module Leaf {
 DataInPort pin[3:0];
 DataRegister value[3:0] {WriteCallBack Chains::ops store <D> <R>;}
 Alias mixed[7:0]=pin,value;
}
Module Chains {
 DataInPort spare[3:0]; DataOutPort spare_out[3:0] {Source spare;}
 DataInPort p[3:0]; DataOutPort q[3:0] {Source p;}
 Instance child Of Leaf {InputPort pin=spare;}
}
"""
        icl, first, second = self.root / "source.icl", self.root / "first.pdl", self.root / "second.pdl"
        icl.write_text(source)
        first.write_text("""iPDLLevel 1
iProcsForModule Chains -iProcNameSpace ops
iProc store {d r} {if {$r ne "child.value"} {error relative}; iWrite p $d}
iProcsForModule Chains
set ::calls 0
proc word {} {incr ::calls; return 0x39}
iReset; iPrefix child; iWrite mixed [word]
""")
        second.write_text("""iPrefix -; iApply
if {$::calls != 1} {error reevaluated}
iRead spare_out 0; iRead q 0; iApply
iNote [iGetReadData spare_out -dec]; iNote [iGetReadData q -dec]
""")
        result = self.run_tool(icl, first, extra=("--pdl", str(second)))
        self.assertEqual(self.notes(result), [b"3", b"9"])
        self.assertEqual(result, self.run_tool(self.root / "debug/Chains.icl", first,
                                              "flat/result.pdl", extra=("--pdl", str(second))))
        self.assertEqual(result, self.run_tool(icl, self.root / "result.pdl", "generated/result.pdl"))

    def test_local_reset_rewrite_preserves_procedure_bytes(self):
        source = callbacks.RESET_MODEL.replace(" DataInPort p;", """ DataInPort spare[3:0];
 DataOutPort spare_out[3:0] {Source spare;}
 Alias mixed[7:0]=spare,child.value;
 DataInPort p;""")
        result, rewritten = self.reset_replay(
            "iReset; iWrite mixed 0x39; iApply; "
            "iRead spare_out 0; iRead child.wire 0; iApply; "
            "iNote [iGetReadData spare_out -dec]; iNote [iGetReadData child.wire -dec]",
            source=source)
        self.assertEqual(self.notes(result), [b"3", b"9"])
        self.assertIn(callbacks.STORE_DEF, rewritten)
        self.assertIn(callbacks.REQUEST_DEF, rewritten)

    def test_repeated_sites_returned_calls_and_source_spans(self):
        body = PROCS + """iProc request {word} {iWrite mixed $word; return ignored}
iReset
foreach word {0x39 0x46} {
 iCall request $word; iApply
 iRead spare_out 0; iRead q 0; iApply
 iNote [iGetReadData spare_out -dec]; iNote [iGetReadData q -dec]
}
"""
        self.assertEqual(self.notes(self.replay(MODEL, body)), [b"3", b"9", b"4", b"6"])

    def test_sparse_masks_status_and_unknown_callback_storage(self):
        body = PROCS + """iReset; iState spare 10
iRead spare_out[3:2] 0; iApply
iWrite duplicate 0b001001; iApply
iNote [iGetReadData spare_out -bin]
iRead spare_out[1:0] 0; iApply
iNote [iGetReadData spare_out -bin]; iNote [iGetStatus -clear]; iNote [iGetStatus]
iNote [iGetReadData value -bin]
"""
        self.assertEqual(self.notes(self.replay(MODEL, body)),
                         [b"0b10xx", b"0b1011", b"3", b"0", b"0bxxxx"])
        source = MODEL.replace(" Alias mixed[7:0] = spare,value;", """ Enum Words {ThreeNine=8'b00111001;}
 Alias mixed[7:0] = spare,value {RefEnum Words;}""")
        output = self.replay(source, PROCS + """iWrite mixed ThreeNine; iApply
iRead spare_out 0; iRead q 0; iApply
iNote [iGetReadData spare_out -dec]; iNote [iGetReadData q -dec]
""")
        self.assertEqual(self.notes(output), [b"3", b"9"])

    def test_scale_receipt_dimensions_and_generated_getters(self):
        # Small receipt smoke; final qualification retains the full target grid.
        output = self.root / "scale"
        command = ["python3", str(Path(__file__).with_name("benchmark_mixed_callback_aliases.py")),
                   "--binary", str(cli.BIN), "--parent", str(cli.BIN),
                   "--sanitized", str(cli.BIN), "--output", str(output),
                   "--sizes", "4", "--repeats", "1", "--wide-widths"]
        result = subprocess.run(command, cwd=cli.ROOT, capture_output=True, text=True, timeout=90)
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        report = json.loads((output / "results.json").read_text())
        self.assertEqual(len(report["runs"]), 11)
        self.assertEqual(report["wide_runs"], [])
        for row in report["runs"]:
            self.assertTrue(row["pio_only_verified"])
            self.assertEqual((row["csu_count"], row["shifted_bits"]), (0, 0))
            self.assertEqual(row["fresh_getters"], 0 if row["mode"] == "observer-off" else 4)
            if row["mode"] in ("mixed", "separate", "observer-off"):
                self.assertEqual((row["apply_count"], row["pio_frame_count"], row["empty_apply_count"]),
                                 (24, 20, 4))
                self.assertEqual(row["endpoint_writes"], 16)
                self.assertEqual(row["callback_postconditions"], 16)
                self.assertEqual(row["original_operand_bits"], 112)
            else:
                self.assertEqual(row["apply_count"], 8)
            if row["mode"] == "mixed" and row["form"] == "source":
                self.assertEqual(row["original_request_counts"]["iWrite"], 8)
                self.assertEqual(row["declared_ordinary_bits"], 4)
                self.assertEqual(row["declared_callback_bits"], 16)
                self.assertEqual(row["declared_aliases"], 1)
            if row["mode"] in ("separate", "unused") and row["form"] == "source":
                self.assertEqual(row["declared_aliases"], 0)
            if row["form"] == "generated":
                body = Path(row["command"][row["command"].index("--pdl") + 1]).read_text()
                self.assertEqual(body.count("iGetReadData spare_out -dec"), 4)
                self.assertNotIn("iNote 1\n", body)

    def test_wide_alias_compact_projection_budget(self):
        # The dedicated normal benchmark measures4K/16K; this runs on every backend.
        output = self.root / "wide"
        command = ["python3", str(Path(__file__).with_name("benchmark_mixed_callback_aliases.py")),
                   "--binary", str(cli.BIN), "--parent", str(cli.BIN),
                   "--sanitized", str(cli.BIN), "--output", str(output),
                   "--sizes", "--wide-widths", "64", "--repeats", "1"]
        result = subprocess.run(command, cwd=cli.ROOT, capture_output=True, text=True, timeout=90)
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        report = json.loads((output / "results.json").read_text())
        self.assertEqual(report["runs"], [])
        self.assertEqual(len(report["wide_runs"]), 1)
        row = report["wide_runs"][0]
        self.assertEqual(row["alias_width"], 64)
        self.assertEqual(row["declared_callback_bits"], 60)
        self.assertEqual(row["declared_ordinary_bits"], 4)
        self.assertEqual(row["original_operand_bits"], 4 * (64 + 8))
        self.assertEqual((row["apply_count"], row["pio_frame_count"], row["empty_apply_count"]), (12, 8, 4))
        self.assertTrue(row["pio_only_verified"])
        self.assertLessEqual(row["rss_kib"], 256 * 1024)


if __name__ == "__main__":
    unittest.main()
