#!/usr/bin/env python3
"""Mixed ordinary/callback groups preserve caller goals and real frame histories."""
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

    def test_compatible_and_rejected_write_collisions(self):
        result = self.replay(MODEL, PROCS + "iWrite p 9; iWrite value 9; iApply; "
                             "iRead q 0; iApply; iNote [iGetReadData q -dec]")
        self.assertEqual(self.notes(result), [b"9"])
        for goal, data in (("p 3", "9"), ("p_inv 0b1100", "9"),
                           ("p[1:0] 3", "9"), ("p 3", "0bxxxx")):
            self.rejected("iWrite " + goal + "; iWrite value " + data + "; iApply", "caller")
        self.rejected("iProc store {data reg} {iState p 9}; "
                      "iWrite p 3; iWrite value 9; iApply", "caller")

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

    def test_retained_guards(self):
        mixed = MODEL.replace(" Alias spare_inv", " Alias mixed[7:0] = value, spare;\n Alias spare_inv")
        self.rejected("iWrite mixed 0x93; iApply", "alias", mixed)
        self.rejected("iWrite spare 3; iWrite value 9; iApply -together", "together")
        self.rejected("iMerge -begin; iWrite spare 3; iWrite value 9; iApply; iMerge -end", "Merge")
        self.rejected("iWrite spare 3; iWrite value 9; iApply", "callback", modeled=False,
                      extra=("--physical_pdl",))
        self.rejected("iWrite spare 3; iRead observed 9; iApply", "callback")


if __name__ == "__main__":
    unittest.main()
