#!/usr/bin/env python3
"""Automatic preparation completes each already active opaque leaf natively."""
import unittest

import test_modeled_child_prefix as helpers
from benchmark_dynamic_opaque_routes import model
from test_dynamic_opaque_routes import mixed


def nested():
    leaf,root = model(2,8).split('Module Chains {',1)
    branch = '''Module Branch {
 ScanInPort si; SelectPort sel; CaptureEnPort ce; ShiftEnPort se;
 UpdateEnPort ue; TCKPort tck; ResetPort reset;
 Instance leaf Of Leaf {InputPort si = si; InputPort sel = enable;
  InputPort ce = ce; InputPort se = se; InputPort ue = ue;
  InputPort tck = tck; InputPort reset = reset;}
 ScanMux route SelectedBy enable {1'b0: si; 1'b1: leaf.so;}
 ScanRegister enable {ScanInSource route; CaptureSource enable; ResetValue 0;}
 ScanOutPort so {Source enable;}
 ScanInterface local {Port si; Port so; Port sel; Port ce; Port se; Port ue;
                      Port tck; Port reset; DefaultLoadValue 1'b0;}
}
'''
    return leaf+branch+'Module Chains {'+root.replace('Instance child1 Of Leaf','Instance child1 Of Branch')


class OpaqueCrossingPreparation(unittest.TestCase):
    setUp = helpers.ModeledChildPrefix.setUp
    run_tool = helpers.ModeledChildPrefix.run_tool
    notes = helpers.ModeledChildPrefix.notes
    continued = helpers.ModeledChildPrefix.continued

    def crossing(self, stable):
        first = '''iReset
iScan client 10 -si 0b1011010011 -so 0bx; iApply
'''
        if stable:
            first += 'iScan child0.local 8 -si 165 -so 0bx -stable; iApply\n'
        last = 'iScan child1.local 8 -si 60 -so 0bx; iApply\n'
        suffix = '''iNote [iGetReadData child1.local -bin]; iNote [iGetMiscompares child1.local -bin]
iNote [iGetReadData client -bin]; iNote [iGetStatus]
'''
        expected = [b'0bxxxxxxxx',b'0b00000000',b'0b0000000000',b'0']
        if stable:
            suffix += 'iNote [iGetReadData child0.local -bin]; iNote [iGetMiscompares child0.local -bin]\n'
            expected += [b'0bxxxxxxxx',b'0b00000000']
        else:
            suffix += 'if {![catch {iGetReadData child0.local}]} {error {implicit child history invented}}\n'
        icl,pdl = self.root/'source.icl',self.root/'manual.pdl'
        icl.write_text(model(2,8))
        pdl.write_text('iPDLLevel 1\niProcsForModule Chains\n'+first+
                       'iWrite enable1 1; iApply\n'+last+suffix)
        control = self.run_tool(icl,pdl,'manual-result.pdl')
        generated,reference = self.continued(model(2,8),first+last,suffix,expected)
        self.assertEqual(reference,control)
        self.assertEqual(generated.count(b'\niApply\n'),4 if stable else 3)
        self.assertEqual(sum(line.startswith(b'iScan ') for line in generated.splitlines()),3 if stable else 2)
        self.assertNotIn(b'__opaque_scan_state',generated)

    def test_reset_default_completes_active_leaf_without_inventing_history(self):
        self.crossing(False)

    def test_stable_active_leaf_keeps_prior_payload_during_preparation(self):
        self.crossing(True)

    def test_nonstable_active_leaf_keeps_unknown_length_rejection_and_output(self):
        icl,pdl,out = (self.root/n for n in ('source.icl','source.pdl','protected.pdl'))
        icl.write_text(model(2,8))
        pdl.write_text('''iPDLLevel 1\niProcsForModule Chains\niReset
iScan client 10 -si 0b1011010011 -so 0bx; iApply
iScan child0.local 8 -si 165 -so 0bx; iApply
iScan child1.local 8 -si 60 -so 0bx; iApply
''')
        out.write_bytes(b'keep existing output\n')
        self.assertIn('opaque scan chain length is unknown',
                      self.run_tool(icl,pdl,'protected.pdl',ok=False))
        self.assertEqual(out.read_bytes(),b'keep existing output\n')

    def test_multiple_stable_active_leaves_complete_in_one_opening(self):
        generated,_ = self.continued(model(3,8),'''iReset
iScan client 11 -si 0b10111010011 -so 0bx; iApply
iScan child0.local 8 -si 165 -so 0bx -stable
iScan child1.local 8 -si 60 -so 0bx -stable; iApply
iScan child2.local 8 -si 90 -so 0bx; iApply
''','''iNote [iGetReadData child0.local -bin]; iNote [iGetReadData child1.local -bin]
iNote [iGetReadData child2.local -bin]; iNote [iGetStatus]
''',[b'0bxxxxxxxx',b'0bxxxxxxxx',b'0bxxxxxxxx',b'0'])
        self.assertEqual(generated.count(b'\niApply\n'),4)
        self.assertNotIn(b'__opaque_scan_state',generated)

    def test_modeled_sibling_prepares_through_opaque_without_losing_histories(self):
        generated,_ = self.continued(mixed(),'''iReset
iScan client 10 -si 0b1011010011 -so 0bx; iApply
iScan child0.local 8 -si 165 -so 0bx -stable; iApply
iScan child1.local 8 -si 60 -so 0; iApply
''','''iNote [iGetReadData child0.local -bin]; iNote [iGetMiscompares child0.local -bin]
iNote [iGetReadData child1.local -bin]; iNote [iGetMiscompares child1.local -bin]
iNote [iGetReadData client -bin]; iNote [iGetStatus]
''',[b'0bxxxxxxxx',b'0b00000000',b'0b00000000',b'0b00000000',b'0b0000000000',b'0'])
        self.assertIn(b'modeled dynamic child raw history client child1.local 0b00000000',generated)

    def test_inverted_active_route_maps_care_bits_and_preserves_unknown_status(self):
        source = model(2,8).replace("1'b1: child0.so;","1'b1: ~child0.so;")
        generated,_ = self.continued(source,'''iReset
iScan client 10 -si 0b1011010011 -so 0bx; iApply
iScan child0.local 8 -si 165 -so 0b10xxxx01 -stable; iApply
iScan child1.local 8 -si 60 -so 0b01xxxx10; iApply
''','''iNote [iGetReadData child0.local -bin]; iNote [iGetReadData child1.local -bin]
iNote [iGetMiscompares child0.local -bin]; iNote [iGetMiscompares child1.local -bin]
if {![catch {iGetStatus -clear}]} {error {unknown status returned}}
if {![catch {iGetStatus}]} {error {unknown status cleared}}
iNote unknown
''',[b'0bxxxxxxxx',b'0bxxxxxxxx',b'0bxx0000xx',b'0bxx0000xx',b'unknown'])
        self.assertIn(b'-so 0bxxx01xxxx10xxxxxxx -stable',generated)

    def test_held_aliases_keep_opaque_queue_and_actual_route_identity(self):
        source = model(2,8).replace(' ScanRegister tail',
            ' Alias held0 = enable0; Alias held1 = enable1;\n ScanRegister tail')
        for i in range(2):
            source = source.replace('SelectedBy enable%d'%i,'SelectedBy held%d'%i)
            source = source.replace('InputPort sel = enable%d;'%i,'InputPort sel = held%d;'%i)
        self.continued(source,'''iReset
iScan client 10 -si 0b1011010011 -so 0bx; iApply
iScan child0.local 8 -si 165 -so 0bx -stable; iApply
iScan child1.local 8 -si 60 -so 0bx; iApply
''','''iNote [iGetReadData child0.local -bin]; iNote [iGetReadData child1.local -bin]
iNote [iGetStatus]
''',[b'0bxxxxxxxx',b'0bxxxxxxxx',b'0'])

    def test_default_regeneration_retains_both_actual_history_intents(self):
        icl,pdl,tail = (self.root/n for n in ('source.icl','source.pdl','tail.pdl'))
        icl.write_text(model(2,8))
        pdl.write_text('''iPDLLevel 1\niProcsForModule Chains\niReset
iScan client 10 -si 0b1011010011 -so 0bx; iApply
iScan child0.local 8 -si 165 -so 0bx -stable; iApply
iScan child1.local 8 -si 60 -so 0bx; iApply
''')
        prefix = self.run_tool(icl,pdl,'prefix.pdl',modeled=False)
        self.assertEqual(self.run_tool(icl,self.root/'prefix.pdl','again.pdl',modeled=False),prefix)
        tail.write_text('iPDLLevel 1\niGetReadData child1.local\n')
        self.assertIn('requires hardware readback',self.run_tool(icl,self.root/'again.pdl',
                      'missing.pdl',modeled=False,ok=False,extra=('--pdl',str(tail))))
        tail.write_text('''iPDLLevel 1
iNote [iGetReadData child0.local -bin]; iNote [iGetReadData child1.local -bin]
iNote [iGetStatus -clear]; iNote [iGetStatus]
''')
        result = self.run_tool(icl,self.root/'again.pdl','observed.pdl',extra=('--pdl',str(tail)))
        self.assertEqual(self.notes(result),[b'0bxxxxxxxx',b'0bxxxxxxxx',b'0',b'0'])

    def test_local_reset_preserves_namespaced_crossing_history_getters(self):
        icl,pdl,tail = (self.root/n for n in ('source.icl','source.pdl','tail.pdl'))
        icl.write_text(model(2,8))
        pdl.write_text('''iPDLLevel 1\niProcsForModule Chains\niReset
iScan client 10 -si 0b1011010011 -so 0bx; iApply
iScan child0.local 8 -si 165 -so 0bx -stable; iApply
iScan child1.local 8 -si 60 -so 0bx; iApply
''')
        self.run_tool(icl,pdl,'prefix.pdl')
        definition = 'iProc result {} {iNote [iGetReadData child0.local -bin]; iNote [iGetReadData child1.local -bin]}'
        tail.write_text('iPDLLevel 1\niProcsForModule Chains -iProcNameSpace ops\n'+definition+
                        '\niCall ops::result; iNote [iGetStatus]\n')
        reference = self.run_tool(icl,self.root/'prefix.pdl','reference.pdl',extra=('--pdl',str(tail)))
        self.assertEqual(self.notes(reference),[b'0bxxxxxxxx',b'0bxxxxxxxx',b'0'])
        for source_icl in (icl,self.root/'debug/Chains.icl'):
            reset = self.root/'reset.pdl'
            result = self.run_tool(source_icl,self.root/'prefix.pdl','reset/result.pdl',
                extra=('--pdl',str(tail),'--local_reset',str(reset)))
            self.assertEqual(result,reference)
            self.assertIn(definition,reset.read_text())
            self.assertEqual(self.run_tool(source_icl,reset,'reset/replayed.pdl'),reference)

    def test_repeated_closure_requires_fresh_stable_knowledge_before_reopening(self):
        generated,_ = self.continued(model(2,8),'''iReset
iScan client 10 -si 0b1011010011 -so 0bx; iApply
iScan child0.local 8 -si 165 -so 0bx -stable; iApply
iScan child1.local 8 -si 60 -so 0bx -stable; iApply
iScan client 26 -si 0b10100000000100000000010011 -so 0bx; iApply
iScan child0.local 8 -si 60 -so 0bx -stable; iApply
iScan child1.local 8 -si 90 -so 0bx; iApply
''','''iNote [iGetReadData child0.local -bin]; iNote [iGetReadData child1.local -bin]
iNote [iGetReadData client -bin]; iNote [iGetStatus]
''',[b'0bxxxxxxxx',b'0bxxxxxxxx',b'0b101xxxxxxxx1xxxxxxxx110011',b'0'])
        self.assertEqual(generated.count(b'\niApply\n'),8)

    def test_locked_closed_selector_stays_rejected_with_existing_output(self):
        icl,pdl,out = (self.root/n for n in ('source.icl','source.pdl','protected.pdl'))
        icl.write_text(model(2,8))
        pdl.write_text('''iPDLLevel 1\niProcsForModule Chains\niReset
iScan client 10 -si 0b1011010011 -so 0bx; iApply
iScan child0.local 8 -si 165 -so 0bx -stable; iApply
iTake enable1; iState enable1 0
iScan child1.local 8 -si 60 -so 0bx; iApply
''')
        out.write_bytes(b'keep existing output\n')
        self.assertIn('unreserved parent',self.run_tool(icl,pdl,'protected.pdl',ok=False))
        self.assertEqual(out.read_bytes(),b'keep existing output\n')

    def test_nested_opening_completes_active_opaque_leaf_before_each_csu(self):
        first = '''iReset
iScan client 10 -si 0b1011010011 -so 0bx; iApply
iScan child0.local 8 -si 165 -so 0bx -stable; iApply
'''
        requested = 'iScan child1.leaf.local 8 -si 60 -so 0bx; iApply\n'
        suffix = '''iNote [iGetReadData child0.local -bin]; iNote [iGetReadData child1.leaf.local -bin]
iNote [iGetMiscompares child0.local -bin]; iNote [iGetMiscompares child1.leaf.local -bin]
iNote [iGetReadData client -bin]; iNote [iGetStatus]
if {![catch {iGetReadData child1.local}]} {error {implicit wrapper history invented}}
'''
        generated,reference = self.continued(nested(),first+requested,suffix,
            [b'0bxxxxxxxx',b'0bxxxxxxxx',b'0b00000000',b'0b00000000',b'0b0000000000',b'0'])
        manual = self.root/'manual.pdl'
        manual.write_text('iPDLLevel 1\niProcsForModule Chains\n'+first+
            'iWrite enable1 1; iApply\niWrite child1.enable 1; iApply\n'+requested+suffix)
        self.assertEqual(self.run_tool(self.root/'source.icl',manual,'manual-result.pdl'),reference)
        self.assertEqual(generated.count(b'\niApply\n'),5)
        self.assertIn(b'iScan client 27 -si ',generated)


if __name__=='__main__':
    unittest.main(argv=[__file__])
