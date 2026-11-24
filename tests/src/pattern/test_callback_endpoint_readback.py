#!/usr/bin/env python3
"""Actual modeled write-callback endpoint data, separate from logical requests."""
import unittest

import test_simultaneous_scan_chains as cli
import test_modeled_readback as observations
import test_callback_execution as callbacks

MODEL = callbacks.MODEL
PROCS = callbacks.PROCS.replace(' iNote "$reg=$data"\n', "")


class CallbackEndpointReadback(unittest.TestCase):
    setUp = cli.SimultaneousChains.setUp
    run_tool = observations.ModeledReadback.run_tool
    replay = cli.SimultaneousChains.replay

    def notes(self, output):
        return [line[6:] for line in output.splitlines()
                if line.startswith(b"iNote ") and b"modeled " not in line]

    def test_actual_endpoint_values_mismatches_and_clear(self):
        output = self.replay(MODEL, PROCS + """
iReset; iWrite value 9; iApply; iRead q 0; iApply
iNote [iGetReadData q -dec]; iNote [iGetMiscompares q -bin]
iNote [iGetStatus -clear]; iNote [iGetStatus]
""")
        self.assertEqual(self.notes(output), [b"9", b"0b1001", b"2", b"0"])

    def test_mixed_alias_and_reset_never_observe_logical_state(self):
        source = MODEL.replace(" Alias low", """
 DataOutPort shadow[3:0] {Source value;}
 Alias mixed[7:0] = value, p;
 Alias low""").replace("WriteCallBack Chains store <D> <R>;",
                      "WriteCallBack Chains store <D> <R>; ResetValue 0;")
        output = self.replay(source, """iPDLLevel 1
iReset; iState mixed 0b10011010 -LastWrittenValue
iRead q 0; iRead shadow 0; iApply
iNote [iGetReadData q -dec]; iNote [iGetReadData shadow -bin]
if {![catch {iGetStatus}]} {error {abstract storage produced known status}}
iReset; iState p 3; iRead q 0; iRead shadow 0; iApply
iNote [iGetReadData q -dec]; iNote [iGetReadData shadow -bin]
""")
        self.assertEqual(self.notes(output), [b"10", b"0bxxxx", b"3", b"0bxxxx"])

    def test_noop_callback_does_not_invent_requested_data(self):
        source = MODEL.replace(" Alias low", """
 DataOutPort shadow[3:0] {Source value;}
 Alias low""")
        body = """iPDLLevel 1
iProcsForModule Chains
iProc store {data reg} {}
iReset; iState p 3; iWrite value 9; iApply
iRead q 9; iApply
iNote [iGetReadData q -dec]; iNote [iGetStatus]
iRead shadow 9; iApply; iNote [iGetReadData shadow -bin]
if {![catch {iGetStatus}]} {error {postcondition became an observation}}
"""
        output = self.replay(source, body)
        self.assertEqual(self.notes(output), [b"3", b"2", b"0bxxxx"])


    def test_reads_inside_write_body_are_actual_endpoint_reads(self):
        body = PROCS.replace(" iWrite p $data", """ iWrite p $data; iApply
 iRead q 0; iApply
 iNote [iGetReadData q -dec]; iNote [iGetStatus -clear]""")
        output = self.replay(MODEL, body + "iReset; iWrite value 9; iApply")
        self.assertEqual(self.notes(output), [b"9", b"2"])

    def test_canceled_read_callback_in_write_body_remains_uninvoked(self):
        body = PROCS.replace(" iWrite p $data", """ iRead observed 0; iReset
 iWrite p $data; iApply; iRead q 0; iApply
 iNote [iGetReadData q -dec]""")
        output = self.replay(MODEL, body + "iWrite value 6; iApply")
        self.assertEqual(self.notes(output), [b"6"])

    def test_canceled_top_read_does_not_disable_write_observations(self):
        output = self.replay(MODEL, PROCS + """
iRead observed 0; iReset
iWrite value 9; iApply; iRead q 0; iApply
iNote [iGetReadData q -dec]
""")
        self.assertEqual(self.notes(output), [b"9"])

    def test_invoked_read_callback_retains_actual_history_across_reset(self):
        output = self.replay(MODEL, PROCS.replace(' iRead q $data',
            ' iReset; iState p 6; iRead q $data; iApply; iNote [iGetReadData q -dec]') +
            'iRead observed 9; iApply; iNote [iGetStatus]')
        self.assertEqual(self.notes(output), [b'6', b'4'])

    def test_sparse_inverted_requests_preserve_actual_histories(self):
        source = MODEL.replace(" Alias low", " Alias outinv[3:0] = ~q;\n Alias low")
        output = self.replay(source, PROCS + """
iReset; iState value 10; iState p 0; iWrite low 1; iApply
iRead q 0; iApply
iNote [iGetReadData q -bin]; iNote [iGetStatus -clear]
iRead q[0] 0; iApply
iNote [iGetReadData outinv -bin]; iNote [iGetStatus]
""")
        self.assertEqual(self.notes(output),
                         [b"0b0001", b"1", b"0b1110", b"1"])

    def test_manual_history_override_keeps_ordinary_semantics(self):
        source = cli.NETWORK.replace(
            "  SelectPort sel;",
            "  DataRegister value[3:0] {WriteCallBack Chains store <D>;}\n"
            "  SelectPort sel;")
        output = self.replay(source, """iPDLLevel 1
iProcsForModule Chains
iProc store {data} {iWrite a $data}
iReset; iWrite value 9; iApply; iRead a 0; iApply
iState a 15 -LastReadValue
iNote [iGetMiscompares a -bin]
iNote [iGetReadData a -dec]; iNote [iGetStatus]
""")
        self.assertEqual(self.notes(output), [b"0b1001", b"15", b"2"])

    def test_selectable_data_register_endpoint(self):
        source = """Module Chains {
 DataInPort data[3:0]; DataInPort enable;
 DataRegister wire[3:0] {WriteDataSource data; WriteEnSource enable; ResetValue 0;}
 DataOutPort q[3:0] {Source wire;}
 DataRegister value[3:0] {WriteCallBack Chains store <D>;}
}"""
        output = self.replay(source, """iPDLLevel 1
iProcsForModule Chains
iProc store {data} {iWrite wire $data}
iReset; iState enable 0; iWrite value 9; iApply
iRead wire 0; iApply
iNote [iGetReadData wire -dec]; iNote [iGetStatus -clear]
iRead q 9; iApply; iNote [iGetReadData q -dec]; iNote [iGetStatus]
""")
        self.assertEqual(self.notes(output), [b"9", b"2", b"9", b"0"])

    def test_serial_register_endpoint(self):
        source = cli.NETWORK.replace(
            "  SelectPort sel;",
            "  DataRegister value[3:0] {WriteCallBack Chains store <D>;}\n"
            "  SelectPort sel;")
        output = self.replay(source, """iPDLLevel 1
iProcsForModule Chains
iProc store {data} {iWrite a $data}
iReset; iWrite value 9; iApply; iRead a 0; iApply
iNote [iGetReadData a -dec]; iNote [iGetStatus]
""")
        self.assertEqual(self.notes(output), [b"9", b"2"])

    def test_nested_callback_namespaces_use_real_endpoints(self):
        source = """Module Leaf {
 DataRegister value[3:0] {WriteCallBack Chains::ops store <D> <R>;}
}
Module Chains {
 DataInPort p[3:0]; DataOutPort q[3:0] {Source p;}
 Instance leaf Of Leaf;
}"""
        body = """iPDLLevel 1
iProcsForModule Chains -iProcNameSpace ops
iProc store {data reg} {iWrite p $data}
iProcsForModule Chains
iWrite leaf.value 6; iApply; iRead q 0; iApply
iNote [iGetReadData q -dec]; iNote [iGetStatus]
"""
        output = self.replay(source, body)
        self.assertEqual(self.notes(output), [b"6", b"2"])

    def test_caught_write_failure_reset_and_retry(self):
        output = self.replay(MODEL, PROCS + """
iProc store {data reg} {
 iWrite p $data; iApply
 if {$data == 9} {error stopped}
}
if {![catch {iWrite value 9; iApply}]} {error {expected failure}}
iReset; iWrite value 6; iApply; iRead q 0; iApply
iNote [iGetReadData q -dec]; iNote [iGetStatus]
""")
        self.assertEqual(self.notes(output), [b"6", b"2"])

    def test_generated_prefix_and_split_files_do_not_observe_postcondition(self):
        icl, first, second = (self.root / n for n in
                              ("source.icl", "first.pdl", "second.pdl"))
        icl.write_text(MODEL.replace(" Alias low",
                                    " DataOutPort shadow[3:0] {Source value;}\n Alias low"))
        first.write_text(PROCS + "iWrite value 9; iApply\n")
        prefix = self.run_tool(icl, first)
        second.write_text("""iPDLLevel 1
iRead shadow 0; iRead q 0; iApply
iNote [iGetReadData shadow -bin]; iNote [iGetReadData q -dec]
if {![catch {iGetStatus}]} {error {postcondition was observed}}
""")
        expected = self.run_tool(icl, first, "split/result.pdl",
                                 extra=("--pdl", str(second)))
        self.assertEqual(self.notes(expected), [b"0bxxxx", b"9"])
        self.assertEqual(expected, self.run_tool(
            icl, self.root / "result.pdl", "prefix/result.pdl",
            extra=("--pdl", str(second))))
        self.assertIn(b"iState value", prefix)

    def test_unsupported_clock_history_in_write_body_stays_rejected(self):
        icl, pdl = self.root / "source.icl", self.root / "source.pdl"
        icl.write_text(MODEL)
        pdl.write_text(PROCS.replace(" iWrite p $data",
                                    " iRunLoop 1 -tck; iWrite p $data") +
                       "iWrite value 9; iApply; iRead q 0; iApply; iGetStatus")
        self.assertIn("histories", self.run_tool(icl, pdl, ok=False))

    def test_physical_callback_boundary_is_unchanged(self):
        icl, pdl = self.root / "source.icl", self.root / "source.pdl"
        icl.write_text(MODEL)
        pdl.write_text(PROCS + "iWrite value 9; iApply")
        self.assertIn("physical", self.run_tool(
            icl, pdl, ok=False, extra=("--physical_pdl",)))


    def test_local_reset_rewritten_source_replays_same_observations(self):
        import subprocess
        icl, first, second = (self.root / n for n in
                              ("source.icl", "first.pdl", "second.pdl"))
        icl.write_text(callbacks.RESET_MODEL)
        first.write_text(callbacks.RESET_PROCS +
                         "iReset; iCall request 9\n")
        second.write_text("""iApply; iRead child.wire 0; iApply
iNote [iGetReadData child.wire -dec]; iNote [iGetStatus]
""")
        command = [str(cli.BIN), "--icl", str(icl), "--pdl", str(first),
                   "--pdl", str(second), "--top_module", "Chains",
                   "--output", str(self.root / "result.pdl"), "--debug",
                   "--model_readback", "--local_reset", str(self.root / "reset.pdl")]
        result = subprocess.run(command, capture_output=True, text=True, timeout=60)
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        output = (self.root / "result.pdl").read_bytes()
        self.assertEqual(self.notes(output), [b"9", b"2"])
        self.assertEqual(output, self.run_tool(
            icl, self.root / "reset.pdl", "rewritten/result.pdl"))
        self.assertEqual(output, self.run_tool(
            self.root / "debug/Chains.icl", self.root / "reset.pdl",
            "flat-rewritten/result.pdl"))
        self.assertEqual(output, self.run_tool(
            icl, self.root / "result.pdl", "generated/result.pdl"))

    def test_endpoint_reservation_scope_is_preserved(self):
        output = self.replay(MODEL, PROCS.replace(
            " iWrite p $data", " iTake p; iWrite p $data; iApply; iRelease p") + """
iReset; iWrite value 9; iApply; iWrite p 6; iApply
iRead q 0; iApply; iNote [iGetReadData q -dec]; iNote [iGetStatus]
""")
        self.assertEqual(self.notes(output), [b"6", b"2"])

    def test_inverted_mixed_alias_masks_only_callback_bits(self):
        source = MODEL.replace(" Alias low", """
 DataOutPort shadow[3:0] {Source value;}
 Alias mixed[7:0] = ~value, ~p;
 Alias low""")
        output = self.replay(source, """iPDLLevel 1
iState mixed 0b10011010
iRead q 0; iRead shadow 0; iApply
iNote [iGetReadData q -dec]; iNote [iGetReadData shadow -bin]
""")
        self.assertEqual(self.notes(output), [b"5", b"0bxxxx"])


    def test_uninvoked_sibling_read_canceled_after_write_error(self):
        body = PROCS.replace(" iWrite p $data",
                             " iWrite p $data; iApply; error failed")
        output = self.replay(MODEL, body + """
iWrite value 9; iRead observed 9
if {![catch {iApply}]} {error {expected write error}}
iReset; iWrite p 5; iApply; iRead q 0; iApply
iNote [iGetReadData q -dec]
""")
        self.assertEqual(self.notes(output), [b"5"])

    def test_unused_callback_local_reset_does_not_break_other_observations(self):
        source = """Module Child {
 ResetPort rst;
 DataRegister value[3:0] {WriteCallBack Chains store <D>; ResetValue 9;}
}
Module Chains {
 DataInPort p[3:0]; DataInPort reset; DataOutPort q[3:0] {Source p;}
 ResetPort global;
 DataMux reset_gate SelectedBy reset {1'b0: global; 1'b1: 1'b1;}
 Instance c Of Child {InputPort rst = reset_gate;}
}"""
        output = self.replay(source, """iPDLLevel 1
iReset; iState reset 1; iWrite p 5; iApply; iRead q 0; iApply
iNote [iGetReadData q -bin]
""")
        self.assertEqual(self.notes(output), [b"0b0101"])

    def test_serial_frames_cannot_materialize_callback_reset_value(self):
        source = """Module Child {
 ResetPort rst;
 DataRegister value[3:0] {WriteCallBack Chains store <D>; ResetValue 9;}
 DataOutPort shadow[3:0] {Source value;}
}
""" + cli.NETWORK.replace("  SelectPort sel;", """
 ResetPort global; DataInPort reset;
 DataMux reset_gate SelectedBy reset {1'b0: global; 1'b1: 1'b1;}
 Instance c Of Child {InputPort rst = reset_gate;}
 DataOutPort shadow[3:0] {Source c.shadow;}
  SelectPort sel;""")
        output = self.replay(source, """iPDLLevel 1
iReset; iState reset 1; iWrite a 5; iApply
iState reset 0; iRead shadow 0; iApply
iNote [iGetReadData shadow -bin]
if {![catch {iGetStatus}]} {error {reset value became actual callback data}}
""")
        self.assertEqual(self.notes(output), [b"0bxxxx"])

    def test_generated_prefix_note_survives_local_reset_exactly_once(self):
        icl, prefix, body = (self.root / n for n in
                             ("source.icl", "prefix.pdl", "body.pdl"))
        icl.write_text(callbacks.RESET_MODEL)
        prefix.write_text("iPDLLevel 1\n")
        self.run_tool(icl, prefix, "prefix-result.pdl")
        body.write_text(callbacks.RESET_PROCS + """
iReset; iCall request 9; iApply; iRead child.wire 0; iApply
iNote [iGetReadData child.wire -dec]
""")
        output = self.run_tool(icl, self.root / "prefix-result.pdl",
                               extra=("--pdl", str(body), "--local_reset",
                                      str(self.root / "reset.pdl")))
        self.assertEqual(output.count(observations.LABEL), 1)
        self.assertEqual(output, self.run_tool(
            icl, self.root / "result.pdl", "generated/result.pdl"))


if __name__ == "__main__":
    unittest.main()
