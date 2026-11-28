#!/usr/bin/env python3
"""Mixed ordinary/callback groups preserve caller goals and real frame histories."""
import json
import subprocess
import unittest

import test_simultaneous_scan_chains as cli
import test_modeled_readback as observations
import test_callback_execution as callbacks

MODEL = callbacks.MODEL.replace(" DataInPort p", """ DataInPort spare[3:0];
 DataOutPort spare_out[3:0] {Source spare;}
 Alias spare_inv[3:0] = ~spare; Alias p_inv[3:0] = ~p;
 DataInPort p""")
PROCS = callbacks.PROCS.replace(' iNote "$reg=$data"\n', "")


class MixedCallbackGroups(unittest.TestCase):
    setUp = cli.SimultaneousChains.setUp
    run_tool = observations.ModeledReadback.run_tool
    replay = cli.SimultaneousChains.replay

    def notes(self, result):
        return [line[6:] for line in result.splitlines()
                if line.startswith(b"iNote ") and b"modeled " not in line]

    def rejected(self, body, diagnostic, source=MODEL, modeled=True, extra=()):
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

    def test_orders_and_actual_values(self):
        for group in ("iWrite spare 3; iWrite value 9",
                      "iWrite value 9; iWrite spare 3"):
            result = self.replay(MODEL, PROCS + "iReset; " + group + """; iApply
iRead spare_out 0; iRead q 0; iApply
iNote [iGetReadData spare_out -dec]; iNote [iGetReadData q -dec]
""")
            self.assertEqual(self.notes(result), [b"3", b"9"])
            self.assertIn(b"iWrite spare 0b0011", result)
            self.assertEqual(result.count(b"iState value 0b1001"), 1)

    def test_ordinary_reads_and_callback_tail(self):
        result = self.replay(MODEL, PROCS + """iReset; iState spare 3
iRead spare_out 0; iWrite value 9; iApply
iNote [iGetReadData spare_out -dec]; iNote [iGetMiscompares spare_out -bin]
iNote [iGetStatus -clear]; iNote [iGetStatus]
iRead q 9; iApply; iNote [iGetReadData q -dec]
""")
        self.assertEqual(self.notes(result), [b"3", b"0b0011", b"2", b"0", b"9"])

    def test_inner_apply_and_selected_batch(self):
        body = PROCS + """set ::calls 0
iProc store {data reg} {
 incr ::calls
 iWrite p $data; iApply
 iRead spare_out 0; iApply; iNote [iGetReadData spare_out -dec]
}
iReset; iWrite spare 3; iWrite value 9; iApply
if {$::calls != 1} {error duplicate}
iNote [iGetReadData spare_out -dec]
"""
        result = self.replay(MODEL, body)
        self.assertEqual(self.notes(result), [b"3", b"3"])
        self.assertEqual(result.count(b"iState value 0b1001"), 1)

    def test_body_reset_cancels_reads_and_writes(self):
        body = PROCS + """iReset; iWrite spare 3; iApply; iRead spare_out 0; iApply
iProc store {data reg} {iReset; iWrite p $data}
iRead spare_out 0; iWrite spare 4; iWrite value 9; iApply
iNote [iGetReadData spare_out -dec]
iRead q 9; iApply; iNote [iGetReadData q -dec]
"""
        self.assertEqual(self.notes(self.replay(MODEL, body)), [b"3", b"9"])

        nested = MODEL.replace(" DataInPort p", """ DataRegister nested[3:0] {
 WriteCallBack Chains nested_op <D>;} DataInPort p""")
        body = PROCS + """iProc nested_op {d} {
 iReset; iTake p -persistent; iWrite p $d; iApply
}
iProc store {d r} {iApply; iWrite nested $d; iApply}
iReset; iWrite p 3; iWrite value 9; iApply
iRead q 0; iApply; iNote [iGetReadData q -dec]
"""
        self.assertEqual(self.notes(self.replay(nested, body)), [b"9"])

    def test_compatible_and_rejected_write_collisions(self):
        result = self.replay(MODEL, PROCS + "iWrite p 9; iWrite value 9; iApply; "
                             "iRead q 0; iApply; iNote [iGetReadData q -dec]")
        self.assertEqual(self.notes(result), [b"9"])
        for goal, data in (("p 3", "9"), ("p_inv 0b1100", "9"),
                           ("p[1:0] 3", "9"), ("p 3", "0bxxxx")):
            self.rejected("iWrite " + goal + "; iWrite value " + data + "; iApply", "caller")
        self.rejected("iProc store {data reg} {iState p 9}; "
                      "iWrite p 3; iWrite value 9; iApply", "caller")
        self.rejected("iProc store {data reg} {iApply; iWrite q $data}; "
                      "iWrite p 3; iWrite value 9; iApply", "caller")
        self.rejected("iProc store {data reg} {iApply; iWrite p $data}; "
                      "iWrite q 3; iWrite value 9; iApply", "caller")
        self.rejected("iProc store {data reg} {iApply; iState p 9}; "
                      "iWrite q 3; iWrite value 9; iApply", "caller")
        self.rejected("iProc endpoint {data} {iWrite q $data; iApply}; "
                      "iProc store {data reg} {iApply; iMerge -begin; iCall endpoint $data; iMerge -end}; "
                      "iWrite p 3; iWrite value 9; iApply", "caller", modeled=False)
        self.rejected("iProc endpoint {data} {iWrite q $data; iApply}; "
                      "iProc independent {} {iWrite spare 2; iApply}; "
                      "iProc store {data reg} {iApply; iMerge -begin; iCall endpoint $data; iCall independent; iMerge -end}; "
                      "iWrite p 3; iWrite value 9; iApply", "caller", modeled=False)
        compatible = PROCS + ("iProc store {data reg} {iApply; iWrite q $data}; "
                              "iWrite p 3; iWrite value 3; iApply; "
                              "iRead q 0; iApply; iNote [iGetReadData q -dec]")
        self.assertEqual(self.notes(self.replay(MODEL, compatible)), [b"3"])

        enable_model = """Module Child {WriteEnPort we;}
Module Chains {WriteEnPort we; Instance child Of Child {InputPort we = we;}
 DataRegister value {WriteCallBack Chains w <D>;}}"""
        for caller, endpoint in (("we", "child.we"), ("child.we", "we")):
            for prefix in ("", "iApply; iWrite " + caller + " 0; "):
                body = ("iProc w {d} {" + prefix + "iWrite " + endpoint +
                        " $d}; iReset; iWrite " + caller + " 0; iWrite value 1; iApply")
                for modeled in (False, True):
                    self.rejected(body, "caller", source=enable_model, modeled=modeled)
            compatible = ("iPDLLevel 1; iProcsForModule Chains; iProc w {d} {iWrite " +
                          endpoint + " $d}; iReset; iWrite " + caller +
                          " 1; iWrite value 1; iApply")
            output = self.replay(enable_model, compatible)
            self.assertIn(b"iWrite we 0b1", output)

    def test_pending_reassertion_cannot_bypass_state_guard(self):
        for modeled in (False, True):
            self.rejected("iProc store {data reg} {iApply; iWrite q 3; iState p 9; iReset}; "
                          "iWrite q 3; iWrite value 9; iApply", "caller", modeled=modeled)
        compatible = PROCS + ("iProc store {data reg} {iApply; iWrite q 3; iState p 3; iReset}; "
                              "iWrite q 3; iWrite value 9; iApply; "
                              "iRead q 0; iApply; iNote [iGetReadData q -dec]")
        self.assertEqual(self.notes(self.replay(MODEL, compatible)), [b"3"])

    def test_static_preflight_before_effects(self):
        body = """set ::calls 0
iProc store {data reg} {incr ::calls; iWrite p $data}
iWrite spare 16; iWrite value 9
if {![catch {iApply}]} {error invalid_accepted}
if {$::calls != 0} {error effects_before_preflight}
iReset
"""
        self.rejected(body, "truncates")
        body = PROCS + """set ::calls 0
iProc store {data reg} {incr ::calls; iWrite p $data}
if {![catch {iWrite value 9; iRead missing 0; iApply}]} {error invalid_accepted}
if {$::calls != 0} {error effects_before_preflight}
iReset; iNote preflight
"""
        self.assertEqual(self.notes(self.replay(MODEL, body)), [b"preflight"])

    def test_reset_error_retry_and_independent_ownership(self):
        body = PROCS + """set ::calls 0
iProc store {data reg} {
 incr ::calls; iWrite p $data; iApply
 if {$data == 9} {error stopped}
}
iReset; iTake spare; iWrite spare 3; iWrite value 9
if {![catch {iApply}]} {error missing_failure}
if {![catch {iApply}]} {error missing_poison}
iReset; iWrite spare 4; iWrite value 6; iApply
if {$::calls != 2} {error count}
iRead spare_out 0; iRead q 0; iApply
iNote [iGetReadData spare_out -dec]; iNote [iGetReadData q -dec]
"""
        self.assertEqual(self.notes(self.replay(MODEL, body)), [b"4", b"6"])

    def reset_replay(self, body, source=None, second=""):
        source = source or callbacks.RESET_MODEL.replace(
            " DataInPort p;", " DataInPort spare[3:0]; DataOutPort spare_out[3:0] {Source spare;}\n DataInPort p;")
        icl, pdl = self.root / "source.icl", self.root / "source.pdl"
        icl.write_text(source)
        pdl.write_text(callbacks.RESET_PROCS + body)
        extra = ()
        if second:
            other = self.root / "second.pdl"
            other.write_text(second)
            extra = ("--pdl", str(other))
        original = self.run_tool(icl, pdl, extra=("--local_reset", str(self.root / "reset.pdl"), *extra))
        reset = (self.root / "reset.pdl").read_text()
        self.assertIn(callbacks.STORE_DEF, reset)
        self.assertIn(callbacks.REQUEST_DEF, reset)
        self.assertEqual(original, self.run_tool(self.root / "debug/Chains.icl", pdl,
                                                "flat/result.pdl", extra=("--local_reset", str(self.root / "flat/reset.pdl"), *extra)))
        self.assertEqual(original, self.run_tool(icl, self.root / "result.pdl", "generated/result.pdl"))
        self.assertEqual(original, self.run_tool(icl, self.root / "reset.pdl", "rewritten/result.pdl"))
        return original, reset

    def test_split_local_reset_replay_executes_second_file_once(self):
        output, rewritten = self.reset_replay('set ::split_count 0\niReset\n',second=
            'incr ::split_count\n'
            'if {$::split_count != 1} {error second_file_repeated}\n'
            'iWrite spare 3; iApply; iRead spare_out 0; iApply\n'
            'iNote [iGetReadData spare_out -dec]\n')
        self.assertEqual(self.notes(output),[b'3'])
        self.assertEqual(rewritten.count('incr ::split_count'),1)

    def test_split_prefix_namespace_arguments(self):
        source = """Module Leaf {DataRegister value[3:0] {WriteCallBack Chains::ops store <D> <R>;}}
Module Chains {DataInPort spare[3:0]; DataOutPort spare_out[3:0] {Source spare;}
 DataInPort p[3:0]; DataOutPort q[3:0] {Source p;} Instance child Of Leaf;}
"""
        icl, first, second = self.root / "source.icl", self.root / "first.pdl", self.root / "second.pdl"
        icl.write_text(source)
        first.write_text("""iPDLLevel 1
iProcsForModule Chains -iProcNameSpace ops
iProc store {data reg} {if {$reg ne "child.value"} {error relative}; iWrite p $data}
iProcsForModule Chains
set ::calls 0
proc word {} {incr ::calls; return 9}
iReset; iWrite spare 3; iPrefix child; iWrite value [word]
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

    def test_local_reset_source_and_immutable_procedures(self):
        result, rewritten = self.reset_replay(
            "iReset; iWrite spare 3; iCall request 9; iApply; "
            "iRead spare_out 0; iRead child.wire 0; iApply; "
            "iNote [iGetReadData spare_out -dec]; iNote [iGetReadData child.wire -dec]")
        self.assertEqual(self.notes(result), [b"3", b"9"])
        self.assertIn("iWrite rst", rewritten)
        self.assertEqual(rewritten.count(callbacks.STORE_DEF), 1)

    def test_repeated_source_sites_and_returned_calls(self):
        result, rewritten = self.reset_replay("""iReset
set ::calls 0
proc word {} {incr ::calls; return 9}
foreach unused {a b} {iWrite spare 3; iCall request [word]; iApply}
if {$::calls != 2} {error reevaluated}
""")
        self.assertEqual(result.count(b"iState child.value 0b1001"), 2)
        self.assertIn("proc word", rewritten)

    def test_getters_inside_body_and_after_outer_apply(self):
        body = PROCS + """iReset; iWrite spare 3; iApply
iProc store {data reg} {
 iWrite p $data; iApply
 iRead spare_out 0; iApply
 iNote [iGetReadData spare_out -dec]
 iWrite p $data
}
iRead spare_out 0; iWrite value 9; iApply
iNote [iGetReadData spare_out -dec]; iNote [iGetStatus -clear]
"""
        self.assertEqual(self.notes(self.replay(MODEL, body)), [b"3", b"3", b"4"])

    def test_read_callback_generation_and_actual_unknown_history(self):
        icl, pdl = self.root / "source.icl", self.root / "source.pdl"
        icl.write_text(MODEL)
        pdl.write_text(PROCS + "iWrite spare 3; iRead observed 9; iApply")
        result = self.run_tool(icl, pdl, modeled=False)
        self.assertIn(b"iWrite spare 0b0011", result)
        self.assertIn(b"iRead q 0b1001", result)
        self.assertEqual(result, self.run_tool(icl, self.root / "result.pdl", "generated/result.pdl", modeled=False))
        observed = self.replay(MODEL, PROCS+'''
iWrite spare 3; iRead observed 9; iApply
iRead spare_out 0; iApply
iNote [iGetReadData spare_out -dec]; iNote [iGetReadData q -bin]
if {![catch {iGetStatus}]} {error {unknown endpoint became known}}
''')
        self.assertEqual(self.notes(observed), [b'3', b'0bxxxx'])

    def test_sparse_alias_masks_status_clear(self):
        body = PROCS + """iReset; iState spare 0
iWrite spare_inv 0b1100; iWrite value 9; iApply
iRead spare_out 0bxx01; iWrite value 6; iApply
iNote [iGetReadData spare_out -dec]; iNote [iGetMiscompares spare_out -bin]
iNote [iGetStatus -clear]; iNote [iGetStatus]
iState spare 2; iRead spare_out 0; iWrite value 3; iApply
iNote [iGetReadData spare_out -dec]; iNote [iGetStatus]
"""
        self.assertEqual(self.notes(self.replay(MODEL, body)),
                         [b"3", b"0b0010", b"1", b"0", b"2", b"1"])

    def test_selected_batch_body_reset_and_unselected_tail(self):
        source = MODEL.replace(" Alias low", " DataRegister other[3:0] {WriteCallBack Chains store <D> <R>;}\n Alias low")
        body = PROCS + """set ::calls 0
iProc store {data reg} {incr ::calls; if {$reg eq "value"} {iReset}; iWrite p $data}
iReset; iWrite spare 3; iWrite value 9; iWrite other 6; iApply
if {$::calls != 2} {error selected_batch}
iRead q 0; iApply; iNote [iGetReadData q -dec]
iWrite spare 7; iWrite value 15; iReset; iApply
if {$::calls != 2} {error canceled_tail}
"""
        self.assertEqual(self.notes(self.replay(source, body)), [b"6"])

    def test_scale_receipt_counts_and_fresh_generated_getters(self):
        output = self.root / "scale"
        result = subprocess.run(["python3", str(cli.ROOT / "tests/src/pattern/benchmark_mixed_callback_groups.py"),
                                 "--binary", str(cli.BIN), "--parent", str(cli.BIN),
                                 "--sanitized", str(cli.BIN), "--sizes", "64", "--repeats", "1",
                                 "--output", str(output)], capture_output=True, text=True, timeout=45)
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        rows = json.loads((output / "results.json").read_text())["runs"]
        self.assertEqual(len(rows), 11)
        for row in rows:
            invoked = row["mode"] in ("observed", "observer-off")
            self.assertEqual(row["apply_count"], 264 if invoked else 8)
            self.assertEqual(row["pio_frame_count"], 260 if invoked else 8)
            self.assertEqual(row["empty_apply_count"], 4 if invoked else 0)
            self.assertNotIn("csu_count", row)
            self.assertNotIn("shifted_bits", row)
            self.assertIn("expected_csu_count", row)
            self.assertIn("expected_shifted_bits", row)
            self.assertEqual(row["expected_csu_count"], 0)
            self.assertEqual(row["expected_shifted_bits"], 0)
            self.assertEqual(row["endpoint_writes"], 256 if invoked else 4)
            self.assertEqual(row["ordinary_spare_writes"], 8)
            self.assertEqual(row["declared_data_register_bits"], 256)
            self.assertEqual(row["primary_input_bits"], 8)
            self.assertEqual(row["scan_storage_bits"], 0)
            self.assertEqual(row["callback_postconditions"], 256 if invoked else 0)
            self.assertEqual(row["original_operand_bits"], 4 * (64 * 4 + 12) if invoked else 64)
            self.assertEqual(row["fresh_getters"], 0 if row["mode"] == "observer-off" else 4)

    def test_retained_guards(self):
        mixed = MODEL.replace(" Alias spare_inv", " Alias mixed[7:0] = value, p;\n Alias spare_inv")
        self.rejected("iWrite mixed 0x93; iApply", "caller", mixed)
        self.rejected("iWrite spare 3; iWrite value 9; iApply -together", "together")
        self.rejected("iMerge -begin; iWrite spare 3; iWrite value 9; iApply; iMerge -end", "Merge")
        self.rejected("iWrite spare 3; iWrite value 9; iApply", "callback", modeled=False,
                      extra=("--physical_pdl",))


if __name__ == "__main__":
    unittest.main()
