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


RESET_MODEL = """Module Leaf {
 ScanInPort si; SelectPort sel; CaptureEnPort ce; ShiftEnPort se;
 UpdateEnPort ue; TCKPort tck; ResetPort reset;
 ScanRegister wire[3:0] {ScanInSource si; CaptureSource wire; ResetValue 0;}
 ScanOutPort so {Source wire[0];}
 DataRegister value[3:0] {WriteCallBack Chains store <D> <R>;}
}
Module Chains {
 ScanInPort si; ScanOutPort so {Source child.so;}
 SelectPort sel; CaptureEnPort ce; ShiftEnPort se; UpdateEnPort ue; TCKPort tck;
 DataInPort p; ResetPort global;
 ScanRegister rst {ScanInSource si; CaptureSource rst; ResetValue 1;}
 DataMux reset_gate SelectedBy rst {1'b0: global; 1'b1: 1'b1;}
 Instance child Of Leaf {InputPort si = rst; InputPort sel = sel;
  InputPort ce = ce; InputPort se = se; InputPort ue = ue;
  InputPort tck = tck; InputPort reset = reset_gate;}
 ScanInterface client {Port si; Port so; Port sel; Port ce; Port se; Port ue; Port tck; Port global;}
}
"""
STORE_DEF = """iProc store {data reg} {
 iWrite p 1; iApply
 iWrite child.wire $data
}
"""
REQUEST_DEF = "iProc request {data} {iWrite child.value $data}\n"
RESET_PROCS = "iPDLLevel 1\niProcsForModule Chains\n" + STORE_DEF + REQUEST_DEF


class CallbackExecution(unittest.TestCase):
    setUp = cli.SimultaneousChains.setUp
    run_tool = cli.SimultaneousChains.run_tool
    replay = cli.SimultaneousChains.replay

    def notes(self, output):
        return [line[6:] for line in output.splitlines() if line.startswith(b"iNote ")]

    def reset_replay(self, body, source=RESET_MODEL, second=""):
        import subprocess
        icl, pdl = self.root / "source.icl", self.root / "source.pdl"
        icl.write_text(source)
        pdl.write_text(RESET_PROCS + body)
        inputs = [pdl]
        if second:
            other = self.root / "second.pdl"
            other.write_text(second)
            inputs.append(other)

        def lower(model, output, rewritten):
            command = [str(cli.BIN), "--icl", str(model), "--top_module", "Chains",
                       "--output", str(self.root / output), "--debug",
                       "--local_reset", str(self.root / rewritten)]
            for path in inputs:
                command += ["--pdl", str(path)]
            result = subprocess.run(command, text=True, capture_output=True, timeout=60)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            return (self.root / output).read_bytes()

        output = lower(icl, "result.pdl", "reset.pdl")
        rewritten = (self.root / "reset.pdl").read_text()
        self.assertIn(STORE_DEF, rewritten)
        self.assertIn(REQUEST_DEF, rewritten)
        self.assertEqual(output, lower(self.root / "debug/Chains.icl",
                                       "flat/result.pdl", "flat/reset.pdl"))
        self.assertEqual(output, self.run_tool(
            icl, self.root / "result.pdl", "generated/result.pdl"))
        self.assertEqual(output, self.run_tool(
            icl, self.root / "reset.pdl", "rewritten/result.pdl"))
        self.assertEqual(output, self.run_tool(
            self.root / "debug/Chains.icl", self.root / "reset.pdl",
            "flat-rewritten/result.pdl"))
        return output, rewritten

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

    def test_conflicting_mixed_and_together_rejected(self):
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
        for option, request in (("--model_readback", "iRead observed 9; iApply"),
                                ("--physical_pdl", "iWrite value 9; iApply")):
            pdl.write_text(PROCS + request)
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

    def test_reset_release_spans_multiple_callback_applies(self):
        output, _ = self.reset_replay("iReset; iWrite child.value 9; iApply")
        self.assertIn(b"iWrite rst 0b0", output)
        self.assertIn(b"iWrite child.wire 0b1001", output)

    def test_returned_call_anchor_covers_callback_expansion(self):
        output, _ = self.reset_replay("iReset; iCall request 9; iApply")
        self.assertIn(b"iState child.value 0b1001", output)

    def test_prefixed_callback_reset_rewrite(self):
        output, _ = self.reset_replay(
            "iReset; iPrefix child; iWrite value 9; iApply; iPrefix -")
        self.assertIn(b"iWrite rst 0b0", output)

    def test_repeated_callback_reset_site_and_once_only_arguments(self):
        output, rewritten = self.reset_replay("""iReset
set calls 0
proc word {} {global calls; incr calls; return 9}
foreach unused {a b} {iCall request [word]; iApply}
if {$calls != 2} {error {callback arguments reevaluated}}
""")
        self.assertEqual(output.count(b"iState child.value 0b1001"), 2)
        self.assertIn("proc word", rewritten)

    def test_split_file_callback_reset_queue(self):
        output, _ = self.reset_replay("iReset; iCall request 9", second="iApply")
        self.assertIn(b"iWrite child.wire 0b1001", output)

    def test_reset_cancels_callback_before_endpoint_release(self):
        output, _ = self.reset_replay(
            "iReset; iCall request 9; iReset; iWrite child.wire 6; iApply")
        self.assertNotIn(b"iWrite p", output)
        self.assertNotIn(b"iState child.value", output)
        self.assertIn(b"iWrite child.wire 0b0110", output)

    def test_caught_callback_failure_reset_replay(self):
        output, rewritten = self.reset_replay("""iProc store {data reg} {
 iWrite p 1; iApply
 iWrite child.wire $data
 if {$data eq "0b1001"} {error broken}
}
iReset; iCall request 9
if {![catch {iApply}]} {error {callback did not fail}}
iReset; iCall request 6; iApply
""")
        self.assertNotIn(b"iWrite child.wire 0b1001", output)
        self.assertIn(b"iWrite child.wire 0b0110", output)
        self.assertIn("if {$data eq", rewritten)

    def test_read_callback_reset_replay(self):
        source = RESET_MODEL.replace(
            "WriteCallBack Chains store <D> <R>;",
            "ReadCallBack Chains load <D> <R>; ReadDataSource wire;")
        output, _ = self.reset_replay("""iProc load {data reg} {
 iWrite p 1; iApply
 iRead child.wire $data
}
iReset; iRead child.value 9; iApply
""", source=source)
        self.assertIn(b"iRead child.wire 0b1001", output)
        self.assertNotIn(b"iState child.value", output)

    def test_callback_arity_is_preflighted_before_any_effect(self):
        source = MODEL.replace(" Alias low", """
 DataRegister other[3:0] {WriteCallBack Chains bad <D> <R>;}
 Alias low""")
        body = PROCS + """
set calls 0
iProc store {d r} {global calls; incr calls; iWrite p $d}
iProc bad {d} {error invoked}
iWrite value 9; iWrite other 6
if {![catch {iApply} message] || [string first {argument} $message] < 0} {
 error {bad arity not detected}
}
if {$calls != 0} {error {callback ran before arity preflight}}
iProc bad {d {r default}} {global calls; incr calls; iWrite p $d}
iApply
if {$calls != 2} {error {preflight retry lost original queue}}
iProc bad {args} {global calls; incr calls; iWrite p [lindex $args 0]}
iWrite other 1; iApply
if {$calls != 3} {error {variadic callback arity rejected}}
"""
        self.replay(source, body)

    def test_access_together_callback_alias_is_rejected_before_effects(self):
        source = MODEL.replace(" Alias low", """
 DataRegister other[3:0] {WriteCallBack Chains store <D> <R>;}
 Alias pair[7:0] = value, other {AccessTogether;}
 Alias low""")
        for target in ("pair", "pair[7:4]"):
            with self.subTest(target=target):
                self.rejected("iWrite " + target + " 9; iApply",
                              "AccessTogether", source)

    def test_unclosed_callback_merge_recovers_after_reset(self):
        for before_error, committed in (
                ("", False), ("iCall endpoint 9;", True),
                ("iCall unfinished 9;", False)):
            with self.subTest(before_error=before_error):
                body = PROCS + """
iProc endpoint {data} {iWrite p $data; iApply}
iProc unfinished {data} {iWrite p $data; error broken}
iProc store {data reg} {iMerge -begin; """ + before_error + """ error broken}
iWrite value 9
if {![catch {iApply}]} {error {callback did not fail}}
iReset
iProc store {data reg} {iWrite p $data}
iWrite value 6; iApply
"""
                output = self.replay(MODEL, body)
                self.assertIn(b"iWrite p 0b0110", output)
                self.assertEqual(b"iWrite p 0b1001" in output, committed)

    def test_qualified_namespace_beats_shorter_ancestor_name(self):
        source = """Module A {
 DataRegister value[3:0] {WriteCallBack A::Top::api store <D> <R>;}
}
NameSpace A;
Module Top {Instance child Of ::A;}
NameSpace;
Module Chains {Instance outer Of A::Top;}
"""
        pdl = """iPDLLevel 1
iProcsForModule A::Top -iProcNameSpace api
iProc store {data reg} {iNote "$reg=$data"}
iWrite outer.child.value 9; iApply
"""
        output = self.replay(source, pdl)
        self.assertEqual(self.notes(output), [b"child.value=0b1001"])

    def test_implicit_callback_tail_has_bounded_depth_and_reset_recovery(self):
        source = "Module Chains {DataInPort p;" + "".join(
            "DataRegister r%d {WriteCallBack Chains store <D> <R>;}" % i
            for i in range(160)) + "}"
        pdl = """iPDLLevel 1
iProcsForModule Chains
iProc store {data reg} {
 set next [expr {[string range $reg 1 end] + 1}]
 if {$next < 160} {iWrite r$next $data} else {iWrite p $data}
}
iWrite r0 1
if {![catch {iApply} message] || [string first {recursion limit} $message] < 0} {
 error {implicit callback chain escaped recursion limit}
}
iReset
iProc store {data reg} {iWrite p $data}
iWrite r0 0; iApply
"""
        output = self.replay(source, pdl)
        self.assertIn(b"iWrite p 0b0", output)
        self.assertNotIn(b"iWrite p 0b1", output)

    def test_callback_inner_reset_cannot_expose_modeled_endpoint_histories(self):
        import subprocess
        icl, pdl = self.root / "source.icl", self.root / "source.pdl"
        icl.write_text(MODEL)
        pdl.write_text(PROCS + """
iProc load {data reg} {iReset; iWrite p 0; iApply; iRead q $data; iApply}
iRead observed 9; iApply; iGetReadData q
""")
        output = self.root / "result.pdl"
        output.write_bytes(b"retained artifact")
        result = subprocess.run(
            [str(cli.BIN), "--icl", str(icl), "--pdl", str(pdl),
             "--top_module", "Chains", "--output", str(output), "--model_readback"],
            text=True, capture_output=True, timeout=45)
        self.assertNotEqual(result.returncode, 0)
        self.assertIn("modeled readback excludes callback histories",
                      result.stdout + result.stderr)
        self.assertEqual(output.read_bytes(), b"retained artifact")

    def test_failed_open_merge_reset_source_replay(self):
        output, _ = self.reset_replay("""iProc store {data reg} {
 iMerge -begin; error broken
}
iReset; iCall request 9
if {![catch {iApply}]} {error {callback did not fail}}
iReset
iProc store {data reg} {iWrite p 1; iApply; iWrite child.wire $data}
iCall request 6; iApply
""")
        self.assertIn(b"iWrite child.wire 0b0110", output)
        self.assertNotIn(b"iWrite child.wire 0b1001", output)

    def test_wrong_ancestor_namespace_rejects(self):
        self.rejected("iWrite value 9; iApply", "callback namespace",
                      MODEL.replace("Chains store", "Missing store"))


if __name__ == "__main__":
    unittest.main()
