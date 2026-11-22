#!/usr/bin/env python3
"""Explicit callbacks execute endpoint PDL once at Apply and replay flat."""
import unittest

import test_simultaneous_scan_chains as cli

MODEL = """Module Chains {
 DataInPort p[3:0]; DataOutPort q[3:0] {Source p;}
 DataRegister value[3:0] {WriteCallBack Chains store <D> <R>;}
 DataRegister observed[3:0] {ReadCallBack Chains load <D> <R>; ReadDataSource p;}
 Alias low[1:0] = value[1:0];
 Alias inv[1:0] = ~value[3:2];
}
"""
PROCS = """iPDLLevel 1
iProcsForModule Chains
iProc store {data reg} {
 iNote "$reg=$data"
 iWrite p $data
}
iProc load {data reg} {
 iNote "$reg=$data"
 iRead q $data
}
"""


class CallbackExecution(unittest.TestCase):
    setUp = cli.SimultaneousChains.setUp
    run_tool = cli.SimultaneousChains.run_tool
    replay = cli.SimultaneousChains.replay

    def notes(self, output):
        return [line[6:] for line in output.splitlines() if line.startswith(b"iNote ")]

    def rejected(self, body, diagnostic, source=MODEL):
        icl, pdl = self.root / "source.icl", self.root / "source.pdl"
        icl.write_text(source)
        pdl.write_text(PROCS + body)
        output = self.root / "result.pdl"
        output.write_bytes(b"retained artifact")
        message = self.run_tool(icl, pdl, ok=False)
        self.assertIn(diagnostic, message)
        self.assertEqual(output.read_bytes(), b"retained artifact")
        return message

    def test_write_tail_apply_real_endpoint_and_postcondition(self):
        output = self.replay(MODEL, PROCS + "iWrite value 9; iApply; iNote done")
        self.assertEqual(self.notes(output), [b"value=0b1001", b"done"])
        self.assertIn(b"iWrite p 0b1001", output)
        self.assertIn(b"iState value 0b1001", output)
        self.assertLess(output.index(b"iWrite p"), output.index(b"iState value"))
        self.assertNotIn(b"iWrite value", output)

    def test_read_uses_procedure_without_expected_state(self):
        output = self.replay(MODEL, PROCS +
                             "iState p 9; iRead observed 9; iApply")
        self.assertEqual(self.notes(output), [b"observed=0b1001"])
        self.assertIn(b"iRead q 0b1001", output)
        self.assertNotIn(b"iState observed", output)

    def test_latest_assignment_and_alias_inversions(self):
        output = self.replay(MODEL, PROCS + """
iWrite value 0; iWrite low 3; iWrite inv 0; iApply
iWrite value 5; iWrite value 9; iApply
""")
        self.assertEqual(self.notes(output), [b"value=0b1111", b"value=0b1001"])

    def test_reset_cancels_before_procedure_is_loaded(self):
        output = self.replay(MODEL, """iPDLLevel 1
iWrite value 5; iRead observed 0; iReset; iApply
""" + PROCS + "iWrite value 9; iApply")
        self.assertEqual(self.notes(output), [b"value=0b1001"])

    def test_explicit_body_apply_is_not_repeated(self):
        procs = PROCS.replace(" iWrite p $data", " iWrite p $data; iApply")
        output = self.replay(MODEL, procs + "iWrite value 9; iApply")
        self.assertEqual(self.notes(output), [b"value=0b1001"])
        self.assertEqual(output.count(b"iWrite p 0b1001"), 1)

    def test_partial_mask_preserves_unknown_argument_bits(self):
        output = self.replay(MODEL, PROCS + "iWrite low 2; iApply")
        self.assertEqual(self.notes(output), [b"value=0bxx10"])
        self.assertIn(b"iState value 0bxx10", output)

    def test_redefinition_loaded_between_files_and_once_only_value(self):
        icl, first, second = (self.root / name for name in
                              ("source.icl", "first.pdl", "second.pdl"))
        icl.write_text(MODEL)
        first.write_text("""iPDLLevel 1
set calls 0
proc word {} {global calls; incr calls; return 9}
iWrite value [word]
""")
        second.write_text(PROCS + """
iProc store {data reg} {iNote new; iWrite p $data}
iApply
if {$calls != 1} {error {value reevaluated}}
""")
        import subprocess
        result = subprocess.run([str(cli.BIN), "--icl", str(icl), "--pdl", str(first),
                                 "--pdl", str(second), "--top_module", "Chains",
                                 "--output", str(self.root / "result.pdl")],
                                text=True, capture_output=True, timeout=45)
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        self.assertEqual(self.notes((self.root / "result.pdl").read_bytes()), [b"new"])

    def test_subnamespace_and_literal_placeholder_arguments(self):
        source = MODEL.replace("Chains store <D> <R>",
                               '::Chains::api store <D> <R> "<D>" "<R>" '
                               '"a; [b] $c" 8\'b10x1')
        procs = """iPDLLevel 1
iProcsForModule Chains -iProcNameSpace api
iProc store {data reg d r literal number} {
 if {$d ne "<D>" || $r ne "<R>" || $literal ne {a; [b] $c}} {
  error {literal callback argument was evaluated}
 }
 if {$number ne "0b000010x1"} {error {numeric width or x lost}}
 iNote "$reg=$data"
 iWrite p $data
}
"""
        output = self.replay(source, procs + "iWrite value 9; iApply")
        self.assertEqual(self.notes(output), [b"value=0b1001"])

    def test_body_error_requires_reset_before_retry(self):
        body = """iPDLLevel 1
iProcsForModule Chains
iProc store {data reg} {iWrite p $data; error broken}
iWrite value 9
if {![catch {iApply}]} {error {callback did not fail}}
if {![catch {iApply} message] || [string first {iReset} $message] < 0} {
 error {failed callback queue was flushed}
}
iReset
iProc store {data reg} {iWrite p $data}
iWrite value 6; iApply
"""
        output = self.replay(MODEL, body)
        self.assertNotIn(b"iWrite p 0b1001", output)
        self.assertIn(b"iWrite p 0b0110", output)

    def test_direct_cycle_rejects_atomically(self):
        direct = PROCS.replace(" iWrite p $data", " iWrite value $data; iApply")
        icl, pdl = self.root / "source.icl", self.root / "source.pdl"
        icl.write_text(MODEL)
        pdl.write_text(direct + "iWrite value 9; iApply")
        output = self.root / "result.pdl"
        output.write_bytes(b"retained artifact")
        message = self.run_tool(icl, pdl, ok=False)
        self.assertIn("callback cycle", message)
        self.assertEqual(output.read_bytes(), b"retained artifact")

    def test_indirect_cycle_rejects_atomically(self):
        source = MODEL.replace(" Alias low", """
 DataRegister other[3:0] {WriteCallBack Chains relay <D> <R>;}
 Alias low""")
        procs = """iPDLLevel 1
iProcsForModule Chains
iProc store {data reg} {iWrite other $data; iApply}
iProc relay {data reg} {iWrite value $data; iApply}
"""
        icl, pdl = self.root / "source.icl", self.root / "source.pdl"
        icl.write_text(source)
        pdl.write_text(procs + "iWrite value 9; iApply")
        message = self.run_tool(icl, pdl, ok=False)
        self.assertIn("callback cycle", message)

    def test_local_module_binding_and_real_serial_endpoint(self):
        source = """Module Leaf {
 Parameter Tag = "leaf";
 ScanInPort si; SelectPort sel; CaptureEnPort ce; ShiftEnPort se;
 UpdateEnPort ue; TCKPort tck; ResetPort reset;
 ScanRegister wire[3:0] {ScanInSource si; CaptureSource wire; ResetValue 0;}
 ScanOutPort so {Source wire[0];}
 DataRegister value[3:0] {WriteCallBack Leaf store <D> <R>;}
}
Module Chains {
 ScanInPort si; ScanOutPort so {Source child.so;}
 SelectPort sel; CaptureEnPort ce; ShiftEnPort se; UpdateEnPort ue;
 TCKPort tck; ResetPort reset;
 Instance child Of Leaf {InputPort si = si; InputPort sel = sel;
  InputPort ce = ce; InputPort se = se; InputPort ue = ue;
  InputPort tck = tck; InputPort reset = reset;}
}
"""
        procs = """iPDLLevel 1
iProcsForModule Leaf
iProc store {data reg} {
 if {$Tag ne "leaf" || $reg ne "value"} {error {wrong local callback instance}}
 iNote "$reg=$data"; iWrite wire $data
}
iProcsForModule Chains
iReset; iWrite child.value 9; iApply
"""
        output = self.replay(source, procs)
        self.assertEqual(self.notes(output), [b"value=0b1001"])
        self.assertIn(b"iWrite child.wire 0b1001", output)
        self.assertIn(b"iState child.value 0b1001", output)

    def test_mixed_and_together_reject_before_invocation(self):
        for body in ("iWrite value 9; iWrite p 3; iApply",
                     "iWrite p 3; iWrite value 9; iApply",
                     "iWrite value 9; iApply -together"):
            with self.subTest(body=body):
                message = self.rejected(body, "callback")
                self.assertNotIn("execution is not implemented", message)

    def test_nested_instance_ancestor_parameters_and_relative_name(self):
        source = """Module Leaf {
 Parameter owner = "Chains";
 DataRegister value[3:0] {WriteCallBack $owner::api store <D> <R>;}
}
Module Chains {
 Parameter Tag = "root";
 DataInPort p[3:0]; DataOutPort q[3:0] {Source p;}
 Instance child Of Leaf;
}
"""
        procs = """iPDLLevel 1
iProcsForModule Chains -iProcNameSpace api
iProc store {data reg} {
 if {$Tag ne "root" || $reg ne "child.value"} {error {wrong ancestor instance}}
 iNote "$reg=$data"; iWrite p $data
}
"""
        output = self.replay(source, procs + "iWrite child.value 9; iApply")
        self.assertEqual(self.notes(output), [b"child.value=0b1001"])

    def test_model_and_physical_modes_reject_callbacks_atomically(self):
        import subprocess
        icl, pdl = self.root / "source.icl", self.root / "source.pdl"
        icl.write_text(MODEL)
        pdl.write_text(PROCS + "iWrite value 9; iApply")
        for option in ("--model_readback", "--physical_pdl"):
            output = self.root / "result.pdl"
            output.write_bytes(b"retained artifact")
            result = subprocess.run(
                [str(cli.BIN), "--icl", str(icl), "--pdl", str(pdl),
                 "--top_module", "Chains", "--output", str(output), option],
                text=True, capture_output=True, timeout=45)
            self.assertNotEqual(result.returncode, 0)
            self.assertIn("callback", result.stdout + result.stderr)
            self.assertEqual(output.read_bytes(), b"retained artifact")

    def test_canceled_requests_preserve_model_mode(self):
        import subprocess
        icl, pdl = self.root / "source.icl", self.root / "source.pdl"
        icl.write_text(MODEL)
        pdl.write_text("""iPDLLevel 1
iWrite value 9; iRead observed 0; iReset
iState p 3; iRead q 3; iApply; iNote [iGetReadData q -dec]
""")
        result = subprocess.run(
            [str(cli.BIN), "--icl", str(icl), "--pdl", str(pdl),
             "--top_module", "Chains", "--output", str(self.root / "result.pdl"),
             "--model_readback"], text=True, capture_output=True, timeout=45)
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        self.assertIn(b"iNote 3", (self.root / "result.pdl").read_bytes())

    def test_canceled_callback_keeps_physical_mode_working(self):
        import subprocess
        fixture = cli.ROOT / "tests/cases/ours/pattern_generation/raw_tap_ir"
        source = (fixture / "model.icl").read_text().replace(
            "Module RawIrTop {", "Module RawIrTop {DataRegister callback_value[3:0] {"
            "WriteCallBack RawIrTop store <D>;}")
        icl, pdl = self.root / "source.icl", self.root / "source.pdl"
        icl.write_text(source)
        pdl.write_text("""iPDLLevel 1
iWrite callback_value 9; iReset
iWrite tap.INSTRUCTION 3; iApply; iWrite core.r 9; iApply
""")
        result = subprocess.run(
            [str(cli.BIN), "--icl", str(icl), "--bsdl", "RawIrProbe",
             str(fixture / "model.bsdl"), "--pdl", str(pdl),
             "--top_module", "RawIrTop", "--output", str(self.root / "result.pdl"),
             "--physical_pdl"], text=True, capture_output=True, timeout=45)
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)

    def test_logical_callback_ownership_is_explicitly_unsupported(self):
        for request in ("iTake value", "iTake", "iRelease value"):
            with self.subTest(request=request):
                self.rejected(request, "callback storage")

    def test_endpoint_resource_guard_is_retained(self):
        output = self.replay(
            MODEL, PROCS + "iTake p; iWrite p 0; iApply; iRead observed 9; iApply")
        self.assertIn(b"iRead q 0b1001", output)
        self.assertIn(b"iWrite p 0b0000", output)
        self.assertNotIn(b"iWrite p 0b1001", output)

    def test_all_unknown_read_is_still_invoked(self):
        output = self.replay(MODEL, PROCS + "iRead observed; iApply")
        self.assertEqual(self.notes(output), [b"observed=0bxxxx"])

    def test_callback_inside_caller_merge_is_rejected(self):
        self.rejected("""iProc request {} {iWrite value 9; iApply}
iMerge -begin; iCall request; iMerge -end
""", "inside iMerge")

    def test_callback_body_can_merge_ordinary_endpoint_calls(self):
        procs = PROCS + """
iProc endpoint {data} {iWrite p $data; iApply}
iProc store {data reg} {
 iMerge -begin; iCall endpoint $data; iMerge -end
}
"""
        output = self.replay(MODEL, procs + "iWrite value 9; iApply")
        self.assertIn(b"iWrite p 0b1001", output)
        self.assertIn(b"iState value 0b1001", output)

    def test_ascending_callback_mask_follows_declaration_order(self):
        source = MODEL.replace("[3:0]", "[0:3]")
        output = self.replay(source, PROCS + "iWrite low 2; iApply")
        self.assertEqual(self.notes(output), [b"value=0b01xx"])
        self.assertIn(b"iState value 0b01xx", output)

    def test_all_procedures_preflight_before_first_effect(self):
        source = MODEL.replace(" Alias low", """
 DataInPort s[3:0];
 DataRegister other[3:0] {WriteCallBack Chains missing <D> <R>;}
 Alias low""")
        body = PROCS + """
set calls 0
iProc store {data reg} {global calls; incr calls; iWrite p $data}
iWrite value 9; iWrite other 6
if {![catch {iApply} message] || [string first {undefined iProc} $message] < 0} {
 error {missing second callback was not detected}
}
if {$calls != 0} {error {first callback ran before complete preflight}}
iProc missing {data reg} {global calls; incr calls; iWrite s $data}
iApply; iNote $calls
"""
        output = self.replay(source, body)
        self.assertEqual(self.notes(output), [b"2"])

    def test_wrong_ancestor_namespace_rejects(self):
        self.rejected("iWrite value 9; iApply", "callback namespace",
                      MODEL.replace("Chains store", "Missing store"))


if __name__ == "__main__":
    unittest.main()
